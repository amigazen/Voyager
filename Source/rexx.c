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
** $Id: rexx.c,v 1.50 2004/01/06 20:23:08 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/rexxsyslib.h>
#if USE_WBSTART
#include <proto/wbstart.h>
#endif /* USE_WBSTART */
#include "voyager_cat.h"
#if USE_REXX
#include <rexx/storage.h>
#endif /* USE_REXX */
#endif

/* private */
#include "classes.h"
#include "urlparser.h"
#include "htmlclasses.h"
#include "rexx.h"
#include "prefs.h"
#include "malloc.h"
#include "file.h"
#include "clip.h"
#include "mui_func.h"
#include "dos_func.h"
#include "template.h"


#ifdef __MORPHOS__
#define RXF(n) \
	static LONG ASM SAVEDS n##_GATE(void); \
	static LONG n##_GATE2(struct Hook *h, ULONG *arg); \
	struct EmulLibEntry n##_hook = { \
	TRAP_LIB, 0, (void (*)(void))n##_GATE }; \
	static LONG ASM SAVEDS n##_GATE(void) { \
	return (n##_GATE2((void *)REG_A0, (void *)REG_A1)); } \
	static struct Hook rxhook_##n = { 0, 0, (void *)&n##_hook }; \
	static LONG n##_GATE2(struct Hook *h, ULONG *arg)
#define RXFS(n) \
	static LONG ASM SAVEDS n##_GATE(void); \
	static LONG n##_GATE2(struct Hook *h, struct RX_##n *arg); \
	struct EmulLibEntry n##_hook = { \
	TRAP_LIB, 0, (void (*)(void))n##_GATE }; \
	static LONG ASM SAVEDS n##_GATE(void) { \
	return (n##_GATE2((void *)REG_A0, (void *)REG_A1)); } \
	static struct Hook rxhook_##n = { 0, 0, (void *)&n##_hook }; \
	static LONG n##_GATE2(struct Hook *h, struct RX_##n *arg)
#else
#define RXF(n) static ULONG ASM SAVEDS rxf_##n();static struct Hook rxhook_##n={{0,0},rxf_##n};static ULONG ASM SAVEDS rxf_##n( __reg( a0, struct Hook *h ), __reg( a1, ULONG *arg ) )
#define RXFS(n) static ULONG ASM SAVEDS rxf_##n();static struct Hook rxhook_##n={{0,0},rxf_##n};static ULONG ASM SAVEDS rxf_##n( __reg( a0, struct Hook *h ), __reg( a1, struct RX_##n *arg ) )
#endif /* !__MORPHOS__ */

#define ARGN(x) (*((ULONG*)(args[x])))

static void setrxs( char *string )
{
	SetAttrs( app, MUIA_Application_RexxString, string, TAG_DONE );
}

static void setrxvar( struct RexxMsg *rm, char *varname, char *contents )
{
#if USE_REXX
	SetRexxVar( ( struct Message * )rm, varname, contents, strlen( contents ) );
#endif
}

static void setrxvarint( struct RexxMsg *rm, char *varname, ULONG num )
{
	char contents[ 64 ];

	stcl_d( contents, num );

#if USE_REXX
	SetRexxVar( ( struct Message * )rm, varname, contents, strlen( contents ) );
#endif
}

/*
 * Common template to be used for ARexx functions.
 * This will be parsed by a special tool to generate
 * the documentation so beware. Arguments are gived
 * with a ' - ' in the first line. Multiple lines are ok, just
 * don't justify them and don't add ' - '.
 *
 * WARNING: MM_DoNumWin	returns 10 if there was no matching window !
 */

/*
 * Structure, remember to use ULONG or STRPTR.
 * Nothing else.
 */

/*
 * Command:
 * Synopsis:
 * Parameters:
 * Result:
 * RC:
 */


/*
 * Command:
 *  - CopyToClip
 * Synopsis:
 *  - Copies the current URL to the clipboard (unit 0)
 * Parameters:
 *  - URL/K: complete URL. Without this argument, the source
 *  is taken from the current frame, image or link.
 * Result:
 * RC:
 *  - 0: Ok
 *  - 5: Error
 */
struct RX_CopyToClip {
	STRPTR url;
};

RXFS( CopyToClip )
{
	D( db_rexx, bug( "called\n" ) );

	if( arg->url )
	{
		if( storetoclip( arg->url ) )
		{
			return( 0 );
		}
		else
		{
			return( 5 );
		}
	}
	else
	{
		return( DoMethod( app, MM_DoRexxWin, /*NULL*/0, MM_HTMLRexx_SetClipFromObject ) );
	}
}


/*
 * Command:
 *  - Fullscreen
 * Synopsis:
 *  - Switches a window to Fullscreen mode
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 */
struct RX_FullScreen {
	ULONG *win;
};

RXFS( FullScreen )
{
	D( db_rexx, bug( "called with window id %ld \n", arg->win ? arg->win : 0 ) );

	//TOFIX!! check the return values
	return( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLWin_ToggleFullScreen ) );
}


/*
 * Command:
 *  - GetActiveWindow
 * Synopsis:
 *  - Gets the current active window.
 * Parameters:
 * Result:
 *  - Number of the active window.
 * RC:
 *  - 0: Ok
 *  - 1: No active window. The number of the previously active window is returned instead.
 *  - 5: No way to determine the active window. Application might be iconified.
 */
RXF( GetActiveWindow )
{
	char buf[ 4 ];
	ULONG num;

	D( db_rexx, bug( "called\n" ) );

	switch( num = DoMethod( app, MM_GetActiveWindow ) )
	{
		case 0:
			return( 5 );
		default:
			stcl_d( buf, num );
			setrxs( buf );
			return( 0 );
	}
}


/*
 * Command:
 *  - GoBackward
 * Synopsis:
 *  - Goes one step back from the current page.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 *  - Position of the page in the list.
 * RC:
 *  - 0: Ok.
 *  - 5: No way to go back. Start of list reached.
 */
struct RX_GoBackward {
	ULONG *win;
};

RXFS( GoBackward )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0 , MM_HTMLWin_Backward ) )
	{
		case 10:
			return( 10 );
		case  0:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 *  - GoForward
 * Synopsis:
 *  - Goes one step forward from the current page.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 *  - Position of the page in the list.
 * RC:
 *  - 0: Ok.
 *  - 5: No way to go forward. Start of list reached.
 */
struct RX_GoForward {
	ULONG *win;
};

RXFS( GoForward )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /* NULL*/ 0, MM_HTMLWin_Forward ) )
	{
		case 10:
			return( 10 );
		case  0:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 *  - GoHome
 * Synopsis:
 *  - Displays the homepage set in the preferences.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 *  - 0: Ok.
 *  - 5: No homepage set in the prefs.
 */
struct RX_GoHome {
	ULONG *win;
};

RXFS( GoHome )
{
	D( db_rexx, bug( "called\n" ) );

	if( !( getprefsstr( DSI_HOMEPAGE, "" )[ 0 ] ) )
	{
		return( 5 );
	}

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLWin_SetURL, getprefsstr( DSI_HOMEPAGE, "about:empty" ), NULL, NULL, MF_HTMLWin_AddURL ) )
	{
		case 10:
			return( 10 );
		case 0:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 *  - LoadBackground
 * Synopsis:
 *  - Loads the background in the page if not already loaded
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 */
struct RX_LoadBackground {
	ULONG *win;
};

RXFS( LoadBackground )
{
	D( db_rexx, bug( "called\n" ) );

	DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /* NULL */ 0, MM_HTMLRexx_LoadBG );

	return( 0 );
}


