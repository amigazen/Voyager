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
 * Mail / news
 * -----------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_mailnews.c,v 1.16 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#endif

/* private */
#include "prefswin.h"
#include "mui_func.h"


struct Data {
	APTR str_mailaddr;
	APTR str_realname;
	APTR str_organization;
	APTR str_smtp;
	APTR str_newsapp;
	APTR str_mailapp;
	APTR str_telnetapp;
	APTR str_sig;
	APTR cyc_mailapp;
	APTR cyc_newsapp;
};


DECMETHOD( Prefswin_Mailnews_SetMode, ULONG )
{
	GETDATA;

	int newsmode = getv( data->cyc_newsapp, MUIA_Cycle_Active );
	int mailmode = getv( data->cyc_mailapp, MUIA_Cycle_Active );

	set( data->str_newsapp, MUIA_Disabled, newsmode != 1 );
	set( data->str_mailapp, MUIA_Disabled, mailmode != 2 );

	set( data->str_organization, MUIA_Disabled, mailmode );
	set( data->str_realname, MUIA_Disabled, mailmode );
	set( data->str_smtp, MUIA_Disabled, mailmode );
	set( data->str_sig, MUIA_Disabled, mailmode );

	return( 0 );
}


static char * popph_telnet[3];
static char * popph_mailer[4];
static char * popph_news[3];

#if USE_POPPH
#define POPPH1 Child, str_mailapp = ppopph( DSI_NET_MAIL_APP, 256, popph_mailer ),
#define POPPH2 Child, str_newsapp = ppopph( DSI_NET_NEWS_APP, 256, popph_news ),
#define POPPH3 Child, str_telnetapp = ppopph( DSI_NET_TELNET, 256, popph_telnet ),
#else
#define POPPH1 Child, str_mailapp = pstring( DSI_NET_MAIL_APP, 256, "_1" ),
#define POPPH2 Child, str_newsapp = pstring( DSI_NET_NEWS_APP, 256, "_2" ),
#define POPPH3 Child, str_telnetapp = pstring( DSI_NET_TELNET, 256, "_3" ),
#endif

