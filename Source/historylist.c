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
 * Historylist list class
 * ----------------------
 * - Handles the history of the URLs. Supports adding of entries and
 *   easy getting of the current/previous/next entry. Handles framesets gracefully (soon, actually)
 *
 * © 1999-2003 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * Started by David Gerber <zapek@vapor.com> on 13-10-1999 after saying:
 * "bah, this back/forward handling really sucks dick"
 *
 * $Id: historylist.c,v 1.76 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

#include "classes.h"
#include "htmlwin.h"
#include "network.h"
#include "js.h"
#include "historylist.h"
#include "htmlclasses.h"
#include "malloc.h"
#include "mui_func.h"
#include "methodstack.h"


static struct MinList historylist;
static struct SignalSemaphore historylistsem;
static ULONG isinit;

/* instance data */
struct Data
{
	struct MinList bflist;             /* Back/Forward list */
	struct MinList bflist2;	           /* List backup to handle failures */
	struct SignalSemaphore bflistsem;  /* Protection when the user plays with the context menu (probably needed for MUI 20+ only) */
	struct history_mainpage *mpglobal;
	LONG pos;                          /* Number of the current entry */
	LONG lastpos;
	LONG total;                        /* Number of entries */
	LONG lowmark;                      /* Insertion point when entries are added from another window */
	int hasback;
	int hasnext;
	int failed;
	int allow_undo;
	APTR owner;
	int standalone;                    /* if TRUE we can add URLs globally */
};


/* hooks */
MUI_HOOK( historylist_disp, STRPTR *array, struct history_mainpage *mp )
{
	*array = mp->url;
	return( 0 );
}


/*
 * Remove and free all the history_mainpage list entries
 */
void emptymplist( struct MinList *l )
{
	struct history_mainpage *mp;

	while( mp = REMHEAD( l ) )
	{
		free( mp->url );
		free( mp );
	}
}


DECNEW
{
	struct Data *data;

	if( !( obj = (Object *)DoSuperNew( cl, obj,
										InputListFrame,
										MUIA_List_DisplayHook, &historylist_disp_hook,
										TAG_MORE, msg->ops_AttrList ) ) )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	data->owner = (APTR)GetTagData( MA_Historylist_Owner, 0, msg->ops_AttrList );

	/*
	 * Building bflist
	 */
	NEWLIST( &data->bflist );
	NEWLIST( &data->bflist2 );
	InitSemaphore( &data->bflistsem );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;
 
	/*
	 * Cleaning up the BFLists
	 */
	ObtainSemaphore( &data->bflistsem );
	
	emptymplist( &data->bflist );
	emptymplist( &data->bflist2 );

	ReleaseSemaphore( &data->bflistsem );

	return( DOSUPER );
}


/*
 * Insert an entry to the HistorylistObject. Automatically broadcasts
 * to other existing windows.
 */
DECSMETHOD( Historylist_AddURL )
{
	struct history_mainpage *mp, *mpn;
	GETDATA;

	DL( DEBUG_IMPORTANT, db_history, bug( "adding URL: %s\n", msg->url ) );

	if( data->failed )
	{
		data->failed = FALSE; 
		DoMethod( obj, MM_Historylist_RestoreBFList );
	}

	/*
	 * Backup the previous state first.
	 */
	data->lastpos = data->pos;
		
	emptymplist( &data->bflist2 );
	ITERATELIST( mp, &data->bflist )
	{
		mpn = malloc( sizeof( struct history_mainpage ) );
		memset( mpn, '\0', sizeof( struct history_mainpage ) ); /* TOFIX: maybe not necessary */
		mpn->url = malloc( strlen( mp->url ) + 1 );
		strcpy( mpn->url, mp->url );
		ADDTAIL( &data->bflist2, mpn );
	}

	if( data->standalone )
	{
		DoMethod( obj, MM_Historylist_AddGlobal, msg->url );
	}
	DoMethod( obj, MM_Historylist_AddBF, msg->url );

	data->allow_undo = TRUE;
	
	return( 0 );
}


/*
 * Restore the backup of the BFList.
 */
