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
 * Window management functions
 * ---------------------------
 * - Handles the numbering of window, adding them to the menu,
 *   setting their title, etc... Currently only blocking window (which
 *   prevent V from closing as long as they're there) can be handled
 *   that way
 *
 * © 2000-2003 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: win_func.c,v 1.42 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "copyright.h"
#include "menus.h"
#include "htmlwin.h"
#include "malloc.h"
#include "rexx.h"
#include "classes.h"
#include "htmlclasses.h"
#include "prefs.h"
#include "mui_func.h"
#include "win_func.h"
#include "js.h"

#define V_MAX_WINDOWS 255 /* maximum number of windows */

/*
 * Summary on how to handle a window properly:
 * - get a window number with get_window_number() if you need it for your MAKE_ID() macro
 * - add_window_menu() with your title and your number you previously go with get_window_number()
 * - set_window_close() if you need to close it
 * - and remove_window_menu() from your OM_DISPOSE method
 */

static struct MinList global_window_list;

struct v_window {
	struct MinNode node;
	APTR obj;
	APTR winobj;
	ULONG num; /* number of the window */
	char win_title[ 128 ];
	char menu_title[ 48 ];
	char menu_shortcut[ 4 ];
	APTR menuitem;
	int closeme; /* set if the window can exiting */
	int removeme; /* set if we can free the structure */
};


void init_global_window_list( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &global_window_list );
}


/*
 * This function is used to get a free window number. Preferably
 * to make your MAKE_ID() look unique. Then you can call add_window_menu().
 * 0 means that the current limit of V_MAX_WINDOWS has been reached
 * and that you shouldn't create your window.
 */
ULONG get_window_number( void )
{
	struct v_window *v_win;
	ULONG num = 0;
	int exitloop = FALSE;

	while( num < V_MAX_WINDOWS && !exitloop )
	{
		num++;
		if( ISLISTEMPTY( &global_window_list ) )
		{
			break;
		}

		exitloop = TRUE;

		ITERATELIST( v_win, &global_window_list )
		{
			if( num == v_win->num )
			{
				exitloop = FALSE;
				break;
			}
		}
	}

	if( num > V_MAX_WINDOWS )
	{
		return( 0 );
	}
	else
	{
		return( num );
	}
}


/* private function */
void set_window_title_internal( APTR obj, STRPTR name, struct v_window *v_win )
{
	if( !v_win )
	{
		int found = FALSE;

		ITERATELIST( v_win, &global_window_list )
		{
			if( v_win->obj == obj )
			{
				found = TRUE;
				break;
			}
		}
		if( !found )
		{
			return;
		}
	}

	/* Window title */
	sprintf( v_win->win_title, "[%ld] " APPNAME " · %.100s", v_win->num, name );
#if !USE_STB_NAV
	// In STB mode, we don't have window titles at all
	set( v_win->winobj ? v_win->winobj : obj, MUIA_Window_Title, v_win->win_title );
#endif
	
	/* Menuitem title */
	if( v_win->num < 11 )
	{
		sprintf( v_win->menu_shortcut, "%ld", v_win->num < 10 ? v_win->num : 0 );
	}

#if USE_STB_NAV
	// No number for STB nav
	sprintf( v_win->menu_title, "%.32s", name );
#else
	sprintf( v_win->menu_title, "[%ld] %.32s", v_win->num, name );
#endif
	if( strlen( v_win->menu_title ) > 32 )
	{
		strcat( v_win->menu_title, "..." );
	}
}


/*
 * Sets the window title and updates the menu title as well.
 * Only use this functions to update it since it's already set from
 * add_window_menu().
 */
void set_window_title( APTR obj, STRPTR name )
{
	set_window_title_internal( obj, name, NULL );
#if USE_SINGLEWINDOW
	if( gp_singlewindow )
		doallwins( MM_HTMLWin_UpdatePageTitles );
#endif
}


/*
 * First function to call from OM_NEW. Adds the window to
 * the menu and the internal list. If the return value is FALSE
 * do not call any other functions as there was not enough
 * memory or something. Either don't open your window or
 * continue. add_window_menu_extended() is needed for objects
 * which are subclass of something else than MUIC_Window but
 * still can have a Window ( like htmlwin.c )
 */
