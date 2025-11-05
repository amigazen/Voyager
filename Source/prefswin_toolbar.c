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
 * Preference toolbar class
 * ------------------------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_toolbar.c,v 1.32 2003/08/31 15:15:23 zapek Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#endif

/* private */
#include "classes.h"
#include "prefswin.h"
#include "mybrush.h"
#include "mui_func.h"
#if USE_POPHOTKEY
#include <mui/pophotkey_mcc.h>
#endif
#include "methodstack.h"
#include "speedbar.h"


/*
 * Direction when moving buttons
 */
#define MV_LEFT 0
#define MV_RIGHT 1

int num_buttons;

struct Data {
	int init_done;
	int active_num;
	int previous_active_num;
	int active_tb;
	APTR grp_vbut;
	APTR grp_but;
	APTR chk_tb_borderless;
	APTR chk_tb_raised;
	APTR chk_tb_sunny;
	APTR chk_tb_small;
	APTR cyc_tbmode;
	APTR pop_cmd;
	APTR bt_tb_add;
	APTR bt_tb_addspace;
	APTR bt_tb_remove;
	APTR bt_tb_left;
	APTR bt_tb_right;
	APTR str_tb_icon;
	APTR str_tb_hotkey;
	APTR str_tb_label;
};


APTR hotkeystring( void )
{
#if USE_POPHOTKEY
	APTR o;

	o = PophotkeyObject,
		MUIA_CycleChain, 1,
		MUIA_String_MaxLen, 80,
	End;

	if( o )
	{
		int v = 0;
		get( o, MUIA_Version, &v );
		if( v < 17 )
		{
			MUI_DisposeObject( o );
			o = NULL;
		}
	}

	if( !o )
		o = string( "", 80, 0 );

	return( o );
#else
	return NULL; //TOFIX!!  We're not using pophotkey maybe, but we should be using something
#endif
}

#if USE_DOS
#define DOS1 ASLFR_RejectIcons, TRUE,
#else
#define DOS1
#endif

#if USE_REXX
#define REXX1 Child, Label2( GS( TB_ACT_LABEL ) ),
#define REXX2 Child, pop_cmd = NewObject( getcommandclass(), NULL, TAG_DONE ),
#else
#define REXX1
#define REXX2 
#endif

