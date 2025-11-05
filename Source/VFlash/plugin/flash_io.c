/*
** VFlash I/O
** ----------
**
** - Load/Save the configuration
**
** © 1999 by David Gerber <zapek@vapor.com>
** All rights reserved
**
** $Id: flash_io.c,v 1.4 2004/02/28 18:51:07 zapek Exp $
**
*/

#include "config.h"

/* OS */
#include <exec/memory.h>
#ifdef __SASC
#include <dos.h>
#else
#include "macros/vapor.h"
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/asyncio.h>
#include <proto/iffparse.h>
#include <prefs/prefhdr.h>

#include <string.h>

#define ASYNCBUFFERSIZE 8192 /* size of the asynchronous I/O buffer */

#include "debug.h"
#include "flash_mcc_private.h"

extern struct FlashPrefs *flashprefs;

#if USE_PREFS_IO
extern struct SignalSemaphore *PrefsSem;
extern struct Library *IFFParseBase;
extern struct DosLibrary *DOSBase;

/*
 * Prefs array to load/save
 */
struct FlashPrefsArray {
	ULONG id;
	APTR var;
	ULONG size;
};

#ifdef __GNUC__
struct FlashPrefsArray prefs_array[] = {
	MAKE_ID('S','O','U','D'), NULL, sizeof(&flashprefs->vfp_soundswitch),      /* sound switch (audio.device, ahi, etc...) */
	MAKE_ID('B','I','T','S'), NULL, sizeof(&flashprefs->vfp_resolution),        /* 8/16-bit sound */
	MAKE_ID('S','T','R','O'), NULL, sizeof(&flashprefs->vfp_stereo),                /* stereo sound */
	MAKE_ID('S','M','P','L'), NULL, sizeof(&flashprefs->vfp_samplingrate),     /* sample rate */
	MAKE_ID('A','U','D','1'), NULL, sizeof(&flashprefs->vfp_audio.volume),     /* audio.device volume */
	MAKE_ID('A','H','I','1'), NULL, sizeof(&flashprefs->vfp_ahi.unit),             /* AHI unit */
	MAKE_ID('D','O','S','1'), NULL, sizeof(&flashprefs->vfp_dosdriver.name), /* DOS Driver name */
	0, 0, 0
};

/*
 * slink is smarter than ld in that area, we have to initialize it
 * at runtime then.
 */
void init_prefs(void)
{
	prefs_array[0].var = &flashprefs->vfp_soundswitch;
	prefs_array[1].var = &flashprefs->vfp_resolution;
	prefs_array[2].var = &flashprefs->vfp_stereo;
	prefs_array[3].var = &flashprefs->vfp_samplingrate;
	prefs_array[4].var = &flashprefs->vfp_audio.volume;
	prefs_array[5].var = &flashprefs->vfp_ahi.unit;
	prefs_array[6].var = &flashprefs->vfp_dosdriver.name;
}

#else
struct FlashPrefsArray prefs_array[] = {
	MAKE_ID('S','O','U','D'), &flashprefs->vfp_soundswitch, sizeof(&flashprefs->vfp_soundswitch),      /* sound switch (audio.device, ahi, etc...) */
	MAKE_ID('B','I','T','S'), &flashprefs->vfp_resolution, sizeof(&flashprefs->vfp_resolution),        /* 8/16-bit sound */
	MAKE_ID('S','T','R','O'), &flashprefs->vfp_stereo, sizeof(&flashprefs->vfp_stereo),                /* stereo sound */
	MAKE_ID('S','M','P','L'), &flashprefs->vfp_samplingrate, sizeof(&flashprefs->vfp_samplingrate),     /* sample rate */
	MAKE_ID('A','U','D','1'), &flashprefs->vfp_audio.volume, sizeof(&flashprefs->vfp_audio.volume),     /* audio.device volume */
	MAKE_ID('A','H','I','1'), &flashprefs->vfp_ahi.unit, sizeof(&flashprefs->vfp_ahi.unit),             /* AHI unit */
	MAKE_ID('D','O','S','1'), &flashprefs->vfp_dosdriver.name, sizeof(&flashprefs->vfp_dosdriver.name), /* DOS Driver name */
	0, 0, 0
};
#endif


