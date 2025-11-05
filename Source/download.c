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
** $Id: download.c,v 1.157 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <libraries/asl.h>
#endif

/* private */
#include "voyager_cat.h"
#include "classes.h"
#include "network.h"
#include "time_func.h"
#include "mui_func.h"
#include "download.h"
#include "prefs.h"
#include "malloc.h"
#include "mime.h"
#include "download.h"
#include "dos_func.h"
#include "template.h"
#include "textinput.h"
#include "nlist.h"
#include "smartreq.h"
#include "file.h"
#include "win_func.h"

APTR win_dl;

struct snapshot {
	ULONG time;
	ULONG docptr;
};

struct dlnode {
	struct MinNode n;
	struct nstream *ns;         /* network stream */
	STRPTR filename;            /* local filename including path */
	STRPTR filename_ptr;        /* pointer to the filename part of 'filename' */
	STRPTR viewapp;             /* app for viewing */
	STRPTR url_short;           /* short URL part (70 chars) used for displaying */
	STRPTR lv_size;             /* listview computed: size */
	STRPTR lv_size_ptr;         /* listview computed: *pointer* (!) to either size or string info */
	STRPTR lv_time;             /* listview computed: time */
	STRPTR lv_time_ptr;         /* listview computed: *pointer* (!) to either computed times or string info */
	STRPTR lv_eta;              /* listview computed: ETA */
	STRPTR lv_eta_ptr;          /* listview computed: *pointer* (!) to ETA or string info */
	STRPTR lv_cps;              /* listview computed: CPS */
	STRPTR lv_state_ptr;        /* listview computed: *pointer* (!) to either CPS or state string info */
	STRPTR lv_retries;          /* listview computed: number of retries */
	STRPTR resume_url;          /* we need to store the URL to be able to launch a resume request */
	STRPTR resume_referer;      /* ditto */
	ULONG id;                   /* unique ID to find back which node is concerned easily */
	ULONG offset;               /* start of file (resume) */
	ULONG length;               /* document length */
	ULONG retries;              /* number of retries */
	ULONG update_count;         /* update counter (used for immediate CPS calculation */
	ULONG start_time;           /* start of downloading */
	struct snapshot	act_snap;   /* current time (last GotData) */
	struct snapshot last_snap;  /* current time between computations */
	time_t remaining_time;      /* remaining time (used for immediate CPS calculation) */
	ULONG flags;                /* specific flags (DLF) */
	ULONG state;                /* state of the download node (DLS) */
	ULONG stall;                /* stall detector */
};

/*
 * Allocation sizes
 */
#define URL_SHORT_SIZE 70
#define LENGTH_BUFFER_SIZE 64
#define TIME_BUFFER_SIZE 20 /* TOFIX: not locale proof, underlooked.. */
#define ETA_BUFFER_SIZE 20 /* TOFIX: blabla */
#define CPS_BUFFER_SIZE 20 /* TOFIX: not locale proof.. maybe the others too.. fix if possible */
#define RETRIES_BUFFER_SIZE 8

/*
 * Some settable constants
 */
#define STALLED_TIME 8 /* seconds */

/*
 * Flags
 */
#define DLB_DOWNLOAD          0 /* do not attempt to view the link (shift + click, etc..) */
#define DLB_ASKPATH           1 /* ask for the path for destination ("Download as...") */
#define DLB_RESUME            2 /* will attempt to resume */
#define DLB_ACTIVELIST        3 /* current entry is in the active downloads list */
#define DLB_CHANGED           4 /* download infos changed, display update needed */
#define DLB_KNOWNLENGTH       5 /* total length is known */
#define DLB_SAMPLE            6 /* sample the current size and time on the next GotData */
#define DLB_INITIAL           7 /* we need that to skip the first sampling (GotData comes just after GotInfo) */
#define DLB_CANRESUME         8 /* this node can attempt to launch a resume request if the user told us to do so */
#define DLB_ANSWERED	      9 /* user answered what to do with the file ( view, save, etc...  ) */
#define DLB_VIEW             10 /* this entry needs to be viewed when the download is finished */
#define DLB_DELETE_AFTER_USE 11 /* once the entry was viewed, delete it */
#define DLB_STALLED          12 /* the "stalled" state was reported */

#define DLF_DOWNLOAD         (1UL << DLB_DOWNLOAD)
#define DLF_ASKPATH          (1UL << DLB_ASKPATH)
#define DLF_RESUME           (1UL << DLB_RESUME)
#define DLF_ACTIVELIST       (1UL << DLB_ACTIVELIST)
#define DLF_CHANGED          (1UL << DLB_CHANGED)
#define DLF_KNOWNLENGTH      (1UL << DLB_KNOWNLENGTH)
#define DLF_SAMPLE           (1UL << DLB_SAMPLE)
#define DLF_INITIAL          (1UL << DLB_INITIAL)
#define DLF_CANRESUME        (1UL << DLB_CANRESUME)
#define DLF_ANSWERED	     (1UL << DLB_ANSWERED)
#define DLF_VIEW             (1UL << DLB_VIEW)
#define DLF_DELETE_AFTER_USE (1UL << DLB_DELETE_AFTER_USE)
#define DLF_STALLED          (1UL << DLB_STALLED)

/*
 * States
 */
#define DLS_WAITING      0 /* waiting to get a connection slot */
#define DLS_DOWNLOADING  1 /* currently downloading */
#define DLS_PAUSED       2 /* paused by the user */
#define DLS_RESUMING     3 /* trying to resume */
#define DLS_DONE         4 /* download finished */
#define DLS_FAILED       5 /* download failed */
#define DLS_USERWAIT     6 /* waiting for user feedback, entry can't be touched otherwise when into that state */
#define DLS_OVERWRITING  7 /* trying to overwrite */


/*
 * Cleanup modes
 */
#define DLC_NEVER 0
#define DLC_LEAVE_FAILURES 1
#define DLC_EVERYTHING 2

/*
 * smartreq buttons
 */
enum {
	BT_CANCEL,
	BT_RESUME,
	BT_RENAME,
	BT_OVERWRITE
};

/*
 * Ask buttons
 */
enum {
	BT_SAVE = 1,
	BT_VIEW,
	BT_SAVE_VIEW
};

struct Data {
	APTR bt_retry;
	APTR bt_cleanup;
	APTR bt_abort;
	APTR lv_dl;
	APTR str_dldir;
	APTR win_req;
	STRPTR save_path;                /* last ASL save directory */
	STRPTR view_path;                /* last ASL view directory */
	STRPTR view_file;                /* last ASL view file */
	ULONG tmpcount;                  /* temporary file count */
	ULONG id;
	struct MinList dllist;           /* contains the *active* downloads (everything but DLS_DONE) */
	struct MUI_InputHandlerNode ihn;
};


/* DestructHook */
MUI_HOOK( dl_dest, APTR pool, struct dlnode *dln )
{
	D( db_dlwin, bug( "removing dlnode 0x%lx\n", dln ) );
	//dprintf( "removing dlnode 0x%lx\n", dln);
	
	if( dln->ns )
	{
		nets_close( dln->ns );
	}

	free( dln->filename );
	free( dln->url_short );
	free( dln->lv_size );
	free( dln->lv_time );
	free( dln->lv_eta );
	//dprintf( "removing dln->eta, dln: 0x%lx ( dln->lv_cps = 0x%lx )\n", dln, dln->lv_cps );
	free( dln->lv_cps ); /* XXX: the following crashes sometimes.. why? no idea.. needs to run some memtracker */
	//dprintf( "done with lv_cps\n" );
	free( dln->lv_retries );
	free( dln->resume_url );
	free( dln->resume_referer );
	free( dln );
	
	return( 0 );
}


/* DisplayHook */
MUI_HOOK( dl_disp, STRPTR *array, struct dlnode *dln )
{
	if( dln )
	{
		/* file */
		*array++ = dln->filename; /* TOFIX: configurable to dln->filename_ptr; ? */

		/* size */
		*array++ = dln->lv_size_ptr;

		/* state */
		*array++ = dln->lv_state_ptr;

		/* time */
		*array++ = dln->lv_time_ptr;

		/* ETA */
		*array++ = dln->lv_eta_ptr;

		/* retries */
		if( dln->lv_retries )
		{
			*array++ = dln->lv_retries;
		}
		else
		{
			*array++ = "";
		}

		/* URL */
		*array = dln->url_short;
	}
	else
	{
		/* header */
		int n;

		if( nlist )
		{
			n = 2;
		}
		else
		{
			n = 0;
		}

		*array++ = GS( DOWNLOAD_LPATH ) + n;
		*array++ = GS( DOWNLOAD_LSIZE ) + n;
		*array++ = GS( DOWNLOAD_LSTATE ) + n;
		*array++ = GS( DOWNLOAD_LTIME ) + n;
		*array++ = GS( DOWNLOAD_LETA ) + n;
		*array++ = GS( DOWNLOAD_LRETRIES ) + n;
		*array = GS( DOWNLOAD_LURL ) + n;
	}
	return( 0 );
}


