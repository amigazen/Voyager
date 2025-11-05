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
 * Javascript
 * ----------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_javascript.c,v 1.15 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#endif

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR chk_js_enable;
	APTR chk_js_debug;
	APTR chk_js_log;
	APTR str_js_file;
};


DECNEW
{
	struct Data *data;
	APTR chk_js_enable, chk_js_debug, chk_js_log, str_js_file;

#if !USE_JS
#define JS_DISABLED1 MUIA_Disabled, TRUE,
#else
#define JS_DISABLED1
#endif

#ifdef MBX
	return( 0 ); //TOFIX!!
#else

	obj = DoSuperNew( cl, obj,
		/* Enable js */
		Child, HGroup, GroupFrameT( GS( PREFSWIN_JS_L ) ),
			Child, HVSpace,
			Child, Label( GS( PREFSWIN_JS_ENABLE ) ),
			Child, chk_js_enable = pchecki( DSI_JS_ENABLE, GS( PREFSWIN_JS_ENABLE ) ),
			Child, HVSpace,
			JS_DISABLED1
		End,

		/* Error handling */
		Child, VGroup, GroupFrameT( GS( PREFSWIN_JS_ERROR ) ),
			Child, HGroup,
				Child, HSpace( 0 ),
				Child, Label1( GS( PREFSWIN_JS_DEBUG ) ),
				Child, chk_js_debug = pchecki( DSI_JS_DEBUG, GS( PREFSWIN_JS_DEBUG ) ),
				Child, HSpace( 0 ),
			End,

			Child, HGroup,
				Child, Label1( GS( PREFSWIN_JS_ERROR_LOG ) ),
				Child, chk_js_log = pcheck( DSI_JS_ERRORLOG, GS( PREFSWIN_JS_ERROR_LOG ) ),

				Child, str_js_file = PopaslObject,
					MUIA_Popasl_Type, ASL_FileRequest,
					MUIA_Popstring_String, pstring( DSI_JS_LOGFILE, 256, "" ),
					MUIA_Popstring_Button, PopButton( MUII_PopDrawer ),
					ASLFR_TitleText, GS( PREFSWIN_JS_ERROR_LOG ),
				End,
			End,
			JS_DISABLED1
		End,
	End;

#endif /* MBX */

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->chk_js_enable = chk_js_enable;
	data->chk_js_debug = chk_js_debug;
	data->chk_js_log = chk_js_log;
	data->str_js_file = str_js_file;

	set( str_js_file, MUIA_Disabled, !getv( chk_js_log, MUIA_Selected ) );

	DoMethod( chk_js_log, MUIM_Notify,
		MUIA_Selected, MUIV_EveryTime,
		str_js_file, 3,
		MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue
	);

		
	set( chk_js_enable, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_JS_ENABLE ) );
	set( chk_js_debug, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_JS_DEBUG ) );
	set( chk_js_log, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_JS_LOG ) );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	storeattr( data->chk_js_enable, MUIA_Selected, DSI_JS_ENABLE );
	storeattr( data->chk_js_debug, MUIA_Selected, DSI_JS_DEBUG );
	storeattr( data->chk_js_log, MUIA_Selected, DSI_JS_ERRORLOG );
	storestring( data->str_js_file, DSI_JS_LOGFILE );

	return( DOSUPER );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_javascriptclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_JavascriptClass";
#endif

	return( TRUE );
}

void delete_prefswin_javascriptclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_javascriptclass( void )
{
	return( mcc->mcc_Class );
}
