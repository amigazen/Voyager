#define __USE_SYSBASE
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <exec/execbase.h>
#include <time.h>
#include <constructor.h>

#include <proto/vat.h>

struct Library *VATBase;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *UtilityBase;
struct Library *WorkbenchBase;
struct Library *LayersBase;

extern char * __vat_appid;
extern ULONG __vat_requirements;

char __ctype[ 260 ];

CONSTRUCTOR_P(initvat,2)
{
	int triedflush = FALSE;

	IntuitionBase = (APTR)OpenLibrary( "intuition.library", 36 );
	if( !IntuitionBase )
		return( -1 );

retry:
	VATBase = (APTR)OpenLibrary( "vapor_toolkit.library", 0 );
	if( !VATBase )
		VATBase = (APTR)OpenLibrary( "LIBS/vapor_toolkit.library", 0 );
	if( !VATBase )
		VATBase = (APTR)OpenLibrary( "/LIBS/vapor_toolkit.library", 0 );
	if( !VATBase )
	{
		struct EasyStruct eas;

		if( !triedflush )
		{
			APTR x;

			triedflush = TRUE;
			x = AllocVec( 0xfffffff, 0 );
			if( x )
				FreeVec( x );
			goto retry;
		}

		if( MYPROC->pr_WindowPtr == ( APTR ) -1 )
			return( -1 );

		eas.es_StructSize = sizeof( struct EasyStruct );
		eas.es_Flags = 0;
		eas.es_Title = __vat_appid;
		eas.es_TextFormat = "Error: %s requires (at least)\nV%ld of vapor_toolkit.library in LIBS:";
		eas.es_GadgetFormat = "Retry|Cancel";

		if( EasyRequest( MYPROC->pr_WindowPtr, &eas, 0, __vat_appid, VAT_VERSION ) )
			goto retry;

		return( 1 );
	}

	if( VAT_CheckVATVersion( VAT_VERSION ) )
		return( 1 );

	if( VAT_Initialize( __vat_appid, (APTR)&GfxBase, __ctype, __vat_requirements ) )
		return( 1 );

	return( 0 );
}

DESTRUCTOR_P(initvat,1)
{
	if( VATBase )
	{
		VAT_Cleanup( (APTR)&GfxBase );
		CloseLibrary( VATBase );
	}
	CloseLibrary( (APTR)IntuitionBase );
}
