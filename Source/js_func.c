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
** $Id: js_func.c,v 1.21 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "mui_func.h"

struct Data {
	LONG index;
	APTR obj;
	APTR argarray;
	APTR window;
};

BEGINPTABLE
DPROP( arguments,	obj )
DPROP( arity,		real )
DPROP( caller,		funcptr )
ENDPTABLE

DECSMETHOD( JS_GetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_arguments:
			if( !data->argarray )
			{
				data->argarray = JSNewObject( getjs_array(), TAG_DONE );
			}
			storeobjprop( msg, data->argarray );
			return( TRUE );
	}

	return( DOSUPER );
}

static void setattrs( struct IClass *cl, APTR obj, struct TagItem *tagp )
{
	struct TagItem *tag;
	GETDATA;

	while( ( tag = NextTagItem( &tagp ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_Func_Index:
			data->index = tag->ti_Data;
			break;

		case MA_JS_Func_Object:
			data->obj = (APTR)tag->ti_Data;
			break;

		case MA_JS_Func_Window:
			data->window = (APTR)tag->ti_Data;
			break;
	}
}

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Function",
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	setattrs( cl, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECSET
{
	setattrs( cl, obj, msg->ops_AttrList );
	return( DOSUPER );
}

DECDEST
{
	//GETDATA;
	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_JS_Func_Index:
			*msg->opg_Storage = (ULONG)data->index;
			return( TRUE );

		case MA_JS_Func_Object:
			*msg->opg_Storage = (ULONG)data->obj;
			return( TRUE );

		case MA_JS_Func_Window:
			*msg->opg_Storage = (ULONG)data->window;
			return( TRUE );
	}

	return( DOSUPER );
}


DECSMETHOD( JS_GetTypeData )
{
	GETDATA;

	*msg->typeptr = expt_funcptr;
	*msg->dataptr = &data->index;
	*msg->datasize = 8;

	return( TRUE );
}

DECSMETHOD( JS_SetData )
{
	GETDATA;
	memcpy( &data->index, msg->dataptr, msg->datasize );
	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	strcpy( msg->tobuffer, "[Function]" );

	if( msg->tosize )
		*msg->tosize = strlen( msg->tobuffer );

	return( TRUE );
}

DECMETHOD( JS_Func_SetParms, APTR )
{
	GETDATA;
	data->argarray = msg[ 1 ];
	return( TRUE );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSET
DEFGET
DEFSMETHOD( JS_GetTypeData )
DEFSMETHOD( JS_SetData )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_Func_SetParms )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_func( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_FuncClass";
#endif

	return( TRUE );
}

void delete_js_func( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_func( void )
{
	return( mcc->mcc_Class );
}
