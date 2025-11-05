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
** $Id: cookies.c,v 1.36 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "voyager_cat.h"
#include "prefs.h"
#include "authipc.h"
#include "cookies.h"
#include "classes.h"
#include "splashwin.h"
#include "mui_func.h"
#include "time_func.h"
#include "methodstack.h"
#include "cache.h"
#include "malloc.h"

extern APTR win_cookiebrowser;

struct MinList cookielist, cookiereqlist;
struct SignalSemaphore cookiesem;
static int cookies_loaded;

void init_cookies( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &cookielist );
	NEWLIST( &cookiereqlist );
	InitSemaphore( &cookiesem );
}

struct cookiemsg {
	struct Message m;
	int method;
	int quitme;
	int rc;
	char name[ 256 ];
	char data[ 1024 ];
	char server[ 256 ];
	char path[ 256 ];
	char comment[ 256 ];
	char realserver[ 256 ];
	time_t expires;
	int secure;
};

struct Data {
	struct MinNode n;
	APTR me;
	struct cookiemsg *cm;
};

DECNEW
{
	struct Data *data;
	APTR bt_ok, bt_cancel;
	APTR txt_info;
	char text[ 1024 ];
	struct cookiemsg *cm = (APTR)GetTagData( MA_Cookie_CookieMessage, -1, msg->ops_AttrList );

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('C','O','O','K'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, GS( COOKIEWIN_TITLE ),
		WindowContents, VGroup, MUIA_Background, MUII_RequesterBack,
			Child, txt_info = TextObject, TextFrame, MUIA_Background, MUII_TextBack, InnerSpacing( 10, 10 ), End,
			Child, HGroup,
				Child, bt_ok = makebutton( MSG_COOKIEWIN_ALLOW ),
				Child, bt_cancel = makebutton( MSG_COOKIEWIN_REJECT ),
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cookie_Close, FALSE
	);
	DoMethod( bt_ok, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cookie_Close, TRUE
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cookie_Close, FALSE
	);

	data->me = obj;
	data->cm = cm;

	ADDTAIL( &cookiereqlist, data );

	sprintf( text, GS( COOKIEWIN_L1 ), cm->realserver );

	sprintf( strchr( text, 0 ), GS( COOKIEWIN_L5 ), cm->name );
	sprintf( strchr( text, 0 ), GS( COOKIEWIN_L6 ), cm->data );

	if( !cm->server[ 0 ] )
		strcat( text, GS( COOKIEWIN_L2A ) );
	else
		sprintf( strchr( text, 0 ), GS( COOKIEWIN_L2B ), cm->server );

	if( !cm->expires )
		strcat( text, GS( COOKIEWIN_L3A ) );
	else
	{
		sprintf( strchr( text, 0 ), GS( COOKIEWIN_L3B ), date2string( cm->expires ) );
	}
	strcat( text, cm->secure ? GS( COOKIEWIN_L4B ) : GS( COOKIEWIN_L4A ) );

	if( cm->comment[ 0 ] )
		sprintf( strchr( text, 0 ), GS( COOKIEWIN_L7 ), cm->comment );

	set( txt_info, MUIA_Text_Contents, text );

	return( (ULONG)obj );
}


DECMETHOD( Cookie_Close, ULONG )
{
	GETDATA;

	set( obj, MUIA_Window_Open, FALSE );
	REMOVE( data );

	DoMethod( app, MUIM_Application_KillPushMethod, ( ULONG )obj );

	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );

	data->cm->rc = msg[ 1 ];

	Forbid();

	if( !data->cm->quitme )
		ReplyMsg( ( struct Message * )data->cm );

	Permit();

	MUI_DisposeObject( obj );
	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFMETHOD( Cookie_Close )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_cookiewinclass( void )
{
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "CookieWinClass";
#endif

	return( TRUE );
}

void delete_cookiewinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getcookiewin( void )
{
	return( mcc->mcc_Class );
}

void cookie_process( struct cookiemsg *cm )
{
	APTR o = NewObject( getcookiewin(), NULL,
		MA_Cookie_CookieMessage, cm,
		TAG_DONE
	);
	if( !o )
	{
		ReplyMsg( ( struct Message * )cm );
		return;
	}
	DoMethod( app, OM_ADDMEMBER, ( ULONG )o );
	set( o, MUIA_Window_Open, TRUE );
}