void add_window_menu_extended( APTR obj, APTR winobj, STRPTR name, ULONG num, int settitle )
{
	struct v_window *v_win;

	if( v_win = malloc( sizeof( struct v_window ) ) )
	{
		memset( v_win, '\0', sizeof( struct v_window ) );

		v_win->obj = obj;
		v_win->num = num;
		v_win->winobj = winobj;

		D( db_gui, bug( "adding object 0x%lx\n", obj ) );

		if (settitle)
			set_window_title_internal( obj, name, v_win );

#if USE_MENUS
		if (( v_win->menuitem = MUI_MakeObject( MUIO_Menuitem, ( ULONG )v_win->menu_title, num < 11 ? ( ULONG )v_win->menu_shortcut : ( ULONG )NULL, 0, ( ULONG )20000 + num ) ))
		{
			D( db_gui, bug( "adding menuitem 0x%lx\n", v_win->menuitem ) );

			if (menu && findmenu( MENU_WINDOWS ))
				DoMethod( findmenu( MENU_WINDOWS ), MUIM_Family_AddTail, v_win->menuitem );
		
			DoMethod( v_win->menuitem, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
				winobj, 3, MUIM_Set, MUIA_Window_Activate, TRUE);

			DoMethod( v_win->menuitem, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
				winobj, 1, MUIM_Window_ToFront);
		}
#endif
		ADDTAIL( &global_window_list, v_win );
	}
}

void add_window_menu( APTR obj, STRPTR name, ULONG num )
{
	add_window_menu_extended( obj, NULL, name, num, TRUE );
}


/*
 * Removes everything that was added with add_window_menu().
 * Call it in OM_DISPOSE as it frees the structure thus your window
 * must NOT be opened.
 */
void remove_window_menu( APTR obj )
{
	struct v_window *v_win;

	D( db_gui, bug( "called\n" ) );

	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->obj == obj )
		{
#if USE_MENUS
			if (v_win->menuitem)
			{
				D( db_gui, bug( "kill menuitem %08lx\n",v_win->menuitem ) );

				if (menu && findmenu( MENU_WINDOWS ))
					DoMethod( findmenu( MENU_WINDOWS ), MUIM_Family_Remove, v_win->menuitem );

				MUI_DisposeObject( v_win->menuitem );
				v_win->menuitem = NULL;
			}
#endif
			v_win->removeme = TRUE;
		}
	}

	D( db_gui, bug( "finished\n" ) );
}


/*
 * Sets the 'close' status flag of a window
 */
void set_window_close( APTR obj )
{
	struct v_window *v_win;

	D( db_gui, bug( "called ( obj == 0x%lx )\n", obj ) );

	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->obj == obj )
		{
			D( db_gui, bug( "object 0x%lx's flag mode set to close\n", obj ) );
			v_win->closeme = TRUE;
			break;
		}
	}
}


/*
 * Closes windows that can be closed
 */
void checkwinremove( void )
{
	struct v_window *v_win;

	D( db_gui, bug( "called\n" ) );

	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->closeme && v_win->obj )
		{
			Object *w = v_win->winobj ? v_win->winobj : v_win->obj;

			D( db_gui, bug( "closing object 0x%lx now. win=%08lx\n", v_win->obj,w ) );

			DoMethod( w, MM_HTMLWin_ExportTearoff );
			set( w, MUIA_Window_Open, FALSE );
			DoMethod( app, OM_REMMEMBER, w);
			if( v_win->obj )
				DoMethod( v_win->obj, MM_HTMLWin_ToDormant );
			remove_window_menu( v_win->obj ); /* TOFIX: this is a fucking mess */
			MUI_DisposeObject( w );

			v_win->removeme = TRUE;
		}
	}


	/*
	 * Garbage collector :)
	 */
	cwr_restart:
	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->removeme )
		{
			D( db_gui, bug( "removing from ( now defunct ) object 0x%lx..\n", v_win->obj ) );
			REMOVE( v_win );
			free( v_win );
			goto cwr_restart;
		}
	}

	/*
	 * We only exit if there's no remaining windows and downloads
	 */
	if( ISLISTEMPTY( &global_window_list ) )
	{
		DoMethod( app, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit );
	}
	else
	{
		js_gc();
	}
}


