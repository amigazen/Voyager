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
 * Context menu settings
 * ---------------------
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_contextmenu.c,v 1.37 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "prefswin.h"
#include "mui_func.h"
#include "menus.h"
#include "rexx.h"
#include "listtree.h"

/*
 * Remove that define once the Listtree bugs are fixed. Well, since I have the
 * sources I guess it'll be my job [zapek]
 */
#define LISTTREE_SUX


#define PWCM_OPTSNUM 3
#define PWCM_LABELMAX 32 /* maximum length of a menu label */
#define PWCM_MAX_SUBMENU_DEPTH 1 /* maximum depth of submenus TOFIX!! check how much MUI 3.8 supports and change that for 3.9+ */

/* Bar mode */
enum {
	BM_NORMAL,
	BM_BAR
};


/*          
 * Menu structure
 */
struct vp_contextmenu {
	LONG action;
	char args[ VREXX_MAXLENGTH ];
	ULONG num;	/* number to be able to know what entry is being pointed */
};

struct vp_iocontextmenu {
	struct MinNode node;
	APTR ln;
};

struct vp_savecontextmenu {
	struct MinNode node;
	APTR ln;
	LONG pos;
};

struct Data {
	APTR lt_menu[ PWCM_OPTSNUM ];
	APTR str_label;
	APTR pop_cmd;
	APTR cyc_type;
	APTR grp_type;
	APTR cyc_bar;
	APTR grp_edit;
	int display_lock; /* lock to not trigger a notify when adding with the 'add' button */
	struct MUIS_Listtree_TreeNode *previous_active_tn;
	ULONG active_type;
};


