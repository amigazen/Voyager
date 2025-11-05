#include <proto/dos.h>
#include <proto/exec.h>

#include <time.h>
#include "vat.h"

struct Library *VATBase;

void main( int argc, char **argv )
{
	VATBase = OpenLibrary( "vapor_toolkit.library", 0 );
	if( VATBase )
	{
		int rc = VAT_IsAmigaGuideFile( argv[ 1 ] );
		Printf( "rc = %ld\n", rc );
	}
	CloseLibrary( VATBase );
}