#if USE_ASYNCIO
/*
 * Asynchronous IFF routines
 */
//ULONG ASM SAVEDS iffasyncfunc(__reg(a0, struct Hook *streamHook), __reg(a2, struct IFFHandle *iffh), __reg(a1, struct IFFStreamCmd *iffcmd))
MUI_HOOK(iffasyncfunc, struct IFFHandle *iffh, struct IFFStreamCmd *iffcmd)
{
#ifdef __SASC
	putreg(REG_A4, streamHook->h_Data);
#endif /* __SASC */
	
	switch(iffcmd->sc_Command)
	{
		case IFFCMD_READ:
			if (ReadAsync((struct AsyncFile *)iffh->iff_Stream, iffcmd->sc_Buf, iffcmd->sc_NBytes) == -1)
			{
				return(IFFERR_READ);
			}
			return(0);
			break;

		case IFFCMD_WRITE:
			if (WriteAsync((struct AsyncFile *)iffh->iff_Stream, iffcmd->sc_Buf, iffcmd->sc_NBytes) == -1)
			{
				return(IFFERR_WRITE);
			}
			return(0);
			break;

		case IFFCMD_SEEK:
			if (SeekAsync((struct AsyncFile *)iffh->iff_Stream, iffcmd->sc_NBytes, MODE_CURRENT) == -1)
			{
				return(IFFERR_SEEK);
			}
			return(0);
			break;

		default:
			return(0);
			break;
	}
}
#endif /* USE_ASYNCIO */


/*
 * Writes an IFF chunk
 */
int writechunk(struct IFFHandle *iffh, LONG id, APTR buf, LONG len)
{
	if (!PushChunk(iffh, ID_PREF, id, IFFSIZE_UNKNOWN))
	{
		if (!WriteChunkBytes(iffh, buf, len) != len)
		{
			if (!PopChunk(iffh))
			{
				return(TRUE);
			}
		}
	}
	return(FALSE);
}


/*
 * Reads an IFF chunk
 */