DECNEW
{
	static STRPTR context_opts[ PWCM_OPTSNUM + 1 ];
	static STRPTR bar_opts[ 3 ];

	struct Data *data;
	APTR lt_menu[ PWCM_OPTSNUM ], bt_add, bt_remove, grp_edit, cyc_bar, str_label, pop_cmd, cyc_type, grp_type;
	ULONG i;

	fillstrarray( context_opts, MSG_PREFSWIN_CONTEXTMENU_OPTS_1, PWCM_OPTSNUM );
	fillstrarray( bar_opts, MSG_PREFSWIN_BARTYPE_OPTS_1, 2 );

	obj = DoSuperNew( cl, obj,
		GroupFrameT( GS( PREFSWIN_CONTEXTMENU_TITLE ) ),
		
		Child, HGroup,
			Child, Label2( GS( PREFSWIN_CONTEXTMENU_TYPE ) ),
			Child, cyc_type = CycleObject,
				MUIA_CycleChain, TRUE,
				MUIA_Cycle_Entries, context_opts,
			End,
		End,
		
		Child, grp_type = PageGroup,
			Child, ListviewObject,
				InputListFrame,
				MUIA_CycleChain, 1,
				MUIA_Listview_List, lt_menu[ VMT_PAGEMODE ] = ListtreeObject,
				End,
			End,

			Child, ListviewObject,
				InputListFrame,
				MUIA_CycleChain, 1,
				MUIA_Listview_List, lt_menu[ VMT_LINKMODE ] = ListtreeObject,
				End,
			End,

			Child, ListviewObject,
				InputListFrame,
				MUIA_CycleChain, 1,
				MUIA_Listview_List, lt_menu[ VMT_IMAGEMODE ] = ListtreeObject,
				End,
			End,
		End,

		Child, HGroup,
			Child, bt_add = button( MSG_PREFSWIN_CONTEXTMENU_ADD, 0 ),
			//Child, bt_addsub = button( MSG_PREFSWIN_CONTEXTMENU_ADDSUB, 0 ), //TOFIX!!
			Child, bt_remove = button( MSG_PREFSWIN_CONTEXTMENU_REMOVE, 0 ),
		End,

		Child, grp_edit = ColGroup( 2 ),
			MUIA_Disabled, TRUE,
			Child, cyc_bar = CycleObject, MUIA_Font, MUIV_Font_Button, MUIA_HorizWeight, 0, MUIA_Cycle_Entries, bar_opts, End,
			Child, str_label = string( "", PWCM_LABELMAX, 0 ),
			
			Child, Label2( GS( PREFSWIN_CONTEXTMENU_ACTION_LABEL ) ),
			Child, pop_cmd = NewObject( getcommandclass(), NULL, TAG_DONE ),

		End,

	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->lt_menu[ VMT_PAGEMODE ] = lt_menu[ VMT_PAGEMODE ];
	data->lt_menu[ VMT_LINKMODE ] = lt_menu[ VMT_LINKMODE ];
	data->lt_menu[ VMT_IMAGEMODE ] = lt_menu[ VMT_IMAGEMODE ];
	data->str_label = str_label;
	data->pop_cmd = pop_cmd;
	data->cyc_type = cyc_type;
	data->grp_type = grp_type;
	data->cyc_bar = cyc_bar;
	data->grp_edit = grp_edit;

	/*
	 * Load the prefs
	 */
	for( i = 0; i < PWCM_OPTSNUM; i++ )
	{
		DoMethod( obj, MM_Prefswin_ContextMenu_LoadTree, i );
	}

	DoMethod( cyc_type, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		obj, 2, MM_Prefswin_ContextMenu_SetType, MUIV_TriggerValue
	);

	DoMethod( lt_menu[ VMT_PAGEMODE ], MUIM_Notify, MUIA_Listtree_Active, MUIV_EveryTime,
		obj, 2, MM_Prefswin_ContextMenu_ChangeEditGroup, MUIV_TriggerValue
	);
	DoMethod( lt_menu[ VMT_LINKMODE ], MUIM_Notify, MUIA_Listtree_Active, MUIV_EveryTime,
		obj, 2, MM_Prefswin_ContextMenu_ChangeEditGroup, MUIV_TriggerValue
	);
	DoMethod( lt_menu[ VMT_IMAGEMODE ], MUIM_Notify, MUIA_Listtree_Active, MUIV_EveryTime,
		obj, 2, MM_Prefswin_ContextMenu_ChangeEditGroup, MUIV_TriggerValue
	);
 
	DoMethod( str_label, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
		obj, 2, MM_Prefswin_ContextMenu_Rename, MUIV_TriggerValue
	);

	/*
	 * Label, Bar stuff
	 */
	DoMethod( cyc_bar, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		str_label, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue
	);
	DoMethod( cyc_bar, MUIM_Notify, MUIA_Cycle_Active, BM_BAR,
		obj, 2, MM_Prefswin_ContextMenu_Rename, "---"
	);
	//DoMethod( cyc_bar, MUIM_Notify, MUIA_Cycle_Active, BM_NORMAL,
	//	  obj, 2, MM_Prefswin_ContextMenu_Rename, ""
	//);
	DoMethod( cyc_bar, MUIM_Notify, MUIA_Cycle_Active, BM_NORMAL,
		str_label, 3, MUIM_Set, MUIA_String_Contents, ""
	);

	/*
	 * Buttons
	 */
	DoMethod( bt_add, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_ContextMenu_Add
	);

	DoMethod( bt_remove, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_ContextMenu_Remove
	);
	//TOFIX!!
	//DoMethod( bt_addsub, MUIM_Notify, MUIA_Pressed, FALSE,
	//	  obj, 1, MM_Prefswin_ContextMenu_AddSub
	//);

	/*
	 * AddSub button
	 */
	//DoMethod( grp_edit, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime,
	//	  bt_addsub, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue
	//);

	/*
	 * Set Listtree for Page mode ( TOFIX!! perhaps we could remember that )
	 */
	DoMethod( obj, MM_Prefswin_ContextMenu_SetType, VMT_PAGEMODE );

	/*
	 * Notify when changing entry
	 */
	DoMethod( lt_menu[ VMT_PAGEMODE ], MUIM_Notify, MUIA_Listtree_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_ContextMenu_DisplayFields
	);
	DoMethod( lt_menu[ VMT_LINKMODE ], MUIM_Notify, MUIA_Listtree_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_ContextMenu_DisplayFields
	);
	DoMethod( lt_menu[ VMT_IMAGEMODE ], MUIM_Notify, MUIA_Listtree_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_ContextMenu_DisplayFields
	);

	/*
	 * Cycle chain
	 */
	DoMethod( obj, MUIM_MultiSet, MUIA_CycleChain, 1,
		str_label, cyc_type, bt_add, /*bt_addsub TOFIX!!,*/ bt_remove, NULL
	);

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;
	ULONG i;
	
	/* set CURRENT entry */
	if( getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active ) != MUIV_Listtree_Active_Off )
	{
		data->previous_active_tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, 0 );
	}

	DoMethod( obj, MM_Prefswin_ContextMenu_FixFields );
	for( i = 0; i < PWCM_OPTSNUM; i++ )
	{
		DoMethod( obj, MM_Prefswin_ContextMenu_SaveTree, i );
	}
	
	return( DOSUPER );
}


