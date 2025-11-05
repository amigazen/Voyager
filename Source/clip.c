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
 * Clipboard functions
 * -------------------
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: clip.c,v 1.6 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/iffparse.h>
#endif

int	storetoclip( STRPTR txt )
{
#if USE_CLIPBOARD
	struct IFFHandle *iff;
	APTR clip;
	int rc = FALSE;

	clip = OpenClipboard( 0 );

	if( clip )
	{
		if( ( iff = AllocIFF() ) )
		{
			iff->iff_Stream = (ULONG)clip;
			InitIFFasClip( iff );

			if( OpenIFF( iff, IFFF_WRITE ) == 0 )
			{
				if( PushChunk( iff, MAKE_ID( 'F','T','X','T' ), MAKE_ID( 'F','O','R','M' ), IFFSIZE_UNKNOWN ) != 0 )
					goto err;

				if( PushChunk( iff, MAKE_ID( 'F','T','X','T' ), MAKE_ID( 'C','H','R','S' ), IFFSIZE_UNKNOWN ) != 0 )
					goto err;

				if( WriteChunkBytes( iff, txt, strlen( txt ) ) < 0 )
					goto err;

				if( PopChunk( iff ) != 0 )
					goto err;

				if( PopChunk( iff ) != 0 )
					goto err;

				rc = TRUE;

				err:
				CloseIFF( iff );
			}
			CloseClipboard( clip );
			FreeIFF( iff );
		}
		else
		{
			CloseClipboard( clip );
		}
	}

	return( rc );
#else
	return 0;
#endif	
}
