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

  $Id: autoproxy.c,v 1.17 2003/07/06 16:51:32 olli Exp $:

  Proxy autoconfiguration
  -----------------------

*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "autoproxy.h"
#include "classes.h"
#include "js.h"
#include "urlparser.h"
#include "prefs.h"

static struct jsop_list *proxyfunc=NULL;

// General proxy mode
// 0 = no proxy
// 1 = automatic (will fallback to 0)
// 2 = manual

static int proxy_mode;
static int use_proxy_http, use_proxy_ftp, use_proxy_ssl;

void free_autoproxy( void )
{
	D( db_init, bug( "freeing structure\n" ) );
	
	jso_cleanup( proxyfunc );
}

void proxy_init( void )
{
	jso_cleanup( proxyfunc );
	proxyfunc = NULL;

	proxy_mode = getprefslong( DSI_PROXY_CONFMODE, 0 );

	use_proxy_http = getprefslong( DSI_PROXY_USE_OFFSET + DSI_PROXY_HTTP, FALSE );
	use_proxy_ftp = getprefslong( DSI_PROXY_USE_OFFSET + DSI_PROXY_FTP, FALSE );
	use_proxy_ssl = getprefslong( DSI_PROXY_USE_OFFSET + DSI_PROXY_HTTPS, FALSE );
}

static int checknoproxy( char *host, int port )
{
	char buff[ 512 ], *p, *p2;

	if( !host )
		return( TRUE );

	stccpy( buff, getprefsstr( DSI_NOPROXY, "" ), sizeof( buff ) );

	Forbid();
	for( p = ( char * )strtok( buff, " ,|" ); p; p = ( char * )strtok( NULL, " ,|" ) )
	{
		int l, l2;
		p2 = strchr( p, ':' );
		if( p2 )
			*p2++ = 0;
		l = strlen( p );
		l2 = strlen( host );
		if( l2 >= l && !stricmp( &host[ l2 - l ], p ) )
		{
			if( !p2 || port == atoi( p2 ) )
			{
				Permit();
				return( TRUE );
			}
		}
	}
	Permit();
	return( FALSE );
}

static char *js_proxy_find( char *url, struct parsedurl *purl, int *proxyport )
{
	return( 0 );
}

char *proxy_for_url( char *url, struct parsedurl *purl, int *proxyport )
{
	if( gp_noproxy )
		return( 0 );

	switch( proxy_mode )
	{
		case 1:
			return( js_proxy_find( url, purl, proxyport ) );

		case 2:
			// Manual
			if( !strcmp( purl->scheme, "http" ) )
			{
				if( use_proxy_http )
				{
					if( !checknoproxy( purl->host, purl->port ) )
					{
						*proxyport = getprefslong( DSI_PROXY_PORT_OFFSET + DSI_PROXY_HTTP, 80 );
						return( getprefsstr( DSI_PROXY_HTTP, "localhost" ) );
					}
				}
				return( 0 );
			}                       
			if( !strcmp( purl->scheme, "ftp" ) )
			{
				if( use_proxy_ftp )
				{
					if( !checknoproxy( purl->host, purl->port ) )
					{
						*proxyport = getprefslong( DSI_PROXY_PORT_OFFSET + DSI_PROXY_FTP, 80 );
						return( getprefsstr( DSI_PROXY_FTP, "localhost" ) );
					}
				}
				return( 0 );
			}                       
			if( !strcmp( purl->scheme, "https" ) )
			{
				if( use_proxy_ssl )
				{
					if( !checknoproxy( purl->host, purl->port ) )
					{
						*proxyport = getprefslong( DSI_PROXY_PORT_OFFSET + DSI_PROXY_HTTPS, 80 );
						return( getprefsstr( DSI_PROXY_HTTPS, "localhost" ) );
					}
				}
				return( 0 );
			}                       
			if( !strcmp( purl->scheme, "gopher" ) )
			{
				*proxyport = getprefslong( DSI_PROXY_PORT_OFFSET + DSI_PROXY_GOPHER, 80 );
				return( getprefsstr( DSI_PROXY_GOPHER, "localhost" ) );
			}                       
			if( !strcmp( purl->scheme, "wais" ) )
			{
				*proxyport = getprefslong( DSI_PROXY_PORT_OFFSET + DSI_PROXY_WAIS, 80 );
				return( getprefsstr( DSI_PROXY_WAIS, "localhost" ) );
			}                       
			break;
	}

	return( 0 );
}

#endif /* USE_NET */