/*
 * Returns the current active window number (or previously active one)
 */
ULONG get_active_window( void )
{
	struct v_window *v_win;
	
	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->winobj )
		{
			if( v_win->winobj == lastactivewin )
			{
				break;
			}
		}
		else
		{
			if( v_win->obj == lastactivewin )
			{
				break;
			}
		}
	}
	return( v_win->num );
}


/*
 * Executes the following method on ALL windows that have
 * been added using add_window_menu()
 */
#ifdef __MORPHOS__
void STDARGS _doallwins( int stub, ... )
{
	va_list va;
	struct v_window *v_win;
	
	va_start( va, stub );

	ITERATELIST( v_win, &global_window_list )
	{
		DoMethodA( v_win->obj, ( Msg )va->overflow_arg_area );
	}
	va_end( va );
}

#else

void STDARGS doallwins( ULONG methodid, ... )
{
	struct v_window *v_win;

	ITERATELIST( v_win, &global_window_list )
	{
		DoMethodA( v_win->obj, ( Msg )&methodid );
	}
}

#endif /* !__MORPHOS__ */

/*
 * Same as above but with an array
 */
void doallwins_a( Msg msg )
{
	struct v_window *v_win;

	ITERATELIST( v_win, &global_window_list )
	{
		DoMethodA( v_win->obj, msg );
	}
}


/*
 * Same as above but on a certain window
 */
int	donumwins_a( ULONG num, Msg msg )
{
	struct v_window *v_win;

	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->num == num )
		{
			if( command_runmode == VREXX_WINDOW )
			{
				return( ( int )DoMethodA( v_win->obj, msg ) );
			}
			else
			{
				return( ( int )DoMethod( v_win->obj, MM_HTMLWin_DoActiveFrame, msg ) );
			}
		}
	}
	return( 10 );
}


/*
 * Force the removal of all windows
 */
void cleanup_windows( void )
{
	struct v_window *v_win;

	D( db_init, bug( "cleaning up..\n" ) );

	//TOFIX!! islistempy needed ?

	//TOFIX!! ask Olli what he plans to do with the following
	//for( win = FIRSTNODE( &winlist ); next = NEXTNODE( win ); win = next )
	//{
	//	  if( !win->owner && !win->winobj )
	//	  {
	//		  MUI_DisposeObject( win->obj );
	//	  }
	//	  else if( win->winobj )
	//	  {
	//		  set( win->winobj, MUIA_Window_Open, FALSE );
	//		  DoMethod( app, OM_REMMEMBER, win->winobj );
	//		  MUI_DisposeObject( win->winobj );
	//	  }
	//}

	ITERATELIST( v_win, &global_window_list )
	{
		v_win->closeme = TRUE;
	}

	checkwinremove();
}

/*
 * Find a window by it's JS/HTML "name"
 */
APTR win_find_by_name( STRPTR name, int toponly )
{
	struct v_window *v_win;

	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->obj )
		{
			if( toponly )
			{
				char *thisname = 0;

				get( v_win->obj, MA_JS_Name, &thisname );
				if( thisname )
				{
					if( !strcmp( name, thisname ) )
						return( v_win->obj );
				}
			}
			else
			{
				APTR thisobj = NULL;

				DoMethod( v_win->obj, MM_HTMLWin_FindByName, name, &thisobj );
				if( thisobj )
					return( thisobj );
			}
		}
	}

	return( NULL );
}

/*
 * Returns a window by "index"
 */
APTR get_window_by_ix( int ix )
{
	struct v_window *v_win;
	
	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->winobj )
		{
			if( !ix-- )
				return( v_win->obj );
		}
	}

	return( NULL );
}

/*
 * Get the menu shortcut title of a window
 */
STRPTR get_window_menu_title( int ix )
{
	struct v_window *v_win;
	
	ITERATELIST( v_win, &global_window_list )
	{
		if( v_win->winobj )
		{
			if( !ix-- )
				return( v_win->menu_title );
		}
	}

	return( NULL );
}

/*
 * Set the smooth scroll mode.
 */
void set_smooth_scroll( APTR obj, int mode )
{
	if( gp_smooth_scroll )
	{
		set( obj, MUIA_Virtgroup_Smooth, mode );
	}
}

