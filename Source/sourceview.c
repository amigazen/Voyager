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
** $Id: sourceview.c,v 1.31 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "voyager_cat.h"
#include "classes.h"
#include "network.h"
#include "mui_func.h"
#include "htmlwin.h"
#include "win_func.h"
#include "textinput.h"

#define FWM_CLOSE (TAG_USER+(137<<16)|89)
#define FWA_URL (FWM_CLOSE+1)
#define FWM_CHANGE (FWM_CLOSE+2)

struct Data {
	APTR lv;
	struct nstream *urln;
};

DECNEW
{
	struct nstream *urln = (APTR)GetTagData( MA_Sourceview_URL, 0, msg->ops_AttrList );
	APTR lv, bt_save, bt_close, bt_change;
	struct Data *data;

	obj = DoSuperNew( cl, obj, 
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, "Voyager · Source view",
		MUIA_Window_ID, MAKE_ID('S','V','I','E'),
		MUIA_Window_RootObject, VGroup,
			Child, HGroup,
				Child, Label2( ( ULONG )"URL:" ),
				Child, TextinputObject,
					TextFrame,
					MUIA_Background, MUII_TextBack,
					MUIA_Textinput_NoInput, TRUE,
					MUIA_Text_Contents, nets_url( urln ),
					MUIA_Font, MUIV_Font_Tiny,
					MUIA_Text_SetMin, FALSE,
				End,
			End,
			Child, lv = TextinputscrollObject, StringFrame,
				MUIA_Textinput_Multiline, TRUE,
				MUIA_Textinput_Styles, MUIV_Textinput_Styles_HTML,
				MUIA_Textinput_Font, MUIV_Textinput_Font_Fixed,
			End,
			Child, HGroup,
				Child, bt_save = makebutton( MSG_SOURCEVIEW_BT_SAVE ),
				Child, bt_change = makebutton( MSG_SOURCEVIEW_BT_CHANGE ),
				Child, bt_close = makebutton( MSG_CLOSE ),
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	data->lv = lv;

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_Sourceview_Close
	);
	DoMethod( bt_close, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_Sourceview_Close
	);

	set( bt_save, MUIA_Disabled, TRUE );

	set( obj, MUIA_Window_DefaultObject, lv );

	set( lv, MUIA_String_Contents, nets_getdocmem( urln ) );

	// Deliberately not closing window
	DoMethod( bt_change, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )obj, 1, MM_Sourceview_Update
	);

	data->urln = nets_open( nets_url( urln ), NULL, NULL, NULL, NULL, 0, 0 );

	return( (ULONG)obj );
}

DECMETHOD( Sourceview_Close, APTR )
{
	GETDATA;

	nets_close( data->urln );
	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );
	MUI_DisposeObject( obj );
	return( 0 );
}

DECMETHOD( Sourceview_Update, APTR )
{
	GETDATA;
	nets_setdocmem( data->urln, getstrp( data->lv ), -1 );
	//doallwins( MUIM_Window_Recalc );
	/* TOFIX: should be reimplemented */
	//doallwins( MM_HTMLView_Redraw );
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMETHOD( Sourceview_Close )
DEFMETHOD( Sourceview_Update )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_sourceviewclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "SourceViewClass";
#endif

	return( TRUE );
}

void delete_sourceviewclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getsourceview( void )
{
	return( mcc->mcc_Class );
}

int opensourceview( struct nstream *urln )
{
	APTR o;

	if( !urln )
	{
		displaybeep();
		return( FALSE );
	}

	o = NewObject( getsourceview(), NULL, MA_Sourceview_URL, urln, TAG_DONE );
	if( o )
	{
		DoMethod( app, OM_ADDMEMBER, ( ULONG )o );
		set( o, MUIA_Window_Open, TRUE );
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}
