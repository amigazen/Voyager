/*
** VFlash plugin interface
** -----------------------
**
** © 1999-2000 by David Gerber <zapek@vapor.com>
** All rights reserved
**
** $Id: flash_vplug.c,v 1.5 2004/02/28 18:51:08 zapek Exp $
**
*/

#include "config.h"

#define CLIB_V_PLUGIN_PROTOS_H /* hack for sas/c :) */
#define PROTO_V_PLUGIN_H /* and hack for gcc.. sigh */
#include <libraries/v_plugin.h>

#ifdef __SASC
#include <proto/v_plugin.h>
#else
#include <clib/alib_protos.h>
#include <clib/v_plugin_protos.h>
#include "macros/vapor.h"
#endif
#include "rev.h"
#include "debug.h"

/* OS */
#include <exec/memory.h>
#ifdef __SASC
#include <dos.h>
#endif
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <libraries/dos.h>

/* MUI */
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>
//#include <libraries/mui.h>

/* Protos */
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/console.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/asyncio.h>

#if USE_CGX /* TOFIX: cleanup, use proto only */
#ifdef __SASC
#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
#else
#include <proto/cybergraphics.h>
#endif
#include <cybergraphx/cybergraphics.h>
struct Library *CyberGfxBase;
#endif /* USE_CGX */

#if USE_PICASSO96
#include <proto/picasso96.h>
struct Library *P96Base;
#endif /* USE_PICASSO96 */

#include "flash_mcc_private.h"

extern void set_default_prefs(void);
#if USE_PREFS_IO
extern LONG load_prefs(STRPTR filename);
extern LONG save_prefs(STRPTR filename);
#endif

static ULONG flash_imageCMap32[]={0x100000,
	0x14141414,0x10101010,0x14141414,
	0x5D5D5D5D,0x35353535,0x28282828,
	0x39393939,0x41414141,0x82828282,
	0x61616161,0x55555555,0x69696969,
	0x65656565,0x6D6D6D6D,0x9E9E9E9E,
	0xAAAAAAAA,0x3D3D3D3D,0x2D2D2D2D,
	0xAAAAAAAA,0x41414141,0x69696969,
	0x92929292,0x8A8A8A8A,0x49494949,
	0xDBDBDBDB,0x61616161,0x7D7D7D7D,
	0xCACACACA,0x79797979,0x92929292,
	0xB2B2B2B2,0xB2B2B2B2,0xC6C6C6C6,
	0xC2C2C2C2,0xC6C6C6C6,0x5D5D5D5D,
	0xDFDFDFDF,0xA6A6A6A6,0xA2A2A2A2,
	0xCACACACA,0xC2C2C2C2,0x08080808,
	0xDFDFDFDF,0xD6D6D6D6,0xD2D2D2D2,
	0xFBFBFBFB,0xFFFFFFFF,0xFBFBFBFB,
	0x00000000};

static UWORD __chip flash_imageData[112]={
	/* BitPlane 0 */
	0xffff,0xff00,0xff83,0xfe00,0xff7a,0xfe00,0xfe0e,0x7e00,
	0xfde1,0x3e00,0xffe7,0x3e00,0xfe60,0x7e00,0xfeb8,0xfe00,
	0xfea2,0x7e00,0xfc0b,0x3e00,0xfe7d,0x3e00,0xff5e,0xfe00,
	0xff83,0xfe00,0x8000,0x0000,
	/* BitPlane 1 */
	0xffff,0xff00,0xffc3,0xfe00,0xff03,0xfe00,0xfe0e,0xfe00,
	0xfe51,0x7e00,0xfef3,0xbe00,0xfdff,0x7e00,0xfe5e,0xfe00,
	0xfcf5,0xbe00,0xfffe,0x7e00,0xfee2,0xfe00,0xff7f,0xfe00,
	0xffc7,0xfe00,0x8000,0x0000,
	/* BitPlane 2 */
	0xffff,0xff00,0xffff,0xff00,0xff84,0xff00,0xff57,0x7f00,
	0xfeee,0xbf00,0xfe0a,0x7f00,0xfc00,0xbf00,0xff01,0x3f00,
	0xfd1c,0x3f00,0xfc14,0xbf00,0xff22,0x3f00,0xffa3,0xff00,
	0xfffb,0xff00,0xffff,0xff00,
	/* BitPlane 3 */
	0xffff,0xff00,0xffff,0xfe00,0xfff9,0xfe00,0xfff0,0xfe00,
	0xff16,0x7e00,0xfc04,0x3e00,0xfc07,0x3e00,0xfce6,0x3e00,
	0xfe15,0xfe00,0xfe34,0xfe00,0xfe36,0xfe00,0xff23,0xfe00,
	0xffc7,0xfe00,0x8000,0x0000};

struct BitMap flash_imageBitMap={4,14,0,4,0,(PLANEPTR)&flash_imageData[0],(PLANEPTR)&flash_imageData[28],(PLANEPTR)&flash_imageData[56],(PLANEPTR)&flash_imageData[84]};

#ifdef POWERUP
struct Library *PPCLibBase;
#endif /* POWERUP */

