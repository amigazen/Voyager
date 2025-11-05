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

	$Id: listjs.c,v 1.7 2003/07/06 16:51:33 olli Exp $

	This is a quick hack to extract the
	list of supported classes and members	
	from the JS elements

*/

/* MA_JS_ClassName, "" */

#include <stdio.h>
#include <string.h>

int intag = 0;

char buffer[ 256 * 1024 ];
int nolist;

void process( void )
{
	char *p, *p2, *p3;
	char *tmp = strdup( buffer );
	
	if( nolist )
	{
		p = strstr( buffer, "NO_PUBLIC_LISTING" );
		if( p )
			return;
	}
	
	p = strstr( buffer, "MA_JS_ClassName" );
	if( !p )
		return;

	p = strchr( p, '\"' );
	if( !p )
		return;
		
	*p++ = 0;

	p2 = strchr( p, '\"' );
	if( !p2 )
		return;
		
	*p2++ = 0;

	if( !*p )
		return;
	
	printf( "%s\n", p );

	p2 = tmp;
	
	while( p = strstr( p2, "DPROP(" ) )
	{
		p += 6;
		while( isspace( *p ) )
			p++;
		p2 = strchr( p, ',' );
		if( !p2 )
			break;
		*p2++ = 0;
		while( isspace( *p2 ) )
			p2++;
		p3 = strpbrk( p2, " )" );
		if( !p3 )
			break;
		*p3++ = 0;
		
		printf( "+%s (%s)\n", p, p2 );
		
		p2 = p3;
	}	

	free( tmp );
}

int main( int argc, char **argv )
{
	FILE *f;
	int c;

	f = fopen( ".revinfo", "r" );
	fgets( buffer, sizeof( buffer ), f );
	printf( "%s\n", buffer );
	fclose( f );

	c = 1;
	if( argc > 1 )
	{
		if( !strcmp( argv[ 1 ], "-nostb" ) )
		{
			nolist = 1;
			c++;
		}
	}	

	for( ; c < argc; c++ )
	{
		FILE *f = fopen( argv[ c ], "r" );
		if( f )
		{
			memset( buffer, 0, sizeof( buffer ) );
			fread( buffer, 1, sizeof(  buffer ), f );
			process();
			fclose( f );
		}
	}

	return( 0 );
}