/*
 * Command:
 *  - LoadImages
 * Synopsis:
 *  - Loads the images in the page.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 */
struct RX_LoadImages {
	ULONG *win;
};

RXFS( LoadImages )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLWin_LoadInlineGfx ) )
	{
		case 10:
			return( 10 );
		case 0:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 *  - OpenDocInfo
 * Synopsis:
 *  - Opens the document information window.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 *  - 0: Ok.
 *  - 5: Couldn't open the window.
 */
struct RX_OpenDocInfo {
	ULONG *win;
	STRPTR url;
};

RXFS( OpenDocInfo )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLRexx_OpenDocInfo, arg->url ) )
	{
		case TRUE:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 *  - OpenSourceView
 * Synopsis:
 *  - Opens the source viewer window.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 *  - 0: Ok.
 *  - 5: Couldn't open the window.
 */
struct RX_OpenSourceView {
	ULONG *win;
};

RXFS( OpenSourceView )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLRexx_OpenSourceView ) )
	{
		case TRUE:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 * - OpenURL (this command is deprecated, use LoadURL instead)
 * Synopsis:
 * - Opens an URL
 * Parameters:
 *  - URL/A: mandatory URL
 *  - NEW=NEWWIN/S: new window
 * Result:
 *  - Window ID.
 * RC:
 * - 0: Ok.
 * - 5: Failure.
 */
struct RX_OpenURL {
	STRPTR url;
	ULONG *newwin;
};

RXFS( OpenURL )
{
	char *url, *pre = "";

	D( db_rexx, bug( "called\n" ) );

	if( !url_hasscheme( arg->url ) )
	{
		pre = "http://";
	}

	if( !( url = malloc( strlen( arg->url ) + strlen( pre ) + 1 ) ) )
	{
		return( 5 );
	}

	sprintf( url, "%s%s", pre, arg->url );
	if( arg->newwin )
	{
		//TOFIX!! check the return values
		win_create( "", url, NULL, NULL, FALSE, TRUE, FALSE );
	}
	else
	{
		switch( DoMethod( app, MM_DoRexxWin, /*NULL*/0, MM_HTMLWin_SetURL, url, NULL, NULL, MF_HTMLWin_Reload | MF_HTMLWin_AddURL ) )
		{
			case 10:
				return( 10 );
			case 0:
				return( 0 );
			default:
				return( 5 );
		}
	}
	return( 0 );
}


/*
 * Command:
 *  - LoadURL
 * Synopsis:
 *  - Opens an URL
 * Parameters:
 *  - WIN/N/K: optional window ID.
 *  - URL/K: complete URL. Without this argument and without the 'RELOAD' keyword, the source
 *  is taken from the current frame, image or link.
 *  - NEW=NEWWIN/S: opens in a new window.
 *  - RELOAD/S: reloads the URL.
 *  - FORCE/S: forces the reloading.
 * Result:
 *  - Window ID.
 * RC:
 *  - 0: Ok.
 *  - 5: Couldn't open a window.
 */
struct RX_LoadURL {
	ULONG *win;
	STRPTR url;
	ULONG *newwin;
	ULONG *reload;
	ULONG *force;
};

RXFS( LoadURL )
{
	char *url, *pre = "";

	D( db_rexx, bug( "called with %ld %s %ld\n", arg->win, ( STRPTR )arg->url ? ( STRPTR )arg->url : ( STRPTR )"no url specified" , arg->newwin ) );

	/*
	 * If we are iconified, we uniconify.. nicer
	 */
	if ( arg->newwin )
	{
		if ( getv( app, MUIA_Application_Iconified ) )
		{
			set( app, MUIA_Application_Iconified, FALSE );
		}
	}

	if( !arg->url )
	{
		//TOFIX!! check the return values
		return( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLRexx_SetURLFromObject, arg->newwin, arg->reload ) );
	}

	if( !url_hasscheme( arg->url ) )
	{
		pre = "http://";
	}

	if( !( url = malloc( strlen( arg->url ) + strlen( pre ) + 1 ) ) )
	{
		return( 5 );
	}

	sprintf( url, "%s%s", pre, arg->url );
	if( arg->newwin )
	{
		//TOFIX!! check the return values
		win_create( "", url, NULL, NULL, FALSE, arg->reload ? TRUE : FALSE, FALSE ); //TOFIX!! force is currently ignored
	}
	else
	{
		switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLWin_SetURL, url, NULL, NULL, ( arg->reload ? MF_HTMLWin_Reload : 0 ) | MF_HTMLWin_AddURL ) ) //TOFIX!! force is currently ignored as well. add 2 values for reload maybe..
		{
			case 10:
				return( 10 );
			case 0:
				return( 0 );
			default:
				return( 5 );
		}
	}
	return( 0 );
}


