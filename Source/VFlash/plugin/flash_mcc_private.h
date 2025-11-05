#ifndef FLASH_MCC_H
#define FLASH_MCC_H
/*
** Flash.mcc
** ---------
** - Shockwave Flash display class
**
** © 1999 by David Gerber <zapek@vapor.com>
** All rights reserved
**
** Unregistered MUI class, serial number: 4023706747
**
** $Id: flash_mcc_private.h,v 1.3 2004/02/28 18:51:08 zapek Exp $
**
*/

#define CRIPPLED_SOUND // remove once it's done

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <graphics/rastport.h>
#ifndef LIBRARIES_MUI_H
#ifdef __SASC
#include "libraries/mui.h"
#else
#include <libraries/mui.h>
#endif
#endif

#include "proto/v_plugin.h"

/* Application */
#define APPNAME "VFlash"
#define AUTHOR "David Gerber"
#define EMAIL "zapek@vapor.com"


#define MUIC_Flash "Flash.mcc"
#define FlashObject MUI_NewObject(MUIC_Flash

#define MCC_VFLASH_TAGBASE (TAG_USER|(1307<<16)+0x712)
#define MCC_VFLASH_ID(x) (MCC_VFLASH_TAGBASE+x)

extern struct MUI_CustomClass *FlashClass;

/* structs */

/*
 * This structure is exactly the same as
 * the struct FlashEvent, except it contains a MinNode
 * for list handling.
 */
struct flashevent {
	struct MinNode n;
	int	type;
	int	x;
	int	y;
	int	key;
};


struct eventzone
{
	struct MinList eventlist;   	/* keypresses, mousebuttons...*/

	struct SignalSemaphore eventsem;
	APTR eventpool;
};

struct StartupData
{
	char  *docmem;
	ULONG size;
	char  *obj;
	struct FlashPrefs  *vfp;
	struct vplug_functable *ft;
	int   exitstatus;
};




struct Data
{
	APTR   		process;   		 /* VFlash_680x0.module process */
	APTR   		ppctask;		 /* VFlash_ppc.module task */
	int    		isrunning;		 /* VFlash process is running */
	APTR   		nethandle;		 /* NStream handle */
	ULONG  		gfxmode;		 /* GFX mode */
	ULONG  		pixfmt;			 /* pixel format (CGFX), warning: also used for RECTFMT_* values for WritePixelArray() call ! */
	struct 		RenderInfo *ri;  /* used by P96 */
	APTR   		canvas;    		 /* rendering canvas */
	ULONG  		modulo;     	 /* bytes per row (bytes per lines, modulo) */
	ULONG  		bpp;       		 /* bytes per pixel (bpp) */
	APTR		ReplyPort;
	APTR		elfobject;
	int			gotdone_done;
	int			setup_done;

	APTR		cmpcanvas;
	int			canvassize;

	APTR 		startupmsg;
	struct StartupData *startupdata;
	APTR		ppcmempool;		   /* only used for the PPC version */

	UBYTE 		*rgb16_to_8; 	  /* Conversion table */
	UBYTE 		*tempclut;		  /* Temporary CLUT array */
	ULONG		bg_red;
	ULONG		bg_green;
	ULONG		bg_blue;
	int			pen_bg;
	int			pen_fg;
	int			pen_bar;
	int 		bar_y;
	char 		*baseref;
	int 		length;

	struct 	eventzone *ez;

	LONG clip_x;
	LONG clip_y;
	LONG clip_width;
	LONG clip_height;

	LONG width; 			/* Shortcut variables */
	LONG height;
	LONG wh;

	int	mouse_x;		   /* mousemoves, ONE per frame is enough */
	int	mouse_y;

	int is_off_window;

	int shutting_down;

	struct BitMap *tempbm;
	unsigned char cmp8[ 256 ];
	char status[ 256 ];

	struct MUI_EventHandlerNode ehn;

	ULONG eventticks;
};


#if defined(__GNUC__) || defined(__DCC__) /* TOFIX: __reg() should work. can't be bothered atm) */
//ULONG dispatch(struct IClass *cl, Object *obj, Msg msg);
#else
ULONG ASM SAVEDS dispatch(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, Msg msg));
#endif

/* Attributes */
#define MA_Flash_P96Modulo 	    MCC_VFLASH_ID(0x4001)   /* V1 s  ULONG */
#define MA_Flash_bpp 		  	MCC_VFLASH_ID(0x4003)   /* V1 sg UBYTE */
#define MA_Flash_CGFXFormat	  	MCC_VFLASH_ID(0x4006)   /* V1 sg ULONG */
#define MA_Flash_P96Format	  	MCC_VFLASH_ID(0x4007)   /* V1 sg ULONG */
#define MA_Flash_Running	  	MCC_VFLASH_ID(0x4018)   /* V1 s  ULONG */
#define MA_Flash_MouseX		  	MCC_VFLASH_ID(0x4024)   /* V1  g int   */
#define MA_Flash_MouseY		  	MCC_VFLASH_ID(0x4025)   /* V1  g int   */
#define MA_Flash_GFXMode	    MCC_VFLASH_ID(0x4026)	/* V2  g ULONG */

