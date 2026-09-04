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
/* Forward declarations for statically linked versions */
int imgdec_libinit_internal( struct imgcallback *cbtptr );
void imgdec_setprefs_internal( long img_jpeg_dct, long img_jpeg_dither, long img_jpeg_quant, long img_lamedecode, long img_progressive_jpeg, long img_gif_dither, long img_png_dither );
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
	D( db_init, bug( "initializing..\n" ) );

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_IMGDEC ) );
	}
#endif /* USE_SPLASHWIN */

#ifndef MBX
	/*
	 * Image decoder is now statically linked - no need to open library
	 * Set VIDBase to non-NULL to indicate it's available
	 */
	VIDBase = (struct Library *)1; /* Dummy value to indicate statically linked */
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

	/* Call internal version directly to avoid SAVEDS issues when statically linked */
	imgdec_setprefs_internal(
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
	
	Printf( "[IMGDEC] start_image_decoders() entry, icbt=0x%lx\n", &icbt );
	Flush( Output() );
	
	Printf( "[IMGDEC] About to call imgdec_libinit_internal()...\n" );
	Flush( Output() );
	/* Call internal version directly to avoid SAVEDS issues when statically linked */
	if( imgdec_libinit_internal( &icbt ) )
	{
		Printf( "[IMGDEC] imgdec_libinit() returned TRUE\n" );
		Flush( Output() );
		
		Printf( "[IMGDEC] About to call imgdec_storeprefs()...\n" );
		Flush( Output() );
		imgdec_storeprefs();
		Printf( "[IMGDEC] imgdec_storeprefs() complete\n" );
		Flush( Output() );
		
		Printf( "[IMGDEC] start_image_decoders() succeeded\n" );
		Flush( Output() );
		return( TRUE );
	}
	Printf( "[IMGDEC] imgdec_libinit() returned FALSE\n" );
	Flush( Output() );
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
		/* No need to CloseLibrary - statically linked */
		VIDBase = NULL;
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