DECNEW
{
	APTR cyc_tbmode;
	APTR chk_tb_sunny, chk_tb_raised, chk_tb_borderless, chk_tb_small;
	APTR grp_but, grp_vbut, bt_tb_add, bt_tb_addspace, bt_tb_remove;
	APTR str_tb_icon, str_tb_hotkey, str_tb_label;
	APTR bt_tb_left, bt_tb_right;
#if USE_REXX
	APTR pop_cmd;
#else
	APTR pop_cmd = NULL;
#endif
	struct Data *data;

	static STRPTR tb_opts[ 4 ];
	static STRPTR tb_acts[ 12 ];

	fillstrarray( tb_opts, MSG_PREFSWIN_STYLE_TB_1, 3 );
	fillstrarray( tb_acts, MSG_TB_ACT_0, 11 );
 
	D( db_gui, bug( "building prefswin_toolbar object..\n" ) );

	obj = DoSuperNew( cl, obj,

		Child, VGroup, GroupFrameT( GS( PREFSWIN_STYLE_L1 ) ),

			Child, HGroup,
				Child, Label2( GS( PREFSWIN_STYLE_TB ) ),
				Child, cyc_tbmode = pcycle( tb_opts, DSI_FLAGS + VFLG_ACTIONBUT_MODES, GS( PREFSWIN_STYLE_TB ) ),
				Child, RectangleObject, MUIA_Weight, 1, End,
			End,
			Child, HGroup,
				Child, Label1( GS( PREFSWIN_TB_SUNNY ) ),
				Child, chk_tb_sunny = pcheck( DSI_BUTTON_STYLE_SUNNY, GS( PREFSWIN_TB_SUNNY ) ),
				Child, Label1( GS( PREFSWIN_TB_RAISED ) ),
				Child, chk_tb_raised = pcheck( DSI_BUTTON_STYLE_RAISED, GS( PREFSWIN_TB_RAISED ) ),
				Child, Label1( GS( PREFSWIN_TB_BORDERLESS ) ),
				Child, chk_tb_borderless = pcheck( DSI_BUTTON_STYLE_BORDERLESS, GS( PREFSWIN_TB_BORDERLESS ) ),
				Child, Label1( GS( PREFSWIN_TB_SMALL ) ),
				Child, chk_tb_small = pcheck( DSI_BUTTON_STYLE_SMALL, GS( PREFSWIN_TB_SMALL ) ),
				Child, RectangleObject, MUIA_Weight, 1, End,
			End,
		End,

		Child, HGroup,

			Child, ScrollgroupObject,
				MUIA_Scrollgroup_FreeHoriz, TRUE,
				MUIA_Scrollgroup_FreeVert, FALSE,
				MUIA_Scrollgroup_Contents, grp_vbut = NewObject( gettoolbarclass(), NULL,
					VirtualFrame,
					Child, VSpace( 0 ),
					Child, grp_but = HGroup,
						Child, HSpace( 0 ),
					End,
					Child, VSpace( 0 ),
				End,
				MUIA_Scrollgroup_AutoBars, TRUE,
			End,

			Child, VGroup, MUIA_Weight, 0, MUIA_Group_Spacing, 0,
				Child, bt_tb_add = button( MSG_TB_BT_ADD, 0 ),
				Child, bt_tb_addspace = button( MSG_TB_BT_ADDSPACE, 0 ),
				Child, bt_tb_remove = button( MSG_TB_BT_REMOVE, 0 ),
				Child, HGroup, MUIA_Group_Spacing, 0,
					Child, bt_tb_left = MUI_MakeObject( MUIO_Button, (ULONG)"\033I[6:13]" ),
					Child, bt_tb_right = MUI_MakeObject( MUIO_Button, (ULONG)"\033I[6:14]" ),
				End,
			End,

		End,

		Child, ColGroup( 2 ),

			Child, Label2( GS( TB_L_ICON ) ),
			Child, PopaslObject, MUIA_CycleChain, 1,
				MUIA_Popstring_String, str_tb_icon = string( "", 256, 0 ),
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),
				DOS1
			End,

			Child, Label2( GS( TB_L_LABEL ) ),
			Child, HGroup,
				Child, str_tb_label = string( "", 32, 0 ),
				Child, Label2( GS( TB_L_HOTKEY ) ),
				Child, str_tb_hotkey = hotkeystring(),
			End,

			REXX1
			REXX2
		
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	data->grp_vbut = grp_vbut;
	data->grp_but = grp_but;
	data->chk_tb_borderless = chk_tb_borderless;
	data->chk_tb_raised = chk_tb_raised;
	data->chk_tb_sunny = chk_tb_sunny;
	data->chk_tb_small = chk_tb_small;
	data->cyc_tbmode = cyc_tbmode;
	data->pop_cmd = pop_cmd;
	data->bt_tb_add = bt_tb_add;
	data->bt_tb_addspace = bt_tb_addspace;
	data->bt_tb_remove = bt_tb_remove;
	data->bt_tb_left = bt_tb_left;
	data->bt_tb_right = bt_tb_right;
	data->str_tb_icon = str_tb_icon;
	data->str_tb_hotkey = str_tb_hotkey;
	data->str_tb_label = str_tb_label;


	setupd( cyc_tbmode, MUIA_Cycle_Active );
	setupd( chk_tb_sunny, MUIA_Selected );
	setupd( chk_tb_raised, MUIA_Selected );
	setupd( chk_tb_borderless, MUIA_Selected );
	setupd( chk_tb_small, MUIA_Selected );

	set( cyc_tbmode, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_TBMODE ) );
	set( chk_tb_sunny, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_TB_SUNNY ) );
	set( chk_tb_raised, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_TB_RAISED ) );
	set( chk_tb_borderless, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_TB_BORDERLESS ) );
	set( chk_tb_small, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_TB_SMALL ) );
	set( bt_tb_add, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_TB_ADD ) );
	set( bt_tb_addspace, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_TB_ADDSPACE ) );
	set( bt_tb_remove, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_TB_REMOVE ) );
	set( str_tb_icon, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_TB_ICON ) );
	set( str_tb_label, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_TB_LABEL ) );
	set( str_tb_hotkey, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_TB_HOTKEY ) );
	//set( cyc_tb_act, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_TB_ACT ) ); //TOFIX !! sux !
	//set( str_tb_cmd, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_TB_CMD ) );  //TOFIX!!
 
	/*
	 * Visual SpeedBar options requiring button rebuilds
	 */
	DoMethod( chk_tb_sunny, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);
	DoMethod( chk_tb_borderless, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);
	DoMethod( chk_tb_raised, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);
	DoMethod( chk_tb_small, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);
	DoMethod( cyc_tbmode, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);

	/*
	 * Toolbar editing actions
	 */
	DoMethod( bt_tb_add, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Toolbar_AddButton
	);

	DoMethod( bt_tb_left, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MM_Prefswin_Toolbar_MoveButton, MV_LEFT
	);
	
	DoMethod( bt_tb_right, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MM_Prefswin_Toolbar_MoveButton, MV_RIGHT
	);
	
	DoMethod( bt_tb_addspace, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Toolbar_AddSpacer
	);
	
	DoMethod( bt_tb_remove, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Toolbar_RemoveButton
	);

	/*
	 * Setting a new label / icon must take effect immediately
	 */
	//TOFIX!! should be smarter.. String_Contents perhaps ?
	DoMethod( str_tb_label, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);

	DoMethod( str_tb_icon, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Toolbar_BuildButtons
	);

	DoMethod( obj, MM_Prefswin_Toolbar_BuildButtons ); 

	return( ( ULONG )obj );
}


