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
 * App class
 * ---------
 * - ..
 *
 * © 1999-2003 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: app.c,v 1.103 2003/11/21 07:48:49 zapek Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/layers.h>
#endif

/* private */
#include "voyager_cat.h"
#include "copyright.h"
#include "classes.h"
#include "htmlwin.h"
#include "htmlclasses.h"
#include <proto/vimgdecode.h>
#include "nagwin.h"
#include "mui_func.h"
#include "prefs.h"
#include "methodstack.h"
#include "prunecache.h"
#include "network.h"
#include "win_func.h"
#include "errorwin.h"
#include "netinfo.h"
#include "rexx.h"
#include "menus.h"
#include "download.h"
#include "cmanager.h"
#if USE_PLUGINS
#include "plugins.h"
#endif /* USE_PLUGINS */
#if USE_STB_NAV
#include "mbx.h"
#include <mbxgui_lib_calls.h>
#include <modules/mbxgui/classes.h>
extern MCPBASE;
#endif

struct Data {
	int dummy;
#if USE_LO_PIP
	Object* pipWindow;
	int pipMode;
	int pipEnabled;
	struct MUI_InputHandlerNode pip_ihn;
	int pip_ihn_active;
#endif
#ifdef MBX
	ULONG screenWidth;
#endif	
};

#ifdef MBX
static void load (struct Data* data)
{
	EnvNode_p e;
	UDWORD t;
		
	kprintf ("Loading options\n");
	
	e = EnterTree(NULL, "www/options", ENVMODE_READ);
	if (e) {
		if (ReadEnvUDword (e, "pipMode", &t,0)==STAT_OK) {
			kprintf ("pipMode=%d\n", t);
			data->pipMode = t;
		}
		LeaveTree(e);
	}

	e = EnterTree(NULL, "www/options", ENVMODE_READ);	
	if (e) {
		if (ReadEnvUDword (e, "pipPos", &t,0)==STAT_OK) {
			kprintf ("pipPosition=%d\n", t);
			// Leave tree, because the set call writes the settings back to
			// env_lib, will cause a deadlock otherwise
			LeaveTree (e);
			set (data->pipWindow, MA_Pipwindow_Position, t);
		} else
			LeaveTree(e);
	}

	e = EnterTree(NULL, "www/options", ENVMODE_READ);
	if (e) {	
		if (ReadEnvUDword (e, "pipSize", &t,0)==STAT_OK) {
			kprintf ("pipSize=%d\n", t);
			// Leave tree, because the set call writes the settings back to
			// env_lib, will cause a deadlock otherwise			
			LeaveTree (e);		
			set (data->pipWindow, MA_Pipwindow_Size, t);
		} else
			LeaveTree(e);
	}
	
	kprintf ("Loading options finished\n");
}

DECCONST
{
	struct Data *data;
	Object *pipWindow;
	
	obj = DoSuperNew( cl, obj,
		MUIA_Application_Window, pipWindow = NewObject (getpipwindowclass(),NULL,TAG_DONE),
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->pipMode = 2;
	data->pipWindow = pipWindow;
	data->pipEnabled = 1;
	data->screenWidth=740;
	load (data);

	{
		UDWORD t = data->pipMode;
		data->pipMode = 0;
		set (obj, MA_App_Pipmode, t);
	}
	
	return( (ULONG)obj );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_App_Pipmode, data->pipMode );
		STOREATTR( MA_App_Pipwindow, data->pipWindow );
		STOREATTR( MA_App_PipEnabled, data->pipEnabled );
		STOREATTR (MA_App_ScreenWidth, data->screenWidth);
	}

	return( DOSUPER );	
}

#endif

DECSMETHOD( DoAllWins )
{
	imgdec_tick();
	doallwins_a( (Msg)&msg->args );
	return( 0 );
}


DECSMETHOD( DoActiveWin )
{
	if( lastactivewin )
	{
		if( command_runmode == VREXX_WINDOW )
		{
			return( DoMethodA( lastactivewin, ( Msg )&msg->args ) );
		}
		else
		{
			return( DoMethod( lastactivewin, MM_HTMLWin_DoActiveFrame, ( Msg )&msg->args ) );
		}
	}
	else
	{
		return( 0 );
	}
}


DECSMETHOD( DoActivePanel )
{
	if( lastactivepanel )
	{
		return( DoMethodA( lastactivepanel, ( Msg )&msg->args ) );
	}
	else
	{
		return( 0 );
	}
}


