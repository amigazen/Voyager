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
 * PluginsWin window class
 * -----------------------
 * - Handles the plugin preference window
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: pluginswin.c,v 1.15 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

#if USE_PLUGINS

#include "classes.h"
#include "voyager_cat.h"
#include <proto/v_plugin.h>
#ifdef MBX
#include "md/v_plugin_lib_calls.h"
#endif
#include "plugins.h"
#include "mui_func.h"


APTR pluginswin;

struct Data {
	APTR lv_sel;
	APTR txt_l_name;
	APTR txt_l_version;
	APTR txt_l_copyright;
	APTR txt_l_infostring;
	APTR lv_info;
	APTR prefscontainer;
	APTR current_prefsobject;
	APTR dummy_prefsobject;
	ULONG cnt;
};


DECNEW
{
	struct Data *data;
	struct plugin *plugin;
	APTR bt_ok;
	APTR bt_cancel;
	APTR lv_sel, lv_info;
	APTR txt_l_name, txt_l_version, txt_l_copyright, txt_l_infostring;
	APTR prefscontainer, current_prefsobject;
	ULONG cnt = 0;
	static STRPTR regtitles[ 3 ];

	/*
	 * Do not open if there's no plugins
	 */
	if( ISLISTEMPTY( &pluginlist ) )
	{
		MUI_Request( app, NULL, 0, NULL, GS( CANCEL ), GS( PLUGIN_NO_PLUGINS ), 0 );
		return( 0 );
	}

	plugin = FIRSTNODE( &pluginlist );

	regtitles[ 0 ] = GS( PLUGIN_REG1 );
	regtitles[ 1 ] = GS( PLUGIN_REG2 );

	/*
	 * Object used when there's no prefs
	 */
	current_prefsobject = TextObject, MUIA_Text_PreParse, "\033c", MUIA_Text_Contents, GS( PLUGIN_L_NOPREFS), End;

	if( !current_prefsobject )
	{
		displaybeep();
		return( 0 );
	}

	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		cnt++;
	}

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID( 'P','R','E','P' ),
		MUIA_Window_Title, GS( PLUGIN_WIN_TITLE ),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_NoMenus, TRUE,
		WindowContents, VGroup,
			Child, HGroup,

				Child, lv_sel = ListviewObject, MUIA_CycleChain, 1, MUIA_Weight, 0,
					MUIA_Listview_List, NewObject( getpluginwinlist(), NULL,
												   MA_PluginsWinlist_Counter, cnt,
												   TAG_DONE ),
				End,

				Child, RegisterGroup( regtitles ),

					Child, VGroup,
						Child, ColGroup( 2 ),

							Child, Label2( GS( PLUGIN_L_NAME ) ),
							Child, txt_l_name = TextObject, NoFrame, End,

							Child, Label2( GS( PLUGIN_L_VERSION ) ),
							Child, txt_l_version = TextObject, NoFrame, End,

							Child, VGroup,
								Child, Label2( GS( PLUGIN_L_COPYRIGHT ) ),
								Child, VSpace( 0 ),
							End,
							Child, txt_l_copyright = FloattextObject, NoFrame, MUIA_Background, MUII_RegisterBack, End,

							Child, Label2( GS( PLUGIN_L_INFOSTRING ) ),
							Child, txt_l_infostring = TextObject, NoFrame, End,
						End,

						Child, lv_info = ListviewObject, MUIA_Listview_Input, FALSE,
							MUIA_Listview_List, ListObject, ReadListFrame,
								MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
								MUIA_List_DestructHook, MUIV_List_DestructHook_String,
							End,
						End,
					End,

					Child, prefscontainer = VGroup,
						Child, current_prefsobject,
					End,

				End,
			End,

			Child, HGroup,
				Child, bt_ok = button( MSG_OK, 0 ),
				Child, bt_cancel = button( MSG_CANCEL, 0 ),
			End,
		End,
	End;

	if( !obj )
	{
		displaybeep();
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	data->lv_sel = lv_sel;
	data->lv_info = lv_info;
	
	data->txt_l_name = txt_l_name;
	data->txt_l_version = txt_l_version;
	data->txt_l_copyright = txt_l_copyright;
	data->txt_l_infostring = txt_l_infostring;

	data->prefscontainer = prefscontainer;
	data->current_prefsobject = current_prefsobject;
	data->dummy_prefsobject = current_prefsobject;

	data->cnt = cnt;

	DoMethod( bt_ok, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 5, MUIM_Application_PushMethod, obj, 2, MM_PluginsWin_Close, TRUE
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 5, MUIM_Application_PushMethod, obj, 2, MM_PluginsWin_Close, FALSE
	);
	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 5, MUIM_Application_PushMethod, obj, 2, MM_PluginsWin_Close, FALSE
	);

	/*
	 * Building up the list of plugins
	 */
	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		DoMethod( lv_sel, MUIM_List_InsertSingle, plugin, MUIV_List_Insert_Bottom );
	}

	set( lv_sel, MUIA_List_Active, 0 );

	return( ( ULONG )obj );
}


