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
** $Id: prefs.c,v 1.154 2004/10/09 14:57:00 henes Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#endif

/* private */
#include "voyager_cat.h"
#include "prefs.h"
#include "copyright.h"
#include "network.h"
#include "time_func.h"
#include "autoproxy.h"
#include "menus.h"
#include "cache.h"
#include <proto/vimgdecode.h>
#include "mui_func.h"
#include "classes.h"
#include "htmlclasses.h"
#include "menus.h"
#include "imgstub.h"
#include "dos_func.h"
#include "nlist.h"

static APTR prefspool, clonepool;
struct MinList prefslist = {NULL,NULL,NULL}, clonelist = {NULL,NULL,NULL};
#if USE_DOS
static int prefsloaded;

static time_t starttime, lastusedtime;
static int usecount;
#endif

#define PREFSFILEID MAKE_ID('V','Y','²',0)

#define MAXDATALENGTH 1024 /* maximum length of the data of a prefsnode */

struct prefsnode {
	struct MinNode n;
	ULONG id;
	ULONG size;
	char data[ 0 ];
};

int init_prefs( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &prefslist );
	prefspool = CreatePool( MEMF_ANY, 2048, 1024 );

#if USE_DOS
	time( &starttime );
#endif

	return( ( int )prefspool );
}

void cleanup_prefs( void )
{
	D( db_init, bug( "cleaning up..\n" ) );

	if( prefspool )
	{
		DeletePool( prefspool );
	}
	
	if( clonepool )
	{
		DeletePool( clonepool );
	}
}

#if defined( MBX ) || defined( __MORPHOS__ )

static __inline struct prefsnode * fpn( ULONG id )
{
	struct prefsnode *n, *nn;

	if( ISLISTEMPTY( &prefslist ) )
		return( NULL );

	for( n = FIRSTNODE( &prefslist ); nn = NEXTNODE( n ); n = nn )
		if( n->id == id )
			return( n );

	return( NULL );
}

static __inline struct prefsnode * fpn_clone( ULONG id )
{
	struct prefsnode *n, *nn;
	
	if( ISLISTEMPTY( &prefslist ) )
		return( NULL );

	for( n = FIRSTNODE( &clonelist ); nn = NEXTNODE( n ); n = nn )
		if( n->id == id )
			return( n );

	return( NULL );
}

APTR getprefs( ULONG id )
{
	struct prefsnode *n = fpn( id );
	return( n ? n->data : NULL);
}

#endif /* MBX || __MORPHOS__ */

#ifndef MBX
extern ASM struct prefsnode *fpn( __reg( d1, ULONG id ) );
extern ASM struct prefsnode *fpn_clone( __reg( d1, ULONG id ) );
#endif

static __inline struct prefsnode *mkpn( ULONG id, ULONG size, APTR data )
{
	struct prefsnode *n;

	n = AllocPooled( prefspool, sizeof( *n ) + size );
	n->id = id;
	n->size = size;
	if( size && data )
		memcpy( n->data, data, size );

	ADDTAIL( &prefslist, n );

	return( n );
}

static __inline void rmpn( struct prefsnode *pn )
{
	REMOVE( pn );
	FreePooled( prefspool, pn, sizeof( *pn ) + pn->size );
}

//
// I/O support
//

#if USE_DOS
static ULONG readlong( struct AsyncFile *f )
{
	ULONG v = 0;

	ReadAsync( f, &v, sizeof( v ) );
	return( v );
}
#endif

#if USE_DOS
static void writelong( struct AsyncFile *f, ULONG v )
{
	WriteAsync( f, &v, sizeof( v ) );
}
#endif

/*
 * Encoding
 */

/*
 * Due to a weird design issue (and pot probably)
 * we are stuck to SAS/C's rand() function to keep
 * cross platform compatibility
 */
#ifdef __SASC
#define SRAND srand
#define RAND rand
#else
#define SRAND sas_srand
#define RAND sas_rand
unsigned int sas_seed;

void sas_srand( unsigned int s )
{
	sas_seed = s - 1;
}

int sas_rand( void )
{
	int ret = sas_seed * 0xe5d + 0xe60;
	ret &= 0x7fffffff;
	sas_seed = ret - 1;

	return( ret );
}
#endif

static void __inline codebytes( UBYTE *data, int size )
{
	while( size-- )
		*data++ ^= RAND() & 0xff;
}

/*
 * This is a kludge. Design decisions aren't
 * easy when public betas are released around.
 * So when there's a command change we try to do
 * our best to change it through the whole
 * prefsfile. This function only works for
 * commands.
 */
void convert_user_prefs( char *in, char *out )
{
	ULONG n;
	ULONG c;
	ULONG mode[ 4 ];

	/*
	 * Toolbar
	 */
	n = getprefslong( DSI_BUTTON_NUM, 0 );

	for( c = 0; c < n; c++ )
	{
		if( !stricmp( getprefsstr( DSI_BUTTONS_ARGS + c, "" ), in ) )
		{
			setprefsstr( DSI_BUTTONS_ARGS + c, out );
		}
	}

	/*
	 * Context menus
	 */
	for( n = VMT_PAGEMODE; n <= VMT_IMAGEMODE; n++ )
	{
		c = 0;

		switch( n )
		{
			case VMT_PAGEMODE:
				mode[ MODE_LABELS ] = DSI_CMENUS_PAGE_LABELS;
				mode[ MODE_ARGS ] = DSI_CMENUS_PAGE_ARGS;
				break;

			case VMT_LINKMODE:
				mode[ MODE_LABELS ] = DSI_CMENUS_LINK_LABELS;
				mode[ MODE_ARGS ] = DSI_CMENUS_LINK_ARGS;
				break;

			case VMT_IMAGEMODE:
				mode[ MODE_LABELS ] = DSI_CMENUS_IMAGE_LABELS;
				mode[ MODE_ARGS ] = DSI_CMENUS_IMAGE_ARGS;
				break;
		}

		while( getprefsstr( mode[ MODE_LABELS ] + c, "" )[ 0 ] )
		{
			if( !stricmp( getprefsstr( mode[ MODE_ARGS ] + c, "" ), in ) )
			{
				setprefsstr( mode[ MODE_ARGS ] + c, out );
			}
			c++;
		}
	}
}


