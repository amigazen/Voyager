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
** $Id: network.c,v 1.292 2003/12/20 14:28:38 zapek Exp $
**
*/

#include "voyager.h"

/* system */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <proto/exec.h>
#endif

/* private */
#include "copyright.h"
#include "classes.h"
#include "voyager_cat.h"
#include "authipc.h"
#include "cookies.h"
#include "certs.h"
#include "urlparser.h"
#include "methodstack.h"
#include "prefs.h"
#include "netinfo.h"
#include "time_func.h"
#include "file.h"
#include "mime.h"
#include "keyparse.h"
#include "mui_func.h"
#include "autoproxy.h"
#include <proto/vimgdecode.h>
#include "dos_func.h"
#include "ledclass.h"
#include "sur_gauge.h"

#ifdef MBX
#include "modules/net/establish.h"
#include "net_lib_calls.h"
#endif

#if USE_PLUGINS
#include "plugins.h"
#endif

#if USE_EXECUTIVE
#include "executive_protos.h"
#endif

#ifndef MBX
#include <dos/exall.h>
#include <proto/v_about.h>
#else
#include "v_about_mbx.h"
#endif

#include <stdarg.h>
#include <limits.h>

#include "dns.h"
#include "cache.h"
#include "http.h"
#include "ftp.h"
#include "network.h"
#include "errorwin.h"
#include "htmlclasses.h"

time_t now;

/* defines */

#define MAXNETCONN 32
#define PARK_FTP_PASSWORD_SIZE 240
#define PARK_HOSTNAME_SIZE 252

int netopen;

// functions dealing with header manipulation


int stripentity( char *bf, char *name, char *to, int tolen )
{
	int bl = strlen( bf ), nl = strlen( name );
	char *p;

	while( bl >= nl )
	{
		if( !strnicmp( bf, name, nl ) && ( bf[ nl ] == '=' ) )
		{
			bf += nl + 1;

			if( *bf == '"' )
			{
				bf++;
				p = bf;
				while( *p && *p != '"' && --tolen > 1 )
				{
					*to++ = *p++;
				}
				*to = 0;
				return( TRUE );
			}

			p = bf;
			while( *p && !isspace( *p ) && *p != ';' && --tolen > 1 )
			{
				*to++ = *p++;
			}
			*to = 0;
			return( TRUE );
		}
		bl--;
		bf++;
	}
	return( FALSE );
}


#define BFSIZE 256
void splitrfcname( char *adr, char *to_real, char *to_addr )
{
	char remain[ BFSIZE ], *rd = remain;
	char *p;
	char *tor = to_real;
	char *toa = to_addr;
	int isquote = FALSE, isangle = FALSE, iscomment = FALSE;
	int l;

	p = adr;

	// first run

	for( p = adr; *p; p++ )
	{
		if( *p == '"' )
		{
			isquote = !isquote;
			continue;
		}
		else if( !isquote )
		{
			if( *p == '<' )
			{
				isangle = TRUE;
				continue;
			}
			else if( *p == '>' )
			{
				isangle = FALSE;
				continue;
			}
			else if( *p == '(' )
			{
				iscomment = TRUE;
				continue;
			}
			else if( *p == ')' )
			{
				iscomment = FALSE;
				continue;
			}
		}
		if( iscomment )
		{
			if( tor - to_real < BFSIZE - 1 )
				*tor++ = *p;
		}
		else if( isangle )
		{
			if( toa - to_addr < BFSIZE - 1 )
				*toa++ = *p;
		}
		else
		{
			if( rd - remain < BFSIZE - 1 )
				*rd++ = *p;
		}
	}
	*rd = 0;
	*tor = 0;
	*toa = 0;

	if( toa == to_addr )
		strcpy( to_addr, remain );
	else if( tor == to_real )
		strcpy( to_real, remain );

	if( isspace( *to_addr ) )
		strcpy( to_addr, stpblk( to_addr ) );
	if( isspace( *to_real ) )
		strcpy( to_real, stpblk( to_real ) );

	while( ( l = strlen( to_addr ) ) && isspace( to_addr[ l - 1 ] ) )
		to_addr[ l - 1 ] = 0;

	while( ( l = strlen( to_real ) ) && isspace( to_real[ l - 1 ] ) )
		to_real[ l - 1 ] = 0;
}


volatile struct Process *netproc;

/* Network memory handling */

static APTR netpool;
struct SignalSemaphore netpoolsem;

APTR nalloc( int size )
{
	ULONG *o;
	ObtainSemaphore( &netpoolsem );
	o = AllocPooled( netpool, size + 4 );
	ReleaseSemaphore( &netpoolsem );
	if( o )
		*o++ = size + 4;
	return( o );
}

static void nfree( APTR o )
{
	ULONG *op = o;

	ObtainSemaphore( &netpoolsem );
	FreePooled( netpool, op - 1, op[ -1 ] );
	ReleaseSemaphore( &netpoolsem );
}

UWORD nethandler_die;

APTR unalloc( struct unode *un, int size )
{
	APTR o = AllocPooled( un->pool, size );
	return( o );
}

STRPTR unstrdup( struct unode *un, STRPTR str )
{
	STRPTR s = unalloc( un, strlen( str ) + 1 );
	if( s )
		strcpy( s, str );
	return( s );
}

static void __inline unfree( struct unode *un, APTR o, int len )
{
	FreePooled( un->pool, o, len );
}

static struct header *unfindheader( struct unode *un, char *header )
{
	struct header *h;

	for( h = FIRSTNODE( &un->headers ); NEXTNODE( h ); h = NEXTNODE( h ) )
	{
		if( !stricmp( h->header, header ) )
			return( h );
	}
	return( NULL );
}

char *ungetheaderdata( struct unode *un, char *header )
{
	struct header *h = unfindheader( un, header );
	if( h )
		return( h->data );
	else
		return( NULL );
}

struct List ulist;
struct List sslcertlist;
static struct SignalSemaphore unlistsem;
static struct MsgPort *netport;
static int activelinks;
static UBYTE ledalloc[ MAXNETCONN ];

// Global Proxy Authentication data
char proxyauth[ 128 ];
int useproxyauth;

#ifdef MBX
#define MAXNETPROC 16
#else
#define MAXNETPROC 32
#endif

#if USE_CONNECT_PROC
static struct Process *connectproc[ MAXNETPROC ];
static struct MsgPort *connectport[ MAXNETPROC ];
static int connecthandler_die;
struct MsgPort *connectreply;

static void SAVEDS connecthandler( void )
{
	int mytasknum;
	char *p;
	ULONG psig;
	struct connectmsg *connectmsg;

	p = strchr( FindTask( 0 )->tc_Node.ln_Name, 0 ) - 2;
	mytasknum = atoi( p ) - 1;

	connectport[ mytasknum ] = CreateMsgPort();
	if( !connectport[ mytasknum ] )
	{
		connectproc[ mytasknum ] = NULL;
		return;
	}
	psig = 1L<<connectport[ mytasknum ]->mp_SigBit;

	while( !connecthandler_die )
	{
		if( Wait( SIGBREAKF_CTRL_C | psig ) & SIGBREAKF_CTRL_C )
		{
			break;
		}

		while((connectmsg = (struct connectmsg*)GetMsg( connectport[ mytasknum ] )))
		{
			connectmsg->rc = connect( connectmsg->sock, (struct sockaddr*)&connectmsg->addr, sizeof( connectmsg->addr ) );
			connectmsg->done = TRUE;
			ReplyMsg( connectmsg );
		}
	}
	DeleteMsgPort( connectport[ mytasknum ] );
	connectproc[ mytasknum ] = NULL;
}


#endif


#if USE_NET
#ifndef MBX
struct Library *SocketBase;
#if USE_SSL
struct Library *VSSLBase;
#endif /* USE_SSL */
#if USE_MIAMI
struct Library *MiamiBase;
#if USE_SSL
struct Library *MiamiSSLBase;
#endif /* USE_SSL */
#endif /* USE_MIAMI */
#else /* !MBX */
NETBASE;
#endif /* MBX */

#if USE_SSL
APTR ssl_ctx;
#endif /* USE_SSL */
#endif /* USE_NET */

static int opennet( void )
{
	if( !netopen )
	{
		setup_useragent();
#if USE_NET
#ifndef MBX
#ifndef NETCONNECT
		SocketBase = OpenLibrary( "bsdsocket.library", 3 );
#else /* NETCONNECT */
		SocketBase = NCL_OpenSocket();
#endif /* NETCONNECT */
		if( SocketBase )
		{
			netopen = 1;
#if USE_MIAMI
			MiamiBase = OpenLibrary( "miami.library", 5 );
#endif /* USE_MIAMI */
		}
#else /* !MBX */
		NetBase = (NetData_p) OpenModule( NETNAME, NETVERSION );
		if ( NetBase )
		{
			Establish(ESTTAG_ACTION, EST_ACTION_LINK, ESTTAG_SYNCHRONOUS, TRUE, TAG_END);
			netopen = 1;
		}
#endif /* MBX */

#endif /* USE_NET */
	}
	return( netopen );
}

#if USE_SSL && USE_NET

int openssl( void )
{
	if( ssl_ctx )
		return( TRUE );

#ifndef MBX
	VSSLBase = OpenLibrary( "PROGDIR:Plugins/voyager_ssl.vlib", 8 );
	if( !VSSLBase )
	{
#if USE_MIAMI
		if( MiamiBase && MiamiBase->lib_Version >= 7 )
			MiamiSSLBase = MiamiOpenSSL( 0 );
		if( !MiamiSSLBase )
			return( FALSE );
		ssl_ctx = SSL_CTX_new();
		if( !ssl_ctx )
#endif /* USE_MIAMI */
			return( FALSE );
	}
	else
#endif
	{
		int opts = 0;

		if( !getprefslong( DSI_NET_SSL2, TRUE ) )
			opts |= SSL_OP_NO_SSLv2;
		if( !getprefslong( DSI_NET_SSL3, TRUE ) )
			opts |= SSL_OP_NO_SSLv3;
		if( !getprefslong( DSI_NET_TLS1, FALSE ) )
			opts |= SSL_OP_NO_TLSv1;
		if( getprefslong( DSI_NET_BUGS, TRUE ) )
			opts |= SSL_OP_ALL;

		VSSL_SetDefaultOptions( opts );

#ifndef MBX
		VSSL_SetTCPMode( netopen == 2, SocketBase ); //TOFIX!!
		DL( DEBUG_INFO, db_net, bug( "SSL: tcpmode %ld, libbase 0x%lx, sslbase 0x%lx\n", netopen == 2, SocketBase, VSSLBase ));
#endif
		ssl_ctx = VSSL_Create_CTX();
		DL( DEBUG_INFO, db_net, bug( "SSL: got ctx 0x%lx\n", ssl_ctx ) );
		if( !ssl_ctx )
		{
			return( FALSE );
		}
		//VSSL_AddCertDir( ssl_ctx, "PROGDIR:Certificates" );
	}

	return( TRUE );
}

void closessl( void )
{
	if( ssl_ctx )
	{
#ifndef MBX
		if( VSSLBase )
#endif
			VSSL_Free_CTX( ssl_ctx );
#if USE_MIAMI
		else
			SSL_CTX_free( ssl_ctx );
#endif
		ssl_ctx = NULL;
	}
#ifndef MBX
	CloseLibrary( VSSLBase );
	VSSLBase = NULL;
#endif
#if USE_MIAMI
	CloseLibrary( MiamiSSLBase );
	MiamiSSLBase = NULL;
#endif /* USE_MIAMI */
}
#endif /* USE_SSL && USE_NET */

// clear all network stuff from unode
void un_netclose( struct unode *un )
{
#if USE_NET
#if USE_SSL
	if( un->sslh )
	{
#ifndef MBX
		if( VSSLBase )
#endif
			VSSL_Close( un->sslh );
#if USE_MIAMI
		else
			SSL_free( un->sslh );
#endif
		un->sslh = NULL;
	}
#endif /* USE_SSL */

	un->sockstate = 0;
	if( un->sock >= 0 )
	{
		CloseSocket( un->sock );

		activelinks--;
		un->sock = -1;
	}
	if( un->sock_pasv >= 0 )
	{
		CloseSocket( un->sock_pasv );

		un->sock_pasv = -1;
	}

	un->protocolstate = 0;
	un->lineread = 0;

	if( un->ledobjnum >= 0 )
	{
		netinfo_clear( un->ledobjnum );
		sur_led( un, 0 );
		ledalloc[ un->ledobjnum ] = FALSE;
		un->ledobjnum = -1;
	}

	DL( DEBUG_CHATTY, db_net, bug( "unode 0x%lx closed its network\n", un ) );

#endif /* USE_NET */
}

#if USE_NET
/*
 * Parking support stuff
 */
static int parked_sockets[ MAXNETCONN ];  /* socket descriptor (un->sock) */
static int parked_leds[ MAXNETCONN ];     /* ledobject number (un->ledobjnum) */
static ULONG parked_ips[ MAXNETCONN ];    /* IP number (dnscachenode->ip) */
static ULONG parked_ports[ MAXNETCONN ];  /* port (un->port) */
static UBYTE parked_type[ MAXNETCONN ];   /* type (http, ftp, file, internal) */
static char *parked_pwds[ MAXNETCONN ];   /* ftp password (un->ftp_pwd) */
static char *parked_hosts[ MAXNETCONN ];  /* hostname (dnscachenode->name) */
static time_t parked_times[ MAXNETCONN ]; /* time when it was the parked */
static int parkcount;                     /* number of parked stuffs */


/*
 * Parks a connection
 */
void un_netpark( struct unode *un )
{
	int c;

	// "park" a 1.1 connection

	for( c = 0; c < MAXNETCONN; c++ )
	{
		if( !parked_ips[ c ] )
		{
			parked_sockets[ c ] = un->sock;
			un->sock = -1; /* socket no longer valid */
			parked_leds[ c ] = un->ledobjnum;
			netinfo_parked( un->ledobjnum );
			sur_led( un, 5 );
			parked_ips[ c ] = *un->dcn->ip;
			parked_ports[ c ] = un->port;
			parked_type[ c ] = un->retr_mode;
			parked_times[ c ] = timed();
			if( un->ftp_pwd )
				stccpy( parked_pwds[ c ], un->ftp_pwd, PARK_FTP_PASSWORD_SIZE );
			else
				parked_pwds[ c ][ 0 ] = 0;
			stccpy( parked_hosts[ c ], un->dcn->name, PARK_HOSTNAME_SIZE );

			DL( DEBUG_INFO, db_net, bug( "PARKing connection to %lx/%ld (led %ld) pwd %s\n", parked_ips[ c ], parked_ports[ c ], parked_leds[ c ], parked_pwds[ c ] ));
			un->ledobjnum = -1;
			un->sockstate = 0;
			parkcount++;
			return;
		}
	}
	// hm, strange, no slot
	DL( DEBUG_ERROR, db_net, bug( "PARK: CRITICAL ERROR - can't find a slot?!?\n" ));
	un_netclose( un );
}

