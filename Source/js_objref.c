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
** $Id: js_objref.c,v 1.26 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "malloc.h"
#include "mui_func.h"


struct Data {
	char *name;
	int funccontext;
	APTR obj;
	ULONG magic;
};

static void setattrs( struct IClass *cl, APTR obj, struct TagItem *tagp )
{
	struct TagItem *tag;
	GETDATA;

	while( tag = NextTagItem( &tagp ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_Name:
			if( data->name )
			{
				free( data->name );
				data->name = NULL;
			}
			if( tag->ti_Data )
				data->name = strdup( (STRPTR)tag->ti_Data ); /* TOFIX */
			break;

		case MA_JS_FuncContext:
			data->funccontext = tag->ti_Data;
			break;

		case MA_JS_ObjRef_Object:
			data->obj = (APTR)tag->ti_Data;
			break;
	}
}

DECNEW
{
	struct Data *data;

	obj = (APTR)DOSUPER;
	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	setattrs( cl, obj, msg->ops_AttrList );

	data->magic = (ULONG)-1;

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;
	if( data->name )
		free( data->name );
	return( DOSUPER );
}

DECSET
{
	setattrs( cl, obj, msg->ops_AttrList );
	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_JS_Name:
			*msg->opg_Storage = (ULONG)data->name;
			return( TRUE );

		case MA_JS_FuncContext:
			*msg->opg_Storage = (ULONG)data->funccontext;
			return( TRUE );

		case MA_JS_ClassName:
			if( !data->obj )
			{
				*msg->opg_Storage = (ULONG)"DOBJREF";
				return( TRUE );
			}
			break;

		case MA_JS_ObjRef_Object:
			*msg->opg_Storage = (ULONG)data->obj;
			return( TRUE );	
	}

	if( data->obj )
		if( DoMethodA( data->obj, ( Msg )msg ) )
			return( TRUE );

	return( DOSUPER );
}

DECMETHOD( JS_NameIs, STRPTR )
{
	GETDATA;

	if( data->name )
	{
		return( (ULONG)!strcmp( data->name, msg[ 1 ] ) );
	}

	return( FALSE );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;
	if( data->obj )
	{
		return( DoMethodA( data->obj, ( Msg )msg ) );
	}
	js_tostring( "null", msg );
	return( TRUE );
}

DECSMETHOD( JS_ToBool )
{
	GETDATA;
	if( data->obj )
	{
		return( DoMethodA( data->obj, ( Msg )msg ) );
	}
	*msg->boolptr = FALSE;
	return( TRUE );
}

DECSMETHOD( JS_SetGCMagic )
{
	GETDATA;

	if( data->obj )
	{
		if( data->magic != msg->magic )
		{
			data->magic = msg->magic;
			DoMethodA( data->obj, (Msg)msg );
		}
	}
	else
	{
		data->magic = msg->magic;
	}
	return( 0 );
}

DECTMETHOD( JS_GetGCMagic )
{
	GETDATA;
	return( data->magic );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSET
DEFGET
DEFSMETHOD( JS_NameIs )
DEFMETHOD( JS_ToString )
DEFMETHOD( JS_ToBool )
DEFSMETHOD( JS_SetGCMagic )
DEFSMETHOD( JS_GetGCMagic )

	case MM_JS_ToReal:
	case MM_JS_SetProperty:
	case MM_JS_GetProperty:
	case MM_JS_HasProperty:
	case MM_JS_DeleteProperty:
	case MM_JS_CallMethod:
		{
			// Forward to "real" object
			GETDATA;
			if( data->obj )
				return( DoMethodA( data->obj, msg ) );
			else
				return( FALSE );
		}
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_objref( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Notify, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_ObjrefClass";
#endif

	return( TRUE );
}

void delete_js_objref( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_objref( void )
{
	return( mcc->mcc_Class );
}
