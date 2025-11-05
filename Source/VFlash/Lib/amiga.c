/* amiga.c
**
** Replacement functions missing from libnix & other Amiga specific stuffs
**
** ©1999-2000 by David Gerber <zapek@vapor.com>
**
** $Id: amiga.c,v 1.4 2004/03/07 00:10:09 zapek Exp $
**
*/

#include "../debug.h"

#ifdef AMIGA
#include <exec/execbase.h>
#include <devices/timer.h>
#include <libraries/mpega.h>
#include <proto/mpega.h>
#include <utility/hooks.h>
#include <emul/emulinterface.h>
#include <proto/exec.h>
#include <stdlib.h>

#if 1
#include <proto/dos.h>
#endif

#endif /* AMIGA */

#ifdef PPC
#include <powerup/gcclib/powerup_protos.h>
#include <powerup/ppclib/interface.h>
#endif /* PPC */

#ifdef MBX
#include <stdlib.h>
#include <stdio.h>
#include <system/types.h>
#include <sys/time.h>
#endif /* MBX */

#include "amiga.h"

#ifdef REENTRANT
#define SQRT vflash_global->SQRT
#else
#ifdef MBX
unsigned char *SQRT = NULL;
#else
unsigned char *SQRT;
#endif
#endif /* !REENTRANT */

//extern struct Library *TimerBase;


/*
 * Unix' gettimeofday() replacement
 */
void GetTimeOfDay(struct unixtimeval *tv)
{
#if defined( __POWERUP__ )
	/* gettimeofday() should be implemented in libnixppc but it's broken so */
	extern struct Caos *CaosGetSysTime;
	extern struct ExecBase *SysBase;
	extern struct unixtimeval *GetSysTime_commzone;
	GetSysTime_commzone->tv_sec = tv->tv_sec;
	GetSysTime_commzone->tv_usec = tv->tv_usec;
	CaosGetSysTime->a0 = (ULONG)GetSysTime_commzone;
	CaosGetSysTime->a6 = (ULONG)TimerBase;
	CaosGetSysTime->M68kStart = (APTR)GetSysTime_commzone;
	CaosGetSysTime->M68kLength = sizeof(struct unixtimeval);
	CaosGetSysTime->PPCStart = (APTR)GetSysTime_commzone;
	CaosGetSysTime->PPCLength = sizeof(struct unixtimeval);
	PPCCallOS(CaosGetSysTime);
	DB(("result: tv_sec == %ld, tv_usec == %ld\n", GetSysTime_commzone->tv_sec, GetSysTime_commzone->tv_usec));
	tv->tv_sec = GetSysTime_commzone->tv_sec;
	tv->tv_usec = GetSysTime_commzone->tv_usec;
#elif defined( AMIGA ) || defined( __MORPHOS__ )
	GetSysTime((struct timeval *)tv);
#elif defined( MBX )
	gettimeofday((struct timeval *)tv, 0);
#endif
}

/*
 * SQRT generation
 */