/*
 * Unparks a connection and return the socket and ledobjnum
 */
static int un_netunpark( struct unode *un, ULONG ip, int port, int *sock, int *ledobj, int type )
{
	int c;

	for( c = 0; c < MAXNETCONN; c++ )
	{
		if( parked_ips[ c ] == ip && !strcmp( parked_hosts[ c ], un->dcn->name ) && parked_ports[ c ] == port && parked_type[ c ] == type )
		{
			parked_ips[ c ] = 0;
			*sock = parked_sockets[ c ];
			*ledobj = parked_leds[ c ];
			if( type == 1 )
				un->ftp_pwd = unstrdup( un, parked_pwds[ c ] );
			parkcount--;
			DL( DEBUG_CHATTY, db_net, bug( "unPARKing connection number %ld (%lx/%ld)\n", c, ip, parked_ports[ c ] ) );
			return( TRUE );
		}
	}

	return( FALSE );
}

/*
 * Finds the oldest parked connection and free it
 */
static int un_netfreeparked( void )
{
	time_t mintime = INT_MAX;
	int minindex = -1;
	int c;

	for( c = 0; c < MAXNETCONN; c++ )
	{
		if( parked_ips[ c ] )
		{
			if( parked_times[ c ] < mintime )
			{
				mintime = parked_times[ c ];
				minindex = c;
			}
		}
	}

	if( minindex < 0 )
	{
		DL( DEBUG_CHATTY, db_net, bug( "no parked connection to free\n" ) );
		return( FALSE );
	}

	parked_ips[ minindex ] = 0;
	CloseSocket( parked_sockets[ minindex ] );

	setled( parked_leds[ minindex ], 0 );
	ledalloc[ parked_leds[ minindex ] ] = FALSE;
	netinfo_clear( parked_leds[ minindex ] );
	activelinks--;
	parkcount--;

	DL( DEBUG_CHATTY, db_net, bug( "park freed ok\n" ) );

	return( TRUE );
}
#else
void un_netpark( struct unode *un )
{
}
#endif /* USE_NET */

static void processunode( struct unode *un );

/*
 * Adds a nstream into the nethandler, creates the unode
 */
static void addstream( struct nstream *ns )
{
	struct unode *un;
	char *p;
	APTR pool;

	/*
	 * Scan if there's an existing unode
	 */
	for( un = FIRSTNODE( &ulist ); NEXTNODE( un ); un = NEXTNODE( un ) )
	{
		if( !strcmp( ns->url, un->url ) )
		{
			DL( DEBUG_CHATTY, db_net, bug( "matches existing unode\n" ) );
			if( ( ns->flags & NOF_RELOAD ) /*|| ns->offset < un->offset*/ ) /* TOFIX: that won't work :( .. see un_dowaiting() */
			{
				/*
				 * Upon reload, drop previous node and, hm, reload :)
				 * Upon different offset, we need to refetch the whole
				 * unode as well. TOFIX: it should be possible to optimize
				 * that but who cares..
				 */
				DL( DEBUG_CHATTY, db_net, bug( "reloading\n" ) );
				un->keepinmem = FALSE;
				break;
			}
			ADDHEAD( &un->clients, ns );
			ns->un = un;
			DL( DEBUG_CHATTY, db_net, bug( "adding client to existing unode\n" ) );
			processunode( un );
			return;
		}
	}

	/*
	 * Create an unode from scratch then
	 */
	DL( DEBUG_CHATTY, db_net, bug( "creating from scratch\n" ) );

	pool = CreatePool( 0, 2048, 1024 );
	if( !pool )
		return;
	un = AllocPooled( pool, sizeof( *un ) );
	if( !un )
		return;
	memset( un, 0, sizeof( *un ) );

	un->pool = pool;

	un->ledobjnum = -1; // no LED


	un->url = unstrdup( un, ns->url );
	un->urlcopy = unstrdup( un, ns->url );
	un->urlcopy2 = unstrdup( un, ns->url );

	un->reload = ns->flags & NOF_RELOAD; /* TOFIX: maybe put flags in un as well */
	un->timeout = ns->timeout;

	if( ns->referer && !gp_no_referer )
	{
		un->referer = unstrdup( un, ns->referer );
		p = strrchr( un->referer, '?' );
		if( p )
		{
			if( p[ 1 ] == '{' )
			{
				if( strstr( p, "}¿" ) )
					*p = 0;
			}
		}
	}

	// filter postid from URL
	strcpy( un->urlcopy2, un->url );
	p = strrchr( un->urlcopy2, '?' );
	if( p )
	{
		if( p[ 1 ] == '{' )
		{
			char *db = p + 2;

			p = db;
			while( isdigit( *p ) )
				p++;
			if( !strcmp( p, "}¿" ) )
			{
				un->postid = atoi( db );
				db[ -2 ] = 0;
			}
		}
	}

	un->sock = -1;
	un->sock_pasv = -1;

	ns->un = un;

	NEWLIST( &un->headers );

	NEWLIST( &un->clients );
	ADDTAIL( &un->clients, ns );

	NEWLIST( &un->replybufferlist );

	ADDTAIL( &ulist, un );

	DL( DEBUG_CHATTY, db_net, bug( "stream added\n" ) );

	// kick off
	processunode( un );
}

/*
 * Extend the memory cache
 */
int allocdata( struct unode *un, int l )
{
	APTR newbuff;
	int oldmembuffsize = un->membuffsize;

	newbuff = unalloc( un, l );
	if( !newbuff )
	{
		// we're quite out of memory, buddy...
		return( -1 );
	}
	if( un->membuffptr )
		memcpy( newbuff, un->membuff, un->membuffsize );

	if( un->membuff )
		unfree( un, un->membuff, oldmembuffsize );

	un->membuff = newbuff;
	un->membuffsize = l;

	return( 0 );
}

void pushreplybuffer( struct unode *un, char *data )
{
	struct rbline *rb = unalloc( un, sizeof( *rb ) );
	if( rb )
	{
		if( data )
			rb->data = unstrdup( un, data );
		else
			rb->data = NULL;

		ADDTAIL( &un->replybufferlist, rb );
	}
}

void un_closedestfiles( struct unode *un )
{
#if USE_DOS
	struct nstream *ns;

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->tofile )
		{
			CloseAsync( ns->tofile );
			setcomment( ns->filename, ns->url );
			ns->tofile = NULL;
			ns->destmode = -1;
		}
	}
#endif
}

/* process incoming data */
void pushdata( struct unode *un, char *data, int l )
{
	if( l < 0 )
	{
		DL( DEBUG_ERROR, db_net, bug( "pushdata(%s) called with negative size!\n", un->url, l ) );
		return;
	}

	DL( DEBUG_CHATTY, db_net, bug( "pushdata (%s) -> %lx,%ld\n", un->url, data, l ) );

#if USE_DOS
	if( !un->tomem )
	{
		struct nstream *ns;
		
		un->docptr += l;

		for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) ) /* downloading a file, write it to disk */
		{
			if( ns->tofile )
			{
				if( un->offset < ns->offset )
				{
					/*
					 * We are waiting for the unode offset to reach us.
					 */
					if( un->docptr + un->offset > ns->offset )
					{
						if( ns->offset - ( un->docptr + un->offset ) < l )
						{
							/*
							 * It just did.. write the right part.
							 */
							WriteAsync( ns->tofile, data - ( ns->offset - ( un->docptr + un->offset ) ), l - ( ns->offset - ( un->docptr + un->offset ) ) );
						}
						else
						{
							/*
							 * It already did from the last call.
							 */
							WriteAsync( ns->tofile, data, l );
						}
					}
				}
				else
				{
					/*
					 * No resume.. write normally.
					 */
					WriteAsync( ns->tofile, data, l );
				}
			}
		}

#if USE_NET
		if( un->ledobjnum >= 0 ) /* TOFIX: hm, maybe fix that */
			netinfo_setprogress( un->ledobjnum, l );
#endif /* USE_NET */

		return;
	}
#endif

	ObtainSemaphore( &netpoolsem );

	if( un->membuffptr + l >= un->membuffsize ) /* extend the memory cache */
	{
		int newsize = un->membuffsize;

		if( !newsize )
			newsize = max( l + 1, 4096 );
		else
			newsize += max( l + 1, ( newsize / 2 ) * 3 );

		if( allocdata( un, newsize ) )
			return;
	}
	((char*)un->membuff)[ un->membuffptr + l ] = 0;
	memcpy( (char*)un->membuff + un->membuffptr, data, l );
	un->membuffptr += l;

	un->docptr = un->membuffptr;

#if USE_NET
	if( un->ledobjnum >= 0 )
		netinfo_setprogress( un->ledobjnum, l );
#endif /* USE_NET */

	ReleaseSemaphore( &netpoolsem );
}

void pushstring( struct unode *un, char *string )
{
	pushdata( un, string, strlen( string ) );
}

void STDARGS pushfmt( struct unode *un, char *fmt, ... )
{
	char buffer[ 2048 ];
	va_list va;

	va_start( va, fmt );

	vsnprintf( buffer, sizeof(buffer), (const STRPTR)fmt, (APTR)va );
	va_end( va );
	pushdata( un, buffer, strlen( buffer ) );
}


void makehtmlerror( struct unode *un )
{
	if ( !un->tofilename && !getprefslong( VFLG_USE_ERROR_REQUESTER, TRUE ) )
	{
		un->tomem = 1; // sucketh...
		pushfmt( un, GS( NWM_ERROR_HTML ),
			un->errorcode >= 0 ? GS( NWM_ERROR_NETWORK ) : GS( NWM_ERROR_INTERNAL ),
			un->errorstring
		);
		strcpy( un->mimetype, "text/html" );
	}
	else
	{
		puterror( LT_NET, LL_ERROR, un->errorcode, un->url, un->errorstring );
	}
	sur_gauge_clear( un );
}

#if USE_NET
void makeneterror( struct unode *un, char *str, int err )
{
	char *errstr = 0;
	STRPTR dummy[ 3 ];

	dummy[ 0 ] = (STRPTR)SBTM_GETVAL(SBTC_ERRNOSTRPTR);
	dummy[ 1 ] = (STRPTR)err;
	dummy[ 2 ] = NULL;

	SocketBaseTagList( (struct TagItem *)dummy );

	errstr = dummy[ 1 ];
	un->errorcode = err;
	snprintf( un->errorstring, sizeof( un->errorstring ), str, err, errstr ? errstr : "" );

	puterror( LT_NET, LL_ERROR, un->errorcode, un->url, un->errorstring );
	sur_gauge_clear( un );
}
#else
void makeneterror( struct unode *un, char *str, int err )
{
	puterror( LT_NET, LL_ERROR, 0xF0AD, un->url, "USE_NET not enabled" );
	sur_gauge_clear( un );
}
#endif /* USE_NET */

#if USE_DOS

#define KILO 1024
#define MEGA (KILO*KILO)
#define GIGA (KILO*MEGA)
#define SCALESIZE(size) (size<KILO?size:size<MEGA?(size+KILO-1)/KILO:size<GIGA?(size+MEGA-1)/MEGA:(size+GIGA-1)/GIGA)
#define SCALECHAR(size) (size<KILO?'b':size<MEGA?'K':size<GIGA?'M':'G')

#define EAC_BUFFERSIZE 2048 /* ExAll buffer */

/*
 * Reads a whole directory and displays its entries.
 */
static void un_read_dir( struct unode *un, BPTR l )
{
	int dircnt = 0, filecnt = 0, filesize = 0;
	char x[ 8 ];
	char parent[ 255 ];
	BPTR pl;
	struct ExAllControl *eac;
	APTR buf;

	if( ( eac = AllocDosObject( DOS_EXALLCONTROL, NULL ) ) )
	{
		if( ( buf = nalloc( EAC_BUFFERSIZE ) ) )
		{
			struct ExAllData *ead;
			char dots[ 40 ];        /* those are used below but that way we avoid stack allocation */
			char *filename;
			struct DateStamp ds;
			char mimeb[ 128 ];
			char *mime;
			int size;
			int	more;

			un->tomem = 1; // sucketh...
			strcpy( un->mimetype, "text/html" );

			pushfmt( un, "<HEAD><TITLE>Local directory %s</TITLE><BODY><H2>Local directory <B>%s</B></H2><HR><PRE>",
				un->purl.path, un->purl.path
			);

			sur_text( un, GS( NETSTAT_LOCALDIR ) );

			/*
			 * Display the parent directory
			 */
		    if( pl = ParentDir( l ) )
			{
				D_S(struct FileInfoBlock,fib);
				if( Examine( pl, fib ) )
				{
					utunpk( __datecvt( &fib->fib_Date ), x );
					NameFromLock( pl, parent, 255 );
					pushfmt( un, "<IMG SRC=internal-gopher-menu WIDTH=13 HEIGHT=17 ALT=[DIR]> <A HREF=\"file:///%s\">[Parent Directory]</A>.....................  [%02ld.%02ld.%04ld] %s\n",
						parent, x[ 2 ], x[ 1 ], x[ 0 ] + 1970,
						fib->fib_Comment
					);
				}
				UnLock( pl );
			}

			/*
			 * Display the entries
			 */
			eac->eac_LastKey = 0;
			do
			{
				more = ExAll( l, buf, EAC_BUFFERSIZE, ED_COMMENT, eac );
				if( ( !more ) && ( IoErr() != ERROR_NO_MORE_ENTRIES ) )
				{
					sur_text( un, GS( NETSTAT_EXALLFAILED ) );
					displaybeep();
					break;
				}

				if( eac->eac_Entries > 0 )
				{
					ead = ( struct ExAllData * )buf;
					
					do
					{
						filename = ead->ed_Name;

						ds.ds_Days = ead->ed_Days;
						ds.ds_Minute = ead->ed_Mins;
						ds.ds_Tick = ead->ed_Ticks;

						utunpk( __datecvt( &ds ), x );

						if( strlen( filename ) < 39 )
						{
							memset( dots, '.', 39 );
							dots[ 39 - strlen( filename ) ] = 0;
						}
						else
							dots[ 0 ] = 0;

						if( ead->ed_Type < 0 )
						{
							size = ead->ed_Size;

							if( !mime_findbyextension( filename, NULL, NULL, 0, mimeb ) )
							{
								mime = "unknown";
							}
							else
							{
								if( !strnicmp( mimeb, "application/", 12 ) )
									mime = "bin";
								else if( !strnicmp( mimeb, "text/", 5 ) )
									mime = "text";
								else if( !strnicmp( mimeb, "image/", 6 ) )
									mime = "image";
								else if( !strnicmp( mimeb, "audio/", 6 ) )
									mime = "audio";
								else if( !strnicmp( mimeb, "video/", 6 ) )
									mime = "video";
								else
									mime = "unknown";
							}
							pushfmt( un, "<IMG SRC=internal-gopher-%s WIDTH=13 HEIGHT=17 ALT=[DIR]> <A HREF=\"%s\">%s</A>%s  [%02ld.%02ld.%04ld] %5ld%lc %s\n",
								mime, filename, filename, dots, x[ 2 ], x[ 1 ], x[ 0 ] + 1970,
								SCALESIZE(size),SCALECHAR(size),
								ead->ed_Comment ? ( char * )ead->ed_Comment : ""
							);
							filecnt++;
							filesize += size;
						}
						else
						{
							pushfmt( un, "<IMG SRC=internal-gopher-menu WIDTH=13 HEIGHT=17 ALT=[DIR]> <A HREF=\"%s/\">%s</A>%s  [%02ld.%02ld.%04ld] %s\n",
								filename, filename, dots, x[ 2 ], x[ 1 ], x[ 0 ] + 1970,
								ead->ed_Comment ? ( char * )ead->ed_Comment : ""
							);
							dircnt++;
						}
						ead = ead->ed_Next;
					} while( ead );
				}
			} while( more );

			pushfmt( un, "<HR></PRE><FONT SIZE=-1>Total %u %s, %u files (%u%c)" INTERNALDOC,
				dircnt, dircnt == 1 ? "directory" : "directories", filecnt,
				SCALESIZE( filesize ), SCALECHAR( filesize )
			);

			un->state = UNS_DONE;
			sur_gauge_clear( un );
		
			nfree( buf );
		}
		else
		{
			sur_text( un, GS( NETSTAT_NODIRMEM ) );
			displaybeep();
		}
		FreeDosObject( DOS_EXALLCONTROL, eac );
	}
	else
	{
		sur_text( un, GS( NETSTAT_NODIRMEM ) );
		displaybeep();
	}
}