DECMETHOD( Prefswin_ContextMenu_Add, ULONG )
{
	GETDATA;
	APTR tn = NULL;
	struct vp_contextmenu *vpcm;

	if( vpcm = malloc( sizeof( struct vp_contextmenu ) ) )
	{
		tn = ( APTR )getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active );

		D( db_gui, bug( "adding mem node 0x%lx, clearing the objects too\n", vpcm ) );
		/*
		 * We first disable the notify because Listtree sucks
		 */
		data->display_lock = TRUE;

		DoMethod( obj, MM_Prefswin_ContextMenu_FixFields );
		DoMethod( obj, MM_Prefswin_ContextMenu_ClearFields );
		DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_Insert, "", vpcm, MUIV_Listtree_Insert_ListNode_Root, tn ? MUIV_Listtree_Insert_PrevNode_Active : MUIV_Listtree_Insert_PrevNode_Tail, MUIV_Listtree_Insert_Flags_Active );
	
		data->display_lock = FALSE;

		DoMethod( obj, MM_Prefswin_ContextMenu_Renumber, 0, 0 );

		set( _win( obj ), MUIA_Window_ActiveObject, data->str_label );

		if( getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active ) != MUIV_Listtree_Active_Off )
		{
			data->previous_active_tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, 0 );
		}
	}

	return( 0 );
}


/*
 * Add a submenu. Must be called only if it's possible. No checking.
 */
DECMETHOD( Prefswin_ContextMenu_AddSub, ULONG )
{
	GETDATA;
	struct MUIS_Listtree_TreeNode *tn;
	struct vp_contextmenu *vpcm;

	D( db_gui, bug( "called\n" ) );

	if( vpcm = malloc( sizeof( struct vp_contextmenu ) ) )
	{
		tn = ( struct MUIS_Listtree_TreeNode * )getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active );

		data->display_lock = TRUE;

		DoMethod( obj, MM_Prefswin_ContextMenu_FixFields );
		DoMethod( obj, MM_Prefswin_ContextMenu_ClearFields );
		
		if( tn && tn->tn_Flags & TNF_LIST )
		{
			DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_Open, MUIV_Listtree_Open_ListNode_Root, MUIV_Listtree_Open_TreeNode_Active, NULL );
		}
		
		DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_Insert, "", vpcm, tn ? tn : MUIV_Listtree_Insert_ListNode_Root, ( tn->tn_Flags & TNF_LIST ) ? MUIV_Listtree_Insert_PrevNode_Tail : MUIV_Listtree_Insert_PrevNode_Active, MUIV_Listtree_Insert_Flags_Active );

		data->display_lock = FALSE;
		
		DoMethod( obj, MM_Prefswin_ContextMenu_Renumber, 0, 0 );
		
		set( _win( obj ), MUIA_Window_ActiveObject, data->str_label );
		//TOFIX!! then select it...
		if( getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active ) != MUIV_Listtree_Active_Off )
		{
			data->previous_active_tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, 0 );
		}
	}

	return( 0 );
}


