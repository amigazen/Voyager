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
 * STB Menu functions
 * ------------------
 *
 * © 2001 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: mbx_menu.c,v 1.58 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"
/* private */
#include "classes.h"
#include "htmlclasses.h"
#include "menus.h"
#include "voyager_cat.h"
#include "prefs.h"
#include "htmlwin.h"
#include "mui_func.h"
#include "textinput.h"


#if USE_STB_NAV
#include <mbxgui_lib_calls.h>
#include <modules/mbxgui/classes.h>

static APTR win_menu = 0;
static APTR win_url = 0;

static APTR tvitem, reloaditem, printitem, enterurlitem, favitem, histitem, newtabitem, closetabitem, selecttabitem, onofflineitem;
static APTR screenWidth, screenWidthSwitch, fontitem, fontswitch, edfavitem, dispszitem, pipdispitem, pippositem;
static APTR tvrendering, tvrenderingswitch, tvsize, tvsizeswitch, tvpos, tvposswitch;

static VOID ASM SAVEDS openURLWindow(__reg( a0, struct Hook *hook ), __reg(a2, Object *obj ), __reg(a1, LONG *delta) )
{
	if (win_url) {
		set (win_url, MUIA_Menuwindow_Hide, FALSE);
	}
}

static VOID ASM SAVEDS screenWidthHookFunc(__reg( a0, struct Hook *hook ), __reg(a2, Object *obj ), __reg(a1, LONG *delta) )
{
	ULONG width = 0;
	get  (screenWidthSwitch, MUIA_ErgoCycle_Active, &width);
	switch (width) 
	{
		case 0:
			set(app, MA_App_ScreenWidth, 740);
			break;

		case 1:
		default:
			set(app, MA_App_ScreenWidth, 1024);
			break;
	}
}

static VOID ASM SAVEDS tvrenderingHookFunc(__reg( a0, struct Hook *hook ), __reg(a2, Object *obj ), __reg(a1, LONG *delta) )
{
	ULONG pipmode = 0;
	get  (tvrenderingswitch, MUIA_ErgoCycle_Active, &pipmode);
	set (app, MA_App_Pipmode, pipmode);	
	if (win_menu && pipmode==2) {
		DoMethod (win_menu, MUIM_Window_ToFront);
	}
}

static VOID ASM SAVEDS tvposHookFunc(__reg( a0, struct Hook *hook ), __reg(a2, Object *obj ), __reg(a1, LONG *delta) )
{
	ULONG pippos = 0;
	Object *pipwindow = 0;
	get  (tvposswitch, MUIA_ErgoCycle_Active, &pippos);
	get (app, MA_App_Pipwindow, &pipwindow);
	if(pipwindow)
		set (pipwindow, MA_Pipwindow_Position, pippos);
}

static VOID ASM SAVEDS tvsizeHookFunc(__reg( a0, struct Hook *hook ), __reg(a2, Object *obj ), __reg(a1, LONG *delta) )
{
	ULONG pipsize = 0;
	Object* pipwindow = 0;
	get  (tvsizeswitch, MUIA_ErgoCycle_Active, &pipsize);
	get (app, MA_App_Pipwindow, &pipwindow);
	if(pipwindow)
		set (pipwindow, MA_Pipwindow_Size, pipsize);
}


static struct Hook openURLWindowHook = { {NULL,NULL},(VOID *) openURLWindow, NULL,NULL };

static struct Hook screenWidthHook = { {NULL,NULL},(VOID *) screenWidthHookFunc ,NULL,NULL };
static struct Hook tvrenderingHook = { {NULL,NULL},(VOID *) tvrenderingHookFunc ,NULL,NULL };
static struct Hook tvposHook = { {NULL,NULL},(VOID *) tvposHookFunc ,NULL,NULL };
static struct Hook tvsizeHook = { {NULL,NULL},(VOID *) tvsizeHookFunc ,NULL,NULL };