DECNEW
{
	struct Data *data;
	APTR bt_cleanup, bt_abort, bt_retry;
	APTR str_dldir;
	APTR lv_dl;

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID( 'D','L','W','N' ),
		MUIA_Window_Title, ( ULONG )GS( DOWNLOAD_TITLE ),
		MUIA_Window_Activate, FALSE,
		MUIA_Window_RootObject, VGroup,
			Child, HGroup,
				Child, Label2( GS( DOWNLOAD_TO ) ),
				Child, PopaslObject,
					MUIA_Popasl_Type, ASL_FileRequest,
					MUIA_Popstring_String, str_dldir = TextinputObject, StringFrame, MUIA_String_MaxLen, 256, MUIA_CycleChain, 1, End,
					MUIA_Popstring_Button, PopButton( MUII_PopDrawer ),
					ASLFR_TitleText, GS( PREFSWIN_MIME_L_DLDIR ),
					ASLFR_DrawersOnly, TRUE,
				End,
			End,

			Child, lv_dl = MUI_NewObject( listviewclass, MUIA_CycleChain, 1,
				MUIA_Listview_List, MUI_NewObject( listclass, InputListFrame,
					MUIA_Font, MUIV_Font_Tiny,
					MUIA_List_Title, TRUE,
					MUIA_List_Format, "BAR,BAR,BAR,BAR,BAR,BAR,MIW=5",
					MUIA_List_DisplayHook, &dl_disp_hook,
					MUIA_List_DestructHook, &dl_dest_hook,
				End,
			End,

			Child, HGroup,
				Child, bt_retry = makebutton( MSG_DOWNLOAD_BT_RETRY ),
				Child, bt_abort = makebutton( MSG_DOWNLOAD_BT_ABORT ),
				Child, bt_cleanup = makebutton( MSG_DOWNLOAD_BT_CLEANUP ),
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->bt_retry = bt_retry;
	data->bt_cleanup = bt_cleanup;
	data->bt_abort = bt_abort;
	data->lv_dl = lv_dl;
	data->str_dldir = str_dldir;

	NEWLIST( &data->dllist ); /* object always stay around once opened fo the first time */

	/* Sets the text of the download string */
	set( str_dldir, MUIA_String_Contents, getprefsstr( DSI_SAVEDEST, "RAM:" ) );

	/* Changes the button text/states depending on the selected entry */
	DoMethod( lv_dl, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		obj, 2, MM_Downloadwin_SetButtons, MUIV_TriggerValue
	);
	/* And if the number of entries change (TOFIX: maybe it's not needed.. check) */
	DoMethod( lv_dl, MUIM_Notify, MUIA_List_Entries, MUIV_EveryTime,
		obj, 2, MM_Downloadwin_SetButtons, MUIV_TriggerValue
	);

	/* Retry/Pause button */
	DoMethod( bt_retry, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MM_Downloadwin_Retry, getv( data->lv_dl, MUIA_List_Active )
	);

	/* Abort button */
	DoMethod( bt_abort, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MM_Downloadwin_Abort, getv( data->lv_dl, MUIA_List_Active )
	);

	/* Cleanup button */
	DoMethod( bt_cleanup, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Downloadwin_Cleanup
	);

	/* Close gadget */
	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 6, MUIM_Application_PushMethod, ( ULONG )obj, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);

	DoMethod( obj, MM_Downloadwin_SetButtons, MUIV_List_Active_Off ); /* TOFIX: what about automatic cross session resume ? */

	return( (ULONG)obj );
}

/*
 * Given a name and optionally a path,
 * allocates and fill in the filename structure.
 * If the name is not given just the path is updated.
 */
DECSMETHOD( Downloadwin_AddFile )
{
	ULONG size;
	char f[ 256 ];

	if( msg->filename )
	{
		/*
		 * Update everything.
		 */
		strcpy( f, msg->filename );
		D( db_dlwin, bug( "updating everything, path: %s, file: %s\n", msg->path, msg->filename ) );
		free( msg->dl->filename );
		msg->dl->filename = NULL;

		size = strlen( f );
		if( msg->path )
		{
			size += strlen( msg->path );
		}

		if( size && ( msg->dl->filename = malloc( size + 2 ) ) )
		{
			msg->dl->filename[ 0 ] = '\0';
			if( msg->path )
			{
				strcat( msg->dl->filename, msg->path );
			}
			AddPart( msg->dl->filename, f, size + 2 );

			msg->dl->filename_ptr = FilePart( msg->dl->filename );

			D( db_dlwin, bug( "updated to filename: %s (filename_ptr: %s)\n", msg->dl->filename, msg->dl->filename_ptr ) );

			return( TRUE );
		}
	}
	else
	{
		/*
		 * Update the path only.
		 */
		D( db_dlwin, bug( "updating the path\n" ) );
		strcpy( f, FilePart( msg->dl->filename ) );
		free( msg->dl->filename );
		msg->dl->filename = NULL;

		size = strlen( msg->path ) + strlen( f );

		if( size && ( msg->dl->filename = malloc( size + 2 ) ) )
		{
			strcpy( msg->dl->filename, msg->path );
			AddPart( msg->dl->filename, f, size + 2 );

			msg->dl->filename_ptr = FilePart( msg->dl->filename );

			D( db_dlwin, bug( "the path is: %s\n", msg->dl->filename ) );

			return( TRUE );
		}
	}
	return( FALSE );
}


/*
 * Given a name and optionally a path,
 * allocates and fill in the filename structure.
 */
DECSMETHOD( Downloadwin_AddViewer )
{
	ULONG size;

	free( msg->dl->viewapp );
	msg->dl->viewapp = NULL;

	size = strlen( msg->filename );
	if( msg->path )
	{
		size += strlen( msg->path );
	}

	if( size && ( msg->dl->viewapp = malloc( size + 2 ) ) )
	{
		msg->dl->viewapp[ 0 ] = '\0';
		if( msg->path )
		{
			strcat( msg->dl->viewapp, msg->path );
		}
		AddPart( msg->dl->viewapp, msg->filename, size + 2 );

		D( db_dlwin, bug( "viewapp: %s\n", msg->dl->viewapp ) );

		return( TRUE );
	}
	return( FALSE );
}


/*
 * Given a path and a filename, opens an ASL requester
 * to chose a path.. and a filename.
 *
 * TOFIX: should really be non-blocking but
 * this is not that easy.
 */
DECSMETHOD( Downloadwin_SelectFile )
{
	GETDATA;
	struct Screen *scr;
	struct FileRequester *fr;
	ULONG rc = FALSE;

	scr = ( struct Screen * )getv( obj, MUIA_Window_Screen );

	set( app, MUIA_Application_Sleep, TRUE );

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		scr ? ASLFR_Screen : TAG_IGNORE, scr,
		ASLFR_TitleText, GS( PREFSWIN_MIME_L_DLDIR ),
		ASLFR_DoSaveMode, TRUE,
		ASLFR_RejectIcons, FALSE,
		ASLFR_InitialDrawer, data->save_path ? data->save_path : ( STRPTR )getv( obj, MA_Downloadwin_Path ),
		ASLFR_InitialFile, msg->filename,
		TAG_DONE
	);

	if( fr )
	{
		if( MUI_AslRequestTags( fr, TAG_DONE ) )
		{
			/* update ASL save path */
			if( data->save_path && stricmp( data->save_path, fr->fr_Drawer ) || !data->save_path )
			{
				free( data->save_path );

				if( ( data->save_path = malloc( strlen( fr->fr_Drawer ) + 1 ) ) )
				{
					strcpy( data->save_path, fr->fr_Drawer );
				}
			}

			if( DoMethod( obj, MM_Downloadwin_AddFile, msg->dl, fr->fr_Drawer, fr->fr_File ) )
			{
				D( db_dlwin, bug( "selected file is %s\n", msg->dl->filename ) );
				rc = TRUE;
			}
			MUI_FreeAslRequest( fr );
		}
	}

	set( app, MUIA_Application_Sleep, FALSE );

	return( rc );
}


