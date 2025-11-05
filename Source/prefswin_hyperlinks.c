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
 * Hyperlinks
 * ----------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_hyperlinks.c,v 1.11 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR chk_links;
	APTR str_links;
};


DECNEW
{
	struct Data *data;

	APTR chk_links, str_links;
	APTR bt_expire;

	obj	= DoSuperNew( cl, obj,

		/* Links */
		Child, VGroup, GroupFrameT( GS( PREFSWIN_STYLE_L3 ) ),

			Child, HGroup,
				Child, HSpace( 0 ),
				Child, Label1( GS( PREFSWIN_STYLE_UNDERLINE_LINKS ) ),
				Child, chk_links = pchecki( DSI_FLAGS + VFLG_UNDERLINE_LINKS, GS( PREFSWIN_STYLE_UNDERLINE_LINKS ) ),
				Child, HSpace( 0 ),
			End,
			Child, HGroup,
				Child, Label2( GS( PREFSWIN_STYLE_LINKS ) ),
				Child, str_links = pinteger( DSI_NET_LINKS_EXPIRE, GS( PREFSWIN_STYLE_LINKS ) ),
				Child, Label2( GS( PREFSWIN_STYLE_LINKS_DAYS ) ),
				Child, bt_expire = button( MSG_PREFSWIN_STYLE_LINKS_NOW, 0 ),
			End,
		End,

	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->chk_links = chk_links;
	data->str_links = str_links;

	DoMethod( bt_expire, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, ID_PM_FLUSH_URLS
	);

	/* Help */
	set( chk_links, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_LINKS ) );
	set( str_links, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_LINKS ) );
	set( bt_expire, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_EXPIRE ) );

	setupd( chk_links, MUIA_Selected );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	storeattr( data->chk_links, MUIA_Selected, DSI_FLAGS + VFLG_UNDERLINE_LINKS );
	storeattr( data->str_links, MUIA_String_Integer, DSI_NET_LINKS_EXPIRE );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_hyperlinksclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_HyperlinksClass";
#endif

	return( TRUE );
}

void delete_prefswin_hyperlinksclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_hyperlinksclass( void )
{
	return( mcc->mcc_Class );
}