static int askcookie( struct cookiemsg *cm )
{
	struct MsgPort *replyport = CreateMsgPort();

	cm->method = 1;
	cm->m.mn_ReplyPort = replyport;

	PutMsg( authport, ( struct Message * )cm );

	for(;;)
	{
		if( GetMsg( replyport ) )
			break;
		if( Wait( ( 1L<<replyport->mp_SigBit ) | SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C )
		{
			cm->quitme = TRUE;
			Signal( authport->mp_SigTask, authportsigmask );
			break;
		}
	}

	DeleteMsgPort( replyport );

	return( cm->rc );
}

void cookie_set( char *line, char *realhost, char *realpath )
{
	struct cookiemsg cm;
	char *p;
	int v;
	struct cookie *n, *cmp, *cmpn;
	int removeflag = FALSE;

	if( !realpath )
		realpath = "";

	memset( &cm, 0, sizeof( cm ) );

	D( db_cookie, bug( "SET %s\n", line ));

	Forbid();
	for( p = strtok( line, ";" ); p; p = strtok( NULL, ";" ) )
	{
		p = stpblk( p );
		if( !strnicmp( p, "path=", 5 ) )
		{
			stccpy( cm.path, stpblk( p + 5 ), sizeof( cm.path ) );
		}
		else if( !strnicmp( p, "comment=", 8 ) )
		{
			stccpy( cm.comment, stpblk( p + 8 ), sizeof( cm.comment ) );
		}
		else if( !strnicmp( p, "domain=", 7 ) )
		{
			if( strchr( p + 7, '.' ) )
				stccpy( cm.server, stpblk( p + 7 ), sizeof( cm.server ) );
		}
		else if( !strnicmp( p, "expires=", 8 ) )
		{
			D( db_cookie, bug("got expires\n"));
			if( !cm.expires )
				cm.expires = convertrfcdate( stpblk( p + 8 ) );
		}
		else if( !strnicmp( p, "max-age=", 8 ) )
		{
			int v = atoi( p + 8 );
			D( db_cookie, bug("got maxage\n"));
			if( !v )
				removeflag = TRUE;
			else
				cm.expires = timev() + v;
		}
		else if( !strnicmp( p, "secure", 6 ) )
			cm.secure = TRUE;
		else if( !cm.name[ 0 ] )
		{
			char *p2 = strchr( p, '=' );
			if( p2 )
			{
				*p2++ = 0;
				stccpy( cm.name, p, sizeof( cm.name ) );
				stccpy( cm.data, p2, sizeof( cm.data ) );
			}
		}
	}
	Permit();

	if( removeflag )
	{
		ObtainSemaphore( &cookiesem );

		for( cmp = FIRSTNODE( &cookielist ); cmpn = NEXTNODE( cmp ); cmp = cmpn )
		{
			if( !strcmp( cm.name, cmp->name ) && !strcmp( cm.server, cmp->server ) )
			{
				if( win_cookiebrowser )
					pushmethod( win_cookiebrowser, 2, MM_CookieBrowser_Delete, cmp );
				
				REMOVE( cmp );
				free( cmp->name );
				free( cmp->data );
				free( cmp->server );
				free( cmp->path );
				free( cmp );
			}
		}

		ReleaseSemaphore( &cookiesem );

		return;
	}

	// ok, cookiemsg has all the relevant data
	v = getprefslong( cm.expires ? DSI_SECURITY_ASK_COOKIE_PERM : DSI_SECURITY_ASK_COOKIE_TEMP, 1 );
	if( !v )
		return;
	if( v == 1 )
	{
		stccpy( cm.realserver, realhost, sizeof( cm.realserver ) );
		v = askcookie( &cm );
	}
	if( !v )
		return;

	if( !cm.server[ 0 ] )
		stccpy( cm.server, realhost, sizeof( cm.server ) );

	//if( !cm.path[ 0 ] )
	//  stccpy( cm.path, realpath, sizeof( cm.path ) );
	if( !cm.path[ 0 ] )
		strcpy( cm.path, "/" );

	if( n = malloc( sizeof( *n ) ) )
	{
		n->name = strdup( cm.name );
		n->data = strdup( cm.data );
		n->server = strdup( cm.server );
		n->path = strdup( cm.path );
		n->expires = cm.expires;
		n->secure = cm.secure;
		n->age = timev();
		n->n.ln_Pri = strlen( cm.path ) - 128;

		ObtainSemaphore( &cookiesem );

		for( cmp = FIRSTNODE( &cookielist ); cmpn = NEXTNODE( cmp ); cmp = cmpn )
		{
			if( !strcmp( n->name, cmp->name ) && !strcmp( n->server, cmp->server ) )
			{
				if( win_cookiebrowser )
					pushmethod( win_cookiebrowser, 2, MM_CookieBrowser_Delete, cmp );
 
				REMOVE( cmp );
				free( cmp->name );
				free( cmp->data );
				free( cmp->server );
				free( cmp->path );
				free( cmp );
			}
		}

		ENQUEUE( &cookielist, n );

		if( win_cookiebrowser )
			pushmethod( win_cookiebrowser, 2, MM_CookieBrowser_Add, n );

		ReleaseSemaphore( &cookiesem );

	}
}

void cookie_checkclose( void )
{
	struct Data *data, *nd;

	for( data = FIRSTNODE( &cookiereqlist ); nd = NEXTNODE( data ); data = nd )
	{
		if( data->cm->quitme )
		{
			DoMethod( app, MUIM_Application_PushMethod, ( ULONG )data->me, 2, MM_Cookie_Close, FALSE );
		}
	}
}

void cookie_get( char *server, char *path, char *to, int issecure )
{
	int cnt = 0;
	struct cookie *n;
	int slen = strlen( server );
	time_t now = timev();

	if( !path )
		path = "";

	D( db_cookie, bug( "myserver = %s, mypath = %s\n", server, path ));

	ObtainSemaphoreShared( &cookiesem );
	for( n = FIRSTNODE( &cookielist ); NEXTNODE( n ); n = NEXTNODE( n ) )
	{
		int l = strlen( n->server );
		//D( db_cookie, bug( "checking cookie server %s\n", n->server ));
		if( slen >= l && !stricmp( &server[ slen - l ], n->server ) )
		{
			char *cmp = n->path;

			D( db_cookie, bug( "found cookie server %s\n", n->server ));

			if( *cmp == '/' )
				cmp++;

			D( db_cookie, bug( "checking cookie path %s\n", n->path ));
			if( !*cmp || !strnicmp( cmp, path, strlen( cmp ) ) ) 
			{
				if( !n->secure || issecure )
				{
					if( n->expires > now || !n->expires )
					{
						to = strchr( to, 0 );
						if( !cnt++ )
						{
							strcpy( to, "Cookie:" );
							to += 7;
						}
						else
							*to++ = ';';
						
						D( db_cookie, bug( "found cookie %s=%s\n", n->name, n->data ));
						
						sprintf( to, " %s=%s", n->name, n->data );
					}
				}
			}
		}
	}
	if( cnt )
		strcat( to, "\r\n" );
	ReleaseSemaphore( &cookiesem );
}

void load_cookies( void )
{
#if USE_DOS
	char path[ 256 ];
	struct cookie *n;
	struct AsyncFile *f;
	char buffer[ 2048 ];

	D( db_cookie, bug( "loading cookies\n" ) );

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_COOKIES ) );
	}
