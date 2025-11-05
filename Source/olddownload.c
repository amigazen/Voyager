/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


/*
**
** $Id: download.c,v 1.102 2001/08/07 22:13:31 zapek Exp $
**
*/

#include "voyager.h"

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

/*
 * States
 */
#define DLS_START		0
#define DLS_READING		1
#define DLS_DONE		-1
#define DLS_FAILED		2

/*
 * Actions
 */
enum {
	DLA_NONE,     /* do not refresh display */
	DLA_REFRESH,  /* refresh display */
	DLA_REMOVE,   /* remove the entry */
	DLA_RESUME	  /* try to resume the entry */
};

APTR win_dl;

static APTR lv_dl;
static APTR dlwinptr;
static APTR str_dldir;

static struct MinList dllist;

void init_downloadlist( void )
{
	NEWLIST( &dllist );
}

/*
 * dllist holds dlnodes which are
 * scanned everytime an event arrives.
 */
struct dlnode {
	struct MinNode n;
	char path[ 256 ];            /* file path */
	char viewapp[ 512 ];
	char urlshort[ 70 ];         /* URL used for displaying in the listview */
	struct nstream *ns;          /* always valid when the entry is in the list */
	time_t start;                /* time in seconds of the start of the transfer */
	int state;                   /* state (see the DLS_ flags above) */
	int dlonly;                  /* download mode only (no view app mode) */
	int finalcps;                /* final CPS calculation */
	int pathoffset;              /* offset for displaying the last part of the path */
	int action;                  /* action to do for the next refresh (see the DLA_ flags above) */
	int askpath;                 /* TRUE: request the path */
	int cached;                  /* TRUE: the file is in the mem/disk cache */
	int doclen;                  /* total size of the file, might be unknown (-1) */
	int offset;                  /* offset from where to resume to (constant) */
	int lastgot;                 /* last offset in the file during the last refresh */
	int askresume;               /* TRUE: ask for resuming a file, FALSE: attempt to resume silently */
	int retries;                 /* number of network retries */
};

struct Data {
	APTR bt_retry;
};