/*
 * Command:
 *  - Print
 * Synopsis:
 *  - Prints the page.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 *  - NOBACKGROUND/S: omits the background.
 *  - TEXT/S: does a text printout.
 *  - ASK/S: opens the preference box.
 * Result:
 * RC:
 *  - 0: Ok.
 *  - 5: Print error. // hm, well... TOFIX!! perhaps it should be asynchronous :)
 */
struct RX_Print {
	ULONG *win;
	ULONG *nobackground;
	ULONG *text;
	ULONG *ask;
};

RXFS( Print )
{
	D( db_rexx, bug( "called\n" ) );

	if( arg->win )
	{
		//TOFIX!! put the Window ID
	}
	else
	{
		//TOFIX!! NYI :)
		return( 0 );
	}
}


/*
 * Command:
 *  - SaveBackground
 * Synopsis:
 *  - Saves the current page background to disk.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 *  - NAME/K: pathname of the file
 *  - ASK/S: opens a requester to ask for a filename. Prefilled by NAME if present
 * Result:
 * RC:
 *  - 0: Ok
 *  - 5: I/O error
 */
struct RX_SaveBackground {
	ULONG *win;
	STRPTR name;
	ULONG *ask;
};

RXFS( SaveBackground )
{
	D( db_rexx, bug( "called\n" ) );

	return( 5 ); //TOFIX!! NYI
}


