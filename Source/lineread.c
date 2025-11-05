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
 * $Id: lineread.c,v 1.3 2003/07/06 16:51:33 olli Exp $
 */

#include "voyager.h"

/* private */
#include "dos_func.h"
#include "network.h"
#include "lineread.h"

#if USE_NET

void initlineread( struct unode *un )
{
	if( !un->linereadbuffer )
	{
		un->linereadbuffer = unalloc( un, LINEREADBUFFERSIZE );
		if( !un->linereadbuffer )
			return;
	}
	un->linereadbufferptr = un->linereadadvanceptr = 0;
	memset( un->linereadbuffer, 0, LINEREADBUFFERSIZE );
	un->lineread = 1;
	un->sockstate = SSW_SR;
}


void initlineread_pasv( struct unode *un )
{
	un->linereadbufferptr = un->linereadadvanceptr = 0;
	memset( un->linereadbuffer, 0, LINEREADBUFFERSIZE );
	un->lineread = 2;
	un->sockstate = SSW_PSR;
}


void exitlineread( struct unode *un )
{
	un->lineread = FALSE;
	un->sockstate = 0;
}


void reinitlineread( struct unode *un )
{
	DL( DEBUG_CHATTY, db_net, bug( "reinitlineread(%s) with %ld bytes remaining\n", un->url, un->linereadbufferptr ));
	un->lineread = TRUE;
	un->sockstate = SSW_SR;
	un->linereadbuffer[ un->linereadbufferptr ] = 0;
}


void dolineread( struct unode *un )
{
	char *p;

	DL( DEBUG_CHATTY, db_net, bug( "entering lineread p %ld adv %ld\n", un->linereadbufferptr, un->linereadadvanceptr ));

	// check if there is still enough data in the line read buffer
retry:
	if( p = strchr( un->linereadbuffer, '\n' ) )
	{
		// quite frankly, there is
		un->linereadstate = 1; // got 1 line of data
		un->linereadadvanceptr = p - un->linereadbuffer + 1;
		*p = 0;
		if( p[ -1 ] == '\r' )
			p[ -1 ] = 0;
		return;
	}

	if( un->sockstategot & ( un->lineread == 1 ? SSW_SR : SSW_PSR ) )
	{
		int rc;
		int bufffree = LINEREADBUFFERSIZE - un->linereadbufferptr - 1;

		if( !bufffree )
		{
			// pretty crappy, we can't extract that line
			un->linereadbufferptr = 0;
			return;
		}

		// data is supposed to be there
		if( un->lineread == 1 )
			rc = uns_read( un, &un->linereadbuffer[ un->linereadbufferptr ], bufffree );
		else
			rc = uns_pread( un, &un->linereadbuffer[ un->linereadbufferptr ], bufffree );

#if USE_SSL
		if( !rc && un->sslh )
		{
			/*
			 * SSL may return "0" even with the r-t-r bit set
			 */
			DL( DEBUG_WARNING, db_net, bug( "SSL lineread no data yet\n" ));
			un->linereadstate = 0;
			un->sockstategot = 0;
			return;
		}
#endif /* USE_SSL */

		DL( DEBUG_CHATTY, db_net, bug( "lineread recv()ed %ld bytes\n", rc ));

		if( rc <= 0 )
		{
			un->linereadstate = -1; // something queer has happened
			return;
		}

		un->linereadbufferptr += rc;
		un->linereadbuffer[ un->linereadbufferptr ] = 0;

		un->sockstategot = 0;
		goto retry;
	}
	un->linereadstate = 0;
}

void purgedata( struct unode *un, int amount )
{
	DL( DEBUG_CHATTY, db_net, bug( "purgedata bp %ld amount %ld adv %ld rem %ld\n", un->linereadbufferptr, amount, un->linereadadvanceptr, LINEREADBUFFERSIZE - amount ));

	if( amount > un->linereadbufferptr )
	{
		DL( DEBUG_ERROR, db_net, bug( "PANIC!!!" ));
		Delay( 100 );
		return;
	}

	memcpy( un->linereadbuffer, &un->linereadbuffer[ amount ], LINEREADBUFFERSIZE - amount );
	un->linereadbufferptr -= amount;
	un->linereadadvanceptr -= min( un->linereadadvanceptr, amount );
}

void purgeline( struct unode *un )
{
	purgedata( un, un->linereadadvanceptr );
}

#endif /* USE_NET */