#endif /* USE_DOS */

#if USE_NETFILE
static void un_setup_file( struct unode *un )
{
	char path[ 256 ], *p;
#if USE_DOS
	char error[ 64 ], *realpath = path;
	BPTR l;
	D_S(struct FileInfoBlock,fib);
#endif

	un->retr_mode = RM_FILE;

	/* convert ascii chars (%20), etc... */   
	uri_decode(un->purl.path);

	stccpy( path, un->purl.path, sizeof( path ) );
#ifndef MBX
	/* Check for PIPE:/APIPE: exploits -- not on CaOS */
	p = path;
	while( *p )
	{
		if( !strnicmp( p, "PIPE:", 5 ) )
		{
			SetIoErr( ERROR_INVALID_COMPONENT_NAME );
			goto filerr;
		}
		p++;
	}
#endif

	/* Check whether referer (if existant) is also file:/// */
	if( un->referer )
	{
		if( strnicmp( un->referer, "file:", 5 ) )
		{
			strcpy( un->errorstring, "Access to file denied" );

			un->errorcode = -4;
			un->state = UNS_FAILED;
			makehtmlerror( un );
			return;
		}
	}

	DL( DEBUG_INFO, db_net, bug( "setup_as_file(%s) -> path %s\n", un->url, path ));

	un->fromfile = Open( path, MODE_OLDFILE );

	if( !un->fromfile ) { // try to reverse slashes, for pages wrote by sucky windows user
		STRPTR  ptr = path;
		BOOL    try = FALSE;

		while( ptr = strchr( ptr, '\\' )) {
			*ptr++ = '/';
			try    = TRUE;
		}

		if( try )
			un->fromfile = Open( path, MODE_OLDFILE );
	}

#if USE_DOS
	if( IoErr() == ERROR_OBJECT_WRONG_TYPE )
	{
		// Directory, apparently
		// AddMe!!!!!! We need an option to determine whether to redirect to index.html when specifying directories
		char urlend = un->url[ strlen( un->url ) - 1 ];
		char path2[ 267 ];

		DL( DEBUG_INFO, db_net, bug( "ERROR_OBJECT_WRONG_TYPE\n" ) );

		if( urlend != '/' && urlend != ':' )
		{
			char *tmp = unalloc( un, strlen( path ) + 2 );
			if( tmp )
			{
				strcpy( tmp, un->url );
				strcat( tmp, "/" );
				un->redirecturl = tmp;
				un->state = UNS_WAITING;
			}
			else
				un->state = UNS_DONE;
			return;
		}

		strcpy( path2, path );
		AddPart( path2, "index.html", 267 );
		if( un->fromfile = Open( path2, MODE_OLDFILE ) )
			realpath = path2;
		else
		{
			l = Lock( path, SHARED_LOCK );
			if( l )
			{
				if( Examine( l, fib ) )
				{
					un_read_dir( un, l );  /* TOFIX: check that we don't clear the status bar afterwards */
					UnLock( l );
					return;
				}
				UnLock( l );
			}
		}
	}
#endif

	if( un->fromfile )
	{
#if USE_DOS
		ExamineFH( un->fromfile, fib );

		if( !mime_findbyextension( realpath, NULL, NULL, NULL, un->mimetype ) )
			strcpy( un->mimetype, "text/plain" );

		un->doclen = fib->fib_Size;
		un->state = UNS_WAITING;
		return;
#else
		if( !mime_findbyextension( path, NULL, NULL, NULL, un->mimetype ) ) //TOFIX!! path doesn't find out if there's an index.html automatically
			strcpy( un->mimetype, "text/plain" );

		un->doclen = GetFileSize( un->fromfile );
		un->state = UNS_WAITING;
		return;
#endif

	}

#if USE_DOS
filerr:
	// file error!
	Fault( IoErr(), NULL, error, sizeof( error ) );
	snprintf( un->errorstring, sizeof(un->errorstring), GS( NWM_ERROR_FILEOPENFAILED ), path, IoErr(), error );
#else
	snprintf( un->errorstring, sizeof(un->errorstring), "Can't open file %s", path );
#endif

	un->errorcode = -4;
	un->state = UNS_FAILED;
	makehtmlerror( un );
}
#endif /* USE_NETFILE */


#define NN(x) x?x:"*NULL*"

static char *ip2a( ULONG ip )
{
	static char ipabuff[ 32 ];

	sprintf( ipabuff, "%lu.%lu.%lu.%lu",
		( ip >> 24 ) ,
		( ip >> 16 ) & 0xff,
		( ip >> 8 ) & 0xff,
		ip & 0xff
	);
	return( ipabuff );
}

/* returns true if a network connection has to be done
** false if it has to be done from the cache */
int isonline( void )
{
	if ( !opennet() )
		return( FALSE );

#if USE_VAT
	if ( gp_checkifonline )
	{
		/* let's query the TCP/IP stack */
		return( VAT_IsOnline() );
	}
	else
#endif
	{
		if ( gp_offlinemode )
		{
			return( FALSE );
		}
		else
		{
			return( TRUE );
		}
	}
}

/* setup an unode, parse the url to send it to the right protocol */
static void un_setup( struct unode *un )
{
	struct parsedurl purl;
	int proxy_port;
	char *proxy_host = NULL;
#if USE_DOS
	int errorread = FALSE;
#endif
#if USE_PLUGINS
	char *data;
	int	size;
#endif /* USE_PLUGINS */

	// initial setup

	// parse URL
	strcpy( un->urlcopy, un->urlcopy2 );
	uri_split( un->urlcopy, &purl );
	un->purl = purl;

	if( !purl.scheme )
		purl.scheme = "http";

	strlwr( purl.scheme );

	DL( DEBUG_INFO, db_net, bug( "setting up %s (%s, %s, %s, %s, %s, %s, %ld, %s, %ld)\n", un->url, NN( purl.scheme ), NN( purl.host ), NN( purl.username ), NN( purl.password ), NN( purl.path ), NN( purl.args ), purl.port, NN( purl.fragment ), purl.pathrelative ));

	if( !strcmp( purl.scheme, "about" ) ) /* about stuff */
	{
		un->retr_mode = RM_INTERNAL;
		un->tomem = 1;
		strlwr( purl.path );
		strcpy( un->mimetype, "text/html" );

		if( !strcmp( purl.path, "owner" ) )
		{

		}
		else if( !strcmp( purl.path, "vaporlogo" ) )  /* about:vaporlogo */
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				int size;
				APTR data;
				data = VABOUT_GetVLogo( &size );
				pushdata( un, data, size );
				strcpy( un->mimetype, "image/jpeg" );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		else if( !strcmp( purl.path, "v3logo" ) )  /* about:vaporlogo */
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				int size;
				APTR data;
				data = VABOUT_GetV3Logo( &size );
				pushdata( un, data, size );
				strcpy( un->mimetype, "image/jpeg" );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		else if( !strcmp( purl.path, "flashlogo" ) )  /* about:vaporlogo */
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				int size;
				APTR data;
				data = VABOUT_GetFlashLogo( &size );
				pushdata( un, data, size );
				strcpy( un->mimetype, "image/jpeg" );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		else if( !strcmp( purl.path, "pnglogo" ) )  /* about:pnglogo */
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				int size;
				APTR data;
				data = VABOUT_GetPNGLogo( &size );
				pushdata( un, data, size );
				strcpy( un->mimetype, "image/gif" );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		else if( !strcmp( purl.path, "ssllogo" ) ) /* about:ssllogo TOFIX: disable if there's no SSL available */
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				int size;
				APTR data;
				data = VABOUT_GetSSLLogo( &size );
				pushdata( un, data, size );
				strcpy( un->mimetype, "image/gif" );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		else if( !strcmp( purl.path, "ibeta" ) )  /* about:ibeta */
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				pushstring( un, VABOUT_GetAboutIbeta() );
				strcpy( un->mimetype, "text/html" );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		else if( !strcmp( purl.path, "blank" ) ) /* about:blank */
		{

		}
		else if( !strcmp( purl.path, "compile" ) ) /* about:compile */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff><PRE>" LVERTAG " " __DATE__ " " __TIME__ "\n</HTML>" );
		}
		else if ( !strcmp( purl.path, "neko" ) )
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "Fuck off!!" );
		}
