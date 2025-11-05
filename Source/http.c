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
** HTTP
** ----
**
**
** $Id: http.c,v 1.92 2004/01/06 20:23:08 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "copyright.h"
#include "voyager_cat.h"
#include "network.h"
#include "dns.h"
#include "http.h"
#include "cache.h"
#include "cookies.h"
#include "certs.h"
#include "authipc.h"
#include "host_os.h"
#include "prefs.h"
#include "form.h"
#include "mime.h"
#include "time_func.h"
#include "netinfo.h"
#include "malloc.h"
#include "dos_func.h"
#include "mui_func.h"
#include "lineread.h"
#include "sur_gauge.h"


#define VACCEPT \
"Accept: text/html;level=3\r\n"\
"Accept: text/html;version=3.0\r\n"\
"Accept: */*\r\n"

char vuseragent[ 128 ];
void SAVEDS setup_useragent( void )
{
#ifdef MBX
	strcpy( vuseragent, "Met@box1000-Browser/" VERSIONSTRING );
#else
	if( !gp_spoof )
		sprintf( vuseragent, "AmigaVoyager/" VERSIONSTRING " (%s/%s)", hostos, cpuid );
	else
		strcpy( vuseragent, getprefsstr( DSI_NET_SPOOF_AS_1 + gp_spoof - 1, "(none)" ) );
#endif
}


#if USE_NET

/*
 * Setups the HTTP request.
 */
void un_setup_http( struct unode *un, char *proxy_host, int proxy_port )
{
	char *host;
	struct dnscachenode *dcn;
	// use a proxy?

	un->retr_mode = RM_HTTP;

	sur_gauge_reset( un );

	if( proxy_host )
	{
		host = proxy_host;
		un->port = proxy_port;
		un->viaproxy = TRUE;
	}
	else
	{
		host = un->purl.host;
		un->port = un->purl.port;
	}

	if( !host )
		host = "undefined";

	DL( DEBUG_INFO, db_http, bug( "setup_http(%s) host: %s viaproxy: %ld\r\n", un->url, host, un->viaproxy ));

	/* find if the ip is already in the cache */
	dcn = dnscache_find( host );
	if( dcn )
	{
		un->dcn = dcn;
		un->state = UNS_WAITINGFORNET;
		DL( DEBUG_CHATTY, db_http, bug( "waiting for net\r\n" ));
	}
	else
	{
		un->dnsmsg.m.mn_ReplyPort = dnsreply;
		un->dnsmsg.name = host;
		dnsmsg_queue( &un->dnsmsg );
		un->state = UNS_LOOKINGUP;
		sur_text( un, GS( NETST_LOOKINGUP ), host );
	}
}

void un_setup_https( struct unode *un )
{
	char *host;
	struct dnscachenode *dcn;
	// use a proxy?

	un->retr_mode = RM_HTTP;
	un->ssl = TRUE;

	sur_gauge_reset( un );

	host = un->purl.host;
	un->port = un->purl.port;

	DL( DEBUG_INFO, db_http, bug( "setup_http(%s) host %s vp %ld\r\n", un->url, host, un->viaproxy ));

	dcn = dnscache_find( host );
	if( dcn )
	{
		un->dcn = dcn;
		un->state = UNS_WAITINGFORNET;
		DL( DEBUG_CHATTY, db_http, bug( "waiting for net\r\n" ));
	}
	else
	{
		un->dnsmsg.m.mn_ReplyPort = dnsreply;
		un->dnsmsg.name = host;
		dnsmsg_queue( &un->dnsmsg );
		un->state = UNS_LOOKINGUP;
		sur_text( un, GS( NETST_LOOKINGUP ), host );
	}
}

/*
 * Builds up and sends the HTTP request.
 */