DECMETHOD( Historylist_RestoreBFList, ULONG )
{
	struct history_mainpage *mp, *mpn;
	GETDATA;

	DL( DEBUG_INFO, db_history, bug( "restoring BFList now\n" ) );
	DL( DEBUG_CHATTY, db_history, bug( " list looks like:\n" ) );

	ObtainSemaphore( &data->bflistsem );
	emptymplist( &data->bflist );
	data->total = 0;
	ITERATELIST( mp, &data->bflist2 )
	{
		mpn = malloc( sizeof( struct history_mainpage ) );
		memset( mpn, '\0', sizeof( struct history_mainpage ) );
		mpn->url = malloc( strlen( mp->url ) + 1 );
		strcpy( mpn->url, mp->url );
		ADDTAIL( &data->bflist, mpn );
		data->total++;
		DL( DEBUG_CHATTY, db_history, bug( "url (%lu)  == %s\n", data->total, mp->url ) );
	}
	set( obj, MA_Historylist_CurrentEntry, data->lastpos );

	DL( DEBUG_INFO, db_history, bug( "BFList restored: data->pos == %lu, data->total == %lu\n", data->pos, data->total ) );

	ReleaseSemaphore( &data->bflistsem );

	return( 0 );
}


/*
 * Adds an entry into the global Historylist (common to all windows)
 */
DECSMETHOD( Historylist_AddGlobal )
{
	struct history_mainpage *mp;
	GETDATA;

	/*
	 * Check if we are already in that evil world.
	 */
	ObtainSemaphore( &historylistsem );
	
	ITERATELIST( mp, &historylist )
	{
		if( !strcmp( mp->url, msg->url ) )
		{
			DL( DEBUG_INFO, db_history, bug( "global entry %s already exists\n", msg->url ) );
			data->mpglobal = NULL;
			ReleaseSemaphore( &historylistsem );
			return( 0 );
		}
	}

	/*
	 * Add the entry.
	 */
	DL( DEBUG_IMPORTANT, db_history, bug( "adding global entry %s\n", msg->url ) );
	data->mpglobal = malloc( sizeof( struct history_mainpage ) );
	memset( data->mpglobal, '\0', sizeof( struct history_mainpage ) );
	data->mpglobal->url = malloc( strlen( msg->url ) + 1 );
	strcpy( data->mpglobal->url, msg->url );

	ADDTAIL( &historylist, data->mpglobal );

	ReleaseSemaphore( &historylistsem );
	
	return( 0 );
}


/*
 * Adds an entry into the Back/Forward list.
 */
DECSMETHOD( Historylist_AddBF )
{
	struct history_mainpage *mp;
	int i;
	GETDATA;

	DL( DEBUG_INFO, db_history, bug( "called\n" ) );

	ObtainSemaphore( &data->bflistsem );

	/*
	 * Check if we exist.
	 */
	ITERATELIST( mp, &data->bflist )
	{
		if( !strcmp( mp->url, msg->url ) )
		{
			/*
			 * Remove the entry so that we can add it at the
			 * tail of the bflist.
			 */
			DL( DEBUG_INFO, db_history, bug( "entry %s already exists in BFList, removing %lx\n", msg->url, mp ) );
			REMOVE( mp );
			free( mp->url );
			free( mp );
			data->total--;
			break;
		}
	}

	/*
	 * Clear the entries following the current
	 * insertion point.
	 */
#ifdef VDEBUG
	ITERATELIST( mp, &data->bflist )
	{
		DL( DEBUG_CHATTY, db_history, bug( "listnode (0x%lx): url == %s\n", mp, mp->url ) );
	}
	DL( DEBUG_CHATTY, db_history, bug( "lastnode (0x%lx): url == %s\n", LASTNODE( &data->bflist ), ( ( struct history_mainpage * )LASTNODE( &data->bflist ) )->url ) );
#endif /* VDEBUG */

	for( i = data->total; i > data->pos + 1; i-- )
	{
		mp = LASTNODE( &data->bflist );
		DL( DEBUG_INFO, db_history, bug( "removing BF %s\n", mp->url ) );
		REMOVE( mp );
		free( mp->url );
		free( mp );
		data->total--;
	}

	/*
	 * And add the entry into the Back/Forward list.
	 */
	mp = malloc( sizeof( struct history_mainpage ) );
	memset( mp, '\0', sizeof( struct history_mainpage ) );
	mp->url = malloc( strlen( msg->url ) + 1 );
	strcpy( mp->url, msg->url );
	ADDTAIL( &data->bflist, mp );
	DL( DEBUG_IMPORTANT, db_history, bug( "adding BF as %s\n", mp->url ) );

	data->total++;
	set( obj, MA_Historylist_CurrentEntry, data->total - 1 );

	/*
	 * Update Back/Next buttons.
	 */
	set( obj, MA_Historylist_HasNext, FALSE );
	set( obj, MA_Historylist_HasBack, ( data->pos > 0 ) ? TRUE : FALSE );

	ReleaseSemaphore( &data->bflistsem );

	return( 0 );
}


