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
 * Fast text fitting routines
 * --------------------------
 * - asm version available for 68k
 *
 * $Id: textfit.c,v 1.9 2003/08/17 18:30:26 olli Exp $
 */

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <graphics/clip.h>
#include <graphics/text.h>
#endif

/* private */
#include "textfit.h"

/*
 * Returns the length of a string in pixels
 *
 * text - text pointer
 * warray - pointer to space array
 * textlen - length of text
 * softstyle - soft styles
 */
int patextlen( STRPTR text, UBYTE *warray, UWORD textlen, UBYTE softstyle )
{
	int res = 0;

	if( softstyle & FSF_BOLD )
		res++;

	if( softstyle & FSF_ITALIC )
		res++;

	while( textlen-- )
	{
		res += warray[ *text++ ];
	}
	return( res );
}

/*
 * Returns the number of characters that fits given pixels.
 *
 * text - text pointer
 * warray - pointer to space array
 * textlen - length of text
 * pixelsize - pixel width constraint
 * softstyle - soft styles
 */
int patextfit( STRPTR text, UBYTE *warray, UWORD textlen, UWORD pixelsize, UBYTE softstyle )
{
	int ps = pixelsize;
	UWORD tl = textlen;

	if( softstyle & FSF_BOLD )
		ps--;

	if( softstyle & FSF_ITALIC )
		ps--;

	while(tl)
	{
		ps -= warray[ *text++ ];
		if(ps<0)
			break;
		tl--;
	}
	return( ( int )textlen - tl );

}

BOOL _IsRectangleVisibleInLayer(struct Layer *layer, WORD x0, WORD y0, WORD x1, WORD y1)
{
   struct ClipRect *cr;
   x0 += layer->bounds.MinX;
   y0 += layer->bounds.MinY;
   x1 += layer->bounds.MinX;
   y1 += layer->bounds.MinY;
   for (cr=layer->ClipRect;cr;cr=cr->Next)
   {
	   if (x0 > cr->bounds.MaxX) continue;
	   if (y0 > cr->bounds.MaxY) continue;
	   if (x1 < cr->bounds.MinX) continue;
	   if (y1 < cr->bounds.MinY) continue;
	   return(TRUE);
   }
   return(FALSE);
}