static void un_doprotocol_http_req( struct unode *un )
{
	char buffer[ 8192 ], *bp; /* XXX: possible overflow here ? yes. we should rewrite that handling.. sucks */
	int len;
	int formlen = 0;
	char *formdata = 0; // Make GCC happy

	// assemble send initial request

	/*
	 * If we're doing SSL-proxying, we need to
	 * "CONNECT" first.
	 */

#if USE_SSL
	if( un->ssl == 1 && un->viaproxy )
	{
		snprintf( buffer, sizeof(buffer), "CONNECT %s:%d HTTP/1.0\r\nUser-Agent: %s\r\n",
			un->purl.host, un->purl.port, vuseragent
		);
		bp = strchr( buffer, 0 );
		if( useproxyauth )
		{
			strcpy( bp, "Proxy-Authorization: Basic " );
			strcat( bp, proxyauth );
			strcat( bp, "\r\n" );
			bp = strchr( bp, 0 );
		}
		strcpy( bp, "\r\n" );
		un->ssl = 2;
		un->viaproxy = 0;
	}
	else if( un->ssl && !un->sslh )
	{
		un->protocolstate = HTTP_DO_SSL_HANDSHAKE; // do SSL handshaking first
		DL( DEBUG_INFO, db_http, bug( "redoing for SSL handshake\n" ));
		return;
	}
	else
#endif /* USE_SSL */
	{
		// plain standard HTTP request
		un->trying11 = !un->dcn->http11failed && !un->postid && !un->ssl;
		if( !un->ssl )
			sur_text( un, GS( NETST_CONNECTED ), un->trying11 ? "HTTP/1.1" : "HTTP/1.0" );

		if( un->postid )
			strcpy( buffer, "POST " );
		else
			strcpy( buffer, "GET " );

		if( un->viaproxy )
		{
			strcat( buffer, un->urlcopy2 );
		}
		else
		{
			if( !un->purl.path || un->purl.path[ 0 ] != '/' )
				strcat( buffer, "/" );
			if( un->purl.path )
				strcat( buffer, un->purl.path );
			if( un->purl.args )
			{
				strcat( buffer, "?" );
				strcat( buffer, un->purl.args );
			}
		}
		sprintf( strchr( buffer, 0 ), " HTTP/1.%s\r\n", un->trying11 ? "1" : "0" );

		bp = strchr( buffer, 0 );

		if( un->referer )
		{
			strcpy( bp, "Referer: " );
			strcat( bp, un->referer );
			strcat( bp, "\r\n" );
			bp = strchr( bp, 0 );
		}

		sprintf( bp, "User-Agent: %s\r\nHost: %s\r\n", vuseragent, un->purl.host );
		bp = strchr( bp, 0 );

		if( gp_languages[ 0 ] )
		{
			strcpy( bp, "Accept-Language: " );
			strcat( bp, gp_languages );
			strcat( bp, "\r\n" );
			bp = strchr( bp, 0 );
		}

		if( un->postid )
		{
			int enctype;
			formdata = formp_getdata( un->postid, &formlen, &enctype );
			if( enctype )
				sprintf( bp, "Content-Type: multipart/form-data; boundary=" MULTIPART_SEP "\r\nContent-Length: %u\r\n", formlen );
			else
				sprintf( bp, "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %u\r\n", formlen );
			bp = strchr( bp, 0 );
		}

		if( un->useauth )
		{
			strcpy( bp, "Authorization: Basic " );
			strcat( bp, un->authdata );
			strcat( bp, "\r\n" );
			bp = strchr( bp, 0 );
		}

		if( useproxyauth )
		{
			strcpy( bp, "Proxy-Authorization: Basic " );
			strcat( bp, proxyauth );
			strcat( bp, "\r\n" );
			bp = strchr( bp, 0 );
		}

#ifndef MBX
		if( un->cachedate && !un->reload )
		{
			extern struct Locale *locale;
			extern int locale_timezone_offset;
			time_t tdate;
			char buffer[ 8 ];

			if( locale )
				tdate = un->cachedate - locale_timezone_offset; // We want GMT..

			utunpk( tdate, buffer );

			// Lame hack to come around sc.lib non-reentrancy..
			Forbid();
			sprintf( bp, "If-Modified-Since: %3.3s, %u %3.3s %04d %02d:%02d:%02d GMT\r\n",
				asctime( gmtime( &tdate ) ),
				buffer[ 2 ], &"JanFebMarAprMayJunJulAugSepOctNovDec"[ ( buffer[ 1 ] - 1 ) * 3 ], buffer[ 0 ] + 1970,
				buffer[ 3 ], buffer[ 4 ], buffer[ 5 ]
			);
			Permit();

			bp = strchr( bp, 0 );
		}
		else 
#endif
		if( un->reload )
		{
			strcpy( bp, "Pragma: no-cache\r\n" );
			bp = strchr( bp, 0 );
		}

		if( un->trying11 )
		{
			strcpy( bp, "Connection: Keep-Alive\r\n" );
			bp = strchr( bp, 0 );
		}

		/* resume */
		if( un->offset && un->range == 1 )
		{
			sprintf( bp, "Range: bytes=%lu-\r\n", un->offset );
			bp = strchr( bp, 0 );
			un->range = -1; // in progress..
		}

		strcpy( bp, VACCEPT );
		bp = strchr( buffer, 0 );

		// Add cookie data
		cookie_get( un->purl.host, un->purl.path, bp, un->ssl );
		strcat( bp, "\r\n" );

		// ok, request is ready to be send off
		DL( DEBUG_CHATTY, db_http, bug( "un(%s) sending request:\r\n%s\r\n", un->url, buffer ));
	}

	// we get this request out no matter what
	len = strlen( buffer );
	bp = buffer;

	// set socket back to blocking mode
	if( !un->ssl )
		setblocking( un->sock, FALSE );

/*	if( formlen )
	{
		DL( DEBUG_CHATTY, db_http, bug( "adding %ld bytes of form data\r\n", formlen ) );
		strcat( buffer, formdata );
		len = strlen( buffer );
	}*/

	uns_write( un, bp, len ); /* XXX: that one can crash.. */

	if( formlen )
	{
		DL( DEBUG_CHATTY, db_http, bug( "posting formdata (%ld bytes): '%s'\n", formlen, formdata ));
		uns_write( un, formdata, formlen );
	}

	un->protocolstate = HTTP_WAIT_REPLY; // waiting for (initital) HTTP reply

	initlineread( un );
}