/*
 * This method is used when we want to put an URL but not necessarily on
 * the top of our historylist. Happens when an URL was added from
 * another window. We keep a pointer to know where to add it, in our case
 * below the window's history. Safe to use on the window who adds the entry.
 */
DECSMETHOD( Historylist_InsertURLExtern )
{
	GETDATA;

	DL( DEBUG_INFO, db_history, bug( "called\n" ) );

	if( DoMethod( obj, MM_Historylist_Exists, msg->mp->url, 0 ) == MV_Historylist_NoEntry )
	{
		DL( DEBUG_INFO, db_history, bug( "inserting %s\n", msg->mp->url ) );
		DoMethod( obj, MUIM_List_InsertSingle, msg->mp, data->lowmark );
	}
	return( 0 );
}


/*
 * Fixes a state. It means the URL is ok and there's no need
 * to keep an undo state anymore. We do broadcast the URL to other
 * windows too since it's valid now.
 */
DECMETHOD( Historylist_End, ULONG )
{
	GETDATA;

	/*
	 * Broadcast.
	 */
	if( data->standalone && data->mpglobal )
	{
		DL( DEBUG_INFO, db_history, bug( "URL to add %s\n", data->mpglobal->url ) );
		DoMethod( obj, MUIM_List_InsertSingle, data->mpglobal, 0 );
		DL( DEBUG_INFO, db_history, bug( "URL to add still %s\n", data->mpglobal->url ) );
		data->lowmark++;
		DoMethod( app, MM_DoAllWins, MM_Historylist_InsertURLExtern, data->mpglobal );
	}

	data->allow_undo = FALSE;

	return( 0 );
}


/*
 * Go to a previous state. Either when the user pressed stop or the
 * URL is not valid, etc..
 */
DECSMETHOD( Historylist_Undo )
{
	GETDATA;
	int i;

	DL( DEBUG_IMPORTANT, db_history, bug( "Undoing\n" ) );

	if( data->allow_undo )
	{
		DL( DEBUG_INFO, db_history, bug( "allow_undo == TRUE\n" ) );

		if( data->standalone && data->mpglobal )
		{
			ObtainSemaphore( &historylistsem );
			DL( DEBUG_INFO, db_history, bug( "removing global entry %s\n", data->mpglobal->url ) );
			i = DoMethod( obj, MM_Historylist_Exists, data->mpglobal->url, 0 );
			if( i != MV_Historylist_NoEntry )
			{
				DoMethod( obj, MUIM_List_Remove, i );
			}
			REMOVE( data->mpglobal );
			free( data->mpglobal->url );
			free( data->mpglobal );
			data->mpglobal = NULL;
			ReleaseSemaphore( &historylistsem );
		}

		/*
		 * When we abort, we want the previous state.
		 */
		if( msg->abort )
		{
			DoMethod( obj, MM_Historylist_RestoreBFList );
		}
		else
		{
			/*
			 * Remove our broken link on the next run.
			 */
			DL( DEBUG_INFO, db_history, bug( "data->failed = TRUE\n" ) );
			data->failed = TRUE;
		}

		/*
		 * Update Back/Next buttons.
		 */
		set( obj, MA_Historylist_HasNext, ( data->pos < ( data->total - 1 ) ) ? TRUE : FALSE );
		set( obj, MA_Historylist_HasBack, ( data->pos > 0 ) ? TRUE : FALSE );
	}

	return( 0 );
}

