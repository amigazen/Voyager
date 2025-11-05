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


#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos/doshunks.h>

#include "vpackdef.h"

struct filedef fds[ NUMFILES ];
char buffer[ 16384 ];

void main( int argc, char **argv )
{
	int c, rc;
	BPTR fh;
	FILE *outf = fopen( "V.pack", "w" ), *lh;
	int len;
	int offset;
	int sizepos;
	ULONG *ptr = (APTR)buffer;
	ULONG id;
	int size = 0;

	lh = fopen( "loader", "r" );
	if( !lh )
	{
		printf( "can't open loader\n" );
		exit( 20 );
	}

	len = fread( buffer, 1, sizeof( buffer ), lh );
	fclose( lh );

	printf( "loader size = %ld\n", len );

	c = sizeof( buffer ) / 2;
	while( c-- && *ptr != 0xac1daffe )
	{
		ptr = (APTR)(((int)ptr) + 2);
	}
	if( *ptr != 0xac1daffe )
	{
		printf( "can't find tag!\n" );
		exit( 20 );
	}
	*ptr = len + 8;
	fwrite( buffer, len, 1, outf );
	id = HUNK_DEBUG;
	fwrite( &id, 4, 1, outf );
	sizepos = ftell( outf );
	fwrite( &id, 4, 1, outf );
	
	fwrite( &fds, sizeof( fds ), 1, outf );
	offset = ftell( outf );

	printf( "offset = %ld\n", offset );

	for( c = 0; c < NUMFILES && c < argc - 1; c++ )
	{
		printf( "Merging in file %s: ", argv[ c + 1 ] );
		fh = Open( argv[ c + 1 ], MODE_OLDFILE );
		if( !fh )
		{
			printf( "can't open\n" );
			break;
		}
		Seek( fh, 0, OFFSET_END );
		len = Seek( fh, 0, OFFSET_BEGINNING );
		printf( "%ld bytes, offset %ld\n", len, offset );

		fds[ c ].fileoffset = offset;
		strcpy( fds[ c ].filename, FilePart( argv[ c + 1 ] ) );
		fds[ c ].filesize = len;

		while( ( rc = Read( fh, buffer, sizeof( buffer ) ) ) > 0 )
		{
			fwrite( buffer, 1, rc, outf );
		}

		offset += len;
		size += len;
	}

	id = HUNK_END;
	fwrite( &id, 4, 1, outf );

	fseek( outf, sizepos, SEEK_SET );
	size += sizeof( fds );
	size /= 4;
	fwrite( &size, 1, 4, outf );
	fwrite( &fds, sizeof( fds ), 1, outf );
	fclose( outf );
	printf( "Done, %ld longwords total\n", size );
}