#ifdef MBX
		else if( !strcmp( purl.path, "tv" ) ) /* about:tv */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff BACKGROUND=tv:>\n"
				"<H1>Very famous Met@Box TV Demonstration!</H1><BR>"
				"The red brown fox jumps quickly over the lazy dog. "
				"The red brown fox jumps quickly over the lazy dog. "
				"The red brown fox jumps quickly over the lazy dog. "
				"The red brown fox jumps quickly over the lazy dog. "
				"The red brown fox jumps quickly over the lazy dog. "
				"The red brown fox jumps quickly over the lazy dog. "
				"The red brown fox jumps quickly over the lazy dog. "
				"</HTML>");
		}
		else if( !strcmp( purl.path, "demo" ) ) /* about:demo */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"<H1>Very famous Met@Box Demonstration!</H1><BR>"
				"<A HREF=\"about:tv\">about:tv</A><BR>"
				"<A HREF=\"file:///rom0:portal/index.html\">romfs portal</A><BR>"
				"<A HREF=\"http://www.niblet.co.uk/pipx.html\">pipscrolltest</A><BR>"
				"<A HREF=\"http://chicken.metabox.de/phx/portal/index.html\">chicken portal</A><BR>"
				"<A HREF=\"http://www.niblet.co.uk/tv.html\">www.niblet.co.uk/tv.html</A><BR>"
				"<A HREF=\"http://212.96.44.169/gbtv/\">GB Village</A><BR>"
				"<A HREF=\"http://212.96.44.55/org/phxupd/index.html\">Phoenix Updates</A><BR>"
				"<A HREF=\"http://212.96.44.85/html4/index.html\">Testsuite HTML 4.01</A><BR>"
				"<A HREF=\"http://www.walla.co.il\">www.walla.co.il</A><BR>"
				"<A HREF=\"http://www.huji.ac.il\">www.huji.ac.il</A><BR>"
				"<A HREF=\"http://chicken.metabox.de/phx/frames.html\">DVB Demo</A><BR>"
				"<A HREF=\"http://www.metabox.de/\">Met@Box</A><BR>"
				"<A HREF=\"http://www.metatv.de/1/\">MetaTV Phoenix Portal</A><BR>"
				"<A HREF=\"http://www.discovery.com/\">Disovery Channel</A><BR>"
				"<A HREF=\"http://v3.vapor.com/vapor/vtestsuite/\">Voyager Test Suite</A><BR>"
				"<A HREF=\"http://www.heise.de/\">Heise</A><BR>"
				"<A HREF=\"http://212.96.44.167/\">Show</A><BR>"
				"<A HREF=\"http://www.theregister.co.uk/\">The Register</A><BR>"
				"<A HREF=\"http://www.altavista.com/\">Alta Vista</A><BR>"
				"<A HREF=\"http://www.finanztreff.de/\">Finanztreff</A><BR>"
				"<A HREF=\"http://172.16.3.98/phoenix/index.html\">Edgar (internal)</A><BR>"
				"<A HREF=\"http://192.168.1.155/\">Jon's masq box</A><BR>"
				"<A HREF=\"http://www.caos.de/\">Carsten's masq box</A><BR>"
				"</HTML>");
		}
		else if( !strcmp( purl.path, "mbx" ) ) /* about:mbx */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"You rule!<BR>"
				"This -&gt;<A HREF=\"about:mbx2\">foo</A>&lt;- is a link<BR>"
				"This too:<A HREF=\"http://10.1.1.1/\">Bodylove's local server</A><BR>"
				"Even this:<A HREF=\"http://192.168.1.155/\">Sircus' local server</A><BR>"
				"External link:<A HREF=\"http://www.gnu.org/\">GNU website</A><BR>"
				"External link:<A HREF=\"http://198.186.203.18/\">GNU website (IP)</A><BR>"
				"External link:<A HREF=\"http://www.metabox.de/\">Met@box website</A><BR>"
				"External link:<A HREF=\"http://news.bbc.co.uk/\">BBC News</A><BR>"
				"External link:<A HREF=\"http://212.96.44.65/\">Met@box website (IP)</A><BR>"
				"External link:<A HREF=\"http://www.metatv.de/html/content/index.html\">Met@TV website</A><BR>"
				"External link:<A HREF=\"http://212.96.44.50/html/content/index.html\">Met@TV website (IP)</A><BR>"
				"External link:<A HREF=\"http://www.webtv.com/\">WebTV website</A><BR>"
				"External link:<A HREF=\"http://207.46.121.12/\">WebTV (IP)</A><BR>"
				"External link:<A HREF=\"http://test.globalmegacorp.org/\">test.globalmegacorp.org</A><BR>"
				"<HR><BR><HR><BR><HR><BR><HR><BR><HR><BR><HR><BR>\n"
				"everything in here suxx!\n"
				"<HR><BR><HR><BR><HR><BR><HR><BR><HR><BR><HR><BR>\n");
		}
		else if( !strcmp( purl.path, "mbx2" ) )
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#00ff00>\nYou still rule!<BR>Another <A HREF=\"about:mbx3\">link</A><BR><BR>Link to an <A HREF=\"about:mbx4\">image</A>\n");
		}
		else if( !strcmp( purl.path, "mbx3" ) )
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#00ffff>\nA ruling table.<BR><P>A paragraph before the ruling table</P><FONT SIZE=\"+5\">Big stuff</FONT>\n<TABLE WIDTH=50% BORDER=1><TR><TD>Cell 1, Row 1<TD>Cell 2, Row 1<TD>Cell 3, Row 1<TR><TD ALIGN=LEFT>Left-aligned cell<TD ALIGN=RIGHT>Right<TD ALIGN=CENTER>Center</TABLE><BR>Another <A HREF=\"about:mbx\">link</A><BR><BR>Link to an <A HREF=\"about:mbx4\">image</A>\n");
		}
		else if( !strcmp( purl.path, "mbx4" ) )
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#00ffff>\nAn absent ruling image.<BR><BR><BR>Link <A HREF=\"about:mbx\">Back</A>\n");
//			pushstring( un, "<HTML>\n<BODY BGCOLOR=#00ffff>\nAn absent ruling image.<BR><IMG SRC=\"about:mbximage\"><BR><BR>Link <A HREF=\"about:mbx\">Back</A>\n");
		}
		else if( !strcmp( purl.path, "mbximage" ) )
		{
			strcpy( un->mimetype, "image/png" );
			pushdata( un, mbximage, mbximagelen );
		}
		else if( !strcmp( purl.path, "metabox" ) ) /* about:mbx */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"<H1>Welcome to met@box test page #1!</H1><BR>"
				"<B>THE FOLLOWING PAGES ARE ONLY FOR TESTING PURPOSE</B><BR>"
				"Please report every broken link and spell mistake :-)<BR>"
				"<HR><img src=tv: width=300 height=200><BR>"
				"<HR><BR><A HREF=about:metabox2>more</A>\n");
		}
		else if( !strcmp( purl.path, "metabox2" ) ) /* about:mbx */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"<H1>Welcome to met@box test page #2!</H1><BR>"
				"<B>The TV Screen is centered:</B><BR>"
				"<HR><CENTER>"
				"<img src=tv: width=300 height=200><BR>"
				"</CENTER>"
				"<HR><BR><A HREF=about:metabox3>more</A>\n");
		}
		else if( !strcmp( purl.path, "metabox3" ) ) /* about:mbx */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"<H1>Welcome to met@box test page #3!</H1><BR>"
				"<B>Watch the TV Screen zoomed:</B><BR>"
				"<HR><img src=tv: width=1280 height=960><BR>"
				"<HR><BR><A HREF=about:metabox4>more</A>\n");
		}
		else if( !strcmp( purl.path, "metabox4" ) ) /* about:mbx */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"<H1>Welcome to met@box test page #4!</H1><BR>"
				"<B>The TV Screen surrounded by text and pictures:</B><BR>"
				"<table><tr><td><img src=mbximage width=400 height=300></td><td><font color=red>The upper side of the TV Screen</font></td><td><img src=mbximage width=400 height=300></td></tr>"
				"<tr><td><font color=red>The left side of the TV Screen</font></td><td><img src=tv: width=400 height=300></td><td><font color=red>The right side of the TV Screen</font></td></tr>"
				"<tr><td><img src=mbximage width=400 height=300></td><td><font color=red>The bottom side of the TV Screen</font></td><td><img src=mbximage width=400 height=300></td></tr></table>"
				"<HR><BR><A HREF=about:metabox5>more</A>\n");
		}
		else if( !strcmp( purl.path, "metabox5" ) ) /* about:mbx */
		{
			strcpy( un->mimetype, "text/html" );
			pushstring( un, "<HTML>\n<BODY BGCOLOR=#ffffff>\n"
				"<H1>Welcome to met@box test page #5!</H1><BR>"
				"<B>Nothing special on this side:</B><BR>"
				"Please report every broken link and spell mistake :-)<BR>"
				"<img src=tv: width=300 height=200><BR>"
				"<HR><BR><A HREF=about:metabox>start</A>\n");
		}
#endif
#if USE_NET
		else if( !strcmp( purl.path, "dnscache" ) ) /* about:dnscache */
		{
			struct dnscachenode *dns;
			int cnt = 0, size = 0;

			pushstring( un, "<HTML>\n<HEAD>\n<TITLE>DNS Cache List</TITLE>\n</HEAD>\n\n"
							"<BODY BGCOLOR='White' TEXT='Black'>\n<CENTER>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black'>\n<TR>\n<TD>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH='100%%'>\n<TR>\n<TD>\n"
							"<FONT COLOR='White'><TT>DNS Cache List</TT></FONT>\n</TD>\n</TR>\n</TABLE>\n\n"
							"</TD>\n</TR>\n\n<TR>\n<TD>\n\n<TABLE BORDER=2 CELLSPACING=0 CELLPADDING=2 WIDTH='100%%'>\n<TR BGCOLOR='Silver'><TH>Name</TH><TH>CName</TH><TH>Flags</TH><TH>IPs</TH></TR>\n"
						);

			ObtainSemaphoreShared( &dnscachesem );

			for( dns = dnscachelist; dns; dns = dns->next )
			{
				int c;

				pushfmt( un, "<TR BGCOLOR='White'><TD VALIGN=TOP><A HREF='http://%s/'>%s</A><TD VALIGN=TOP>%s<TD VALIGN=TOP>%s<TD VALIGN=TOP>",
					dns->name, dns->name, dns->name + dns->namelen + 1,
					dns->http11failed ? "F11" : ""
				);

				for( c = 0; dns->ip[ c ]; c++ )
				{
					pushfmt( un, "%s%s", c ? "<BR>" : "",
						ip2a( dns->ip[ c ] )
					);
					size += 4;
				}

				pushstring( un, "</TR>\n" );

				cnt++;
				size += sizeof( *dns ) + dns->namelen + strlen( dns->name + dns->namelen + 1 ) + 2 + 4;
			}
			ReleaseSemaphore( &dnscachesem );
			pushfmt( un, "<TR BGCOLOR='Silver'><TH COLSPAN=4>Total %u entries, %u bytes used</TH></TR>\n</TABLE>\n\n</TD>\n</TR>\n</TABLE>\n\n</CENTER>\n" INTERNALDOC, cnt, size );
		}
		else if( !strcmp( purl.path, "sslcache" ) ) /* about:sslcache */
		{
#if USE_SSL
			struct VSSL_CacheInfo ci;
#endif
			pushstring( un, "<HTML>\n<HEAD>\n<TITLE>SSL Session Cache</TITLE>\n</HEAD>\n\n"
							"<BODY BGCOLOR='White' TEXT='Black'>\n<CENTER>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black'>\n<TR>\n<TD>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH='100%%'>\n<TR>\n<TD>\n"
							"<FONT COLOR='White'><TT>SSL Session Cache</TT></FONT>\n</TD>\n</TR>\n</TABLE>\n\n"
							"</TD>\n</TR>\n\n<TR>\n<TD>\n\n<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 WIDTH='100%%'>\n<TR BGCOLOR='White'>\n"
						);


#if USE_SSL
			if(
#ifndef MBX
				VSSLBase &&
#endif
				ssl_ctx
			)
			{
				VSSL_GetStats( ssl_ctx, &ci );
				pushfmt( un,
						"<TD ALIGN=RIGHT><TT><B>Number of Sessions:<P>Connections:<P>Successful connections:<P>Hits:<P>Misses:<P>Timeouts:</B></TT></TD>\n"
						"<TD><TT>%ld<P>%ld<P>%ld<P>%ld<P>%ld<P>%ld</TD>\n</TR>\n</TABLE>\n\n",

					ci.sess_number,
					ci.sess_connect,
					ci.sess_connect_good,
					ci.sess_hits,
					ci.sess_misses,
					ci.sess_timeouts
				);
			}
			else
#endif /* USE_SSL */
				pushstring( un, "<TD ALIGN=CENTER>(no SSL sessions yet)</TD>\n</TR>\n</TABLE>\n\n" );

			pushstring( un, "</TD>\n</TR>\n</TABLE>\n\n</CENTER>\n" );
			pushstring( un, INTERNALDOC );
		}
		else if( !strcmp( purl.path, "cachestats" ) ) /* about:cachestats */
		{
			pushstring( un, "<HTML>\n<HEAD>\n<TITLE>Disk Cache Stats</TITLE>\n</HEAD>\n\n"
							"<BODY BGCOLOR='White' TEXT='Black'>\n<CENTER>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black'>\n<TR>\n<TD>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH='100%%'>\n<TR>\n<TD>\n"
							"<FONT COLOR='White'><TT>Disk Cache Stats</TT></FONT>\n</TD>\n</TR>\n</TABLE>\n\n"
							"</TD>\n</TR>\n\n<TR>\n<TD>\n\n<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 WIDTH='100%%'>\n<TR BGCOLOR='White'>\n"
						);

			pushfmt( un, "<TD ALIGN=RIGHT><TT><B>Total hits:<P>From cache:<P>Verified OK:<P>Not verified:<P>Stored after fetch:<P>Approximate size:</B></TT></TD>\n"
						"<TD><TT>%ld<P>%ld<P>%ld<P>%ld<P>%ld<P>%ld bytes</TD>\n</TR>\n</TABLE>\n\n",

				cache_stats.total_hits,
				cache_stats.from_cache,
				cache_stats.verified_ok,
				cache_stats.verified_failed,
				cache_stats.stored_fetched,
				estimated_cache_size
			);

			pushstring( un, "</TD>\n</TR>\n</TABLE>\n\n</CENTER>\n" );
			pushstring( un, INTERNALDOC );
		}
		else if( !strcmp( purl.path, "verifylist" ) )  /* about:verifylist */
		{
			struct verifynode *vn = verifylist;

			pushstring( un, "<HTML>\n<HEAD>\n<TITLE>Cache Verify List</TITLE>\n</HEAD>\n\n"
							"<BODY BGCOLOR='White' TEXT='Black'>\n<CENTER>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black'>\n<TR>\n<TD>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH='100%%'>\n<TR>\n<TD>\n"
							"<FONT COLOR='White'><TT>Cache Verify List</TT></FONT>\n</TD>\n</TR>\n</TABLE>\n\n"
							"</TD>\n</TR>\n\n<TR>\n<TD>\n\n<TABLE BORDER=2 CELLSPACING=0 CELLPADDING=2 WIDTH='100%%'>\n<TR BGCOLOR='Silver'><TH>Verified URLs in Cache</TH></TR>\n<TR BGCOLOR='White'>\n<TD>\n"
						);

			while( vn )
			{
				pushdata( un, vn->url, vn->len );
				pushstring( un, "<BR>\n" );
				vn = vn->next;
			}

			pushstring( un, "</TD>\n</TR>\n</TABLE>\n\n</TD>\n</TR>\n</TABLE>\n\n</CENTER>\n" );
			pushstring( un, INTERNALDOC );
		}
#endif /* USE_NET */
		else if( !strcmp( purl.path, "memcache" ) ) /* about:memcache */
		{
			struct unode *unl;
			int cnt = 0, size = 0;

			pushstring( un, "<HTML>\n<HEAD>\n<TITLE>Cache Verify List</TITLE>\n</HEAD>\n\n"
							"<BODY BGCOLOR='White' TEXT='Black'>\n<CENTER>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black'>\n<TR>\n<TD>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH='100%%'>\n<TR>\n<TD>\n"
							"<FONT COLOR='White'><TT>Cache Verify List</TT></FONT>\n</TD>\n</TR>\n</TABLE>\n\n"
							"</TD>\n</TR>\n\n<TR>\n<TD>\n\n<TABLE BORDER=2 CELLSPACING=0 CELLPADDING=2 WIDTH='100%%'>\n<TR BGCOLOR='Silver'><TH>URL</TH><TH>State</TH><TH>Flags</TH><TH>Size</TH><TH>MIME</TH><TH>Realm</TH></TR>\n"
						);

			for( unl = FIRSTNODE( &ulist ); NEXTNODE( unl ); unl = NEXTNODE( unl ) )
			{
				pushfmt( un, "<TR BGCOLOR='White'><TD NOWRAP>%s<TD NOWRAP>%ld/%ld<TD NOWRAP>%s%s%s<TD NOWRAP>%ld/%ld<TD NOWRAP>%s<TD NOWRAP>%s</TD</TR>\n",
					unl->urlcopy2,
					unl->state, un->protocolstate,
					unl->errorcode ? " ERROR" : "",
#if USE_SSL
					unl->ssl ? " SSL" : "",
#else
					"",
#endif /* !USE_SSL */
					unl->postid ? " POST" : "",
					unl->doclen, unl->membuffsize,
					unl->mimetype,
					unl->authrealm
				);

				size += unl->membuffsize;

				cnt++;
			}
			pushfmt( un, "<TR BGCOLOR='Silver'><TH COLSPAN=6>Total %ld entries, %ld bytes used</TH></TR>\n</TABLE>\n\n</TD>\n</TR>\n</TABLE>\n\n</CENTER>\n" INTERNALDOC, cnt, size );
		}
