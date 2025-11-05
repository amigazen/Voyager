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
 * Menu functions
 * --------------
 * - Context menus, normal menus, etc...
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: menus.c,v 1.16 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/gadtools.h>
#include <proto/exec.h>
#endif

/* private */
#include "menus.h"
#include "voyager_cat.h"
#include "prefs.h"
#include "mui_func.h"

struct v_tempobj {
	struct MinNode n;
	APTR obj;
};


/*
 * Emergency function to cleanup the
 * "recursive" replacement list
 */
static void context_menu_cleanup( struct MinList *l, APTR obj )
{
	struct v_tempobj *vto;

	while( vto = REMTAIL( l ) )
	{
		free( vto );
	}
	MUI_DisposeObject( obj );
}



/*
 * Builds a complete context menu object depending on the
 * object's type.
 */
static ULONG context_menu_struct( int menumode, APTR menustripobj, APTR menuobj, ULONG userdatanum )
{
	APTR m;
	ULONG c;
	ULONG mode[ 4 ]; //TOFIX!! hm, perhaps not 4... well I'm lazy atm. check later

	/*
	 * Set the right context menu
	 */
	switch( menumode )
	{
		case VMT_PAGEMODE:
			D( db_gui, bug( "VMT_PAGEMODE chosen\n" ) );
			mode[ MODE_LABELS ] = DSI_CMENUS_PAGE_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_PAGE_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_PAGE_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_PAGE_DEPTH;
			break;
		
		case VMT_LINKMODE:
			D( db_gui, bug( "VMT_LINKMODE chosen\n" ) );
			mode[ MODE_LABELS ] = DSI_CMENUS_LINK_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_LINK_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_LINK_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_LINK_DEPTH;
			break;

		case VMT_IMAGEMODE:
			D( db_gui, bug( "VMT_IMAGEMODE chosen\n" ) );
			mode[ MODE_LABELS ] = DSI_CMENUS_IMAGE_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_IMAGE_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_IMAGE_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_IMAGE_DEPTH;
			break;
	}

	/*
	 * PageMode is always there. We use a list to
	 * avoid recursions to avoid possible stack
	 * overflow.
	 */
	
	/*
	 * First check if there are menus configured..
	 */
	if( getprefsstr( mode[ MODE_LABELS ], "" )[ 0 ] )
	{
		struct MinList l;
		struct v_tempobj *vto;
		ULONG depth;
		APTR current_obj = menuobj;
		APTR current_obj_lurking = 0; /* to make compiler happy */
		ULONG current_depth = 0;

		NEWLIST( &l );

		for( c = 0; getprefsstr( mode[ MODE_LABELS ] + c, "" )[ 0 ] || ( getprefslong( mode[ MODE_ACTION ] + c, 0 ) == BFUNC_BAR ); c++ )
		{
			D( db_gui, bug( "adding menu %s\n", ( getprefslong( mode[ MODE_ACTION ] + c, 0 ) == BFUNC_BAR ) ? "---" : getprefsstr( mode[ MODE_LABELS ] + c, "" ) ) );
			depth = getprefslong( mode[ MODE_DEPTH ] + c, 0 );

			/*
			 * Let's handle the changing
			 * of the positions (horizontaly)
			 */
			if( depth > current_depth )
			{
				D( db_gui, bug( "->\n" ) );
				if( vto = malloc( sizeof( struct v_tempobj ) ) )
				{
					/* save return object */
					vto->obj = current_obj = current_obj_lurking;
					ADDTAIL( &l, vto );
				}
				else
				{
					/* argh, no mem. cleanup and abort */
					context_menu_cleanup( &l, menustripobj );
					return( 0 );
				}
			}
			else if( depth < current_depth )
			{
				/* going back to previous level */
				/* how much ? */
				ULONG b = current_depth - depth;

				D( db_gui, bug( "<-\n" ) );

				while( b-- )
				{
					vto = REMTAIL( &l );
					free( vto );
				}

				if( ISLISTEMPTY( &l ) )
				{
					current_obj = menuobj;
				}
				else
				{
					current_obj = LASTNODE( &l );
				}
			}
			else
			{
				D( db_gui, bug( "==\n" ) );
				/* same level */

			}

			if( m = MenuitemObject, MUIA_Menuitem_Title, ( getprefslong( mode[ MODE_ACTION ] + c, 0 ) == BFUNC_BAR ) ? NM_BARLABEL : getprefsstr( mode[ MODE_LABELS ] + c, "" ), MUIA_UserData, userdatanum, End )
			{
				D( db_gui, bug( "adding menuobject\n" ) );
				DoMethod( current_obj, OM_ADDMEMBER, m );
						
				current_obj_lurking = m;
				current_depth = depth;
			}
			else
			{
				context_menu_cleanup( &l, menustripobj );
				return( 0 );
			}
			userdatanum++;
		}
		
		/*
		 * And finally cleanup.
		 */
		while ( vto = REMTAIL( &l ) )
		{
			free( vto );
		}

		return( c );
	}
	return( 0 );
}


/*
 * Builds a context menu and returns its structure.
 * split1 is a reference to the number where the second menu is
 * added (currently only used by VMT_IMAGELINKMODE). Is it OK to
 * supply NULL if you're using another menumode type.
 */
APTR build_context_menu( int menumode, APTR obj, ULONG *split1 )
{
	APTR m, sep;
	STRPTR title;
	ULONG count;

	switch( menumode )
	{
		case VMT_PAGEMODE:
			D( db_gui, bug( "VMT_PAGEMODE chosen\n" ) );
			title = GS( CMENU_PAGEMODE );
			break;
		
		case VMT_LINKMODE:
			D( db_gui, bug( "VMT_LINKMODE chosen\n" ) );
			title = GS( CMENU_LINKMODE );
			break;

		case VMT_IMAGEMODE:
			D( db_gui, bug( "VMT_IMAGEMODE chosen\n" ) );
			title = GS( CMENU_IMAGEMODE );
			break;

		case VMT_IMAGELINKMODE:
			D( db_gui, bug( "VMT_IMAGELINKMODE chosen\n" ) );
			title = GS( CMENU_IMAGELINKMODE );
			break;

		default:
			title = "ollinase";
			break;
	}

	if( m = MenuObjectT( title ), End )
	{
		DoMethod( obj, OM_ADDMEMBER, m );
		
		if( ( count = context_menu_struct( ( menumode == VMT_IMAGELINKMODE ) ? VMT_IMAGEMODE : menumode, obj, m, 0 ) ) )
		{
			if( menumode == VMT_IMAGELINKMODE )
			{
				*split1 = count;

				if( sep = MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End )
				{
					DoMethod( m, OM_ADDMEMBER, sep );

					if( context_menu_struct( VMT_LINKMODE, obj, m, count ) )
					{
						return( obj );
					}
				}
			}
			else
			{
				return( obj );
			}
		}
	}
	return( NULL );
}