/*
 * Stores the current fields into the prefsclone
 */
DECDISPOSE
{
	GETDATA;

	D( db_gui, bug( "starting...\n" ) );

	DoMethod( obj, MM_Prefswin_Toolbar_FixFields );

	storeattr( data->cyc_tbmode, MUIA_Cycle_Active, DSI_FLAGS + VFLG_ACTIONBUT_MODES );
	storeattr( data->chk_tb_sunny, MUIA_Selected, DSI_BUTTON_STYLE_SUNNY );
	storeattr( data->chk_tb_raised, MUIA_Selected, DSI_BUTTON_STYLE_RAISED );
	storeattr( data->chk_tb_borderless, MUIA_Selected, DSI_BUTTON_STYLE_BORDERLESS );
	storeattr( data->chk_tb_small, MUIA_Selected, DSI_BUTTON_STYLE_SMALL );

	D( db_gui, bug( "attributes stored in the cloneprefs\n" ) );

	return( DOSUPER );
}


/*
 * Builds the Prefs toolbar (all the buttons).
 */
DECMETHOD( Prefswin_Toolbar_BuildButtons, ULONG )
{
	GETDATA;

	int c;
	struct MinList *l;
	APTR o, ostate;

	D( db_gui, bug( "starting..\n" ) );

	set( app, MUIA_Application_Sleep, TRUE );

	DoMethod( data->grp_vbut, MUIM_Group_InitChange );
	DoMethod( data->grp_but, MUIM_Group_InitChange );

	/*
	 * Remove first.
	 */
	get( data->grp_but, MUIA_Group_ChildList, &l );
	ostate = FIRSTNODE( l );
	while( o = NextObject( &ostate ) )
	{
		DoMethod( data->grp_but, OM_REMMEMBER, o );
		MUI_DisposeObject( o );
	}

	/*
	 * Then rebuild.
	 */
	for( c = 0; getprefs( DSI_BUTTONS_LABELS + c ) && c < num_buttons; c++ )
	{
#if USE_SPEEDBAR
		APTR o = ( APTR )DoMethod( obj, MM_Prefswin_Toolbar_CreateBar, c  );
#endif
		if( o )
		{
			DoMethod( data->grp_but, OM_ADDMEMBER, o );

			if( c == data->active_num )
				set( o, MUIA_Selected, TRUE );
				//pushmethod( o, 3, MUIM_Set, MUIA_Selected, TRUE );
		}
	}
	DoMethod( data->grp_but, MUIM_Group_ExitChange );
	DoMethod( data->grp_vbut, MUIM_Group_ExitChange );

	set( app, MUIA_Application_Sleep, FALSE );
	
	return( 0 );
}