/* goes one step backward in the history and displays the page in the current window */
DECMETHOD( Historylist_Back, ULONG )
{
	struct history_mainpage *mp;
	int i;
	GETDATA;

	DL( DEBUG_IMPORTANT, db_history, bug( "called\n" ) );

	if( data->pos > 0 )
	{
	    /*
		 * Since the user changed something. All outstanding
		 * MM_Historylist_Undo methods are useless.
		 */
		data->allow_undo = FALSE;

		set( obj, MA_Historylist_CurrentEntry, data->pos - 1 );
		
		mp = FIRSTNODE( &data->bflist );
		for( i = 0; i < data->pos; i++ )
		{
			mp = NEXTNODE( mp );
		}

		DoMethod( data->owner, MM_HTMLWin_SetURL, mp->url, NULL, NULL, 0 );
		
		if( data->failed )
		{
			data->failed = FALSE; 
			DoMethod( obj, MM_Historylist_RestoreBFList );
		}
	 
		set( obj, MA_Historylist_HasNext, ( data->pos < ( data->total -1  ) ) ? TRUE : FALSE );
		set( obj, MA_Historylist_HasBack, ( data->pos > 0 ) ? TRUE : FALSE );

		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


/* goes one step forward in the history and displays the page in the current window */
DECMETHOD( Historylist_Next, ULONG )
{
	struct history_mainpage *mp;
	int i;
	GETDATA;

	DL( DEBUG_IMPORTANT, db_history, bug( "called\n" ) );

	if( data->pos < data->total -1 )
	{
	    /*
		 * Since the user changed something. All outstanding
		 * MM_Historylist_Undo methods are useless.
		 */
		data->allow_undo = FALSE;

		set( obj, MA_Historylist_CurrentEntry, data->pos + 1 );
		
		mp = FIRSTNODE( &data->bflist );
		for( i = 0; i < data->pos; i++ )
		{
			mp = NEXTNODE( mp );
		}

		DoMethod( data->owner, MM_HTMLWin_SetURL, mp->url, NULL, NULL, 0 );
		
		if( data->failed )
		{
			data->failed = FALSE; 
			DoMethod( obj, MM_Historylist_RestoreBFList );
		}
	 
		set( obj, MA_Historylist_HasNext, ( data->pos < ( data->total -1  ) ) ? TRUE : FALSE );
		set( obj, MA_Historylist_HasBack, ( data->pos > 0 ) ? TRUE : FALSE );
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


/*
 * Goes to the given entry.
 * If msg->pos == MV_Historylist_Popdown it means the selection came from the history popdown.
 */
DECSMETHOD( Historylist_GotoEntry )
{
	struct history_mainpage *mp;
	GETDATA;

	DL( DEBUG_IMPORTANT, db_history, bug( "called\n" ) );

	/*
	 * Since the user changed something. All outstanding
	 * MM_Historylist_Undo methods are useless.
	 */
	data->allow_undo = FALSE;

	if( msg->pos == MV_Historylist_Popdown )
	{
		DL( DEBUG_IMPORTANT, db_history, bug( "from popdown, adding URL now\n" ) );
		DoMethod( obj, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mp );
		DoMethod( data->owner, MM_HTMLWin_SetURL, mp->url, NULL, NULL, 0 );
		return( 0 );
	}
	else
	{
		int i;
		GETDATA;

		set( obj, MA_Historylist_CurrentEntry, msg->pos );
	
		mp = FIRSTNODE( &data->bflist );
		for( i = 0; i < msg->pos; i++ )
		{
			mp = NEXTNODE( mp );
		}
	
		DoMethod( data->owner, MM_HTMLWin_SetURL, mp->url, NULL, NULL, 0 );

		if( data->failed )
		{
			data->failed = FALSE;
			DoMethod( obj, MM_Historylist_RestoreBFList );
		}
		
		set( obj, MA_Historylist_HasNext, ( data->pos < ( data->total -1  ) ) ? TRUE : FALSE );
		set( obj, MA_Historylist_HasBack, ( data->pos > 0 ) ? TRUE : FALSE );
	}
	
	return( 0 );
}


/*
 * Checks if the entry exists and returns its position.
 * Only works for HistorylistObjects, not the global list.
 */
DECSMETHOD( Historylist_Exists )
{
	LONG i, j;
	struct history_mainpage *mp;

	j = getv( obj, MUIA_List_Entries );

	DL( DEBUG_CHATTY, db_history, bug( "got %ld entries existing (msg->startentry == %ld, URL to add == %s)\n", j, msg->startentry, msg->url ) );

	DL( DEBUG_CHATTY, db_history, bug( "-- start of loop\n" ) );
	for( i = msg->startentry; i < j; i++ )
	{
		DL( DEBUG_CHATTY, db_history, bug( "iteration %ld\n", i ) );
		DoMethod( obj, MUIM_List_GetEntry, i, &mp );
		if( mp )
		{
			DL( DEBUG_CHATTY, db_history, bug( "mp exists:\n" ) );
			DL( DEBUG_CHATTY, db_history, bug( "  msg->url == %s\n", msg->url ) );
			DL( DEBUG_CHATTY, db_history, bug( "  mp->url == %s\n", mp->url ) );
			if( !strcmp( msg->url, mp->url ) )
				return( ( ULONG )i );
		}
	}
	DL( DEBUG_CHATTY, db_history, bug( "results of the loop, nothing found, returning MV_Historylist_NoEntry\n" ) );
	return( MV_Historylist_NoEntry );
}


/*
 * Stores the X/Y Virtgroup position of an historylist entry.
 */
DECSMETHOD( Historylist_StoreXY )
{
	struct history_mainpage *mp;
	GETDATA;

	/*
	 * We only store in the back/forward list.
	 */
	ITERATELIST( mp, &data->bflist )
	{
		if( !strcmp( mp->url, msg->url ) )
		{
			mp->x = msg->x;
			mp->y = msg->y;
			break;
		}
	}
	return( 0 );
}


/*
 * Returns the X and Y Virtgroup position of an historylist entry (arguments by reference)
 */
DECSMETHOD( Historylist_GetXY )
{
	struct history_mainpage *mp;
	GETDATA;

	ITERATELIST( mp, &data->bflist )
	{
		if( !strcmp( mp->url, msg->url ) )
		{
			*msg->x = mp->x;
			*msg->y = mp->y;
			return( TRUE );
		}
	}
	return( FALSE );
}


DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_Historylist_HasBack:
			*msg->opg_Storage = ( ULONG )data->hasback;
			return( TRUE );
		case MA_Historylist_HasNext:
			*msg->opg_Storage = ( ULONG )data->hasnext;
			return( TRUE );
		case MA_Historylist_CurrentEntry:
			*msg->opg_Storage = ( ULONG )data->pos;
			return( TRUE );
		case MA_Historylist_Entries:
			*msg->opg_Storage = ( ULONG )data->total;
			return( TRUE );
		case MA_Historylist_BFList:
			*msg->opg_Storage = ( ULONG )&data->bflist;
			return( TRUE );

		case MA_JS_ClassName:
			*msg->opg_Storage = ( ULONG )"History";
			break;
	}
	return( DOSUPER );
}


DECSET
{
	struct TagItem *tag, *tagp = msg->ops_AttrList;
	struct history_mainpage *mp;
	GETDATA;

	while( tag = NextTagItem( &tagp ) ) switch( tag->ti_Tag )
	{
		case MA_Historylist_HasBack:
			data->hasback = tag->ti_Data;
			break;

		case MA_Historylist_HasNext:
			data->hasnext = tag->ti_Data;
			break;
	
		case MA_Historylist_BFReadLock:
			if( tag->ti_Data )
			{
				ObtainSemaphoreShared( &data->bflistsem );
			}
			else
			{
				ReleaseSemaphore( &data->bflistsem );
			}
			break;

		case MA_Historylist_CurrentEntry:
			data->pos = tag->ti_Data;
			break;

		case MA_Historylist_Owner:
			data->owner = (APTR)tag->ti_Data;
			break;

		case MA_Historylist_Standalone:

			/*
			 * Let's fill in our HistorylistObject from the global history
			 */
			ObtainSemaphore( &historylistsem );
			ITERATELIST( mp, &historylist )
			{
				DL( DEBUG_CHATTY, db_history, bug( "adding entry %s\n", mp->url ) );
				DoMethod( obj, MUIM_List_InsertSingle, mp, MUIV_List_Insert_Top );
			}
			ReleaseSemaphore( &historylistsem );

			data->standalone = tag->ti_Data;
			break;
	}
	return( DOSUPER );
}

BEGINPTABLE
DPROP( back,		funcptr )
DPROP( forward,		funcptr )
DPROP( go,			funcptr )
DPROP( length,		real )
DPROP( current, 	string )
DPROP( next, 		string )
DPROP( previous, 	string )
ENDPTABLE

static ULONG jsentry( struct MP_JS_GetProperty *msg, int ix, struct Data *data )
{
	struct history_mainpage *mp;

	if( ix < 0 )
		return( FALSE );
	else if( ix >= data->total )
		return( FALSE );

	mp = FIRSTNODE( &data->bflist );
	while( ix-- > 0 )
	{
		mp = NEXTNODE( mp );
	}
	storestrprop( msg, mp->url );
	return( TRUE );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	GETDATA;

	if( !pt )
	{
		if( isnum( msg->propname ) )
		{
			int v = atoi( msg->propname );
			return( jsentry( msg, v, data ) );
		}
		return( DOSUPER );
	}

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}
	
	switch( pt->id )
	{
		case JSPID_length:
			storerealprop( msg, (double)data->total );
			return( TRUE );

		case JSPID_current:
			return( jsentry( msg, data->pos, data ) );

		case JSPID_next:
			return( jsentry( msg, data->pos + 1, data ) );

		case JSPID_previous:
			return( jsentry( msg, data->pos - 1, data ) );

	}

	return( DOSUPER );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;

	switch( msg->pid )
	{
		case JSPID_back:
			{
				pushmethod( obj, 1, MM_Historylist_Back );
				return( TRUE );
			}
			break;

		case JSPID_forward:
			{
				pushmethod( obj, 1, MM_Historylist_Next );
				return( TRUE );
			}
			break;

		case JSPID_go:
			{
				int newpos;

				if( msg->argcnt-- < 1 )
					return( 0 );
				
				newpos = data->pos + exprs_pop_as_real( msg->es );
				if( newpos < 0 )
					newpos = 0;
				else if( newpos >= data->total )
					newpos = data->total - 1;				

				pushmethod( obj, 2, MM_Historylist_GotoEntry, newpos );
			}
			break;

		default:
			return( DOSUPER );
	}

	return( TRUE );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFGET
DEFSET
DEFMETHOD( Historylist_Back )
DEFMETHOD( Historylist_Next )
DEFMETHOD( Historylist_End ) 
DEFMETHOD( Historylist_RestoreBFList );
DEFSMETHOD( Historylist_AddURL )
DEFSMETHOD( Historylist_AddGlobal )
DEFSMETHOD( Historylist_AddBF )
DEFSMETHOD( Historylist_Undo )
DEFSMETHOD( Historylist_GotoEntry )
DEFSMETHOD( Historylist_Exists )
DEFSMETHOD( Historylist_InsertURLExtern )
DEFSMETHOD( Historylist_StoreXY )
DEFSMETHOD( Historylist_GetXY )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_historylistclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_List, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "HistorylistClass";
#endif

	return( TRUE );
}

void delete_historylistclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR gethistorylistclass( void )
{
	return( mcc->mcc_Class );
}

void init_historylist( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &historylist );
	InitSemaphore( &historylistsem );

	isinit = TRUE;
}


void cleanup_historylist( void )
{
	DL( DEBUG_IMPORTANT, db_history, bug( "removing historylist\n" ) );
	if ( isinit )
	{
		ObtainSemaphore( &historylistsem );
		emptymplist( &historylist );
		ReleaseSemaphore( &historylistsem );
	}
}
