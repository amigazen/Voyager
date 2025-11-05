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
** Tearoff functions
** -----------------
** - Handles the Tearoff panels
**
** © 2000 by VaporWare CVS team <ibcvs@vapor.com>
** All rights reserved
**
** $Id: tearoff.c,v 1.5 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/iffparse.h>
#endif

/* private */
#include "mui_func.h"
#include "tearoff.h"
#include "dos_func.h"

APTR tearoff_dataspace;

/*
 * Saves the TearOff config
 */
void savetearoff( STRPTR filename )
{
	if( tearoff_dataspace )
	{
		struct IFFHandle *handle;
		char buffer[ 256 ];
		STRPTR p;

		strcpy( buffer, filename );
		p = FilePart( buffer );
		*p = '\0';
		strncat( buffer, "Voyager.tearoff", sizeof( buffer ) );

		if( handle = AllocIFF() )
		{
			handle->iff_Stream = Open( buffer, MODE_NEWFILE );
			InitIFFasDOS( handle );
			if( handle->iff_Stream )
			{
				OpenIFF( handle, IFFF_WRITE );
				PushChunk( handle, MAKE_ID('P','R','E','F'), MAKE_ID('F','O','R','M'), IFFSIZE_UNKNOWN );
				DoMethod( tearoff_dataspace, MUIM_Dataspace_WriteIFF, handle, MAKE_ID('P','R','E','F'), MAKE_ID('T','E','A','R') );
				PopChunk( handle );
				CloseIFF( handle );
				Close( handle->iff_Stream );
			}
			FreeIFF( handle );
		}
	}
}


/*
 * Loads the Tearoff config
 */
void loadtearoff( STRPTR filename )
{
	char buffer[ 256 ];
	STRPTR p;
	BPTR f;

	strcpy( buffer, filename );
	p = FilePart( buffer );
	*p = '\0';
	strncat( buffer, "Voyager.tearoff", sizeof( buffer ) );

	f = Open( buffer, MODE_OLDFILE );

	/*
	 * Try to open from ENV: to avoid lusers moaning about their settings
	 * as it was used in a previous version of V
	 */
	if( !f )
		f = Open( "ENV:MUI/Voyager.tearoff", MODE_OLDFILE );
	if( f )
	{
		struct IFFHandle *handle;
		handle = AllocIFF();
		if( handle )
		{
			handle->iff_Stream = f;
			InitIFFasDOS( handle );
			OpenIFF( handle, IFFF_READ );
			StopChunk( handle, MAKE_ID('P','R','E','F'), MAKE_ID('T','E','A','R') );
			ParseIFF( handle, IFFPARSE_SCAN );
			DoMethod( tearoff_dataspace, MUIM_Dataspace_ReadIFF, handle );
			CloseIFF( handle );
			FreeIFF( handle );
		}
		Close( f );
	}
}


/*
 * Reads the TearOff layout
 */
void init_tearoff( void )
{
	extern char startup_cfgfile[ 256 ];

	tearoff_dataspace = MUI_NewObject( MUIC_Dataspace, TAG_DONE );

	loadtearoff( startup_cfgfile );

}


/*
 * Bye bye Tearoff
 */
void cleanup_tearoff( void )
{
	if( tearoff_dataspace )
	{
		D( db_init, bug( "cleaning up..\n" ) );
		MUI_DisposeObject( tearoff_dataspace );
	}
}