DECMETHOD( Prefswin_ContextMenu_Remove, ULONG )
{
	GETDATA;
	struct MUIS_Listtree_TreeNode *tn;

	if( tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, NULL ) )
	{
		D( db_gui, bug( "freeing memnode 0x%lx\n", tn->tn_User ) );
		free( tn->tn_User );
		
		data->display_lock = TRUE;
		DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_Remove, tn, MUIV_Listtree_Remove_TreeNode_Active, NULL );
		data->display_lock = FALSE;
	
		data->previous_active_tn = NULL;
		
		DoMethod( obj, MM_Prefswin_ContextMenu_Renumber, 0, 0 );

		if( getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active ) == MUIV_Listtree_Active_Off )
		{
			DoMethod( obj, MM_Prefswin_ContextMenu_ClearFields );
		}
		else
		{
			DoMethod( obj, MM_Prefswin_ContextMenu_DisplayFields );
		}
	}

	return( 0 );
}


DECSMETHOD( Prefswin_ContextMenu_SetType )
{
	GETDATA;
	
	/*
	 * We make sure the current fields are fixed then we switch to the
	 * new list
	 */
	if( getv( data->lt_menu[ msg->type ], MUIA_Listtree_Active ) != MUIV_Listtree_Active_Off )
	{
		DoMethod( obj, MM_Prefswin_ContextMenu_DisplayFields );
	}
	else
	{
		data->active_type = getv( data->cyc_type, MUIA_Cycle_Active );
	}

	set( data->grp_type, MUIA_Group_ActivePage, msg->type );

	return( 0 );
}


/*
 * Fills in the Listtree according to the prefs
 */
DECSMETHOD( Prefswin_ContextMenu_LoadTree )
{
	GETDATA;
	APTR tn = NULL;
	ULONG current_depth = 0;
	ULONG c;
	struct vp_contextmenu *vpcm;
	struct vp_iocontextmenu *vplcm;
	ULONG mode[ 4 ];
	struct MinList ll; /* load list */

	D( db_gui, bug( "setting type..\n" ) );

	switch( msg->type )
	{
		case VMT_PAGEMODE:
			mode[ MODE_LABELS ] = DSI_CMENUS_PAGE_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_PAGE_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_PAGE_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_PAGE_DEPTH;
			break;

		case VMT_LINKMODE:
			mode[ MODE_LABELS ] = DSI_CMENUS_LINK_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_LINK_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_LINK_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_LINK_DEPTH;
			break;

		case VMT_IMAGEMODE:
			mode[ MODE_LABELS ] = DSI_CMENUS_IMAGE_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_IMAGE_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_IMAGE_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_IMAGE_DEPTH;
			break;
	}

	NEWLIST( &ll );
	
	for( c = 0; getprefsstr( mode[ MODE_LABELS ] + c, "" )[ 0 ] || ( getprefslong( mode[ MODE_ACTION ] + c, 0 ) == BFUNC_BAR ); c++ )
	{
		D( db_gui, bug( "loop count == %ld\n", c ) );
		if( vpcm = malloc( sizeof( struct vp_contextmenu ) ) )
		{
			ULONG depth = getprefslong( mode[ MODE_DEPTH ] + c + 1, 0 ); // nasty..

			D( db_gui, bug( "adding context menu %s in listtree\n", ( getprefslong( mode[ MODE_ACTION ] + c, 0 ) == BFUNC_BAR ) ? "---" : getprefsstr( mode[ MODE_LABELS ] + c, "" ) ) );

			vpcm->num = c;
			vpcm->action = getprefslong( mode[ MODE_ACTION ] + c, 0 );
			stccpy( vpcm->args, getprefsstr( mode[ MODE_ARGS ] + c, "" ), VREXX_MAXLENGTH );
			tn = ( APTR )DoMethod( data->lt_menu[ msg->type ], MUIM_Listtree_Insert, ( getprefslong( mode[ MODE_ACTION ] + c, 0 ) == BFUNC_BAR ) ? "---" : getprefsstr( mode[ MODE_LABELS ] + c, "" ), vpcm, ISLISTEMPTY( &ll ) ? MUIV_Listtree_Insert_ListNode_Root : ( ( struct vp_iocontextmenu * )LASTNODE( &ll ) )->ln, MUIV_Listtree_Insert_PrevNode_Tail, ( getprefslong( mode[ MODE_DEPTH ] + c + 1, 0 ) > current_depth ) ? TNF_LIST : NULL );
					
			/*
			 * Adjust depth
			 */
			if( depth > current_depth )
			{
				if( vplcm = malloc( sizeof( struct vp_iocontextmenu ) ) )
				{
					D( db_gui, bug( "adjusting depth from %ld to %ld\n", current_depth, depth ) );
					vplcm->ln = tn;
					ADDTAIL( &ll, vplcm );
					current_depth = depth;
				}
				else
				{
					return( FALSE ); //TOFIX!! call the freeing function. none cares about the return value
				}
			}
			else if( depth < current_depth )
			{
				D( db_gui, bug( "adjusting depth from %ld to %ld\n", current_depth, depth ) );
				vplcm = LASTNODE( &ll );
				REMTAIL( &ll );
				free( vplcm );
				current_depth = depth;
			}

		}
		else
		{
			return( FALSE );
		}
	}

	return( 0 );
}