static void un_doprotocol_http_initialreply( struct unode *un )
{
	if( !un->linereadstate )
		return;

	if( un->linereadstate < 0 )
	{
		if( un->wasparked || un->trying11 )
		{
			// failed early, so reconnect from scratch
			//un->dcn->http11failed = TRUE;
			un_netclose( un );
			un->state = UNS_WAITINGFORNET;
			return;
		}
		else
		{
			int rc;

			rc = Errno(  );
			
			// header error?!?
			makeneterror( un, GS( NWM_ERROR_NODATA ), rc );
			un->state = UNS_FAILED;
			exitlineread( un );
			un_netclose( un );
			return;
		}
	}

	DL( DEBUG_CHATTY, db_http, bug( "got http reply %s rc %ld\r\n", un->linereadbuffer, un->linereadstate ));

	if( !un->linereadbuffer[ 0 ] )
	{
		DL( DEBUG_CHATTY, db_http, bug( "skipping blank line\r\n" ));
		purgeline( un );
		return;
	}

	if( strnicmp( un->linereadbuffer, "HTTP", 4 ) || !strchr( un->linereadbuffer, ' ' ) )
	{
		// We're hosed, it's a pre 1.0 server..
		un->protocolstate = HTTP_GOT_HEADER; // pretend END OF HEADER
		un->state = UNS_CONNECTED;
		exitlineread( un );
		purgeline( un );
		return;

		// FIXME!
		/*makeneterror( un, GS( NWM_ERROR_NODATA ), -1 );
		un->state = UNS_FAILED;
		exitlineread( un );
		un_netclose( un );
		return;*/
	}
	else
	{
		un->httpcode = atoi( strchr( un->linereadbuffer, ' ' ) + 1 );
		if( un->httpcode >= 200 )
			un->protocolstate = HTTP_READ_HEADER;
		else if( un->httpcode == 100 )
		{
			purgeline( un );
			return;
		}

		if( un->httpcode == 304 && un->cachedate )
		{
			// we just verified an URL
			cache_stats.verified_ok++;
			addverify( un->url );
			un_netclose( un );
			un_setup_cache( un );
			return;
		}
		else if( un->httpcode >= 300 )
			un->nocache = TRUE;

#if USE_DOS
		if( un->l )
		{
			UnLock( un->l );
			un->l = NULL;
		}
#endif

		// FIXME! checking for failed 1.1 request!!!

		if( un->trying11 && un->linereadbuffer[ 7 ] < '1' )
		{

			DL( DEBUG_CHATTY, db_http, bug( "un(%s) got %s response for 1.1 request!\r\n", un->url, un->linereadbuffer ));

			// if there is some error code
			// restart from scratch, trying HTTP/1.0
			// (bleech!)

			if( un->httpcode >= 400 )
			{
				un->dcn->http11failed = TRUE;
				un->trying11 = FALSE;
				un_netclose( un );

				exitlineread( un );
				un->protocolstate = HTTP_SEND_REQUEST;
				un->state = UNS_WAITINGFORNET;
				return;
			}
		}

		if( un->cachedate )
			cache_stats.verified_failed++;

		if( un->httpcode >= 400 )
		{
			DL( DEBUG_CHATTY, db_http, bug( "un->httpcode == %ld\r\n", un->httpcode ) );
			sprintf( un->errorstring, GS( NWM_ERROR_SERVERERROR ), un->linereadbuffer );
		}
	}

	purgeline( un );
}

