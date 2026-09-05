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
	int w;
	int ch;
	int fixed;
	WORD *space;
	WORD *kern;

	if( !tf || !fa )
		return;

	w = ( tf->tf_YSize + 1 ) / 2;
	if( w < 1 )
		w = 1;
	if( w > 255 )
		w = 255;
	fa[ 256 ] = (UBYTE)w;

	fixed = tf->tf_XSize;
	if( fixed < 1 )
		fixed = 8;
	if( fixed > 255 )
		fixed = 255;

	space = (WORD *)tf->tf_CharSpace;
	kern = (WORD *)tf->tf_CharKern;

	/* Outline/TTF conversions often set FPF_PROPORTIONAL with NULL
	 * kerning tables. Treating those pointers as UWORD also wraps
	 * negative kerning into 0-width glyphs, so FACE=Georgia/Times
	 * text laid out to nothing while default-font cells looked fine. */
	if( !( tf->tf_Flags & FPF_PROPORTIONAL ) || !space || !kern )
	{
		memset( fa, fixed, 256 );
		return;
	}

	for( c = 0; c < 256; c++ )
	{
		if( c < tf->tf_LoChar || c > tf->tf_HiChar )
			ch = tf->tf_HiChar - tf->tf_LoChar + 1;
		else
			ch = c - tf->tf_LoChar;
		w = (int)kern[ ch ] + (int)space[ ch ];
		if( w < 1 )
			w = 1;
		if( w > 255 )
			w = 255;
		fa[ c ] = (UBYTE)w;
	}
}