DECSMETHOD( Downloadwin_SelectViewer )
{
	GETDATA;
	struct Screen *scr;
	struct FileRequester *fr;
	ULONG rc = FALSE;

	scr = ( struct Screen * )getv( obj, MUIA_Window_Screen );

	set( app, MUIA_Application_Sleep, TRUE );

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		scr ? ASLFR_Screen : TAG_IGNORE, scr,
		ASLFR_TitleText, GS( PREFSWIN_MIME_E_APP ),
		ASLFR_RejectIcons, FALSE,
		ASLFR_InitialDrawer, ( STRPTR )data->view_path ? ( STRPTR )data->view_path : ( STRPTR )"SYS:Utilities",
		ASLFR_InitialFile, ( STRPTR )data->view_file ? ( STRPTR )data->view_file : ( STRPTR )"Multiview",
		TAG_DONE
	);

	if( fr )
	{
		if( MUI_AslRequestTags( fr, TAG_DONE ) )
		{
			/* update ASL view path */
			if( data->view_path && stricmp( data->view_path, fr->fr_Drawer ) || !data->view_path )
			{
				free( data->view_path );

				if( ( data->view_path = malloc( strlen( fr->fr_Drawer ) + 1 ) ) )
				{
					strcpy( data->view_path, fr->fr_Drawer );
				}
			}

			/* update ASL view file */
			if( data->view_file && stricmp( data->view_file, fr->fr_File ) || !data->view_file )
			{
				free( data->view_file );

				if( ( data->view_file = malloc( strlen( fr->fr_File ) + 1 ) ) )
				{
					strcpy( data->view_file, fr->fr_File );
				}
			}

			if( DoMethod( obj, MM_Downloadwin_AddViewer, msg->dl, fr->fr_Drawer, fr->fr_File ) )
			{
				rc = TRUE;
			}
			MUI_FreeAslRequest( fr );
		}
	}

	set( app, MUIA_Application_Sleep, FALSE );

	return( rc );
}


/*
 * Runs a viewer.
 *
 * TOFIX: this should really be done by a separate task. There's no
 * cleanup done immediately and a big file could end up in the
 * cache.
 */
DECSMETHOD( Downloadwin_View )
{
	char cmd[ 512 ];
	struct Screen *scr;
	struct List *psl;
	struct PubScreenNode *psn;

	scr = ( struct Screen * )getv( obj, MUIA_Window_Screen );

	psl = LockPubScreenList();

	for( psn = FIRSTNODE( psl ); NEXTNODE( psn ); psn = NEXTNODE( psn ) )
	{
		if( psn->psn_Screen == scr )
		{
			break;
		}
	}

	if( !NEXTNODE( psn ) )
	{
		psn = NULL;
	}

	UnlockPubScreenList();

	if( strchr( msg->dl->viewapp, '%' ) )
	{
		expandtemplate( msg->dl->viewapp, cmd, sizeof( cmd ),
			'f', msg->dl->filename,
			'p', psn ? psn->psn_Node.ln_Name : "",
			NULL
		);
	}
	else
	{
		sprintf( cmd, "%s %s", msg->dl->viewapp, msg->dl->filename );
	}

	D( db_dlwin, bug( "async launching of %s\n", cmd ) );

	mySystemTags( cmd,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);

	return( 0 );
}


/*
 * Enqueues a request.
 */
DECSMETHOD( Downloadwin_Enqueue )
{
	GETDATA;
	struct dlnode *dln;

	if( ( dln = malloc( sizeof( struct dlnode ) ) ) )
	{
		STRPTR q;
		int size = strlen( msg->url ) + 1;
		memset( dln, '\0', sizeof( struct dlnode ) );

		/* Build the complete local filename path */
		if( ( q = malloc( size ) ) )
		{
			struct parsedurl purl;
			STRPTR u;
			STRPTR p = NULL;

			stccpy( q, msg->url, size );
			uri_split( q, &purl );
			if( purl.path )
			{
				p = FilePart( purl.path );
			}
				
			if( p && *p )
			{
				uri_decode( p ); /* %20 to spaces, etc.. */
				size = strlen( p ) + 1;
				u = malloc( size );

				if( u )
				{
					ULONG err = FALSE;
					stccpy( u, p, size );

					/* TOFIX: hm, do we need to add index.html ? */

					if( msg->flags & DLF_ASKPATH )
					{
						if( !DoMethod( obj, MM_Downloadwin_SelectFile, dln, u ) )
						{
							err	= TRUE;
						}
					}
					else
					{
						if( !DoMethod( obj, MM_Downloadwin_AddFile, dln, ( STRPTR )getv( obj, MA_Downloadwin_Path ), u ) )
						{
							err = TRUE;
						}
					}
					
					if( !err )
					{
	                    free( u );
						/* flags */
						dln->flags = msg->flags;
							
						/* state */
						dln->state = DLS_WAITING;

						/* ID */
						dln->id = ++data->id;

						/* short URL name for display */
						if( ( dln->url_short = malloc( URL_SHORT_SIZE ) ) )
						{
							stccpy( dln->url_short, msg->url, URL_SHORT_SIZE );
							if( strlen( dln->url_short ) >= URL_SHORT_SIZE - 4 );
							{
								strcpy( dln->url_short + URL_SHORT_SIZE - 4, "..." );
							}
							
							if( ( dln->lv_retries = malloc( RETRIES_BUFFER_SIZE ) ) )
							{
								snprintf( dln->lv_retries, RETRIES_BUFFER_SIZE, "%lu", dln->retries );
							}

							/* default entries */
							DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_WAITING );

							if( ( dln->ns = nets_open( msg->url, msg->referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
							{
								DoMethod( data->lv_dl, MUIM_List_InsertSingle, dln, MUIV_List_Insert_Bottom );
								set( data->lv_dl, MUIA_List_Active, MUIV_List_Active_Bottom );

								dln->flags |= DLF_ACTIVELIST;
								ADDTAIL( &data->dllist, dln );

								if( !getv( obj, MUIA_Window_Open ) )
								{
									set( obj, MUIA_Window_Open, TRUE );
								}
									
								if( getv( obj, MUIA_Window_Open ) )
								{
									D( db_dlwin, bug( "dlnode 0x%lx successfully enqueued\nfilename: %s, url: %s, offset %ld, length %ld\n", dln, dln->filename, dln->url_short, dln->offset, dln->length ) );
									return( TRUE );
								}
								/* failure: fallback */
								nets_close( dln->ns );
								dln->ns = NULL;
							}
							free( dln->url_short );
						}
					}
					free( u );
				}
			}
			free( q );
		}
		free( dln );
	}
	return( FALSE );
}


DECGET
{
	if( msg->opg_AttrID == MA_Downloadwin_Path )
	{
		GETDATA;
		STRPTR p;
		
		/*
		 * Return what is in the string, otherwise what the
		 * user configured in the prefs, otherwise RAM:
		 */
		p = ( STRPTR )getv( data->str_dldir, MUIA_String_Contents );
		if( *p )
		{
			*msg->opg_Storage = ( ULONG )p;
		}
		else
		{
			*msg->opg_Storage = ( ULONG )getprefsstr( DSI_SAVEDEST, "RAM:" );
		}
		return( TRUE );
	}
	return( DOSUPER );
}


/*
 * Changes the buttons states/texts depending
 * on the currently selected entry.
 */
DECSMETHOD( Downloadwin_SetButtons )
{
	GETDATA;

	if( msg->entry == MUIV_List_Active_Off )
	{
		DoMethod( obj, MUIM_MultiSet, MUIA_Disabled, TRUE,
			data->bt_retry, data->bt_abort, data->bt_cleanup, NULL
		);
		set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_RETRY ) );
	}
	else
	{
		struct dlnode *dln;
		
		DoMethod( data->lv_dl, MUIM_List_GetEntry, msg->entry, &dln );

		switch( dln->state )
		{
			case DLS_WAITING:
			case DLS_DOWNLOADING:
				set( data->bt_retry, MUIA_Disabled, FALSE );
				set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_PAUSE ) );
				set( data->bt_abort, MUIA_Disabled, FALSE );
				set( data->bt_cleanup, MUIA_Disabled, FALSE ); /* TOFIX: that button is special.. */
				break;

			case DLS_PAUSED:
				set( data->bt_retry, MUIA_Disabled, FALSE );
				set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_RETRY ) );
				set( data->bt_abort, MUIA_Disabled, FALSE );
				set( data->bt_cleanup, MUIA_Disabled, FALSE ); /* TOFIX: same.. */
				break;

			case DLS_RESUMING:
			case DLS_OVERWRITING:
				set( data->bt_retry, MUIA_Disabled, FALSE );
				set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_PAUSE ) );
				set( data->bt_abort, MUIA_Disabled, FALSE );
				set( data->bt_cleanup, MUIA_Disabled, FALSE ); /* TOFIX: :)	*/
				break;

			case DLS_DONE:
				set( data->bt_retry, MUIA_Disabled, TRUE );
				set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_RETRY ) );
				set( data->bt_abort, MUIA_Disabled, FALSE ); /* TOFIX: this button is always false isn't it ? */
				set( data->bt_cleanup, MUIA_Disabled, FALSE ); /* TOFIX: plop */
				break;

			case DLS_FAILED:
				set( data->bt_retry, MUIA_Disabled, FALSE );
				set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_RETRY ) );
				set( data->bt_abort, MUIA_Disabled, FALSE );
				set( data->bt_cleanup, MUIA_Disabled, FALSE ); /* TOFIX: hmm */
				break;

			case DLS_USERWAIT:
				set( data->bt_retry, MUIA_Disabled, TRUE );
				set( data->bt_retry, MUIA_Text_Contents, GS( DOWNLOAD_BT_RETRY ) );
				set( data->bt_abort, MUIA_Disabled, TRUE );
				set( data->bt_cleanup, MUIA_Disabled, FALSE ); /* TOFIX: hm */
				break;
		}
	}
	return( 0 );
}


