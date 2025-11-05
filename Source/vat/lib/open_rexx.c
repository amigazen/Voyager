#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <time.h>
#include <proto/vat.h>
#include <constructor.h>

struct Library *RexxSysBase;

CBMLIB_CONSTRUCTOR(openrx)
{
	if( !( RexxSysBase = VAT_OpenLibraryCode( VATOC_REXXSYS ) ) )
		return( 1 );
	return( 0 );
}

CBMLIB_DESTRUCTOR(closerx)
{
	CloseLibrary( RexxSysBase );
}