/*
 * Saves a listtree to prefs recursively
 */
DECSMETHOD( Prefswin_ContextMenu_SaveTree )
{
	GETDATA;
	struct MUIS_Listtree_TreeNode *ln = NULL;
	struct MUIS_Listtree_TreeNode *tn = NULL;
	ULONG pos = 0;
	ULONG mode[ 4 ];
	struct MinList sl; /* save list */
	struct vp_savecontextmenu *vpscm;
	ULONG menunum = 0;
	ULONG depth = 0;
	int rootdone = FALSE;

	D( db_gui, bug( "saving tree..\n" ) );
	
	switch( msg->type )
	{
		case VMT_PAGEMODE:
			mode[ MODE_LABELS ] = DSI_CMENUS_PAGE_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_PAGE_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_PAGE_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_PAGE_DEPTH;
			break;

		case VMT_LINKMODE:
			mode[ MODE_LABELS ] = DSI_CMENUS_LINK_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_LINK_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_LINK_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_LINK_DEPTH;
			break;

		case VMT_IMAGEMODE:
			mode[ MODE_LABELS ] = DSI_CMENUS_IMAGE_LABELS;
			mode[ MODE_ACTION ] = DSI_CMENUS_IMAGE_ACTION;
			mode[ MODE_ARGS ] = DSI_CMENUS_IMAGE_ARGS;
			mode[ MODE_DEPTH ] = DSI_CMENUS_IMAGE_DEPTH;
			break;
	}

	NEWLIST( &sl );

	while( 1 )
	{
		if( tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ msg->type ], MUIM_Listtree_GetEntry, ln ? ln : MUIV_Listtree_GetEntry_ListNode_Root, pos, MUIV_Listtree_GetEntry_Flags_SameLevel ) )
		{
			D( db_gui, bug( "adding %s (num == %ld, pos == %ld, depth == %ld)\n", tn->tn_Name, menunum, pos, depth ) );
			D( db_gui, bug( "  type == %ld, args == %s\n", ( ( struct vp_contextmenu * )tn->tn_User )->action, ( ( struct vp_contextmenu * )tn->tn_User )->args ) );
			
			setprefsstr( mode[ MODE_LABELS ] + menunum, tn->tn_Name );
			setprefslong( mode[ MODE_ACTION ] + menunum, ( ( struct vp_contextmenu * )tn->tn_User )->action );
			setprefsstr( mode[ MODE_ARGS ] + menunum, ( ( struct vp_contextmenu * )tn->tn_User )->args );
			setprefslong( mode[ MODE_DEPTH ] + menunum, depth );
			menunum++;

			if( tn->tn_Flags & TNF_LIST )
			{
				if( vpscm = malloc( sizeof( struct vp_savecontextmenu ) ) )
				{
					D( db_gui, bug( "saving list %s\n", tn->tn_Name ) );

					ln = vpscm->ln = tn;
					vpscm->pos = pos;
					ADDTAIL( &sl, vpscm );
					depth++;
					pos = 0;
				}
				else
				{
					return( FALSE ); //TOFIX!! call the freeing function
				}
			}
			else
			{
				/* free entry */
				D( db_gui, bug( "freeing entry\n" ) );
				free( tn->tn_User );
				pos++;
			}
		}
		else
		{
			/*
			 * no tn, check if we have a parent in the tree
			 */
			if( !ISLISTEMPTY( &sl ) )
			{
				D( db_gui, bug( "restoring context\n" ) );
				vpscm = LASTNODE( &sl );
				/* restore context */
				pos = vpscm->pos + 1;
				free( ( ( struct MUIS_Listtree_TreeNode * ) vpscm->ln )->tn_User );
				REMTAIL( &sl );
				free( vpscm );
				depth--;
			}
			else
			{
				/*
				 * Put a flag because we have no ln for the root so
				 * we need to do that part twice
				 */

				if( rootdone )
				{
					/*
					 * Otherwise we are done.
					 * Make sure to zero the prefs so that it's not loaded
					 */
					setprefsstr( mode[ MODE_LABELS ] + menunum, "" );
					break;
				}
				else
				{
					ln = NULL;
					rootdone = TRUE;
				}
			}
		}
	}
	return( 0 );
}