int loadprefs( char *filename )
{
#if USE_DOS
	struct AsyncFile *f;
	ULONG id, size;
	ULONG numnodes;
	struct prefsnode *n;
	ULONG num;
	ULONG c;

	prefsloaded = TRUE;

	f = OpenAsync( filename, MODE_READ, 1024 );
	if( !f )
		return( -1 );

	id = readlong( f );

	if( id != PREFSFILEID )
	{
		CloseAsync( f );
		return( -2 );
	}
	numnodes = readlong( f );   
	SRAND( readlong( f ) );

	while( numnodes-- )
	{
		id = readlong( f );
		size = readlong( f );

		if( size > MAXDATALENGTH )
			break;

		if( n = fpn( id ) )
			rmpn( n );

		n = mkpn( id, size, NULL );
		if( size )
		{
			if( ReadAsync( f, n->data, size ) == size )
			{
				codebytes( n->data, size );
			}
			else
			{
				rmpn( n );
				break;
			}
		}
	}
	
	CloseAsync( f );

	usecount = getprefslong( DSI_NAG_USECOUNT, 0 ) + 1;
	lastusedtime = getprefslong( DSI_NAG_UPTIME, 0 );
	
	/*
	 * Update old style font prefs to new one
	 */
	if( !getprefs( DSI_FONT_FACENAME( 0 ) ) )
	{
		static __far UBYTE fmap[ 7 ] = { 2, 1, 0, 3, 4, 5, 6 };

		for( c = 0; c < 7; c++ )
		{
			setprefsstr( DSI_FONT_MAP( 0, c ), getprefsstr( DSI_FONTS + fmap[ c ], "" ) );
			setprefsstr( DSI_FONT_MAP( 2, c ), getprefsstr( DSI_FONTS + fmap[ c ], "" ) );
		}

		for( c = 0; c < 7; c++ )
			setprefsstr( DSI_FONT_MAP( 1, c ), getprefsstr( DSI_FONTS + 13, "" ) );

		setprefsstr( DSI_FONT_FACENAME( 0 ), "(Default)" );
		setprefsstr( DSI_FONT_FACENAME( 1 ), "(Fixed)" );
		setprefsstr( DSI_FONT_FACENAME( 2 ), "(Template)" );

		setprefslong( DSI_FONT_FACE_NUM, 3 );
	}

	/*
	 * Update old style button prefs to new one
	 */
	num = getprefslong( DSI_BUTTON_NUM, 0 );

	for( c = 0; c < num; c++ )
	{
		switch( getprefslong( DSI_BUTTONS_ACTION + c, BFUNC_SEP ) )
		{
			case bfunc_rexx:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_AREXX );
				/* well... that's lost, too bad :) */
				break;
			
			case bfunc_js:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_JAVASCRIPT );
				/* lost too... but who cares */
				break;

			case bfunc_back:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "GoBackward" );
				break;

			case bfunc_forward:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "GoForward" );
				break;

			case bfunc_find:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "LoadURL URL=search:" );
				break;

			case bfunc_home:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "GoHome" );
				break;

			case bfunc_loadimages:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "LoadImages" );
				break;

			case bfunc_print:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "Print ASK" );
				break;

			case bfunc_reload:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "LoadURL RELOAD FORCE" );
				break;
			
			case bfunc_stop:
				setprefslong( DSI_BUTTONS_ACTION + c, BFUNC_COMMAND );
				setprefsstr( DSI_BUTTONS_ARGS + c, "Stop" );
				break;
		}
	}

	/* 3.3.56+ */
	convert_user_prefs( "ReloadURL FORCE", "LoadURL RELOAD FORCE" );

	/* 3.3.77+ */
	convert_user_prefs( "OpenURL RELOAD FORCE", "LoadURL RELOAD FORCE" );
	convert_user_prefs( "OpenURL NEW", "LoadURL NEW" );
	convert_user_prefs( "OpenURL", "LoadURL" );
	
	setup_useragent();

	return( 0 );
#else
	return( -1 );
#endif /* USE_DOS */
}

int saveprefs( char *filename )
{
#if USE_DOS
	struct AsyncFile *f;
	ULONG id;
	ULONG numnodes;
	char buffer[ 256 ];
	struct prefsnode *n;

	if( !prefsloaded )
		return( FALSE);

	strcpy( buffer, filename );
	strcat( buffer, ".bak" );
	DeleteFile( buffer );
	Rename( filename, buffer );

	f = OpenAsync( filename, MODE_WRITE, 1024 );
	if( !f )
		return( -1 );

	setprefslong( DSI_NAG_USECOUNT, usecount );
	setprefslong( DSI_NAG_UPTIME, lastusedtime + ( timev() - starttime + 59 ) / 60 );

	writelong( f, PREFSFILEID );

	for( numnodes = 0, n = FIRSTNODE( &prefslist ); NEXTNODE( n ); n = NEXTNODE( n ) )
		numnodes++;
	writelong( f, numnodes );

	id = timev() + RAND();
	SRAND( id );
	writelong( f, id );

	for( n = FIRSTNODE( &prefslist ); numnodes--; n = NEXTNODE( n ) )
	{
		writelong( f, n->id );
		writelong( f, n->size );
		if( n->size )
		{
			UBYTE *p = n->data;
			int size = n->size;

			while( size-- )
				WriteCharAsync( f, *p++ ^ ( RAND() & 0xff ) );
		}
	}

#define IDSTRING1 "$VER"
#define IDSTRING2 ": Voyager_Prefs " LVERTAG

	WriteAsync( f, IDSTRING1, strlen( IDSTRING1 ) );
	WriteAsync( f, IDSTRING2, strlen( IDSTRING2 ) );

	CloseAsync( f );

	return( 0 );
#else
	return( FALSE );
#endif /* USE_DOS */
}

void killprefs( ULONG id )
{
	struct prefsnode *n = fpn( id );
	if( n )
		rmpn( n );  
}

void killprefsrange( ULONG fromid, ULONG toid )
{
	struct prefsnode *pn, *next;

	for( pn = FIRSTNODE( &prefslist ); next = NEXTNODE( pn ); pn = next )
	{
		if( pn->id >= fromid && pn->id <= toid )
			rmpn( pn );
	}
}


void setprefs( ULONG id, ULONG size, APTR data )
{
	killprefs( id );
	mkpn( id, size, data );
}

void setprefsstr( ULONG id, STRPTR data )
{
	killprefs( id );
	mkpn( id, strlen( data ) + 1, data );
}


void setprefslong( ULONG id, ULONG v )
{
	killprefs( id );
	mkpn( id, 4, &v );
}

STRPTR getprefsstr( ULONG id, STRPTR def )
{
	struct prefsnode *n = fpn( id );

	if( n )
		return( n->data );
	else
		return( def );
}

ULONG getprefslong( ULONG id, ULONG def )
{
	struct prefsnode *n = fpn( id );

	if( n )
	{
		ULONG *x = (ULONG*)n->data;
		return( *x );
	}
	else
		return( def );
}

//
// ************************************************************
// Pre init standard prefs
//

static void maketb( int num, char *label, char *file, int func, char *args, char *hotkey )
{
	setprefsstr( DSI_BUTTONS_LABELS + num, label );
	setprefslong( DSI_BUTTONS_ACTION + num, func );
	setprefsstr( DSI_BUTTONS_IMAGES + num, file );
	setprefsstr( DSI_BUTTONS_SHORTCUT + num, hotkey );
	setprefsstr( DSI_BUTTONS_ARGS + num, args );
}