#if USE_NET
		else if( !strcmp( purl.path, "cache" ) ) /* about:cache */
		{
#if USE_DOS
			int cnt = 0, size = 0;
			int cix;
			ULONG urllen;
			ULONG mimelen;
			ULONG checksum;

			sur_text( un, GS( NETSTAT_LISTCACHE ) );

			pushstring( un, "<HTML>\n<HEAD>\n<TITLE>Disk Cache</TITLE>\n</HEAD>\n\n"
							"<BODY BGCOLOR='White' TEXT='Black'>\n<CENTER>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black'>\n<TR>\n<TD>\n\n"
							"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH='100%%'>\n<TR>\n<TD>\n"
							"<FONT COLOR='White'><TT>Disk Cache</TT></FONT>\n</TD>\n</TR>\n</TABLE>\n\n"
							"</TD>\n</TR>\n\n<TR>\n<TD>\n\n<TABLE BORDER=2 CELLSPACING=0 CELLPADDING=2 WIDTH='100%%'>\n<TR BGCOLOR='Silver'><TH>URL</TH><TH>Size</TH><TH>Date</TH><TH>Expires</TH><TH>MIME</TH><TH>File</TH></TR>\n"
						);

			for( cix = 0; cix < 256; cix++ )
			{
				BPTR olddir = chcache( cix );
				char name[ 256 ], name2[ 256 ];
				D_S(struct FileInfoBlock,fib);

				NameFromLock( cache_locks[ cix ], name, sizeof( name ) );

				strcpy( name, FilePart( name ) );

				if( Examine( cache_locks[ cix ], fib ) )
				{
					while( ExNext( cache_locks[ cix ], fib ) )
					{
						BPTR f = Open( fib->fib_FileName, MODE_OLDFILE );
						if( f )
						{
							struct cacheheader ch;
							if ( strlen( fib->fib_Comment ) != 40 || sscanf( fib->fib_Comment, "%08X%08X%08X%08X%08X", ( int * )&checksum, ( int * )&ch.expires, &ch.doclen, ( int * )&urllen, ( int * )&mimelen ) != 5 )
							{
								DL( DEBUG_WARNING, db_net, bug("wrong comment format\n"));
								errorread = TRUE;
							}
							else
							{
								if ( checksum == ch.expires ^ ch.doclen ^ urllen ^ mimelen )
								{
									ch.urllen = urllen;
									ch.mimelen = mimelen;
								}
								else
								{
									DL( DEBUG_WARNING, db_net, bug("wrong checksum\n"));
									errorread = TRUE;
								}
							}

							if (!errorread)
							{
								char mime[ 256 ], url[ 512 ];
								char db_s[ 32 ], db_e[ 32 ];

								strcpy( db_s, datestamp2string( &fib->fib_Date ) );
								strcpy( db_e, date2string( ch.expires ) );

								if( ch.urllen > 511 || ch.mimelen > 255 )
								{
									pushfmt( un, "<TR BGCOLOR='White'><TD COLSPAN=6><FONT SIZE='-2' COLOR='Red'>Nuked cache entry: %s/%s, urllen %ld, mimelen %ld</FONT></TD></TR>\n",
										name, fib->fib_FileName,
										ch.urllen, ch.mimelen
									);
								}
								else /* can I have the filename trimmed to filename only, no PROGDIR please */
								{
									Read( f, url, min( ch.urllen, sizeof( url ) - 1 ) );
									url[ min( ch.urllen, sizeof( url ) -1 ) ] = 0;
									Read( f, mime, ch.mimelen );
									mime[ ch.mimelen ] = 0;
									strcpy( name2, name );
									AddPart( name2, fib->fib_FileName, sizeof( name2 ) );

									pushfmt( un, "<TR BGCOLOR='White'><TD NOWRAP><FONT SIZE='-2'><A HREF='%s'>%s</A><TD><FONT SIZE='-2'>%ld<TD NOWRAP><FONT SIZE='-2'>%s<TD NOWRAP><FONT SIZE='-2'>%s<TD><FONT SIZE='-2'>%s<TD><FONT SIZE='-2'>%s</TD></TR>\n",
										url, url, fib->fib_Size,
										db_s,
										ch.expires ? db_e : "(never)",
										mime,
										name2
									);
								}
								cnt++;
								size += fib->fib_Size;
							}
							Close( f );
						}
					}
				}
				CurrentDir( olddir );
			}

			pushfmt( un, "<TR BGCOLOR='Silver'><TH COLSPAN=6>Total %ld entries, %ld bytes used</TH></TR>\n</TABLE>\n\n</TD>\n</TR>\n</TABLE>\n\n</CENTER>\n" INTERNALDOC, cnt, size );
#else /* !USE_DOS */
			pushfmt( un, "<HTML><BODY>No disk cache</BODY></HTML>" INTERNALDOC );
//TODO - be a bit more informative here
#endif /* USE_DOS */
		}
#endif /* USE_NET */
		else
		{
#if USE_ABOUTLIB
			struct Library *VAboutBase;
			VAboutBase = OpenLibrary( VABOUT_NAME, VABOUT_VERSION );
			if( VAboutBase )
			{
				STRPTR data;
				char bf2[ 256 ];
				extern struct Library *VIDBase;

#if USE_NET
				#if USE_KEYFILES
				if( getserial() != ~0 )
				{
					sprintf( bf2, "Registered to %s [%s]", getowner(), getserialtext() );
				}
				else
					strcpy( bf2, "· Unregistered Demo Copy ·" );
				#else
				#ifdef __MORPHOS__
				strcpy( bf2, "· MorphOS licensed version ·" );
				#endif
				#endif
#else /* USE_NET */
				strcpy( bf2, "· Freely distributable NoNet version ·" );
#endif /* USE_NET */

				data = VABOUT_GetAboutPtr( LVERTAG, bf2, VIDBase->lib_IdString );
				pushstring( un, data );
				CloseLibrary( VAboutBase );
			}
			else
#endif /* USE_ABOUTLIB */
				pushstring( un, "?about?" );
		}
		un->state = UNS_DONE;
		sur_gauge_clear( un );
	}
#if USE_NET
	else if( !strcmp( purl.scheme, "http" ) ) /* http: */
	{
		cache_stats.total_hits++;
		if( !un->reload && !un->postid )
		{
			time_t expires, now = timev();

			if( checkincache( un, &expires ) && ( expires > now || isverified( un->url ) ) )
			{
				cache_stats.from_cache++;
				un_setup_cache( un );
				return;
			}
		}
		if( isonline() )
		{
			proxy_host = proxy_for_url( un->url, &purl, &proxy_port );
			un_setup_http( un, proxy_host, proxy_port );
		}
		else
		{
			sprintf( un->errorstring, "Not in the cache" /*, purl.scheme*/ );
			un->errorcode = -1;
			un->state = UNS_FAILED;
			makehtmlerror( un );
		}
	}
	else if( !strcmp( purl.scheme, "https" ) ) /* https: */
	{
#ifdef ISSPECIALDEMO
		sprintf( un->errorstring, "Protocol %s not available in this special version.", purl.scheme );
		un->errorcode = -1;
		un->state = UNS_FAILED;
		makehtmlerror( un );
#else
		cache_stats.total_hits++;
		if( !un->reload && !getprefslong( DSI_SECURITY_NO_SSLCACHE, FALSE ) && !un->postid )
		{
			time_t expires, now = timev();

			if( checkincache( un, &expires ) && ( expires > now || isverified( un->url ) ) )
			{
				cache_stats.from_cache++;
				un_setup_cache( un );
				return;
			}
		}
		if( isonline() )
		{
			proxy_host = proxy_for_url( un->url, &purl, &proxy_port );
			if( proxy_host )
				un_setup_http( un, proxy_host, proxy_port );
			else
				un_setup_https( un );
		}
		else
		{
			sprintf( un->errorstring, "Not in the cache" /*, purl.scheme*/ );
			un->errorcode = -1;
			un->state = UNS_FAILED;
			makehtmlerror( un );
		}
#endif
	}
	else if( !strcmp( purl.scheme, "ftp" ) ) /* ftp: */
	{
#ifdef ISSPECIALDEMO
		sprintf( un->errorstring, "Protocol %s not available in this special version.", purl.scheme );
		un->errorcode = -1;
		un->state = UNS_FAILED;
		makehtmlerror( un );
#else
		if( isonline() )
		{
			proxy_host = proxy_for_url( un->url, &purl, &proxy_port );
			if( proxy_host )
				un_setup_http( un, proxy_host, proxy_port );
			else
				un_setup_ftp( un );
		}
		else
		{
			sprintf( un->errorstring, "Not in the cache" /*, purl.scheme*/ );
			un->errorcode = -1;
			un->state = UNS_FAILED;
			makehtmlerror( un );
		}
#endif /* ISSPECIALDEMO */
	}
	else if( !strcmp( purl.scheme, "gopher" ) ) /* gopher: */
	{
		cache_stats.total_hits++;
		if ( isonline() )
			un_setup_http( un, proxy_host, proxy_port );
	}
	else if( !strcmp( purl.scheme, "wais" ) )
	{
		cache_stats.total_hits++;
		if ( isonline() )
			un_setup_http( un, proxy_host, proxy_port );
		else
		{
			sprintf( un->errorstring, "Not in the cache" /*, purl.scheme*/ );
			un->errorcode = -1;
			un->state = UNS_FAILED;
			makehtmlerror( un );
		}
	}
#endif /* USE_NET */
#if USE_NETFILE
	else if( !strcmp( purl.scheme, "file" ) ) /* file: */
	{
		un_setup_file( un );
	}
#endif /* USE_NETFILE */
#if USE_PLUGINS
	else if( data = plugin_processurl( un->url, &size ) )
	{
		un->retr_mode = RM_INTERNAL;
		un->tomem = 1;
		strcpy( un->mimetype, "text/html" );
		pushdata( un, data, size );
		free( data );
		un->state = UNS_DONE;
		sur_gauge_clear( un );
	}
#endif /* USE_PLUGINS */
	else
	{
		// unknown method
		snprintf( un->errorstring, sizeof(un->errorstring), GS( NWM_ERROR_UNKNOWNMETHOD ), purl.scheme );
		un->errorcode = -1;
		un->state = UNS_FAILED;
		makehtmlerror( un );
	}
}

void un_delete( struct unode *un )
{
#if USE_DOS
	if( un->l )
		UnLock( un->l );
#endif /* USE_DOS */

#if USE_NETFILE
	if( un->fromfile )
#ifdef MBX
		CloseFile( un->fromfile );
#else
		Close( un->fromfile );
#endif
#endif

	// dispose this URL
	un_netclose( un );

	REMOVE( un );
	// kills all assorted memory
	DeletePool( un->pool );
}

static void un_done( struct unode *un )
{
#if USE_DOS
	struct nstream *ns;
#endif

	// data transfer successfully done

	if( !un->newclientdestmode )
		return;

#if USE_DOS

	// check if we're tomem and some download to file clients have been
	// added

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->destmode < 1 )
			continue;

		if( ns->tofile )
		{
			CloseAsync( ns->tofile );
			setcomment( ns->filename, ns->url );
			ns->tofile = NULL;
			ns->destmode = -1;
			continue;
		}

		if( ns->destmode == DM_FILE )
		{
			BPTR f;
			DL( DEBUG_INFO, db_net, bug("opening new dest file %s\n", ns->filename));
			f = Open( ns->filename, MODE_NEWFILE );
			if( f )
			{
				DL( DEBUG_INFO, db_net, bug("writing data to dest file\n"));
				Write( f, un->membuff, un->docptr );
				Close( f );
				setcomment( ns->filename, ns->url );
			}
		}

		ns->destmode = -1;
	}
#endif /* USE_DOS */

	un->newclientdestmode = 0;

}

#if USE_NET

void un_doprotocol( struct unode *un )
{
	switch( un->retr_mode )
	{
		case RM_HTTP:
			un_doprotocol_http( un );
			break;

		case RM_FTP:
			un_doprotocol_ftp( un );
			break;
	}
}

static void un_dnsdone( struct unode *un )
{
	un->dcn = un->dnsmsg.dcn;

	if( !un->dcn )
	{
		char buff[ 256 ];

		DL( DEBUG_WARNING, db_net, bug("dnsfailed(%s)\n", un->url ));

		sprintf( buff, GS( NWM_ERROR_NODNS ), un->dnsmsg.name );
		makeneterror( un, buff, -1 );
		un->errorcode = -2;
		un->doinform = TRUE;
		un->state = UNS_FAILED;

		return;
	}

	DL( DEBUG_INFO, db_net, bug( "dnsdone(%s) -> %lx (%s)\n", un->url, *un->dcn->ip, un->dcn->name ));
	un->state = UNS_WAITINGFORNET;
}

void setblocking( int sock, int mode )
{
#if USE_NET
#if (USE_BLOCKING_CONN==0)
	IoctlSocket( sock, FIONBIO, (char*)&mode );
#endif
#else /* !USE_NET */

#endif /* !USE_NET */
}

int uns_write( struct unode *un, STRPTR data, int len )
{
#if USE_NET
	if( len < 0 )
		len = strlen( data );

	while( len > 0 )
	{
		int rc = 0;

#if USE_SSL
		if( un->sslh )
		{
#ifndef MBX
			if( VSSLBase )
#endif
				rc = VSSL_Write( un->sslh, data, len );
#if USE_MIAMI
			else
				rc = SSL_write( un->sslh, data, len );
#endif
		}
		else
#endif /* USE_SSL */
		{
			rc = send( un->sock, data, len, 0 );
		}

		if( rc <= 0 )
			return( rc );

		len -= rc;
		data += rc;
	}
	return( TRUE );
#else
	return( 0 );
#endif /* !USE_NET */
}

int uns_read( struct unode *un, char *buffer, int maxlen )
{
#if USE_NET
	int rc = 0;

#if USE_SSL
	if( un->sslh )
	{
#ifndef MBX
		if( VSSLBase )
#endif
			rc = VSSL_Read( un->sslh, buffer, maxlen );
#if USE_MIAMI
		else
			rc = SSL_read( un->sslh, buffer, maxlen );
#endif
	}
	else
#endif /* USE_SSL */
	{
		while( maxlen > 0 )
		{
			int len, trc;
			trc = recv( un->sock, &buffer[ rc ], maxlen, 0 );
			if( trc <= 0 )
				break;
			maxlen -= trc;
			rc += trc;

			if( maxlen < 0 )
				break;

			len = 0;
			IoctlSocket( un->sock, FIONREAD, (char*)&len );
			if( len <= 0 )
				break;
		}
	}
	return( rc );
#else
	return( 0 );
#endif /* !USE_NET */
}

int uns_pread( struct unode *un, APTR buffer, int maxlen )
{
	int rc;

	rc = recv( un->sock_pasv, buffer, maxlen, 0 );

	return( rc );
}

static int un_doconnect( struct unode *un )
{
	int rc;
	int l = sizeof( rc );

	if( !( un->sockstategot & SSW_SW ) )
		return( 0 );

	un->sockstate = 0;

#if USE_CONNECT_PROC

	rc = un->cmsg.rc;
	if( rc > 0 )
		rc = 0;

#else

#if USE_BLOCKING_CONN

	rc = un->connect_rc;
	if( rc > 0 )
		rc = 0;

#else
	// we got write permit selection
	// check error state

	getsockopt( un->sock, SOL_SOCKET, SO_ERROR, (char*)&rc, (long*)&l );

#endif // USE_BLOCKING_CONN

#endif // USE_CONNECT_PROC

	DL( DEBUG_CHATTY, db_net, bug( "un(%s) after connect rc %ld\n", un->url, rc ));

	if( rc )
	{
		makeneterror( un, GS( NWM_ERROR_NOCONNECT ), rc );
		un->state = UNS_FAILED;
		un_netclose( un );
	}
	else
	{
		// hm, we're done with the low level net setup
		// now let the protocol kick in...
		un->state = UNS_CONNECTED;
		sur_led( un, 2 );
	}

	return( -1 );
}

