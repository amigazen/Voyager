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
 * PluginsWinList list class
 * -------------------------
 * - Custom list class for the plugin prefs window
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: pluginswin_list.c,v 1.17 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

#if USE_PLUGINS

#include "classes.h"
#include "voyager_cat.h"
#include <proto/v_plugin.h>
#include "plugins.h"
#include "mui_func.h"
#include "malloc.h"

struct Data {
	APTR *images;
	APTR *iobj;
	int cnt;
} *data;


MUI_HOOK( pluginwinlistdisp, STRPTR *array, struct plugin *plug )
{
	static char tmp[ 40 ];
	sprintf( tmp, "\033O[%08lx] %s", (ULONG)data->images[ (ULONG)array[ -1 ] ], plug->prefs.label );
	array[ 0 ] = tmp;
	return( 0 );
}

DECNEW
{
	struct TagItem *tag;

	obj = DoSuperNew( cl, obj,
		InputListFrame,
		MUIA_List_DisplayHook, &pluginwinlistdisp_hook,
		MUIA_List_MinLineHeight, 17,
		MUIA_List_AdjustWidth, TRUE,
	End;

	D( db_plugin, bug( "new list\n" ) );

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	tag = FindTagItem( MA_PluginsWinlist_Counter, msg->ops_AttrList );
	if( tag  )
	{
		D( db_plugin, bug( "allocating %lu images..\n", tag->ti_Data ) );
		
		data->cnt = tag->ti_Data;
		data->images = malloc( data->cnt * 4 );
		memset( data->images, '\0', data->cnt * 4 ); /* TOFIX: maybe not needed */
		data->iobj = malloc( data->cnt * 4 );
		memset( data->iobj, '\0', data->cnt * 4 ); /* TOFIX: maybe not needed, missing NULL check btw */
	}

	DoMethod( obj, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Prefslist_Selectchange
	);

	return( (ULONG)obj );
}


DECMMETHOD( Setup )
{
	GETDATA;
	struct plugin *plugin;
	int cnt = 0;

	if( !DOSUPER )
		return( FALSE );

	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ), cnt++ )
	{
		data->iobj[ cnt ] = BitmapObject,
			MUIA_Bitmap_Bitmap, plugin->prefs.bitmap,
			MUIA_Bitmap_SourceColors, plugin->prefs.colormap,
			MUIA_Bitmap_Width       , 24, MUIA_FixWidth  , 24,
			MUIA_Bitmap_Height      , 14, MUIA_FixHeight , 14,
			MUIA_Bitmap_Transparent , 0,
		End;
		data->images[ cnt ] = (APTR)DoMethod( obj, MUIM_List_CreateImage, data->iobj[ cnt ], 0 );
	}

	return( TRUE );
}

DECMMETHOD( Cleanup )
{
	int c;
	GETDATA;

	D( db_plugin, bug( "in Cleanup..\n" ) );

	for( c = 0; c < data->cnt; c++ )
	{
		DoMethod( obj, MUIM_List_DeleteImage, data->images[ c ] );
		MUI_DisposeObject( data->iobj[ c ] );
	}

	return( DOSUPER );
}


DECDISPOSE
{
	GETDATA;

	D( db_plugin, bug( "data->images == %lx\n", data->images ) );

	if( data->images )
	{	
		free( data->images );
	}

	D( db_plugin, bug( "data->iobj == %lx\n", data->iobj ) );

	if( data->iobj )
	{
		free( data->iobj );
	}

	D( db_plugin, bug( "fine...\n" ) );

	return( DOSUPER );
}


DECMETHOD( Prefslist_Selectchange, APTR )
{
	struct plugin *plugin;
	struct TagItem *tag, *tagp;
	char buffer[ 1024 ];
	int ver = 42, rev = 23, api = 1;

	DoMethod( ( APTR )getv( pluginswin, MA_PluginsWin_Listview ), MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &plugin );
	if( !plugin )
		return( 0 );

	DoMethod( pluginswin, MM_PluginsWin_ClearInfo );

	set( pluginswin, MA_PluginsWin_Name, plugin->name );

	tagp = plugin->querylist;

	while( tag = NextTagItem( &tagp ) ) switch( tag->ti_Tag )
	{
		case VPLUG_Query_Version:
			ver = tag->ti_Data;
			break;

		case VPLUG_Query_Revision:
			rev = tag->ti_Data;
			break;

		case VPLUG_Query_APIVersion:
			api = tag->ti_Data;
			break;

		case VPLUG_Query_Copyright:
			set( pluginswin, MA_PluginsWin_Copyright, tag->ti_Data );
			break;

		case VPLUG_Query_Infostring:
			set( pluginswin, MA_PluginsWin_InfoString, tag->ti_Data );
			break;

		case VPLUG_Query_RegisterURLMethod:
			sprintf( buffer, GS( PLUGIN_L_HANDLES_URL ), tag->ti_Data );
			set( pluginswin, MA_PluginsWin_Info, buffer );
			break;

		case VPLUG_Query_RegisterMIMEType:
			sprintf( buffer, GS( PLUGIN_L_HANDLES_MIME ), tag->ti_Data );
			set( pluginswin, MA_PluginsWin_Info, buffer ); 
			break;

		case VPLUG_Query_RegisterMIMEExtension:
			sprintf( buffer, GS( PLUGIN_L_HANDLES_MIMEEXT ), tag->ti_Data );
			set( pluginswin, MA_PluginsWin_Info, buffer ); 
			break;
	}

	/* changing the plugin's prefs */
	DoMethod( pluginswin, MM_PluginsWin_ChangeGroup, plugin );
	
	DoMethod( pluginswin, MM_PluginsWin_SetVer, ver, rev, api );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISP
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMETHOD( Prefslist_Selectchange )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_pluginwinlistclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_List, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "PluginWinListClass";
#endif

	return( TRUE );
}

void delete_pluginwinlistclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getpluginwinlist( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_PLUGINS */