/*
 * Command:
 *  - SaveURL
 * Synopsis:
 *  - Saves the URL to disk.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 *  - URL/K: complete URL. Without this argument, the source is taken from the current
 *  frame, image or link.
 *  - NAME/K: name of the file to save. If not supplied, the name of the file will be used.
 *  - ASK/S: ask the name/path of the file before saving.
 * Result:
 * RC:
 *  - 0: Ok
 *  - 5: I/O error
 */
struct RX_SaveURL {
	ULONG *win;
	STRPTR url;
	STRPTR name;
	ULONG *ask;
};

RXFS( SaveURL )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLRexx_SaveURL, arg->url, arg->name, arg->ask ) )
	{
		case TRUE:
			return( 0 );
		default:
			return( 5 );
	}
}


/*
 * Command:
 *  - ScreenToBack
 * Synopsis:
 *  - Puts the current or defined window to back.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 */
struct RX_ScreenToBack {
	ULONG *win;
};

RXFS( ScreenToBack )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MUIM_Window_ScreenToBack ) )
	{
		case 10:
			return( 10 );
		default:
			return( 0 );
	}
}


/*
 * Command:
 *  - ScreenToFront
 * Synopsis:
 *  - Puts the current or defined window to front.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 */
struct RX_ScreenToFront {
	ULONG *win;
};

RXFS( ScreenToFront )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MUIM_Window_ScreenToFront ) )
	{
		case 10:
			return( 10 );
		default:
			return( 0 );
	}
}


/*
 * Command:
 *  - Stop
 * Synopsis:
 *  - Stops the page currently being loaded.
 * Parameters:
 *  - WIN/N/K: optional window ID.
 * Result:
 * RC:
 */
struct RX_Stop {
	ULONG *win;
};

RXFS( Stop )
{
	D( db_rexx, bug( "called\n" ) );

	switch( DoMethod( app, MM_DoRexxWin, arg->win ? *arg->win : /*NULL*/0, MM_HTMLWin_StopXfer ) )
	{
		case 10:
			return( 10 );
		default:
			return( 0 );
	}
}


#define RXH(s) &rxhook_##s
#define DEFTE(n,id) { n, MC_TEMPLATE_ID, id, NULL },
#define DEFHE( n,t,tn,f ) { n, t, tn, RXH( f ) },

