/*
 * Support functions
 * -----------------
 * - Portable support functions
 *
 * © 2001 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: sup_generic.c,v 1.4 2001/09/23 22:46:57 zapek Exp $
 *
 */

#define NO_VAT_SHORTCUTS
#include "libraries/vat.h"
#include <proto/exec.h>
#include "globals.h"

APTR ASM SAVEDS VAT_CreatePool( __reg( d0, ULONG flag ), __reg( d1, ULONG puddlesize ), __reg( d2, ULONG threshsize ) )
{
#ifdef DEBUG_MALLOC
	DB( ( "called with 0x%lx, 0x%lx, 0x%lx\n", flag, puddlesize, threshsize ) );
#endif
	
	return( CreatePool( flag, puddlesize, threshsize ) );
}


void ASM SAVEDS VAT_DeletePool( __reg( a0, APTR poolheader ) )
{
#ifdef DEBUG_MALLOC
	DB( ( "called\n" ) );
#endif

	DeletePool( poolheader );
}


APTR ASM SAVEDS VAT_AllocPooled( __reg( a0, APTR poolheader ), __reg( d0, ULONG memsize ) )
{
#ifdef DEBUG_MALLOC
	DB( ( "called\n" ) );
#endif

	return( AllocPooled( poolheader, memsize ) );
}


void ASM SAVEDS VAT_FreePooled( __reg( a0, APTR poolheader ), __reg( a1, APTR memory ), __reg( d0, ULONG memsize ) )
{
#ifdef DEBUG_MALLOC
	DB( ( "called\n" ) );
#endif

	FreePooled( poolheader, memory, memsize );
}


APTR ASM SAVEDS VAT_AllocVecPooled( __reg( a0, APTR poolheader ), __reg( d0, ULONG memsize ) )
{
#ifdef DEBUG_MALLOC
	DB( ( "called\n" ) );
#endif

	return( ( APTR )AllocVecPooled( poolheader, memsize ) );
}


void ASM SAVEDS VAT_FreeVecPooled( __reg( a0, APTR poolheader ), __reg( a1, APTR memory ) )
{
#ifdef DEBUG_MALLOC
	DB( ( "called\n" ) );
#endif
	FreeVecPooled( poolheader, memory );
}


STRPTR ASM SAVEDS VAT_StrDupPooled( __reg( a0, APTR poolheader ), __reg( a1, STRPTR s ) )
{
	STRPTR s1 = ( STRPTR )AllocVecPooled( poolheader, strlen( s ) + 1 ); /* inlined on purpose */

#ifdef DEBUG_MALLOC
	DB( ( "called\n" ) );
#endif

	if( s1 )
	{
		strcpy( s1, s );
	}
	return( s1 );
}

#ifdef __MORPHOS__
/*
 * Workaround for stupid libgmp
 */
void exit( int status )
{
	/* TOFIX: does nothing ATM, have to find out why libgmp.a can fail */
}

#endif /* __MORPHOS__ */
