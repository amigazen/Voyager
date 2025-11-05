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
 * Clock display class
 * -------------------
 * - Stupid clock for RobR so that he stops whining.
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: clock.c,v 1.14 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/gadtools.h>
#endif

/* private */
#include "classes.h"
#include "mui_func.h"
#include "prefs.h"


#if USE_CLOCK

struct Data {
	int display_seconds; /* bool flag, seconds enabled or not */
	LONG hour; /* current hour */
	LONG minute; /* current minute */
	struct MUI_InputHandlerNode ihnode;
	APTR cmenu;
	APTR grp_clock;
	APTR txt_hours;
	APTR txt_hours_sep;
	APTR txt_minutes;
	APTR txt_minutes_sep;
	APTR txt_seconds;
};

/*
 * Context menu options
 */
#define CMENU_SECONDS 1


/*
 * Clock display modes
 */
enum {
	CDM_NORMAL,
	CDM_SECONDS,
};


static struct NewMenu nmcontext[] = { //TOFIX!! add locale
	{ NM_TITLE, "Display", 0, 0, 0, 0 },
	{ NM_ITEM, "Seconds", 0, CHECKIT | MENUTOGGLE, 0, ( APTR )CMENU_SECONDS },
	NM_END
};

DECNEW
{
	struct Data *data;
	APTR grp_clock;
	APTR txt_hours, txt_hours_sep, txt_minutes;

	obj = DoSuperNew( cl, obj,
		Child, grp_clock = HGroup,
			MUIA_Group_Spacing, 0,
			TextFrame, 
			MUIA_Background, MUII_TextBack,
			Child, txt_hours = TextObject,
				MUIA_Text_SetMax, TRUE,
				MUIA_Font, MUIV_Font_Tiny,
			End,
			Child, txt_hours_sep = TextObject,
				MUIA_Text_SetMax, TRUE,
				MUIA_Font, MUIV_Font_Tiny,
				MUIA_Text_Contents, ":",
			End,
			Child, txt_minutes = TextObject,
				MUIA_Text_SetMax, TRUE,
				MUIA_Font, MUIV_Font_Tiny,
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->grp_clock = grp_clock;
	data->txt_hours = txt_hours;
	data->txt_hours_sep = txt_hours_sep;
	data->txt_minutes = txt_minutes;
	data->hour = -1;
	data->minute = -1;

	/*
	 * Context menu
	 */
	if( data->cmenu = MUI_MakeObject( MUIO_MenustripNM, ( ULONG )nmcontext, 0 ) )
	{
		set( obj, MUIA_ContextMenu, data->cmenu );
		DoMethod( data->cmenu, MUIM_SetUData, CMENU_SECONDS, MUIA_Menuitem_Checked, getflag( VFLG_CLOCK_SECONDS ) );
	}

	DoMethod( obj, MM_Clock_Tick );

	return( ( ULONG )obj );
}


DECMMETHOD( Setup )
{
	GETDATA;

	if( !DOSUPER )
	{
		return( FALSE );
	}

	data->ihnode.ihn_Object = obj;
	data->ihnode.ihn_Flags = MUIIHNF_TIMER;
	data->ihnode.ihn_Millis = 1000;
	data->ihnode.ihn_Method = MM_Clock_Tick;

	DoMethod( _app( obj ), MUIM_Application_AddInputHandler, &data->ihnode );

	return( TRUE );
}


DECMMETHOD( Cleanup )
{
	GETDATA;

	DoMethod( _app( obj ), MUIM_Application_RemInputHandler, &data->ihnode );

	return( DOSUPER );
}


DECMETHOD( Clock_Tick, ULONG )
{
	GETDATA;
	time_t t;
	struct tm tm;

	time( &t );
	tm = *localtime( &t );

	if( data->display_seconds )
	{
		DoMethod( data->txt_seconds, MUIM_SetAsString, MUIA_Text_Contents, "%02ld", tm.tm_sec );
	}
	else
	{
		set( data->txt_hours_sep, MUIA_Text_Contents, ( tm.tm_sec % 2 ) ? " " : ":" );
	}

	if( data->minute != tm.tm_min )
	{
		DoMethod( data->txt_minutes, MUIM_SetAsString, MUIA_Text_Contents, "%02ld", tm.tm_min );
		data->minute = tm.tm_min;

		if( data->hour != tm.tm_hour )
		{
			DoMethod( data->txt_hours, MUIM_SetAsString, MUIA_Text_Contents, "%02ld", tm.tm_hour );
			data->hour = tm.tm_hour;
		}
	}

	return( 0 );
}


DECSMETHOD( Clock_ChangeDisplay )
{
	GETDATA;
	int x;

	DoMethod( data->grp_clock, MUIM_Group_InitChange );

	switch( msg->mode )
	{
		case CDM_NORMAL:
			DoMethod( data->grp_clock, OM_REMMEMBER, data->txt_minutes_sep );
			MUI_DisposeObject( data->txt_minutes_sep );
			DoMethod( data->grp_clock, OM_REMMEMBER, data->txt_seconds );
			MUI_DisposeObject( data->txt_seconds );
			data->display_seconds = FALSE;
			break;

		case CDM_SECONDS:
			data->txt_minutes_sep = TextObject,
				MUIA_Text_SetMax, TRUE,
				MUIA_Font, MUIV_Font_Tiny,
				MUIA_Text_Contents, ":",
			End;

			if( data->txt_minutes_sep )
			{
				data->txt_seconds = TextObject,
					MUIA_Text_SetMax, TRUE,
					MUIA_Font, MUIV_Font_Tiny,
					MUIA_Text_Contents, "00",
				End;

				if( data->txt_seconds )
				{
					DoMethod( data->grp_clock, OM_ADDMEMBER, data->txt_minutes_sep );
					DoMethod( data->grp_clock, OM_ADDMEMBER, data->txt_seconds );
					data->display_seconds = TRUE;
				
					set( data->txt_hours_sep, MUIA_Text_Contents, ":" );
				}
				else
				{
					MUI_DisposeObject( data->txt_minutes_sep );
				}
			}
			break;
	}

	DoMethod( data->grp_clock, MUIM_Group_ExitChange );
	DoMethod( obj, MM_Clock_Tick );

	if( data->cmenu )
	{
		DoMethod( data->cmenu, MUIM_GetUData, CMENU_SECONDS, MUIA_Menuitem_Checked, &x );
	}
	setflag( VFLG_CLOCK_SECONDS, x );

	return( 0 );
}


DECMMETHOD( ContextMenuChoice )
{
	GETDATA;
	
	switch( getv( msg->item, MUIA_UserData ) )
	{
		case CMENU_SECONDS:
			DoMethod( obj, MM_Clock_ChangeDisplay, data->display_seconds ? CDM_NORMAL : CDM_SECONDS );
			break;
	}
	return( 0 );
}


DECDISPOSE
{
	GETDATA;

	if( data->cmenu )
	{
		MUI_DisposeObject( data->cmenu );
	}

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( ContextMenuChoice )
DEFMETHOD( Clock_Tick )
DEFSMETHOD( Clock_ChangeDisplay )
ENDMTABLE


static struct MUI_CustomClass *mcc;

int create_clockclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "ClockClass";
#endif

	return( TRUE );
}

void delete_clockclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getclockclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_CLOCK */