DECSMETHOD( Prefswin_ContextMenu_Renumber )
{
	GETDATA;
	struct MUIS_Listtree_TreeNode *ln = NULL;
	struct MUIS_Listtree_TreeNode *tn = NULL;
	ULONG pos = 0;
	ULONG menunum = 0;
	struct MinList rl; /* renumber list */
	struct vp_savecontextmenu *vpscm;
	LONG rootdone = FALSE;

	D( db_gui, bug( "renumbering tree...\n" ) );

	NEWLIST( &rl );

	while( 1 )
	{
		if( tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, ln ? ln : MUIV_Listtree_GetEntry_ListNode_Root, pos, MUIV_Listtree_GetEntry_Flags_SameLevel ) )
		{
			D( db_gui, bug( "setting %s with number %ld ( vpcm at 0x%lx )\n", tn->tn_Name, menunum, tn->tn_User ) );
			( ( struct vp_contextmenu * )tn->tn_User )->num = pos;
			menunum++;

			if( tn->tn_Flags & TNF_LIST )
			{
				if( vpscm = malloc( sizeof( struct vp_savecontextmenu ) ) )
				{
					D( db_gui, bug( "saving list %s\n", tn->tn_Name ) );
					ln = vpscm->ln = tn;
					vpscm->pos = pos;
					ADDTAIL( &rl, vpscm );
					pos = 0;
				}
				else
				{
					return( 0 ); //TOFIX!! at this point the memory is really low and the menu is fucked
								 // we should abort the prefs
				}
			}
			pos++;
		}
		else
		{
			/* check for parent */
			if( !ISLISTEMPTY( &rl ) )
			{
				vpscm = LASTNODE( &rl );
				/* restore previous context */
				pos = vpscm->pos + 1;
				REMTAIL( &rl );
				free( vpscm );
			}
			else
			{
				if( rootdone )
				{
					break;
				}
				else
				{
					ln = NULL;
					rootdone = TRUE;
				}
			}
		}
	}

	return( 0 );
}


/*
 * Save the fields into the prefsclone
 */
