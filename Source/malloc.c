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
 * Memory allocation functions
 * ---------------------------
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: malloc.c,v 1.20 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#endif

/* private */
#include "malloc.h"

#if USE_MALLOC
static APTR mallocpool;
static struct SignalSemaphore msem;
#else
#ifdef __MORPHOS__
unsigned long _MSTEP = 4096;
#endif
#endif


#if USE_MALLOC

int init_malloc( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	InitSemaphore( &msem );
	mallocpool = CreatePool( 0, 4096, 2048 );
	return( mallocpool ? TRUE : FALSE );
}

void cleanup_malloc( void )
{
	D( db_init, bug( "cleaning up..\n" ) );

	if( mallocpool )
	{
		DeletePool( mallocpool );
	}
}


void *malloc( size_t size )
{
	ULONG *memblock;

	ObtainSemaphore( &msem );
	memblock = AllocPooled( mallocpool, size + 4 );
	ReleaseSemaphore( &msem );

	if( memblock )
	{
		*memblock++ = size + 4;
	}
	return( memblock );
}

void free( void *memb )
{
	ULONG *memblock = (ULONG*)memb;
	if( memblock-- )
	{
		ObtainSemaphore( &msem );
		FreePooled( mallocpool, memblock, *memblock );
		ReleaseSemaphore( &msem );
	}
}

void *realloc( void *old, size_t nsize )
{
	size_t osize;
	void *new;

	if( !old )
	{
		return( malloc( nsize ) );
	}

	osize = *( (size_t *)old - 1 );

	if( ( new = malloc( nsize ) ) )
	{
		memcpy( new, old, osize > nsize ? nsize : osize );
		free( old );
	}

	return( new );
}

char *strdup( const char *string )
{
	char *news;

	if( !string )
		return( NULL );
	news = malloc( strlen( string ) + 1 );
	if( news )
		strcpy( news, string );
	return( news );
}
#endif /* USE_MALLOC */

void *mallocz( size_t size )
{
	void *p;

	p = malloc( size );
	if( p )
		memset( p, 0, size );
	return( p );
}

void strupd( char **dst, const char *src )
{
	char *d;

	if( !dst || !src )
		return;
	d = strdup( src );
	if( d )
		*dst = d;
}

STRPTR StrDupPooled( APTR pool, STRPTR string )
{
	STRPTR s;

	if( !string )
		return( NULL );
	s = AllocPooled( pool, strlen( string ) + 1 );
	if( s )
		strcpy( s, string );
	return( s );
}