void initprefs( void )
{
	static struct MUI_PenSpec pcols[ 5 ] = {
		{ "rffffffff,ffffffff,ffffffff" },
		{ "r00000000,00000000,00000000" },
		{ "r00000000,00000000,cccccccc" },
		{ "rffffffff,10101010,10101010" },
		{ "r80808080,80808080,80808080" }
	};
	int c;

	// Setup default colors
	for( c = 0; c < 5; c++ )
		setprefs( DSI_COLORS + c, sizeof( pcols[ 0 ] ), &pcols[ c ] );

	// Fonts
	for( c = 0; c < 7; c++ )
	{
		static UBYTE psizes[] = { 11, 12, 14, 16, 24, 32, 40 };
		char buffer[ 64 ];

		sprintf( buffer, "ttftemplate/%ld", (LONG)psizes[ c ] );
		setprefsstr( DSI_FONT_MAP( 2, c ), buffer );
	}

#ifdef MBX
	setprefsstr( DSI_FONT_MAP( 0, 0 ), "romprop/15" );       // smallest
	setprefsstr( DSI_FONT_MAP( 0, 1 ), "romprop/17" );
	setprefsstr( DSI_FONT_MAP( 0, 2 ), "romprop/20" );       // default
	setprefsstr( DSI_FONT_MAP( 0, 3 ), "romprop/25" );
	setprefsstr( DSI_FONT_MAP( 0, 4 ), "romprop/30" );
	setprefsstr( DSI_FONT_MAP( 0, 5 ), "romprop/35" );
	setprefsstr( DSI_FONT_MAP( 0, 6 ), "romprop/40" );       // largest

	setprefsstr( DSI_FONT_MAP( 1, 0 ), "romfixed/11" );
	setprefsstr( DSI_FONT_MAP( 1, 1 ), "romfixed/13" );
	setprefsstr( DSI_FONT_MAP( 1, 2 ), "romfixed/15" );
	setprefsstr( DSI_FONT_MAP( 1, 3 ), "romfixed/17" );
	setprefsstr( DSI_FONT_MAP( 1, 4 ), "romfixed/20" );
	setprefsstr( DSI_FONT_MAP( 1, 5 ), "romfixed/25" );
	setprefsstr( DSI_FONT_MAP( 1, 6 ), "romfixed/30" );

	setprefsstr( DSI_FONT_FACENAME( 3 ), "Unicode" );

	setprefsstr( DSI_FONT_MAP( 3, 0 ), "unicode/11" );
	setprefsstr( DSI_FONT_MAP( 3, 1 ), "unicode/13" );
	setprefsstr( DSI_FONT_MAP( 3, 2 ), "unicode/15" );
	setprefsstr( DSI_FONT_MAP( 3, 3 ), "unicode/17" );
	setprefsstr( DSI_FONT_MAP( 3, 4 ), "unicode/20" );
	setprefsstr( DSI_FONT_MAP( 3, 5 ), "unicode/25" );
	setprefsstr( DSI_FONT_MAP( 3, 6 ), "unicode/30" );

#else
	setprefsstr( DSI_FONT_MAP( 0, 0 ), "helvetica/9" );    // smallest
	setprefsstr( DSI_FONT_MAP( 0, 1 ), "times/11" );
	setprefsstr( DSI_FONT_MAP( 0, 2 ), "times/13" );       // default
	setprefsstr( DSI_FONT_MAP( 0, 3 ), "times/15" );
	setprefsstr( DSI_FONT_MAP( 0, 4 ), "times/18" );
	setprefsstr( DSI_FONT_MAP( 0, 5 ), "times/24" );
	setprefsstr( DSI_FONT_MAP( 0, 6 ), "times/24" );       // largest

	setprefsstr( DSI_FONT_MAP( 1, 0 ), "courier/11" );
	setprefsstr( DSI_FONT_MAP( 1, 1 ), "courier/11" );
	setprefsstr( DSI_FONT_MAP( 1, 2 ), "courier/13" );
	setprefsstr( DSI_FONT_MAP( 1, 3 ), "courier/15" );
	setprefsstr( DSI_FONT_MAP( 1, 4 ), "courier/18" );
	setprefsstr( DSI_FONT_MAP( 1, 5 ), "courier/24" );
	setprefsstr( DSI_FONT_MAP( 1, 6 ), "courier/24" );
#endif

	setprefsstr( DSI_FONT_FACENAME( 0 ), "(Default)" );
	setprefsstr( DSI_FONT_FACENAME( 1 ), "(Fixed)" );
	setprefsstr( DSI_FONT_FACENAME( 2 ), "(Template)" );

	/*setprefsstr( DSI_FONTS + 7, "cgtriumvirate/38" );
	setprefsstr( DSI_FONTS + 8, "cgtriumvirate/30" );
	setprefsstr( DSI_FONTS + 9, "cgtriumvirate/24" );
	setprefsstr( DSI_FONTS + 10, "cgtriumvirate/18" );
	setprefsstr( DSI_FONTS + 11, "cgtriumvirate/15" );
	setprefsstr( DSI_FONTS + 12, "cgtriumvirate/13" );

	setprefsstr( DSI_FONTS + 7, "times/24" );
	setprefsstr( DSI_FONTS + 8, "times/18" );
	setprefsstr( DSI_FONTS + 9, "times/15" );
	setprefsstr( DSI_FONTS + 10, "times/13" );
	setprefsstr( DSI_FONTS + 11, "times/11" );
	setprefsstr( DSI_FONTS + 12, "helvetica/9" );

	setprefsstr( DSI_FONTS + 13, "courier/13" );*/

	// Fastlinks
#define FASTLINKG( n, l, u ) setprefsstr( DSI_FASTLINKS_LABELS + n, l ); setprefsstr( DSI_FASTLINKS_URLS + n, u );

#ifndef MBX
	FASTLINKG( 0, "Vapor", "http://www.vapor.com/" )
	FASTLINKG( 1, "AmigaNews", "http://www.amiga-news.de/" )
	FASTLINKG( 2, "Google", "http://www.google.com/" )
	FASTLINKG( 3, "Yahoo", "http://www.yahoo.com/" )
	FASTLINKG( 4, "Alta Vista", "http://www.altavista.com/" )
	FASTLINKG( 5, "Lycos", "http://www.lycos.com/" )
	FASTLINKG( 6, "Metacrawler", "http://www.metacrawler.com/" )
	FASTLINKG( 7, "AllTheWeb", "http://www.alltheweb.com/" )
#else
	FASTLINKG( 0, "Met@box", "http://www.metabox.de/" )
	FASTLINKG( 1, "Met@TV", "http://www.metatv.de/html/content/index.html" )
	FASTLINKG( 2, "Discovery", "http://www.discovery.com/" )
	//FASTLINKG( 2, "WebTV", "http://www.webtv.com/" )
	//FASTLINKG( 3, "gnu", "http://www.gnu.org/" )
	//FASTLINKG( 3, "w3", "http://www.w3.org/Graphics/PNG/inline-alpha.html" )
	//FASTLINKG( 3, "Sejin", "http://www.sejin.com/free00.htm" )
	FASTLINKG( 3, "Testsuite", "http://v3.vapor.com/vapor/vtestsuite/" )
	FASTLINKG( 4, "heise", "http://www.heise.de/" )
	FASTLINKG( 5, "Show", "http://212.96.44.167/" )
	FASTLINKG( 6, "The Register", "http://www.theregister.co.uk/" )
	FASTLINKG( 7, "Alta Vista", "http://www.altavista.com/" )
#endif

	// set proxy
	for( c = 0; c < 4; c++ )
		setprefslong( DSI_PROXY_PORT_OFFSET + DSI_PROXY_HTTP + c, 3128 );
	setprefsstr( DSI_PROXY_GOPHER, "proxy" );
	setprefsstr( DSI_PROXY_WAIS, "proxy" );

	// network
	setprefslong( DSI_NET_MAXCON, 8 );
	setprefslong( DSI_NET_CHECKIFONLINE, FALSE );

	// cache
	setprefslong( DSI_CACHE_SIZE, 8192 );
	setprefslong( DSI_CACHE_AUTOVERIFY, 15 );
	setprefslong( DSI_CACHE_MEMSIZE, 1024 );
	setprefslong( DSI_CACHE_IMAGES, TRUE );
	setprefsstr( DSI_CACHE_DIR, "PROGDIR:Cache" );

	// mail & news
	setprefsstr( DSI_NET_REALNAME, "Lazy Bone" );
	setprefsstr( DSI_NET_ORGANIZATION, "Badly Configured Clients, Inc." );
	setprefsstr( DSI_NET_TELNET, "AmTelnet %h %p" );
	setprefsstr( DSI_NET_SMTP, "mail" );
	setprefsstr( DSI_NET_NNTP, "news" );
	setprefsstr( DSI_NET_SIG, "S:.signature" );

	// homepage
#ifdef MBX
	//	setprefsstr( DSI_HOMEPAGE, "about:demo" );
#ifdef MBXUSA
//	setprefsstr( DSI_HOMEPAGE, "file:///rom0:portal/index.html");
//	setprefsstr( DSI_HOMEPAGE, "about:demo" );
	setprefsstr( DSI_HOMEPAGE, "http://www.metaboxusa.com/demo2/index.htm" );
#else
	setprefsstr( DSI_HOMEPAGE, "file:///rom0:portal/index.html" );
#endif
//	setprefsstr( DSI_HOMEPAGE, "http://212.96.33.222:8082" );
//	setprefsstr( DSI_HOMEPAGE, "http://www.vorwerkbox.de" );	
#else
	setprefsstr( DSI_HOMEPAGE, "http://v3.vapor.com/" );
#endif
#if USE_NET
	setflag( VFLG_HOMEPAGEURL_MODES, 0 );
#else
	setflag( VFLG_HOMEPAGEURL_MODES, 2 );
#endif /* USE_NET */

#ifdef STUNTZI
	setprefsstr( DSI_HOMEPAGE, "about:demo" );
#endif

	// languages
	setprefsstr( DSI_MISC_LANGUAGES, "en, *" );

	// links
	setprefslong( DSI_NET_LINKS_EXPIRE, 30 );

	setflag( VFLG_LOAD_IMAGES, TRUE );
	setflag( VFLG_SHOW_FASTLINKS, TRUE );
	//setflag( VFLG_ASKDLDIR, TRUE ); obsolete

#ifdef MBX
	setflag( VFLG_UNDERLINE_LINKS, FALSE );
#else
	setflag( VFLG_UNDERLINE_LINKS, TRUE );
#endif

	setflag( VFLG_SAVE_ON_EXIT, TRUE );
	setflag( VFLG_AUTOCLEANUP_MODES, 1 );
	setflag( VFLG_SCROLLBARS, TRUE );
	setflag( VFLG_TEAROFF, TRUE );
	setflag( VFLG_FASTLINKS_STRIPTEXT, FALSE );

#ifdef MBX
	// TOFIX! If we have a working font engine again
	setflag( VFLG_FONTFACE, FALSE );
#else
	setflag( VFLG_FONTFACE, TRUE );
#endif

	setflag( VFLG_IMG_SHOW_ALT_TEXT, TRUE );
	setflag( VFLG_IMG_SHOW_PH_BORDER, TRUE );

	setprefslong( DSI_SECURITY_WARN_POST, TRUE );
	setprefslong( DSI_SECURITY_ASK_MAILTO, TRUE );
	setprefslong( DSI_SECURITY_ASK_COOKIE_TEMP, 2 );
	setprefslong( DSI_SECURITY_ASK_COOKIE_PERM, 2 );
	setprefslong( DSI_SECURITY_NO_SSLCACHE, FALSE );

	setprefslong( DSI_IMG_DOUBLEBUFFER, TRUE );
	setprefslong( DSI_IMG_STOP_ANIMGIF, FALSE );
	setprefslong( DSI_IMG_JPEG_DCT, 1 );

	setprefslong( DSI_NET_KEEPFTP, TRUE );
	setprefslong( DSI_NET_METAREFRESH, FALSE );
	setprefslong( DSI_NET_OFFLINEMODE, FALSE );
	setprefslong( DSI_NET_CHECKIFONLINE, FALSE );

#if USE_JS
	setprefslong( DSI_JS_ENABLE, TRUE );
#endif
	setprefslong( DSI_JS_DEBUG, FALSE );
	setprefslong( DSI_JS_ERRORLOG, FALSE );
	setprefsstr( DSI_JS_LOGFILE, "jserror.log" );

#ifdef NETCONNECT
	setflag( VFLG_USECM, TRUE );
#endif

	maketb( 0, "Back", "Buttons/Devo/_n_back.iff", BFUNC_COMMAND, "GoBackward", "b" );
	maketb( 1, "Forward", "Buttons/Devo/_n_forward.iff", BFUNC_COMMAND, "GoForward", "f" );
	maketb( 2, "Home", "Buttons/Devo/_n_home.iff", BFUNC_COMMAND, "GoHome", "h" );
	maketb( 3, "", "", BFUNC_SEP, "", "" );
	maketb( 4, "Reload", "Buttons/Devo/_n_reload.iff", BFUNC_COMMAND, "LoadURL RELOAD FORCE", "r" );
	maketb( 5, "", "", BFUNC_SEP, "", "" );
	maketb( 6, "Search", "Buttons/Devo/_n_find.iff", BFUNC_COMMAND, "LoadURL URL=search:", "i" );
	maketb( 7, "Print", "Buttons/Devo/_n_print.iff", BFUNC_COMMAND, "Print ASK", "p" );
	maketb( 8, "", "", BFUNC_SEP, "", "" );
	maketb( 9, "Stop", "Buttons/Devo/_n_stop.iff", BFUNC_COMMAND, "Stop", "s" );

	setprefslong( DSI_BUTTON_STYLE_SUNNY, TRUE );
	setprefslong( DSI_BUTTON_STYLE_RAISED, TRUE );
	setprefslong( DSI_BUTTON_STYLE_BORDERLESS, TRUE );
	// Don't  preset, will screw existing toolbars otherwise
	//setprefslong( DSI_BUTTON_NUM, 10 );

	// misc
	setprefslong( DSI_MISC_COOKIEBROWSER_SORT_COLUMN, 0 );
	setprefslong( DSI_MISC_COOKIEBROWSER_SORT_REVERSE, FALSE );

	setflag( VFLG_HIDE_ICON, FALSE );
	setflag( VFLG_USE_ERROR_REQUESTER, TRUE );
#if USE_NET
	setflag( VFLG_HOMEPAGE_AUTOLOAD, TRUE );
#else
	setflag( VFLG_HOMEPAGE_AUTOLOAD, FALSE );
#endif

	setprefslong( DSI_NET_SSL2, TRUE );
	setprefslong( DSI_NET_SSL3, TRUE );
	setprefslong( DSI_NET_TLS1, FALSE );
	setprefslong( DSI_NET_BUGS, TRUE );

	setprefslong( DSI_NAG_FIRSTINSTALLTIME, timev() );

	// spoofing
#ifdef __MORPHOS__
	setprefsstr( DSI_NET_SPOOF_AS_1, "Mozilla/4.6 (compatible; AmigaVoyager; MorphOS)" );
#else
	setprefsstr( DSI_NET_SPOOF_AS_1, "Mozilla/4.6 (compatible; AmigaVoyager; AmigaOS)" );
#endif
	setprefsstr( DSI_NET_SPOOF_AS_1_AN, "AmigaVoyager" );
	setprefsstr( DSI_NET_SPOOF_AS_1_AC, "Mozilla" );
#ifdef __MORPHOS__
	setprefsstr( DSI_NET_SPOOF_AS_1_AV, "4.6 (compatible; AmigaVoyager; MorphOS)" );
#else
	setprefsstr( DSI_NET_SPOOF_AS_1_AV, "4.6 (compatible; AmigaVoyager; AmigaOS)" );
#endif

	setprefsstr( DSI_NET_SPOOF_AS_2, "Mozilla/4.6 (i386; Win9x; D)" );
	setprefsstr( DSI_NET_SPOOF_AS_2_AN, "Netscape" );
	setprefsstr( DSI_NET_SPOOF_AS_2_AC, "Mozilla" );
	setprefsstr( DSI_NET_SPOOF_AS_2_AV, "4.6 (i386; Win9x; D)" );

	setprefsstr( DSI_NET_SPOOF_AS_3, "Mozilla/4.0 (compatible; MSIE 5.0; Win98)" );
	setprefsstr( DSI_NET_SPOOF_AS_3_AN, "Microsoft Internet Explorer" );
	setprefsstr( DSI_NET_SPOOF_AS_3_AC, "Mozilla" );
	setprefsstr( DSI_NET_SPOOF_AS_3_AV, "4.0 (compatible; MSIE 5.0; Win98)" );

	// download
	setprefslong( DSI_NET_DOWNLOAD_TIMEOUT, 900 );
	setprefslong( DSI_NET_DOWNLOAD_RETRIES, 3 );

#if USE_CLOCK
	setflag( VFLG_USE_CLOCK, FALSE );
	setflag( VFLG_CLOCK_SECONDS, FALSE );
#endif /* USE_CLOCK */

	// custom
#ifdef MBX
	setflag( VFLG_SMOOTH_SCROLL, TRUE );
#else
	setflag( VFLG_SMOOTH_SCROLL, FALSE );
#endif /* !MBX */

#if USE_MENUS
	/*
	 * Context menus setup. They probably should be localized but...
	 * Beware of the numbering !
	 */
	/* Page */
	setprefsstr( DSI_CMENUS_PAGE_LABELS, "Back" );
	setprefslong( DSI_CMENUS_PAGE_ACTION, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS, "GoBackward" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 1, "Forward" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 1, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 1, "GoForward" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 2, "Reload" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 2, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 2, "LoadURL RELOAD FORCE" );
	/* --- */
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 3, "" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 3, BFUNC_BAR );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 4, "Load background" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 4, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 4, "LoadBackground" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 5, "Save background" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 5, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 5, "SaveBackground" ); //TOFIX!! NYI

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 6, "Save background as..." );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 6, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 6, "SaveBackground ASK" ); //TOFIX!! NYI
	
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 7, "Use background as wallpaper" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 7, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 7, "SetWallpaper" ); //TOFIX!! NYI
	/* --- */
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 8, "" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 8, BFUNC_BAR );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 9, "Select all" ); //TOFIX!! NYI
	setprefslong( DSI_CMENUS_PAGE_ACTION + 9, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 9, "SelectAll" ); //TOFIX!! NYI
	/* --- */
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 10, "" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 10, BFUNC_BAR );
	/* --- */
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 11, "Open in new window" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 11, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 11, "LoadURL NEW" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 12, "View source..." );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 12, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 12, "OpenSourceView" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 13, "Find..." );
	
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 14, "Print..." );
	
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 15, "Save" );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 15, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 15, "SaveURL" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 16, "Save as" );
	
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 17, "HTML..." );
	setprefslong( DSI_CMENUS_PAGE_DEPTH + 17, 1 );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 17, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 17, "SaveURL ASK" );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 18, "Text..." );
	setprefslong( DSI_CMENUS_PAGE_DEPTH + 18, 1 );

	setprefsstr( DSI_CMENUS_PAGE_LABELS + 19, "Info..." );
	setprefslong( DSI_CMENUS_PAGE_ACTION + 19, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_PAGE_ARGS + 19, "OpenDocInfo" );
	/* --- */
	setprefsstr( DSI_CMENUS_PAGE_LABELS + 20, "Create desktop shortcut" );

	/* Image */
	setprefsstr( DSI_CMENUS_IMAGE_LABELS, "Load image..." );
	
	setprefsstr( DSI_CMENUS_IMAGE_LABELS + 1, "View separately..." );
	setprefslong( DSI_CMENUS_IMAGE_ACTION + 1, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_IMAGE_ARGS + 1, "LoadURL NEW" );

	setprefsstr( DSI_CMENUS_IMAGE_LABELS + 2, "View with WebVision..." );
	
	setprefsstr( DSI_CMENUS_IMAGE_LABELS + 3, "Save image" );
	setprefslong( DSI_CMENUS_IMAGE_ACTION + 3, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_IMAGE_ARGS + 3, "SaveURL" );

	setprefsstr( DSI_CMENUS_IMAGE_LABELS + 4, "Save image as..." );
	setprefslong( DSI_CMENUS_IMAGE_ACTION + 4, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_IMAGE_ARGS + 4, "SaveURL ASK" );
	
	setprefsstr( DSI_CMENUS_IMAGE_LABELS + 5, "Set image as wallpaper" );

	setprefsstr( DSI_CMENUS_IMAGE_LABELS + 6, "Copy URL to Clipboard" );
	setprefslong( DSI_CMENUS_IMAGE_ACTION + 6, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_IMAGE_ARGS + 6, "CopyToClip" );

	/* Links */
	setprefsstr( DSI_CMENUS_LINK_LABELS, "Open..." );
	setprefslong( DSI_CMENUS_LINK_ACTION, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_LINK_ARGS, "LoadURL" );
	
	setprefsstr( DSI_CMENUS_LINK_LABELS + 1, "Open in new window..." );
	setprefslong( DSI_CMENUS_LINK_ACTION + 1, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_LINK_ARGS + 1, "LoadURL NEW" );
	
	setprefsstr( DSI_CMENUS_LINK_LABELS + 2, "Download file" );
	setprefslong( DSI_CMENUS_LINK_ACTION + 2, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_LINK_ARGS + 2, "SaveURL" );

	setprefsstr( DSI_CMENUS_LINK_LABELS + 3, "Download file as..." );
	setprefslong( DSI_CMENUS_LINK_ACTION + 3, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_LINK_ARGS + 3, "SaveURL ASK" );
	
	setprefsstr( DSI_CMENUS_LINK_LABELS + 4, "Copy URL to Clipboard" );
	setprefslong( DSI_CMENUS_IMAGE_ACTION + 4, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_IMAGE_ARGS + 4, "CopyToClip" );

	setprefsstr( DSI_CMENUS_LINK_LABELS + 5, "Add URL to Bookmarks" );
	
	setprefsstr( DSI_CMENUS_LINK_LABELS + 6, "Info..." );
	setprefslong( DSI_CMENUS_LINK_ACTION + 6, BFUNC_COMMAND );
	setprefsstr( DSI_CMENUS_LINK_ARGS + 6, "OpenDocInfo" );