static int un_startnet( struct unode *un )
{
	static int ledcnt;
	struct sockaddr_in sockadr;

	// check for a parked connection...
	if( un_netunpark( un, *un->dcn->ip, un->port, &un->sock, &un->ledobjnum, un->retr_mode ) )
	{
		sur_led( un, 2 );
		// we found a parked connection
		// reuse it!
		un->state = UNS_CONNECTED;
		un->wasparked = TRUE;
		netinfo_url( un );
		return( TRUE );
	}

	if( activelinks >= gp_maxnetproc )
	{
		if( !un_netfreeparked() )
			return( FALSE );
	}

	// setup networking

	while( ledalloc[ ledcnt ] )
		ledcnt = ( ledcnt + 1 ) % gp_maxnetproc;

	un->ledobjnum = ledcnt;
	ledalloc[ ledcnt ] = TRUE;

	// get the guy a socket...
	if( !opennet() )
	{
		// shit, no networking available...
		strcpy( un->errorstring, GS( NWM_ERROR_NONET ) );
		un->errorcode = -2; // no net
		un->state = UNS_FAILED;
		makehtmlerror( un );
		return( TRUE );
	}

	un->sock = socket( AF_INET, SOCK_STREAM, 0 );

	if( un->sock < 0 )
	{
		// shit, no networking available...
		strcpy( un->errorstring, GS( NWM_ERROR_NOSOCK ) );
		un->errorcode = -3; // no socket
		un->state = UNS_FAILED;
		makehtmlerror( un );
		return( TRUE );
	}
	activelinks++;

	DL( DEBUG_INFO, db_net, bug( "un(%s) got socket %ld, now connecting (IP %lx, port %ld)\n", un->url, un->sock, *un->dcn->ip, un->port ));

	setblocking( un->sock, TRUE );
	sur_led( un, 1 );
	if( un->viaproxy )
		sur_text( un, GS( NETST_CONNECTING_PROXY ), un->dcn->name, ip2a( *un->dcn->ip ), un->port, un->purl.host );
	else
		sur_text( un, GS( NETST_CONNECTING ), un->dcn->name, ip2a( *un->dcn->ip ), un->port );

	netinfo_url( un );

	// now connect...
	sockadr.sin_len = sizeof( sockadr );
	memcpy( &sockadr.sin_addr, un->dcn->ip, 4 );
	sockadr.sin_family = AF_INET;
	sockadr.sin_port = un->port;

	/*
	 * We set a timeout for downloads
	 */
	if( un->timeout )
	{
		struct timeval tv;

		tv.tv_secs = un->timeout;
		tv.tv_micro = 0;

		setsockopt( un->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) );
	}

#if USE_CONNECT_PROC
	// Send connect() message to connect handler proc
	un->cmsg.m.mn_ReplyPort = connectreply;
	un->cmsg.sock = un->sock;
	un->cmsg.done = FALSE;
	un->cmsg_pending = 1;
	memcpy( &un->cmsg.addr, &sockadr, sizeof( sockadr ) );
	PutMsg( connectport[ un->sock % MAXNETPROC ], &un->cmsg );
	un->state = UNS_CONNECTING;
	un->sockstate = SSW_SW;
#else
#if USE_BLOCKING_CONN
	un->connect_rc = connect( un->sock, (struct sockaddr*)&sockadr, sizeof( sockadr ) );
	un->sockstategot |= SSW_SW;
	un_doconnect( un );
#else
	connect( un->sock, (struct sockaddr*)&sockadr, sizeof( sockadr ) );

	un->state = UNS_CONNECTING;
	un->sockstate = SSW_SW;
#endif

#endif // USE_CONNECT_PROC

	return( TRUE );
}


#endif


static void readfiletomem( struct unode *un )
{
#if USE_NETFILE
	ObtainSemaphore( &netpoolsem );
	if( !allocdata( un, un->doclen + 1 ) )
	{
#ifdef MBX
		un->docptr = ReadFile( un->fromfile, un->membuff, un->doclen );
#else
		un->docptr = Read( un->fromfile, un->membuff, un->doclen );
#endif
		if( un->docptr >= 0 )
		{
			((char*)un->membuff)[ un->docptr ] = 0;
		}
	}
	ReleaseSemaphore( &netpoolsem );
	Close( un->fromfile );
	un->fromfile = 0;
	un->state = UNS_DONE;
	sur_gauge_clear( un );
#endif
}

#if USE_DOS
static void readfiletofile( struct unode *un, struct nstream *ns )
{
	char buffer[ 512 ];
	int rc;

	while( ( rc = Read( un->fromfile, buffer, sizeof( buffer ) ) ) > 0 )
		WriteAsync( ns->tofile, buffer, rc );

	CloseAsync( ns->tofile );
	setcomment( ns->filename, ns->url );
	ns->tofile = 0;

	Close( un->fromfile );
	un->fromfile = NULL;
	un->state = UNS_DONE;

	ns->destmode = -1;
}
#endif /* USE_DOS */

static int un_dowaiting( struct unode *un )
{
	struct nstream *ns, *nsn;

	// at this point

	for( ns = FIRSTNODE( &un->clients ); nsn = NEXTNODE( ns ); ns = nsn )
	{
		if( ns->destmode )
		{
			DL( DEBUG_INFO, db_net, bug( "client has destmode %ld, notifying protocol\n", ns->destmode ));

			if( !un->offset )
			{
				un->offset = ns->offset; /* TOFIX: what happens with multiple clients ? */
			}

#if USE_DOS
			if( ns->destmode == DM_FILE )
			{
				char comment[ 80 ] = "failed: ";
				/* 404 error or so */
				if( nets_errorstring( ns )[ 0 ]	 )
				{
					DL( DEBUG_INFO, db_net, bug( "net error, aborting..\n" ) );
					un->state = UNS_FAILED;
					un->doinform = TRUE;
					return( 0 );
				}

#if 0
				/*
				 * Failed resume. TOFIX: handle it differently
				 */
				if ( ns->offset && un->range == -1 )
				{
					DL( DEBUG_INFO, db_net, bug( "resume failed, aborting..\n" ) );
					CloseAsync( ns->tofile );

					un->state = UNS_FAILED;
					un->doinform = TRUE;
					return ( 0 );

				}
#endif

				/*
				 * Resume
				 */
				if( un->offset /*|| un->range == -1*/ )
				{
					DL( DEBUG_INFO, db_net, bug( "resume in progress..\n" ) );
					CloseAsync( ns->tofile );

					/* if file is in the cache, seek correctly */
					if( un->fromfile )
					{
						DL( DEBUG_INFO, db_net, bug( "resuming from cache\n" ) );
						if( Seek( un->fromfile, ns->offset, OFFSET_CURRENT ) == -1 )
						{
							un->state = UNS_FAILED; // that shouldn't happen in 99.999% of the cases but..
							un->doinform = TRUE;
							return( 0 );
						}
					}
				}

				ns->tofile = OpenAsync( ns->filename, ns->offset ? MODE_APPEND : MODE_SHAREDWRITE, 4096 );

				if( !ns->tofile )
				{
					// Ouch, file open failed
					un->state = UNS_FAILED;
					un->doinform = TRUE;
					return( 0 );
				}

				un->tofilename = unstrdup( un, ns->filename );

				strncat( comment, ns->url, 71 );
				setcomment( ns->filename, comment );

				un->state = UNS_READING;
				if( un->fromfile )
				{
					readfiletofile( un, ns );
					return( 0 );
				}
			}
			else
#endif /* USE_DOS */
			{
				un->state = UNS_READING;
				un->tomem = 1;

#if USE_NETFILE
				if( un->fromfile )
				{
					readfiletomem( un );
					return( 0 );
				}
#endif
			}

#if USE_NET
			un_doprotocol( un );
			return( -1 );
#endif /* USE_NET */
		}
	}
	return( 0 );
}

/* starting loop for the unode */
static void processunode( struct unode *un )
{
	struct nstream *ns, *nsn;
	int informstate = 0;

rescanall:
	// check for closed clients
	for( ns = FIRSTNODE( &un->clients ); nsn = NEXTNODE( ns ); ns = nsn )
	{
		if( ns->removeme && ( un->state != UNS_LOOKINGUP ) )
		{
			DL( DEBUG_INFO, db_net, bug( "removing client (%s)\n", ns->url ? ns->url : "no url available"));

#if USE_DOS
			if( ns->tofile )
				CloseAsync( ns->tofile );
#endif

			if( ns->filename )
				nfree( ns->filename );

			REMOVE( ns );
			nfree( ns );
		}
	}

	if( un->net_abort )
	{
		if( un->state != UNS_DONE && un->state != UNS_FAILED )
		{
			if( !un->mimetype[ 0 ] || !strnicmp( un->mimetype, "text/", 5 ) )
			{
				if( !un->mimetype[ 0 ] )
					strcpy( un->mimetype, "text/html" );
				pushstring( un, "></TABLE></TABLE></TABLE></TABLE><CENTER><HR><FONT SIZE=4><B>*** TRANSFER ABORTED ***" );
			}
			un->state = UNS_FAILED;
			un->nocache = TRUE;
			un_netclose( un );
			goto dothatinform;
		}
	}

	if( ISLISTEMPTY( &un->clients ) && ( !un->membuff || un->errorcode || !un->keepinmem ) )
	{
		DL( DEBUG_INFO, db_net, bug( "removing un(%s) -> mb %lx, ec %ld, keep %ld\n", un->url, un->membuff, un->errorcode, un->keepinmem ));
		un_delete( un );
		return;
	}

rescan:
	DL( DEBUG_CHATTY, db_net, bug( "procunode(%s) state %ld ps %ld\n", un->url, un->state, un->protocolstate ));

	switch( un->state )
	{
		case UNS_SETUP:
			un_setup( un );
			goto rescan;
			break;

#if USE_NET
		case UNS_LOOKINGUP:
			if( un->dnsmsg.m.mn_Node.ln_Type == NT_REPLYMSG )
			{
				un_dnsdone( un );
				goto rescanall; /* don't continue if the operation was aborted, so we need to rescan everything */
			}
			break;

		case UNS_WAITINGFORNET:
			if( un_startnet( un ) )
				goto rescan;
			break;

		case UNS_CONNECTING:
			if( un_doconnect( un ) )
				goto rescan;
			break;

#endif /* USE_NET */

		case UNS_WAITING:
			if( !un_dowaiting( un ) )
				break;
			// fallthrough

#if USE_NET

		case UNS_CONNECTED:
		case UNS_READING:
			un_doprotocol( un );
			if( un->state == UNS_WAITING || un->state == UNS_WAITINGFORNET || un->state == UNS_DONE )
				goto rescan;
			break;

#endif /* USE_NET */

		case UNS_DONE:
			un_done( un );
			break;
	}

	DL( DEBUG_CHATTY, db_net, bug( "after procunode\n" ));

	/*
	 * XXX: beginning of trashing bug !!!!!!!!!!!!!!!!!
	 */

dothatinform:

	ASSERT( un );

	DL( DEBUG_CHATTY, db_net, bug( "procunode bug part 1\n" ));

	if( un->state == UNS_WAITING )
		informstate = 1;
	else if( un->state == UNS_READING )
		informstate = 2;
	else if( un->state == UNS_FAILED || un->state == UNS_DONE )
		informstate = 3;

	DL( DEBUG_CHATTY, db_net, bug( "procunode bug part 2\n" ));

	// now, see if we have to inform clients
	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		// when we don't have a destmode, don't increase state
		if( ns->removeme || ( ns->informstate == 1 && !ns->destmode ) )
			continue;

		DL( DEBUG_CHATTY, db_net, bug( "icl: %ld/%ld doinfo %ld iobj %lx\n", ns->informstate, informstate, un->doinform, ns ));
		while( ns->informstate < informstate || ( ns->informstate == 2 && un->doinform ) )
		{
			if( informstate > ns->informstate )
				ns->informstate++;
			if( ns->informobj )
			{
				DL( DEBUG_CHATTY, db_net, bug( "sending inform %lx to %lx\n", ns->informstate, ns->informobj ));
				if( ns->informobj == (APTR)-1 )
				{
					imgdec_tick();
				}
				else
				{
					if( !ns->removeme )
					{
						ASSERT( ns->informobj );
						ASSERT( ns->un );
						
						pushmethod( ns->informobj,
									4,
									MM_NStream_GotInfo + ns->informstate - 1,
									ns,
									( ns->flags & NOF_TIMESTAMP ) ? timedm() : 0,
									ns->un->docptr + ns->offset /* TOFIX: the flag is useless, really */
						);
					}
				}
			}
			if( ns->informstate == 1 && !ns->destmode )
				break;
			if( ns->informstate == 2 && un->doinform )
				break;
		}
	}
	DL( DEBUG_CHATTY, db_net, bug( "procunode bug part 3\n" ));
	
	ASSERT( un );
	DL( DEBUG_CHATTY, db_net, bug( "procunode bug part 3\n" ));
	un->doinform = FALSE;
	
	/*
	 * XXX: end of trashing bug
	 */

	DL( DEBUG_CHATTY, db_net, bug( "exiting pcu\n" ));
}

