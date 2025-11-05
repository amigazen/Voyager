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
 * Group subclass with settable default size
 *
 * $Id: sizegroup.c,v 1.5 2003/07/06 16:51:34 olli Exp $
 */

#include "voyager.h"

/* private */
#include "classes.h"
#include "mui_func.h"


struct Data {
	ULONG size_x;
	ULONG size_y;
};


DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->size_x = GetTagData( MA_SizeGroup_SizeX, 0, msg->ops_AttrList );
	data->size_y = GetTagData( MA_SizeGroup_SizeY, 0, msg->ops_AttrList );

	return( ( ULONG )obj );
}


DECMMETHOD( AskMinMax )
{
	struct MUI_MinMax *mmx;
	GETDATA;

	DOSUPER;

	mmx = msg->MinMaxInfo;

	if( data->size_x )
	{
		mmx->DefWidth = data->size_x;
	}
	if( data->size_y )
	{
		mmx->DefHeight = data->size_y;
	}
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMMETHOD( AskMinMax )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_sizegroupclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "SizeGroupClass";
#endif

	return( TRUE );
}

void delete_sizegroupclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getsizegroupclass( void )
{
	return( mcc->mcc_Class );
}
