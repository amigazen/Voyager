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
 * CSS tree parser
 * ----------------------
 * - parses a style sheet or individual style definition
 *
 * © 1999-2003 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: cssengine.c,v 1.8 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* private */
#include "copyright.h"
#include "voyager_cat.h"
#include "mui_func.h"

static struct MinList css_tree;
static APTR css_pool;

int init_css( void )
{
	D( db_init, bug( "initializing..\n" ) );

	NEWLIST( &css_tree );

	return( TRUE );
}

void cleanup_css( void )
{

}

int parse_css( char *spec, int sheet )
{
	return(0);
}