static void un_doprotocol_http_readheader( struct unode *un )
{
	if( !un->linereadstate )
		return;

	if( un->linereadstate < 0 )
	{
		int rc;

		rc = Errno(  );

		// header error?!?
		makeneterror( un, GS( NWM_ERROR_NODATA ), rc );
		un->state = UNS_FAILED;
		un_netclose( un );
		return;
	}

	//
	// either reading a header line itself or a folded header
	// or
	//

	DL( DEBUG_CHATTY, db_http, bug( "got http line %s\r\n", un->linereadbuffer ));

	if( !un->linereadbuffer[ 0 ] )
	{
		// if we were establishing a SSL proxy link,
		// let now the "real" protocol kick it

		if( un->ssl == 2 )
		{
			un->protocolstate = HTTP_SEND_REQUEST; // send "real" SSL request
			un->ssl = 1;
		}
		else
		{
			un->protocolstate = HTTP_GOT_HEADER; // end of header
		}
		exitlineread( un );
	}
	else if( isspace( un->linereadbuffer[ 0 ] ) )
	{
		struct header *h = LASTNODE( &un->headers );
		char *bf = stpblk( un->linereadbuffer );

		// folded header, attach to previous one
		if( NEXTNODE( h ) )
		{
			int nlen = strlen( h->data ) + strlen( bf ) + 2;
			char *ndata = unalloc( un, nlen );
			if( ndata )
			{
				strcpy( ndata, h->data );
				strcat( ndata, " " );
				strcat( ndata, bf );
				h->data = ndata;
			}
		}
	}
	else
	{
		char *p = strchr( un->linereadbuffer, ':' );
		if( p )
		{
			struct header *h;

			*p++ = 0;
			h = unalloc( un, sizeof( *h ) );
			if( h )
			{
				h->header = unstrdup( un, un->linereadbuffer );

				h->data = unstrdup( un, stpblk( p ) );
				ADDTAIL( &un->headers, h );
			}
		}
	}

	purgeline( un );
}