int readchunk(struct IFFHandle *iffh, LONG id, APTR buf, LONG len)
{
	struct ContextNode *cn;	
	
	if (!ParseIFF(iffh, IFFPARSE_SCAN))
	{
		if (cn = CurrentChunk(iffh))
		{
			if (!((cn->cn_Type != ID_PREF) || (cn->cn_ID != id) || (cn->cn_Size != (len))))
			{
				if (!ReadChunkBytes(iffh, buf, cn->cn_Size) != cn->cn_Size)
				{
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}
 

/*
 * Save the preferences to disk
 */
LONG save_prefs(STRPTR filename)
{
	BOOL retval = FALSE;
	BPTR lock;
	struct IFFHandle *iffh;
	struct PrefHeader prhd;
	ULONG i = 0;

	ObtainSemaphore(PrefsSem);

	memset(&prhd, '\0', sizeof(struct PrefHeader));

	DB(("got semaphore\n"));

	if (lock = CreateDir(VPLUGPREFSPATH))
		UnLock(lock);

	if (!(iffh = AllocIFF()))
		goto done;

#if USE_ASYNCIO
	if (AsyncIOBase)
	{
		if (!(iffh->iff_Stream = (ULONG)OpenAsync(filename, MODE_WRITE, ASYNCBUFFERSIZE)))
			goto done;
		InitIFF(iffh, IFFF_FSEEK | IFFF_RSEEK, (struct Hook *)&iffasyncfunc_hook);
	}
	else
#endif /* USE_ASYNCIO */
	{
		if (!(iffh->iff_Stream = Open(filename, MODE_NEWFILE)))
			goto done;
		InitIFFasDOS(iffh);
	}

	if (OpenIFF(iffh, IFFF_WRITE))
		goto done;

	/* IFF PREF FORM */
	if (PushChunk(iffh, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN))
		goto done;

	/* write pref header */
	prhd.ph_Version = PREFS_VERSION;
	prhd.ph_Type    = 0;
	prhd.ph_Flags   = 0;

	if (!writechunk(iffh, ID_PRHD, &prhd, sizeof(struct PrefHeader)))
		goto done;

	DB(("wrote PRHD\n"));

	/* write the whole prefs */
	while (prefs_array[i].id)
	{
		if (!writechunk(iffh, prefs_array[i].id, prefs_array[i].var, prefs_array[i].size))
			goto done;

		i++;
	}

	retval = TRUE;

done:
	if (iffh)
	{
		CloseIFF(iffh);
#if USE_ASYNCIO
		if (AsyncIOBase)
		{
			CloseAsync((struct AsyncFile *)iffh->iff_Stream);
		}
		else
#endif /* USE_ASYNCIO */
		{
			Close(iffh->iff_Stream);
		}
		FreeIFF(iffh);
	}

	if (!(retval)) DeleteFile(filename);

	ReleaseSemaphore(PrefsSem);

	return(retval);
}


/*
 * Loads the preferences from disk
 */
LONG load_prefs(STRPTR filename)
{
	BOOL retval = FALSE;
	struct IFFHandle *iffh;
	struct PrefHeader prhd;
	ULONG i = 0;

	ObtainSemaphore(PrefsSem);

	DB(("got semaphore\n"));

	/* init iff handle */
	if (!(iffh = AllocIFF()))
		goto done;

#if USE_ASYNCIO
	if (AsyncIOBase)
	{
		if (!(iffh->iff_Stream = (ULONG)OpenAsync(filename, MODE_READ, ASYNCBUFFERSIZE)))
			goto done;
		InitIFF(iffh, IFFF_FSEEK | IFFF_RSEEK, &iffasyncfunc_hook);
	}
	else
#endif /* USE_ASYNCIO */
	{
		if (!(iffh->iff_Stream = Open(filename, MODE_OLDFILE)))
			goto done;
		InitIFFasDOS(iffh);
	}

	if (OpenIFF(iffh, IFFF_READ))
		goto done;

	DB(("opened prefsfile\n"));

	/* stop at these chunks */
	while (prefs_array[i].id)
	{
		if (StopChunk(iffh, ID_PREF, prefs_array[i].id))
			goto done;

		i += sizeof(struct FlashPrefsArray) >> 2;
	}

	/* right version ? */
	if (!readchunk(iffh, ID_PRHD, &prhd, sizeof(struct PrefHeader)))
		goto done;

	DB(("found PRHD chunk, phrd.ph_Version == %lu\n", prhd.ph_Version));

	if (prhd.ph_Version < PREFS_VERSION)
		goto done;

	DB(("version is ok\n"));

	i = 0;

	while (prefs_array[i].id)
	{
		if (!readchunk(iffh, prefs_array[i].id, prefs_array[i].var, prefs_array[i].size))
			goto done;

		i += sizeof(struct FlashPrefsArray) >> 2;
	}
	
	retval = TRUE;

done:
	if (iffh)
	{
		CloseIFF(iffh);
		
#if USE_ASYNCIO
		if (AsyncIOBase)
		{
			CloseAsync((struct AsyncFile *)iffh->iff_Stream);
		}
		else
#endif /* USE_ASYNCIO */
		{
			Close(iffh->iff_Stream);
		}
		FreeIFF(iffh);
	}

	ReleaseSemaphore(PrefsSem);

	return(retval);
}

#endif /* USE_PREFS_IO */

void set_default_prefs(void)
{
	#ifdef __GNUC__
	init_prefs();
	#endif
	
	/*
	 * General options
	 */
	flashprefs->vfp_soundswitch = VSS_AMBIENT;
	flashprefs->vfp_resolution = 1;   /* 16 bits */
	flashprefs->vfp_stereo = 1; 	  /* stereo */
	flashprefs->vfp_samplingrate = 1; /* 11000 hz */

	/*
	 * audio.device
	 */
	flashprefs->vfp_audio.volume = 64;

	/*
	 * AHI
	 */
	flashprefs->vfp_ahi.unit = 0;

	/*
	 * CyberSound
	 */
	 // nothing

	/*
	 * DOS driver
	 */
	strcpy(flashprefs->vfp_dosdriver.name, "AUDIO:BITS/16/CHANNELS/2/FREQUENCY/11000/VOLUME/100/POSITION/0/PRIORITY/0/BUFFER/4096/UNIT/0");
}

