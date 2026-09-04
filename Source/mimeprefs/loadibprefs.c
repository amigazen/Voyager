#include "mimeprefs.h"

int importibprefs( char *filename )
{
	BPTR f = Open( filename, MODE_OLDFILE );
	int len;
	APTR buffer;
	ULONG *lp;
	UWORD *wp;
	UBYTE *cp;
	int c;
	ULONG misc = MAKE_ID('M','I','S','C');
	int rc = -1;

	if( !f )
		return( -1 );

	Seek( f, 0, OFFSET_END );
	len = Seek( f, 0, OFFSET_BEGINNING );

	buffer = AllocVec( len, 0 );
	Read( f, buffer, len );
	Close( f );

	cp = buffer;

	for( c = 0; c < len; c += 2 )
	{
		if( !memcmp( &cp[ c ], &misc, 4 ) )
			break;
	}

	if( c < len )
	{
		cp += 4;
		c += 4;

		while( c < len )
		{
			lp = (ULONG*)cp;
			Printf( "ID %08lx len %ld: ",
				lp[ 0 ], lp[ 1 ]
			);
			cp += 8;
			WriteChars( cp, lp[ 1 ] );
			cp += lp[ 1 ] + ( lp[ 1 ] % 2 );
			PutStr( "\n" );
		}	
	}
	FreeVec( buffer );

	return( rc );
}
