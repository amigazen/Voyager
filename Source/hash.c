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
 * Computes a 32-bit hash value quickly.
 * m68k version available in hash.s
 *
 * $Id: hash.c,v 1.6 2003/07/06 16:51:33 olli Exp $
 */

#include "voyager.h"

ULONG hash( char *p )
{
	ULONG ret = 0;

	while( *p )
	{
		ret = ret << 3;
		ret += (*p-32);
		ret ^= 0x711d4309;
		p++;
	}
	return( ret );
}
