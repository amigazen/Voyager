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
** $Id: lo_form_option.c,v 1.9 2003/07/06 16:51:33 olli Exp $
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
	APTR selectobject;
	int defaultselected;
	int selected;
	int index;
	char *text;
	char *value;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_FormOption_Text:
			free( data->text );
			if( tag->ti_Data )
			{
				data->text = strdup( (char*)tag->ti_Data ); /* TOFIX */
			}
			else
			{
				data->text = strdup( "" ); /* TOFIX */
			}
            break;

		case MA_Layout_FormOption_Value:
			free( data->value );
			if( tag->ti_Data )
			{
				data->value = strdup( (char*)tag->ti_Data ); /* TOFIX */
			}
			else
			{
				data->text = strdup( "" ); /* TOFIX */
			}
            break;

		case MA_Layout_FormOption_Index:
			data->index = tag->ti_Data;
			break;

		case MA_Layout_FormOption_Selected:
			data->selected = tag->ti_Data;
			break;

		case MA_Layout_FormOption_DefaultSelected:
			data->defaultselected = tag->ti_Data;
			break;

		case MA_Layout_FormOption_SelectObject:
			data->selectobject = (APTR)tag->ti_Data;
			break;
		
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "option",
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

	free( data->text );
	free( data->value );

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_JS_Name:
			{
				static char name[ 16 ];

				sprintf( name, "%d", data->index );
				*msg->opg_Storage = (ULONG)name;
				return( TRUE );
			}
	}

	return( DOSUPER );
}

DECSET
{
	GETDATA;

	doset( data, obj, msg->ops_AttrList );

	return( DOSUPER );
}

BEGINPTABLE
DPROP( text,			string )
DPROP( value,			string )
DPROP( index,			real )
DPROP( selected,		bool )
DPROP( defaultSelected,	bool )
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

		DOM_PROP( text )
			storestrprop( msg, data->text );
			return( TRUE );

		DOM_PROP( value )
			storestrprop( msg, data->value );
			return( TRUE );

		DOM_PROP( selected )
			storeintprop( msg, data->selected );
			return( TRUE );

		DOM_PROP( defaultSelected )
			storeintprop( msg, data->defaultselected );
			return( TRUE );

		DOM_PROP( index )
			storeintprop( msg, data->index );
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

		DOM_PROP( text )
			free( data->text );
			if( msg->dataptr )
			{
				data->text = strdup( msg->dataptr ); /* TOFIX */
			}
			else
			{
				data->text = strdup( "" ); /* TOFIX */
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
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

int create_loform_optionclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loform_optionClass";
#endif

	return( TRUE );
}

void delete_loform_optionclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloform_optionclass( void )
{
	return( lcc->mcc_Class );
}