#endif /* USE_MENUS */

	setflag( VFLG_FULLSCREEN, FALSE );

#ifdef VDEBUG
	/* debugging */
	setprefslong( DSI_DEBUG_AUTH, FALSE );
	setprefslong( DSI_DEBUG_CACHE, FALSE );
	setprefslong( DSI_DEBUG_CACHEPRUNE, FALSE );
	setprefslong( DSI_DEBUG_COOKIE, FALSE );
	setprefslong( DSI_DEBUG_DOCINFOWIN, FALSE );
	setprefslong( DSI_DEBUG_DOWNLOADWIN, FALSE );
	setprefslong( DSI_DEBUG_DNS, FALSE );
	setprefslong( DSI_DEBUG_FTP, FALSE );
	setprefslong( DSI_DEBUG_GUI, FALSE );
	setprefslong( DSI_DEBUG_HISTORY, FALSE );
	setprefslong( DSI_DEBUG_HTML, FALSE );
	setprefslong( DSI_DEBUG_HTTP, FALSE );
	setprefslong( DSI_DEBUG_INIT, FALSE );
	setprefslong( DSI_DEBUG_JS, FALSE );
	setprefslong( DSI_DEBUG_MAIL, FALSE );
	setprefslong( DSI_DEBUG_MISC, FALSE );
	setprefslong( DSI_DEBUG_NET, FALSE );
	setprefslong( DSI_DEBUG_PLUGIN, FALSE );
	setprefslong( DSI_DEBUG_REXX, FALSE );
