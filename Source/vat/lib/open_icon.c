#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <time.h>
#include <proto/vat.h>
#include <constructor.h>

struct Library *IconBase;

CBMLIB_CONSTRUCTOR(openicon)
{
	if( !( IconBase = VAT_OpenLibraryCode( VATOC_ICON ) ) )
		return( 1 );
	return( 0 );
}

CBMLIB_DESTRUCTOR(closeicon)
{
	CloseLibrary( IconBase );
}