static void un_doprotocol_http_afterreadheader( struct unode *un )
{
	char *mimetype;
	char *server;
	char *len;
	char *coding;
	char *connection;
	char *location;
	char *range;
	struct header *h;
	char *date;

	/* Store the server string (ie. Apache, etc..) */
	server = ungetheaderdata( un, "SERVER" );
	if ( server )
	{
		stccpy( un->server, server, sizeof( un->server ) );
	}
	else
	{
		strcpy( un->server, "Unknown" );
	}
	// here, we're done reading the HTTP header
	// check the MIME type
	mimetype = ungetheaderdata( un, "CONTENT-TYPE" );
	// We force text/html for 400/500 replies, for very-lame servers
	if( mimetype && un->httpcode < 400 )
	{
		stccpy( un->mimetype, mimetype, sizeof( un->mimetype ) );
	}
	else
		strcpy( un->mimetype, "text/html" );

	DL( DEBUG_CHATTY, db_http, bug( "mime type %s, im %ld\r\n", un->mimetype, gp_ignorehttpmime ));

	if( gp_ignorehttpmime )
	{
		char buffer[ 256 ];

		mime_findbyextension( un->purl.path, NULL, NULL, NULL, buffer );
		if( buffer[ 0 ] )
		{
			strcpy( un->mimetype, buffer );
		}
		else
		{
			/* not found. Assume text/html then. Happens for http://www.blah.com/ and
			 * ignoring server-sent mimetypes.
			 */
			strcpy( un->mimetype, "text/html" );
		}
	}

	date = ungetheaderdata( un, "LAST-MODIFIED" );
	if( date )
		un->lastmodified = convertrfcdate( date );

	date = ungetheaderdata( un, "EXPIRES" );
	if( date )
		un->cacheexpires = convertrfcdate( date );

	if( un->range != -1 )
	{
	    len = ungetheaderdata( un, "CONTENT-LENGTH" );
		if( len )
		{
			un->doclen = atoi( len );
		}
		else
			un->doclen = -1; // unknown size (shit shit shit)
	}

	DL( DEBUG_CHATTY, db_http, bug( "before: un->chunked == %lu\r\n", un->chunked ) );
	coding = ungetheaderdata( un, "TRANSFER-ENCODING" );
	if( coding && !strnicmp( coding, "CHUNKED", 7 ) )
		un->chunked = TRUE;

	DL( DEBUG_CHATTY, db_http, bug( "after: un->chunked == %lu\r\n", un->chunked  ) );

	if( !un->chunked && un->doclen >= 0 )
	{
		if( un->ledobjnum >= 0 )
			netinfo_setmax( un->ledobjnum, un->doclen );
	}

	if( un->trying11 )
	{
		connection = ungetheaderdata( un, "CONNECTION" );
		if( !connection || stricmp( connection, "CLOSE" ) )
			un->keepalive = TRUE;
	}

	location = ungetheaderdata( un, "LOCATION" );
	if( location )
	{
		char *tmp = unalloc( un, strlen( un->urlcopy2 ) + strlen( location ) + 2 );
		if( tmp )
		{
			uri_mergeurl( un->urlcopy2, location, tmp );
			un->redirecturl = tmp;
		}
	}

	/* resume */
	if( un->range != -1 )
	{
		range = ungetheaderdata( un, "ACCEPT-RANGES" );
		if( range )
		{
			if( !strcmp( "none", range ) )
			{
				un->range = 0; // no range possible
			}
			else if( !strcmp( "bytes", range ) )
			{
				un->range = 1; // bytes
			}
			else
			{
				un->range = 0; // unknown.. better not handle it
			}
		}
		else
		{
			/* since a server is not forced to send us a range header, we can
			   try anyway
			*/
			un->range = 1; // in bytes
		}
	}
	else
	{
		range = ungetheaderdata( un, "CONTENT-RANGE" );
		if( range )
		{
			ULONG size;

			range=strchr( range, ' ' );
			if( !range )
				size=0;
			else
			{
				range++;
				stcd_l( range, (long*)&size );
			}

			if( size != un->offset ) // failed.. get from start
				un->offset = 0;
			else
			{
				if( un->doclen != -1 )
					un->doclen += un->offset; // un->doclen needs to be the real file size
			}
		}
		else
		{
			un->offset = 0; // server doesn't support resume, get from start
		}
	}

	// scan for cookies and other multiple headers
	for( h = FIRSTNODE( &un->headers ); NEXTNODE( h ); h = NEXTNODE( h ) )
	{
		if( !stricmp( h->header, "SET-COOKIE" ) )
		{
			cookie_set( h->data, un->purl.host, un->purl.path );
		}
		else if( !stricmp( h->header, "PRAGMA" ) )
		{
			if( !stricmp( h->data, "NO-CACHE" ) )
				un->nocache = TRUE;
		}
	}
 

	// authentication
	if( un->httpcode == 401 || un->httpcode == 407 ) // authentication required
	{
		int needproxyauth = ( un->httpcode == 407 );

		if( un->useauth < 3 )
		{
			char *auth;
			auth = ungetheaderdata( un, "WWW-AUTHENTICATE" );
			DL( DEBUG_CHATTY, db_http, bug("there, auth == %s\r\n", auth));
			if( auth )
			{
				stripentity( auth, "REALM", un->authrealm, sizeof( un->authrealm ) );
			}
			if( auth_query( &un->purl, un->dcn->name, needproxyauth ? "Proxy-Login" : un->authrealm, needproxyauth ? useproxyauth : un->useauth, needproxyauth ? proxyauth : un->authdata, FALSE ) )
			{
				un_netclose( un );
				exitlineread( un );
				un->protocolstate = HTTP_SEND_REQUEST;
				un->state = UNS_WAITINGFORNET;
				
				/*
				 * workaround for weird servers which send an auth request chunked but not the
				 * real object. we need to purge the headers since they'll be new
				 */
				un->chunked = FALSE; 
				NEWLIST( &un->headers ); // deletes all the headers from an unode

				if( needproxyauth )
					useproxyauth++;
				else
					un->useauth++;

				/*
				 * The following is used because other part of the code
				 * check if there's an errorstring and assume there's an
				 * error. If we authenticated properly there's no error
				 * anymore.
				 */
				un->errorstring[ 0 ] = '\0';

				return;
			}
		}
	}
 
	// signal application to say where it wants it's data
	// and get ready for sleep
	un->state = UNS_WAITING;
	un->protocolstate = HTTP_BEGIN_READ_DATA;
}

