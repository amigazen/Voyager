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
** $Id: ddstring.c,v 1.14 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "classes.h"
#include "mui_func.h"
#include "textinput.h"

struct Data {
	ULONG ddid;
};

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		StringFrame,
		MUIA_Draggable, TRUE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	data->ddid = GetTagData( MA_DDID, 0, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECGET
{
	if( msg->opg_AttrID == MA_DDID )
	{
		struct Data *data = INST_DATA( cl, obj );
		*msg->opg_Storage = data->ddid;
		return( TRUE );
	}
	return( DOSUPER );
}

DECMMETHOD( DragQuery )
{
	ULONG ddid = 0;
	struct Data *data = INST_DATA( cl, obj );

	if( msg->obj == obj )
		return( MUIV_DragQuery_Refuse );

	get( msg->obj, MA_DDID, &ddid );

	if( ddid == data->ddid )
		return( MUIV_DragQuery_Accept );
	else
		return( MUIV_DragQuery_Refuse );
}

DECMMETHOD( DragDrop )
{
	set( obj, MUIA_String_Contents, getstrp( msg->obj ) );
	return( 0 );
}


BEGINMTABLE
DEFGET
DEFNEW
DEFMMETHOD( DragQuery )
DEFMMETHOD( DragDrop )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_ddstringclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Textinput, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "DDStringClass";
#endif

	return( TRUE );
}

void delete_ddstringclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}
