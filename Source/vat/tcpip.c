#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/rexxsyslib.h>
#include <proto/muimaster.h>
#include <proto/asl.h>
#include <proto/datatypes.h>
#include <exec/execbase.h>
#include <libraries/mui.h>
#include <devices/timer.h>
#include <string.h>
#include <dos/dostags.h>
#include <proto/openurl.h>
#include <ctype.h>
#include <dos/notify.h>
#include <datatypes/datatypesclass.h>
#include <exec/memory.h>

#include "libraries/vat.h"
#include "globals.h"

#ifdef __MORPHOS__

//TOFIX: I'm a lazy bastard :)
int ASM SAVEDS VAT_IsOnline( void )
{
	DB( ( "called\n" ) );
	
	return( TRUE );
}

#else

#pragma libcall GenesisBase Genesis_IsOnline 78 001
#pragma libcall MiamiBase MiamiIsOnline d2 801

int Genesis_IsOnline( APTR );
int MiamiIsOnline( APTR );

int ASM SAVEDS VAT_IsOnline( void )
{
	int rc = TRUE;
	struct Library *GenesisBase;
	struct Library *MiamiBase;
	struct Library *l;

	DB( ( "called\n" ) );

	if( MiamiBase = OpenLibrary( "miami.library", 9 ) )
	{
		char buffer[ 128 ];

		if( GetVar( "VAPOR/VAT_ISONLINE_MIAMIDEVICE", buffer, sizeof( buffer ), 0 ) < 1 )
			strcpy( buffer, "mi0" );

		if( !MiamiIsOnline( buffer ) )
			rc = FALSE;

		CloseLibrary( MiamiBase );
	}
	else if( GenesisBase = OpenLibrary( "genesis.library", 3 ) )
	{
		if( !Genesis_IsOnline( NULL ) )
			rc = FALSE;
		CloseLibrary( GenesisBase );
	}

	if( l = OpenLibrary( "bsdsocket.library", 3 ) )
		CloseLibrary( l );
	else
		rc = FALSE;

	return( rc );
}
#endif
