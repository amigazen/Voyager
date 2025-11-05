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
 * PrefsWin window class
 * ---------------------
 * - Handles the preference window of Voyager
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_main.c,v 1.53 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* private */
#include "prefswin.h"
#include "mybrush.h"
#include "network.h"
#include "htmlclasses.h"
#include "mui_func.h"
#include "menus.h"
#include <proto/vimgdecode.h>
#include "autoproxy.h"
#include "htmlwin.h"
#include "prefsimages_protos.h"
#include "imgstub.h"
#include "win_func.h"
#include "textinput.h"
#include "popph.h"


static int lastactivepage;


/*
 * Common functions
 */
void storestring( APTR strobj, ULONG pid )
{
	setprefsstr_clone( pid, getstrp( strobj ) );
}

void storeattr( APTR obj, ULONG attr, ULONG pid )
{
	setprefslong_clone( pid, getv( obj, attr ) );
}

APTR pstring( ULONG pid, int maxlen, char *label )
{
	APTR o;

	o = TextinputObject, StringFrame, MUIA_String_MaxLen, maxlen,
		MUIA_String_Contents, getprefsstr( pid, "" ),
		MUIA_CycleChain, 1,
		MUIA_ControlChar, ParseHotKey( label ),
	End;
	return( o );
}

#if USE_POPPH
APTR ppopph( ULONG pid, int maxlen, char **array )
{
	APTR o;

	o = PopplaceholderObject,
		MUIA_Popph_StringMaxLen, maxlen,
		MUIA_Popph_Contents, getprefsstr( pid, "" ),
		MUIA_Popph_Array, array,
	End;
	return( o );
}
#endif

APTR pinteger( ULONG pid, char * label )
{
	APTR o;

	o = TextinputObject, StringFrame, 
		MUIA_String_Integer, getprefslong( pid, 0 ),
		MUIA_CycleChain, 1,
		MUIA_String_Accept, "0123456789",
		MUIA_ControlChar, ParseHotKey( label ),
	End;
	return( o );
}

APTR pcycle( STRPTR *opts, ULONG pid, char *label )
{
	APTR o = MUI_MakeObject( MUIO_Cycle, 0, ( ULONG )opts );
	SetAttrs( o,
		MUIA_Cycle_Active, getprefslong( pid, 0 ),
		MUIA_CycleChain, TRUE,
		MUIA_ControlChar, ParseHotKey( label ),
	   TAG_DONE
	);
	return( o );
}

APTR pcheck( ULONG pid, char *label )
{
	APTR o = MUI_MakeObject( MUIO_Checkmark, 0 );

	set( o, MUIA_Selected, getprefslong( pid, FALSE ) );
	SetAttrs( o,
		MUIA_CycleChain, TRUE,
		MUIA_ControlChar, ParseHotKey( label ),
		TAG_DONE
	);
	return( o );
}

APTR pchecki( ULONG pid, char *label )
{
	APTR o = MUI_MakeObject( MUIO_Checkmark, 0 );

	set( o, MUIA_Selected, getprefslong( pid, TRUE ) );
	SetAttrs( o,
		MUIA_CycleChain , TRUE,
		MUIA_ControlChar, ParseHotKey( label ),
		TAG_DONE
	);
	return( o );
}

void fillstrarray( STRPTR *to, ULONG msg, int num )
{
	while( num-- )
	{
		*to++ = GSI( msg++ );
	}
}

/* tell V it needs to update its display after closing the prefs window */
int needdisplayupdate;
void setupd( APTR o, ULONG attr )
{
	DoMethod( o, MUIM_Notify, attr, MUIV_EveryTime,
		notify, 3, MUIM_WriteLong, TRUE, &needdisplayupdate
	);
}


APTR prefswindow;

/*
 * Structures ( don't forget to change PREFSWIN_NUMPAGES in prefswin.h )
 */

struct Data {
	int oldactive;
	APTR lv_sel;
	APTR prefscontainer;
	APTR prefscontents1;
	APTR prefscontents;
	APTR prefscontents2;
};

