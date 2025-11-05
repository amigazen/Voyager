/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_GLOBALS_H
#define VOYAGER_GLOBALS_H
/*
 * Common includes and symbols
 * ---------------------------
 * © 2001 VaporWare Inc. Co. Ltd. Unlimited.
 *
 * - general symbols
 * - common includes
 * - some parts of the MBX compatibility layer
 * - for SAS/C specific things -> gst.h
 *
 * $Id: globals.h,v 1.5 2003/04/25 19:13:53 zapek Exp $
 */

#define __USE_SYSBASE

#ifdef __MORPHOS__

/*
 * We include only varprotos so that there's no special
 * NewObject() macro generated. Although it works, it's impossible
 * to stuff #ifdefs within those macros and since V uses them a lot
 * we use another way where linklibs are used.
 */

/* no stdargs, they'll come from lib* at linking */
#define NO_PPCINLINE_STDARG 1

#include <exec/types.h>


/*
 * min()/max() without macro side effects.
 */
#define max(a,b) \
	({typeof(a) _a = (a); \
	typeof(b) _b = (b);	\
	_a > _b ? _a : _b;})

#define min(a,b) \
	({typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a > _b ? _b : _a;})


/*
 * MorphOS needs global libbases for the varproto-way
 */
extern struct DosLibrary *DOSBase;
extern struct ExecBase *SysBase;
extern struct Library *IconBase;
extern struct Library *DataTypesBase;
extern struct Library *UtilityBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *DiskfontBase;
extern struct Library *CyberGfxBase;
extern struct Library *LayersBase;
extern struct Library *VIDBase;
extern struct Library *LocaleBase;
extern struct Library *InputBase;
extern struct Library *IFFParseBase;

#endif /* __MORPHOS__ */


#if USE_VAT
/*
 * We use direct calls because gcc optimizes them
 * away and I don't know why. Besides they're pretty
 * pointless and the only interesting call
 * is StrDupPooled().
 */
#ifdef __MORPHOS__
#define NO_VAT_SHORTCUTS
#endif /* __MORPHOS__ */
#include <proto/vat.h>

/*
 * We still use ASyncIO stuff there...
 */
#define MODE_READ   0  /* read an existing file                             */
#define MODE_WRITE  1  /* create a new file, delete existing file if needed */
#define MODE_APPEND 2  /* append to end of existing file, or create new     */
#define MODE_SHAREDWRITE  3  /* create a new file, delete existing file if needed */
#define OpenAsync VAT_OpenAsync
#define CloseAsync VAT_CloseAsync
#define ReadAsync VAT_ReadAsync
#define WriteAsync VAT_WriteAsync
#define ReadCharAsync VAT_ReadCharAsync
#define WriteCharAsync VAT_WriteCharAsync
#define SeekAsync VAT_SeekAsync
#define FGetsAsync VAT_FGetsAsync
#define FGetsAsyncNoLF VAT_FGetsAsyncNoLF
#define FPrintfAsync VAT_FPrintfAsync
#define FtellAsync VAT_FtellAsync
#define UnGetCAsync VAT_UnGetCAsync
#define GetFilesizeAsync VAT_GetFilesizeAsync
#endif /* USE_VAT */

extern char version[];
extern char lversion[];
extern char copyright[];

#define VAPOR_H_BROKEN /* TOFIX: this will go away, eventually */
#include <macros/vapor.h>

/*
 * Frequently used functions
 */
#ifndef __SASC
#define USE_BUILTIN_MATH
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#endif /* !__SASC */

/*
 * To move somewhere maybe..
 */
extern char shorthelpinfotext[ 512 ];

// main.c
#ifndef __MORPHOS__
int sscanf(const char *, const char *, ...);
#endif /* __MORPHOS__ */
#ifdef __SASC
int STDARGS sprintf( char *, const char *, ... );
int STDARGS vsprintf( char *, const char *, void * );
#endif /* __SASC */

// http.c
int stripentity( char *bf, char *name, char *to, int tolen );
void splitrfcname( char *adr, char *to_real, char *to_addr );
void mergeurl( char *from, char *add, char *to );

// Assembler stuff
#ifdef __SASC
extern APTR ASM myNextObject( __reg( a0, APTR ) );
#define NextObject(o) myNextObject(o)
#endif


/* end of to move somewhere */

/*
 * Global Symbol Table
 */
#ifdef __SASC
#include "gst.h"
#endif /* __SASC */


/*
 * Compatibility layer for MBX
 */

/* Any types which MBX has, but AmigaOS doesn't - should be hardly any */

#define FileLock_p BPTR
#define FileHandle_p BPTR
#define FileNo BPTR

/* These should be made to all be v_ types later */

#define rp_DefFont Font
#define rp_AlgoStyle AlgoStyle
#define rp_BitMap BitMap
#define rp_TmpRas TmpRas
#define sc_Width Width
#define sc_Height Height
#define sc_RastPort RastPort
#define sc_ViewPort ViewPort
#define sc_BitMap BitMap
#define sc_LayerInfo LayerInfo
#define v_Task Task
#define vp_ColorMap ColorMap

#endif /* VOYAGER_GLOBALS_H */
