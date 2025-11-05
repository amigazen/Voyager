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
 * Window text gauge functions
 * ---------------------------
 *
 * $Id: sur_gauge.c,v 1.3 2003/10/05 01:32:42 zapek Exp $
 */

#include "voyager.h"

/* private */
#include "classes.h"
#include "htmlclasses.h"
#include "voyager_cat.h"
#include "methodstack.h"
#include "ledclass.h"
#include "network.h"

extern time_t now; /* TOFIX: sucks.. */

void sur_gauge_report( struct unode *un )
{
	struct nstream *ns;

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->gaugeobj && !ns->removeme )
		{
			if( un->stalled )
			{
				pushmethod( ns->gaugeobj, 2, MM_Gauge_SetText, GS( NETST_TRANSFER_STALLED ) );
			}
			else
			{
				static char bf[ 80 ];
				time_t elapsed = now - un->beginxfer;

				if( elapsed < 1 )
					elapsed = 1;

				if( un->doclen > 0 )
				{
					sprintf( bf, GS( NETST_TRANSFER_GAUGE ), ( un->docptr * 100 ) / un->doclen, ( un->doclen + 1023 ) / 1024, un->docptr / elapsed );
					pushmethod( ns->gaugeobj, 4, MM_Gauge_Set, un->doclen, un->docptr, bf );
				}
				else
				{
					sprintf( bf, GS( NETST_TRANSFER_GAUGE_SIZE ), ( un->docptr + 1023 ) / 1024, un->docptr / elapsed );
					pushmethod( ns->gaugeobj, 4, MM_Gauge_Set, 0, 0, bf );
				}
			}
		}
	}
}


void sur_gauge_reset( struct unode *un )
{
	struct nstream *ns;

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->gaugeobj && !ns->removeme )
			pushmethod( ns->gaugeobj, 1, MM_Gauge_Reset );
	}
}


void sur_gauge_clear( struct unode *un )
{
	struct nstream *ns;

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->gaugeobj && !ns->removeme )
		{
			pushmethod( ns->gaugeobj, 1, MM_Gauge_Clear );
		}
	}
}

/*
 * Update all the nstream clients with
 * a text.
 */
void sur_txt( struct unode *un, char *txt )
{
	struct nstream *ns;

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->statusobj && !ns->removeme )
			pushmethod( ns->statusobj, 2, MM_HTMLWin_SetTxt, txt );
	}
}


static char __far textbuff[ 8 * 128 ];
static int textbuffcnt;
void STDARGS sur_text( struct unode *un, char *fmt, ... )
{
	char *to = 0;
	struct nstream *ns;

	for( ns = FIRSTNODE( &un->clients ); NEXTNODE( ns ); ns = NEXTNODE( ns ) )
	{
		if( ns->statusobj && !ns->removeme )
		{
			if( !to )
			{
				va_list arg;

				va_start( arg, fmt );
				to = &textbuff[ textbuffcnt * 128 ];
				textbuffcnt = ( textbuffcnt + 1 ) % 8;
				vsprintf( to, fmt, arg );
				va_end( arg );
			}
			pushmethod( ns->statusobj, 2, MM_HTMLWin_SetTxt, to );
		}
	}
}


void sur_led( struct unode *un, ULONG state )
{
	if( un->ledobjnum >= 0 )
		setled( un->ledobjnum, state );
}