static void un_doprotocol_http_beginreaddata( struct unode *un )
{
	DL( DEBUG_CHATTY, db_http, bug( "beginread(%s) tomem %ld len %ld\r\n", un->url, un->tomem, un->doclen ));

	/* fire off another GET to try to resume */
	if( un->offset && un->range == 1 )
	{
		DL( DEBUG_CHATTY, db_http, bug( "firing off new GET request to resume at offset %lu\r\n", un->offset ) );
		un_netclose( un );
		exitlineread( un );
		un->protocolstate = HTTP_SEND_REQUEST;
		un->state = UNS_WAITINGFORNET;
		NEWLIST( &un->headers );
		return;
	}

	// if we have a size, we can prealloc
	if( un->tomem && un->doclen > 0 )
	{
		ObtainSemaphore( &netpoolsem );
		if( !allocdata( un, un->doclen + 1 ) )
			((UBYTE*)un->membuff)[ 0 ] = 0;
		ReleaseSemaphore( &netpoolsem );
	}

	DL( DEBUG_CHATTY, db_http, bug( "ready to read; chunked = %ld\r\n", un->chunked ));

	// get read to read
	if( un->chunked )
	{
		un->protocolstate = HTTP_READ_CHUNK_11;
		reinitlineread( un );
	}
	else
	{
		un->protocolstate = HTTP_READ_DATA_10;
		un->sockstate = SSW_SR;
	}

	sur_led( un, 3 );

	if( un->doclen > 0 )
		sur_text( un, GS( NETST_TRANSFER_SIZE ), un->doclen );
	else
		sur_txt( un, GS( NETST_TRANSFER ) );

	//time( &un->beginxfer );
	un->beginxfer = un->lastdata = now;

	sur_gauge_report( un );
}


static void un_doprotocol_http_readdata_10( struct unode *un )
{
	int rc = 0;
	int maxread;

	if( un->chunked )
		maxread = un->chunksize;
	else if( un->doclen > 0 )
		maxread = un->doclen;
	else
		maxread = LINEREADBUFFERSIZE;

	if( un->linereadbufferptr )
	{
		rc = min( maxread, un->linereadbufferptr );
	}
	else if( un->sockstategot & SSW_SR )
	{
		// read some data
		// we're be abusing the linereadbuffer :-)

		DL( DEBUG_CHATTY, db_http, bug( "(%s) before recv()\r\n", un->url ));

		maxread = min( LINEREADBUFFERSIZE, maxread );

		rc = uns_read( un, un->linereadbuffer, maxread );

		DL( DEBUG_CHATTY, db_http, bug( "(%s) got %ld bytes of data\r\n", un->url, rc ));

		if( rc <= 0 )
		{
			// end of data
			if( ( un->doclen <= 0 || un->docptr + un->offset >= un->doclen ) && !un->chunked )
			{
				// unknown size, or realdocptr is at end of file, we assume end of stream
				// also, connection is to be closed
				un_closedestfiles( un );
				un->keepinmem = TRUE;
				un_netclose( un );
				un->state = UNS_DONE;
				un->sockstate = 0;
				sur_gauge_clear( un );
				if( !un->nocache )
					un_memtocache( un );
				return;
			}

			// FIXME!
			un_netclose( un );
			un->state = UNS_FAILED;
			un->sockstate = 0;
			return;
		}
	}
	else if( !un->stalled )
	{
		// not flagged
		// check for stall
		if( now - un->lastdata >= 5 )
		{
			un->informptr = un->docptr + 2048;
			un->stalled = TRUE;
			un->doinform = TRUE;
			sur_led( un, 4 );
			sur_gauge_report( un );
			return;
		}
	}

	// push data
	if( !rc )
		return; // nothing to push, really

	pushdata( un, un->linereadbuffer, rc );

	if( un->linereadbufferptr )
		purgedata( un, rc );

	if( un->docptr > un->informptr )
	{
		un->informptr = un->docptr + 2048;
		un->doinform = TRUE;
	}

	if( un->stalled )
	{
		un->stalled = FALSE;
		sur_led( un, 3 ); // reading again
	}
	un->lastdata = now;

	sur_gauge_report( un );

	if( un->chunked )
	{
		un->chunksize -= rc;
		if( un->chunksize <= 0 )
		{
			// eventually, the next chunk is pending
			DL( DEBUG_CHATTY, db_http, bug( "un(%s) -> chunk finished, ready for next\r\n", un->url ));
			reinitlineread( un );
			un->protocolstate = HTTP_READ_NEXT_CHUNK_11;
			return;
		}
	}

	if( un->doclen > 0 && ( un->docptr + un->offset ) >= un->doclen )
	{
		DL( DEBUG_CHATTY, db_http, bug("done(%s)\r\n", un->url ));
		// done
		if( un->keepalive )
		{
			un_netpark( un );
		}
		else
		{
			un_netclose( un );
		}
		un->state = UNS_DONE;
		un->doinform = TRUE;
		sur_gauge_clear( un );

		un_closedestfiles( un );
		un->keepinmem = TRUE;

		// write to cache..
		if( !un->nocache )
			un_memtocache( un );
	}
}

