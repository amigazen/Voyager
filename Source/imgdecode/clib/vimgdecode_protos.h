#ifndef CLIB_VIMGDECODE_PROTOS_H
#define CLIB_VIMGDECODE_PROTOS_H
/*
 * $Id: vimgdecode_protos.h,v 1.3 2001/08/28 20:33:19 zapek Exp $
 */

#include <libraries/vimgdecode.h>

void imgdec_tick( void );
int imgdec_getinfo( struct imgclient *client, struct BitMap **bm, int *xsize, int *ysize );
void imgdec_close( struct imgclient *client );
APTR imgdec_open( char *url, APTR clientobject, char *referer, int reload );
int imgdec_dowehave( char *url );
void imgdec_setclientobject( struct imgclient *client, APTR object );
int imgdec_setdestscreen( struct Screen *scr, int bgpen, int framepen, int shadow, int shine );
struct MinList *imgdec_getimagelist( struct imgclient *client );
void imgdec_flushimages( void );
int imgdec_isdone( struct imgclient *client );
void imgdec_abortload( struct imgclient *client );
void imgdec_markforreload( struct imgclient *client );
void imgdec_setprefs( int, int, int, int, int, int, int );
int	imgdec_libinit( APTR );
void imgdec_libexit( void );
int imgdec_getrepeatcnt( struct imgclient *client );
void imgdec_getinfostring( struct imgclient *client, STRPTR );
struct BitMap *imgdec_getmaskbm( struct imgclient *client );
char *imgdec_errormsg( struct imgclient *client );
int imgdec_isblank( struct imgclient *client );
void imgdec_setdebug( int lvl );
int imgdec_maskused( struct imgclient *client );

#endif /* !CLIB_VIMGDECODE_PROTOS_H */