DECMETHOD( Prefswin_ContextMenu_FixFields, ULONG )
{
	GETDATA;
	ULONG mode[ 3 ];

	D( db_gui, bug( "fixing fields (images, shortcut, labels, args, action) into prefsclone\n" ) );

	if( data->previous_active_tn )
	{
		switch( getv( data->cyc_type, MUIA_Cycle_Active ) )
		{
			case VMT_PAGEMODE:
				D( db_gui, bug( "saving page %s\n", data->previous_active_tn->tn_Name ) );
				mode[ MODE_LABELS ] = DSI_CMENUS_PAGE_LABELS;
				mode[ MODE_ACTION ] = DSI_CMENUS_PAGE_ACTION;
				mode[ MODE_ARGS ] = DSI_CMENUS_PAGE_ARGS;
				break;

			case VMT_LINKMODE:
				D( db_gui, bug( "saving link %s\n", data->previous_active_tn->tn_Name ) );
				mode[ MODE_LABELS ] = DSI_CMENUS_LINK_LABELS;
				mode[ MODE_ACTION ] = DSI_CMENUS_LINK_ACTION;
				mode[ MODE_ARGS ] = DSI_CMENUS_LINK_ARGS;
				break;

			case VMT_IMAGEMODE:
				D( db_gui, bug( "saving image %s\n", data->previous_active_tn->tn_Name ) );
				mode[ MODE_LABELS ] = DSI_CMENUS_IMAGE_LABELS;
				mode[ MODE_ACTION ] = DSI_CMENUS_IMAGE_ACTION;
				mode[ MODE_ARGS ] = DSI_CMENUS_IMAGE_ARGS;
				break;
		}
		
		if( getv( data->cyc_bar, MUIA_Cycle_Active ) == BM_BAR )
		{
			( ( struct vp_contextmenu * )data->previous_active_tn->tn_User )->action = BFUNC_BAR;
			( ( struct vp_contextmenu * )data->previous_active_tn->tn_User )->args[ 0 ] = '\0';
		}
		else
		{
			( ( struct vp_contextmenu * )data->previous_active_tn->tn_User )->action = getv( data->pop_cmd, MA_Command_Mode );
			strcpy( ( ( struct vp_contextmenu * )data->previous_active_tn->tn_User )->args, ( STRPTR )getv( data->pop_cmd, MA_Command_String ) );
		}
	}
	
	if( getv( data->lt_menu[ data->active_type ], MUIA_Listtree_Active ) != MUIV_Listtree_Active_Off )
	{
		data->previous_active_tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, 0 );
	}

	data->active_type = getv( data->cyc_type, MUIA_Cycle_Active );

	return( 0 );
}


/*
 * Takes the values from the prefsclone and puts them in
 * the fields
 */
DECMETHOD( Prefswin_ContextMenu_DisplayFields, ULONG )
{
	GETDATA;
	struct MUIS_Listtree_TreeNode *tn;
	ULONG mode[ 3 ];

	if( !data->display_lock )
	{
		D( db_gui, bug( "entering\n" ) );

		DoMethod( obj, MM_Prefswin_ContextMenu_FixFields );

		if( tn = ( struct MUIS_Listtree_TreeNode * )DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, 0 ) )
		{
			ULONG active = getv( data->cyc_type, MUIA_Cycle_Active );

			D( db_gui, bug( "entry number %ld\n", ( ( struct vp_contextmenu * )tn->tn_User )->num ) );

	        switch( active )
			{
				case VMT_PAGEMODE:
					D( db_gui, bug( "displaying page %s\n", tn->tn_Name ) );
					mode[ MODE_LABELS ] = DSI_CMENUS_PAGE_LABELS;
					mode[ MODE_ACTION ] = DSI_CMENUS_PAGE_ACTION;
					mode[ MODE_ARGS ] = DSI_CMENUS_PAGE_ARGS;
					break;

				case VMT_LINKMODE:
					D( db_gui, bug( "displaying link %s\n", tn->tn_Name ) );
					mode[ MODE_LABELS ] = DSI_CMENUS_LINK_LABELS;
					mode[ MODE_ACTION ] = DSI_CMENUS_LINK_ACTION;
					mode[ MODE_ARGS ] = DSI_CMENUS_LINK_ARGS;
					break;

				case VMT_IMAGEMODE:
					D( db_gui, bug( "displaying image %s\n", tn->tn_Name ) );
					mode[ MODE_LABELS ] = DSI_CMENUS_IMAGE_LABELS;
					mode[ MODE_ACTION ] = DSI_CMENUS_IMAGE_ACTION;
					mode[ MODE_ARGS ] = DSI_CMENUS_IMAGE_ARGS;
					break;
	 
			}
			
			if( ( ( struct vp_contextmenu * )tn->tn_User )->action == BFUNC_BAR )
			{
				nnset( data->cyc_bar, MUIA_Cycle_Active, BM_BAR );
				set( data->str_label, MUIA_Disabled, TRUE );
			}
			else
			{
				nnset( data->str_label, MUIA_String_Contents, tn->tn_Name );
				nnset( data->cyc_bar, MUIA_Cycle_Active, BM_NORMAL );
				set( data->str_label, MUIA_Disabled, FALSE );
				nnset( data->pop_cmd, MA_Command_Mode, ( ( struct vp_contextmenu * )tn->tn_User )->action );
				nnset( data->pop_cmd, MA_Command_String, ( ( struct vp_contextmenu * )tn->tn_User )->args );
			}
		}
	}
	return( 0 );
}


