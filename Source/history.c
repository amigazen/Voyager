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
** $Id: history.c,v 1.29 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#include <limits.h>

/* private */
#include "classes.h"
#include "voyager_cat.h"
#include "splashwin.h"
#include "history.h"
#include "time_func.h"
#include "prefs.h"
#include "malloc.h"
#include "cache.h"
#include "init.h"
#include "mui_func.h"


#define MAXHISTORY 2048
#define FN "URL-History.1"

static struct histent {
	ULONG hash;
	time_t t;
	char *url;
} *histents;
static ULONG hisp;

struct histdisk {
	ULONG hash;
	time_t t;
	ULONG len;
};

extern ASM ULONG hash( __reg( a0, const char * ) );

void init_history( void )
{
#if USE_DOS
	struct AsyncFile *f;
	struct histdisk hd;
	time_t expire = timev() - ( getprefslong( DSI_NET_LINKS_EXPIRE, 30 ) * ( 3600 * 24 ) );
	char buffer[ 256 ];
#endif /* USE_DOS */
	
	D( db_init, bug( "initializing..\n" ) );

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_HISTORY ) );
	}
#endif
	
	histents = malloc( MAXHISTORY * sizeof( *histents ) );
	memset( histents, '\0', MAXHISTORY * sizeof( *histents ) );

#if USE_DOS
	makecachepath( FN, buffer );

	f = OpenAsync( buffer, MODE_READ, 4096 );
	if( !f )
		return;

	while( ReadAsync( f, &hd, sizeof( hd ) ) == sizeof( hd ) )
	{
		if( hd.t < expire )
		{
			SeekAsync( f, hd.len, OFFSET_CURRENT );
		}
		else
		{
			histents[ hisp ].hash = hd.hash;
			histents[ hisp ].t = hd.t;
			histents[ hisp ].url = malloc( hd.len );
			ReadAsync( f, histents[ hisp ].url, hd.len );
			hisp = ( hisp + 1 ) % MAXHISTORY;
		}
	}

	CloseAsync( f );
#endif /* USE_DOS */

	return;
}

void save_history( void )
{
#if USE_DOS
	struct AsyncFile *f;
	struct histdisk hd;
	char buffer[ 256 ];
	struct histent *hp = histents;
	int c;

	D( db_init, bug( "saving history..\n" ) );

	if( !app_started || !histents )
		return;

	makecachepath( FN, buffer );

	f = OpenAsync( buffer, MODE_WRITE, 4096 );
	if( !f )
		return;

	for( c = 0; c < MAXHISTORY; c++, hp++ )
	{
		if( !hp->url )
			break;

		hd.t = hp->t;
		hd.hash = hp->hash;
		hd.len = strlen( hp->url ) + 1;
		WriteAsync( f, &hd, sizeof( hd ) );
		WriteAsync( f, hp->url, hd.len );
	}

	CloseAsync( f );

	D( db_init, bug( "saved\n" ) );
#endif /* USE_DOS */
}

time_t checkurlhistory( char *url )
{
	int c;
	ULONG myhash = hash( url );
	struct histent *hp = histents;

	for( c = 0; c < MAXHISTORY; c++, hp++ )
	{
		if( !hp->url )
			break;
		if( hp->hash != myhash )
			continue;
		if( !strcmp( hp->url, url ) )
			return( hp->t );
		//D( db_history, bug( "hash collide %s[%lx] <-> %s[%lx]\n", url, myhash, hp->url, hp->hash ));
	}
	return( 0 );
}

void addurlhistory( char *url )
{
	ULONG myhash = hash( url );
	struct histent *hpp = histents;
	int c;

	for( c = 0; c < MAXHISTORY; c++, hpp++ )
	{
		if( !hpp->url )
			break;

		if( hpp->hash != myhash )
			continue;

		if( !strcmp( hpp->url, url ) )
		{
			time( &hpp->t );
			return;
		}
	}

	histents[ hisp ].t = timev();
	histents[ hisp ].hash = myhash;
	histents[ hisp ].url = strdup( url );
	hisp = ( hisp + 1 ) % MAXHISTORY;
}

void flushurlhistory( void )
{
	int c;

	for( c = 0; c < MAXHISTORY; c++ )
	{
		if( histents[ c ].url )
			free( histents[ c ].url );
	}
	memset( histents, 0, sizeof( *histents ) * MAXHISTORY );
	hisp = 0;
}

char *findurlhismatch( char *what )
{
	int l = strlen( what );
	int c;
	char *bestmatch = NULL;
	int bestmatchval = INT_MAX;

	for( c = 0; c < MAXHISTORY && histents[ c ].url; c++ )
	{
		if( !strncmp( what, histents[ c ].url, l ) )
		{
			int l2 = strlen( histents[ c ].url );
			if( l2 - l < bestmatchval )
			{
				bestmatchval = l2 - l;
				bestmatch = histents[ c ].url;              
			}
		}
	}
	return( bestmatch );
}
