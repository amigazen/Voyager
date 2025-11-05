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
 * Error window class
 * ------------------
 * - A sort of replacement for MUI_Request(). Able to handle any number of errors
 *   in a user friendly fashion as it happens that the old way opened too
 *   many requesters. This classe opens a window with a listview containing
 *   all the errors. As long as the window is open, all the errors end up in
 *   the listview and will be reopened if more errors are comming.
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: errorwin.c,v 1.38 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"
#include "classes.h"
#include "voyager_cat.h"
#include "errorwin.h"
#include "mui_func.h"
#include "methodstack.h"
#include "malloc.h"
#include "nlist.h"

#include <proto/dos.h> /* XXX: debugging */


extern char *filter_escapecodes( char *src ); // imported from htmlwin.c


/* global data */
APTR errorwin;

struct errorlog {
	ULONG type;
	ULONG level;
	char errorcode[ 16 ];
	STRPTR url;
	STRPTR message;
};


/* instance data */
struct Data {
	APTR lv_info;
};


/* hooks */
MUI_HOOK( info_destruct, APTR pool, struct errorlog *el )
{
	if( el->message )
		free( el->message );
	
	if( el->url )
		free( el->url );
	
	free( el );
	
	return( 0 );
}

MUI_HOOK( info_display, STRPTR *array, struct errorlog *el )
{
	if( el )
	{
		*array++ = GSI( MSG_ERRORWIN_ERRORTYPE + el->type );
		*array++ = GSI( MSG_ERRORWIN_ERRORLEVEL + el->level );
		*array++ = el->errorcode[ 0 ] ? el->errorcode : "";
		*array++ = el->url ? el->url : (STRPTR)GS( ERRORWIN_NONE );
		*array   = el->message ? el->message : (STRPTR)GS( ERRORWIN_NONE );
	}
	else
	{
		*array++ = GS( ERRORWIN_TYPE );
		*array++ = GS( ERRORWIN_LEVEL );
		*array++ = GS( ERRORWIN_NUM );
		*array++ = GS( ERRORWIN_URL );
		*array	 = GS( ERRORWIN_MESSAGE );
	}

	return( 0 );
}


#ifdef MBX
#define MUIA_WINDOW_LEFTEDGE MUIA_Window_LeftEdge, 50,
#define MUIA_WINDOW_TOPEDGE	MUIA_Window_TopEdge , 300,
#define MUIA_WINDOW_WIDTH MUIA_Window_Width   , MUIV_Window_Width_Screen(90),
#define MUIA_WINDOW_HEIGHT MUIA_Window_Height  , 20,
#else
#define MUIA_WINDOW_LEFTEDGE
#define MUIA_WINDOW_TOPEDGE
#define MUIA_WINDOW_WIDTH
#define MUIA_WINDOW_HEIGHT
#endif


DECNEW
{
	struct Data *data;
	APTR bt_close, lv_info;

	obj = DoSuperNew(cl, obj,
		MUIA_Window_ID, MAKE_ID( 'E','R','R','S' ),
		MUIA_Window_Title, GS( ERRORWIN_WINTITLE ),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_NoMenus, TRUE,
		MUIA_WINDOW_LEFTEDGE
		MUIA_WINDOW_TOPEDGE
		MUIA_WINDOW_WIDTH
		MUIA_WINDOW_HEIGHT
		WindowContents, VGroup,
			Child, lv_info = MUI_NewObject( listviewclass,
				MUIA_CycleChain, 1,
				MUIA_Listview_Input, FALSE,
				MUIA_Listview_List, MUI_NewObject( listclass,
					ReadListFrame,
					MUIA_List_Title, TRUE,
					MUIA_List_Format, "BAR,BAR,BAR,BAR,BAR",
					MUIA_List_DisplayHook, &info_display_hook,
					MUIA_List_DestructHook, &info_destruct_hook,
				End,
			End,
			Child, bt_close = button( MSG_CLOSE, 0 ),
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA(cl, obj);

	data->lv_info = lv_info;

	DoMethod( bt_close, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_ErrorWin_Close
	);
	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_ErrorWin_Close );

	set( obj, MUIA_Window_ActiveObject, bt_close );

	return( (ULONG)obj );
}


DECMETHOD( ErrorWin_Close, ULONG )
{
	GETDATA;

	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( data->lv_info, MUIM_List_Clear );
	return( 0 );
}


DECSMETHOD( ErrorWin_PutError )
{
	GETDATA;

	/* make that settable in the settings */
	if( !getv( obj, MUIA_Window_Open ) )
	{
		set( obj, MUIA_Window_Open, TRUE );
	}

	DoMethod( data->lv_info, MUIM_List_InsertSingle, msg->el, MUIV_List_Insert_Bottom );
	DoMethod( data->lv_info, MUIM_List_Jump, getv( data->lv_info, MUIA_List_Entries ) - 1 );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMETHOD( ErrorWin_Close )
DEFSMETHOD( ErrorWin_PutError )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_errorwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "ErrorWinClass";
#endif

	return( TRUE );
}

void delete_errorwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR geterrorwinclass( void )
{
	return( mcc->mcc_Class );
}


/*
 * Shows an error into the Error Window. Automatically opens it
 * if needed. May be called from any task. url and message may be NULL.
 */
void puterror( ULONG type, ULONG level, LONG errorcode, STRPTR url, STRPTR message )
{
	struct errorlog *el = NULL;
	
	//DBD(("type %ld, level %ld, code %ld, url %s: %s\n",type,level,errorcode,url,message));

	if( !errorwin )
	{
		static ULONG psmmsg[] = { MM_App_GetErrorWinClass };
		errorwin = ( APTR )pushsyncmethod( app, psmmsg );

		/* pushsyncmethod returns -1 on failure */
		if( errorwin == (APTR)-1 )
			errorwin = NULL;
		
		if( errorwin )
		{
			pushmethod( app, 2, OM_ADDMEMBER, errorwin );
		}
	}

	if( errorwin )
	{
		if( el = malloc( sizeof( struct errorlog ) ) )
		{
			memset( el, 0, sizeof( *el ) );
			if( !message || ( el->message = strdup( filter_escapecodes( message ) ) ) )
			{
				if( !url || ( el->url = strdup( filter_escapecodes( url ) ) ) )
				{
					el->type = type;
					el->level = level;
					
					if( errorcode > 0 )
					{
						stcl_d( el->errorcode, errorcode );
					}
					
					pushmethod( errorwin, 2, MM_ErrorWin_PutError, el );
					return;
				}
			}
		}
	}
	
	if( el )
	{
		if( el->url )
			free( el->url );
	
		if( el->message )
			free( el->message );

		free( el );
	}

	displaybeep();
}

