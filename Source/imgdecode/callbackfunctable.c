/*
 * Callback stubs for calling direct V 68k functions
 *
 * $Id: callbackfunctable.c,v 1.6 2001/08/28 20:33:17 zapek Exp $
 */

#ifdef __MORPHOS__

#include "config.h"

#include "globals.h"
#include "imgcallback.h"

extern struct imgcallback *cbt;

APTR nets_open( STRPTR url, STRPTR referer, APTR informobj, APTR gauge, APTR txtstatus, ULONG timeout, ULONG flags )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_open;
	MyCaos.reg_a0 = ( ULONG )url;
	MyCaos.reg_a1 = ( ULONG )referer;
	MyCaos.reg_a2 = ( ULONG )informobj;
	MyCaos.reg_a3 = ( ULONG )gauge;
	MyCaos.reg_a4 = ( ULONG )txtstatus;
	MyCaos.reg_d0 = ( ULONG )timeout;
	MyCaos.reg_d1 = ( ULONG )flags;

	return( ( APTR )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}


int nets_state( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_state;
	MyCaos.reg_a0 = ( ULONG )ns;

	return( ( int )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}


void nets_close( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_close;
	MyCaos.reg_a0 = ( ULONG )ns;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}


char * nets_getdocmem( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_getdocmem;
	MyCaos.reg_a0 = ( ULONG )ns;

	return( ( char * )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}


int nets_getdocptr( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_getdocptr;
	MyCaos.reg_a0 = ( ULONG )ns;

	return( ( int )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}
 

void nets_settomem( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_settomem;
	MyCaos.reg_a0 = ( ULONG )ns;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}
 

char * nets_url( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_url;
	MyCaos.reg_a0 = ( ULONG )ns;

	return( ( char * )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}
 

char * nets_redirecturl( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_redirecturl;
	MyCaos.reg_a0 = ( ULONG )ns;

	return( ( char * )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}
 

void nets_lockdocmem( void )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_lockdocmem;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}


void nets_unlockdocmem( void )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_unlockdocmem;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}
 

char *nets_errorstring( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_errorstring;
	MyCaos.reg_a0 = ( ULONG )ns;

	return( ( char * )( *MyEmulHandle->EmulCall68k )( &MyCaos ) );
}
 

void imgcallback_decode_hasinfo( APTR obj, struct BitMap *bm, int img_x, int img_y, struct BitMap *maskbm, struct MinList *imagelist )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->imgcallback_decode_hasinfo;
	MyCaos.reg_a0 = ( ULONG )obj;
	MyCaos.reg_a1 = ( ULONG )bm;
	MyCaos.reg_d0 = ( ULONG )img_x;
	MyCaos.reg_d1 = ( ULONG )img_y;
	MyCaos.reg_a2 = ( ULONG )maskbm;
	MyCaos.reg_a3 = ( ULONG )imagelist;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}
 

void imgcallback_decode_gotscanline( APTR obj, int min_touched_y, int max_touched_y )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->imgcallback_decode_gotscanline;
	MyCaos.reg_a0 = ( ULONG )obj;
	MyCaos.reg_d0 = ( ULONG )min_touched_y;
	MyCaos.reg_d1 = ( ULONG )max_touched_y;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}


void imgcallback_decode_done( APTR obj )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->imgcallback_decode_done;
	MyCaos.reg_a0 = ( ULONG )obj;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}


void nets_release_buffer( struct nstream *ns )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->nets_release_buffer;
	MyCaos.reg_a0 = ( ULONG )ns;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}


void removeclone( struct BitMap *src )
{
	GETEMULHANDLE
	struct EmulCaos MyCaos;

	MyCaos.caos_Un.Function = cbt->removeclone;
	MyCaos.reg_a0 = ( ULONG )src;

	( *MyEmulHandle->EmulCall68k )( &MyCaos );
}

#endif /* __MORPHOS__ */
