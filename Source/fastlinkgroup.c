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
**
** $Id: fastlinkgroup.c,v 1.20 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET
#include "classes.h"
#include "prefs.h"
#include "urlparser.h"
#include "htmlclasses.h"
#include "mui_func.h"
#include "malloc.h"
#include "methodstack.h"

struct Data {
	int dummy;
};

DECMMETHOD( DragDrop )
{
	STRPTR r_url = (STRPTR)getv( msg->obj, MA_DropObject_URL );
	STRPTR r_name = (STRPTR)getv( msg->obj, MA_DropObject_Name );
	int num = getprefslong( DSI_FASTLINKS_NUM, 8 );
	int c;
	char tmp[ 128 ];

	if( r_url && !r_name || !*r_name )
	{
		char *p = strdup( r_url ); /* TOFIX */
		
		if( p )
		{
	        struct parsedurl purl;

			strcpy( tmp, "?" );
			r_name = tmp;

			uri_split( p, &purl );

			if( purl.host )
			{
				char *ptr;

				stccpy( tmp, purl.host, sizeof( tmp ) );
				if( !strnicmp( tmp, "www.", 4 ) || !strnicmp( tmp, "ftp.", 4 ) )
					strcpy( tmp, &tmp[ 4 ] );

				ptr = strchr( tmp, '.' );
				if( ptr )
					*ptr++ = 0;
			}

			free( p );
		}
		else
		{
			displaybeep();
		}
	}

	for( c = 0; c < num; c++ )
	{
		if( !strcmp( getprefs( DSI_FASTLINKS_URLS + c ), r_url ) )
		{
			displaybeep();
			return( 0 );
		}
	}

	// Append
	setprefsstr( DSI_FASTLINKS_URLS + c, r_url );
	setprefsstr( DSI_FASTLINKS_LABELS + c, r_name );
	setprefslong( DSI_FASTLINKS_NUM, c + 1 );

	// Update everything
	pushmethod( app, 2, MM_DoAllWins, MM_HTMLWin_SetupFastlinks );

	return( 0 );
}

DECMMETHOD( DragQuery )
{
	//if( msg->obj == obj )
	//  return( MUIV_DragQuery_Refuse );

	if( getv( msg->obj, MA_DropObject_URL ) )
		return( MUIV_DragQuery_Accept );

	return( MUIV_DragQuery_Refuse );
}

BEGINMTABLE
DEFMMETHOD( DragDrop )
DEFMMETHOD( DragQuery )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_fastlinkgroupclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "FastlinkGroupClass";
#endif

	return( TRUE );
}

void delete_fastlinkgroupclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getfastlinkgroupclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
