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
** $Id: js_stb_root.c,v 1.11 2003/07/06 16:51:33 olli Exp $
*/

// This is the root object of the "STB" JS control tree

/* Don't show this on public JS object listing: NO_PUBLIC_LISTING */

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "copyright.h"
#include "mui_func.h"

#ifdef MBX

struct Data {
	APTR cd_player;
};

BEGINPTABLE
DPROP( CDPlayer,	obj )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "STB",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	GETDATA;

	if( !pt )
		return( DOSUPER );

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}

	switch( pt->id )
	{
		case JSPID_CDPlayer:
			if( !data->cd_player )
				data->cd_player = JSNewObject( getjs_stb_cdplayer(), TAG_DONE );
			storeobjprop( msg, data->cd_player );
			return( TRUE);
	}

	return( FALSE );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DECSMETHOD( JS_SetGCMagic )
{
	GETDATA;

	if( data->cd_player )
		DoMethodA( data->cd_player, (Msg)msg );

	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_SetGCMagic )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_stb_root( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_STB_RootClass";
#endif

	return( TRUE );
}

void delete_js_stb_root( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_stb_root( void )
{
	return( mcc->mcc_Class );
}

#endif /* MBX */