DECSMETHOD( NStream_GotInfo )
{
	GETDATA;
	struct dlnode *dln;

	/*
	 * Check if it's needed to add an InputHandler
	 * to refresh the list periodically.
	 */
	if( !data->ihn.ihn_Object )
	{
		data->ihn.ihn_Object = obj;
		data->ihn.ihn_Flags = MUIIHNF_TIMER;
		data->ihn.ihn_Millis = 1000;
		data->ihn.ihn_Method = MM_Downloadwin_Refresh;

		DoMethod( _app( obj ), MUIM_Application_AddInputHandler, &data->ihn );
	}

	D( db_dlwin, bug( "gotinfo for nstream 0x%lx\n", msg->ns ) );

	ITERATELIST( dln, &data->dllist )
	{
		if( dln->ns == msg->ns )
		{
			BPTR l;
			ULONG offset = 0;
			ULONG mime_found;
			int viewprefs;
			D_S( struct FileInfoBlock, fib );

			if( nets_state( dln->ns ) != UNS_FAILED )
			{
				/*
				 * Check if it's a redirect.
				 */
				if( nets_redirecturl( dln->ns ) )
				{
					STRPTR buf;
					struct nstream *ns;
					STRPTR redirect = nets_redirecturl( dln->ns );
					STRPTR url = nets_url( dln->ns );

					if( ( buf = malloc( strlen( url ) + strlen( redirect) + 1 ) ) )
					{
						uri_mergeurl( url, redirect, buf );
						ns = nets_open( buf, nets_referer( dln->ns ), obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS );
						nets_close( dln->ns );
						dln->ns = ns;
						if( dln->ns )
						{
							dln->lv_state_ptr = GS( DOWNLOAD_STATE_REDIRECT );
						}
						else
						{
							dln->lv_state_ptr = GS( DOWNLOAD_STATE_OUTOFMEM );
						}
						free( buf );
					}
					else
					{
						displaybeep();
						dln->lv_state_ptr = GS( DOWNLOAD_STATE_OUTOFMEM );
					}
					DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
					break;
				}

				/*
				 * Check what the user configured in mimeprefs.
				 */
				if( !( dln->flags & DLF_ANSWERED ) && !( dln->flags & DLF_DOWNLOAD ) )
				{
					char savedir[ 256 ];
					char viewer[ 256 ];
					
					mime_found = mime_findbytype( nets_mimetype( dln->ns ), savedir, viewer, &viewprefs );

					D( db_dlwin, bug( "viewprefs == 0x%lx\n", viewprefs ) );

					if( mime_found )
					{
						D( db_dlwin, bug( "mimetype found, acting on path: %s, filename: %s\n", savedir, dln->filename_ptr ) );
						if( !DoMethod( obj, MM_Downloadwin_AddFile, dln, savedir, NULL ) )
						{
							displaybeep();
							return( 0 );
						}

						switch( viewprefs )
						{
							case MT_ACTION_ASK:
								/*
								 * Ask what to do.
								 */
								DoMethod( obj, MM_Downloadwin_InitReq, dln );
								DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_USERWAIT );
								DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
								D( db_dlwin, bug( "node closed\n" ) );
								smartreq_request( &data->win_req,
									GS( DOWNLOAD_ASK_KNOWN_TITLE ),
									obj,
									MM_SmartReq_Ask,
									dln->id,
									GS( DOWNLOAD_ASK_GADGETS ),
									GS( DOWNLOAD_ASK_KNOWN_BODY ),
									nets_mimetype( dln->ns ),
									dln->url_short
								);
								DoMethod( obj, MM_Downloadwin_ExitReq, dln );
								return( 0 );
						

							case MT_ACTION_VIEW:
								/*
								 * We use a viewer.
								 */
								if( viewer[ 0 ] )
								{
									if( DoMethod( obj, MM_Downloadwin_AddViewer, dln, NULL, viewer ) )
									{
										char buf[ 32 ];

										sprintf( buf, "VViewTemp.%02d", ( int )++data->tmpcount % 100 ); /* TOFIX: not very smart :) */

										if( DoMethod( obj, MM_Downloadwin_AddFile, dln, gp_cachedir, buf ) )
										{
											D( db_dlwin, bug( "viewer accepted, downloading to %s\n", dln->filename ) );
											dln->flags |= DLF_VIEW;
											dln->flags |= DLF_DELETE_AFTER_USE;

											break;
										}
									}
									return( 0 ); /* TOFIX: not nice */
								}
								break;

							case MT_ACTION_SAVE_AND_VIEW:
								/*
								 * Save and view, TOFIX: finish
								 */
								if( viewer[ 0 ] )
								{
									if( DoMethod( obj, MM_Downloadwin_AddViewer, dln, NULL, viewer ) )
									{
										D( db_dlwin, bug( "added viewer %s\n", dln->viewapp ) );
										dln->flags |= DLF_VIEW;
									}
									else
									{
										return( 0 ); /* TOFIX: not nice */
									}
								}
								// fallthrough

							case MT_ACTION_SAVE:
								/*
								 * We save directly.
								 */
								if( savedir[ 0 ] )
								{
									D( db_dlwin, bug( "saving to %s\n", savedir ) );
									DoMethod( obj, MM_Downloadwin_UpdateEntry, dln ); /* update the path */
								}
								break;
						}
					
					}
					else
					{
						/*
						 * No mimeprefs configured and the file is
						 * not necessarily to be downloaded.
						 */
						DoMethod( obj, MM_Downloadwin_InitReq, dln );
						DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_USERWAIT );
						DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
						D( db_dlwin, bug( "node closed\n" ) );
						smartreq_request( &data->win_req,
							GS( DOWNLOAD_ASK_TITLE ),
							obj,
							MM_SmartReq_Ask,
							dln->id,
							GS( DOWNLOAD_ASK_GADGETS ),
							GS( DOWNLOAD_ASK_UNKNOWN_BODY ),
							nets_mimetype( dln->ns ),
							dln->url_short
						);
						DoMethod( obj, MM_Downloadwin_ExitReq, dln );
						break;
					}
				}

				/* size */
				if( ( dln->length = nets_getdoclen( dln->ns ) ) != -1 )
				{
					dln->flags |= DLF_KNOWNLENGTH;
				}
				else
				{
					dln->lv_eta_ptr = GS( DOWNLOAD_STATE_NA );
				}

				D( db_dlwin, bug( "trying to lock file %s\n", dln->filename ) );

				/*
				 * Check if the file exists so that we can resume.
				 */
				if( ( l = Lock( dln->filename, ACCESS_READ ) ) )
				{
					D( db_dlwin, bug( "locked !\n" ) );
					if( Examine( l, fib ) == DOSTRUE ) /* TOFIX: in which cases can Examine() fail ? */
					{
						char r_date[ 32 ], l_date[ 32 ];
						char serverfilesize[ 12 ];
						APTR gadgets;
						
						if( dln->state == DLS_RESUMING )
						{
							D( db_dlwin, bug( "storing offset %ld\n", fib->fib_Size ) );
							offset = fib->fib_Size; /* TOFIX: detect if it changed between the request and now then insult the user if it did */
						}
						else if ( dln->state != DLS_OVERWRITING ) /* TOFIX: hacky.. put a switch() case and add a DLS_RENAMING too maybe */
						{
							int success = FALSE;
							
							if( ( gadgets = malloc( strlen( GS( DOWNLOAD_RESUME_GADGETS ) ) + strlen( GS( DOWNLOAD_RESUME_RESUMEGAD ) ) + 5 ) ) )
							{
								DoMethod( obj, MM_Downloadwin_InitReq, dln );
								
								strcpy( gadgets, "*\033b" );

								strcpy( l_date, datestamp2string( &fib->fib_Date ) );

								if( dln->ns->un->lastmodified )
								{
									strcpy( r_date, date2string( dln->ns->un->lastmodified ) );
								}

								/* resume button ? */
								if( ( dln->flags & DLF_KNOWNLENGTH && fib->fib_Size < dln->length ) || !( dln->flags & DLF_KNOWNLENGTH ) )
								{
									strcat( gadgets, GS( DOWNLOAD_RESUME_RESUMEGAD ) );
									strcat( gadgets, "|" );
								}
								strcat( gadgets, GS( DOWNLOAD_RESUME_GADGETS ) );
										
			                    if( dln->flags & DLF_KNOWNLENGTH )
								{
									stcul_d( serverfilesize, dln->length );

									D( db_dlwin, bug( "length is known\n" ) );

			                        if( fib->fib_Size > dln->length )
									{
										/*
										 * Sanity check. If the remote file size is known and the
										 * local size is bigger than it, don't attempt a resume but
										 * warn the user and ask him what to do.
										 */
										D( db_dlwin, bug( "filesize is greater than length\n" ) );
										smartreq_request( &data->win_req, /* TOFIX: check return code */
											GS( DOWNLOAD_RESUME_TITLE ),
											obj,
											NULL,
											dln->id,
											gadgets,
											GS( DOWNLOAD_RESUME_BODY ),
											dln->filename_ptr,
											dln->ns->un->lastmodified ? ( dln->ns->un->lastmodified < __datecvt( &fib->fib_Date ) ? GS( DOWNLOAD_RESUME_BIGGER_NEWER ) : GS( DOWNLOAD_RESUME_BIGGER_OLDER ) ) : GS( DOWNLOAD_RESUME_BIGGER ),
											serverfilesize,
											dln->ns->un->lastmodified ? ( ULONG )r_date : ( ULONG )GS( DOWNLOAD_RESUME_UNKNOWN ),
											fib->fib_Size,
											l_date
										);

									}
									else if( fib->fib_Size == dln->length )
									{
										/*
										 * Both files are equal. Disable resuming and ask the
										 * user what he wants to do.
										 */
										D( db_dlwin, bug( "filesize is the same as the length\n" ) );
										smartreq_request( &data->win_req, /* TOFIX: check return code */
											GS( DOWNLOAD_RESUME_TITLE ),
											obj,
											NULL,
											dln->id,
											gadgets,
											GS( DOWNLOAD_RESUME_BODY ),
											dln->filename_ptr,
											dln->ns->un->lastmodified ? ( dln->ns->un->lastmodified < __datecvt( &fib->fib_Date ) ? GS( DOWNLOAD_RESUME_SAME_NEWER ) : GS( DOWNLOAD_RESUME_SAME_OLDER ) ) : GS( DOWNLOAD_RESUME_SAME ),
											serverfilesize,
											dln->ns->un->lastmodified ? ( ULONG )r_date : ( ULONG )GS( DOWNLOAD_RESUME_UNKNOWN ),
											fib->fib_Size,
											l_date
										);
									}
									else
									{
										/*
										 * Normal resume.
										 */
										D( db_dlwin, bug( "normal resume\n" ) );
										smartreq_request( &data->win_req, /* TOFIX: check return code */
											GS( DOWNLOAD_RESUME_TITLE ),
											obj,
											NULL,
											dln->id,
											gadgets,
											GS( DOWNLOAD_RESUME_BODY ),
											dln->filename_ptr,
											dln->ns->un->lastmodified ? ( ULONG )( dln->ns->un->lastmodified < __datecvt( &fib->fib_Date ) ? ( ULONG )GS( DOWNLOAD_RESUME_NEWER ) : ( ULONG )GS( DOWNLOAD_RESUME_OLDER ) ) : ( ULONG )"",
											serverfilesize,
											dln->ns->un->lastmodified ? ( ULONG )r_date : ( ULONG )GS( DOWNLOAD_RESUME_UNKNOWN ),
											fib->fib_Size,
											l_date
										);
										dln->flags |= DLF_CANRESUME;
									}
								}
								else
								{
									/*
									 * Normal resume without knowing the size.
									 */
									D( db_dlwin, bug( "normal resume but with unknown size\n" ) );
									smartreq_request( &data->win_req, /* TOFIX: check return code */
										GS( DOWNLOAD_RESUME_TITLE ),
										obj,
										NULL,
										dln->id,
										gadgets,
										GS( DOWNLOAD_RESUME_BODY ),
										dln->filename_ptr,
										dln->ns->un->lastmodified ? ( ULONG )( dln->ns->un->lastmodified < __datecvt( &fib->fib_Date ) ? ( ULONG )GS( DOWNLOAD_RESUME_NEWER ) : ( ULONG )GS( DOWNLOAD_RESUME_OLDER ) ) : ( ULONG )"",
										GS( DOWNLOAD_RESUME_UNKNOWN ),
										dln->ns->un->lastmodified ? ( ULONG )r_date : ( ULONG )GS( DOWNLOAD_RESUME_UNKNOWN ),
										fib->fib_Size,
										l_date
									);
									dln->flags |= DLF_CANRESUME;
								}
								free( gadgets );
								D( db_dlwin, bug( "done, closing node..\n" ) );
								/*
								 * The node is now waiting for user choice. We can
								 * close it to let the network slots continue
								 * processing.
								 * Note: although it looks like it, there's no race condition.
								 * This method needs to finish first before the user can
								 * do any input into the smart requester.
								 */
								DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_USERWAIT );
								DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
								D( db_dlwin, bug( "node closed\n" ) );
								DoMethod( obj, MM_Downloadwin_ExitReq, dln );
								success = TRUE;
		                    }
							
							if( !success )
							{
								displaybeep();
							}
						}
					}
					UnLock( l );
				}
				
				/* TOFIX: what happens if we download to the same file twice ? -> failure I think, but be smarter maybe */
					
				if( dln->state == DLS_WAITING || dln->state == DLS_RESUMING || dln->state == DLS_OVERWRITING )
				{
					if ( dln->lv_size )
						free( dln->lv_size );
					dln->lv_size = malloc( LENGTH_BUFFER_SIZE ); /* TOFIX: remove the tofix once *every* part checks if != NULL before */
					
					if ( dln->lv_time )
						free( dln->lv_time );
					dln->lv_time = malloc( TIME_BUFFER_SIZE ); /* TOFIX: same as above */
					
					if ( dln->lv_eta )
						free( dln->lv_eta );
					dln->lv_eta = malloc( ETA_BUFFER_SIZE );
					
					if ( dln->lv_cps )
						free( dln->lv_cps );
					dln->lv_cps = malloc( CPS_BUFFER_SIZE );

					dln->lv_time_ptr = "0:00:00";
					
					if( dln->state != DLS_RESUMING && dln->state != DLS_OVERWRITING )
					{
						dln->lv_state_ptr = GS( DOWNLOAD_STATE_FETCHING );
					}

					DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_DOWNLOADING );

					dln->start_time = dln->act_snap.time = dln->last_snap.time = msg->ts;
					dln->act_snap.docptr = dln->last_snap.docptr = msg->docptr;

					dln->flags |= DLF_SAMPLE;
					
					/*
					 * This is done because we sample at the start,
					 * thus the first sampling would be too inacurate.
					 */
					dln->flags |= DLF_INITIAL;
					nets_settofile( dln->ns, dln->filename, offset );
				}
			}
			/*
			 * If we failed, the NStream_Done will
			 * arrive pretty soon anyway.
			 */

			DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
			D( db_dlwin, bug( "got info for node 0x%lx\n", dln ) );
			break;
		}
	}
	return( 0 );
}


