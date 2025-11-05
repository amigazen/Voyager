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
** $Id: lo_div.c,v 1.25 2003/07/06 16:51:33 olli Exp $
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

static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	int linealign;
	int clear_left, clear_right;
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

		case MA_Layout_Div_Height:
			data->li.minheight = data->li.maxheight = data->li.defheight = tag->ti_Data;
			break;

		case MA_Layout_Div_Align:
			data->li.align = tag->ti_Data;
			break;

		case MA_Layout_LineAlign:
			data->linealign = tag->ti_Data;
			break;

		case MA_Layout_Div_ClearMargin_Left:
			data->clear_left = tag->ti_Data;
			break;

		case MA_Layout_Div_ClearMargin_Right:
			data->clear_right = tag->ti_Data;
			break;

	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Div",
		MA_JS_Object_TerseArray, TRUE,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->li.valign = valign_top;
	data->linealign = -1;

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_Layout_Info:
			*msg->opg_Storage = (ULONG)&data->li;
			return( TRUE );

		case MA_Layout_LineAlign:
			*msg->opg_Storage = (ULONG)data->linealign;
			return( TRUE );

		STOREATTR( MA_Layout_Div_ClearMargin_Left, data->clear_left );
		STOREATTR( MA_Layout_Div_ClearMargin_Right, data->clear_right );
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
	data->li.ys = data->li.defheight;
	return( (ULONG)&data->li );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

DECMMETHOD( Draw )
{
	return( 0 );
}

BEGINMTABLE
DEFCONST
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( AskMinMax )
DEFMMETHOD( Draw )
ENDMTABLE

int create_lodivclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_array_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lodivClass";
#endif

	return( TRUE );
}

void delete_lodivclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlodivclass( void )
{
	return( lcc->mcc_Class );
}