struct prefsgroup prefsgroups[ PREFSWIN_NUMPAGES ] = {
	{ MSG_PREFSWIN_TAB1, getprefswin_generalclass, &generalBitMap },
	{ MSG_PREFSWIN_TAB14, getprefswin_toolbarclass, &toolbarBitMap },
	{ MSG_PREFSWIN_TAB13, getprefswin_hyperlinksclass, &hyperlinksBitMap },
	{ MSG_PREFSWIN_TAB3, getprefswin_fontsclass, &fontsBitMap },
	{ MSG_PREFSWIN_TAB2, getprefswin_colorsclass, &colorsBitMap },
	{ MSG_PREFSWIN_TAB10, getprefswin_imagesclass, &imagesBitMap },
#if USE_NET
	{ MSG_PREFSWIN_TAB7, getprefswin_fastlinksclass, &fastlinksBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB4, getprefswin_networkclass, &networkBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB5, getprefswin_cacheclass, &cacheBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB8, getprefswin_securityclass, &securityBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB11, getprefswin_certsclass, &certsBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB9, getprefswin_mailnewsclass, &mailnewsBitMap },
#endif /* USE_NET */
	{ MSG_PREFSWIN_TAB12, getprefswin_javascriptclass, &javascriptBitMap },
#if USE_NET
	{ MSG_PREFSWIN_TAB15, getprefswin_languagesclass, &languagesBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB16, getprefswin_spoofclass, &spoofBitMap },
#endif /* USE_NET */
#if USE_NET
	{ MSG_PREFSWIN_TAB17, getprefswin_downloadclass, &downloadBitMap },
#endif /* USE_NET */
#ifndef MBX
	{ MSG_PREFSWIN_TAB18, getprefswin_contextmenuclass, &contextmenuBitMap },
#endif
};

DECNEW
{
	struct Data *data;
	APTR lv_sel, prefscontainer, prefscontents1, prefscontents2;
	APTR prefscontents;
	APTR bt_ok, bt_cancel;
	int c;
	struct prefsgroup *pg = prefsgroups;

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('P','R','E','F'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, GS( PREFSWIN_TITLE ),
		MUIA_Window_NoMenus, TRUE,
		MUIA_Window_NeedsMouseObject, TRUE,
		MUIA_Window_RootObject, VGroup,

			Child, HGroup,
	            Child, lv_sel = ListviewObject, MUIA_CycleChain, 1, MUIA_Weight, 0,
					MUIA_Listview_List, NewObject( getprefswin_listclass(), NULL, TAG_DONE ),
				End,
				
				Child, prefscontainer = VGroup, GroupFrame, MUIA_Background, MUII_PageBack, MUIA_Group_Spacing, 0,
					Child, prefscontents1 = RectangleObject, MUIA_Weight, 1, End,
					Child, prefscontents  = HVSpace,
					Child, prefscontents2 = RectangleObject, MUIA_Weight, 1, End,
				End,
			End,

			Child, HGroup,
				Child, bt_ok     = button( MSG_OK, 0 ),
				Child, bt_cancel = button( MSG_CANCEL, 0 ),
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->oldactive = MUIV_List_Active_Off;
	data->lv_sel = lv_sel;
	data->prefscontainer = prefscontainer;
	data->prefscontents1 = prefscontents1;
	data->prefscontents = prefscontents;
	data->prefscontents2 = prefscontents2;

	needdisplayupdate = 0;
	initprefsclone();

	if( !getprefs( DSI_BUTTON_NUM ) )
	{
		for( num_buttons = 0; getprefs( DSI_BUTTONS_LABELS + num_buttons ); num_buttons++ );
	}
	else
	{
		num_buttons = getprefslong( DSI_BUTTON_NUM, 0 );
	}

	// insert labels
	for( c = 0; c < PREFSWIN_NUMPAGES; c++, pg++ )
	{
		DoMethod( lv_sel, MUIM_List_InsertSingle, GSI( pg->labelid ), MUIV_List_Insert_Bottom );
	}
	
	DoMethod( lv_sel, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Prefswin_Main_SelectChange
		//obj, 1, MM_Prefswin_Main_SelectChange
	);
 
	set( lv_sel, MUIA_List_Active, lastactivepage );

	set( obj, MUIA_Window_ActiveObject, lv_sel );
	set( obj, MUIA_Window_DefaultObject, lv_sel );

	DoMethod( bt_ok, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 5, MUIM_Application_PushMethod, obj, 2, MM_Prefswin_Main_Close, TRUE
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 5, MUIM_Application_PushMethod, obj, 2, MM_Prefswin_Main_Close, FALSE
	);
	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 5, MUIM_Application_PushMethod, obj, 2, MM_Prefswin_Main_Close, FALSE
	);

	return( ( ULONG )obj );
}