DECSMETHOD( PluginsWin_Close )
{
	GETDATA;
	struct plugin *plugin;
	
	D( db_plugin, bug( "closing...\n" ) );

	set( obj, MUIA_Window_Open, FALSE );
	
	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		if ( plugin->hasprefs )
		{
			if( msg->use == TRUE )
			{
				D( db_plugin, bug( "sending VPLUGPREFS_Use...\n" ) );
				VPLUG_Hook_Prefs( VPLUGPREFS_Use, &plugin->prefs );
			}

			if( plugin->prefsobject_exists )
			{
				D( db_plugin, bug( "plugin %lx has a prefsobject\n", plugin ) );
				
				if( data->current_prefsobject == plugin->prefs.object )
				{
					D( db_plugin, bug( "it's the one currently displayed !\n" ) );
					DoMethod( data->prefscontainer, OM_REMMEMBER, data->current_prefsobject );
				}

				VPLUG_Hook_Prefs( VPLUGPREFS_Dispose, &plugin->prefs );
				
				plugin->prefsobject_exists = FALSE;

				D( db_plugin, bug( "disposed plugin %lx\n", plugin ) );
			}
		}
	}

	D( db_plugin, bug( "removing object...\n" ) );
	DoMethod( app, OM_REMMEMBER, obj );
	D( db_plugin, bug( "disposing...\n" ) );
	MUI_DisposeObject( obj );
	D( db_plugin, bug( "all done, returning\n" ) );

	pluginswin = NULL;

	return( 0 );
}


/*
 * Sets the Version string of the infolist
 */
DECSMETHOD( PluginsWin_SetVer )
{
	GETDATA;

	DoMethod( data->txt_l_version, MUIM_SetAsString, MUIA_Text_Contents, "%ld.%ld (API version %ld)", msg->ver, msg->rev, msg->api );

	return( 0 );
}


/*
 * Clears the infolist
 */
DECMETHOD( PluginsWin_ClearInfo, ULONG )
{
	GETDATA;

	DoMethod( data->lv_info, MUIM_List_Clear );
	
	return( 0 );
}


/*
 * Changes the prefsobject for the plugin's one
 */
DECSMETHOD( PluginsWin_ChangeGroup )
{
	GETDATA;
	struct plugin *plugin; 

	//D( db_plugin, bug("plugin == %lx\n", msg->plugin ) );
	
	DoMethod( data->prefscontainer, MUIM_Group_InitChange );

	DoMethod( data->prefscontainer, OM_REMMEMBER, data->current_prefsobject );
	
	D( db_plugin, bug( "prefscontainer OM_REMMEMBERed\n" ) );

	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		if( plugin == msg->plugin )
		{
			D( db_plugin, bug( "plugin found\n" ) );
			break;
		}
	}

	D( db_plugin, bug( "loop iterated\n" ) );

	if( msg->plugin->hasprefs )
	{
		D( db_plugin, bug( "has prefs\n" ) );
		if( !msg->plugin->prefsobject_exists )
		{
			D( db_plugin, bug( "sending create method..\n" ) );
			
			VPLUG_Hook_Prefs( VPLUGPREFS_Create, &msg->plugin->prefs );
			
			D( db_plugin, bug( "done... prefs.object is %lx\n", msg->plugin->prefs.object ) );
			
			if( msg->plugin->prefs.object )
			{
				msg->plugin->prefsobject_exists = TRUE;
			}
			else
			{
				VPLUG_Hook_Prefs( VPLUGPREFS_Dispose, &msg->plugin->prefs );
				msg->plugin->hasprefs = FALSE; /* shut up forever, could be smarter though */
			}
		}
	}

	if( !msg->plugin->hasprefs )
	{
		D( db_plugin, bug( "no prefs\n" ) );
		msg->plugin->prefs.object = data->dummy_prefsobject;
	}

	D( db_plugin, bug( "adding prefs.object now...\n" ) );
	
	DoMethod( data->prefscontainer, OM_ADDMEMBER, msg->plugin->prefs.object );
	
	data->current_prefsobject = msg->plugin->prefs.object;
	DoMethod( data->prefscontainer, MUIM_Group_ExitChange );

	D( db_plugin, bug( "ExitChange done\n" ) );

	return( 0 );
}


DECGET
{
	GETDATA;

	if( msg->opg_AttrID == MA_PluginsWin_Listview )
	{
		*msg->opg_Storage = ( ULONG )data->lv_sel;
		return( TRUE );
	}
	return( DOSUPER );
}


DECSET
{
	struct TagItem *tag, *tagp = msg->ops_AttrList;
	GETDATA;

	while( tag = NextTagItem( &tagp ) ) switch( (int)tag->ti_Tag )
	{
		case MA_PluginsWin_Name:
			set( data->txt_l_name, MUIA_Text_Contents, ( STRPTR )tag->ti_Data );
			break;

		case MA_PluginsWin_Copyright:
			set( data->txt_l_copyright, MUIA_Floattext_Text, ( STRPTR )tag->ti_Data );
			break;

		case MA_PluginsWin_InfoString:
			set( data->txt_l_infostring, MUIA_Text_Contents, ( STRPTR )tag->ti_Data );
			break;

		case MA_PluginsWin_Info:
			DoMethod( data->lv_info, MUIM_List_InsertSingle, tag->ti_Data, MUIV_List_Insert_Bottom );
			break;
	}
	
	return( DOSUPER );
}

BEGINMTABLE
DEFNEW
DEFGET
DEFSET
DEFSMETHOD( PluginsWin_SetVer )
DEFSMETHOD( PluginsWin_Close )
DEFSMETHOD( PluginsWin_ChangeGroup )
DEFMETHOD( PluginsWin_ClearInfo )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_pluginwinclass( void )
{
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "PluginWinClass";
#endif

	return( TRUE );
}

void delete_pluginwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getpluginswin( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_PLUGINS */