struct MUI_Command rexxcmds[] = {
	DEFHE( "CopyToClip",            "URL/K",   1, CopyToClip )
	DEFHE( "FullScreen",			"WIN/N/K", 1, FullScreen )
	DEFHE( "GetActiveWindow",		"", 	   0, GetActiveWindow )
	DEFHE( "GoBackward",			"WIN/N/K", 1, GoBackward )
	DEFHE( "GoForward",				"WIN/N/K", 1, GoForward )
	DEFHE( "GoHome",				"WIN/N/K", 1, GoHome )
	DEFHE( "LoadBackground",		"WIN/N/K", 1, LoadBackground )
	DEFHE( "LoadImages",			"WIN/N/K", 1, LoadImages )
	DEFHE( "LoadURL",          		"WIN/N/K,URL/K,NEW=NEWWIN/S,RELOAD/S,FORCE/S", 5, LoadURL )
	DEFHE( "OpenDocInfo",			"WIN/N/K,URL/K", 2, OpenDocInfo )
	DEFHE( "OpenSourceView",		"WIN/N/K", 1, OpenSourceView )
	DEFHE( "OpenURL",          		"URL/A,NEW=NEWWIN/S", 2, OpenURL )
	DEFHE( "Print",					"WIN/N/K,NOBACKGROUND/S,TEXT/S,ASK/S", 4, Print )
	DEFHE( "SaveBackground",		"WIN/N/K,NAME/K,ASK/S", 3, SaveBackground )
	DEFHE( "SaveURL",				"WIN/N/K,URL/K,NAME/K,ASK/S", 4, SaveURL )
	DEFHE( "ScreenToBack",          "WIN/N/K", 1, ScreenToBack )
	DEFHE( "ScreenToFront",         "WIN/N/K", 1, ScreenToFront )
	DEFHE( "Stop",					"WIN/N/K", 1, Stop )
	{ NULL }
};


/*
 * ARexx initialization stuff
 */
static struct MsgPort *ipcport, *rexxreply;
static ULONG rexxpending;

#if (INCLUDE_VERSION >= 44) && !defined(__MORPHOS__)
	struct RxsLib *RexxSysBase;
#else
	struct Library *RexxSysBase;
#endif

#ifdef MBX
#define createmsgport CreateMsgPort
#else
struct MsgPort *createmsgport( void )
{
	struct MsgPort *mp;

	mp = malloc( sizeof( *mp ) );
	if( mp )
	{
		memset( mp, 0, sizeof( *mp ) );
		NEWLIST( &mp->mp_MsgList );
		mp->mp_SigBit = SIGBREAKB_CTRL_E;
		mp->mp_SigTask = FindTask( NULL );
	}
	return( mp );
}
#endif

int init_ipc( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( ipcport = createmsgport() )
	{
#if USE_REXX
		if( rexxreply = createmsgport() )
			RexxSysBase = OpenLibrary( "rexxsyslib.library", 33 );

#endif
		return( TRUE );
	}

	return( FALSE );
}

#if USE_REXX
static void freerxmsg( struct RexxMsg *m )
{
	ClearRexxMsg( m, 2 );
	DeleteRexxMsg( m );
}
#endif

void cleanup_ipc( void )
{
#if USE_REXX
	D( db_init, bug( "cleaning up..\n" ) );

	while( rexxpending-- )
	{
		WaitPort( rexxreply );
		freerxmsg( ( struct RexxMsg * ) GetMsg( rexxreply ) );
	}
	CloseLibrary( RexxSysBase );
#endif

#ifdef MBX
	DeleteMsgPort(ipcport);
#endif
}


/*
 * Returns TRUE if the string s matches the command c
 */