#endif

	makecachepath( "Cookies.1", path );
	f = OpenAsync( path, MODE_READ, 4096 );
	if( f )
	{
		ULONG cerr;

		D( db_cookie, bug( "opened cookie file <%s>\n", path ) );
		for(;;)
		{
			cerr = 0;

			if( !FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) ) /* XXX: make sure nothing can store an empty cookiename */
				break;

			n = malloc( sizeof( *n ) ); /* XXX: yeah.. small memleak.. this cookie stuff should be rewritten from scratch */
			if( !n )
				break;

			n->name = strdup( buffer );     

			/*
			 * Sigh. I saw ann.lu sending cookies with empty data. We have to handle
			 * that stupid case now.
			 */
			FGetsAsyncNoLF( f, buffer, sizeof( buffer ) );
			n->data = strdup( buffer );
			
			if( !FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
				break;
			n->server = strdup( buffer );       

			if( !FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
				break;
			n->path = strdup( buffer );

			if( !FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
				break;
			n->expires = atoi( buffer );

			if( !FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
				break;
			n->age = atoi( buffer );

			if( !FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
				break;
			n->secure = atoi( buffer );

			n->n.ln_Pri = strlen( n->path ) - 128;
			ENQUEUE( &cookielist, n );
			D( db_cookie, bug( "added cookie, server: <%s>, name: <%s>\n", n->server, n->name ) );
		}

		CloseAsync( f );
	}
	else
	{
		D( db_cookie, bug( "Couldn't open cookie file <%s>\n", path ) ); /* XXX: maybe complain in some cases */
	}
#endif /* USE_DOS */

	cookies_loaded = TRUE;
}

void save_cookies( void )
{
#if USE_DOS
	char path[ 256 ];
	char buffer[ 2048 ];
	struct cookie *n;
	struct AsyncFile *f;

	if( !cookies_loaded )
	{
		return;
	}

	D( db_cookie, bug( "saving cookies to disk..\n" ) );

	makecachepath( "Cookies.1", path ); 

	f = OpenAsync( path, MODE_WRITE, 4096 );
	if( f )
	{
		time_t now = timev();
		for( n = FIRSTNODE( &cookielist ); NEXTNODE( n ); n = NEXTNODE( n ) )
		{
			if( n->expires > now )
			{
				sprintf( buffer, "%s\n%s\n%s\n%s\n%lu\n%lu\n%lu\n",
					n->name, n->data, n->server, n->path,
					n->expires, n->age, ( long unsigned int )n->secure
				);
				D( db_cookie, bug( "cookie server: <%s>, name: <%s> saved\n", n->server, n->name ) );
				WriteAsync( f, buffer, strlen( buffer ) ); /* XXX: we should check the return value */
			}
			else
			{
				D( db_cookie, bug( "cookie server: <%s>, name: <%s> expired\n", n->server, n->name ) );
			}
		}
		CloseAsync( f );
	}
	else
	{
		D( db_cookie, bug( "Couldn't open cookie file <%s>\n", path ) ); /* XXX: maybe we should report to the user */
	}

	D( db_cookie, bug( "finished saving cookies\n" ) );

#endif /* USE_DOS */
}

#endif /* USE_NET */