DECSMETHOD( DoLastActivePanel )
{
	if( lastactivepanel )
	{
		return( DoMethodA( lastactivepanel, ( Msg )&msg->args ) );
	}
	else
	{
		return( 0 );
	}
}


/*
 * This method uses DoActiveWin if there's no WIN/K/N, otherwise
 * DoNumWin
 */
DECSMETHOD( DoRexxWin )
{
	/* heavy magic */
	if( msg->win )
	{
		msg->win = MM_DoNumWin;
	}
	else
	{
		msg->win = MM_DoActiveWin;
	}
	return( DoMethodA( obj, ( Msg )&msg->win ) );
}


DECSMETHOD( DoLastActiveWin )
{
	if( lastactivewin )
	{
		return( DoMethodA( lastactivewin, ( Msg )&msg->args ) );
	}
	else
	{
		return( 0 );
	}
}


DECSMETHOD( DoNumWin )
{
	return( ( ULONG )donumwins_a( msg->num, ( Msg )&msg->args ) );
}


DECMETHOD( GetActiveWindow, ULONG )
{
	if( getv( app, MUIA_Application_Iconified ) )
	{
		return( 0 );
	}
	else
	{
		return( get_active_window() );
	}
}

#ifdef MBX
DECMMETHOD( MBXApplication_GotoPortal )
{
#ifdef DEBUG	
	kprintf ("Entering Application_GotoPortal\r\n");
#endif	
	if( veryfirstwin )
	{
#ifdef DEBUG
		kprintf ("veryfirstwin!=0\r\n");
#endif		
		if( msg->url )
			DoMethod( veryfirstwin, MM_HTMLWin_SetURL, msg->url, NULL, NULL, MF_HTMLWin_AddURL );
		DoMethod( veryfirstwin, MM_HTMLWin_SelectPage, veryfirstwin );
		//	mcp handles iconification	set( app, MUIA_Application_Iconified, FALSE );
	}
	else
	{
#ifdef DEBUG
		kprintf ("veryfirstwin=0\r\n");
#endif		
		DoMethod( app, MUIM_MBXApplication_GotoURL, msg->url, FALSE );
	}
#ifdef DEBUG	
	kprintf ("Leaving Application_GotoPortal\r\n");
#endif	
	return( 0 );
}
#endif

#ifndef MUICFG_UseRexx
#define MUICFG_UseRexx                126 /* private */
#endif

DECMMETHOD( Application_PreConfig )
{
	/*
	 * V needs ARexx so we set MUI to automatically do so.
	 */

	DoMethod( msg->configdata, MUIM_Configdata_Set, MUICFG_UseRexx, TRUE );
	return( DOSUPER );
}


#ifdef MBX
DECMMETHOD( MBXApplication_GotoURL )
{
	kprintf ("VOYAGER: Entering Application_GotoURL\r\n");	
	if( msg->newwin )
	{
		win_create( "unnamed", msg->url, NULL, NULL, FALSE, FALSE, FALSE );
		return( TRUE );
	}
	if( veryfirstwin )
		{
			if( !strcmp( msg->url, (char*)getv( veryfirstwin, MA_JS_Window_URL ) ) )
				{
					DoMethod( veryfirstwin, MM_HTMLWin_SelectPage, veryfirstwin );
					// mcp handles iconification set( app, MUIA_Application_Iconified,
					// FALSE );
					kprintf ("VOYAGER: Leaving Application_GotoURL\r\n");			
					return( TRUE );
				}
		}
	
	DoMethod( app, MM_DoLastActiveWin, MM_HTMLWin_SetURL, msg->url, NULL, ( lastactivewin == veryfirstwin ) ? "_blank" : NULL, MF_HTMLWin_AddURL );
	// mcp handles iconification set( app, MUIA_Application_Iconified, FALSE
	// );
	kprintf ("VOYAGER: Leaving Application_GotoURL\r\n");		
	return( TRUE );
}
#endif

DECMMETHOD( Application_ClosePublic )
{
	// During Iconify or stuff
	// Tell Image Decoder that the screen may go away
#ifndef MBX
	if( VIDBase )
		imgdec_setdestscreen( NULL, 0, 0, 0 , 0 );
#endif /* !MBX */
	return( DOSUPER );
}


DECMETHOD( App_DemoTimeout, APTR )
{
#if USE_KEYFILES
	// Demo timeout
	donag( 0 );
	DoMethod( obj, MM_DoAllWins, MM_App_DemoTimeout );
#endif
	return( DOSUPER );
}