struct GfxBase 			*GfxBase;
struct Library 			*MUIMasterBase;
struct DosLibrary 		*DOSBase;
struct Library 			*UtilityBase;
struct Library			*IFFParseBase;
struct IntuitionBase    *IntuitionBase;
struct Library			*AsyncIOBase;

struct MUI_CustomClass	*FlashClass = NULL;
struct MUI_CustomClass	*PrefsClass = NULL;

struct FlashPrefs		*flashprefs = NULL; /* and add a semaphore for the prefs */
struct SignalSemaphore  *PrefsSem = NULL;

struct vplug_functable *ft;

/* Failure message */

void Fail(STRPTR txt)
{
	static struct IntuiText BodyText = { 255, 255, 0, 4, 4, NULL, NULL, NULL };
	static struct IntuiText ContinueText = { 255, 255, 0, 4, 4, NULL, "Ok", NULL };

	BodyText.IText = txt;
	AutoRequest(NULL, &BodyText, NULL, &ContinueText, 0, 0, 320, 60);
}

#ifdef __SASC
long ASM SAVEDS __UserLibInit(__reg(a6, struct Library *libbase))
{
	DB(("opened\n"));
	libbase->lib_Node.ln_Pri = -128;
	return( 0 );
}
#else
ULONG lib_init(struct ExecBase *SBase)
{
	SysBase = SBase;

	/* XXX: we can't set ln_Pri from here.. we should do that in lib_open or so.. or have SysBase set in lib_init then put the real base here or so.. check */
	return (TRUE);
}

ULONG lib_open(void)
{
	return (TRUE);
}

void lib_cleanup(void)
{
}

#endif /* __SASC */

struct TagItem tags[] = {
	VPLUG_Query_Version, VERSION,
	VPLUG_Query_Revision, REVISION,
	VPLUG_Query_Copyright, (ULONG)"(C) 1999-2004 David Gerber <zapek@vapor.com>, All Rights Reserved",
	VPLUG_Query_Infostring, (ULONG)"Voyager Flash Plugin", 
	VPLUG_Query_APIVersion, 3,
	VPLUG_Query_RegisterMIMEType, (ULONG)"application/futuresplash",
	VPLUG_Query_RegisterMIMEType, (ULONG)"application/x-shockwave-flash",
	VPLUG_Query_RegisterMIMEExtension, (ULONG)"swf",
	VPLUG_Query_PluginID, (ULONG)"Shockwave Flash",
	VPLUG_Query_HasPrefs, TRUE,
	VPLUG_Query_PPC_DirectCallbacks, TRUE,
	TAG_DONE
};


/* Get information */
struct TagItem * ASM SAVEDS VPLUG_Query(void)
{
	return (tags);
}


struct ExecBase *SysBase;

/* Setup */
BOOL ASM SAVEDS VPLUG_Setup(__reg(a0, struct vplug_functable *table))
{
#if USE_PICASSO96
	struct Library *l;
#endif /* USE_PICASSO96 */

	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)))
	{
		DB(("Couldn't open intuition.library v39 or higher.")); /* <shrug> */
		return(FALSE);
	}

	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 18)))
	{
		Fail("Couldn't open "MUIMASTER_NAME" v18 or higher.");
		return(FALSE);
	}
	
	if (!(flashprefs = AllocVec(sizeof(struct FlashPrefs), MEMF_PUBLIC | MEMF_CLEAR)))
	{
		Fail("Not enough memory.");
		return(FALSE);
	}

#ifdef POWERUP
	PPCLibBase = OpenLibrary("ppc.library", 46);
#endif /* POWERUP */

#if USE_CGX
	CyberGfxBase = OpenLibrary("cybergraphics.library", 0);
#endif /* USE_CGX */
	
	GfxBase = (APTR)OpenLibrary("graphics.library", 39);

#if USE_ASYNCIO
	AsyncIOBase = OpenLibrary("asyncio.library", 38);
#endif /* USE_ASYNCIO */

#if USE_PICASSO96
	/*
	 * We only want the "real" P96 library
	 */
	if (!(l = OpenLibrary("cgxsystem.library", 0)))
	{
		P96Base = OpenLibrary("Picasso96API.library", 2);
	}
	else
	{
		CloseLibrary(l);
	}
#endif /* USE_PICASSO96 */

	if (!(IFFParseBase = OpenLibrary("iffparse.library", 36)))
	{
		Fail("Couldn't open iffparse.library v36 or higher.");
		return(FALSE);
	}

	if (!(PrefsSem = AllocVec(sizeof(struct SignalSemaphore), MEMF_ANY | MEMF_CLEAR)))
	{
		Fail("Not enough memory.");	
		return(0);
	}

	InitSemaphore(PrefsSem);

	/* setup custom classes */
	if (!(FlashClass = MUI_CreateCustomClass(NULL, MUIC_Area, NULL, sizeof(struct Data), DISPATCHERREF2(flash))))
	{
		Fail("Couldn't create custom class.");
		return(FALSE);
	}
	if (MUIMasterBase->lib_Version >= 20)
	{
		/* for MUI debugs */
		FlashClass->mcc_Class->cl_ID = "FlashClass";
	}
	DB(("FlashClass ok\n"));

	DOSBase = (struct DosLibrary *)FlashClass->mcc_DOSBase;
	UtilityBase = FlashClass->mcc_UtilityBase;

	if (!(PrefsClass = MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct PrefsData), DISPATCHERREF2(prefs))))
	{
		Fail("Couldn't create custom class."); 
		return(FALSE);
	}
	if (MUIMasterBase->lib_Version >= 20)
	{
		/* for MUI debugs */
		PrefsClass->mcc_Class->cl_ID = "FlashPrefsClass";
	}
	DB(("PrefsClass ok\n"));