int match_command( int type, STRPTR c, STRPTR s )
{
	int first_run = TRUE;

	if( type == BFUNC_COMMAND )
	{
	    while( *s )
		{
			if( *s == ' ' )
			{
				if( first_run )
				{
					return( FALSE );
				}
				break;
			}

			if( toupper( *s ) != toupper( *c ) )
			{
				return( FALSE );
			}

			c++;
			s++;

			first_run = FALSE;
		}
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


#if 0
void send_internal_command( STRPTR buf )
{
	ULONG c = 0;

	while( rexxcmds[ c ].mc_Name )
	{
		if( !strnicmp( buf + 1, rexxcmds[ c ].mc_Name, strlen( rexxcmds[ c ].mc_Name ) ) ) /* XXX: this is wrong.. see how ambient does it.. also there's a '' around the command name ( buf + 1 is the workaround ) */
		{
			D( db_rexx, bug( "processing command <%s>\n", rexxcmds[ c ].mc_Name ) );
			{
				//void (*fkt)(void);
				//fkt = (void(*)(void))(rexxcmds[ c ].mc_Hook->h_Entry);
				//fkt();
				CallHookPkt( rexxcmds[ c ].mc_Hook, NULL, NULL );
//				( rexxcmds[ c ].mc_Hook->h_Entry )( );
			}
			break;
		}
		c++;
	}

}
#else
static struct RDArgs * readargsstring(STRPTR source, STRPTR templ, ULONG *array)
{
	struct RDArgs *result = NULL;

	struct RDArgs *rdasrc;
	int len;
	STRPTR alloc = NULL;

	if ((rdasrc = AllocDosObject(DOS_RDARGS,NULL)))
	{
		if (!source || !*source) source = "\n";

		len = strlen(source);

		if (source[len - 1] != '\n')
		{
			if (alloc = malloc(len + 2))
			{
				strcpy(alloc, source);
				alloc[len] = '\n';
				alloc[++len] = 0;
				source = alloc;
			}
		}

		rdasrc->RDA_Source.CS_Buffer = source;
		rdasrc->RDA_Source.CS_Length = len;
		rdasrc->RDA_Source.CS_CurChr = 0;

		result = ReadArgs(templ,(LONG *)array,rdasrc);

		//D(DBF_MISC,bug("ReadArgs(rdasrc=%08lx)=%08lx\n",rdasrc,result));

		if (alloc)
		{
			free(alloc);
		}

		if (!result)
		{
			FreeDosObject(DOS_RDARGS, rdasrc);
		}
	}
	return (result);
}


static void freeargsstring(struct RDArgs *rda)
{
	if (rda)
	{
		FreeArgs(rda);
		FreeDosObject(DOS_RDARGS, rda);
	}
}


void send_internal_command(STRPTR buf)
{
	//ULONG c = 0;
	ULONG cmdlen;
	STRPTR params;

	cmdlen = strlen(buf);

	if (params = strchr(buf, ' '))
	{
		cmdlen = params - buf;
		while (*params == ' ')
		{
			params++;
		}
	}

	if (cmdlen > 0)
	{
		struct MUI_Command *com;


		for (com = rexxcmds; com && com->mc_Name && strnicmp(buf, com->mc_Name, cmdlen); com++);

		if (com && com->mc_Name)
		{
			/* XXX: handle MC_TEMPLATE_ID ? */

			if (com->mc_Hook)
			{
				ULONG *array = NULL;

				if (!com->mc_Template || com->mc_Parameters <= 0 || (array = malloc(com->mc_Parameters * 4)))
				{
					struct RDArgs *rda = NULL;

					memset(array, '\0', com->mc_Parameters * 4);

					if (!array || (rda = readargsstring(params, com->mc_Template, array)))
					{
						LONG erg;

						erg = CallHookPkt(com->mc_Hook, NULL, array);

						/* XXX: we could check for rexxstring & stuff maybe.. check all that.. */
						//*resval = erg;

						if (rda)
						{
							freeargsstring(rda);
						}
						else
						{
							//*resval = MUI_RXERR_BADSYNTAX;
						}

						if (array)
						{
							free(array);
						}
					}
					else
					{
						//*resval = MUI_RXERR_OUTOFMEMORY;
					}
				}
				else
				{
					//*resval = MUI_RXERR_BADDEFINITION;
				}
			}
			/* XXX: stripped some stuff there.. see original */
		}
		else
		{
			//*resval = MUI_RXERR_UNKNOWNCOMMAND;
		}
	}
	return;
}

#endif


struct Library *WBStartBase;
ULONG command_runmode;
APTR rexx_obj; /* object which will get the method */
ULONG use_rexx;

/*
 * Command execution part
 */
void execute_command( int type, STRPTR str, ULONG mode, STRPTR obj_url, STRPTR obj_link, STRPTR obj_window )
{
	char str_parsed[ VREXX_MAXLENGTH ]; /* TOFIX: could be smarter */
	char buf[ VREXX_MAXLENGTH + 14 ]; /* 10 == 'C:Execute ', well no, that's old.. now 14 == ' FRAME="_top" ' */

	D( db_rexx, bug( "obj_url == %s, obj_link == %s, obj_window == %s\n", ( STRPTR )obj_url ? ( STRPTR )obj_url : ( STRPTR )"none", ( STRPTR ) obj_link ? ( STRPTR )obj_link : ( STRPTR )"none", ( STRPTR )obj_window ? ( STRPTR )obj_window : ( STRPTR )"none" ) );

	if( str[ 0 ] )
	{
		/*
		 * Parse template
		 */
		STRPTR ps = ( STRPTR )getv( app, MUIA_Application_PubScreenName );

		expandtemplate( str, str_parsed, sizeof( str_parsed ),
			'u', obj_url,
			'l', obj_link,
			'w', obj_window,
			'p', ( ps && *ps ) ? ( STRPTR )ps : ( STRPTR )"Workbench"
		);

		switch( type )
		{
			case BFUNC_COMMAND:
				/*
				 * We set a global variable to know
				 * if we need to use a frame or a window
				 * because it's not easy to parse the string
				 * like ReadArgs() does and is not very
				 * portable
				 */
				if( mode == VREXX_WINDOW )
				{
					/*
					 * Method will be sent to
					 * the HTMLWin object
					 */
					command_runmode = VREXX_WINDOW;
				}
				else
				{
					/*
					 * Method will be sent to
					 * the HTMLView object
					 */
					command_runmode = VREXX_FRAME;
				}

#if USE_REXX
				if ( use_rexx )
				{
					sprintf( buf, "'%s'", str_parsed );
					D( db_rexx, bug( "sending message %s\n", buf ) );

					VAT_SendRXMsg( buf, VREXXPORT, VREXXEXT );
				}
				else
#endif
				{
					sprintf( buf, "%s", str_parsed ); /* XXX: yeah well.. this isn't too smart.. the parser should take care of that.. */
					D( db_rexx, bug( "sending message %s\n", buf ) );
					send_internal_command( buf );
				}
				break;

#ifndef MBX
			case BFUNC_AMIGADOS:
				D( db_rexx, bug( "executing %s in AmigaDOS mode\n", str_parsed ) );
				mySystemTags( str_parsed, TAG_DONE );
				break;

			case BFUNC_WORKBENCH:
#if USE_WBSTART
				D( db_rexx, bug( "executing %s in Workbench mode\n", str_parsed ) );
				if( !WBStartBase )
				{
					if( !( WBStartBase = OpenLibrary( WBSTART_NAME, 2 ) ) )
					{
						MUI_Request( app, NULL, 0, copyright, GS( OK ), GS( WBSTART_FAILED ) );
						break;
					}
				}

				//TOFIX!! add args support... shrug :)
				WBStartTags( WBStart_Name, str,
							 WBStart_DirectoryLock, currentdir_lock,
							 TAG_DONE
				);
#endif /* USE_WBSTART */
				break;

			case BFUNC_SCRIPT:
				D( db_rexx, bug( "executing DOS script %s\n", str_parsed ) );
				Execute( str_parsed, NULL, NULL );
				break;

			case BFUNC_AREXX:
				D( db_rexx, bug( "executing ARexx script %s\n", str_parsed ) );
				VAT_SendRXMsg( str_parsed, VREXXPORT, VREXXEXT );
				break;
#endif /* !MBX */
		}
	}
}