DECMETHOD( App_GetErrorWinClass, ULONG )
{
	return( ( ULONG )NewObject( geterrorwinclass(), NULL, TAG_DONE ) );
}


#if 0
DECSMETHOD( App_DisplayError )
{
	// well well, find the active window if any and MUI_Request.. but I don't like sync requesters..

	MUI_Request( app, NULL, 0, GS( ERROR ), GS( OK ), "\033cUnable to open page\n%.60s\n\n%s", msg->url, msg->errorstring );

	free( msg->url );
	free( msg->errorstring );
	return( DOSUPER );
}
#endif

#if USE_CMANAGER
DECGET
{
	struct opGet *ogm = (APTR)msg;
	if( ogm->opg_AttrID == MUIA_CManager_Magic )
	{
#ifdef __MORPHOS__
		*ogm->opg_Storage = (LONG)&GATEmagicfunc;
#else
		*ogm->opg_Storage = (LONG)magicfunc;
#endif /* !__MORPHOS__ */
		return( TRUE );
	}
	return( DOSUPER );
}
#endif


/*
 * Opens a new Browser window
 */
DECMETHOD( App_NewWindow, ULONG )
{
	win_create( "", "", NULL, NULL, FALSE, FALSE, FALSE );
	return( 0 );
}


DECMMETHOD( DragQuery )
{
	int num = -1;
	struct MUIP_DragQuery *dm = (APTR)msg;

	get( dm->obj, MA_Fastlink_Number, &num );
	if( num >= 0 )
		return( MUIV_DragQuery_Accept );
	else
		return( MUIV_DragQuery_Refuse );
}


DECMMETHOD( DragDrop )
{
#ifdef MBX
	return( 0 );
#else
	int num = -1, totalnum, c;
	struct MUIP_DragDrop *dm = (APTR)msg;
#if USE_NET

	get( dm->obj, MA_Fastlink_Number, &num );
	if( num >= 0 )
	{
		if( DoMethod( app, MUIM_Application_IdentifyLayer, WhichLayer( &_screen( dm->obj )->sc_LayerInfo, dm->x, dm->y ) ) )
			return( 0 );
		// Removing a fastlink
		if( !MUI_Request( app, _win( dm->obj ), 0, NULL, GS( FASTLINK_REMOVE_YESNO ), GS( FASTLINK_REMOVE ),
			getv( dm->obj, MA_DropObject_Name ),
			getv( dm->obj, MA_DropObject_URL )
		))
		return( 0 );

		totalnum = getprefslong( DSI_FASTLINKS_NUM, 8 );
		for( c = num; c < totalnum - 1; c++ )
		{
			setprefsstr( DSI_FASTLINKS_URLS + c, getprefs( DSI_FASTLINKS_URLS + c + 1 ) );
			setprefsstr( DSI_FASTLINKS_LABELS + c, getprefs( DSI_FASTLINKS_LABELS + c + 1 ) );
		}

		setprefslong( DSI_FASTLINKS_NUM, totalnum - 1 );

		pushmethod( app, 2, MM_DoAllWins, MM_HTMLWin_SetupFastlinks );
	}
#endif /* USE_NET */
	return( DOSUPER );
#endif
}


DECSMETHOD( App_CacheFlush )
{
	switch( msg->type )
	{
		case MV_App_CacheFlush_Mem:
			if( MUI_Request( app, 0, 0, GS( NOTE ), GS( FLUSH_CACHE_GAD ), GS( FLUSH_CACHE ), (ULONG) GS( FLUSH_CACHE_MEM ) ) )
			{
				set( app, MUIA_Application_Sleep, TRUE );
				nets_flushmem();
				set( app, MUIA_Application_Sleep, FALSE );
				MUI_Request( app, 0, 0, GS( NOTE ), GS( OK ), GS( MEMCACHE_FLUSHED ), 0 );
			}
			break;
#if USE_NET
		case MV_App_CacheFlush_Disk:
			openprunecachewin();
			break;
#endif /* USE_NET */
	}
	return( 0 );
}


DECMETHOD( App_CheckWinRemove, ULONG )
{
	checkwinremove();
	return( 0 );
}


DECSMETHOD( App_SetWinClose )
{
	set_window_close( msg->obj );
	return( 0 );
}


#ifdef MBX

// FIXME: some keys should only be handeld when htmlwin is active, move these
// keys to a handler in htmlwin (e.g. channelswitching keys in pip). When
// displaying the menu, those must not be active. (ckulla, 27.09.2001)

