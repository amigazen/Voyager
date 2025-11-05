#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <time.h>
#include <proto/vat.h>
#include <constructor.h>

struct Library *DataTypesBase;

CBMLIB_CONSTRUCTOR(opendt)
{
	if( !( DataTypesBase = VAT_OpenLibraryCode( VATOC_DATATYPES ) ) )
		return( 1 );
	return( 0 );
}

CBMLIB_DESTRUCTOR(closedt)
{
	CloseLibrary( DataTypesBase );
}
