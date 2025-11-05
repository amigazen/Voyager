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
** $Id: lo_margin.c,v 1.8 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"
#include "classes.h"
#include "prefs.h"
#include "voyager_cat.h"
#include "htmlclasses.h"
#include "layout.h"
#include "download.h"
#include "malloc.h"
#include "mui_func.h"


static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	int addmargin_left;
	int addmargin_right;
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

		case MA_Layout_AddMargin_Left:
			data->addmargin_left = tag->ti_Data;
			break;

		case MA_Layout_AddMargin_Right:
			data->addmargin_right = tag->ti_Data;
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
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
	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_Layout_Info:
			*msg->opg_Storage = (ULONG)&data->li;
			return( TRUE );

		case MA_Layout_AddMargin_Left:
			*msg->opg_Storage = data->addmargin_left;
			return( TRUE );

		case MA_Layout_AddMargin_Right:
			*msg->opg_Storage = data->addmargin_right;
			return( TRUE );
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

DECMMETHOD( Draw )
{
	return( 0 );
}

BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( Draw )
ENDMTABLE

int create_lomarginclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lomarginClass";
#endif

	return( TRUE );
}

void delete_lomarginclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlomarginclass( void )
{
	return( lcc->mcc_Class );
}