/* DisplayHook */
MUI_HOOK( dl_disp, STRPTR *array, struct dlnode *dl )
{
	static char sizebf[ 64 ], cpsbf[ 16 ], viewbf[ 128 ], retrybf[ 8 ], times[ 36 ];
	
	if( dl )
	{
		/*
		 * Path
		 */
		if( dl->state == DLS_START )
		{
			*array++ = "";
		}
		else if( dl->dlonly )
		{
			*array++ = &dl->path[ dl->pathoffset ];
		}
		else
		{
			char *p;

			stccpy( viewbf, FilePart( dl->viewapp ), sizeof( viewbf ) - 64 );
			p = strchr( viewbf, ' ' );
			if( p )
				*p = 0;
			strins( viewbf, "\033bView\033n with " );
			*array++ = viewbf;
		}

		/*
		 * Size
		 */
		if( dl->doclen > 0 )
			sprintf( sizebf, "%u of %u (%.0f%%)", dl->lastgot, dl->doclen, (float) dl->lastgot / dl->doclen * 100 );
		else
			sprintf( sizebf, GS(DOWNLOAD_SIZE_UNKNOWN), dl->lastgot );

		*array++ = sizebf;

		/*
		 * State
		 */
		switch( dl->state )
		{
			case DLS_START: /* not started yet */
				*array++ = GS( DOWNLOAD_STATE_0 );
				*array++ = "";
				break;

			case DLS_READING:
				if( dl->cached )
				{
					*array++ = GS( DOWNLOAD_STATE_CACHED );
					*array++ = "";
				}
				else
				{
					time_t gone = timed() - dl->start;
					sprintf( cpsbf, "%lu cps", ( dl->lastgot - dl->offset ) / ( gone > 0 ? gone : 1 ) );
					*array++ = cpsbf;
					if( gone > 9999 * 3600 )
					{
						*array++ = "overflow";
					}
					else
					{
	                    if( dl->doclen > 0 )
						{
							int lastgot = ( dl->lastgot - dl->offset ) / 1024;
							time_t rt = (gone * ( ( dl->doclen - dl->offset ) / 1024 ) ) / lastgot ? lastgot : 1;
							sprintf( times, "%ld:%02ld:%02ld / %ld:%02ld:%02ld",
								gone / 3600, ( gone / 60 ) % 60, gone % 60,
								( rt > 9999 * 3600 ) ? ( LONG )"XX" : ( rt ? ( rt / 3600 ) : ( LONG )"--" ), rt ? ( ( rt / 60 ) % 60 ) : ( LONG )"--", rt ? ( rt % 60 ) : ( LONG )"--"
							);
						}
						else
						{
							sprintf( times, "%ld:%02ld:%02ld",
								gone / 3600, ( gone / 60 ) % 60, gone % 60
							);
						}
						*array++ = times;
					}
				}
				break;

			case DLS_DONE:  /* finished */
				if( dl->finalcps )
				{
					if( dl->doclen > 0 )
					{
						sprintf( sizebf, "%u of %u (100%%)", dl->doclen, dl->doclen );
					}
					else
					{
						sprintf( sizebf, "%u (100%%)", dl->lastgot );
					}

					if ( dl->cached )
					{
						strcpy( cpsbf, "\033bdone" );
					}
					else
					{
						sprintf( cpsbf, "\033bdone; %u cps", dl->finalcps );
					}
					*array++ = cpsbf;
					*array++ = ""; /* TOFIX :) */
					break;
				}
				/* TOFIX: why the assumption about the finalcps thing ? */
				//nets_close( dl->ns );
				//dl->ns = NULL;
				// if failed, fallthrough (TOFIX: in which case a download can be DLS_DONE -> DLS_FAILED ?)

			case DLS_FAILED: /* failed */
				*array++ = "\0333failed";
				*array++ = ""; /* TOFIX :) */
				
				break;
		}

		/*
		 * Number of retries
		 */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
		stci_d( retrybf, dl->retries );
#else
		sprintf( retrybf, "%u", dl->retries );
#endif
		*array++ = retrybf;

		/*
		 * URL
		 */
		*array = dl->urlshort;
	}
	else
	{
		/* header */
		*array++ = GS( DOWNLOAD_LPATH ) + ( nlist ? 2 : 0 );
		*array++ = GS( DOWNLOAD_LSIZE ) + ( nlist ? 2: 0 );
		*array++ = GS( DOWNLOAD_LSTATE ) + ( nlist ? 2 : 0 );
		*array++ = GS( DOWNLOAD_LTIMES ) + ( nlist ? 2 : 0 );
		*array++ = GS( DOWNLOAD_LRETRIES ) + ( nlist ? 2 : 0 );
		*array = GS( DOWNLOAD_LURL ) + ( nlist ? 2 : 0 );
	}
	return( 0 );
}


/* ConstructHook */
MUI_HOOK( dl_const, APTR pool, struct dlnode *dl )
{
	struct dlnode *dln;

	dln = AllocPooled( pool, sizeof( *dln ) );
	if( dln )
	{
		*dln = *dl;
		ADDTAIL( &dllist, dln );
	}

	return( (LONG)dln );
}


/* DestructHook */
MUI_HOOK( dl_dest, APTR pool, struct dlnode *dl )
{
	REMOVE( dl );
	if( dl->ns )
		nets_close( dl->ns );
	FreePooled( pool, dl, sizeof( *dl ) );
	return( 0 );
}


/*
 * Returns the download path. If the download window exists and
 * contains a reasonable value, take it from there, otherwise
 * return the global preference one.
 * That way the user can easily set the path temporary.
 */
STRPTR get_download_path( void )
{
	if( win_dl )
	{
		STRPTR p = ( STRPTR )getv( str_dldir, MUIA_String_Contents );

		if( *p )
		{
			return( ( STRPTR )getv( str_dldir, MUIA_String_Contents ) );
		}
	}
	
	return( getprefsstr( DSI_SAVEDEST, "RAM:" ) );

}