DECSMETHOD( NStream_GotData )
{
	GETDATA;
	struct dlnode *dln;

	ITERATELIST( dln, &data->dllist )
	{
		if( dln->ns == msg->ns )
		{
			dln->flags |= DLF_CHANGED;
			if( dln->flags & DLF_SAMPLE )
			{
				dln->act_snap.time = msg->ts;
				dln->act_snap.docptr = msg->docptr;
				D( db_dlwin, bug ( "changed!\n" ) );
				dln->flags &= ~DLF_SAMPLE;
			}
			break;
		}
	}
	return( 0 );
}


DECSMETHOD( NStream_Done )
{
	struct dlnode *dln;
	GETDATA;

	ITERATELIST( dln, &data->dllist )
	{
		if( dln->ns == msg->ns )
		{
			if( nets_state( dln->ns ) > 0 )
			{
				D( db_dlwin, bug( "got done for 0x%lx (state: DLS_DONE)\n", dln ) );
				/* success */
				DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_DONE );

				/*
				 * TOFIX: add viewer launching here.
				 */
				if( dln->flags & DLF_VIEW )
				{
					D( db_dlwin, bug( "viewing..\n" ) );
					DoMethod( obj, MM_Downloadwin_View, dln );
				}

				/*
				 * Do we automatically remove the entry ?
				 */
				if( getflag( VFLG_AUTOCLEANUP_MODES ) != DLC_NEVER )
				{
					DoMethod( obj, MM_Downloadwin_RemoveEntry, dln );
					break;
				}

				/* final CPS */
				if( dln->lv_cps )
				{
					if( dln->flags & DLF_KNOWNLENGTH )
					{
						time_t gone = ( timedm() - dln->start_time ) / 1000;
						snprintf( dln->lv_cps, CPS_BUFFER_SIZE, "\033bdone; %lu cps", ( dln->length - dln->offset ) / ( gone > 0 ? gone : 1 ) );  /* TOFIX: not locale proof */
						dln->lv_state_ptr = dln->lv_cps;
					}
					else
					{
						dln->lv_state_ptr = "\033bdone;";
					}
				}
				if ( dln->lv_size )
				{
					snprintf( dln->lv_size, LENGTH_BUFFER_SIZE, "%u (100%%)", nets_getdocrealptr( dln->ns ) ); /* TOFIX: msg->docptr ? */
				}
				DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
				nets_close( dln->ns );
				dln->ns = NULL;
			}
			else
			{
				D( db_dlwin, bug( "got done for 0x%lx (state: DLS_FAILED)\n", dln ) );
				/* failed */
				
				if( dln->retries++ < gp_download_retries )
				{
					/*
					 * Automatic retry
					 */
					D( db_dlwin, bug( "running automatic retry ( %ld time )\n", dln->retries ) );
					DoMethod( obj, MM_Downloadwin_InitReq, dln );

					DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_RESUMING );
							
					if( dln->lv_retries ) /* TOFIX: is that ok ? */
					{
						snprintf( dln->lv_retries, RETRIES_BUFFER_SIZE, "%lu", dln->retries );
					}

					DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
					
					DoMethod( obj, MM_Downloadwin_ExitReq, dln );
					if( ( dln->ns = nets_open( dln->resume_url, dln->resume_referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
					{
						DoMethod( obj, MM_Downloadwin_FreeResume, dln );
						if( !getv( obj, MUIA_Window_Open ) )
						{
							set( obj, MUIA_Window_Open, TRUE ); /* TOFIX: could be cleaner.. hm.. */
						}
						break;
					}
				}
				else
				{
					/*
					 * Do we automatically remove the entry ?
					 */
					if( getflag( VFLG_AUTOCLEANUP_MODES ) == DLC_EVERYTHING )
					{
						DoMethod( obj, MM_Downloadwin_RemoveEntry, dln );
						break;
					}
				}

				DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_FAILED );
				DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
				/* TOFIX: we have to nets_close() somewhere, you know. automatically remove the entry ? See above */
				if( dln->ns )
				{
					nets_close( dln->ns ); /* TOFIX: THIS IS TEMPORARY */
					dln->ns = NULL;
				}
			}
			break;
		}
	}

	/*
	 * Check if we close the window.
	 */
	if( ISLISTEMPTY( &data->dllist ) && getflag( VFLG_AUTOCLOSEDLWIN ) )
	{
		set( obj, MUIA_Window_Open, FALSE );
	}

	return( 0 );
}