static void SAVEDS nethandler( void )
{
	ULONG psig, dnssig;
	struct nstream *ns;
	struct unode *un, *nun;
	int c;
	int memsize;
	int netclose_requested = FALSE;
	int activenodes = 0;
	int activelinksbefore;
	ULONG connectreplysig = 0;
#if USE_EXECUTIVE
	APTR executivemsg;

	executivemsg = InitExecutive();
	if( executivemsg )
	{
		SetNice( executivemsg, -20 ); /* TOFIX! this should be configurable one day */
		ExitExecutive( executivemsg );
	}
#endif /* USE_EXECUTIVE */

	NEWLIST( &ulist );
	NEWLIST( &sslcertlist );

	InitSemaphore( &unlistsem );
	netport = CreateMsgPort();
	if( !netport )
	{
		netproc = NULL;
		return;
	}

#if USE_NET
	dnsreply = CreateMsgPort();
	dnssig = 1L<<dnsreply->mp_SigBit;
#else /* !USE_NET  */
	dnssig = 0;
#endif /* USE_NET */

	psig = 1L<<netport->mp_SigBit;

#if USE_CONNECT_PROC
	connectreply = CreateMsgPort();
	connectreplysig = 1L<<connectreply->mp_SigBit;
#endif

	while( !nethandler_die )
	{
		ULONG sig;
#if USE_NET
		fd_set fdrset, fdwset;
		int maxsock = -1;

		FD_ZERO( &fdrset );
		FD_ZERO( &fdwset );

		DL( DEBUG_CHATTY, db_net, bug( "entering loop (free: %ld/%ld)\n", AvailMem(0), AvailMem(MEMF_LARGEST) ));

		for( un = FIRSTNODE( &ulist ); NEXTNODE( un ); un = NEXTNODE( un ) )
		{
			un->sockstategot = 0;

			// DEBUGGING
#if VDEBUG
			if( un->sockstate & ( SSW_SR | SSW_SW ) )
			{
				if( un->sock < 0 )
				{
					ALERT( ( "un(%s) sockstate %ld, sock %ld\n", un->url, un->sockstate, un->sock ) );
					continue;
				}
			}
			if( un->sockstate & ( SSW_PSR | SSW_PSW ) )
			{
				if( un->sock_pasv < 0 )
				{
					ALERT( ( "un(%s) sockstate %ld, sock_pasv %ld\n", un->url, un->sockstate, un->sock_pasv ) );
					continue;
				}
			}
#endif /* VDEBUG */

			if( un->sockstate & SSW_SR )
			{
				FD_SET( un->sock, &fdrset );
				maxsock = max( un->sock, maxsock );
			}
			if( un->sockstate & SSW_SW )
			{
				FD_SET( un->sock, &fdwset );
				maxsock = max( un->sock, maxsock );
			}
			if( un->sockstate & SSW_PSR )
			{
				FD_SET( un->sock_pasv, &fdrset );
				maxsock = max( un->sock_pasv, maxsock );
			}
			if( un->sockstate & SSW_PSW )
			{
				FD_SET( un->sock_pasv, &fdwset );
				maxsock = max( un->sock_pasv, maxsock );
			}
		}

		if( parkcount > 0 )
		{
			for( c = 0; c < MAXNETCONN; c++ )
			{
				if( parked_ips[ c ] )
				{
					if( parked_sockets[ c ] < 0 )
					{
						ALERT( ( "parked_socket(%d)==%d\n", c, parked_sockets[ c ] ) );
					}
					FD_SET( parked_sockets[ c ], &fdrset );
					maxsock = max( parked_sockets[ c ], maxsock );
				}
			}
		}

		DL( DEBUG_CHATTY, db_net, bug( "maxsock %ld parkcount %ld\n", maxsock, parkcount ));

		if( maxsock >= 0 )
		{
			int rc;
			ULONG sig = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | psig | dnssig | connectreplysig;
#ifndef MBX
			struct timeval tv;

			tv.tv_secs = 5;
			tv.tv_micro = 0;
#else
			timeval_s tv;

			tv.tv_secs = 5;
			tv.tv_usec = 0;
#endif
			rc = WaitSelect( maxsock + 1, &fdrset, &fdwset, NULL, &tv, &sig );

			if( rc < 0 || ( sig & SIGBREAKF_CTRL_C ) )
			{
#ifdef MBX
				if( rc < 0 )
					kprintf( "got rc = %d errno=%d\n", rc, Errno() );
#endif
				netclose_requested = TRUE;
			}
			else if( rc > 0 )
			{
				// parse sets for gotten bits
				for( un = FIRSTNODE( &ulist ); NEXTNODE( un ); un = NEXTNODE( un ) )
				{
					if( un->sock >= 0 )
					{
						if( FD_ISSET( un->sock, &fdrset ) )
							un->sockstategot |= SSW_SR;
						if( FD_ISSET( un->sock, &fdwset ) )
							un->sockstategot |= SSW_SW;
					}
					if( un->sock_pasv >= 0 )
					{
						if( FD_ISSET( un->sock_pasv, &fdrset ) )
							un->sockstategot |= SSW_PSR;
						if( FD_ISSET( un->sock_pasv, &fdwset ) )
							un->sockstategot |= SSW_PSW;
					}
					if( un->sockstategot )
					{
						DL( DEBUG_CHATTY, db_net, bug( "un(%s) got sock bits %lx/%lx %lx/%lx\n", un->url, un->sockstategot & SSW_SR, un->sockstategot & SSW_SW, un->sockstategot & SSW_PSR, un->sockstategot & SSW_PSW ));
					}

#if USE_CONNECT_PROC
					if( un->cmsg_pending == 1 && un->cmsg.done )
					{
						un->cmsg_pending = 0;
						un->cmsg.done = FALSE;
						un->sockstategot |= SSW_SW;
					}
					if( un->cmsg_pending == 2 && un->cmsg.done )
					{
						un->cmsg_pending = 0;
						un->cmsg.done = FALSE;
						un->sockstategot |= SSW_PSW;
					}
#endif
				}

				// check parked connections for closures
				if( parkcount > 0 )
				{
					for( c = 0; c < MAXNETCONN; c++ )
					{
						if( parked_ips[ c ] )
						{
							if( FD_ISSET( parked_sockets[ c ], &fdrset ) )
							{
								DL( DEBUG_CHATTY, db_net, bug( "closing parked connection to %lx/%ld (sock %ld)\n", parked_ips[ c ], parked_ports[ c ], parked_sockets[ c ] ));
								parked_ips[ c ] = 0;
								CloseSocket( parked_sockets[ c ] );

								setled( parked_leds[ c ], 0 );
								ledalloc[ parked_leds[ c ] ] = FALSE;
								netinfo_clear( parked_leds[ c ] );
								activelinks--;
								parkcount--;
							}
						}
					}
				}
			}
		}
		else
		{
#endif /* USE_NET */
			sig = Wait( SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | psig | dnssig );
			if( sig & SIGBREAKF_CTRL_C )
			{
				netclose_requested = TRUE;
			}
#if USE_NET
		}
#endif /* USE_NET */

		DL( DEBUG_CHATTY, db_net, bug( "after wait\n" ));

		now = timed();

		ObtainSemaphore( &unlistsem );

		for(;;)
		{
			activelinksbefore = activelinks;
			activenodes = 0;
			for( un = FIRSTNODE( &ulist ); nun = NEXTNODE( un ); un = nun )
			{
				if( netclose_requested )
					un->net_abort = TRUE;
				processunode( un );
			}

			// WARNING!
			// This CANNOT be folded into above loop,
			// since processunode may DESTROY the unode!
			for( un = FIRSTNODE( &ulist ); nun = NEXTNODE( un ); un = nun )
			{
				if( un->state != UNS_DONE && un->state != UNS_FAILED )
					activenodes++;
			}

			if( activelinks >= activelinksbefore )
				break;
		}

		if( netclose_requested && !activenodes )
		{
			// check whether we can shut down networking

#if USE_NET
#if USE_SSL
			closessl();
#endif
#ifndef MBX
			if (SocketBase)
			{
				CloseLibrary( SocketBase );
				SocketBase = NULL;
			}
#else
			if (NetBase)
			{
				CloseModule( &NetBase->net_Module );
				NetBase = NULL;
			}
#endif

#if USE_MIAMI
			if (MiamiBase)
			{
				CloseLibrary( MiamiBase );
				MiamiBase = NULL;
			}
#endif /* USE_MIAMI */
			netopen = FALSE;
#endif /* USE_NET */
			netclose_requested = 0;
		}

		DL( DEBUG_CHATTY, db_net, bug( "after pu\n" ));

		memsize = 0;

		for( un = FIRSTNODE( &ulist ); nun = NEXTNODE( un ); un = nun )
		{
			memsize += un->membuffsize;
		}

		if( memsize > gp_memcachesize )
		{
			DL( DEBUG_INFO, db_net, bug( "memsize %ld memcachesize %ld, FLUSHING nodes\n", memsize, gp_memcachesize ));
			for( un = FIRSTNODE( &ulist ); memsize > gp_memcachesize && ( nun = NEXTNODE( un ) ); un = nun )
			{
				if( ISLISTEMPTY( &un->clients ) )
				{
					DL( DEBUG_INFO, db_net, bug( "flushing %s\n", un->url ));
					memsize -= un->membuffsize;
					un_delete( un );
				}
			}
		}

		while( ns = (struct nstream*)GetMsg( netport ) )
			addstream( ns );

		ReleaseSemaphore( &unlistsem );

		DL( DEBUG_CHATTY, db_net, bug( "after adds\n" ));

#if USE_NET
		while( GetMsg( dnsreply ) );
#endif
#if USE_CONNECT_PROC
		while( GetMsg( connectreply ) );
#endif
	}

	DL( DEBUG_CHATTY, db_net, bug( "leaving main loop\n" ));

#if USE_NET
	if( !nethandler_die )
	{
		while( GetMsg( dnsreply ) );
	}
#endif /* USE_NET */

	DL( DEBUG_CHATTY, db_net, bug( "removing remaining unodes\n" ));

	// remove any remaining unodes
	for(;;)
	{
		un = FIRSTNODE( &ulist );
		if( !NEXTNODE( un ) )
			break;
		DL( DEBUG_CHATTY, db_net, bug( "removing unode %lx (%s)\n", un, un->url ));
		un_delete( un );
	}

	DL( DEBUG_CHATTY, db_net, bug( "before closing ssl\n" ));

#if USE_NET
#if USE_SSL
	closessl();
#endif

	DL( DEBUG_CHATTY, db_net, bug( "after closing SSL\n" ));

#ifndef MBX
	CloseLibrary( SocketBase );
#else
	CloseModule( &NetBase->net_Module );
#endif
#if USE_MIAMI
	CloseLibrary( MiamiBase );
#endif /* USE_MIAMI */

	DeleteMsgPort( dnsreply );
#endif /* USE_NET */
	DeleteMsgPort( netport );

	DL( DEBUG_CHATTY, db_net, bug( "done, zeroing netproc and leaving\n" ));

	netproc = NULL;
}

