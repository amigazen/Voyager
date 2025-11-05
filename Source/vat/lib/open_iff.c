#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <time.h>
#include <proto/vat.h>
#include <constructor.h>

struct Library *IFFParseBase;

CBMLIB_CONSTRUCTOR(openiff)
{
	if( !( IFFParseBase = VAT_OpenLibraryCode( VATOC_IFFPARSE ) ) )
		return( 1 );
	return( 0 );
}

CBMLIB_DESTRUCTOR(closeiff)
{
	CloseLibrary( IFFParseBase );
}