int	makesqrt(void)
{
	int i, j;
	int cnt = 1;
	unsigned char *s;

	if ((s = SQRT = (unsigned char *)malloc(65536)))
	{
		for (i = 0; i < 256; i++)
		{
			for (j = 0; j < cnt; j++)
			{
				*s++ = i;
			}
			cnt += 2;
		}
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}


struct mp3_handle {
	UBYTE *buf;
	LONG bufcnt;
	LONG bufsize;
};

/*
 * mpega support. (XXX: use a custom handle structure!)
 */
//MUI_HOOK(mpegacb, APTR handle, MPEGA_ACCESS *access)
static LONG mpegacb_GATE(void);
static LONG mpegacb_GATE2(struct Hook *h, APTR handle, MPEGA_ACCESS *access);
struct EmulLibEntry mpegacb_func = {
TRAP_LIB, 0, (void (*)(void))mpegacb_GATE };
static LONG mpegacb_GATE(void) {
return (mpegacb_GATE2((void *)REG_A0, (void *)REG_A2, (void *)REG_A1)); }
static struct Hook mpegacb_hook = { { 0, 0}, (void *)&mpegacb_func, (void *)&mpegacb_GATE2 };
static LONG mpegacb_GATE2(struct Hook *h, APTR handle, MPEGA_ACCESS *access)
{
	switch (access->func)
	{
		case MPEGA_BSFUNC_OPEN:
			{
				struct mp3_handle *mp3h;

				if (mp3h = malloc(sizeof(*mp3h)))
				{
					mp3h->buf = ((struct mpega_openbuf *)access->data.open.stream_name)->buf;
					mp3h->bufcnt = 0;
					mp3h->bufsize = ((struct mpega_openbuf *)access->data.open.stream_name)->bufsize;
					access->data.open.stream_size = ((struct mpega_openbuf *)access->data.open.stream_name)->bufsize;; /* that thing is just for calculating time.. useless */
				}
				return ((LONG)mp3h);
			}
			break;

		case MPEGA_BSFUNC_CLOSE:
			if (handle)
			{
				free(handle);
			}
			break;

		case MPEGA_BSFUNC_READ:
			if (handle)
			{
				ULONG size;
				struct mp3_handle *mp3h = handle;

				if (mp3h->bufsize - mp3h->bufcnt < access->data.read.num_bytes)
				{
					size = mp3h->bufsize - mp3h->bufcnt;
				}
				else
				{
					size = access->data.read.num_bytes;
				}

				if (size)
				{
					memcpy(access->data.read.buffer, mp3h->buf, size);
					mp3h->buf += size;
					mp3h->bufcnt += size;
				}
				return (size);
			}
			break;

		case MPEGA_BSFUNC_SEEK:
			if (handle)
			{
				/* XXX: no seeking needed I think :) */
				{
					return (1); /* error */
				}
			}
			break;
	}
	return (0);
}


static UBYTE *pcm[2];
struct Library *MPEGABase;

void * mpega_open(struct mpega_openbuf *ob)
{
	if (!MPEGABase)
	{
		MPEGABase = OpenLibrary("mpega.library", 2);
	}

	if (MPEGABase)
	{
		if (pcm[0] = malloc(MPEGA_PCM_SIZE * 2))
		{
			if (pcm[1] = malloc(MPEGA_PCM_SIZE * 2))
			{
				APTR handle;
				MPEGA_CTRL mpa_ctrl;

				mpa_ctrl.bs_access = &mpegacb_hook;
				mpa_ctrl.layer_1_2.force_mono = 0;
				mpa_ctrl.layer_1_2.mono.freq_div = 1;
				mpa_ctrl.layer_1_2.mono.quality = MPEGA_QUALITY_HIGH;
				mpa_ctrl.layer_1_2.mono.freq_max = 48000;
				mpa_ctrl.layer_1_2.stereo.freq_div = 1;
				mpa_ctrl.layer_1_2.stereo.quality = MPEGA_QUALITY_HIGH;
				mpa_ctrl.layer_1_2.stereo.freq_max = 48000;
				mpa_ctrl.layer_3.force_mono = 0;
				mpa_ctrl.layer_3.mono.freq_div = 1;
				mpa_ctrl.layer_3.mono.quality = MPEGA_QUALITY_HIGH;
				mpa_ctrl.layer_3.mono.freq_max = 48000;
				mpa_ctrl.layer_3.stereo.freq_div = 1;
				mpa_ctrl.layer_3.stereo.quality = MPEGA_QUALITY_HIGH;
				mpa_ctrl.layer_3.stereo.freq_max = 48000;
				mpa_ctrl.check_mpeg = 0;
				mpa_ctrl.stream_buffer_size = 0; /* mpegalib ignores that anyway */

				if (handle = MPEGA_open((char *)ob, &mpa_ctrl))
				{
					return (handle);
				}
				free(pcm[1]);
			}
			free(pcm[0]);
		}
		CloseLibrary(MPEGABase); /* XXX: optimize that maybe.. */
		MPEGABase = NULL;
	}
	return (NULL);
}


void mpega_close(void * handle)
{
	MPEGA_close(handle);

	free(pcm[1]);
	free(pcm[0]);

	CloseLibrary(MPEGABase); /* XXX: there too */
	MPEGABase = NULL;
}


long mpega_decode(void * handle, char *buf, int stereo) /* XXX: need nbSamples too! */
{
	LONG cnt;
	LONG rc = 0;

	cnt = MPEGA_decode_frame(handle, (WORD **)pcm);

	//dprintf("*** mpega bitrate: %ld\n", ((MPEGA_STREAM *)handle)->bitrate);
	//dprintf("*** mpega version: %ld\n", ((MPEGA_STREAM *)handle)->norm);
	//dprintf("*** mpega frequency: %ld\n", ((MPEGA_STREAM *)handle)->frequency);

	if (cnt > 0)
	{
		ULONG len = cnt * 2;

		while (len)
		{
			if (stereo)
			{
				*((ULONG *)buf)++ = (*(UWORD *)(pcm[0][rc]) << 16) | (*(UWORD *)(pcm[1][rc]));
				len -= 4;
				rc += 4;
			}
			else
			{
				*buf++ = pcm[0][rc];
				*buf++ = pcm[0][rc + 1];
				len -= 2;
				rc += 2;
			}
		}
	}
	else
	{
		if (cnt != -1)
		{
			dprintf("error: cnt: %ld\n", cnt);
		}
	}
	return (rc);
	/* XXX: no error handling for now :) */
}


unsigned long mpega_getsize(void * handle)
{
	MPEGA_STREAM *ms = handle;

	return (ms->ms_duration * ms->frequency * ms->channels * 2 / 1000);
}


#if 1
static int streamnum;

void extract_streamnum(char *buf, int size)
{
	char path[32];
	BPTR f;

	sprintf(path, "ram:stream_%02ld.mp3", streamnum++);

	if (f = Open(path, MODE_NEWFILE))
	{
		Write(f, buf, size);
		Close(f);
	}
}
#endif