DECNEW
{
	struct Data *data;
	APTR bt_cleanup, bt_abort, bt_retry;

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID( 'D','L','W','N' ),
		MUIA_Window_Title, GS( DOWNLOAD_TITLE ),
		MUIA_Window_Activate, FALSE,
		MUIA_Window_RootObject, VGroup,
#if USE_DOS
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
#endif
			Child, lv_dl = MUI_NewObject( listviewclass, MUIA_CycleChain, 1,
				MUIA_Listview_List, MUI_NewObject( listclass, InputListFrame,
					MUIA_Font, MUIV_Font_Tiny,
					MUIA_List_Title, TRUE,
					MUIA_List_Format, "BAR,BAR,BAR,BAR,BAR,MIW=5",
					MUIA_List_DisplayHook, &dl_disp_hook,
					MUIA_List_ConstructHook, &dl_const_hook,
					MUIA_List_DestructHook, &dl_dest_hook,
				End,
			End,

			Child, HGroup,
				Child, bt_abort = makebutton( MSG_DOWNLOAD_BT_ABORT ),
				Child, bt_retry = makebutton( MSG_DOWNLOAD_BT_RETRY ),
				Child, bt_cleanup = makebutton( MSG_DOWNLOAD_BT_CLEANUP ),
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	data->bt_retry = bt_retry;
	
	/* Download string */
	set( str_dldir, MUIA_String_Contents, getprefsstr( DSI_SAVEDEST, "RAM:" ) );

	/* Abort button TOFIX: should work like CheckEntry.. */
	DoMethod( lv_dl, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		bt_abort, 3, MUIM_Set, MUIA_Disabled, FALSE
	);
	DoMethod( lv_dl, MUIM_Notify, MUIA_List_Active, MUIV_List_Active_Off,
		bt_abort, 3, MUIM_Set, MUIA_Disabled, TRUE
	);
	
	/* Retry button */
	DoMethod( lv_dl, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		obj, 2, MM_Downloadwin_CheckEntry, MUIV_TriggerValue
	);
	DoMethod( bt_retry, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Downloadwin_Retry
	);

	/* Cleanup button */
	DoMethod( lv_dl, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
		lv_dl, 2, MUIM_List_Remove, MUIV_List_Remove_Active
	);
	DoMethod( bt_abort, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_dl, 2, MUIM_List_Remove, MUIV_List_Remove_Active
	);
	DoMethod( bt_cleanup, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Downloadwin_Cleanup
	);

	/* Close gadget */
	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 6, MUIM_Application_PushMethod, ( ULONG )obj, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);

	set( bt_abort, MUIA_Disabled, TRUE );
	set( bt_retry, MUIA_Disabled, TRUE );

	dlwinptr = obj;

	return( (ULONG)obj );
}

int download_check( void );

DECMETHOD( NStream_GotInfo, APTR )
{
	download_check();
	return( 0 );
}


DECMETHOD( NStream_GotData, APTR )
{
	download_check();
	return( 0 );
}


DECMETHOD( NStream_Done, APTR )
{
	if( download_check() )
	{
		DoMethod( app, MM_App_CheckWinRemove ); /* we can quit if the user closed everything */
	}
	return( 0 );
}


DECMETHOD( Downloadwin_Cleanup, ULONG )
{
	int c;
	struct dlnode *dln;

	set( lv_dl, MUIA_List_Quiet, TRUE);

	for( c = 0; ; )
	{
		DoMethod( lv_dl, MUIM_List_GetEntry, c, &dln );
		if( !dln )
			break;
		if( dln->state >= 0 )
		{
			c++;
			continue;
		}
		DoMethod( lv_dl, MUIM_List_Remove, c );
	}

	set( lv_dl, MUIA_List_Quiet, FALSE);

	return( 0 );
}


/*
 * Makes the "Retry" button selectable if the entry is in a failed state
 */
DECSMETHOD( Downloadwin_CheckEntry )
{
	struct dlnode *dln;
	GETDATA;

	if( msg->entry != MUIV_List_Active_Off )
	{
		DoMethod( lv_dl, MUIM_List_GetEntry, msg->entry, &dln );
		
		if( msg->entry == getv( lv_dl, MUIA_List_Active ) )
		{
			if( dln && dln->state == DLS_FAILED )
			{
				set( data->bt_retry, MUIA_Disabled, FALSE );
				return( 0 );
			}
			else
			{
				set( data->bt_retry, MUIA_Disabled, TRUE );	
			}
		}
	}
	return( 0 );
}


/*
 * Launches a retry request
 */
DECSMETHOD( Downloadwin_Retry )
{
	struct dlnode *dln;
	struct nstream *ns;

	DoMethod( lv_dl, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &dln );
	ns = dln->ns;
	dln->state = DLS_START;
	dln->action = DLA_RESUME;
	dln->ns = nets_open( nets_url( dln->ns ), nets_referer( dln->ns ), FALSE, dlwinptr, NULL, NULL, FALSE, gp_download_timeout );
	nets_close( ns );

	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFMETHOD( NStream_GotInfo )
DEFMETHOD( NStream_GotData )
DEFMETHOD( NStream_Done )
DEFMETHOD( Downloadwin_Cleanup )
DEFSMETHOD( Downloadwin_Retry )
DEFSMETHOD( Downloadwin_CheckEntry )
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
 * Create a download node
 */
void createdlwindow( char *url, int dlonly, char *referer, int ask )
{
	struct dlnode dln;
	char tmp[ 2048 ], tmp2[ 32 ], *p = 0;
	struct parsedurl purl;

#if !USE_DOS
	D( db_downloadwin, bug( "no dos available, no downloading..\n" ) );
	return;
#endif

	D( db_downloadwin, bug( "adding dlnode %s\n", url ));

	memset( &dln, 0, sizeof( dln ) );
	dln.dlonly = dlonly;

	/*
	 * Build the complete filename path
	 */
	strcpy( dln.path, get_download_path() );
	stccpy( tmp, url, sizeof( tmp ) );
	uri_split( tmp, &purl );
	if( purl.path )
	{
		p = FilePart( purl.path );
	}
	if( p && *p )
	{
		stccpy( tmp2, p, sizeof( tmp2 ) );
	}
	else
		strcpy( tmp2, "index.html" );

	if( ask )
		dln.askpath = TRUE;

	dln.askresume = TRUE; /* first resume attempt must be confirmed by the user (TOFIX: unless it comes from the automatic resume logfile feature) */

	dln.retries = gp_download_retries;

	dln.pathoffset = strlen( dln.path );
	AddPart( dln.path, tmp2, sizeof( dln.path ) );

	stccpy( dln.urlshort, url, sizeof( dln.urlshort ) );
	if( strlen( dln.urlshort ) >= sizeof( dln.urlshort ) - 4 )
		strcpy( &dln.urlshort[ sizeof( dln.urlshort ) - 4 ], "..." );

	if( create_downloadwin() )
	{
		dln.ns = nets_open( url, referer, FALSE, dlwinptr, NULL, NULL, FALSE, gp_download_timeout );
		if( !dln.ns )
		{
			displaybeep();
			return;
		}
	
		DoMethod( lv_dl, MUIM_List_InsertSingle, &dln, MUIV_List_Insert_Bottom );
	
		set( lv_dl, MUIA_List_Active, MUIV_List_Active_Bottom );

		if (!getv( win_dl, MUIA_Window_Open ))
		{
			set( win_dl, MUIA_Window_Open, TRUE );
		}

		download_check();
	}
}


static void runmv( char *filename, char *app )
{
#if USE_DOS

	char cmd[ 512 ], fullpath[ 128 ];
	BPTR lock;
	struct Screen *scr;
	struct List *psl;
	struct PubScreenNode *psn;

	//Printf( "runmv %s %s\n", filename, app );

	lock = Lock( filename, SHARED_LOCK );
	if( !lock )
	{
		//PrintFault( IoErr(), "can't open" );
		return;
	}
	NameFromLock( lock, fullpath, 256 );
	UnLock( lock );

	get( win_dl, MUIA_Window_Screen, &scr );

	psl = LockPubScreenList();

	for( psn = FIRSTNODE( psl ); NEXTNODE( psn ); psn = NEXTNODE( psn ) )
		if( psn->psn_Screen == scr )
			break;

	if( !NEXTNODE( psn ) )
		psn = NULL;

	UnlockPubScreenList();

	if( !strchr( app, '%' ) )
	{
		sprintf( cmd, "%s %s", app, fullpath );
	}
	else
	{
		expandtemplate( app, cmd, sizeof( cmd ),
			'f', fullpath,
			'p', psn ? psn->psn_Node.ln_Name : "",
			NULL
		);
	}

	//Printf( "final command %s\n", cmd );

	SystemTags( cmd,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);
#endif
}


static int tempcount;
static void maketemppath( struct dlnode *dl )
{
	char buff[ 32 ];

	strcpy( dl->path, gp_cachedir );

	sprintf( buff, "VViewTemp.%02d", tempcount++ % 100 );
	AddPart( dl->path, buff, sizeof( dl->path ) );
}

/* This needs to go into disposing of the object */
void cleanup_viewtemp( void )
{
#if USE_DOS
	char buff[ 256 ];

	while( tempcount >= 0 )
	{
		D( db_init, bug( "cleaning up temporary files (remaining == %ld)..\n", tempcount ) );
		strcpy( buff, gp_cachedir );
		AddPart( buff, "VViewTemp.", sizeof( buff ) );
		sprintf( strchr( buff, 0 ), "%02ld", ( long int )tempcount-- );
		DeleteFile( buff );
	}
#endif
}

static char last_save_dldir[ 256 ];

static int askdldir( struct dlnode *dl )
{
	int rc = FALSE;
#if USE_DOS
	struct Screen *scr = NULL;
	struct FileRequester *fr;
	char tmp[ 256 ];

	get( win_dl, MUIA_Window_Screen, &scr );

	if( last_save_dldir[ 0 ] )
	{
		strcpy( tmp, last_save_dldir );
	}
	else
	{
		strcpy( tmp, dl->path );
	}
	*PathPart( tmp ) = 0;

	set( app, MUIA_Application_Sleep, TRUE );

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_Screen, scr,
		ASLFR_TitleText, GS( PREFSWIN_MIME_L_DLDIR ),
		ASLFR_DoSaveMode, TRUE,
		ASLFR_RejectIcons, FALSE,
		ASLFR_InitialDrawer, tmp,
		ASLFR_InitialFile, FilePart( dl->path ),
		TAG_DONE
	);

	if( fr )
	{
		if( MUI_AslRequestTags( fr, TAG_DONE ) )
		{
			strcpy( dl->path, fr->fr_Drawer );
			if( stricmp( dl->path, get_download_path() ) )
				dl->pathoffset = 0;
			AddPart( dl->path, fr->fr_File, sizeof( dl->path ) );
			stccpy( last_save_dldir, dl->path, 256 );
			rc = TRUE;
		}
		MUI_FreeAslRequest( fr );
	}

	set( app, MUIA_Application_Sleep, FALSE );

#endif
	return( rc );
}


static int askviewer( struct dlnode *dl )
{
	int rc = FALSE;
#if USE_DOS
	struct Screen *scr = NULL;
	struct FileRequester *fr;

	get( win_dl, MUIA_Window_Screen, &scr );

	set( app, MUIA_Application_Sleep, TRUE );

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_Screen, scr,
		ASLFR_TitleText, GS( PREFSWIN_MIME_E_APP ),
		ASLFR_RejectIcons, FALSE,
		ASLFR_InitialDrawer, "SYS:Utilities",
		ASLFR_InitialFile, "Multiview",
		TAG_DONE
	);

	if( fr )
	{
		if( MUI_AslRequestTags( fr, TAG_DONE ) )
		{
			strcpy( dl->viewapp, fr->fr_Drawer );
			AddPart( dl->viewapp, fr->fr_File, sizeof( dl->viewapp ) );
			rc = TRUE;
		}
		MUI_FreeAslRequest( fr );
	}

	set( app, MUIA_Application_Sleep, FALSE );
#endif
	return( rc );
}