static void un_doprotocol_http_readdata_chunk( struct unode *un )
{
	if( !un->linereadstate )
		return;

	if( un->linereadstate < 0 )
	{
		int rc;

		rc = Errno(  );
	
		// header error?!?
		makeneterror( un, GS( NWM_ERROR_NODATA ), rc );
		un->state = UNS_FAILED;
		exitlineread( un );
		sur_led( un, 0 );
		return;
	}

	if( un->protocolstate == HTTP_READ_NEXT_CHUNK_11 )
	{
		purgeline( un );
		// read trailing CRLF
		un->protocolstate = HTTP_READ_CHUNK_11;
		return;
	}

	// read fucking chunk size
	stch_l( un->linereadbuffer, (long*)&un->chunksize );

	// we eventually got a chunk header line
	DL( DEBUG_CHATTY, db_http, bug( "un(%s) got CHUNK header '%s' size %ld\r\n", un->url, un->linereadbuffer, un->chunksize ));
	purgeline( un );

	if( un->chunksize <= 0 )
	{
		// read remains
		un->protocolstate = HTTP_READ_POST_CHUNK_11;
	}
	else
	{
		if( un->ledobjnum >= 0 )
			netinfo_setmax( un->ledobjnum, un->chunksize );
		exitlineread( un );
		un->protocolstate = HTTP_READ_DATA_10;
		un->sockstate = SSW_SR;
	}
}

static void un_doprotocol_http_readdata_postchunk( struct unode *un )
{
	if( !un->linereadstate )
		return;

	if( un->linereadstate < 0 )
	{
		int rc;

		rc = Errno(  );

		// header error?!?
		makeneterror( un, GS( NWM_ERROR_NODATA ), rc );
		un->state = UNS_FAILED;
		exitlineread( un );
		sur_led( un, 0 );
		return;
	}

	if( !un->linereadbuffer[ 0 ] )
	{
		purgeline( un );

		// ok, done with this document

		DL( DEBUG_CHATTY, db_http, bug("done(%s) by postheaderchunk\r\n", un->url ));
		// done

		un_closedestfiles( un );
		un->keepinmem = TRUE;

		if( un->keepalive )
		{
			un_netpark( un );
		}
		else
		{
			un_netclose( un );
		}
		un->state = UNS_DONE;
		un->doinform = TRUE;
		if( !un->nocache )
			un_memtocache( un );
		sur_gauge_clear( un );

		return;
	}

	purgeline( un );
}

