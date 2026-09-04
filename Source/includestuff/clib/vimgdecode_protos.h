#ifndef CLIB_VIMGDECODE_PROTOS_H
#define CLIB_VIMGDECODE_PROTOS_H

struct Screen;
struct imgclient;
struct BitMap;
struct imgcallback;

/*
 * $Id: vimgdecode_protos.h,v 1.3 2001/08/28 20:33:19 zapek Exp $
 */

#include <libraries/vimgdecode.h>
#include <macros/compilers.h>

/*
 * The image decoder used to be vimgdecode.library and these were reached
 * through the libcall pragmas in <pragmas/vimgdecode_pragmas.h>, i.e.
 * "move.l VIDBase,a6 / jsr -offset(a6)". It is linked into the executable
 * now, nothing opens the library, and imgstub.c only parks a dummy value in
 * VIDBase so its availability checks still pass - so every one of those calls
 * jumped through a bogus base and took the machine down on the first image.
 *
 * These are therefore plain function calls now (proto/vimgdecode.h no longer
 * pulls the pragmas in). The definitions in imgdecode.c take their arguments
 * in registers, so the prototypes must carry the same ASM/__reg decoration or
 * the caller puts the arguments somewhere the callee never looks.
 */

void ASM imgdec_tick( void );

int ASM imgdec_getinfo(
	__reg( a0, struct imgclient *client ),
	__reg( a1, struct BitMap **bm ),
	__reg( a2, int *xsize ),
	__reg( a3, int *ysize )
);

void ASM imgdec_close( __reg( a0, struct imgclient *client ) );

APTR ASM imgdec_open(
	__reg( a0, char *url ),
	__reg( a1, APTR clientobject ),
	__reg( a2, char *referer ),
	__reg( d0, int reload )
);

int ASM imgdec_dowehave( __reg( a0, char *url ) );

void ASM imgdec_setclientobject(
	__reg( a0, struct imgclient *client ),
	__reg( a1, APTR object )
);

int ASM imgdec_setdestscreen(
	__reg( a0, struct Screen *scr ),
	__reg( d0, int bgpen ),
	__reg( d1, int framepen ),
	__reg( d2, int shadow ),
	__reg( d3, int shine )
);

struct MinList * ASM imgdec_getimagelist( __reg( a0, struct imgclient *client ) );

void ASM imgdec_flushimages( void );

int ASM imgdec_isdone( __reg( a0, struct imgclient *client ) );

void ASM imgdec_abortload( __reg( a0, struct imgclient *client ) );

void ASM imgdec_markforreload( __reg( a0, struct imgclient *client ) );

void ASM imgdec_setprefs(
	__reg( d0, long img_jpeg_dct ),
	__reg( d1, long img_jpeg_dither ),
	__reg( d2, long img_jpeg_quant ),
	__reg( d3, long img_lamedecode ),
	__reg( d4, long img_progressive_jpeg ),
	__reg( d5, long img_gif_dither ),
	__reg( d6, long img_png_dither )
);

int ASM imgdec_libinit( __reg( a0, struct imgcallback *cbtptr ) );

void ASM imgdec_libexit( void );

int ASM imgdec_getrepeatcnt( __reg( a0, struct imgclient *client ) );

void ASM imgdec_getinfostring(
	__reg( a0, struct imgclient *client ),
	__reg( a1, STRPTR buffer )
);

struct BitMap * ASM imgdec_getmaskbm( __reg( a0, struct imgclient *client ) );

char * ASM imgdec_errormsg( __reg( a0, struct imgclient *client ) );

int ASM imgdec_isblank( __reg( a0, struct imgclient *client ) );

void ASM imgdec_setdebug( __reg( d0, int lvl ) );

int ASM imgdec_maskused( __reg( a0, struct imgclient *client ) );

#endif /* !CLIB_VIMGDECODE_PROTOS_H */