#endif /* VDEBUG */

}

/*
 * Prefs vars to use if the item is accessed often
 */
int gp_underline_links;
int gp_no_referer;
int	gp_dbblit;
int	gp_memcachesize;
int gp_cacheimg;
int gp_noproxy;
int	gp_spoof;
int	gp_maxnetproc;
int	gp_keepftp;
int	gp_ignorehttpmime;
int gp_metarefresh;
int gp_usecm;
int	gp_offlinemode;
int	gp_checkifonline;
int gp_javascript;
int	gp_cache_verify;
int gp_fontface;
int gp_tearoff;
ULONG gp_download_timeout; /* download timeout */
ULONG gp_download_retries; /* download retries */
char gp_languages[ 256 ];
int gp_loadimages;
#ifdef MBX
int gp_loadimages_bg = 1;
#else
int gp_loadimages_bg;
#endif
int gp_image_stop_animgif;
int gp_image_show_alt_text;
int gp_image_show_ph_border;
int gp_smooth_scroll;
int gp_singlewindow;

void create_cache_dir( void )
{
#if USE_DOS && USE_NET
	if( gp_cachedir[ 0 ] && gp_cachesize )
	{
		char *p = strchr( gp_cachedir, 0 ) - 1;
		if( *p != ':' && *p != '/' )
		{
			UnLock( CreateDir( gp_cachedir ) );
			strcpy( p + 1, "/" );
		}
		else
		{
			if( *p == '/' )
			{
				*p = 0;
				UnLock( CreateDir( gp_cachedir ) );
				*p = '/';
			}
		}
	}
#endif
}