int init_netprocess( void )
{
	int c;
	char name[ 32 ];

	D( db_init, bug( "initializing..\n" ) );

	InitSemaphore( &netpoolsem );
	if( netpool = CreatePool( 0, 4096, 2048 ) )
	{
#if USE_NET
		for( c = 0; c < DNSTASKS; c++ )
		{
			sprintf( name, "V's DNS Server %d", c + 1 );

			dnsproc[ c ] = CreateNewProcTags(
				NP_Entry, dnshandler,
				NP_Name, name,
				NP_Priority, 2,
				NP_Cli, FALSE,
				#ifdef __MORPHOS__
				NP_StackSize, 10 * 1024 * 2,
				NP_CodeType, CODETYPE_PPC,
				NP_TaskMsgPort, &dnsport[ c ],
				#else
				NP_StackSize, 10 * 1024,
				#endif
				TAG_DONE
			);

			#ifndef __MORPHOS__
			while( dnsproc[ c ] && !dnsport[ c ] )
				Delay( 1 );
			#endif

			if( !dnsproc[ c ] )
				return( FALSE );
		}

#if USE_CONNECT_PROC
		for( c = 0; c < MAXNETPROC; c++ )
		{
			sprintf( name, "V's connect() Handler %02d", c + 1 );
			connectproc[ c ] = CreateNewProcTags(
				NP_Entry, connecthandler,
				NP_Name, name,
				NP_Priority, 2,
				NP_Cli, FALSE,
				NP_StackSize, 10 * 1024,
				TAG_DONE
			);
			while( connectproc[ c ] && !connectport[ c ] )
				Delay( 1 );
			if( !connectproc[ c ] )
				return( FALSE );
		}
#endif

		// allocate buffers for hostnames and FTP passwords
		for( c = 0; c < MAXNETPROC; c++ )
		{
			parked_pwds[ c ] = nalloc( PARK_FTP_PASSWORD_SIZE );
			parked_hosts[ c ] = nalloc( PARK_HOSTNAME_SIZE );
		}

#endif /* USE_NET */

		netproc = CreateNewProcTags(
			NP_Entry, nethandler,
			NP_Name, "V's Network & File Server",
			NP_Priority, 0,
			NP_Cli, FALSE,
			#ifdef __MORPHOS__
			NP_StackSize, 32 * 1024 * 2,
			NP_CodeType, CODETYPE_PPC,
			#else
			NP_StackSize, 32 * 1024,
			#endif
			TAG_DONE
		);

		while( netproc && !netport )
			Delay( 1 );

		if( !netproc )
			return( FALSE );

#if USE_NET
		proxy_init();
#endif /* USE_NET */

		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

void cleanup_netprocess( void )
{
	int done = FALSE;
	int first = TRUE;

	D( db_init, bug( "cleaning up..\n" ) );

	if( netpool )
	{
		Forbid();

#if USE_NET
		dnshandler_die = TRUE;

		while( !done )
		{
			int c;

			done = TRUE;
			for( c = 0; c < DNSTASKS; c++ )
			{
				D( db_init, bug( "dnsproc[%ld]=%lx\n", c, dnsproc[ c ] ) );
				if( dnsproc[ c ] )
				{
					Signal( ( struct Task * )dnsproc[ c ], SIGBREAKF_CTRL_C );
					done = FALSE;
				}
			}
			if( !done )
				Delay( first ? 1 : 5 );
			first = FALSE;
		}
#endif /* USE_NET */

#if USE_CONNECT_PROC
		connecthandler_die = TRUE;

		while( !done )
		{
			int c;

			done = TRUE;
			for( c = 0; c < MAXNETPROC; c++ )
			{
				if( connectproc[ c ] )
				{
					Signal( connectproc[ c ], SIGBREAKF_CTRL_C );
					done = FALSE;
				}
			}
			if( !done )
				Delay( 1 );
		}
#endif /* USE_NET */

		nethandler_die = TRUE;
		first = TRUE;

		while( netproc )
		{
			D( db_init, bug( "netproc %lx\n", netproc ) );
			Signal( ( struct Task * )netproc, SIGBREAKF_CTRL_C );
			if( !netproc )
				break;
			DL( DEBUG_WARNING, db_net, bug( "waiting to kill netproc\n" ));
			Delay( first ? 1 : 5 );
			first = FALSE;
		}

		Permit();

		DeletePool( netpool );
	}
}

int nets_flushmem( void )
{
#if USE_DOS
	struct unode *un, *nun;
	int didsomething = FALSE;

	DL( DEBUG_INFO, db_net, bug( "entering memhandler" ) );

//kprintf("NET:entering memhandler\n" );

	ObtainSemaphore( &unlistsem );

	for( un = FIRSTNODE( &ulist ); nun = NEXTNODE( un ); un = nun )
	{
		DL( DEBUG_INFO, db_net, bug( "MEMFLUSH; checking %s\n", un->url ));
//kprintf("NET:checking %s\n", un->url );
		if( un->state == UNS_DONE || un->state == UNS_FAILED )
		{
			if( ISLISTEMPTY( &un->clients ) )
			{
				if( !un->l && !un->fromfile )
				{
					DL( DEBUG_INFO, db_net, bug( "MEMFLUSH: !disposing %s\n", un->url ));
//kprintf("NET:disposing\n" );
					REMOVE( un );
					DeletePool( un->pool );
					didsomething = TRUE;
				}
			}
		}
	}

	ReleaseSemaphore( &unlistsem );

	DL( DEBUG_INFO, db_net, bug( "exiting memhandler" ) );

//kprintf("NET:exiting memhandler\n" );

	return( didsomething );
#else
	return( FALSE );
#endif
}

#define URLCACHE_MEMHANDLER_NAME "V's URL Cache Low Mem Handler"
#define URLCACHE_MEMHANDLER_PRI 15

#ifdef MBX
static STATUS memhandlerfunc(struct SystemData*sysbase,struct MemHandler*mh,struct MemHandlerData*mhd);

static MemHandler_p urlcache_memhandler;
#endif /* MBX */
#ifdef AMIGAOS
static int ASM SAVEDS memhandlerfunc( __reg( a0, struct MemHandlerData *mhd ) );

typedef void (*INTFUNC)();

static __far struct Interrupt memhandlerint = {
	0, 0,
	NT_INTERRUPT,
	URLCACHE_MEMHANDLER_PRI,
	URLCACHE_MEMHANDLER_NAME,
	NULL,
	(INTFUNC)memhandlerfunc
};
static int memhandleractive;
#endif /* AMIGAOS */

#ifdef __MORPHOS__
static int memhandlerfunc( void );

struct IntData
{
	struct EmulLibEntry InterruptFunc;
	struct Interrupt Interrupt;
	struct ExecBase *SysBase;
} mosintdata;
static int memhandleractive;
#endif /* __MORPHOS__ */
 

#ifdef MBX
static STATUS memhandlerfunc(struct SystemData*sysbase,struct MemHandler*mh,struct MemHandlerData*mhd)
#endif /* MBX */
#ifdef AMIGAOS
static int ASM SAVEDS memhandlerfunc( __reg( a0, struct MemHandlerData *mhd ) )
#endif /* AMIGAOS */
#ifdef __MORPHOS__
static int memhandlerfunc( void )
#endif /* __MORPHOS__ */
{
	int didsomething = MEM_DID_NOTHING;

#ifdef __MORPHOS__
	struct ExecBase *SysBase = mosintdata.SysBase;
#endif /* __MORPHOS__ */

	// try to get hold of the list semaphore (to be save)
	DL( DEBUG_INFO, db_net, bug( "MEMFLUSH: entering...\n" ));
	//if( netproc != (struct Process*)FindTask( 0 ) && AttemptSemaphore( &unlistsem ) )
//kprintf("NET: memhandler -- intro\n" );
	if( AttemptSemaphore( &unlistsem ) )
	{
		DL( DEBUG_INFO, db_net, bug( "MEMFLUSH: got semaphore!\n" ));
//kprintf( "NET:got semaphore\n" );
		if( nets_flushmem() )
			didsomething = MEM_ALL_DONE;
		ReleaseSemaphore( &unlistsem );
	}
//kprintf("NET: memhandler -- exit rc %ld, free %ld/%ld\n", didsomething, AvailMem(0),AvailMem(MEMF_LARGEST) );
	return( didsomething );
}


void init_memhandler( void )
{
	// setup memhandler
	D( db_init, bug( "initializing..\n" ) );

#ifdef MBX
	urlcache_memhandler = AddMemHandler( URLCACHE_MEMHANDLER_NAME, URLCACHE_MEMHANDLER_PRI, memhandlerfunc, 0 );
#endif /* MBX */
#ifdef AMIGAOS
	AddMemHandler( &memhandlerint );
	memhandleractive = TRUE;
#endif /* AMIGAOS */
#ifdef __MORPHOS__
	mosintdata.SysBase = SysBase;
	mosintdata.InterruptFunc.Trap = TRAP_LIB;
	mosintdata.InterruptFunc.Extension = 0;
	mosintdata.InterruptFunc.Func = ( void ( * )( void ) )memhandlerfunc;
	mosintdata.Interrupt.is_Node.ln_Type = NT_INTERRUPT;
	mosintdata.Interrupt.is_Node.ln_Pri = URLCACHE_MEMHANDLER_PRI;
	mosintdata.Interrupt.is_Node.ln_Name = URLCACHE_MEMHANDLER_NAME;
	mosintdata.Interrupt.is_Data = &mosintdata;
	mosintdata.Interrupt.is_Code = ( void ( * )( void ) )&mosintdata.InterruptFunc;

	AddMemHandler( &mosintdata.Interrupt );
	memhandleractive = TRUE;
#endif /* __MORPHOS__ */
}

void cleanup_memhandler( void )
{
	D( db_init, bug( "cleaning up..\n" ) );
#ifdef MBX
	if( urlcache_memhandler )
	{
		RemMemHandler( urlcache_memhandler );
	}
#endif /* MBX */
#ifdef AMIGAOS
	if( memhandleractive )
	{
		RemMemHandler( &memhandlerint );
	}
#endif /* AMIGAOS */
#ifdef __MORPHOS__
	if( memhandleractive )
	{
		RemMemHandler( &mosintdata.Interrupt );
	}
#endif /* __MORPHOS__ */
}

//
// API functions
//

/*
 * Checks if the given string only contain spaces
 */
int is_only_spaces_remaining( char *str )
{
	while( *str )
	{
		if( *str++ != ' ' )
		{
			return( FALSE );
		}
	}
	return( TRUE );
}

/*
 * Sends a message to the network process to tell it to download a file and update some objects
 *
 * TOFIX: NOF_ADDURL isn't handled yet it seems ? hu ?
*/
APTR ASM SAVEDS nets_open(
	__reg( a0, STRPTR url ),
	__reg( a1, STRPTR referer ),
	__reg( a2, APTR informobj ),
	__reg( a3, APTR gauge ),
	__reg( a4, APTR txtstatus ),
	__reg( d0, ULONG timeout ),
	__reg( d1, ULONG flags )
 )
{
	struct nstream *ns;
	int size = sizeof( *ns );
	char *p, *p2;
	STRPTR nurl = NULL;
	ULONG cnt = 0;

	DL( DEBUG_INFO, db_net, bug( "nets_open(%s,%s,...)\n", url, referer ? referer : (STRPTR)"NULL" ));

	/*
	 * Workaround against bogus URL with real spaces in them.
	 * Replaces them by %20 and strip any remaining empty spaces
	 * at the end
	 */
	p = url;

	while( *p )
	{
		if( *p++ == ' ' )
			cnt++;
	}

	if( cnt )
	{
		STRPTR q;

		q = nurl = nalloc(strlen( url ) + 1 + 2 * cnt );

		if( nurl )
		{
			p = url;

			while( *p )
			{
				if( *p == ' ' )
				{
					if( !is_only_spaces_remaining( p ) )
					{
						*q++ = '%';
						*q++ = '2';
						*q++ = '0';
					}
				}
				else
				{
					*q++ = *p;
				}
				p++;
			}
			*q = '\0';
		}
	}
	size += ( strlen( nurl ? nurl : url ) + 1 ) * 2;
	if( referer )
		size += strlen( referer ) + 1;

	ns = nalloc( size );
	if( !ns )
		return( NULL );

	memset( ns, 0, sizeof( *ns ) );
	ns->url = (char*)( ns + 1 );
	strcpy( ns->url, nurl ? nurl : url );
	ns->fullurl = strchr( ns->url, 0 ) + 1;
	strcpy( ns->fullurl, ns->url );

	if( nurl )
	{
		nfree( nurl );
	}

	if( referer )
	{
		char *chk;
		ns->referer = ns->fullurl + strlen( ns->fullurl ) + 1;
		strcpy( ns->referer, referer );
		// Remove possible passwords
		chk=strstr(ns->referer,"//");
		if(chk)
		{
			char *chk2=strchr(chk,'/');
			if(chk2)
			{
				char *chk3=strchr(chk,'@');
				if(chk3&&chk<chk2)
				{
					// copy stuff after @ to //, removing username and password
					strcpy(chk+2,chk+1);
				}
			}
		}
	}

	p = strchr( ns->url, '#' );
	if( p )
	{
		*p++ = 0;
		p2 = strrchr( p, '?' );
		if( p2 )
		{
			if( p2[ 1 ] == '{' )
				strcpy( p - 1, p2 );
		}
	}

	ns->informobj = informobj;
	ns->gaugeobj = gauge;
	ns->statusobj = txtstatus;
	ns->timeout = timeout;
	ns->flags = flags;

	PutMsg( netport, ( struct Message * )ns );

	return( ns );
}

int ASM SAVEDS nets_state( __reg( a0, struct nstream *ns ) )
{
	if( ns->un )
	{
		if( ns->un->state == UNS_DONE )
			return( 1 );
		else if( ns->un->state == UNS_FAILED )
			return( -1 );
	}
	return( 0 );
}

void ASM SAVEDS nets_close( __reg( a0, struct nstream *ns ) )
{
	ns->removeme = TRUE;
	// TEMP!
	//ns->un = (APTR)1; /* XXX: TEMP? WTF is that? [zapek]
	// let netproc rescan its client list
	Signal( ( struct Task * )netproc, SIGBREAKF_CTRL_D );
}


/*
 * Returns the mimetype of a given nstream
 */
char *nets_mimetype( struct nstream *ns )
{
	if( ns->un )
		return( ns->un->mimetype );
	else
		return( "" );
}


/*
 * Returns the extension of a file
 */
char *nets_mimeextension( struct nstream *ns )
{
	if( ns->un )
	{
		char *ext;

		if( ext = strrchr( ns->un->purl.path, '.' ) )
		{
			return( ext );
		}
	}
	return( 0 );
}


char * ASM SAVEDS nets_getdocmem( __reg( a0, struct nstream *ns ) )
{
	if( ns->un )
	{
		DL( DEBUG_CHATTY, db_net, bug( "gdm (%s)\n", ns->un->membuff ));
		return( ns->un->membuff );
	}
	else
		return( NULL );
}

int ASM SAVEDS nets_getdocptr( __reg( a0, struct nstream *ns ) )
{
	if( ns->un )
		return( ns->un->docptr );
	else
		return( 0 );
}

/* same as above but getting the resume possibility into account */
int SAVEDS nets_getdocrealptr( struct nstream *ns )
{
	if( ns->un )
		return( (int)ns->un->docptr + (int)ns->offset );
	else
		return( 0 );
}

int nets_getdoclen( struct nstream *ns )
{
	if( ns->un )
	{
		return( ns->un->doclen );
	}
	else
		return( 0 );
}


void ASM SAVEDS nets_settomem( __reg( a0, struct nstream *ns ) )
{
	ns->destmode = DM_MEMORY;
	// notify net proc
	if( ns->un )
		ns->un->newclientdestmode++;
	Signal( ( struct Task * )netproc, SIGBREAKF_CTRL_D );
}

/* redirects a http stream to a file */
void nets_settofile( struct nstream *ns, STRPTR filename, ULONG offset )
{
	DL( DEBUG_INFO, db_net, bug( "setting URL %s to file %s (resume offset == %ld)\n", ns->url, filename, offset ) );
	ns->filename = nalloc( strlen( filename ) + 1 );
	if( ns->filename )
		strcpy( ns->filename, filename );
	ns->destmode = DM_FILE;
	ns->offset = offset;

	// notify net proc
	if( ns->un )
	{
		ns->un->newclientdestmode++;
		sur_gauge_clear( ns->un );
	}

	Signal( ( struct Task * )netproc, SIGBREAKF_CTRL_D );
}

char * ASM SAVEDS nets_redirecturl( __reg( a0, struct nstream *ns ) )
{
	if( ns->un )
		return( ns->un->redirecturl );
	else
		return( 0 );
}

void ASM SAVEDS nets_lockdocmem( void )
{
	ObtainSemaphore( &netpoolsem );
}

void ASM SAVEDS nets_unlockdocmem( void )
{
	ReleaseSemaphore( &netpoolsem );
}

void nets_abort( struct nstream *ns )
{
	Forbid();
	if( ns->un )
		ns->un->net_abort = TRUE;
	Permit();
}

char * ASM SAVEDS nets_url( __reg( a0, struct nstream *ns ) )
{
	return( ns->url );
}

char * ASM SAVEDS nets_fullurl( __reg( a0, struct nstream *ns ) )
{
	return( ns->fullurl );
}

char * nets_referer( struct nstream *ns )
{
	return( ns->referer );
}

void nets_setiobj( struct nstream *ns, APTR gauge, APTR txt )
{
	ns->gaugeobj = gauge;
	ns->statusobj = txt;
}

char ASM *nets_errorstring( __reg( a0, struct nstream *ns ) )
{
	if( ns->un )
		return( ns->un->errorstring );
	else
		return( "" );
}

int nets_issecure( struct nstream *ns )
{
	if( ns->un )
#if USE_SSL
		return( ns->un->ssl );
#else
		return( FALSE );
#endif /* !USE_SSL */
	else
		return( FALSE );
}

int nets_sourceid2( struct unode *un )
{
	if( !un )
		return( 0 );

	if( un->fromcache )
		return( 1 ); // from (disk) cache

	switch( un->retr_mode )
	{
		case RM_HTTP:
			if( un->trying11 )
				return( 2 );
			else
				return( 3 );

		case RM_FTP:
			return( 4 );

		case RM_FILE:
			return( 1 );

		default:
			return( 0 ); // probably internal
	}

}

/* little wrapper :) erm, the above might not be needed now btw */
int nets_sourceid( struct nstream *ns )
{
	return( nets_sourceid2( ns->un ) );
}

void nets_setdocmem( struct nstream *ns, APTR data, int len )
{
	if( len < 0 )
		len = strlen( data ) + 1;

	if( ns->un )
	{
		nets_lockdocmem();
		ns->un->membuffptr = 0;
		allocdata( ns->un, len  );
		if( ns->un->membuff )
		{
			CopyMem( data, ns->un->membuff, len );
		}
		nets_unlockdocmem();
	}
}

// tell V to give up the memory
void ASM SAVEDS nets_release_buffer( __reg( a0, struct nstream *ns ) )
{
	if( ns->un )
		ns->un->keepinmem = FALSE;
}

char ASM SAVEDS *nets_getheader( __reg( a0, struct nstream *ns ), __reg( a1, char *header ) )
{
	if( ns->un )
		return( ungetheaderdata( ns->un, header ) );
	else
		return( NULL );
}