#ifdef __SASC
	/* saves the A4 register (this is needed for a library + dispatchers' hooks) */
#if USE_PREFS_IO
	IFFHook.h_Data = FlashClass->mcc_Class->cl_UserData = PrefsClass->mcc_Class->cl_UserData = getreg(REG_A4);
#endif
#endif /* __SASC */

	ft = table;

	/*
	 * Setting up default prefs
	 */
	set_default_prefs();

#if USE_PREFS_IO
	load_prefs(PREFSFILE);
#endif

	return (TRUE);
}

/* Cleanup */
void ASM SAVEDS VPLUG_Cleanup(void)
{
	if (PrefsSem)
	{
		DB(("waiting for prefs to be written...\n"));
		ObtainSemaphore(PrefsSem);
		ReleaseSemaphore(PrefsSem);
		FreeVec(PrefsSem);
	}

	if( PrefsClass )
	{
		MUI_DeleteCustomClass( PrefsClass );
	}

	if( FlashClass )
	{
		MUI_DeleteCustomClass( FlashClass );
	}

#if USE_ASYNCIO
	if (AsyncIOBase)
	{
		CloseLibrary(AsyncIOBase);
	}
#endif /* USE_ASYNCIO */
	
#if USE_CGX
	if (CyberGfxBase)
	{
		CloseLibrary(CyberGfxBase);
	}
#endif /* USE_CGX */

	if (GfxBase)
	{
		CloseLibrary((APTR)GfxBase);
	}

#if USE_PICASSO96
	if (P96Base)
	{
		CloseLibrary(P96Base);
	}
#endif /* USE_PICASSO96 */

#ifdef POWERUP
	if (PPCLibBase)
	{
		CloseLibrary(PPCLibBase);
	}
#endif /* POWERUP */

	CloseLibrary(MUIMasterBase);
	CloseLibrary((struct Library *)IntuitionBase);
}

/* Embedding objects, V will NewObject() it */
APTR ASM SAVEDS VPLUG_GetClass(__reg(a0, STRPTR mimetype))
{
	DB(("asking for class, returning 0x%lx\n", FlashClass->mcc_Class));
	return (FlashClass->mcc_Class);
}

APTR ASM SAVEDS VPLUG_ProcessURLMethod(__reg(a0, STRPTR url))
{
	return (0); // dummy
}

APTR ASM SAVEDS VPLUG_GetURLData(__reg(a0, APTR handle))
{
	return (0); // dummy
}

STRPTR ASM SAVEDS VPLUG_GetURLMIMEType(__reg(a0, APTR handle))
{
	return (0); // dummy
}

void ASM SAVEDS VPLUG_FreeURLData(__reg(a0, APTR handle))
{
	return; // dummy
}

void ASM SAVEDS VPLUG_FinalSetup(void)
{
	return; // dummy
}

BOOL ASM SAVEDS VPLUG_GetInfo(__reg(a0, struct VPlugInfo *pi), __reg(a1, APTR nethandle))
{
	return (TRUE); // we do nothing :)
}

int ASM SAVEDS VPLUG_GetURLDataSize(__reg(a0, APTR handle))
{
	return (0); // dummy
}

int ASM SAVEDS VPLUG_ProcessURLString(__reg(a0, STRPTR url))
{
	return (0); // dummy
}

void ASM SAVEDS VPLUG_Hook_Prefs(__reg(d0, ULONG methodid), __reg(a0, struct vplug_prefs *prefs))
{
	switch (methodid)
	{
		case VPLUGPREFS_Setup:
			prefs->bitmap = &flash_imageBitMap;
			prefs->colormap = flash_imageCMap32;
			break;

		case VPLUGPREFS_Create:
			DB(("got prefs create\n"));
				prefs->object = VGroup,
									Child, NewObject(PrefsClass->mcc_Class, NULL, TAG_DONE),
								End;
			break;

		case VPLUGPREFS_Dispose:
			DB(("got prefs dispose\n"));
			MUI_DisposeObject(prefs->object);
			break;

		case VPLUGPREFS_Use:
			DB(("got use prefs\n"));
			DoMethod(prefs->object, MM_Prefs_StorePrefs);
			break;

		case VPLUGPREFS_Save:
			DB(("got save prefs\n"));
#if USE_PREFS_IO
			if (!(save_prefs(PREFSFILE)))
			{
				Fail("Couldn't save prefs to disk\n");
			}
#endif
			break;
	}
	return;
}

