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
 * Vapor Updater
 * -------------
 * - Checks if there's a new version of Voyager using
 * vapor_update.library
 *
 * © 2000-2003 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: update.c,v 1.10 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

#if USE_VAPOR_UPDATE

#include "copyright.h"
#include "vup/vupdate.h"

static struct Library *VUPBase;
static APTR vuphandle;

void check_update( void )
{
	D( db_init, bug( "initializing..\n" ) );

#if USE_VAT
	if( VUPBase = OpenLibrary( "vapor_update.library", 1 ) )
	{
		vuphandle = VUP_BeginCheckUpdate( 2, VERHEXID, "V³ " VERTAG );
	}
#if VAT_ECRYPT_CHECK
	{
		extern __far ULONG __startup[];

		if( __startup[ -1 ] )
		{
			VAT_CheckEcrypt( 0x29ecc380 );
		}
	}
#endif /* VAT_ECRYPT_CHECK */
#endif
}

/*
 * Close the vapor_update.library
 */
void close_vapor_update( void )
{
#if USE_VAT
	if( VUPBase )
	{
		D( db_init, bug( "closing vapor_update.library\n" ) );
		VUP_Quit( vuphandle );
		CloseLibrary( VUPBase );
	}
#endif
}

#endif /* USE_VAPOR_UPDATE */