char gp_cachedir[ 256 ];

void set_prefs_globals( void )
{
	gp_usecm = getflag( VFLG_USECM );
	gp_fontface = getflag( VFLG_FONTFACE );
	gp_tearoff = getflag( VFLG_TEAROFF );
	gp_image_show_alt_text = getflag( VFLG_IMG_SHOW_ALT_TEXT );
	gp_image_show_ph_border = getflag( VFLG_IMG_SHOW_PH_BORDER );
	gp_maxnetproc = getprefslong( DSI_NET_MAXCON, 8 );
	gp_underline_links = getprefslong( DSI_FLAGS + VFLG_UNDERLINE_LINKS, FALSE );
	gp_memcachesize = getprefslong( DSI_CACHE_MEMSIZE , 0 ) * 1024;
	gp_no_referer = getprefslong( DSI_SECURITY_NO_REFERER, FALSE );
	gp_dbblit = getprefslong( DSI_IMG_DOUBLEBUFFER, TRUE );
	gp_cacheimg = getprefslong( DSI_CACHE_IMAGES, TRUE );
#if USE_JS
#ifndef MBX
	gp_javascript = getprefslong( DSI_JS_ENABLE, TRUE );
#else
	gp_javascript = getprefslong( DSI_JS_ENABLE, 0 );
#endif
#endif
	gp_cache_verify = getprefslong( DSI_CACHE_VERIFY, 0 );
	gp_checkifonline = getprefslong( DSI_NET_CHECKIFONLINE, FALSE );
	strcpy( gp_cachedir, getprefsstr( DSI_CACHE_DIR, "PROGDIR:Cache/" ) );
#if USE_DOS && USE_NET
	gp_cachesize = getprefslong( DSI_CACHE_SIZE, 8192 );
#endif
	strcpy( gp_languages, getprefsstr( DSI_MISC_LANGUAGES, "en, *" ) );
	gp_download_timeout = getprefslong( DSI_NET_DOWNLOAD_TIMEOUT, 900 );
	gp_download_retries = getprefslong( DSI_NET_DOWNLOAD_RETRIES, 3 );
	gp_image_stop_animgif = getprefslong( DSI_IMG_STOP_ANIMGIF, FALSE );
	gp_image_show_alt_text = getflag( VFLG_IMG_SHOW_ALT_TEXT );
	gp_image_show_ph_border = getflag( VFLG_IMG_SHOW_PH_BORDER );
#if USE_STB_NAV
	gp_singlewindow = TRUE;
	gp_smooth_scroll = FALSE;
#else
	#if USE_SINGLEWINDOW
		gp_singlewindow = getflag( VFLG_SINGLEWINDOW );
	#else
		gp_singlewindow = FALSE;
	#endif
	gp_smooth_scroll = getflag( VFLG_SMOOTH_SCROLL );
#endif
}