int download_check( void )
{
	struct dlnode *dl, *dln;
	static int refresh = FALSE;
	static time_t lastrefresh;
	int alldone = TRUE;

	for( dl = FIRSTNODE( &dllist ); dln = NEXTNODE( dl ); dl = dln )
	{
		//Printf( "node %s state %ld mime %s path %s\n", dl->urlshort, dl->state, dl->ns ? nets_mimetype( dl->ns ) : "??", dl->path );
		switch( dl->state )
		{
			case DLS_START: /* not started yet */
				if( dl->ns && nets_mimetype( dl->ns )[ 0 ] )
				{
					char viewer[ 256 ], savedir[ 256 ];
					int viewmode = 0;
					BPTR l;
					int cancel = TRUE;
					char serverfilesize[ 12 ];
					int resume = FALSE;
					int offset = 0;
#if USE_DOS
					D_S(struct FileInfoBlock, fib);
#endif

					strcpy( savedir, get_download_path() );
					mime_findbytype( nets_mimetype( dl->ns ), savedir, viewer, &viewmode );

					//Printf( "mimetype %s, savedir = %s, viewer = %s, viewmode = %ld\n", nets_mimetype( dl->ns ), savedir, viewer, viewmode );

					// either failed, or MIME type available
					switch( dl->dlonly ? 1 : ( viewmode & 7 ) )
					{
						default: /* this catches case '1' */
							dodownload:
							
							if( dl->askpath )
							{
								if( !askdldir( dl ) )
								{
									/*
									 * User pressed ASL's cancel. Removing entry..
									 */
									dl->action = DLA_REMOVE;
									refresh++;
									break;
								}
							}
							
#if USE_DOS
							if( l = Lock( dl->path, ACCESS_READ ) ) /* check if the file exists */
							{
								if( Examine(l, fib) == DOSTRUE )
								{
									char s_s[ 32 ], l_s[ 32 ];
									APTR gadgets = NULL;
									LONG req;
									int len;

									/*
									 * Currently only files with a known final size which is bigger
									 * then the one of the local file size are resumed.
									 * This could be improved.
									 */
									if( ( ( len = nets_getdoclen( dl->ns ) ) != -1 ) )
									{
										D( db_downloadwin, bug( "server reported size %ld\n", len ) );
										stcul_d( serverfilesize, len );
										if( len > fib->fib_Size )
										{
											resume = TRUE;
										}
									}
									else
									{
										*serverfilesize = '\0';
									}
									strcpy( l_s, datestamp2string( &fib->fib_Date ) );

									if( dl->ns->un->lastmodified )
									{
										strcpy( s_s, date2string( dl->ns->un->lastmodified ) );
									}

									if( dl->askresume )
									{
										/*
										 * Ask the user what he wants to do with the file.
										 * He can also check local/remote size and date and
										 * decide accordingly if he has enough brain.
										 */
										if( gadgets = myalloc( strlen( GS( DOWNLOAD_RESUME_GADGETS ) ) + ( resume ? strlen( GS( DOWNLOAD_RESUME_RESUMEGAD ) ) : 0 ) + 5 ) )
										{
											strcpy( gadgets, "*\033b" );

											if( resume ) /* add a resume button */
											{
												strcat( gadgets, GS( DOWNLOAD_RESUME_RESUMEGAD ) );
												strcat( gadgets, "|" );
											}
											strcat( gadgets, GS( DOWNLOAD_RESUME_GADGETS ) );
											req = MUI_Request( app,
													win_dl,
													0,
													GS( DOWNLOAD_RESUME_TITLE ),
													gadgets,
													GS( DOWNLOAD_RESUME_BODY ),
													FilePart( dl->path ),
													( *serverfilesize && dl->ns->un->lastmodified ) ? ( dl->ns->un->lastmodified < __datecvt( &fib->fib_Date ) ? GS( DOWNLOAD_RESUME_NEWER ) : GS( DOWNLOAD_RESUME_OLDER ) ) : "",
													*serverfilesize ? serverfilesize : GS( DOWNLOAD_RESUME_UNKNOWN ),
													dl->ns->un->lastmodified ? s_s : GS( DOWNLOAD_RESUME_UNKNOWN ),
													fib->fib_Size,
													&l_s );
											if( !resume && req > 0 )
											{
												req++; /* skip the resume button */
											}

											switch( req )
											{
												case 0: /* cancel */
													dl->action = DLA_REMOVE;
													refresh++;
													break;

												case 1: /* resume */
													offset = fib->fib_Size;
													D( db_downloadwin, bug("pressed resume so offset is: %lu\n", offset));
													dl->askresume = FALSE; /* next one will be automatic */
													cancel = FALSE;
													break;

												case 2: /* rename file */
													if( !askdldir( dl ) )
													{
														dl->action = DLA_REMOVE; /* TOFIX: maybe something smarter.. */
														refresh++;
														break;
													}
													cancel = FALSE;
													break;

												case 3: /* overwrite */
													cancel = FALSE;
													break;

												default: /* failure */
													displaybeep();
													dl->action = DLA_REMOVE;
													refresh++;
													break;
											}
										
											myfree( gadgets );

										}
										else
										{
											cancel = TRUE;
										}
									}
									else
									{
										if( resume )
										{
											offset = fib->fib_Size;
											D( db_downloadwin, bug( "automatic resume in progress at offset %lu\n", offset ) );
										}
										cancel = FALSE;
									}
								}

								UnLock( l );
							}
							else
#endif
							{
								cancel = FALSE;
							}

							if ( cancel)
							{
								break;
							}

							D( db_downloadwin, bug("resume is %lu\n", resume));
							dl->start = timed();
							dl->state = DLS_READING;
							dl->dlonly = TRUE;
							nets_settofile( dl->ns, dl->path, offset );
							dl->cached = nets_sourceid( dl->ns ) == 1;
							dl->doclen = nets_getdoclen( dl->ns );
							dl->offset = dl->ns->un->offset;
							dl->lastgot = dl->offset;
							dl->action = DLA_REFRESH;
							refresh++;
							break;

						case 0: /* ask user if he wants to download the file or select a viewer */
							doask:
							switch( MUI_Request( app, win_dl, 0, GS( DOWNLOAD_ASKUSER_TITLE ), GS( DOWNLOAD_ASKUSER_GADGETS ), GS( DOWNLOAD_ASKUSER_BODY ), nets_mimetype( dl->ns ), dl->urlshort ) )
							{
								case 0: // cancel
									dl->action = DLA_REMOVE;
									refresh++;
									break;

								case 1: // Download
									goto dodownload;

								case 2: // select viewer
									if( !askviewer( dl ) )
										goto doask;
									else
										goto doview;
									break;

							}
							break;

						case 2: // View with app
						case 3:
							strcpy( dl->viewapp, viewer );
							doview:
							maketemppath( dl );
							dl->start = timed();
							nets_settofile( dl->ns, dl->path, FALSE );
							dl->offset = dl->ns->un->offset;
							dl->state = DLS_READING;
							dl->action = DLA_REFRESH;
							refresh++;
							break;
					}
				}
				break;

			case DLS_READING: /* in progress */
				if( dl->ns )
				{
					if( nets_state( dl->ns ) ) /* file is either done or has failed */
					{
						int tv = timed() - dl->start;
						//Printf( "nets_state(%s) = %ld\n", dl->urlshort, nets_state( dl->ns ) );
						if( nets_state( dl->ns ) > 0 )
						{
							/*
							 * File has been downloaded properly, print the cps and
							 * run the viewer if needed
							 */
							dl->finalcps = max( 1, nets_getdocrealptr( dl->ns ) / ( tv > 0 ? tv : 1 ) );
							if( !dl->dlonly )
							{
								runmv( dl->path, dl->viewapp );
							}
							dl->state = DLS_DONE;
							dl->action = DLA_REFRESH; /* TOFIX: I think.. not sure */
						}
						else
						{
							/* failed */
							dl->state = DLS_FAILED;
							
							if( dl->retries > 0 )
							{
								dl->retries--;
								dl->action = DLA_RESUME;
								dl->state = DLS_START;
							}
						}
						dl->lastgot = nets_getdocrealptr( dl->ns );
						refresh++;
					}
					else if( nets_getdocrealptr( dl->ns) > dl->lastgot )
					{
						dl->lastgot = nets_getdocrealptr( dl->ns );
						dl->action = DLA_REFRESH;
						refresh++;
					}
				}
				break;
		}

		if( dl->state != DLS_DONE )
			alldone = FALSE;
	}

	/* refreshes the list once a second */
	if( refresh )
	{
		int c;

		for( c = 0; ; c++ )
		{
			DoMethod( lv_dl, MUIM_List_GetEntry, c, &dl );
			if( !dl )
				break;
			if( dl->action == DLA_REFRESH && (timed() > lastrefresh || dl->state == DLS_DONE || dl->state == DLS_FAILED ) )
			{
				if( dl->state == DLS_DONE && getflag( VFLG_AUTOCLEANUP_MODES ) )  /* auto cleanup */
				{
					if( ( !dl->finalcps && getflag( VFLG_AUTOCLEANUP_MODES ) == 2 ) || ( dl->finalcps ) )
					{
						DoMethod( lv_dl, MUIM_List_Remove, c );
					}
				}
				
				DoMethod( lv_dl, MUIM_List_Redraw, c );
				dl->action = DLA_NONE;

			}
			else if( dl->action == DLA_REMOVE )
			{
				DoMethod( lv_dl, MUIM_List_Remove, c-- );
			}
			else if( dl->action == DLA_RESUME )
			{
				/* fire off a new network request and close the old one */
				struct nstream *ns = dl->ns;
				dl->ns = nets_open( nets_url( dl->ns ), nets_referer( dl->ns ), FALSE, dlwinptr, NULL, NULL, FALSE, gp_download_timeout );
				nets_close( ns );
			}
		}

		refresh = FALSE;
		lastrefresh = timed();
	}

	if ( alldone && getflag( VFLG_AUTOCLOSEDLWIN ) && getflag( VFLG_AUTOCLEANUP_MODES ) && !getv( lv_dl, MUIA_List_Entries ) )
		set( dlwinptr, MUIA_Window_Open, FALSE );

	if ( alldone )
		return TRUE;
	else
		return FALSE;
}


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