DECNEW
{
	struct Data *data;
	static STRPTR mailapps[ 4 ];
	static STRPTR newsapps[ 3 ];
	APTR str_mailaddr, str_realname, str_smtp, str_mailapp, str_telnetapp, str_newsapp, cyc_mailapp, cyc_newsapp, str_organization, str_sig;

	popph_telnet[ 0 ] = GS( PREFSWIN_TELNET_POPPH_0 );
	popph_telnet[ 1 ] = GS( PREFSWIN_TELNET_POPPH_1 );

	popph_mailer[ 0 ] = GS( PREFSWIN_MAIL_POPPH_0 );
	popph_mailer[ 1 ] = GS( PREFSWIN_MAIL_POPPH_1 );
	popph_mailer[ 2 ] = GS( PREFSWIN_MAIL_POPPH_2 );

	popph_news[ 0 ] = GS( PREFSWIN_NEWS_POPPH_0 );
	popph_news[ 1 ] = GS( PREFSWIN_NEWS_POPPH_1 );

	mailapps[ 0 ] = GS( PREFSWIN_MAIL_APP_INTERNAL );
	mailapps[ 1 ] = GS( PREFSWIN_MAIL_APP_MD2 );
	mailapps[ 2 ] = GS( PREFSWIN_MAIL_APP_EXT );

	newsapps[ 0 ] = GS( PREFSWIN_MAIL_APP_MD2 );
	newsapps[ 1 ] = GS( PREFSWIN_MAIL_APP_EXT );

#ifdef MBX
	return( 0 ); //TOFIX!!
#else

	obj = DoSuperNew( cl, obj,
		MUIA_Group_Columns, 2,
		GroupFrameT( GS( PREFSWIN_MAIL_GFT ) ),

		Child, Label2( GS( PREFSWIN_MAIL_ADDR ) ),
		Child, str_mailaddr = pstring( DSI_NET_MAILADDR, 256,  GS( PREFSWIN_MAIL_ADDR ) ),

		Child, Label2( GS( PREFSWIN_MAIL_REAL ) ),
		Child, str_realname = pstring( DSI_NET_REALNAME, 256, GS( PREFSWIN_MAIL_REAL ) ),

		Child, Label2( GS( PREFSWIN_MAIL_ORG ) ),
		Child, str_organization = pstring( DSI_NET_ORGANIZATION, 256, GS( PREFSWIN_MAIL_ORG ) ),

		Child, Label2( GS( PREFSWIN_MAIL_SIG ) ),
		Child, str_sig = PopaslObject,
			MUIA_Popasl_Type, ASL_FileRequest,
			MUIA_Popstring_String, pstring( DSI_NET_SIG, 256, GS( PREFSWIN_MAIL_SIG ) ),
			MUIA_Popstring_Button, PopButton( MUII_PopDrawer ),
			ASLFR_TitleText, GS( PREFSWIN_MAIL_SIG ),
		End,

		Child, Label2( GS( PREFSWIN_MAIL_SMTP ) ),
		Child, str_smtp = pstring( DSI_NET_SMTP, 256, GS( PREFSWIN_MAIL_SMTP ) ),

		Child, Label2( GS( PREFSWIN_MAIL_APP ) ),
		Child, HGroup,
			Child, cyc_mailapp = pcycle( mailapps, DSI_NET_USE_MAILAPP, GS( PREFSWIN_MAIL_APP ) ),
			POPPH1
		End,

		Child, Label2( GS( PREFSWIN_NEWS_APP ) ),
		Child, HGroup,
			Child, cyc_newsapp = pcycle( newsapps, DSI_NET_USE_NEWSAPP, GS( PREFSWIN_NEWS_APP ) ),
			POPPH2
		End,

		Child, Label2( GS( PREFSWIN_TELNET_APP ) ),
		POPPH3
	End;

#endif /* !MBX */

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->str_mailaddr = str_mailaddr;
	data->str_realname = str_realname;
	data->str_organization = str_organization;
	data->str_smtp = str_smtp;
	data->str_mailapp = str_mailapp;
	data->str_newsapp = str_newsapp;
	data->str_telnetapp = str_telnetapp;
	data->str_sig = str_sig;
	data->cyc_mailapp = cyc_mailapp;
	data->cyc_newsapp = cyc_newsapp;

	set( cyc_mailapp, MUIA_Weight, 0 );
	set( cyc_newsapp, MUIA_Weight, 0 );

	DoMethod( cyc_mailapp, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Mailnews_SetMode
	);
	
	DoMethod( cyc_newsapp, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Mailnews_SetMode
	);

	DoMethod( obj, MM_Prefswin_Mailnews_SetMode );

	/* Help */
	set( str_mailaddr, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_MAILADDR ) );
	set( str_realname, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_REALNAME ) );
	set( str_organization, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_ORGANIZATION ) );
	set( str_smtp, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_SMTP ) );
	set( cyc_mailapp, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_MAILAPP ) );
	set( cyc_newsapp, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_NEWSAPP ) );
	set( str_mailapp, MUIA_ShortHelp, GS( PREFSWIN_MAIL_APP_SH ) );
	set( str_newsapp, MUIA_ShortHelp, GS( PREFSWIN_NEWS_APP_SH ) );
	set( str_telnetapp, MUIA_ShortHelp, GS( PREFSWIN_TELNET_APP_SH ) );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	storestring( data->str_mailaddr, DSI_NET_MAILADDR );
	storestring( data->str_realname, DSI_NET_REALNAME );
	storestring( data->str_organization, DSI_NET_ORGANIZATION );
	storestring( data->str_smtp, DSI_NET_SMTP );
	storestring( data->str_mailapp, DSI_NET_MAIL_APP );
	storestring( data->str_newsapp, DSI_NET_NEWS_APP );
	storestring( data->str_telnetapp, DSI_NET_TELNET );
	storestring( data->str_sig, DSI_NET_SIG );
	storeattr( data->cyc_mailapp, MUIA_Cycle_Active, DSI_NET_USE_MAILAPP );
	storeattr( data->cyc_newsapp, MUIA_Cycle_Active, DSI_NET_USE_NEWSAPP );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Prefswin_Mailnews_SetMode )
ENDMTABLE


static struct MUI_CustomClass *mcc;

int create_prefswin_mailnewsclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_MailNewsClass";
#endif

	return( TRUE );
}

void delete_prefswin_mailnewsclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_mailnewsclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