/* Methods */
#define MM_Flash_Redraw 	  			MCC_VFLASH_ID(0x4010)
#define MM_Flash_SyncDraw 	  			MCC_VFLASH_ID(0x4011)
#define MM_Flash_GetNextEvent 			MCC_VFLASH_ID(0x4012)
#define MM_Flash_FreeEvent	  			MCC_VFLASH_ID(0x4013)
#define MM_Flash_GetCMP8	  			MCC_VFLASH_ID(0x4014)
#define MM_Flash_GotoURL	  			MCC_VFLASH_ID(0x4015)
#define MM_Flash_UnLockdocmem			MCC_VFLASH_ID(0x4016)
#define MM_Flash_AllocCanvas			MCC_VFLASH_ID(0x4017)
#define MM_Flash_Launch					MCC_VFLASH_ID(0x4019)
#define MM_Flash_ClearDrawArea			MCC_VFLASH_ID(0x4020)

/* Structures */
struct MP_Flash_SyncDraw {
	ULONG MethodID;
	LONG clip_x;
	LONG clip_y;
	LONG clip_width;
	LONG clip_height;
};

struct MP_Flash_FreeEvent {
	ULONG MethodID;
	APTR event; // sure ?
};

struct MP_Flash_GetCMP8 {
	ULONG MethodID;
	APTR area; // sure ?
};

struct MP_Flash_GotoURL {
	ULONG MethodID;
	STRPTR url;
	STRPTR target;
};

struct MP_Flash_GetNextEvent {
	ULONG MethodID;
	LONG type;
};

/* Special values */
#define MV_Flash_P96Mode		0
#define MV_Flash_CGFXMode		1
#define MV_Flash_ChunkyCLUTMode	2
#define MV_Flash_PlanarMode		3

/* -------------------------------- PrefsWin ------------------------------------

 Preference Window

*/

extern struct MUI_CustomClass *PrefsClass;

/* Prefs */
#define PREFS_VERSION ((UBYTE)2)
#define PREFSFILE "PROGDIR:Plugins/Data/VFlash.data"

#define VFP_SOUND_OFF 0
#define VFP_SOUND_ON  1

struct FlashAudioDevicePrefs {
	ULONG volume;		 /* 0..64 */
};

struct AudioDeviceGroup {
	APTR sl_volume;
};


struct FlashAHIPrefs {
	ULONG unit;
};

struct AHIGroup {
	APTR cyc_unit;
};


struct FlashDOSDriverPrefs {
	char name[256];
};

struct DOSDriverGroup {
	APTR str_name;
};


struct PrefsData
{
	APTR soundswitch;	 						/* selects between the audio modes */
	APTR stereo;
	APTR resolution;
	APTR samplingrate;
	APTR audiomaingrp;
	APTR audiogrp;
	APTR audiogrp_child;
#ifdef CRIPPLED_SOUND
	APTR audioobjs[7];
#else
	APTR audioobjs[6];
#endif
	int  group_changed;
	struct AudioDeviceGroup	audio;
	struct AHIGroup 		ahi;
	struct DOSDriverGroup	dosdriver;
};


struct FlashPrefs
{
	ULONG 							vfp_soundswitch;		/* sound mode */
	struct FlashAudioDevicePrefs 	vfp_audio;
	struct FlashAHIPrefs 			vfp_ahi;
	struct FlashDOSDriverPrefs 		vfp_dosdriver;
	ULONG 							vfp_resolution;			/* 8/16-bit sound */
	ULONG 							vfp_stereo;				/* guess :)	*/
	ULONG 							vfp_samplingrate; 		/* 5500, 11000, 22000 or 44000 */
};

/*
 * VFlash Sound Switch
 */
enum {
	VSS_NONE,
	VSS_AUDIODEVICE, /* NYI */
	VSS_AHI,         /* NYI */
	VSS_CYBERSOUND,  /* NYI */
	VSS_DOSDRIVER,   /* NYI */
	VSS_AMBIENT,
};


/* Methods */
#define MM_Prefs_StorePrefs       MCC_VFLASH_ID(0x4030)
#define MM_Prefs_ChangeAudioGroup MCC_VFLASH_ID(0x4031)

/* Attributes*/
#define MA_Prefs_AboutWin		  MCC_VFLASH_ID(0x4032)

/* Special values */
#define MV_Prefs_None			  0
#define MV_Prefs_AudioDevice	  1
#define MV_Prefs_AHI			  2
#define MV_Prefs_CyberSound		  3
#define MV_Prefs_DOSDriver		  4
#define MV_Prefs_AmbientSound     5


/* Structures */
struct MP_Prefs_ChangeAudioGroup {
	ULONG MethodID;
	ULONG group;
};

#ifndef __GNUC__ /* gcc still sucks */
ULONG __saveds __asm prefs_dispatch(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
ULONG __asm __saveds iffasyncfunc(register __a0 struct Hook *streamHook, register __a2 struct IFFHandle *iffh, register __a1 struct IFFStreamCmd *iffcmd);
extern struct Hook IFFHook;
#else
extern struct EmulLibEntry GATEflash_dispatch;
extern struct EmulLibEntry GATEprefs_dispatch;
//ULONG prefs_dispatch( struct IClass *cl, Object *obj, Msg msg);
//ULONG iffasyncfunc(struct Hook *streamHook, struct IFFHandle *iffh, struct IFFStreamCmd *iffcmd);
extern struct Hook IFFHook;
#endif


#endif /* FLASH_MCC_H */

