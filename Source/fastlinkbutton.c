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
** $Id: fastlinkbutton.c,v 1.22 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

#include "classes.h"
#include "prefs.h"
#include "voyager_cat.h"
#include "mui_func.h"
#include "malloc.h"
#include "history.h"
#include "time_func.h"
#include "methodstack.h"

struct Data {
	ULONG num;
};

char shorthelpinfotext[ 512 ];

DECMETHOD( Fastlink_Update, APTR )
{
	GETDATA;

	SetSuperAttrs( cl, obj,
		MUIA_Text_Contents, getprefs( DSI_FASTLINKS_LABELS + data->num ),
		MUIA_ShortHelp, -1,
		TAG_DONE
	);

	return( 0 );
}

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_Font, MUIV_Font_Tiny,
		MUIA_Draggable, TRUE,
		MUIA_Text_PreParse, "\033c",
		MUIA_CycleChain, 1,
		MUIA_Weight, 1,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	data->num = GetTagData( MA_Fastlink_Number, 0, msg->ops_AttrList );

	DoMethod( obj, MM_Fastlink_Update );

	return( (ULONG)obj );
}

DECMMETHOD( DragDrop )
{
	GETDATA;
	STRPTR r_url = (STRPTR)getv( msg->obj, MA_DropObject_URL );
	STRPTR r_name = (STRPTR)getv( msg->obj, MA_DropObject_Name );

	if( OCLASS( msg->obj ) == cl && r_url && r_name )
	{
		int remote_num = getv( msg->obj, MA_Fastlink_Number );

		r_url = strdup( r_url ); /* TOFIX: why ? */
		r_name = strdup( r_name ); /* TOFIX */

		setprefsstr( DSI_FASTLINKS_URLS + remote_num, getprefs( DSI_FASTLINKS_URLS + data->num ) );
		setprefsstr( DSI_FASTLINKS_LABELS + remote_num, getprefs( DSI_FASTLINKS_LABELS + data->num ) );

		setprefsstr( DSI_FASTLINKS_URLS + data->num, r_url );
		setprefsstr( DSI_FASTLINKS_LABELS + data->num, r_name );

		free( r_url );
		free( r_name );

		DoMethod( msg->obj, MM_Fastlink_Update );
	}
	else
	{
		setprefsstr( DSI_FASTLINKS_URLS + data->num, r_url );
		setprefsstr( DSI_FASTLINKS_LABELS + data->num, r_name ? r_name : (STRPTR)"[No Title]" );
	}

	DoMethod( obj, MM_Fastlink_Update );

	return( 0 );
}

DECGET
{
	GETDATA;

	if( msg->opg_AttrID == MA_DropObject_URL )
	{
		*msg->opg_Storage = (ULONG)getprefs( DSI_FASTLINKS_URLS + data->num );
		return( TRUE );
	}
	else if( msg->opg_AttrID == MA_DropObject_Name )
	{
		*msg->opg_Storage = (ULONG)getprefs( DSI_FASTLINKS_LABELS + data->num );
		return( TRUE );
	}
	else if( msg->opg_AttrID == MA_Fastlink_Number )
	{
		*msg->opg_Storage = (ULONG)data->num;
		return( TRUE );
	}
	return( DOSUPER );
}

DECMMETHOD( CreateShortHelp )
{
	GETDATA;
	char *url = getprefs( DSI_FASTLINKS_URLS + data->num );
	time_t lastvisit = checkurlhistory( url );
	time_t days = ( timev() - lastvisit ) / ( 3600 * 24 );

	if( lastvisit )
		sprintf( shorthelpinfotext, GS( SH_FASTLINK_VISITED ), url, date2string( lastvisit ), days );
	else
		sprintf( shorthelpinfotext, GS( SH_FASTLINK_NOTVISITED ), url );

	_shorthelp( obj ) = shorthelpinfotext;

	return( DOSUPER );
}

DECMMETHOD( DragQuery )
{
	if( msg->obj == obj )
		return( MUIV_DragQuery_Refuse );

	if( getv( msg->obj, MA_DropObject_URL ) )
		return( MUIV_DragQuery_Accept );

	return( MUIV_DragQuery_Refuse );
}

BEGINMTABLE
DEFNEW
DEFGET
DEFMMETHOD( DragDrop )
DEFMMETHOD( DragQuery )
DEFMMETHOD( CreateShortHelp )
DEFMETHOD( Fastlink_Update )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_fastlinkclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Text, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "FastlinkClass";
#endif

	return( TRUE );
}

void delete_fastlinkclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getfastlinkclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
