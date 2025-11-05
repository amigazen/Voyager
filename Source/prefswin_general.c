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
 * General UI style
 * ----------------
 *
 * © 2000-2003 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_general.c,v 1.20 2004/06/09 22:50:32 zapek Exp $
*/

#include "voyager.h"

/* private */
#include "prefswin.h"
#include "mui_func.h"
#include "htmlclasses.h" /* TOFIX: bah */

struct Data {
	APTR chk_exit;
	APTR chk_scrollbars;
	APTR chk_icon;
	APTR cyc_homepageurlmode;
	APTR chk_errorrequester;
	APTR chk_homepage_autoload;
	APTR chk_use_clock;
	#if USE_SMOOTH_SCROLLING
	APTR chk_smooth_scroll;
	#endif
	#if USE_SINGLEWINDOW
	APTR chk_singlewindow;
	#endif
	APTR str_homepage;
	/* need special actions (initial values stored to find out) */
	int smooth_scroll_initial;
};


DECNEW
{
	struct Data *data;
	APTR str_homepage, chk_exit, chk_icon, chk_scrollbars, chk_errorrequester, cyc_homepageurlmode, chk_homepage_autoload, chk_use_clock;
#if USE_SINGLEWINDOW
	APTR chk_singlewindow;
#endif
	APTR grp_misc;
	static STRPTR homepageurl_opts[ 5 ];

	fillstrarray( homepageurl_opts, MSG_PREFSWIN_HOMEPAGEURL_1, 4 );

	obj	= DoSuperNew( cl, obj,

		/* Transfer animation */
		Child, HGroup, GroupFrameT( GS( PREFSWIN_STYLE_ANIM ) ),
			Child, HSpace( 0 ),
			Child, Label1( GS( PREFSWIN_STYLE_ANIMHIDE ) ),
			Child, chk_icon = pchecki( DSI_FLAGS + VFLG_HIDE_ICON, GS( PREFSWIN_STYLE_ANIMHIDE ) ),
			Child, HSpace( 0 ),
		End,

		/* Home page */
		Child, VGroup, GroupFrameT( GS( PREFSWIN_STYLE_L2 ) ),
			Child, HGroup,
				Child, HSpace( 0 ),
				Child, Label2( GS( PREFSWIN_HOMEPAGEURL ) ),
				Child, cyc_homepageurlmode = pcycle( homepageurl_opts, DSI_FLAGS + VFLG_HOMEPAGEURL_MODES, GS( PREFSWIN_HOMEPAGEURL ) ),
				Child, str_homepage = NewObject( geturlstringclass(), NULL, MUIA_String_Contents, getprefs( DSI_HOMEPAGE ), MUIA_CycleChain, 1, MUIA_ControlChar, ParseHotKey(GS( PREFSWIN_STYLE_URL )), MUIA_Disabled, TRUE, End,
				Child, HSpace( 0 ),
			End,
			Child, HGroup,
				Child, HSpace( 0 ),
				Child, Label1( GS( PREFSWIN_HOMEPAGE_AUTOLOAD ) ),
				Child, chk_homepage_autoload = pchecki( DSI_FLAGS + VFLG_HOMEPAGE_AUTOLOAD, GS( PREFSWIN_HOMEPAGE_AUTOLOAD ) ),
				Child, HSpace( 0 ),
			End,
		End,

		/* Misc panel */
		Child, grp_misc = ColGroup( 4 ), GroupFrameT( GS( PREFSWIN_STYLE_L5 ) ),
			Child, HSpace( 0 ),
			Child, Label1( GS( PREFSWIN_STYLE_SCROLLBARS ) ),
			Child, chk_scrollbars = pchecki( DSI_FLAGS + VFLG_SCROLLBARS, GS( PREFSWIN_STYLE_SCROLLBARS ) ),
			Child, HSpace( 0 ),

			Child, HSpace( 0 ),
			Child, Label1( GS( PREFSWIN_STYLE_EXIT ) ),
			Child, chk_exit = pchecki( DSI_FLAGS + VFLG_SAVE_ON_EXIT, GS( PREFSWIN_STYLE_EXIT ) ),
			Child, HSpace( 0 ),

			Child, HSpace( 0 ),
			Child, Label1( GS( PREFSWIN_USE_REQUESTER ) ),
			Child, chk_errorrequester = pchecki( DSI_FLAGS + VFLG_USE_ERROR_REQUESTER, GS( PREFSWIN_USE_REQUESTER ) ),
			Child, HSpace( 0 ),

			Child, HSpace( 0 ),
			Child, Label1( GS( PREFSWIN_USE_CLOCK ) ),
			Child, chk_use_clock = pchecki( DSI_FLAGS + VFLG_USE_CLOCK, GS( PREFSWIN_USE_CLOCK ) ),
			Child, HSpace( 0 ),

#if USE_SINGLEWINDOW
			Child, HSpace( 0 ),
			Child, Label1( GS( PREFSWIN_SINGLEWINDOW ) ),
			Child, chk_singlewindow = pcheck( DSI_FLAGS + VFLG_SINGLEWINDOW, GS( PREFSWIN_SINGLEWINDOW ) ),
			Child, HSpace( 0 ),
#endif
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->chk_exit = chk_exit;
	data->chk_scrollbars = chk_scrollbars;
	data->chk_icon = chk_icon;
	data->cyc_homepageurlmode = cyc_homepageurlmode;
	data->chk_errorrequester = chk_errorrequester;
	data->chk_homepage_autoload = chk_homepage_autoload;
	data->chk_use_clock = chk_use_clock;
	data->str_homepage = str_homepage;
#if USE_SINGLEWINDOW
	data->chk_singlewindow = chk_singlewindow;
#endif

	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 0, str_homepage, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 0, str_homepage, 3, MUIM_SetAsString, MUIA_Text_Contents, "http://v3.vapor.com/" );
	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 1, str_homepage, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 1, str_homepage, 3, MUIM_SetAsString, MUIA_Text_Contents, "about:" );
	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 2, str_homepage, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 2, str_homepage, 3, MUIM_SetAsString, MUIA_Text_Contents, "" );
	DoMethod( cyc_homepageurlmode, MUIM_Notify, MUIA_Cycle_Active, 3, str_homepage, 3, MUIM_Set, MUIA_Disabled, FALSE);

	/* Help */
	set( chk_icon, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_ICON ) );
	set( str_homepage, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_HOMEPAGE ) );
	set( chk_scrollbars, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_SCROLLBARS ) );
	set( chk_exit, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_EXIT ) ) ;
	set( cyc_homepageurlmode, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_HOMEPAGEURL ) );
	set( chk_errorrequester, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_ERRORREQUESTER ) );
	
	/* Update */
	setupd( chk_scrollbars, MUIA_Selected );
	setupd( chk_use_clock, MUIA_Selected );


	switch( getv( cyc_homepageurlmode, MUIA_Cycle_Active ) )
	{
		case 0: DoMethod( str_homepage, MUIM_SetAsString, MUIA_Text_Contents, "http://v3.vapor.com/" );
				break;

		case 1: DoMethod( str_homepage, MUIM_SetAsString, MUIA_Text_Contents, "about:" );
				break;

		case 2: DoMethod( str_homepage, MUIM_SetAsString, MUIA_Text_Contents, "" );
				break;

		case 3: set( str_homepage, MUIA_Disabled, FALSE );
				break;
	}

	/*
	 * Smooth scrolling is for MUI4 only
	 */
	#if USE_SMOOTH_SCROLLING
	if( ( MUIMasterBase->lib_Version > 20 || ( MUIMasterBase->lib_Version == 20 && MUIMasterBase->lib_Revision >= 2137 ) ) )
	{
		DoMethod( grp_misc, OM_ADDMEMBER, HSpace( 0 ) );
		DoMethod( grp_misc, OM_ADDMEMBER, Label1( GS( PREFSWIN_SMOOTH_SCROLL ) ) );
		DoMethod( grp_misc, OM_ADDMEMBER, data->chk_smooth_scroll = pchecki( DSI_FLAGS + VFLG_SMOOTH_SCROLL, GS( PREFSWIN_SMOOTH_SCROLL ) ) );
		DoMethod( grp_misc, OM_ADDMEMBER, HSpace( 0 ) );
	}
	
	/* Redisplay */
	if( data->chk_smooth_scroll )
	{
		data->smooth_scroll_initial = getv( data->chk_smooth_scroll, MUIA_Selected );
	}
	#endif

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	storeattr( data->chk_exit, MUIA_Selected, DSI_FLAGS + VFLG_SAVE_ON_EXIT );
	storeattr( data->chk_scrollbars, MUIA_Selected, DSI_FLAGS + VFLG_SCROLLBARS );
	storeattr( data->chk_icon, MUIA_Selected, DSI_FLAGS + VFLG_HIDE_ICON );
	storeattr( data->cyc_homepageurlmode, MUIA_Cycle_Active, DSI_FLAGS + VFLG_HOMEPAGEURL_MODES );
	storeattr( data->chk_errorrequester, MUIA_Selected, DSI_FLAGS + VFLG_USE_ERROR_REQUESTER );
	storeattr( data->chk_homepage_autoload, MUIA_Selected, DSI_FLAGS + VFLG_HOMEPAGE_AUTOLOAD );
	storeattr( data->chk_use_clock, MUIA_Selected, DSI_FLAGS + VFLG_USE_CLOCK );
	storestring( data->str_homepage, DSI_HOMEPAGE );

	#if USE_SMOOTH_SCROLLING
	if( data->chk_smooth_scroll )
	{
		storeattr( data->chk_smooth_scroll, MUIA_Selected, DSI_FLAGS + VFLG_SMOOTH_SCROLL );
		gp_smooth_scroll = getv( data->chk_smooth_scroll, MUIA_Selected );
		if( data->smooth_scroll_initial != gp_smooth_scroll )
		{
			DoMethod( app, MM_DoAllWins, MM_HTMLWin_SetSmoothScroll, gp_smooth_scroll );
		}
	}
	#endif

#if USE_SINGLEWINDOW
	storeattr( data->chk_singlewindow, MUIA_Selected, DSI_FLAGS + VFLG_SINGLEWINDOW );
	gp_singlewindow = getv( data->chk_singlewindow, MUIA_Selected );
#else
	gp_singlewindow = FALSE;
#endif

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_generalclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_GeneralClass";
#endif

	return( TRUE );
}

void delete_prefswin_generalclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_generalclass( void )
{
	return( mcc->mcc_Class );
}
