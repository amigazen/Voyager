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
** $Id: js_navigator.c,v 1.44 2004/11/16 16:01:15 zapek Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "copyright.h"
#if USE_PLUGINS
#include <proto/v_plugin.h>
#include "plugins.h"
#endif
#include "prefs.h"
#include "mui_func.h"
#include "host_os.h"

#ifdef MBX

#include <mpegcf_lib_calls.h>

static Mp3Data_p Mp3Base;

static void playmp3( char *file )
{
	if( !Mp3Base )
 	       Mp3Base = ( Mp3Data_p ) OpenModule( MP3NAME, MP3VERSION );
 
	if(Mp3Base)
	{
		if(OpenMp3Player()==TRUE)  // Open Player + GUI
		{
			PlayMp3Song( file ,0); // File Name, StartPosition in sec 
		}
	}
}

static void stopmp3( void )
{
	if( Mp3Base )
	{
		StopMp3Song();
		CloseMp3Player();
		CloseModule( (Module_p)Mp3Base );
		Mp3Base = NULL;
	}
}

#else

static void playmp3( char *dummy )
{

}

static void stopmp3( void )
{

}

#endif

struct Data {
	APTR plugin_array;
	APTR mime_array;
};

BEGINPTABLE
DPROP( appName, 	string )
DPROP( appCodeName, string )
DPROP( appVersion, 	string )
DPROP( cookieEnabled, bool  )
DPROP( onLine,      bool )
DPROP( cpuClass,    string )
DPROP( vendor,      string )
DPROP( userAgent, 	string )
DPROP( platform, 	string )
DPROP( javaEnabled, funcptr )
DPROP( playmp3,		funcptr )
DPROP( stopmp3,		funcptr )
DPROP( plugins,		obj )
DPROP( mimeTypes,	obj )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Navigator",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECDEST
{
	stopmp3();

	return( DOSUPER );
}

extern char vuseragent[];

static void buildpluginarray( struct Data *data )
{
#if USE_PLUGINS
	struct plugin *pl;
#endif

	data->plugin_array = JSNewObject( getjs_array(), MA_JS_Object_TerseArray, TRUE, TAG_DONE );

#if USE_PLUGINS
	for( pl = FIRSTNODE( &pluginlist ); NEXTNODE( pl ); pl = NEXTNODE( pl ) )
	{
		APTR id = (APTR)GetTagData( VPLUG_Query_PluginID, 0, pl->querylist );
		APTR o;
		char tmpname[ 128 ];
		struct TagItem *tag, *tagp;

		tagp = pl->querylist;

		if( !id )
		{
			sprintf( id = tmpname, "Unnamed%lx", (ULONG)pl );

			o = JSNewObject( getjs_plugin(),
				MA_JS_Plugin_Plugin, pl,
				MA_JS_Name, id,
				TAG_DONE
			);
	
			DoMethod( data->plugin_array, MM_JS_SetProperty, id, &o, sizeof( o ), expt_obj );
		}

		while( tag = NextTagItem( &tagp ) )
		{
			if( tag->ti_Tag == VPLUG_Query_PluginID )
			{
				o = JSNewObject( getjs_plugin(),
					MA_JS_Plugin_Plugin, pl,
					MA_JS_Name, tag->ti_Data,
					TAG_DONE
				);

				DoMethod( data->mime_array, MM_JS_SetProperty, tag->ti_Data, &o, sizeof( o ), expt_obj );
			}
		}
	}
#endif
}