void preinit_prefs( void )
{
	initprefs();
	set_prefs_globals();
}

int loadprefsfrom( char *filename )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !loadprefs( filename ) )
	{
		D( db_init, bug( "couldn't load the prefs.. setting up globals\n" ) );
		set_prefs_globals();
		create_cache_dir();
#if USE_NET
		proxy_init();
#endif /* USE_NET */
		return( 0 );
	}
	D( db_init, bug( "creating cache..\n" ) );
	create_cache_dir();
	D( db_init, bug( "init proxy..\n" ) );
#if USE_NET
	proxy_init();
#endif /* USE_NET */
	return( -1 );
}

int saveprefsas( char *filename )
{
	if( !saveprefs( filename ) )
	{
		return( 0 );
	}
	return( -1 );
}

// **********************************************
// Cloned prefs

void initprefsclone( void )
{
	NEWLIST( &clonelist );
	clonepool = CreatePool( 0, 2048, 1024 );
}

void freeprefsclone( int writeback )
{
	if( clonepool )
	{
		if( writeback )
		{
			struct prefsnode *n;

			while( n = REMHEAD( &clonelist ) ) 
				setprefs( n->id, n->size, n->data );
		}

		DeletePool( clonepool );
		clonepool = NULL;
	}
}

static __inline struct prefsnode *mkpn_clone( ULONG id, ULONG size, APTR data )
{
	struct prefsnode *n;

	n = AllocPooled( clonepool, sizeof( *n ) + size );
	n->id = id;
	n->size = size;
	if( size && data )
		memcpy( n->data, data, size );

	ADDTAIL( &clonelist, n );

	return( n );
}

void killprefs_clone( ULONG id )
{
	struct prefsnode *n = fpn_clone( id );
	if( n )
	{
		REMOVE( n );
	}
}

void setprefs_clone( ULONG id, ULONG size, APTR data )
{
	killprefs_clone( id );
	mkpn_clone( id, size, data );
}

void setprefsstr_clone( ULONG id, STRPTR data )
{
	killprefs_clone( id );
	mkpn_clone( id, strlen( data ) + 1, data );
}

void setprefslong_clone( ULONG id, ULONG v )
{
	killprefs_clone( id );
	mkpn_clone( id, 4, &v );
}

STRPTR getprefsstr_clone( ULONG id, STRPTR def )
{
	struct prefsnode *n = fpn_clone( id );

	if( !n )
		n = fpn( id );

	if( n )
		return( n->data );
	else
		return( def );
}

APTR getprefs_clone( ULONG id )
{
	struct prefsnode *n = fpn_clone( id );

	if( !n )
		n = fpn( id );

	if( n )
		return( n->data );
	else
		return( NULL );
}

ULONG getprefslong_clone( ULONG id, ULONG def )
{
	struct prefsnode *n = fpn_clone( id );

	if( !n )
		n = fpn( id );

	if( n )
	{
		ULONG *x = (ULONG*)n->data;
		return( *x );
	}
	else
		return( def );
}

void copyprefs_clone( ULONG fromid, ULONG toid )
{
	struct prefsnode *pn, *next;

	for( pn = FIRSTNODE( &prefslist ); next = NEXTNODE( pn ); pn = next )
	{
		if( pn->id >= fromid && pn->id <= toid )
			setprefs_clone( pn->id, pn->size, pn->data );
	}
}

void exchangeprefs_clone( ULONG id1, ULONG id2 )
{
	struct prefsnode *pn1 = fpn_clone( id1 ), *pn2 = fpn_clone( id2 );

	if( !pn1 )
		pn1 = fpn( id1 );
	if( !pn2 )
		pn2 = fpn( id2 );

	if( !pn1 || !pn2 )
		return;

	pn1->id = id2;
	pn2->id = id1;

}

#if USE_DOS
void makecachepath( char *ext, char *to )
{
	strcpy( to, gp_cachedir );
	AddPart( to, ext, 256 );
}

static void setmenu( ULONG menuid, int state )
{
	set( findmenu( menuid ), MUIA_Menuitem_Checked, state );
}
#endif

