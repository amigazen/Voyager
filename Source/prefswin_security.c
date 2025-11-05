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
 * Security
 * --------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_security.c,v 1.10 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

#if USE_NET

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR chk_ask_mailto;
	APTR cyc_ask_cookie_perm;
	APTR cyc_ask_cookie_temp;
	APTR chk_warn_post;
	APTR chk_no_referer;
	APTR chk_no_sslcache;
	APTR chk_no_mailaddr;
};


DECNEW
{
	struct Data *data;
	static STRPTR cookieask[ 4 ];
	APTR chk_ask_mailto, cyc_ask_cookie_temp, cyc_ask_cookie_perm, chk_warn_post, chk_no_referer, chk_no_mailaddr, chk_no_sslcache;

	cookieask[ 0 ] = GS( PREFSWIN_SECURITY_COOKIE_S_0 );
	cookieask[ 1 ] = GS( PREFSWIN_SECURITY_COOKIE_S_1 );
	cookieask[ 2 ] = GS( PREFSWIN_SECURITY_COOKIE_S_2 );

	obj = DoSuperNew( cl, obj,
		/* Security */
		Child, ColGroup( 4 ), GroupFrameT( GS( PREFSWIN_SECURITY_GFT ) ),
			Child, HSpace( 0 ),
			Child, chk_ask_mailto = pcheck( DSI_SECURITY_ASK_MAILTO, GS( PREFSWIN_SECURITY_MAILTO ) ),
			Child, LLabel1( GS( PREFSWIN_SECURITY_MAILTO ) ),
			Child, HSpace( 0 ),
			
			Child, HSpace( 0 ),
			Child, chk_warn_post = pcheck( DSI_SECURITY_WARN_POST, GS( PREFSWIN_SECURITY_WARN_POST ) ),
			Child, LLabel1( GS( PREFSWIN_SECURITY_WARN_POST ) ),
			Child, HSpace( 0 ),

			Child, HSpace( 0 ),
			Child, chk_no_referer = pcheck( DSI_SECURITY_NO_REFERER,GS( PREFSWIN_SECURITY_NO_REFERER ) ),
			Child, LLabel1( GS( PREFSWIN_SECURITY_NO_REFERER ) ),
			Child, HSpace( 0 ),
				
			Child, HSpace( 0 ),
			Child, chk_no_mailaddr = pcheck( DSI_SECURITY_NO_MAILADDR, GS( PREFSWIN_SECURITY_NO_MAILADDR ) ),
			Child, LLabel1( GS( PREFSWIN_SECURITY_NO_MAILADDR ) ),
			Child, HSpace( 0 ),

			Child, HSpace( 0 ),
			Child, chk_no_sslcache = pcheck( DSI_SECURITY_NO_SSLCACHE, GS( PREFSWIN_SECURITY_NO_SSLCACHE ) ),
			Child, LLabel1( GS( PREFSWIN_SECURITY_NO_SSLCACHE ) ),
			Child, HSpace( 0 ),
	    End,

		/* Cookies */
		Child, HGroup, GroupFrameT( GS( PREFSWIN_SECURITY_COOKIE ) ),
			Child, Label2( GS( PREFSWIN_SECURITY_COOKIE_TEMP ) ),
			Child, cyc_ask_cookie_temp = pcycle( cookieask, DSI_SECURITY_ASK_COOKIE_TEMP, GS( PREFSWIN_SECURITY_COOKIE_TEMP ) ),
			Child, HSpace( 0 ),
			Child, Label2( GS( PREFSWIN_SECURITY_COOKIE_PERM ) ),
			Child, cyc_ask_cookie_perm = pcycle( cookieask, DSI_SECURITY_ASK_COOKIE_PERM, GS( PREFSWIN_SECURITY_COOKIE_PERM ) ),
		End,
//      Child, RectangleObject, MUIA_Weight, 1, End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->chk_ask_mailto = chk_ask_mailto;
	data->cyc_ask_cookie_perm = cyc_ask_cookie_perm;
	data->cyc_ask_cookie_temp = cyc_ask_cookie_temp;
	data->chk_warn_post = chk_warn_post;
	data->chk_no_referer = chk_no_referer;
	data->chk_no_sslcache = chk_no_sslcache;
	data->chk_no_mailaddr = chk_no_mailaddr;

	/* Help */
	set( chk_ask_mailto, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_ASK_MAILTO ) );
	set( chk_warn_post, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_WARN_POST ) );
	set( chk_no_referer, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_NO_REFERER ) );
	set( chk_no_mailaddr, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_NO_MAILADDR ) );
	set( chk_no_sslcache, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_NO_SSLCACHE ) );
	set( cyc_ask_cookie_temp, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_ASK_COOKIE_TEMP ) );
	set( cyc_ask_cookie_perm, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_ASK_COOKIE_PERM ) );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	storeattr( data->chk_ask_mailto, MUIA_Selected, DSI_SECURITY_ASK_MAILTO );
	storeattr( data->cyc_ask_cookie_perm, MUIA_Cycle_Active, DSI_SECURITY_ASK_COOKIE_PERM );
	storeattr( data->cyc_ask_cookie_temp, MUIA_Cycle_Active, DSI_SECURITY_ASK_COOKIE_TEMP );
	storeattr( data->chk_warn_post, MUIA_Selected, DSI_SECURITY_WARN_POST );
	storeattr( data->chk_no_referer, MUIA_Selected, DSI_SECURITY_NO_REFERER );
	storeattr( data->chk_no_sslcache, MUIA_Selected, DSI_SECURITY_NO_SSLCACHE );
	storeattr( data->chk_no_mailaddr, MUIA_Selected, DSI_SECURITY_NO_MAILADDR );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_securityclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_SecurityClass";
#endif

	return( TRUE );
}

void delete_prefswin_securityclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_securityclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
