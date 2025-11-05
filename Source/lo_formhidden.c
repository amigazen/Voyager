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
** $Id: lo_formhidden.c,v 1.22 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"
#include "classes.h"
#include <proto/vimgdecode.h>
#include "prefs.h"
#include "voyager_cat.h"
#include "js.h"
#include "urlparser.h"
#include "htmlclasses.h"
#include "layout.h"
#include "fontcache.h"
#include "mui_func.h"
#include "malloc.h"


static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	APTR formobject;
	char *name;
	char *value;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			break;

		case MA_Layout_FormElement_Name:
			if( tag->ti_Data )
				data->name = strdup( (char*)tag->ti_Data ); /* TOFIX */
			break;

		case MA_Layout_FormElement_Value:
			if( tag->ti_Data )
			{
				data->value = strdup( (char*)tag->ti_Data ); /* TOFIX */
			}
			break;

		case MA_Layout_FormElement_Form:
			data->formobject = (APTR)tag->ti_Data;
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "hidden",
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->name );
	free( data->value );

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_Layout_Info, &data->li );
		case MA_JS_Name:
		STOREATTR( MA_Layout_FormElement_Name, data->name );
		STOREATTR( MA_Layout_FormElement_Value, data->value );
		STOREATTR( MA_Layout_FormElement_Form, data->formobject );
	}

	return( DOSUPER );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( DOSUPER );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_FormElement_ReportValue )
{
	GETDATA;

	if( msg->whichform != data->formobject )
		return( 0 );

	DoMethod( data->formobject, MM_Layout_Form_AttachValue,
		data->name, data->value,
		-1,
		NULL
	);

	return( 0 );
}

BEGINPTABLE
DPROP( name,		string )
DPROP( value,		string )
DPROP( type,		string )
DPROP( form,		obj )
ENDPTABLE


DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DECSMETHOD( JS_GetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{

		DOM_PROP( name )
			storestrprop( msg, data->name );
			return( TRUE );

		DOM_PROP( value )
			storestrprop( msg, data->value );
			return( TRUE );

		DOM_PROP( form )
			storeobjprop( msg, data->formobject );
			return( TRUE );
	}

	return( DOSUPER );
}



DECSMETHOD( JS_SetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		DOM_PROP( value )
			free( data->value );
			if( msg->dataptr )
			{
				data->value = strdup( msg->dataptr ); /* TOFIX */
			}
			else
			{
				data->value = strdup( "" ); /* TOFIX */
			}
            return( TRUE );
	}

	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_FormElement_ReportValue )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

int create_loformhiddenclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loformhiddenClass";
#endif

	return( TRUE );
}

void delete_loformhiddenclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloformhiddenclass( void )
{
	return( lcc->mcc_Class );
}