/*
 * Called to remove an entry properly.
 */
DECSMETHOD( Downloadwin_RemoveEntry )
{
	GETDATA;

	if( msg->dl->flags & DLF_ACTIVELIST )
	{
		msg->dl->flags &= ~DLF_CHANGED; /* we refresh from here, not MM_Downloadwin_Refresh */
		msg->dl->flags &= ~DLF_ACTIVELIST;
		REMOVE( msg->dl );
		DoMethod( data->lv_dl, MUIM_List_Remove, DoMethod( obj, MM_Downloadwin_FindEntry, msg->dl ) );
	}
	return( 0 );
}


/*
 * Given a dlnode, return the list entry.
 */
DECSMETHOD( Downloadwin_FindEntry )
{
	GETDATA;
	ULONG c;
	struct dlnode *dln;

	/*
	 * Well, we need to scan the list to find which
	 * number to refresh. There could be a renumber of
	 * the entries only on additions/removal but given
	 * the way the downloadwindow works, fresh downloads
	 * are *VERY* likely to be on the top of the list.
	 */
	for( c = 0; ; c++ )
	{
		DoMethod( data->lv_dl, MUIM_List_GetEntry, c, &dln );
		//if( dln->ns == msg->dl->ns )
		if( dln == msg->dl )
		{
			break;
		}
	}
	return( c );
}


/*
 * Called once the values of an entry have been
 * computed to refresh them (display).
 */
DECSMETHOD( Downloadwin_UpdateEntry )
{
	GETDATA;

	DoMethod( data->lv_dl, MUIM_List_Redraw, DoMethod( obj, MM_Downloadwin_FindEntry, msg->dl ) );
	return( 0 );
}


/*
 * Called to compute the values in the listview.
 * Only DLS_DOWNLOADING is computed from there as
 * the others are immediate and not on a 1 second
 * refresh basis.
 */
