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
**
** $Id: searchrequest.c,v 1.16 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "voyager_cat.h"
#include "copyright.h"
#include "prefs.h"
#include "classes.h"
#include "mui_func.h"
#include "html.h"
#include "network.h"


struct Data {
	APTR myvobj;
	APTR str_search;
	APTR cyc_case;
};

DECCONST
{
	struct Data *data;
	APTR bt_search, bt_close, str_search, cyc_case;
	static STRPTR searchcase[ 3 ];

	searchcase[ 0 ] = GS( SEARCH_CASE0 );
	searchcase[ 1 ] = GS( SEARCH_CASE1 );

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('S','E','A','R'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, GS( SEARCH_TITLE ),
		WindowContents, VGroup,

			Child, HGroup,
				Child, Label2( GS( SEARCH_WHAT ) ),
				Child, str_search = String( "", 256 ),
			End,

			Child, cyc_case = Cycle( searchcase ),

			Child, HGroup,
				Child, bt_search = button( MSG_SEARCH_BT_SEARCH, 0 ),
				Child, bt_close = button( MSG_CLOSE, 0 ),
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->myvobj = (APTR)GetTagData( MA_Searchwin_HTMLView, 0, msg->ops_AttrList );
	data->str_search = str_search;
	data->cyc_case = cyc_case;

	SetAttrs( cyc_case,
		MUIA_Cycle_Active, getflag( VFLG_SEARCHCASE ),
		MUIA_CycleChain, 1,
		TAG_DONE
	);
	set( str_search, MUIA_CycleChain, 1 );
	set( obj, MUIA_Window_DefaultObject, str_search );
	set( obj, MUIA_Window_ActiveObject, str_search );

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Search_Close
	);
	DoMethod( bt_search, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Search_Start
	);
	DoMethod( str_search, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Search_Start
	);
	DoMethod( bt_close, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Search_Close
	);

	DoMethod( bt_search, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 3, MUIM_Set, MUIA_Window_ActiveObject, str_search
	);
	DoMethod( str_search, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		obj, 3, MUIM_Set, MUIA_Window_ActiveObject, str_search
	);

	return( (ULONG)obj );
}


DECMETHOD( Search_Close, ULONG )
{
	GETDATA;

	setflag( VFLG_SEARCHCASE, getv( data->cyc_case, MUIA_Cycle_Active ) );

/*
	TOFIX
	DoMethod( data->myvobj, MM_HTMLView_SearchWinClosed );
*/
	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( app, OM_REMMEMBER, obj );
	MUI_DisposeObject( obj );

	return( 0 );
}

DECMETHOD( Search_Start, ULONG )
{
	GETDATA;

	if( !*getstrp( data->str_search ) )
	{
		displaybeep();
		return( 0 );
	}

	set( _app( obj ), MUIA_Application_Sleep, TRUE );
/*
	TOFIX
	DoMethod( data->myvobj, MM_HTMLView_DoFind,
		getstrp( data->str_search ),
		getv( data->cyc_case, MUIA_Cycle_Active )
	);
*/
	set( _app( obj ), MUIA_Application_Sleep, FALSE );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMETHOD( Search_Close )
DEFMETHOD( Search_Start )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_searchwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "SearchWinClass";
#endif

	return( TRUE );
}

void delete_searchwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getsearchwinclass( void )
{
	return( mcc->mcc_Class );
}
