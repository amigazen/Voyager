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
 * Network
 * -------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_network.c,v 1.12 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

#if USE_NET

/* private */
#include "prefswin.h"
#include "mui_func.h"

#define PWN_NUM_PROXY 5
#define PWN_NUM_PROXY_CHK 3

struct Data {
	APTR chk_proxy_use[ PWN_NUM_PROXY_CHK ];
	APTR str_proxy_port[ PWN_NUM_PROXY ];
	APTR str_proxy_host[ PWN_NUM_PROXY ];
	APTR str_noproxy;
	APTR str_proxy_autourl;
	APTR sl_maxcon;
	APTR cyc_proxy_mode;
	APTR cyc_checkifonline;
};


DECNEW
{
	struct Data *data;
	int c;
	static STRPTR proxymodes[ 4 ];
	static STRPTR offlineopts[ 3 ];
	APTR chk_proxy_use[ PWN_NUM_PROXY_CHK ], str_proxy_host[ PWN_NUM_PROXY ], str_proxy_port[ PWN_NUM_PROXY ], str_noproxy;
	APTR sl_maxcon, cyc_checkifonline;
	APTR cyc_proxy_mode, str_proxy_autourl;
	APTR grp_proxies;

	proxymodes[ 0 ] = GS( PREFSWIN_PROXIES_MODE0 );
	proxymodes[ 1 ] = GS( PREFSWIN_PROXIES_MODE1 );
	proxymodes[ 2 ] = GS( PREFSWIN_PROXIES_MODE2 );

	offlineopts[ 0 ] = GS( PREFSWIN_CYC_OFFLINE_MANUAL );
	offlineopts[ 1 ] = GS( PREFSWIN_CYC_OFFLINE_CHECKSTACK );

	obj = DoSuperNew( cl, obj,

		Child, VGroup, GroupFrameT( GS( PREFSWIN_PROXIES_GFT ) ),
			Child, cyc_proxy_mode = pcycle( proxymodes, DSI_PROXY_CONFMODE, NULL ),

			Child, grp_proxies = PageGroup,
				MUIA_Group_ActivePage, getprefslong( DSI_PROXY_CONFMODE, 0 ),

				Child, RectangleObject, End, // No Proxies

				Child, VGroup, // Autoconfig

					Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_PROXIES_AUTOCONF ), End,
					Child, str_proxy_autourl = pstring( DSI_PROXY_AUTOCONF_URL, 256, NULL ),
				End,

				Child, VGroup, // Manual proxy config

					Child, ColGroup( 4 ),

					Child, HSpace( 0 ),
					Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_PROXIES_HOST ), End,
					Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_PROXIES_PORT ), End,
					Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_PROXIES_USE ), End,

					Child, Label2( GS(PREFSWIN_PROXY_HTTP) ),
					Child, str_proxy_host[ 0 ] = pstring( DSI_PROXY_HTTP, 256, "_1" ),
					Child, str_proxy_port[ 0 ] = pinteger( DSI_PROXY_HTTP + DSI_PROXY_PORT_OFFSET, "_2" ),
					Child, chk_proxy_use[ 0 ] = pcheck( DSI_PROXY_HTTP + DSI_PROXY_USE_OFFSET, GS(PREFSWIN_PROXY_HTTP) ),

					Child, Label2( GS(PREFSWIN_PROXY_FTP) ),
					Child, str_proxy_host[ 1 ] = pstring( DSI_PROXY_FTP , 256, "_3" ),
					Child, str_proxy_port[ 1 ] = pinteger( DSI_PROXY_FTP + DSI_PROXY_PORT_OFFSET, "_4" ),
					Child, chk_proxy_use[ 1 ] = pcheck( DSI_PROXY_FTP + DSI_PROXY_USE_OFFSET, GS(PREFSWIN_PROXY_FTP) ),

					Child, Label2( GS(PREFSWIN_PROXY_SSL) ),
					Child, str_proxy_host[ 2 ] = pstring( DSI_PROXY_HTTPS, 256, "_5" ),
					Child, str_proxy_port[ 2 ] = pinteger( DSI_PROXY_HTTPS + DSI_PROXY_PORT_OFFSET, "_6" ),
					Child, chk_proxy_use[ 2 ] = pcheck( DSI_PROXY_HTTPS + DSI_PROXY_USE_OFFSET, GS(PREFSWIN_PROXY_SSL) ),

					Child, Label2( GS(PREFSWIN_PROXY_GOPHER) ),
					Child, str_proxy_host[ 3 ] = pstring( DSI_PROXY_GOPHER, 256, "_7" ),
					Child, str_proxy_port[ 3 ] = pinteger( DSI_PROXY_GOPHER + DSI_PROXY_PORT_OFFSET, "_8" ),
					Child, HSpace( 0 ),
//					Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_PROXIES_ALWAYS ), MUIA_Font, MUIV_Font_Tiny, End,

					Child, Label2( GS(PREFSWIN_PROXY_WAIS) ),
					Child, str_proxy_host[ 4 ] = pstring( DSI_PROXY_WAIS , 256 , "_9" ),
					Child, str_proxy_port[ 4 ] = pinteger( DSI_PROXY_WAIS + DSI_PROXY_PORT_OFFSET, "_0" ),
					Child, HSpace( 0 ),