/*
 * Create one button or spacer
 */
DECSMETHOD( Prefswin_Toolbar_CreateButton )
{
#if USE_SPEEDBAR
	GETDATA;
	APTR o;
#endif

	D( db_gui, bug( "starting..\n" ) );

	setprefsstr( DSI_BUTTONS_SHORTCUT + msg->cnt, "" );
	setprefsstr( DSI_BUTTONS_ARGS + msg->cnt, "" );

#if USE_SPEEDBAR
	o = ( APTR )DoMethod( obj, MM_Prefswin_Toolbar_CreateBar, msg->cnt );
	if( o )
	{
		DoMethod( data->grp_vbut, MUIM_Group_InitChange );
		DoMethod( data->grp_but, MUIM_Group_InitChange );
		DoMethod( data->grp_but, OM_ADDMEMBER, o );
		DoMethod( data->grp_but, MUIM_Group_ExitChange );
		DoMethod( data->grp_vbut, MUIM_Group_ExitChange );
		//set( o, MUIA_Selected, TRUE ); //TOFIX!! check!
		DoMethod( data->grp_but, MUIM_Virtgroup_MakeVisible, o, 0 );
		num_buttons++;
	}
#endif
	return( 0 );
}


/*
 * Create a button and setup the notifies
 */
DECSMETHOD( Prefswin_Toolbar_CreateBar )
{
#if USE_SPEEDBAR

	GETDATA;
	APTR o;

	D( db_gui, bug( "starting..\n" ) );

	o = SpeedButtonObject,
		MUIA_SpeedButton_Label, getprefsstr( DSI_BUTTONS_LABELS + msg->index, "- New -" ),
		MUIA_SpeedButton_Image, b_load( getprefsstr( DSI_BUTTONS_IMAGES + msg->index, "" ) ),
		MUIA_SpeedButton_ImmediateMode, TRUE,
		MUIA_SpeedButton_Borderless, getv( data->chk_tb_borderless, MUIA_Selected ),
		MUIA_SpeedButton_Raising, getv( data->chk_tb_raised, MUIA_Selected ),
		MUIA_SpeedButton_Sunny, getv( data->chk_tb_sunny, MUIA_Selected ),
		MUIA_SpeedButton_SmallImage, getv( data->chk_tb_small, MUIA_Selected ),
		MUIA_SpeedButton_ViewMode, getv( data->cyc_tbmode, MUIA_Cycle_Active ),
		MUIA_UserData, msg->index,
		MUIA_Draggable, TRUE,
	End;

	if( o )
	{
		DoMethod( o, MUIM_Notify, MUIA_Selected, TRUE,
			notify, 3, MUIM_WriteLong, o, &data->active_tb
		);
		DoMethod( o, MUIM_Notify, MUIA_Selected, TRUE,
			data->grp_but, 3, MUIM_NoNotifySet, MUIA_Selected, FALSE
		);
//		  DoMethod( o, MUIM_Notify, MUIA_Selected, TRUE,
//			  o, 3, MUIM_NoNotifySet, MUIA_Selected, TRUE
//		  );
		DoMethod( o, MUIM_Notify, MUIA_Selected, TRUE,
			notify, 3, MUIM_WriteLong, msg->index, &data->active_num
		);
		DoMethod( o, MUIM_Notify, MUIA_Selected, TRUE,
			obj, 1, MM_Prefswin_Toolbar_DisplayFields
		);
	}
	return( ( ULONG )o );
#else
	return( 0 ); //TOFIX!!
#endif /* !USE_SPEEDBAR */
}


