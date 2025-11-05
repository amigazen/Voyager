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
** $Id: textfit2.c,v 1.7 2003/08/17 18:30:26 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <graphics/text.h>
#endif


void makefontarray( struct TextFont *tf, UBYTE *fa )
{
	int c;
	UWORD *space = tf->tf_CharSpace, *kern = tf->tf_CharKern;

	fa[ 256 ] = ( tf->tf_YSize + 1 ) / 2;

	if( !( tf->tf_Flags & FPF_PROPORTIONAL ) )
	{
		memset( fa, tf->tf_XSize, 256 );
		return;
	}

	for( c = 0; c < 256; c++ )
	{
		int ch;

		if( c < tf->tf_LoChar || c > tf->tf_HiChar )
		{
			ch = tf->tf_HiChar - tf->tf_LoChar + 1;
		}
		else
		{
			ch = c - tf->tf_LoChar;
		}
		fa[ c ] = kern[ ch ] + space[ ch ];
	}
}