APTR build_url_window (void)
{
	win_url = URLWindowObject,
		MUIA_Window_ID         , MAKE_ID('U','R','L','.'),
		MUIA_Window_Activate   , FALSE,
	End;

	// Set position after obj creation since menubrowser specifies postion by
	// itself
	if (win_url) {
		set(win_url, MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered);		
		set(win_url, MUIA_Window_Width,  MUIV_Window_Width_Screen(95));
		set(win_url, MUIA_Window_TopEdge, MUIV_Window_TopEdge_Bottom(15));
		
		DoMethod( win_url,
				  MUIM_Notify, MUIA_Menuwindow_HideRequest, TRUE,
				  win_url, 3,
				  MUIM_Set, MUIA_Menuwindow_Hide, TRUE );
	}
	
	return win_url;
}

APTR build_stb_menu( void )
{
	static const STRPTR fontSizes[] =
	{
		"small",
		"medium",
		"large",
		"extra large",
		NULL
	};

	static const STRPTR screenwidths[] =
	{
		"tv screen",
		"extra large",
		NULL
	};
	
	static const STRPTR tvmodes[] =
	{
		"off",
		"audio",
		"picture",
		NULL
	};
	

	static const STRPTR pipSizes[] =
	{
		"small",
		"medium",
		"large",
		NULL
	};

	static const STRPTR pipPositions[] =
	{
		"top left",
		"top middle",
		"top right",
		"center left",
		"center",
		"center right",
		"bottom left",
		"bottom middle",
		"bottom right",
		NULL
	};
	
	win_menu = MenuwindowObject,
		MUIA_Window_RootObject, MenubrowserObject,
		
		MUIA_Menubrowser_MenupageObject, MenupageObject,
			MUIA_Menupage_Title,		"WWW Menu",
			Child, ErgolistObject,

				Child, tvitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Box",
					MUIA_Ergoitem_HelpText, "Switch to fullscreen TV",				
					End,

				Child, reloaditem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Reload page",
		            MUIA_Ergoitem_HelpText, "Load the current web page again",						
					End,

				Child, enterurlitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Goto a web page",
		            MUIA_Ergoitem_HelpText, "Load a web page by entering a URL",		
				End,

				Child, SubmenuitemObject,
					MUIA_Ergoitem_Label, "Goto Bookmark",
		MUIA_Ergoitem_HelpText, "Select a web page from the bookmarks menu",
					MUIA_Submenuitem_Submenu, URLselectObject,
						MUIA_Menupage_Title, "Bookmarks",
						End,
					End,

				/*Child, printitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Print page",
					MUIA_Ergoitem_Disabled, TRUE,
					End,*/

				/*Child, histitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "History",
					MUIA_Ergoitem_Disabled, TRUE,
					End,*/

				Child, newtabitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "New window",
		            MUIA_Ergoitem_HelpText, "Open a new web window",
					End,

				Child, closetabitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Close window",
		            MUIA_Ergoitem_HelpText, "Close current window",		
					End,

				/*Child, selecttabitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Select tab",
					MUIA_Ergoitem_Disabled, TRUE,
					End,*/

				Child, onofflineitem = ErgoitemObject,
					MUIA_Ergoitem_Label, "Go online/offline",
					End,

				End,
			End,

			MUIA_Menubrowser_MenupageObject, MenupageObject,
				MUIA_Menupage_Title, "WWW Options",

				Child, ErgolistObject,

//  					Child, ErgoitemObject,
//  						MUIA_Ergoitem_Label, "Edit Bookmarks",
//  						MUIA_Ergoitem_Disabled, FALSE,
//  						End,
//  
//  					Child, ErgoitemObject,
//  						MUIA_Ergoitem_Label, "Size of Display",
//  						MUIA_Ergoitem_Disabled, FALSE,
//  						End,
		
// 		           Child, fontitem = GadgetitemObject,
// 						MUIA_Ergoitem_Label, "Font size",
// 		                MUIA_Gadgetitem_Gadget, fontswitch =
// 		                    ErgoCycleObject,
// 		                      MUIA_ErgoCycle_Entries, fontSizes,
//                     		End,
// 		           End,

		           Child, screenWidth = GadgetitemObject,
		                MUIA_Ergoitem_DontPress, TRUE,		
						MUIA_Ergoitem_Label, "Screen width",
						MUIA_Ergoitem_HelpText, "Change your default width of web pages",		
		                MUIA_Ergoitem_CloseMenu, MUIV_Ergoitem_CloseMenu_None,
		                MUIA_Gadgetitem_Gadget, screenWidthSwitch =
		                    ErgoCycleObject,
		                      MUIA_ErgoCycle_Entries, screenwidths,
                    		End,
		           End,
		
		           Child, tvrendering = GadgetitemObject,
		                MUIA_Ergoitem_DontPress, TRUE,
						MUIA_Ergoitem_Label, "TV rendering",
						MUIA_Ergoitem_HelpText, "Setup tv rendering",		
		                MUIA_Ergoitem_CloseMenu, MUIV_Ergoitem_CloseMenu_None,
		                MUIA_Gadgetitem_Gadget, tvrenderingswitch =
		                    ErgoCycleObject,
		                      MUIA_ErgoCycle_Entries, tvmodes,
                    		End,
		           End,

		           Child, tvpos  = GadgetitemObject,
		                MUIA_Ergoitem_DontPress, TRUE,
						MUIA_Ergoitem_Label, "TV picture position",
						MUIA_Ergoitem_HelpText, "Setup the position of the tv picture",				
                        MUIA_Ergoitem_CloseMenu, MUIV_Ergoitem_CloseMenu_None,		
		                MUIA_Gadgetitem_Gadget, tvposswitch =
		                    ErgoCycleObject,
		                      MUIA_ErgoCycle_Entries, pipPositions,
                    		End,
		           End,
		
		           Child, tvsize = GadgetitemObject,		
                        MUIA_Ergoitem_DontPress, TRUE,		
						MUIA_Ergoitem_Label, "TV picture size",
						MUIA_Ergoitem_HelpText, "Setup the size of the tv picture",		
                        MUIA_Ergoitem_CloseMenu, MUIV_Ergoitem_CloseMenu_None,		
		                MUIA_Gadgetitem_Gadget, tvsizeswitch =
		                    ErgoCycleObject,
		                      MUIA_ErgoCycle_Entries, pipSizes,
                    		End,		
		           End,
		
//  					Child, ErgoitemObject,
//  						MUIA_Ergoitem_Label, "Edit Bookmarks",
//  						MUIA_Ergoitem_Disabled, TRUE,
//  						End,
//  
//  					Child, ErgoitemObject,
//  						MUIA_Ergoitem_Label, "Size of Display",
//  						MUIA_Ergoitem_Disabled, TRUE,
//  						End,
// 
// 					Child, pipdispitem = ErgoitemObject,
// 						MUIA_Ergoitem_Label, "TV-in-Browser display",
// 						MUIA_Ergoitem_Disabled, TRUE,
// 						End,
// 
// 					Child, pippositem = ErgoitemObject,
// 						MUIA_Ergoitem_Label, "TV-in-Browser position",
// 						MUIA_Ergoitem_Disabled, TRUE,
// 						End,

					/*Child, SubmenuitemObject,
						MUIA_Ergoitem_Label, "Further options",
						MUIA_Submenuitem_Submenu, MenupageObject,
							MUIA_Menupage_Title,		"Browser settings",
							Child, ErgolistObject,
								Child, ErgoitemObject,
									MUIA_Ergoitem_Label, "foo",
									End,
								Child, ErgoitemObject,
									MUIA_Ergoitem_Label, "bar",
									End,
								Child, ErgoitemObject,
									MUIA_Ergoitem_Label, "baz",
									End,
								End,
							End,
						End,*/

					End,
			End,

 		 MUIA_Menubrowser_MenupageObject, ApplicationsMenupageObject,
		 End,
		End,
		End;

	if( win_menu )
	{
		if( tvitem )
			DoMethod( tvitem, MUIM_Notify, MUIA_Ergoitem_Pressed, TRUE, MUIV_Notify_Application, 1, MM_STB_FullScreenTV );

		if( reloaditem )
			DoMethod( reloaditem, MUIM_Notify, MUIA_Ergoitem_Pressed, TRUE, MUIV_Notify_Application, 2, MM_DoLastActiveWin, MM_HTMLWin_Reload );

		if( enterurlitem )
			DoMethod (enterurlitem, MUIM_Notify, MUIA_Ergoitem_Pressed, TRUE, enterurlitem, 3,
					  MUIM_CallHook, &openURLWindowHook, 0);

		if( newtabitem )
			DoMethod( newtabitem, MUIM_Notify, MUIA_Ergoitem_Pressed, TRUE, MUIV_Notify_Application, 2, MM_App_NewWindow, 0 );

		if( closetabitem )
			DoMethod( closetabitem, MUIM_Notify, MUIA_Ergoitem_Pressed, TRUE, MUIV_Notify_Application, 2, MM_DoLastActiveWin, MM_HTMLWin_Close );

		if( onofflineitem )
			DoMethod( onofflineitem, MUIM_Notify, MUIA_Ergoitem_Pressed, TRUE, MUIV_Notify_Application, 1, MM_STB_ToggleOnline );

		if (screenWidthSwitch) 
			DoMethod (screenWidthSwitch,
					  MUIM_Notify,  MUIA_ErgoCycle_Active, MUIV_EveryTime,
					  screenWidth, 3,
					  MUIM_CallHook, &screenWidthHook, 0);
		
		if (tvrenderingswitch) 
			DoMethod (tvrenderingswitch,
					  MUIM_Notify,  MUIA_ErgoCycle_Active, MUIV_EveryTime,
					  tvrendering, 3,
					  MUIM_CallHook, &tvrenderingHook, 0);

		if (tvposswitch)
			DoMethod (tvposswitch,
					  MUIM_Notify,  MUIA_ErgoCycle_Active, MUIV_EveryTime,
					  tvpos, 3,
					  MUIM_CallHook, &tvposHook, 0);

		if (tvsizeswitch) 
			DoMethod (tvsizeswitch,
					  MUIM_Notify,  MUIA_ErgoCycle_Active, MUIV_EveryTime,
					  tvsize,  3,
					  MUIM_CallHook, &tvsizeHook, 0);
		
	//	if( printitem )
	//	if( favitem )
	//	if( histitem )
	//	if( selecttabitem )
	//	if( edfavitem )
	//	if( dispszitem )
	//	if( pipdispitem )
	//	if( pippositem )
		
		DoMethod( win_menu,
				  MUIM_Notify, MUIA_Menuwindow_HideRequest, TRUE,
				  win_menu, 3,
				  MUIM_Set, MUIA_Menuwindow_Hide, TRUE );
	}

	return( win_menu );
}

