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
** $Id: js_plugin.c,v 1.19 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"

/* private */
#include "classes.h"
#include "js.h"
#include "copyright.h"
#if USE_PLUGINS
#include <proto/v_plugin.h>
#include "plugins.h"
#endif
#include "mui_func.h"


struct Data {
#if USE_PLUGINS
	struct plugin *pl;
#else
	BOOL	pad;
#endif  
};

BEGINPTABLE
DPROP( name, 		string )
DPROP( filename, 	string )
DPROP( description,	string )
DPROP( enabledPlugin, bool )
ENDPTABLE

DECNEW
{
	struct Data *data;
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Plugin",
		TAG_MORE, msg->ops_AttrList
	);
	
	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
#if USE_PLUGINS
	data->pl = (APTR)GetTagData( MA_JS_Plugin_Plugin, 0, msg->ops_AttrList );
#endif
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
#if USE_PLUGINS
		case JSPID_description:
			storestrprop( msg, (STRPTR)GetTagData( VPLUG_Query_Infostring, (ULONG)"",  data->pl->querylist ) );
			return( TRUE );

		case JSPID_filename:
			storestrprop( msg, data->pl->name );
			return( TRUE );

		case JSPID_name:
			storestrprop( msg, (STRPTR)getv( obj, MA_JS_Name ) );
			return( TRUE );

		case JSPID_enabledPlugin:
			storeintprop( msg, TRUE );
			return( TRUE );
#else
		case JSPID_description:
		case JSPID_filename:
		case JSPID_name:
			storestrprop( msg, "N/A" ); //TOFIX!! This should be internationalised, right?
			return( TRUE );

#endif /* This shouldn't be used if we're not using plugins, as the plugin list will be empty. */
	}

	return( DOSUPER );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_plugin( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_array_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_PluginClass";
#endif

	return( TRUE );
}

void delete_js_plugin( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_plugin( void )
{
	return( mcc->mcc_Class );
}
