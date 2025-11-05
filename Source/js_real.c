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
** $Id: js_real.c,v 1.24 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "mui_func.h"

struct Data {
	double val;
};

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Number",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetTypeData )
{
	GETDATA;

	*msg->typeptr = expt_real;
	*msg->dataptr = &data->val;
	*msg->datasize = sizeof( data->val );

	return( TRUE );
}

DECSMETHOD( JS_SetData )
{
	GETDATA;
	data->val = TO_DOUBLE( msg->dataptr );
	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	char to[ 256 ];
	GETDATA;

	if( msg->radix )
	{
		ULONG v = (ULONG)data->val;
		int r = msg->radix;
		char *p = to;

		r = min( 16, r );
		r = max( 2, r );

		while( v )
		{
			*p++ = "0123456789ABCDEF"[v%r];
			v /= r;
		}
		*p = 0;
		strrev( to );
	}
	else
	{
		sprintf( to, "%.16g", data->val );
		process_nan( to );
	}

	if( msg->tosize && *msg->tosize )
	{
		stccpy( msg->tobuffer, to, *msg->tosize );
	}
	else
		strcpy( msg->tobuffer, to );

	if( msg->tosize )
		*msg->tosize = strlen( to );

	return( TRUE );
}

DECSMETHOD( JS_ToBool )
{
	GETDATA;

	*msg->boolptr = data->val ? TRUE : FALSE;

	return( TRUE );
}

DECSMETHOD( JS_ToReal )
{
	GETDATA;

	*msg->realptr = data->val;

	return( TRUE );
}

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetTypeData )
DEFSMETHOD( JS_SetData )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_ToBool )
DEFSMETHOD( JS_ToReal )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_real( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_RealClass";
#endif

	return( TRUE );
}

void delete_js_real( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_real( void )
{
	return( mcc->mcc_Class );
}
