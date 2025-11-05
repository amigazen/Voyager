/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


/*
**
** $Id: imgstub.c,v 1.68 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "voyager_cat.h"
#include <proto/vimgdecode.h>
#include "imgcallback.h"
#include "network.h"
#include "prefs.h"
#include "classes.h"
#include "splashwin.h"
#include "methodstack.h"
#ifndef MBX
#include "bitmapclone.h"
#endif /* !MBX */
void ASM SAVEDS removeclone( __reg( a0, struct BitMap *src ) ); //TOFIX!! sux..
#include "copyright.h"
#include "mui_func.h"


#define FILE void

#include "vjpeglib.h"

struct imgcallback icbt;

#ifndef MBX
struct Library *VIDBase = NULL;
#endif /* !MBX */

#define VID_VERSION 19

#ifndef MBX
static void tryopen( char *n )
{
	char name[ 64 ];

	sprintf( name, "PROGDIR:Plugins/vimgdec_%s.vlib", n );
	D( db_init, bug( "trying to open image decoder %s\n", name ) );
	VIDBase = OpenLibrary( name, VID_VERSION );
}
#endif /* !MBX */

int init_imgdec( void )
{
#ifndef MBX
	extern char hostos[ 8 ];
#endif
	
	D( db_init, bug( "initializing..\n" ) );

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_IMGDEC ) );
	}
#endif /* USE_SPLASHWIN */

#ifndef MBX
	/*
	 * Try to open the image decoders
	 */
#ifdef __MORPHOS__
	if( !VIDBase && strstr( hostos, "MorphOS" ) )
	{
		tryopen( "604e" );
	}
#endif

	/*
	 * MorphOS *needs* the PPC version because
	 * it uses directcalls. What's the point
	 * in having 68k decoding with a PPC version
	 * anyway..
	 */
#ifdef AMIGAOS
	if( !VIDBase && ( SysBase->AttnFlags & AFF_68060 ) && ( SysBase->AttnFlags & AFF_68881 ) )
		tryopen( "68060" );
	if( !VIDBase && ( SysBase->AttnFlags & AFF_68040 ) && ( SysBase->AttnFlags & AFF_FPU40 ) )
		tryopen( "68040fpu" );
	if( !VIDBase && ( SysBase->AttnFlags & AFF_68020 ) && ( SysBase->AttnFlags & AFF_68881 ) )
		tryopen( "68030fpu" );
	if( !VIDBase && ( SysBase->AttnFlags & AFF_68020 ) )
		tryopen( "68020" );
#endif /* AMIGAOS */

	if( !VIDBase )
	{
		MUI_Request( NULL, NULL, 0, ( char * )GS( ERROR ), ( char * )GS( CANCEL ), ( char * )GS( IMGDEC_ERROR ), 0 );
		return( FALSE );
	}
#endif /* !MBX */

	icbt.nets_open = nets_open;
	icbt.nets_state = nets_state;
	icbt.nets_close = nets_close;
	icbt.nets_getdocmem = nets_getdocmem;
	icbt.nets_getdocptr = nets_getdocptr;
	icbt.nets_settomem = nets_settomem;
	icbt.nets_url = nets_url;
	icbt.nets_redirecturl = nets_redirecturl;
	icbt.nets_lockdocmem = nets_lockdocmem;
	icbt.nets_unlockdocmem = nets_unlockdocmem;
	icbt.nets_release_buffer = nets_release_buffer;
	icbt.removeclone = removeclone;
	icbt.nets_errorstring = nets_errorstring;
	icbt.imgcallback_decode_hasinfo = imgcallback_decode_hasinfo;
	icbt.imgcallback_decode_gotscanline = imgcallback_decode_gotscanline;
	icbt.imgcallback_decode_done = imgcallback_decode_done;

	icbt.v_major = VERSION;
	icbt.v_minor = REVISION;
	icbt.v_build = COMPILEREV;

#ifdef __MORPHOS__
	icbt.v_isppc = TRUE;
#else
	icbt.v_isppc = FALSE;
#endif /* !__MORPHOS__ */

	return( TRUE );
}

void imgdec_storeprefs( void )
{
#ifndef MBX
	if( !VIDBase )
	{
		return;
	}
#endif /* !MBX */

	imgdec_setprefs(
		getprefslong( DSI_IMG_JPEG_DCT, JDCT_ISLOW ),
		getprefslong( DSI_IMG_JPEG_DITHER, JDITHER_NONE ),
		getprefslong( DSI_IMG_JPEG_QUANT, FALSE ),
		FALSE,
		getprefslong( DSI_IMG_JPEG_PROGRESSIVE, TRUE ),
		getprefslong( DSI_IMG_GIF_DITHER, JDITHER_NONE ),
		getprefslong( DSI_IMG_PNG_DITHER, JDITHER_NONE )
	);

}


int	start_image_decoders( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( imgdec_libinit( &icbt ) )
	{
		imgdec_storeprefs();
		return( TRUE );
	}
	return( FALSE );
}

void close_image_decoders( void )
{
	D( db_init, bug( "closing decoders..\n" ) );
#ifdef MBX
	imgdec_libexit();
#else
	if( VIDBase )
	{
		imgdec_libexit();
		CloseLibrary( VIDBase );
		VIDBase = 0;
	}
#endif /* !MBX */
	D( db_init, bug( "done closing decoders..\n" ) );
}


/*
 * Those functions are used because it's a pain to write
 * varargs converters
 */
void ASM SAVEDS imgcallback_decode_hasinfo(
	__reg( a0, APTR obj ),
	__reg( a1, struct BitMap *bm ),
	__reg( d0, int img_x ),
	__reg( d1, int img_y ),
	__reg( a2, struct BitMap *maskbm ),
	__reg( a3, struct MinList *imagelist )
)
{
	pushmethod( obj, 6, MM_ImgDecode_HasInfo, bm, img_x, img_y, maskbm, imagelist );
}


void ASM SAVEDS imgcallback_decode_gotscanline(
	__reg( a0, APTR obj ),
	__reg( d0, int min_touched_y ),
	__reg( d1, int max_touched_y )
)
{
	pushmethod( obj, 3, MM_ImgDecode_GotScanline, min_touched_y, max_touched_y );
}


void ASM SAVEDS imgcallback_decode_done( __reg( a0, APTR obj ) )
{
	pushmethod( obj, 1, MM_ImgDecode_Done );
}