DECMMETHOD( Application_RawKeyHandler )
{
	if (msg->imsg->Class==IDCMP_RAWKEY)
	{
		extern void dorawkey(Object *obj,struct IntuiMessage *imsg);
		dorawkey(obj,msg->imsg);
	}
	return(0);
}
#endif /* MBX */


#if USE_PLUGINS
DECMETHOD( App_OpenPluginsWindow, ULONG )
{
	if( !pluginswin )
	{
		if( pluginswin = NewObject(getpluginswin(), NULL, TAG_DONE ) )
		{
			DoMethod( app, OM_ADDMEMBER, pluginswin );
		}
		else
		{
			displaybeep();
		}
	}

	if( pluginswin )
	{
		set( pluginswin, MUIA_Window_Open, TRUE );
	}

	return( 0 );
}
#endif /* USE_PLUGINS */


DECMETHOD( App_OpenErrorWindow, ULONG )
{
	if( !errorwin )
	{
		if( ( errorwin = ( APTR )DoMethod( app, MM_App_GetErrorWinClass ) ) )
		{
			DoMethod( app, OM_ADDMEMBER, errorwin );
		}
		else
		{
			displaybeep();
			return( 0 );
		}
	}
	
	set( errorwin, MUIA_Window_Open, TRUE );

	return( 0 );
}


extern APTR prefswindow; /* needed because we can't #include "prefswin.h" */
DECMETHOD( App_OpenPrefsWindow, ULONG )
{
	if( !prefswindow )
	{
		if( ( prefswindow = NewObject( getprefswin_mainclass(), NULL, TAG_DONE ) ) )
		{
			DoMethod( app, OM_ADDMEMBER, prefswindow );
		}
		else
		{
			displaybeep();
			return( 0 );
		}
	}

	set( prefswindow, MUIA_Window_Open, TRUE );

	if( !getv( prefswindow, MUIA_Window_Open ) )
	{
		displaybeep();
		MUI_Request( app, NULL, 0, NULL, GS( CANCEL ), GS( WINDOW_CANT_OPEN ), 0 );
	}
	return( 0 );
}


#if USE_NET
DECMETHOD( App_OpenNetinfoWindow, ULONG )
{
	if( !win_ni )
	{
		win_ni = NewObject( getnetinfowin(), NULL, TAG_DONE );
		if( win_ni )
		{
			DoMethod( app, OM_ADDMEMBER, ( ULONG )win_ni );
		}
		else
		{
			displaybeep();
			return( 0 );
		}
	}
	
	if( win_ni )
	{
		set( win_ni, MUIA_Window_Open, TRUE );
	}
	return( 0 );
}
#endif /* USE_NET */

DECTMETHOD( App_UpdateSpoofMenu )
{
	int c;

	if (menu)
	{
		for( c = 0; c < 3; c++ )
		{
			if (findmenu( MENU_SET_SPOOF_1 + c ))
				set( findmenu( MENU_SET_SPOOF_1 + c ), MUIA_Menuitem_Title, getprefsstr( DSI_NET_SPOOF_AS_1 + c, "(undefined)" ) );
		}
	}

	return( 0 );
}

DECTMETHOD( App_ApplySpoof )
{
	setup_useragent();

	return( 0 );
}

#if USE_CMANAGER
DECTMETHOD( App_OpenBMWin )
{
	return( (ULONG)bm_openwin() );
}
#endif /* USE_CMANAGER */

#if USE_NET
DECTMETHOD( App_OpenDownloadWin )
{
	return( (ULONG)open_downloadwin() );
}
#endif /* USE_NET */

#ifdef MBX
DECTMETHOD( JS_CDPlayer_DiskChange )
{
	extern void js_stb_gotdiskchange( void );
	js_stb_gotdiskchange();
	return( 0 );
}
#endif

#if USE_STB_NAV
DECTMETHOD( STB_FullScreenTV )
{
	mcp_GotoTV();
	return( 0 );
}

