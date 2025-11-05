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


/*
 * Portable vat opening
 *
 * $Id: vatstart.c,v 1.7 2003/07/06 16:51:34 olli Exp $
 */

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/execbase.h>
#include <proto/exec.h>
#include <intuition/intuition.h>
#endif

/* private */
#include "copyright.h"

struct Library *VATBase;
struct Library *DataTypesBase;
struct Library *DiskfontBase;
struct Library *IconBase;
struct Library *IFFParseBase;
struct Library *RexxSysBase;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *UtilityBase;
struct Library *WorkbenchBase;
struct Library *LayersBase;


int vat_open_libs( void )
{
	if( !( DataTypesBase = VAT_OpenLibraryCode( VATOC_DATATYPES ) ) )
		return( FALSE );

	if( !( DiskfontBase = VAT_OpenLibraryCode( VATOC_DISKFONT ) ) )
		return( FALSE );

	if( !( IconBase = VAT_OpenLibraryCode( VATOC_ICON ) ) )
		return( FALSE );

	if( !( IFFParseBase = VAT_OpenLibraryCode( VATOC_IFFPARSE ) ) )
		return( FALSE );

	if( !( RexxSysBase = VAT_OpenLibraryCode( VATOC_REXXSYS ) ) )
		return( FALSE );

	return( TRUE );
}

void vat_close_libs( void )
{
	CloseLibrary( DataTypesBase );
	CloseLibrary( DiskfontBase );
	CloseLibrary( IconBase );
	CloseLibrary( IFFParseBase );
	CloseLibrary( RexxSysBase );
}

#define VAT_APPID "Voyager " LVERTAG

int init_vat( void )
{
	ULONG libvec[ 4 ];
	int triedflush = FALSE;
	
	IntuitionBase = (APTR)OpenLibrary( "intuition.library", 36 );
	if( !IntuitionBase )
		return( FALSE );

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
			return( FALSE );

		eas.es_StructSize = sizeof( struct EasyStruct );
		eas.es_Flags = 0;
		eas.es_Title = VAT_APPID;
		eas.es_TextFormat = "Error: %s requires (at least)\nV%ld of vapor_toolkit.library in LIBS:";
		eas.es_GadgetFormat = "Retry|Cancel";

#ifndef __MORPHOS__
		if( EasyRequest( MYPROC->pr_WindowPtr, &eas, 0, VAT_APPID, VAT_VERSION ) )
			goto retry;
#else
//TOFIX:
#endif

		return( FALSE );
	}

	if( !vat_open_libs() )
		return( FALSE );

	if( VAT_CheckVATVersion( VAT_VERSION ) )
		return( FALSE );

	/*
	 * See Olli's original vatlib.c autoopening code for a good
	 * laugh. Actually, I still wonder how you did find out that
	 * trick :)
	 */

	if( VAT_Initialize( VAT_APPID, ( APTR )&libvec[ 0 ], NULL, VATIR_OS3 ) )
		return( FALSE );

	GfxBase = ( struct GfxBase * )libvec[ 0 ];
	UtilityBase = ( struct UtilityBase * )libvec[ 1 ];
	WorkbenchBase = ( struct Library * )libvec[ 2 ];
	LayersBase = ( struct Library * )libvec[ 3 ];

	return( TRUE );
}

void close_vat( void )
{
	if( VATBase )
	{
		ULONG libvec[ 4 ];
		libvec[ 0 ] = ( ULONG )GfxBase;
		libvec[ 1 ] = ( ULONG )UtilityBase;
		libvec[ 2 ] = ( ULONG )WorkbenchBase;
		libvec[ 3 ] = ( ULONG )LayersBase;
		VAT_Cleanup( ( APTR )libvec );
		CloseLibrary( VATBase );
	}

	vat_close_libs();

	CloseLibrary( (APTR)IntuitionBase );
}
