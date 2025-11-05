/*
 * Library function table
 * ----------------------
 *
 * $Id: libfunctable.c,v 1.6 2001/08/28 20:33:18 zapek Exp $
*/

#if defined( __MORPHOS__ )

#include "lib.h"
#include <libraries/vimgdecode.h>

void LIB_Open( void );
void LIB_Close( void );
void LIB_Expunge( void );
void LIB_Reserved( void );
void LIB_imgdec_markforreload( void );
void LIB_imgdec_abortload( void );
int LIB_imgdec_isdone( void );
void LIB_imgdec_flushimages( void );
struct BitMap * LIB_imgdec_getmaskbm( void );
struct MinList * LIB_imgdec_getimagelist( void );
void LIB_imgdec_setclientobject( void );
void LIB_imgdec_tick( void );
int LIB_imgdec_getinfo( void );
void LIB_imgdec_close( void );
int LIB_imgdec_dowehave( void );
APTR LIB_imgdec_open( void );
int LIB_imgdec_setdestscreen( void );
int	LIB_imgdec_libinit( void );
void LIB_imgdec_libexit( void );
void LIB_imgdec_setprefs( void );
int LIB_imgdec_getrepeatcnt( void );
void LIB_imgdec_getinfostring( void );
char * LIB_imgdec_errormsg( void );
void LIB_imgdec_setdebug( void );
int LIB_imgdec_isblank( void );
int LIB_imgdec_maskused( void );


ULONG LibFuncTable[] =
{
	FUNCARRAY_32BIT_NATIVE,
	( ULONG )&LIB_Open,
	( ULONG )&LIB_Close,
	( ULONG )&LIB_Expunge,
	( ULONG )&LIB_Reserved,
	( ULONG )&LIB_imgdec_markforreload,
	( ULONG )&LIB_imgdec_abortload,
	( ULONG )&LIB_imgdec_isdone,
	( ULONG )&LIB_imgdec_flushimages,
	( ULONG )&LIB_imgdec_getmaskbm,
	( ULONG )&LIB_imgdec_getimagelist,
	( ULONG )&LIB_imgdec_setclientobject,
	( ULONG )&LIB_imgdec_tick,
	( ULONG )&LIB_imgdec_getinfo,
	( ULONG )&LIB_imgdec_close,
	( ULONG )&LIB_imgdec_dowehave,
	( ULONG )&LIB_imgdec_open,
	( ULONG )&LIB_imgdec_setdestscreen,
	( ULONG )&LIB_imgdec_libinit,
	( ULONG )&LIB_imgdec_libexit,
	( ULONG )&LIB_imgdec_setprefs,
	( ULONG )&LIB_imgdec_getrepeatcnt,
	( ULONG )&LIB_imgdec_getinfostring,
	( ULONG )&LIB_imgdec_errormsg,
	( ULONG )&LIB_imgdec_setdebug,
	( ULONG )&LIB_imgdec_isblank,
	( ULONG )&LIB_imgdec_maskused,
	0xffffffff
};

#endif /* __MORPHOS__ */
