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
** $Id: js_mimetype.c,v 1.12 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "copyright.h"
#include "mui_func.h"

struct Data {
	int dummy;
};

BEGINPTABLE
DPROP( name, 		string )
DPROP( filename, 	string )
DPROP( description,	string )
DPROP( length,		real )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "MimeType",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetProperty )
{
	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_name:
			storeintprop( msg, TRUE );
			return( TRUE );
	}

	return( FALSE );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_mimetype( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_MimeTypeClass";
#endif

	return( TRUE );
}

void delete_js_mimetype( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}