static void un_doprotocol_http_ssl_setup( struct unode *un )
{
#if USE_SSL
	char *cipher = cipher; /* shut up gcc */
	STRPTR sslver = sslver; /* shut up gcc */
	APTR peercert;
	char cert_info[ 2048 ];

	sur_txt( un, GS( NETST_SSLHANDSHAKE ) );

	if( !openssl() )
	{
#endif
		// no SSL
		strcpy( un->errorstring, GS( NWM_ERROR_NOSSL ) );
		makehtmlerror( un );
		un->state = UNS_FAILED;
		un_netclose( un );
		return;
#if USE_SSL
	}

	setblocking( un->sock, FALSE );

	// ok, establish SSL connection
#ifndef MBX
	if( VSSLBase )
#endif
	{
		un->sslh = VSSL_Connect( ssl_ctx, un->sock );
	}
#if USE_MIAMI
	else
	{
		un->sslh = SSL_new( ssl_ctx );
		SSL_set_fd( un->sslh, un->sock );
		if( SSL_connect( un->sslh ) < 0 )
		{
			SSL_free( un->sslh );
			un->sslh = NULL;
		}
	}
#endif

	if( !un->sslh )
	{
		DL( DEBUG_CHATTY, db_http, bug( "ssl_connect() failed\r\n" ));
ssl_failed:
		strcpy( un->errorstring, GS( NWM_ERROR_SSLFAILED ) );
		makehtmlerror( un );
		un->state = UNS_FAILED;
		un_netclose( un );
		return;
	}

	// at this point, the SSL handshake was completed

#ifndef MBX	
	if( VSSLBase)
#endif
	{
		peercert = VSSL_GetPeerCertificate( un->sslh );

		if( !peercert )
			goto ssl_failed;

		DL( DEBUG_INFO, db_http, bug( "peercert %lx\r\n", peercert ));
		
		cert_getinfo( peercert, cert_info, VSSLVAR );
		un->sslpeercert = StrDupPooled( un->pool, cert_info );

		DL( DEBUG_CHATTY, db_http, bug( "un->sslpeercert == %s\r\n", un->sslpeercert ));

		if( !FINDNAME( &sslcertlist, un->purl.host ) )
		{
			char *errorp;
			int rc;
			rc = VSSL_GetVerifyResult( un->sslh, &errorp );
			if( rc >= 2 )
			{
				rc = certreq_ask( peercert, errorp, VSSLVAR, un );
				if( !rc )
				{
					DL( DEBUG_CHATTY, db_http, bug( "nocert!!!\r\n" ));
					strcpy( un->errorstring, GS( NWM_ERROR_SSLFAILED ) );
					makehtmlerror( un );
					un->state = UNS_FAILED;
					un_netclose( un );
					return;
				}
				else
				{
					struct Node *ssln;
					ssln = (APTR)nalloc( sizeof( *ssln ) + strlen( un->purl.host ) + 1 );
					if( ssln )
					{
						ssln->ln_Name = (char*)(ssln + 1 );
						strcpy( ssln->ln_Name, un->purl.host );
						ADDTAIL( &sslcertlist, ssln );
					}
				}
			}
		}
		cipher = VSSL_GetCipher( un->sslh );    
		sslver = VSSL_GetVersion( un->sslh );
	}
#if USE_MIAMI
	else
	{
		cipher = SSL_get_cipher( un->sslh );
		sslver = "Not implemented in Miami"; //TOFIX !! not available in that version at least..
	}
#endif

	/* copy the cipher and version in the unode */
	un->sslcipher = StrDupPooled( un->pool, cipher );
	un->sslversion = StrDupPooled( un->pool, sslver );

	sur_text( un, GS( NETST_SSLESTABLISHED ), cipher );

	un->protocolstate = HTTP_SEND_REQUEST;
#endif
}

void un_doprotocol_http( struct unode *un )
{
	while( un->state == UNS_CONNECTED || un->state == UNS_READING )
	{
		DL( DEBUG_CHATTY, db_http, bug( "un_doprotocol_http un->lineread: %02X un->protocolstate: %02X \r\n", un->lineread, un->protocolstate ));
		if( un->lineread )
		{
			dolineread( un );
			if( !un->linereadstate )
				return; // didn't get no data
		}

		switch( un->protocolstate )
		{
			case HTTP_SEND_REQUEST: // send request
				un_doprotocol_http_req( un );

				ASSERT( un );
				if( un->protocolstate != HTTP_DO_SSL_HANDSHAKE )
					return;

			case HTTP_DO_SSL_HANDSHAKE: // special SSL connection establishment
				un_doprotocol_http_ssl_setup( un );
				break;

			case HTTP_WAIT_REPLY: // we're waiting for the initial HTTP reply..
				un_doprotocol_http_initialreply( un );
				break;

			case HTTP_READ_HEADER: // we're reading header..
				un_doprotocol_http_readheader( un );
				break;

			case HTTP_GOT_HEADER: // done reading header
				un_doprotocol_http_afterreadheader( un );
				return;

			case HTTP_BEGIN_READ_DATA: // client requested data
				un_doprotocol_http_beginreaddata( un );
				// fallthrough, to fetch some lineread remains
				if( un->protocolstate != HTTP_READ_DATA_10 )
					break;

			case HTTP_READ_DATA_10: // reading data (1.0 style)
				un_doprotocol_http_readdata_10( un );
				if( un->protocolstate != HTTP_READ_NEXT_CHUNK_11 )
					return;
				else
					continue; // more chunks

			case HTTP_READ_CHUNK_11: // reading chunk (1.1 style)
			case HTTP_READ_NEXT_CHUNK_11:
				un_doprotocol_http_readdata_chunk( un );
				break;

			case HTTP_READ_POST_CHUNK_11:
				un_doprotocol_http_readdata_postchunk( un );
				break;

			default:
				return;
		}
	}
}

#endif /* USE_NET */