/*
 * Resets the fields to a default value
 */
DECMETHOD( Prefswin_ContextMenu_ClearFields, ULONG )
{
	GETDATA;

	D( db_gui, bug( "clearing fields\n" ) );

	nnset( data->str_label, MUIA_String_Contents, "" );
	nnset( data->pop_cmd, MA_Command_Mode, BFUNC_COMMAND );
	nnset( data->pop_cmd, MA_Command_String, "" );
	nnset( data->cyc_bar, MUIA_Cycle_Active, BM_NORMAL );
	set( data->str_label, MUIA_Disabled, FALSE );

	return( 0 );
}


/*
 * Sets the current active Listtree entry with the string
 * that is being typed by the user
 */
DECSMETHOD( Prefswin_ContextMenu_Rename )
{
	GETDATA;

	DoMethod( data->lt_menu[ data->active_type ], MUIM_Listtree_Rename, MUIV_Listtree_Rename_TreeNode_Active, msg->txt, NULL );

	return( 0 );
}


/*
 * Enable or disable the edit group depending
 * on the entry in the listtree ( list, off, normal entry )
 */
DECSMETHOD( Prefswin_ContextMenu_ChangeEditGroup )
{
	GETDATA;

	if( msg->tn == MUIV_Listtree_Active_Off || ( ( struct MUIS_Listtree_TreeNode * ) msg->tn )->tn_Flags & TNF_LIST )
	{
		set( data->grp_edit, MUIA_Disabled, TRUE );
	}
	else
	{
		set( data->grp_edit, MUIA_Disabled, FALSE );
	}

	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Prefswin_ContextMenu_Add )
DEFMETHOD( Prefswin_ContextMenu_AddSub )
DEFMETHOD( Prefswin_ContextMenu_Remove )
DEFMETHOD( Prefswin_ContextMenu_FixFields )
DEFMETHOD( Prefswin_ContextMenu_DisplayFields )
DEFMETHOD( Prefswin_ContextMenu_ClearFields )
DEFSMETHOD( Prefswin_ContextMenu_SetType )
DEFSMETHOD( Prefswin_ContextMenu_LoadTree )
DEFSMETHOD( Prefswin_ContextMenu_SaveTree )
DEFSMETHOD( Prefswin_ContextMenu_Renumber )
DEFSMETHOD( Prefswin_ContextMenu_Rename )
DEFSMETHOD( Prefswin_ContextMenu_ChangeEditGroup )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_contextmenuclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_ContextMenuClass";
#endif

	return( TRUE );
}

void delete_prefswin_contextmenuclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_contextmenuclass( void )
{
	return( mcc->mcc_Class );
}