//					Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_PROXIES_ALWAYS ), MUIA_Font, MUIV_Font_Tiny, End,

				End,

				Child, HGroup,
					Child, Label( GS( PREFSWIN_PROXIES_NO ) ),
					Child, str_noproxy = pstring( DSI_NOPROXY, 512, GS( PREFSWIN_PROXIES_NO ) ),
				End,
			End,
		End,
		End,

		Child, VGroup, GroupFrameT( GS( PREFSWIN_MAXCON_GFT ) ),
			Child, sl_maxcon = SliderObject, MUIA_Numeric_Min, 1, MUIA_Numeric_Max, 32, MUIA_Numeric_Value, getprefslong( DSI_NET_MAXCON, 8 ), MUIA_CycleChain, 1,
			End,
		End,

		Child, VGroup, GroupFrameT( GS( PREFSWIN_OFFLINE_TITLE ) ),
			Child, HGroup,
				Child, Label( GS( PREFSWIN_OFFLINEMODE ) ),
				Child, cyc_checkifonline = pcycle( offlineopts, DSI_NET_CHECKIFONLINE, GS( PREFSWIN_OFFLINEMODE ) ),
	    	End,
 		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	for( c = 0; c < PWN_NUM_PROXY_CHK; c++ )
	{
		data->chk_proxy_use[ c ] = chk_proxy_use[ c ];
	}
	for( c = 0; c < PWN_NUM_PROXY; c++ )
	{
		data->str_proxy_port[ c ] = str_proxy_port[ c ];
		data->str_proxy_host[ c ] = str_proxy_host[ c ];
	}
	data->str_noproxy = str_noproxy;
	data->str_proxy_autourl = str_proxy_autourl;
	data->sl_maxcon = sl_maxcon;
	data->cyc_proxy_mode = cyc_proxy_mode;
	data->cyc_checkifonline = cyc_checkifonline;

	DoMethod( chk_proxy_use[ 0 ], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		notify, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue,
		str_proxy_host[ 0 ], str_proxy_port[ 0 ], NULL
	);

	DoMethod( chk_proxy_use[ 1 ], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		notify, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue,
		str_proxy_host[ 1 ], str_proxy_port[ 1 ], NULL
	);

	DoMethod( chk_proxy_use[ 2 ], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		notify, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue,
		str_proxy_host[ 2 ], str_proxy_port[ 2 ], NULL
	);

	for( c = 0; c < PWN_NUM_PROXY_CHK; c++ )
	{
		int x = getv( chk_proxy_use[ c ], MUIA_Selected );
		set( chk_proxy_use[ c ], MUIA_Selected, !x );
		set( chk_proxy_use[ c ], MUIA_Selected, x );
	}

	for( c = 0; c < PWN_NUM_PROXY; c++ )
		set( str_proxy_port[ c ], MUIA_Weight, 9 );

	DoMethod( cyc_proxy_mode, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		grp_proxies, 3, MUIM_Set, MUIA_Group_ActivePage, MUIV_TriggerValue
	);

	/* Help */
	set( cyc_proxy_mode, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_PROXY_MODE ) );
	set( str_proxy_autourl, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_AUTOURL ) );
	set( chk_proxy_use[0], MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_PROXY_USE_HTTP ) );
	set( str_proxy_host[0], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_HOST ) );
	set( str_proxy_port[0], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_PORT ) );
	set( chk_proxy_use[1], MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_PROXY_USE_FTP ) );
	set( str_proxy_host[1], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_HOST ) );
	set( str_proxy_port[1], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_PORT ) );
	set( chk_proxy_use[2], MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_PROXY_USE_SSL ) );
	set( str_proxy_host[2], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_HOST ) );
	set( str_proxy_port[2], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_PORT ) );
	set( str_proxy_host[3], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_HOST ) );
	set( str_proxy_port[3], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_PORT ) );
	set( str_proxy_host[4], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_HOST ) );
	set( str_proxy_port[4], MUIA_ShortHelp, GS( SH_PREFSWIN_STR_PROXY_PORT ) );
	set( str_noproxy, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_NOPROXY ) );
	set( sl_maxcon, MUIA_ShortHelp, GS( SH_PREFSWIN_SL_MAXCON ) );
	
	setupd( sl_maxcon, MUIA_Numeric_Value );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;
	int c;

	for( c = 0; c < PWN_NUM_PROXY; c++ )
	{
		static ULONG ptypes[] = { DSI_PROXY_HTTP, DSI_PROXY_FTP, DSI_PROXY_HTTPS, DSI_PROXY_GOPHER, DSI_PROXY_WAIS };

		if( c < PWN_NUM_PROXY_CHK )
			storeattr( data->chk_proxy_use[ c ], MUIA_Selected, ptypes[ c ] + DSI_PROXY_USE_OFFSET );
		storeattr( data->str_proxy_port[ c ], MUIA_String_Integer, ptypes[ c ] + DSI_PROXY_PORT_OFFSET );
		storestring( data->str_proxy_host[ c ], ptypes[ c ]);
	}

	storestring( data->str_noproxy, DSI_NOPROXY );
	storestring( data->str_proxy_autourl, DSI_PROXY_AUTOCONF_URL );
	storeattr( data->sl_maxcon, MUIA_Slider_Level, DSI_NET_MAXCON );
	storeattr( data->cyc_proxy_mode, MUIA_Cycle_Active, DSI_PROXY_CONFMODE );
	storeattr( data->cyc_checkifonline, MUIA_Cycle_Active, DSI_NET_CHECKIFONLINE );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_networkclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_NewworkClass";
#endif

	return( TRUE );
}

void delete_prefswin_networkclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_networkclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