DECMETHOD( Prefswin_Toolbar_AddButton, ULONG )
{
	D( db_gui, bug( "adding button\n" ) );

	setprefsstr( DSI_BUTTONS_IMAGES + num_buttons, "Buttons/" );
	setprefsstr( DSI_BUTTONS_LABELS + num_buttons, "- New -" );
	setprefslong( DSI_BUTTONS_ACTION + num_buttons, BFUNC_COMMAND );
	setprefsstr( DSI_BUTTONS_ARGS + num_buttons, "" );

	DoMethod( obj, MM_Prefswin_Toolbar_CreateButton, num_buttons );

	return( 0 );
}


DECSMETHOD( Prefswin_Toolbar_MoveButton )
{
	GETDATA;

	D( db_gui, bug( "moving button to %s\n", msg->direction == MV_LEFT ? "the left" : "the right" ) );

	if( msg->direction == MV_LEFT )
	{
		data->active_num--;
	}

	exchangeprefs_clone( DSI_BUTTONS_IMAGES + data->active_num, DSI_BUTTONS_IMAGES + data->active_num + 1 );
	exchangeprefs_clone( DSI_BUTTONS_LABELS + data->active_num, DSI_BUTTONS_LABELS + data->active_num + 1 );
	exchangeprefs_clone( DSI_BUTTONS_SHORTCUT + data->active_num, DSI_BUTTONS_SHORTCUT + data->active_num + 1 );
	exchangeprefs_clone( DSI_BUTTONS_ARGS + data->active_num, DSI_BUTTONS_ARGS + data->active_num + 1 );
	exchangeprefs_clone( DSI_BUTTONS_ACTION + data->active_num, DSI_BUTTONS_ACTION + data->active_num + 1 );

	if( msg->direction == MV_RIGHT )
	{
		data->active_num++;
	}

	DoMethod( obj, MM_Prefswin_Toolbar_BuildButtons );

	return( 0 );
}


DECMETHOD( Prefswin_Toolbar_AddSpacer, ULONG )
{
	D( db_gui, bug( "adding spacer\n" ) );

	setprefsstr( DSI_BUTTONS_IMAGES + num_buttons, "" );
	setprefsstr( DSI_BUTTONS_LABELS + num_buttons, "" );
	setprefslong( DSI_BUTTONS_ACTION + num_buttons, BFUNC_SEP );
	setprefsstr( DSI_BUTTONS_ARGS + num_buttons, "" );

	DoMethod( obj, MM_Prefswin_Toolbar_CreateButton, num_buttons );
	
	return( 0 );
}


DECMETHOD( Prefswin_Toolbar_RemoveButton, ULONG )
{
	GETDATA;

	D( db_gui, bug( "removing button\n" ) );

	if( data->active_tb )
	{
		int c;

		for( c = data->active_num; c < num_buttons - 1; c++ )
		{
			setprefsstr( DSI_BUTTONS_IMAGES + c, getprefsstr( DSI_BUTTONS_IMAGES + c + 1, "" ) );
			setprefsstr( DSI_BUTTONS_SHORTCUT + c, getprefsstr( DSI_BUTTONS_SHORTCUT + c + 1, "" ) );
			setprefsstr( DSI_BUTTONS_LABELS + c, getprefsstr( DSI_BUTTONS_LABELS + c + 1, "" ) );
			setprefsstr( DSI_BUTTONS_ARGS + c, getprefsstr( DSI_BUTTONS_ARGS + c + 1, "" ) );
			setprefslong( DSI_BUTTONS_ACTION + c, getprefslong( DSI_BUTTONS_ACTION + c + 1, BFUNC_SEP ) );
		}

		if( num_buttons )
			num_buttons--;

		data->active_tb = 0;
		data->active_num = -1;

		DoMethod( obj, MM_Prefswin_Toolbar_BuildButtons );
	
		DoMethod( obj, MM_Prefswin_Toolbar_DisplayFields);
	}

	return( 0 );
}