DECTMETHOD( STB_ToggleOnline )
{
#ifdef MBX
	NetData_p NetBase;
	NetBase = (NetData_p) OpenModule( NETNAME, NETVERSION );
	switch( Establish( TAG_DONE ) )
	{
		case DEV_EST_STATE_UNLINKED:
		case DEV_EST_STATE_UNLINKING:
		case DEV_EST_STATE_ASKUNLINK:
			Establish( ESTTAG_ACTION, EST_ACTION_LINK, ESTTAG_POLICY, EST_POLICY_ALWAYS, ESTTAG_SYNCHRONOUS, 1, TAG_DONE );
			break;	
		case DEV_EST_STATE_LINKED:
		case DEV_EST_STATE_LINKING:
		case DEV_EST_STATE_ASKLINK:
		default:
			Establish( ESTTAG_ACTION, EST_ACTION_UNLINK, ESTTAG_POLICY, EST_POLICY_ALWAYS, ESTTAG_SYNCHRONOUS, 1, TAG_DONE );
			break;
	}
	CloseModule( NetBase );
#endif
	return( 0 );
}
#endif

#ifdef MBX
int saveValueToEnv (char *str, UDWORD v);

DECSET
{
	GETDATA;
	extern ULONG use_mcp;
	extern ULONG iconifySignalsMCP;

	{
		struct TagItem *tag;
		struct TagItem *tags = msg->ops_AttrList;
		while( ( tag = NextTagItem( &tags ) ) ) 
		{
			switch(tag->ti_Tag) 
			{
			case MA_App_ScreenWidth:
				// FIXME! This sucks
				data->screenWidth =  tag->ti_Data;
				doallwins( MUIM_Set, MA_HTMLWin_VirtualWidth, data->screenWidth );
				break;
			case MA_App_PipEnabled:
				//kprintf("***GOT PIPENABLED %ld (%ld)\r\n", tag->ti_Data, data->pipEnabled );
				if( data->pipEnabled != tag->ti_Data )
				{
					data->pipEnabled = tag->ti_Data;
					//kprintf ("*** VOYAGER: set pipEnabled to %d\r\n", data->pipEnabled);
					if( data->pip_ihn_active )
					{
						DoMethod( obj, MUIM_Application_RemInputHandler, &data->pip_ihn );
						data->pip_ihn_active = FALSE;	
					}
					if( !data->pipEnabled )
					{
						set( data->pipWindow, MUIA_Window_Open, FALSE );
					}
					else
					{
						// enable PIP after 3sec timeout (wait while loading
						// another page).
						data->pip_ihn.ihn_Object = obj;
						data->pip_ihn.ihn_Flags = MUIIHNF_TIMER;
						data->pip_ihn.ihn_Millis = 1000 * 3;
						data->pip_ihn.ihn_Method = MM_App_ReenablePip;
						data->pip_ihn_active = TRUE;
						DoMethod( obj, MUIM_Application_AddInputHandler, &data->pip_ihn );
					}	
				}
				break;
			case MA_App_Pipmode:
				//kprintf( "***GOT PIPMODE %ld\r\n", tag->ti_Data );
				if( data->pipMode != tag->ti_Data)  
				{
					UDWORD iconified = FALSE;
					// FIXME: move to a global save function, called on box
					// shutdown					
					saveValueToEnv ("pipMode", tag->ti_Data);					
					get (obj, MUIA_Application_Iconified, &iconified);
					
					data->pipMode = tag->ti_Data;
					// mode = 0: no audio, no picture					
					// mode = 1: only audio, no picture
					// mode = 2: audio and picture

					if (data->pipEnabled) 
					{
						if (data->pipMode==0 && !iconified) 
						{
							mcp_tv_DisableTV();
						}
						if (data->pipMode>=1 && !iconified) 
						{
							mcp_tv_EnableTV();
						}
						if (data->pipMode==2) 
							set (data->pipWindow, MUIA_Window_Open, TRUE );
						else
							set (data->pipWindow, MUIA_Window_Open, FALSE );
					}
				} 
				break;
			}
		}
	}
	
	if (use_mcp) {
		
		struct TagItem *tag;
		struct TagItem *tags = msg->ops_AttrList;
		while( ( tag = NextTagItem( &tags ) ) ) 
		{
			switch(tag->ti_Tag) 
			{
				// Hook into iconify to signal mcp when V (de)iconfiy is done.
			case MUIA_Application_Iconified:
				{
					UDWORD iconified = FALSE;
					get (obj, MUIA_Application_Iconified, &iconified);

					if (iconified!=tag->ti_Data) 
					{
						if(!tag->ti_Data) 
						{
							KPrintF ("Entering voyager\n");
							// Turn off synclock to uniconify windows in synclock mode.
							mcp_SetSynclockMode (FALSE);
							tag->ti_Tag = 0;							
							SetSuperAttrs(cl,obj, MUIA_Application_Iconified,
							FALSE, TAG_DONE);

							if (data->pipEnabled) 
							{
								switch (data->pipMode) 
								{
								case 0:
									if (data->pipWindow)
										set (data->pipWindow, MUIA_Window_Open, FALSE);
									mcp_tv_DisableTV ();
									break;
								case 1:
									if (data->pipWindow)
										set (data->pipWindow, MUIA_Window_Open, FALSE);
									mcp_tv_EnableTV();
									break;
								case 2:
									mcp_tv_EnableTV();								
									break;
								}
							}
							if (iconifySignalsMCP)
							{
								mcp_SignalReady ();
							}
						} 
						else 
						{
							SetSuperAttrs(cl,obj, MUIA_Application_Iconified, TRUE, TAG_DONE);
							KPrintF ("Leaving voyager\n");
							if (iconifySignalsMCP) 
							{
								mcp_SignalReady ();
							}
						}
					}
				}
				break;
			}
		}
	}
	// FIXME: this may trigger a second iconify (but doesn't matter)
	return DoSuperMethodA(cl,obj,msg);		
}
#endif

