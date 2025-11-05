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
 * Host OS detection
 * -----------------
 *
 * © 2000-2003 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: host_os.c,v 1.7 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

#include "host_os.h"


char cpuid[ 12 ];
char hostos[ 8 ];

/*
 * Find the host OS
 */
void find_host_os( void )
{
#ifdef MBX
	strcpy( hostos, "CaOS" );
	strcpy( cpuid, "mcf5307" );

	return;
#else
#ifndef __MORPHOS__
	struct Library *PPCBase;
#endif /* !__MORPHOS__ */

	D( db_init, bug( "initializing..\n" ) );

	/*
	 * Find the hosting OS, default to AmigaOS
	 */
	strcpy( hostos, "AmigaOS" );

	/*
	 * Check for MorphOS
	 */
	if( FindResident( "MorphOS" ) )
	{
		strcpy( hostos, "MorphOS" );
	}
#ifdef __MORPHOS__
	strcpy( cpuid, "PPC native" );
#else
	if( PPCBase = OpenLibrary( "ppc.library", 46 ) )
	{
		strcpy( cpuid, "PPC" );
		CloseLibrary( PPCBase );
	}
	else
	{
		strcpy( cpuid, "MC680x0" );
	}
#endif /* !__MORPHOS__ */
#endif
}

