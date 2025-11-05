#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <time.h>
#include <proto/vat.h>
#include <constructor.h>

struct Library *DiskFontBase;

CBMLIB_CONSTRUCTOR(opendiskfont)
{
	if( !( DiskFontBase = VAT_OpenLibraryCode( VATOC_DISKFONT ) ) )
		return( 1 );
	return( 0 );
}

CBMLIB_DESTRUCTOR(closediskfont)
{
	CloseLibrary( DiskFontBase );
}