#if USE_LO_PIP
DECTMETHOD( App_ReenablePip )
{
	GETDATA;
	int oldpipmode = data->pipMode;
	data->pipMode = -1;
	set( obj, MA_App_Pipmode, oldpipmode );
	if( data->pip_ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, &data->pip_ihn );
		data->pip_ihn_active = FALSE;
	}		
	return( 0 );
}
#endif

#ifdef VDEBUG
DECTMETHOD( App_SetImgDebug )
{
	imgdec_setdebug( db_level );
	
	return ( 0 );
}
#endif

BEGINMTABLE
#ifdef MBX
DEFCONST
DEFGET
DEFSET
DEFTMETHOD( JS_CDPlayer_DiskChange )
#endif
#if USE_CMANAGER
DEFGET
#endif
#if USE_STB_NAV
DEFTMETHOD( STB_FullScreenTV )
DEFTMETHOD( STB_ToggleOnline )
#endif
DEFMMETHOD( Application_ClosePublic )
DEFMMETHOD( Application_PreConfig )
#ifdef MBX
DEFMMETHOD( MBXApplication_GotoPortal )
DEFMMETHOD( MBXApplication_GotoURL )
#endif
DEFMMETHOD( DragQuery )
DEFMMETHOD( DragDrop )
#ifdef MBX
DEFMMETHOD( Application_RawKeyHandler )
#endif
DEFMETHOD( App_DemoTimeout )
DEFMETHOD( App_OpenErrorWindow )
DEFMETHOD( App_NewWindow )
DEFMETHOD( App_CheckWinRemove )
DEFMETHOD( GetActiveWindow )
DEFMETHOD( App_GetErrorWinClass )
#if USE_PLUGINS
DEFMETHOD( App_OpenPluginsWindow )
#endif /* USE_PLUGINS */
DEFMETHOD( App_OpenPrefsWindow )
#if USE_NET
DEFMETHOD( App_OpenNetinfoWindow )
#endif /* USE_NET */
DEFSMETHOD( DoLastActiveWin )
DEFSMETHOD( DoAllWins )
DEFSMETHOD( DoActiveWin )
DEFSMETHOD( DoActivePanel )
DEFSMETHOD( DoLastActivePanel )
DEFSMETHOD( DoNumWin )
DEFSMETHOD( DoRexxWin )
DEFSMETHOD( App_CacheFlush )
DEFSMETHOD( App_SetWinClose )
DEFTMETHOD( App_UpdateSpoofMenu )
DEFTMETHOD( App_ApplySpoof )
#if USE_CMANAGER
DEFTMETHOD( App_OpenBMWin )
#endif /* USE_CMANAGER */
#if USE_NET
DEFTMETHOD( App_OpenDownloadWin )
#endif /* USE_NET */
#if USE_LO_PIP
DEFTMETHOD( App_ReenablePip )
#endif
#ifdef VDEBUG
DEFTMETHOD( App_SetImgDebug )
#endif
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_appclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
#ifdef MBX
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL,
	NULL, MBXGUI_GetCustomClass (MUIC_MBXApplication), sizeof( struct Data ), DISPATCHERREF ) ) )	
#else
		if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Application, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
#endif
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "AppClass";
#endif

	return( TRUE );
}

void delete_appclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getappclass( void )
{
	return( mcc->mcc_Class );
}