DECMETHOD( Prefswin_Main_SelectChange, APTR )
{
	GETDATA;

	if( DoMethod( data->prefscontainer, MUIM_Group_InitChange ) )
	{
		D( db_gui, bug( "removing prefscontents\n" ) );
		DoMethod( data->prefscontainer, OM_REMMEMBER, data->prefscontents );

		D( db_gui, bug( "disposing object\n" ) );
		MUI_DisposeObject( data->prefscontents );

		D( db_gui, bug( "object disposed OK\n" ) );

		data->prefscontents = NULL;

		/*
		 * Creating new object
		 */
		get( data->lv_sel, MUIA_List_Active, &data->oldactive );
		if( data->oldactive >= 0 )
		{
			D( db_gui, bug( "about to create the object..\n" ) );
			data->prefscontents = NewObject( prefsgroups[ data->oldactive ].class(), NULL, TAG_DONE );
			D( db_gui, bug( "object created successfully\n" ) );
		}

		if( data->prefscontents )
		{
			DoMethod( data->prefscontainer, OM_ADDMEMBER, data->prefscontents );
			DoMethod( data->prefscontainer, MUIM_Group_Sort, data->prefscontents1, data->prefscontents, data->prefscontents2, NULL );
		}
		else
		{
			data->prefscontents = VCenter( ( TextObject, TextFrame, MUIA_Text_Contents, "SEVERE INTERNAL ERROR WHILE\nSETTING UP PREFS WINDOW!", End ) );
		}
		
		D( db_gui, bug( "ExitChange...\n" ) );
		DoMethod( data->prefscontainer, MUIM_Group_ExitChange );
		D( db_gui, bug( "object added, leaving with ExitChange\n" ) );
	}
	else
	{
		displaybeep();
	}

	return( 0 );
}


DECSMETHOD( Prefswin_Main_Close )
{
	GETDATA;

	lastactivepage = data->oldactive;
	set( obj, MUIA_Window_Open, FALSE );

	if( DoMethod( data->prefscontainer, MUIM_Group_InitChange ) )
	{
		DoMethod( data->prefscontainer, OM_REMMEMBER, data->prefscontents2 );
		DoMethod( data->prefscontainer, OM_REMMEMBER, data->prefscontents );
		DoMethod( data->prefscontainer, OM_REMMEMBER, data->prefscontents1 );
		MUI_DisposeObject( data->prefscontents );
		DoMethod( data->prefscontainer, MUIM_Group_ExitChange );
	}

	setprefslong( DSI_BUTTON_NUM, num_buttons );

	freeprefsclone( msg->use );
	DoMethod( app, OM_REMMEMBER, obj );
	MUI_DisposeObject( obj );
	prefswindow = NULL;

	set_prefs_globals();

	if (menu && findmenu(MENU_SET_LOAD  )) set( findmenu( MENU_SET_LOAD  ), MUIA_Menuitem_Enabled, TRUE );
	if (menu && findmenu(MENU_SET_SAVE  )) set( findmenu( MENU_SET_SAVE  ), MUIA_Menuitem_Enabled, TRUE );
	if (menu && findmenu(MENU_SET_SAVEAS)) set( findmenu( MENU_SET_SAVEAS), MUIA_Menuitem_Enabled, TRUE );

#if USE_NET
	cleanupcerts( msg->use );
#endif /* USE_NET */

	if( msg->use ) // OK button pressed
	{
		imgdec_storeprefs();

		// Reload proxy settings
#if USE_NET
		proxy_init();
#endif /* USE_NET */

		setup_useragent();

		// Offline menu
		if (menu && findmenu(MENU_SET_OFFLINEMODE)) set( findmenu( MENU_SET_OFFLINEMODE ), MUIA_Menuitem_Enabled, getprefslong( DSI_NET_CHECKIFONLINE, FALSE ) ? FALSE : TRUE );

		if( needdisplayupdate )
			set( app, MUIA_Application_Iconified, TRUE );

		doallwins( MM_HTMLWin_SetupToolbar );
#if USE_NET
		doallwins( MM_HTMLWin_SetupFastlinks );
#endif /* USE_NET */
		doallwins( MM_HTMLWin_SetupIcon );
		doallwins( MM_HTMLWin_UpdateScrollersClock );
#if USE_NET
		DoMethod( app, MM_App_UpdateSpoofMenu );
#endif /* USE_NET */
		/*DoMethod( app, MM_DoAllWins, MM_Win_CheckWinScrollbars );*/

		if( needdisplayupdate )
			set( app, MUIA_Application_Iconified, FALSE );
	}
	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFMETHOD( Prefswin_Main_SelectChange )
DEFSMETHOD( Prefswin_Main_Close )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_mainclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_MainClass";
#endif
	return( TRUE );
}

void delete_prefswin_mainclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_mainclass( void )
{
	return( mcc->mcc_Class );
}


