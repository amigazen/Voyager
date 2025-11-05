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
	Ansi C "rev" generator
	Hack
	$Id: crev.c,v 1.4 2003/07/06 16:51:33 olli Exp $
*/

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
              
int ver = 1, rev, comp;
struct stat st;

char buffer1[ 1024 ], buffer2[ 1024 ];

static void generaterev( void )
{
	struct tm *tm = localtime( &st.st_mtime );
	FILE *f;
	
	f = fopen( "rev.h", "r" );
	if( f )
	{
		fread( buffer1, 1, sizeof( buffer1 ), f );
		fclose( f );
	}
	sprintf( buffer2, "#define VERSIONSTRING \"%d.%d.%d\"\n#define VERSION %d\n#define REVISION %d\n#define COMPILEREV %d\n#define REVDATE \"(%02d.%02d.%04d)\"\n#define VERTAG \"%d.%d.%d (%d.%d.%d)\"\n#define LVERTAG \"%d.%d.%d (%d.%d.%d)\"\n#define VERHEXID 0x%x\n",
		ver, rev, comp,
		ver,
		rev,
		comp,
		tm->tm_mday,
		tm->tm_mon + 1,
		tm->tm_year + 1900,
		ver, rev, comp,	
		tm->tm_mday,
		tm->tm_mon + 1,
		tm->tm_year,
		ver, rev, comp,	
		tm->tm_mday,
		tm->tm_mon + 1,
		tm->tm_year + 1900,
		( ver << 24 ) | ( rev << 16 ) | comp
	);
	if( strcmp( buffer1, buffer2 ) )
	{
		f = fopen( "rev.h", "w" );
		if( f )
		{
			fputs( buffer2, f );
			fclose( f );
		}
	}
}

int main( int argc, char **argv )
{
	FILE *f;

	stat( ".revinfo", &st );
	f = fopen( ".revinfo", "r" );
	if( !f )
		return( 1 );
	fscanf( f, "%d.%d.%d", &ver, &rev, &comp );
	fclose( f );

	generaterev();
	return( 0 );
}