DECTMETHOD( Downloadwin_Refresh )
{
	GETDATA;
	struct dlnode *dln;

	ITERATELIST( dln, &data->dllist )
	{
		if( dln->state & DLS_DOWNLOADING && ( dln->ns && nets_state( dln->ns ) != UNS_FAILED ) ) /* TOFIX: hackish */
		{
			if( dln->flags & DLF_CHANGED )
			{
				dln->stall = 0;
				dln->flags &= ~DLF_STALLED;

				D( db_dlwin, bug( "processing node 0x%lx\n", dln ) );

				/* state (cps), size progress and time computation */
				if( dln->lv_size && dln->lv_time && dln->lv_eta && dln->lv_cps ) /* TOFIX: ETA should be allocated when required only */
				{
					int docptr;

					if( dln->update_count++ == 3 )
					{
						float speed;
						time_t gone = ( dln->act_snap.time - dln->start_time ) / 1000;
						if( gone > 9999 * 3600 )
						{
							dln->lv_time_ptr = GS( DOWNLOAD_STATE_OVERFLOW ); /* probably can't happen but we handle it anyway (416 days, hehe) :) */
						}
						else
						{
							dln->lv_time_ptr = dln->lv_time;
						}
							
						speed = ( float )( dln->act_snap.docptr - dln->last_snap.docptr ) / ( ( float )( dln->act_snap.time - dln->last_snap.time ) / 1000 );
						D( db_dlwin, bug( "act.docptr %ld, last.docptr %ld | act.time %ld, last.time %ld\n", dln->act_snap.docptr, dln->last_snap.docptr, dln->act_snap.time, dln->last_snap.time ) );
						if( !( dln->flags & DLF_INITIAL ) )
						{
							snprintf( dln->lv_cps, CPS_BUFFER_SIZE, "%.0f cps", speed);
							dln->lv_state_ptr = dln->lv_cps;
							
							if( dln->flags & DLF_KNOWNLENGTH )
							{
								dln->remaining_time	= ( dln->length - dln->act_snap.docptr ) / ( speed ? speed : 1 );
							}

							dln->last_snap.time = dln->act_snap.time;
							dln->last_snap.docptr = dln->act_snap.docptr;
							
							if( dln->flags & DLF_KNOWNLENGTH )
							{
								dln->lv_eta_ptr = dln->lv_eta;
								sprintf( dln->lv_eta, "%ld:%02ld:%02ld",
									( dln->remaining_time > 9999 * 3600 ) ? ( LONG )"XX" : ( dln->remaining_time ? ( dln->remaining_time / 3600 ) : ( LONG )"--" ), dln->remaining_time ? ( ( dln->remaining_time / 60 ) % 60 ) : ( LONG )"--", dln->remaining_time ? ( dln->remaining_time % 60 ) : ( LONG )"--"
								);
							}
						}
						else
						{
							dln->flags &= ~DLF_INITIAL;
						}

						dln->update_count = 0;
						dln->flags |= DLF_SAMPLE;
					}

					docptr = nets_getdocrealptr( dln->ns );

					if( docptr ) /* TOFIX: is that super clean ? hm */
					{
						if( dln->flags & DLF_KNOWNLENGTH )
						{
							snprintf( dln->lv_size, LENGTH_BUFFER_SIZE, "%u of %u (%.0f%%)", docptr, dln->length, ( float )docptr / dln->length * 100 );   /* TOFIX: not locale proof + 2 nets(  ) call ! argh! */
						}
						else
						{
							snprintf( dln->lv_size, LENGTH_BUFFER_SIZE, GS( DOWNLOAD_SIZE_UNKNOWN ), docptr );
						}
						dln->lv_size_ptr = dln->lv_size;
					}

					if( dln->lv_time_ptr == dln->lv_time )
					{
						time_t gone_r = ( timedm() - dln->start_time ) / 1000;
						snprintf( dln->lv_time, TIME_BUFFER_SIZE, "%ld:%02ld:%02ld",
								gone_r / 3600, ( gone_r / 60 ) % 60, gone_r % 60
						);
					}
				}
				else
				{
					/*
					 * Give the user a chance to have display if there's enough
					 * memory by now.
					 */
					D( db_dlwin, bug( "not enough memory.. allocating again\n" ) );
					if( !dln->lv_size )
					{
						if( ( dln->lv_size = malloc( LENGTH_BUFFER_SIZE ) ) )
						{
							dln->lv_size[ 0 ] = '\0';
						}
					}
					if( !dln->lv_time )
					{
						dln->lv_time = malloc( TIME_BUFFER_SIZE );
					}
					if( !dln->lv_eta )
					{
						dln->lv_eta = malloc( ETA_BUFFER_SIZE );
					}
					if( !dln->lv_cps )
					{
						dln->lv_cps = malloc( CPS_BUFFER_SIZE );
					}
					if( !dln->lv_retries )
					{
						if( ( dln->lv_retries = malloc( RETRIES_BUFFER_SIZE ) ) )
						{
							snprintf( dln->lv_retries, RETRIES_BUFFER_SIZE, "%lu", dln->retries );
						}
					}
				}
				DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
				dln->flags &= ~DLF_CHANGED;
			}
			else
			{
				/*
				 * Check for stalled transfers.
				 */
				if( dln->stall > STALLED_TIME )
				{
					if( !( dln->flags & DLF_STALLED ) )
					{
						if ( dln->lv_cps )
						{
							snprintf( dln->lv_cps, CPS_BUFFER_SIZE, "stalled" ); /* TOFIX: locale proof */
						}
						/* XXX: what to do if it's not there yet ? */
						DoMethod( obj, MM_Downloadwin_UpdateEntry, dln );
						dln->flags &= ~DLF_CHANGED;
						dln->flags |= DLF_STALLED;
					}
				}
				else
				{
					dln->stall++;
				}
			}
		}
	}
	
	if( ISLISTEMPTY( &data->dllist ) )
	{
		if( data->ihn.ihn_Object )
		{
			DoMethod( _app( obj ), MUIM_Application_RemInputHandler, &data->ihn );
			data->ihn.ihn_Object = NULL;
		}
	}
	
	return( 0 );
}


/*
 * Aborts a download, remove it from the list.
 */
DECSMETHOD( Downloadwin_Abort )
{
	GETDATA;
	struct dlnode *dln;
	
	DoMethod( data->lv_dl, MUIM_List_GetEntry, msg->entry, &dln );
	DoMethod( obj, MM_Downloadwin_RemoveEntry, dln );

	return( 0 );
}


/*
 * Changes the state of an unode.. update the required
 * stuff accordingly.
 */
DECSMETHOD( Downloadwin_SetState )
{
	GETDATA;

	msg->dl->state = msg->state;
	D( db_dlwin, bug( "setting state of 0x%lx to %ld\n", msg->dl, msg->state ) );
	
	switch( msg->state )
	{
		case DLS_USERWAIT:
			msg->dl->lv_state_ptr = GS( DOWNLOAD_STATE_USERFEEDBACK );
			break;

		case DLS_DOWNLOADING:
			//msg->dl->lv_state_ptr = "fetching";
			break;

		case DLS_DONE:
			msg->dl->lv_eta_ptr = "-:--:--";
			break;

		case DLS_RESUMING:
			msg->dl->lv_state_ptr = GS( DOWNLOAD_STATE_RESUMING );
			break;

		case DLS_OVERWRITING:
			msg->dl->lv_state_ptr = GS( DOWNLOAD_STATE_OVERWRITING );
			break;

		case DLS_WAITING:
			msg->dl->lv_state_ptr = GS( DOWNLOAD_STATE_QUEUED );
			msg->dl->lv_size_ptr = "";
			msg->dl->lv_time_ptr = "-:--:--";
			msg->dl->lv_eta_ptr = "-:--:--";
			break;
		
		case DLS_FAILED:
			msg->dl->lv_state_ptr = GS( DOWNLOAD_STATE_1 );
			msg->dl->lv_size_ptr = "";
			break;
	}
	
	DoMethod( obj, MM_Downloadwin_SetButtons, getv( data->lv_dl, MUIA_List_Active ) );

	return( 0 );
}


/*
 * Removes the entries to be cleaned up.
 * TOFIX: only removes DLS_DONE for now.. DLS_FAILED needed ?
 */
DECTMETHOD( Downloadwin_Cleanup )
{
	GETDATA;
	ULONG c, m;
	struct dlnode *dln;

	set( data->lv_dl, MUIA_List_Quiet, TRUE );

	m = getv( data->lv_dl, MUIA_List_Entries );

	for( c = 0; c < m ; c++ )
	{
		DoMethod( data->lv_dl, MUIM_List_GetEntry, c, &dln );
		if( dln->state == DLS_DONE )
		{
			DoMethod( obj, MM_Downloadwin_RemoveEntry, dln );
		}
	}
	
	set( data->lv_dl, MUIA_List_Quiet, FALSE );

	return( 0 );
}


/*
 * Allocates the needed fields to prepare for the
 * later smartreq() call.
 */
DECSMETHOD( Downloadwin_InitReq )
{
	DoMethod( obj, MM_Downloadwin_FreeResume, msg->dl );

	if( ( msg->dl->resume_url = malloc( strlen( msg->dl->ns->url ) + 1 ) ) )
	{
		if( !msg->dl->ns->referer || ( msg->dl->ns->referer && ( msg->dl->resume_referer = malloc( strlen( msg->dl->ns->referer ) + 1 ) ) ) )
		{
			strcpy( msg->dl->resume_url, msg->dl->ns->url );
			if( msg->dl->ns->referer )
			{
				strcpy( msg->dl->resume_referer, msg->dl->ns->referer );
			}
			return( TRUE );
		}
	}
	return( FALSE );
}


/*
 * To use after smartreq() has been called.
 */
DECSMETHOD( Downloadwin_ExitReq )
{
	nets_close( msg->dl->ns );
	msg->dl->ns = NULL;

	return ( 0 );
}


/*
 * Frees the resume URL stuff.
 */
DECSMETHOD( Downloadwin_FreeResume )
{
	free( msg->dl->resume_url );
	msg->dl->resume_url = NULL;
	free( msg->dl->resume_referer );
	msg->dl->resume_referer = NULL;
	return( 0 );
}


/*
 * When we get back the requester from the user,
 * we scan the list and fire do what he told us to
 * do with the entry.
 */
