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
 * Colors
 * ------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_colors.c,v 1.10 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR pp_col1;
	APTR pp_col2;
	APTR pp_col3;
	APTR pp_col4;
	APTR chk_ignoredoccols;
};


DECNEW
{
	struct Data *data;
	APTR pp_col1, pp_col2, pp_col3, pp_col4;
	APTR chk_ignoredoccols;

	obj = DoSuperNew( cl, obj,
		GroupFrameT( GS( PREFSWIN_COLORS_L1 ) ),
		Child, ColGroup( 2 ),	
		Child, Label2( GSI( MSG_PREFSWIN_COLORS_COL1 + 0 ) ),
		Child, pp_col1 = PoppenObject, MUIA_Window_Title, GSI( MSG_PREFSWIN_COLORS_COL1 + 0 ), MUIA_CycleChain, 1, MUIA_Pendisplay_Spec, getprefs( DSI_COLORS + 0 ), End,
		Child, Label2( GSI( MSG_PREFSWIN_COLORS_COL1 + 1 ) ),
		Child, pp_col2 = PoppenObject, MUIA_Window_Title, GSI( MSG_PREFSWIN_COLORS_COL1 + 1 ), MUIA_CycleChain, 1, MUIA_Pendisplay_Spec, getprefs( DSI_COLORS + 1 ), End,
		Child, Label2( GSI( MSG_PREFSWIN_COLORS_COL1 + 2 ) ),
		Child, pp_col3 = PoppenObject, MUIA_Window_Title, GSI( MSG_PREFSWIN_COLORS_COL1 + 2 ), MUIA_CycleChain, 1, MUIA_Pendisplay_Spec, getprefs( DSI_COLORS + 2 ), End,
		Child, Label2( GSI( MSG_PREFSWIN_COLORS_COL1 + 3 ) ),
		Child, pp_col4 = PoppenObject, MUIA_Window_Title, GSI( MSG_PREFSWIN_COLORS_COL1 + 3 ), MUIA_CycleChain, 1, MUIA_Pendisplay_Spec, getprefs( DSI_COLORS + 3 ), End,
		End,
		Child, hbar(),
		Child, HGroup,
			Child, HSpace( 0 ),
			Child, chk_ignoredoccols = pcheck( DSI_FLAGS + VFLG_IGNOREDOCCOLS, GS( PREFSWIN_COLORS_IGNOREDOC ) ),
			Child, Label1( GS( PREFSWIN_COLORS_IGNOREDOC ) ),
			Child, HSpace( 0 ),
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->pp_col1 = pp_col1;
	data->pp_col2 = pp_col2;
	data->pp_col3 = pp_col3;
	data->pp_col4 = pp_col4;
	data->chk_ignoredoccols = chk_ignoredoccols;

	set( chk_ignoredoccols, MUIA_Disabled, TRUE );

	setupd( pp_col1, MUIA_Pendisplay_Spec );
	setupd( pp_col2, MUIA_Pendisplay_Spec );
	setupd( pp_col3, MUIA_Pendisplay_Spec );
	setupd( pp_col4, MUIA_Pendisplay_Spec );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;
	struct MUI_PenSpec *ps;

	storeattr( data->chk_ignoredoccols, MUIA_Selected, DSI_FLAGS + VFLG_IGNOREDOCCOLS );
		
	get( data->pp_col1, MUIA_Pendisplay_Spec, &ps );
	setprefs( DSI_COLORS + 0, sizeof( *ps ), ps );
	get( data->pp_col2, MUIA_Pendisplay_Spec, &ps );
	setprefs( DSI_COLORS + 1, sizeof( *ps ), ps );
	get( data->pp_col3, MUIA_Pendisplay_Spec, &ps );
	setprefs( DSI_COLORS + 2, sizeof( *ps ), ps );
	get( data->pp_col4, MUIA_Pendisplay_Spec, &ps );
	setprefs( DSI_COLORS + 3, sizeof( *ps ), ps );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_colorsclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_ColorsClass";
#endif

	return( TRUE );
}

void delete_prefswin_colorsclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_colorsclass( void )
{
	return( mcc->mcc_Class );
}
