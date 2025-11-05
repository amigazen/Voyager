/*
 * Gateway functions
 *
 * $Id: libfunctions.c,v 1.7 2003/04/25 19:13:59 zapek Exp $
*/

#if defined( __MORPHOS__ )

#include "debug.h"

#include "lib.h"

#include <proto/vimgdecode.h>

void LIB_imgdec_markforreload( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;

	DB( ( "here\n" ) );

	imgdec_markforreload( client );
}


void LIB_imgdec_abortload( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	imgdec_abortload( client );
}


int LIB_imgdec_isdone( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	return( imgdec_isdone( client ) );
}

 
void LIB_imgdec_flushimages( void )
{
	DB( ( "here\n" ) );
	
	imgdec_flushimages();
}

 
struct BitMap * LIB_imgdec_getmaskbm( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	return( ( struct BitMap * )imgdec_getmaskbm( client ) );
}

 
struct MinList * LIB_imgdec_getimagelist( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	return( ( struct MinList * )imgdec_getimagelist( client ) );
}

 
void LIB_imgdec_setclientobject( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	APTR object = ( APTR )REG_A1;
	
	DB( ( "here\n" ) );

	imgdec_setclientobject( client, object );
}

 
void LIB_imgdec_tick( void )
{
	DB( ( "here\n" ) );
	
	imgdec_tick();
}

 
int LIB_imgdec_getinfo( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	struct BitMap **bm = ( struct BitMap ** )REG_A1;
	int *xsize = ( int * )REG_A2;
	int *ysize = ( int * )REG_A3;
	
	DB( ( "here\n" ) );

	return( imgdec_getinfo( client, bm, xsize, ysize ) );
}

 
void LIB_imgdec_close( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	imgdec_close( client );
}

 
int LIB_imgdec_dowehave( void )
{
	char *url = ( char * )REG_A0;
	
	DB( ( "here\n" ) );

	return( imgdec_dowehave( url ) );
}

 
APTR LIB_imgdec_open( void )
{
	char *url = ( char * )REG_A0;
	APTR clientobject = ( APTR )REG_A1;
	char *referer = ( char * )REG_A2;
	int reloadflag = ( int )REG_D0;
	
	DB( ( "here\n" ) );

	return( ( APTR )imgdec_open( url, clientobject, referer, reloadflag ) );
}

 
int LIB_imgdec_setdestscreen( void )
{
	struct Screen *scr = ( struct Screen * )REG_A0;
	int bgpen = ( int )REG_D0;
	int framepen = ( int )REG_D1;
	int shadowpen = ( int )REG_D2;
	int shinepen = ( int )REG_D3;
	
	DB( ( "here\n" ) );

	return( imgdec_setdestscreen( scr, bgpen, framepen, shadowpen, shinepen ) );
}

 
int	LIB_imgdec_libinit( void )
{
	struct imgcallback *cbtptr = ( struct imgcallback * )REG_A0;
	
	DB( ( "here, image callback table is at 0x%lx\n", cbtptr ) );

	return( imgdec_libinit( cbtptr ) );
}

 
void LIB_imgdec_libexit( void )
{
	DB( ( "here\n" ) );
	
	imgdec_libexit();
}

 
void LIB_imgdec_setprefs( void )
{
	long img_jpeg_dct = ( long )REG_D0;
	long img_jpeg_dither = ( long )REG_D1;
	long img_jpeg_quant = ( long )REG_D2;
	long img_lamedecode = ( long )REG_D3;
	long img_progressive_jpeg = ( long )REG_D4;
	long img_gif_dither = ( long )REG_D5;
	long img_png_dither = ( long )REG_D6;
	
	DB( ( "here\n" ) );

	imgdec_setprefs( img_jpeg_dct, img_jpeg_dither, img_jpeg_quant, img_lamedecode, img_progressive_jpeg, img_gif_dither, img_png_dither );
}

 
int LIB_imgdec_getrepeatcnt( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	return( imgdec_getrepeatcnt( client ) );
}

 
void LIB_imgdec_getinfostring( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	STRPTR buffer = ( STRPTR )REG_A1;
	
	DB( ( "here\n" ) );

	imgdec_getinfostring( client, buffer );
}

 
char * LIB_imgdec_errormsg( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;
	
	DB( ( "here\n" ) );

	return( ( char * )imgdec_errormsg( client ) );
}


void LIB_imgdec_setdebug( void )
{
	int lvl = ( int )REG_D0;

	imgdec_setdebug( lvl );
}


int LIB_imgdec_isblank( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;

	return( imgdec_isblank( client ) );
}


int LIB_imgdec_maskused( void )
{
	struct imgclient *client = ( struct imgclient * )REG_A0;

	return( imgdec_maskused( client ) );
}

#endif /* __MORPHOS__ */