/*
 * Save the state into the prefsclone
 */
DECMETHOD( Prefswin_Toolbar_FixFields, ULONG )
{
	GETDATA;
	
	D( db_gui, bug( "fixing fields (images, shortcut, labels, args, action) into prefsclone\n" ) );

	if( data->active_tb )
	{
		setprefsstr( DSI_BUTTONS_IMAGES + data->previous_active_num, getstrp( data->str_tb_icon ) );
		setprefsstr( DSI_BUTTONS_SHORTCUT + data->previous_active_num, getstrp( data->str_tb_hotkey ) );
		setprefsstr( DSI_BUTTONS_LABELS + data->previous_active_num, getstrp( data->str_tb_label ) );
		setprefsstr( DSI_BUTTONS_ARGS + data->previous_active_num, ( STRPTR )getv( data->pop_cmd, MA_Command_String ) );
		setprefslong( DSI_BUTTONS_ACTION + data->previous_active_num, getv( data->pop_cmd, MA_Command_Mode ) );
	}
	
	data->previous_active_num = data->active_num;
	
	return( 0 );
}


/*
 * Display a new button's settings
 */
DECMETHOD( Prefswin_Toolbar_DisplayFields, ULONG )
{
	GETDATA;
	
	D( db_gui, bug( "display the fields..\n" ) );

	if( data->init_done )
	{
		DoMethod( obj, MM_Prefswin_Toolbar_FixFields ); /* we copy the current fields first */
	}
	else
	{
		/*
		 * We don't FixFields on the first run otherwise
		 * we would end up fixing empty fields.
		 */
		data->init_done = TRUE;
	}

	if( data->active_tb ) /* then we switch */
	{
		nnset( data->str_tb_icon, MUIA_String_Contents, getprefsstr( DSI_BUTTONS_IMAGES + data->active_num, "" ) );
		nnset( data->str_tb_hotkey, MUIA_String_Contents, getprefsstr( DSI_BUTTONS_SHORTCUT + data->active_num, "" ) );
		nnset( data->str_tb_label, MUIA_String_Contents, getprefsstr( DSI_BUTTONS_LABELS + data->active_num, "" ) );
		nnset( data->pop_cmd, MA_Command_Mode, getprefslong( DSI_BUTTONS_ACTION + data->active_num, 0 ) );
		nnset( data->pop_cmd, MA_Command_String, getprefsstr( DSI_BUTTONS_ARGS + data->active_num, "" ) );
	}

	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Prefswin_Toolbar_BuildButtons )
DEFMETHOD( Prefswin_Toolbar_CreateButton )
DEFSMETHOD( Prefswin_Toolbar_CreateBar )
DEFMETHOD( Prefswin_Toolbar_AddButton )
DEFSMETHOD( Prefswin_Toolbar_MoveButton )
DEFMETHOD( Prefswin_Toolbar_AddSpacer )
DEFMETHOD( Prefswin_Toolbar_RemoveButton )
DEFMETHOD( Prefswin_Toolbar_FixFields )
DEFMETHOD( Prefswin_Toolbar_DisplayFields )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_toolbarclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_ToolbarClass";
#endif

	return( TRUE );
}

void delete_prefswin_toolbarclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_toolbarclass( void )
{
	return( mcc->mcc_Class );
}
