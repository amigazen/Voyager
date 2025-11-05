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

	$Id: listtags.c,v 1.4 2003/07/06 16:51:33 olli Exp $

	This is a quick hack to extract the
	list of supported tags from layout_do()

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

int intag = 0;

char buffer[ 2048 ];

void strupr( char *str )
{
	while( *str )
		*str++ = toupper( *str );
}

int main( void )
{
	FILE *f;

	f = fopen( ".revinfo", "r" );
	fgets( buffer, sizeof( buffer ), f );
	printf( "%s\n", buffer );
	fclose( f );
	f = fopen( "layout_parse.c", "r" );

	while( fgets( buffer, sizeof( buffer ), f ) )
	{
		char *p, *p2;

		p = strstr( buffer, "case ht_" );
		if( p )
		{
			p += 8;
			p2 = strchr( p, ':' );
			if( p2 )
				*p2 = 0;
			strupr( p );
			if( !strstr( p, "HTF_NEGATE" ) )
				printf( "%s\n", p );
			continue;
		}
		p = strstr( buffer, "getargs" );
		if( !p )
			p = strstr( buffer, "getargsncv" );
		if( p )
		{
			p = strtok( p, "\"" );
			p = strtok( NULL, "\"" );
			if( p )
			{
				strupr( p );
				printf( "+%s (string/flag)\n", p );
			}
			continue;
		}
		p = strstr( buffer, "getnumargp" );
		if( p )
		{
			p = strtok( p, "\"" );
			p = strtok( NULL, "\"" );
			if( p )
			{
				strupr( p );
				printf( "+%s (numeric, positive, default %s)\n", p, strtok( NULL, " ,)" ) );
			}
			continue;
		}
		p = strstr( buffer, "getnumargmm" );
		if( p )
		{
			p = strtok( p, "\"" );
			p = strtok( NULL, "\"" );
			if( p )
			{
				strupr( p );
				printf( "+%s (numeric, default %s, from %s to %s)\n", p, strtok( NULL, " ,)" ), strtok( NULL, " ,)" ), strtok( NULL, " ,)" ) );
			}
			continue;
		}
	}

	fclose( f );

	return( 0 );
}