void show_stb_menu( char *currenturl )
{
	if (win_url && currenturl && strcmp(currenturl,"")!=0)
		set(win_url, MUIA_URLWindow_URL, currenturl);
	
	if( win_menu )
	{
		if( closetabitem )
			set( closetabitem, MUIA_Ergoitem_Disabled, veryfirstwin == lastactivewin );

		if (screenWidthSwitch) {
			ULONG screenWidth = 0;
			get (app, MA_App_ScreenWidth, &screenWidth);
			switch (screenWidth) {
			case 1024:
				set(screenWidthSwitch, MUIA_ErgoCycle_Active, 1);
				break;
			case 740:
			default:
				set(screenWidthSwitch, MUIA_ErgoCycle_Active, 0);
				break;				
			}
		}
		
		if (tvrenderingswitch) {
			ULONG pipmode = 0;
			get (app, MA_App_Pipmode, &pipmode);
			set (tvrenderingswitch, MUIA_ErgoCycle_Active, pipmode);
		}

		if (tvposswitch) {
			Object *pipwindow = 0;
			ULONG pippos = 0;
			get (app, MA_App_Pipwindow, &pipwindow);
			if (pipwindow) {
				get (pipwindow, MA_Pipwindow_Position, &pippos);
				set (tvposswitch,  MUIA_ErgoCycle_Active, pippos);
			}
		}

		if (tvsizeswitch) {
			Object *pipwindow = 0;
			ULONG pipsize = 0;
			get (app, MA_App_Pipwindow, &pipwindow);
			if (pipwindow) {
				get (pipwindow, MA_Pipwindow_Size, &pipsize);
				set (tvsizeswitch,  MUIA_ErgoCycle_Active, pipsize);
			}			
		}

		{
			ULONG pipEnabled = 0;
			get (app, MA_App_PipEnabled, &pipEnabled);

			if (tvrendering)
				set (tvrendering, MUIA_Ergoitem_Disabled, !pipEnabled);
			if (tvpos)
				set (tvpos, MUIA_Ergoitem_Disabled, !pipEnabled);
			if (tvsize)
				set (tvsize, MUIA_Ergoitem_Disabled, !pipEnabled);				
		}
		
		DoMethod (win_menu, MUIM_Menuwindow_ShowMenu);
	}
}

#endif