DECSMETHOD( SmartReq_Pressed )
{
	GETDATA;
	struct dlnode *dln;
	LONG r;

	D( db_dlwin, bug( "got pressed method\n" ) );

	ITERATELIST( dln, &data->dllist )
	{
		if( dln->id == msg->userdata )
		{
			D( db_dlwin, bug( "found entry 0x%lx\n", dln ) );

			r = msg->butnum;

			if( dln->flags & DLF_CANRESUME )
			{
				dln->flags &= ~DLF_CANRESUME;
			}
			else
			{
				if ( r > 0 )
				{
					r++;
				}
			}

			switch( r )
			{
				case BT_CANCEL: /* remove the entry */
					DoMethod( obj, MM_Downloadwin_RemoveEntry, dln );
					break;

				case BT_RESUME:
					DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_RESUMING );
					if( ( dln->ns = nets_open( dln->resume_url, dln->resume_referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
					{
						DoMethod( obj, MM_Downloadwin_FreeResume, dln );
						if( !getv( obj, MUIA_Window_Open ) )
						{
							set( obj, MUIA_Window_Open, TRUE ); /* TOFIX: could be cleaner.. hm.. */
						}
					}
					break;

				case BT_RENAME:
					if( DoMethod( obj, MM_Downloadwin_SelectFile, dln, FilePart( dln->filename_ptr ) ) )
					{
						DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_WAITING );
						if( ( dln->ns = nets_open( dln->resume_url, dln->resume_referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
						{
							DoMethod( obj, MM_Downloadwin_FreeResume, dln );
							if( !getv( obj, MUIA_Window_Open ) )
							{
								set( obj, MUIA_Window_Open, TRUE ); /* TOFIX: could be cleaner.. hm .. */
							}
						}// TOFIX: displaybeep() here I think.. but how do we handle the rest ?
					}
					break;

				case BT_OVERWRITE:
					DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_OVERWRITING );
					if( ( dln->ns = nets_open( dln->resume_url, dln->resume_referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
					{
						DoMethod( obj, MM_Downloadwin_FreeResume, dln );
						if( !getv( obj, MUIA_Window_Open ) )
						{
							set( obj, MUIA_Window_Open, TRUE ); /* TOFIX: could be cleaner.. hm.. */
						}
					}
					break;
			}
			break;
		}
	}

	return( 0 );
}


/*
 * Ask what to do depending on the mimetype.
 * Well, this is after the user answered.
 */
DECSMETHOD( SmartReq_Ask )
{
	GETDATA;
	struct dlnode *dln;

	D( db_dlwin, bug( "got pressed method\n" ) );

	ITERATELIST( dln, &data->dllist )
	{
		if( dln->id == msg->userdata )
		{
			D( db_dlwin, bug( "found entry 0x%lx\n", dln ) );

			dln->flags |= DLF_ANSWERED;

			switch( msg->butnum )
			{
				case BT_CANCEL: /* remove the entry */
					DoMethod( obj, MM_Downloadwin_RemoveEntry, dln );
					break;

				case BT_VIEW:
					D( db_dlwin, bug( "viewing..\n" ) );
					if( DoMethod( obj, MM_Downloadwin_SelectViewer, dln ) )
					{
						char buf[ 32 ];

						sprintf( buf, "VViewTemp.%02d", ( int )++data->tmpcount % 100 ); /* TOFIX: not very smart :) */

						if( DoMethod( obj, MM_Downloadwin_AddFile, dln, gp_cachedir, buf ) )
						{
							dln->flags |= DLF_VIEW;
							dln->flags |= DLF_DELETE_AFTER_USE;
					
							DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_WAITING );
							if( ( dln->ns = nets_open( dln->resume_url, dln->resume_referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
							{
								DoMethod( obj, MM_Downloadwin_FreeResume, dln );

								if( !getv( obj, MUIA_Window_Open ) )
								{
									set( obj, MUIA_Window_Open, TRUE );
								}
							}
						}
					}
					break;

				case BT_SAVE_VIEW:
					if( DoMethod( obj, MM_Downloadwin_SelectViewer, dln ) )
					{
						dln->flags |= DLF_VIEW;
					}
					else
					{
						break;
					}
					// fallthrough

				case BT_SAVE:
					D( db_dlwin, bug( "saving..\n" ) );
					DoMethod( obj, MM_Downloadwin_SetState, dln, DLS_WAITING );
					if( ( dln->ns = nets_open( dln->resume_url, dln->resume_referer, obj, NULL, NULL, gp_download_timeout, NOF_TIMESTAMP | NOF_PROGRESS ) ) )
					{
						DoMethod( obj, MM_Downloadwin_FreeResume, dln );

						if( !getv( obj, MUIA_Window_Open ) )
						{
							set( obj, MUIA_Window_Open, TRUE );
						}
					}
					break;

			}
			break;
		}
	}
	return( 0 );
}


DECMMETHOD( Cleanup )
{
	GETDATA;

	if( data->ihn.ihn_Object )
	{
		DoMethod( _app( obj ), MUIM_Application_RemInputHandler, &data->ihn );
		data->ihn.ihn_Object = NULL;
	}
	return( DOSUPER );
}


DECDISPOSE
{
	GETDATA;
	char buf[ 256 ];

	/*
	 * Remove the temporary view files (TOFIX: sucky..)
	 */
	while( data->tmpcount > 0 )
	{
		strcpy( buf, gp_cachedir );
		AddPart( buf, "VViewTemp.", sizeof( buf ) );
		sprintf( strchr( buf, 0 ), "%02ld", ( long int )data->tmpcount-- );
		DeleteFile( buf );
	}

	if( data->win_req )
	{
		DoMethod( app, OM_REMMEMBER, data->win_req );
		MUI_DisposeObject( data->win_req );
	}
	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFGET
DEFDISPOSE
DEFSMETHOD( Downloadwin_Enqueue )
DEFSMETHOD( Downloadwin_SetButtons )
DEFTMETHOD( Downloadwin_Refresh )
DEFTMETHOD( Downloadwin_Cleanup )
DEFSMETHOD( Downloadwin_Abort )
DEFSMETHOD( Downloadwin_UpdateEntry )
DEFSMETHOD( Downloadwin_SetState )
DEFSMETHOD( Downloadwin_RemoveEntry )
DEFSMETHOD( Downloadwin_FindEntry )
DEFSMETHOD( Downloadwin_InitReq )
DEFSMETHOD( Downloadwin_ExitReq )
DEFSMETHOD( Downloadwin_FreeResume )
DEFSMETHOD( Downloadwin_AddFile )
DEFSMETHOD( Downloadwin_SelectFile )
DEFSMETHOD( Downloadwin_AddViewer )
DEFSMETHOD( Downloadwin_SelectViewer )
DEFSMETHOD( Downloadwin_View )
DEFSMETHOD( SmartReq_Pressed )
DEFSMETHOD( SmartReq_Ask )
DEFSMETHOD( NStream_GotInfo )
DEFSMETHOD( NStream_GotData )
DEFSMETHOD( NStream_Done )
DEFMMETHOD( Cleanup )
ENDMTABLE


static struct MUI_CustomClass *mcc;

int create_downloadwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "DownloadWinClass";
#endif

	return( TRUE );
}

void delete_downloadwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getdownloadwin( void )
{
	return( mcc->mcc_Class );
}

int create_downloadwin( void )
{
	/*
	 * Try to create/open the download window.
	 */
	if( !win_dl )
	{
		if( ( win_dl = NewObject( getdownloadwin(), NULL, TAG_DONE ) ) )
		{
			DoMethod( app, OM_ADDMEMBER, win_dl );
			return( TRUE );
		}
		else
		{
			displaybeep();
			return( FALSE );
		}
	}
	return( TRUE );
}

/*
 * Creates and enqueus a download node.
 */
void queue_download( STRPTR url, STRPTR referer, int dlonly, int askpath )
{
#if !USE_DOS
	D( db_dlwin, bug( "no dos, no downloading..\n" ) );
	return;
#endif /* !USE_DOS */

	D( db_dlwin, bug( "adding url %s\n", url ) );

	if( create_downloadwin() )
	{
		ULONG f = 0;

		if( dlonly )
			f |= DLF_DOWNLOAD;

		if( askpath )
			f |= DLF_ASKPATH;

		f |= DLF_RESUME; /* attempt to resume by default */

		if( !DoMethod( win_dl, MM_Downloadwin_Enqueue, url, referer, f ) )
		{
			displaybeep();
		}
	}
}

/*
 * Opens the download window
 */
int open_downloadwin( void )
{
	if( create_downloadwin() )
	{
		set( win_dl, MUIA_Window_Open, TRUE );
		return( (int)getv( win_dl, MUIA_Window_Open ) );
	}
	else
	{
		return( FALSE );
	}
}

#endif /* USE_NET */
