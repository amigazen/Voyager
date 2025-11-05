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
** $Id: urlstring.c,v 1.27 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "classes.h"
#include "voyager_cat.h"
#include "urlparser.h"
#include "history.h"
#include "mui_func.h"
#include "textinput.h"

#define NOTIFY_KEYPRESS

struct Data {
	APTR texto;
	char *title;
	LONG lastwasdel;
};

DECCONST
{
	obj = DoSuperNew( cl, obj,
		StringFrame,
		MUIA_String_MaxLen, MAXURLSIZE,
		MUIA_Draggable, TRUE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );
#ifndef NOTIFY_KEYPRESS
	DoMethod( obj, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
		( ULONG )obj, 2, MM_URLString_Change, MUIV_TriggerValue
	);
#endif

	return( (ULONG)obj );
}

DECSET
{
	GETDATA;

	data->texto = (APTR)GetTagData( MA_URLString_Text, (ULONG)data->texto, msg->ops_AttrList );
	data->title = (APTR)GetTagData( MA_DropObject_Name, (ULONG)data->title, msg->ops_AttrList );
	return( DOSUPER );
}

DECGET
{
	if( msg->opg_AttrID == MA_DropObject_URL )
	{
		return( DoSuperMethod( cl, obj, OM_GET, MUIA_String_Contents, ( ULONG )msg->opg_Storage ) );
	}
	else if( msg->opg_AttrID == MA_DropObject_Name )
	{
		struct Data *data = INST_DATA( cl, obj );
		*msg->opg_Storage = (ULONG)data->title;
		return( TRUE );
	}
	return( DOSUPER );
}

DECMMETHOD( DragQuery )
{
	char *url = NULL;

	if( msg->obj == obj )
		return( MUIV_DragQuery_Refuse );

	get( msg->obj, MA_DropObject_URL, &url );

	if( url )
		return( MUIV_DragQuery_Accept );
	else
		return( MUIV_DragQuery_Refuse );
}

DECMMETHOD( DragDrop )
{
	char *url = NULL;

	get( msg->obj, MA_DropObject_URL, &url );
	set( obj, MUIA_String_Contents, url );
	set( obj, MUIA_String_Acknowledge, url );

	return( 0 );
}

DECMMETHOD( GoInactive )
{
	struct Data *data = INST_DATA( cl, obj );

	if( data->texto )
	{
		set( data->texto, MA_Smartlabel_Text, GS( WIN_URL ) );
	}

	return( DOSUPER );
}

DECMMETHOD( GoActive )
{
	struct Data *data = INST_DATA( cl, obj );

	if( data->texto )
	{
		set( data->texto, MA_Smartlabel_Text, GS( WIN_URL_GOTO ) );
	}

	return( DOSUPER );
}

#ifdef NOTIFY_KEYPRESS
DECMMETHOD( Textinput_HandleChar )
#else
DECSMETHOD( URLString_Change )
#endif
{
	STRPTR newcontents;
#if USE_AUTOCOMPLETE
	char *match;
#endif
	ULONG rv=0;

#ifdef NOTIFY_KEYPRESS
	if( msg->ch == 13 )
		return( DOSUPER );
	rv = DOSUPER;

	get( obj, MUIA_String_Contents, &newcontents );
#else
	GETDATA;
	if( data->lastwasdel )
		return( 0 );

	newcontents = msg->newcontents;
#endif

	if( strchr( newcontents, '.' ) )
	{
		char buffer[ MAXURLSIZE ];
		newcontents = stpblk( newcontents );
		stccpy( buffer, newcontents, sizeof( buffer ) - 12 );
		
		if( !url_hasscheme( buffer ) )
		{
			if( !strncmp( buffer, "ftp.", 4 ) )
			{
				strins( buffer, "ftp://" );
			}
			else
			{
				strins( buffer, "http://" );
			}
			SetSuperAttrs( cl, obj,
				MUIA_NoNotify, TRUE,
				MUIA_Textinput_Contents, ( ULONG )buffer,
				TAG_DONE
			);
			return( 0 );
		}

#if USE_AUTOCOMPLETE
		match = findurlhismatch( newcontents );
		if( match )
		{
			int ol = strlen( newcontents );
			SetSuperAttrs( cl, obj,
				MUIA_NoNotify, TRUE,
				MUIA_Textinput_Contents, ( ULONG )match,
				MUIA_Textinput_CursorPos, ol,
				MUIA_Textinput_MarkEnd, ol,
				MUIA_Textinput_MarkStart, strlen( match ),
				TAG_DONE
			);
		}
#endif /* USE_AUTOCOMPLETE */
	}
	return( rv );
}

#ifndef NOTIFY_KEYPRESS
DECMMETHOD( Textinput_DoDel )
{
	GETDATA;
	ULONG rv;
	data->lastwasdel=TRUE;
	rv=DOSUPER;
	data->lastwasdel=FALSE;
	return( rv );
}
#endif

DECMMETHOD( Textinput_DoBS )
{
	GETDATA;
	ULONG rv;
	data->lastwasdel=TRUE;
	if( (int)getv( obj, MUIA_Textinput_MarkStart ) > 0 )
		DOSUPER;
	rv=DOSUPER;
	data->lastwasdel=FALSE;
	return( rv );
}

BEGINMTABLE
DEFNEW
DEFGET
DEFSET
DEFMMETHOD( DragQuery )
DEFMMETHOD( DragDrop )
DEFMMETHOD( GoActive )
DEFMMETHOD( GoInactive )
#ifdef NOTIFY_KEYPRESS
DEFMMETHOD( Textinput_HandleChar )
#else
case MUIM_Textinput_DoDelEOL:
case MUIM_Textinput_DoBSSOL:
DEFMMETHOD( Textinput_DoDel )
DEFSMETHOD( URLString_Change )
#endif
DEFMMETHOD( Textinput_DoBS )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_urlstringclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Textinput, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "URLStringClass";
#endif

	return( TRUE );
}

void delete_urlstringclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR geturlstringclass( void )
{
	return( mcc->mcc_Class );
}