static void buildmimearray( struct Data *data )
{
#if USE_PLUGINS
	struct plugin *pl;
#endif

	data->mime_array = JSNewObject( getjs_array(), MA_JS_Object_TerseArray, TRUE, TAG_DONE );

#if USE_PLUGINS
	for( pl = FIRSTNODE( &pluginlist ); NEXTNODE( pl ); pl = NEXTNODE( pl ) )
	{
		APTR id = (APTR)GetTagData( VPLUG_Query_PluginID, 0, pl->querylist );
		APTR o;
		char tmpname[ 128 ];
		struct TagItem *tag, *tagp;

		tagp = pl->querylist;

		if( !id )
			sprintf( id = tmpname, "Unnamed%lx", (ULONG)pl );

		while( tag = NextTagItem( &tagp ) )
		{
			if( tag->ti_Tag == VPLUG_Query_RegisterMIMEType )
			{
				o = JSNewObject( getjs_plugin(),
					MA_JS_Plugin_Plugin, pl,
					MA_JS_Name, id,
					TAG_DONE
				);

				DoMethod( data->mime_array, MM_JS_SetProperty, tag->ti_Data, &o, sizeof( o ), expt_obj );
			}
		}
	}
#endif
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
		case JSPID_appName:
			if( gp_spoof )
			{
				storestrprop( msg, getprefsstr( DSI_NET_SPOOF_AS_1_AN + gp_spoof - 1, "AmigaVoyager" ) );
			}
			else
			{
				storestrprop( msg, "AmigaVoyager" );
			}
			return( TRUE );
		
		case JSPID_appCodeName:
			if( gp_spoof )
			{
				storestrprop( msg, getprefsstr( DSI_NET_SPOOF_AS_1_AC + gp_spoof - 1, "AmigaVoyager" ) );
			}
			else
			{
				storestrprop( msg, "AmigaVoyager" );
			}
			return( TRUE );

		case JSPID_platform:
			if( gp_spoof )
			{
				storestrprop( msg, "Win32" );
			}
			else
			{
				storestrprop( msg, hostos );
			}
			return( TRUE );

		case JSPID_appVersion:
			if( gp_spoof )
			{
				storestrprop( msg, getprefsstr( DSI_NET_SPOOF_AS_1_AV + gp_spoof - 1, VERSIONSTRING ) );
			}
			else
			{
				storestrprop( msg, VERSIONSTRING );
			}
			return( TRUE );

		case JSPID_cookieEnabled:
			/*
			 * We assume both must be enabled. The fucking "standard"
			 * doesn't take this into account.
			 */
			if( getprefslong( DSI_SECURITY_ASK_COOKIE_PERM, 0 ) && getprefslong( DSI_SECURITY_ASK_COOKIE_TEMP, 0 ) )
			{
				storeintprop( msg, TRUE );
			}
			else
			{
				storeintprop( msg, FALSE );
			}
			return( TRUE );

		case JSPID_cpuClass:
			#ifdef __MORPHOS__
			storestrprop( msg, "PPC" );
			#else
			storestrprop( msg, "m68k" );
			#endif
			return ( TRUE );

		case JSPID_vendor:
			storestrprop( msg, "VaporWare" );
			return( TRUE );

		case JSPID_onLine:
			if ( gp_offlinemode )
			{
				storeintprop( msg, FALSE );
			}
			else
			{
				storeintprop( msg, TRUE );
			}
			return ( TRUE );

		case JSPID_userAgent:
			storestrprop( msg, vuseragent );
			return( TRUE );

		case JSPID_plugins:
			if( !data->plugin_array )
				buildpluginarray( data );
			storeobjprop( msg, data->plugin_array );
			return( TRUE);

		case JSPID_mimeTypes:
			if( !data->mime_array )
				buildmimearray( data );
			storeobjprop( msg, data->mime_array );
			return( TRUE);
	}

	return( FALSE );
}

DECSMETHOD( JS_CallMethod )
{
	//static double rval;

	switch( msg->pid )
	{
		case JSPID_javaEnabled:
			{
				*msg->typeptr = expt_bool;
				*msg->dataptr = (APTR)FALSE;
				return( TRUE );
			}
			break;

		case JSPID_playmp3:
			{
				char *search;

				if( msg->argcnt-- < 1 )
					return( FALSE );

				search = exprs_peek_as_string( msg->es, 0 );

				playmp3( search );

				exprs_pop( msg->es );

				*msg->typeptr = expt_bool;
				*msg->dataptr = (APTR)FALSE;
				return( TRUE );
			}
			break;

		case JSPID_stopmp3:
			{
				stopmp3();
				*msg->typeptr = expt_bool;
				*msg->dataptr = (APTR)FALSE;
				return( TRUE );
			}
			break;
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

DECSMETHOD( JS_SetGCMagic )
{
	GETDATA;

	if( data->plugin_array )
		DoMethodA( data->plugin_array, (Msg)msg );

	if( data->mime_array )
		DoMethodA( data->mime_array, (Msg)msg );

	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_SetGCMagic )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_navigator( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_NavigatorClass";
#endif

	return( TRUE );
}

void delete_js_navigator( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_navigator( void )
{
	return( mcc->mcc_Class );
}