void cfg_load( char *filename )
{
#if USE_TEAROFF
	extern APTR tearoff_dataspace;
#endif
	
	D( db_init, bug( "loading prefs..\n" ) );

	loadprefsfrom( filename );

#if USE_TEAROFF
	D( db_init, bug( "loading tearoff prefs..\n" ) );
	
	if ( tearoff_dataspace )
		loadtearoff( filename );
#endif

#if USE_MENUS
	/*
	 * Menus
	 */
	setmenu( MENU_SET_LOADIMAGES_BACKGROUNDS, !getflag( VFLG_IGNORE_BACKGROUNDS ) );
	setmenu( MENU_SET_LOADIMAGES_ALL, getflag( VFLG_LOAD_IMAGES ) == 1 );
	setmenu( MENU_SET_LOADIMAGES_NONE, !getflag( VFLG_LOAD_IMAGES ) );
	setmenu( MENU_SET_LOADIMAGES_IMAPS, getflag( VFLG_LOAD_IMAGES ) == 2 );
	setmenu( MENU_SET_IGNOREFRAMES, getflag( VFLG_IGNORE_FRAMES ) );

	setmenu( MENU_SET_NOPROXY, getprefslong( DSI_TEMPNOPROXY, FALSE ) );

	setmenu( MENU_SET_SPOOF_0 + getprefslong( DSI_NET_SPOOF, 0 ), TRUE );

	setmenu( MENU_SET_KEEPFTP, getprefslong( DSI_NET_KEEPFTP, FALSE ) );
	setmenu( MENU_SET_IGNOREMIME, getprefslong( DSI_NET_IGNOREHTTPMIME, FALSE ) );
	setmenu( MENU_SET_METAREFRESH, getprefslong( DSI_NET_METAREFRESH, FALSE ) );

#ifdef VDEBUG
	setmenu( MENU_SET_DEBUG_AUTH, getprefslong( DSI_DEBUG_AUTH, FALSE ) );
	setmenu( MENU_SET_DEBUG_CACHE, getprefslong( DSI_DEBUG_CACHE, FALSE ) );
	setmenu( MENU_SET_DEBUG_CACHEPRUNE, getprefslong( DSI_DEBUG_CACHEPRUNE, FALSE ) );
	setmenu( MENU_SET_DEBUG_COOKIE, getprefslong( DSI_DEBUG_COOKIE, FALSE ) );
	setmenu( MENU_SET_DEBUG_DOCINFOWIN, getprefslong( DSI_DEBUG_DOCINFOWIN, FALSE ) );
	setmenu( MENU_SET_DEBUG_DOWNLOADWIN, getprefslong( DSI_DEBUG_DOWNLOADWIN, FALSE ) );
	setmenu( MENU_SET_DEBUG_DNS, getprefslong( DSI_DEBUG_DNS, FALSE ) );
	setmenu( MENU_SET_DEBUG_FTP, getprefslong( DSI_DEBUG_FTP, FALSE ) );
	setmenu( MENU_SET_DEBUG_GUI, getprefslong( DSI_DEBUG_GUI, FALSE ) );
	setmenu( MENU_SET_DEBUG_HISTORY, getprefslong( DSI_DEBUG_HISTORY, FALSE ) );
	setmenu( MENU_SET_DEBUG_HTML, getprefslong( DSI_DEBUG_HTML, FALSE ) );
	setmenu( MENU_SET_DEBUG_CSS, getprefslong( DSI_DEBUG_CSS, FALSE ) );
	setmenu( MENU_SET_DEBUG_HTTP, getprefslong( DSI_DEBUG_HTTP, FALSE ) );
	setmenu( MENU_SET_DEBUG_INIT, db_init ? TRUE : getprefslong( DSI_DEBUG_INIT, FALSE ) );
	setmenu( MENU_SET_DEBUG_JS, getprefslong( DSI_DEBUG_JS, FALSE ) );
	setmenu( MENU_SET_DEBUG_MAIL, getprefslong( DSI_DEBUG_MAIL, FALSE ) );
	setmenu( MENU_SET_DEBUG_MISC, getprefslong( DSI_DEBUG_MISC, FALSE ) );
	setmenu( MENU_SET_DEBUG_NET, getprefslong( DSI_DEBUG_NET, FALSE ) );
	setmenu( MENU_SET_DEBUG_PLUGIN, getprefslong( DSI_DEBUG_PLUGIN, FALSE ) );
	setmenu( MENU_SET_DEBUG_LEVEL_0 + getprefslong( DSI_DEBUG_LEVEL, 0 ), TRUE );
#endif

	DoMethod( app, MM_App_UpdateSpoofMenu );

	if (menu && findmenu( MENU_SET_OFFLINEMODE ))
	{
		SetAttrs( findmenu( MENU_SET_OFFLINEMODE ),
			MUIA_Menuitem_Checked, getprefslong( DSI_NET_OFFLINEMODE, FALSE ),
			MUIA_Menuitem_Enabled, getprefslong( DSI_NET_CHECKIFONLINE, FALSE ) ? FALSE : TRUE,
			TAG_DONE
		);
	}
#endif

	D( db_init, bug( "reading global cache..\n" ) );
#if USE_NET
	readglobalcache();
#endif /* USE_NET */

#ifndef MBX
//TOFIX!! IMGDECFIX
	D( db_init, bug( "setting imgprefs..\n" ) );
	imgdec_storeprefs();
#endif
}

extern void write_estimated_cachesize( void );

void cfg_save( char *filename )
{
	int flg = 0;
	extern APTR win_cookiebrowser;
	ULONG fullscr;

#if USE_NET
	write_estimated_cachesize();
#endif /* USE_NET */

	if( getmenucheck( MENU_SET_LOADIMAGES_IMAPS ) )
		flg = 2;
	else if( getmenucheck( MENU_SET_LOADIMAGES_ALL ) )
		flg = 1;
	setflag( VFLG_LOAD_IMAGES, flg );

	setflag( VFLG_IGNORE_BACKGROUNDS, getmenucheck( MENU_SET_LOADIMAGES_BACKGROUNDS ) ? 0 : 1 );
	setflag( VFLG_IGNORE_FRAMES, getmenucheck( MENU_SET_IGNOREFRAMES ) ? 1 : 0 );

	//setflag( VFLG_USECM, getmenucheck( MENU_BM_USECM ) ? 1 : 0 );

	setprefslong( DSI_NET_SPOOF, gp_spoof );
	setprefslong( DSI_TEMPNOPROXY, getmenucheck( MENU_SET_NOPROXY ) ? 1 : 0 );
	setprefslong( DSI_NET_KEEPFTP, getmenucheck( MENU_SET_KEEPFTP ) ? 1 : 0 );
	setprefslong( DSI_NET_IGNOREHTTPMIME, getmenucheck( MENU_SET_IGNOREMIME ) ? 1 : 0 );
	setprefslong( DSI_NET_METAREFRESH, getmenucheck( MENU_SET_METAREFRESH ) ? 1 : 0 );
	setprefslong( DSI_NET_OFFLINEMODE, getmenucheck( MENU_SET_OFFLINEMODE ) ? 1 : 0 );

	DoMethod( app, MM_DoLastActiveWin, OM_GET, MA_HTMLWin_FullScreen, &fullscr );
	setflag( VFLG_FULLSCREEN, fullscr );

#ifdef VDEBUG
	setprefslong( DSI_DEBUG_AUTH, db_auth );
	setprefslong( DSI_DEBUG_CACHE, db_cache );
	setprefslong( DSI_DEBUG_CACHEPRUNE, db_cacheprune );
	setprefslong( DSI_DEBUG_COOKIE, db_cookie );
	setprefslong( DSI_DEBUG_DOCINFOWIN, db_docinfowin );
	setprefslong( DSI_DEBUG_DOWNLOADWIN, db_dlwin );
	setprefslong( DSI_DEBUG_DNS, db_dns );
	setprefslong( DSI_DEBUG_FTP, db_ftp );
	setprefslong( DSI_DEBUG_GUI, db_gui );
	setprefslong( DSI_DEBUG_HISTORY, db_history );
	setprefslong( DSI_DEBUG_HTML, db_html );
	setprefslong( DSI_DEBUG_CSS, db_css );
	setprefslong( DSI_DEBUG_HTTP, db_http );
	setprefslong( DSI_DEBUG_INIT, db_init );
	setprefslong( DSI_DEBUG_JS, db_js );
	setprefslong( DSI_DEBUG_MAIL, db_mail );
	setprefslong( DSI_DEBUG_MISC, db_misc );
	setprefslong( DSI_DEBUG_NET, db_net );
	setprefslong( DSI_DEBUG_PLUGIN, db_plugin );
	setprefslong( DSI_DEBUG_REXX, db_rexx );
	setprefslong( DSI_DEBUG_LEVEL, db_level );
#endif

#if USE_NET
#if USE_NLIST
	if( win_cookiebrowser && nlist )
#else
	if( win_cookiebrowser )
#endif
	{
		setprefslong( DSI_MISC_COOKIEBROWSER_SORT_COLUMN, getv(win_cookiebrowser, MA_CookieBrowser_Column ) );
		setprefslong( DSI_MISC_COOKIEBROWSER_SORT_REVERSE, getv(win_cookiebrowser, MA_CookieBrowser_Reverse ) );
	}
#endif /* USE_NET */


	DoMethod( app, MM_DoActiveWin, MM_HTMLWin_ExportTearoff );
#if USE_TEAROFF
	savetearoff( filename );
#endif
	saveprefsas( filename );

}


