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
 * Download
 * --------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_download.c,v 1.16 2003/07/06 16:51:34 olli Exp $
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
	APTR str_dlpath;
	APTR chk_autoclose;
	APTR cyc_dlmode;
	APTR str_dltimeout;
	APTR str_dlretries;
};


DECNEW
{
#ifndef MBX
	static STRPTR dl_opts[ 4 ];
	struct Data *data;

	APTR str_dlpath;
	APTR chk_autoclose;
	APTR cyc_dlmode;
	APTR str_dltimeout;
	APTR str_dlretries;

	fillstrarray( dl_opts, MSG_PREFSWIN_AUTOCLEANUP_1, 3 );

	obj	= DoSuperNew( cl, obj,
		Child, VGroup, GroupFrameT( GS( PREFSWIN_STYLE_L4 ) ),
			Child, HGroup,
				Child, Label2( GS( PREFSWIN_MIME_L_DLDIR ) ),
				Child, str_dlpath = PopaslObject,
					MUIA_Popasl_Type, ASL_FileRequest,
					MUIA_Popstring_String, pstring( DSI_SAVEDEST, 256, GS( PREFSWIN_MIME_L_DLDIR ) ),
					MUIA_Popstring_Button, PopButton( MUII_PopDrawer ),
					ASLFR_TitleText, GS( PREFSWIN_MIME_L_DLDIR ),
					ASLFR_DrawersOnly, TRUE,
				End,
			End,
			Child, HGroup,
				Child, HSpace( 0 ),
				Child, Label2( GS( PREFSWIN_AUTOCLOSEDLWIN ) ),
				Child, chk_autoclose = pchecki( DSI_FLAGS + VFLG_AUTOCLOSEDLWIN, GS( PREFSWIN_AUTOCLOSEDLWIN ) ),
				Child, HSpace( 0 ),
				End,
			Child, HGroup,
				Child, HSpace( 0 ),
				Child, Label2( GS( PREFSWIN_AUTOCLEANUP ) ),
				Child, cyc_dlmode = pcycle( dl_opts, DSI_FLAGS + VFLG_AUTOCLEANUP_MODES, GS( PREFSWIN_AUTOCLEANUP ) ),
				Child, HSpace( 0 ),
			End,
		End,

		Child, VGroup, GroupFrameT( GS( PREFSWIN_DLTIMINGS ) ),
			Child, HGroup,
				Child, Label2( GS( PREFSWIN_DLTIMEOUT ) ),
				Child, str_dltimeout = pinteger( DSI_NET_DOWNLOAD_TIMEOUT, GS( PREFSWIN_DLTIMEOUT ) ),
				
				Child, Label2( GS( PREFSWIN_DLRETRIES ) ),
				Child, str_dlretries = pinteger( DSI_NET_DOWNLOAD_RETRIES, GS( PREFSWIN_DLRETRIES ) ),
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->str_dlpath = str_dlpath;
	data->chk_autoclose = chk_autoclose;
	data->cyc_dlmode = cyc_dlmode;
	data->str_dltimeout = str_dltimeout;
	data->str_dlretries = str_dlretries;

	/* Help */
	set( str_dlpath, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_DLPATH ) );
	set( chk_autoclose, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_AUTOCLOSE ) );
	set( cyc_dlmode, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_DLMODE ) );

	return( ( ULONG )obj );
#else
	return( 0 );
#endif /* !MBX */
}


DECDISPOSE
{
	GETDATA;

	storestring( data->str_dlpath, DSI_SAVEDEST );
	storeattr( data->str_dltimeout, MUIA_String_Integer, DSI_NET_DOWNLOAD_TIMEOUT );
	gp_download_timeout = getv( data->str_dltimeout, MUIA_String_Integer );
	storeattr( data->str_dlretries, MUIA_String_Integer, DSI_NET_DOWNLOAD_RETRIES );
	gp_download_retries = getv( data->str_dlretries, MUIA_String_Integer );
	storeattr( data->chk_autoclose, MUIA_Selected, DSI_FLAGS + VFLG_AUTOCLOSEDLWIN );
	storeattr( data->cyc_dlmode, MUIA_Cycle_Active, DSI_FLAGS + VFLG_AUTOCLEANUP_MODES );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_downloadclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_DownloadClass";
#endif

	return( TRUE );
}

void delete_prefswin_downloadclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_downloadclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
