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
** $Id: js_event.c,v 1.16 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "malloc.h"
#include "mui_func.h"


struct Data {
	char type[ 128 ];
	APTR data;
	int height, width;
	int layerx, layery;
	int modifiers;
	int pagex, pagey;
	int screenx, screeny;
	APTR target;
	APTR which;
};

BEGINPTABLE
DPROP( type, string )
DPROP( data, obj )
DPROP( height, real )
DPROP( layerx, real )
DPROP( layery, real )
DPROP( modifiers, real )
DPROP( pagex, real )
DPROP( pagey, real )
DPROP( screenx, real )
DPROP( screeny, real )
DPROP( target, obj )
DPROP( which, real )
DPROP( width, real )
DPROP( x, real )
DPROP( y, real )
ENDPTABLE

static void doset( APTR obj, struct Data *data, struct TagItem *taglist )
{
	struct TagItem *tag;

	while( ( tag = NextTagItem( &taglist ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_Event_Type:
			stccpy( data->type, (char*)tag->ti_Data, sizeof( data->type ) );
			break;
	 	
 		case MA_JS_Event_Data:
 			data->data = (APTR)tag->ti_Data;
 			break;
 		
 		case MA_JS_Event_Height:
 			data->height = tag->ti_Data;
 			break;
 			
 		case MA_JS_Event_LayerX:
 			data->layerx = tag->ti_Data;
 			break;
 			
 		case MA_JS_Event_LayerY:
 			data->layery = tag->ti_Data;
 			break;
 			
 		case MA_JS_Event_Modifiers:
 			data->modifiers = tag->ti_Data;
 			break;

 		case MA_JS_Event_PageX:
 			data->pagex = tag->ti_Data;
 			break;

 		case MA_JS_Event_PageY:
 			data->pagey = tag->ti_Data;
 			break;

 		case MA_JS_Event_ScreenX:
 			data->screenx = tag->ti_Data;
 			break;

 		case MA_JS_Event_ScreenY:
 			data->screeny = tag->ti_Data;
 			break;

 		case MA_JS_Event_Target:
 			data->target = (APTR)tag->ti_Data;
 			break;

 		case MA_JS_Event_Which:
 			data->which = (APTR)tag->ti_Data;
 			break;

 		case MA_JS_Event_Width:
 			data->width = tag->ti_Data;
 			break;
	}
}

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "event",
		TAG_MORE, msg->ops_AttrList
	);
	if( obj )
	{
		struct Data *data = INST_DATA( cl, obj );
		doset( obj, data, msg->ops_AttrList );
	}
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetProperty )
{
	/*switch( findpropid( ptable, msg->propname ) )
	{
	}*/

	return( DOSUPER );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_event( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );
#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_EventClass";
#endif

	return( TRUE );
}

void delete_js_event( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_event( void )
{
	return( mcc->mcc_Class );
}

struct MUI_CustomClass * getjs_event_class( void )
{
	return( mcc );
}
