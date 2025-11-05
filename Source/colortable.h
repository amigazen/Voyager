/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_COLORTABLE_H
#define VOYAGER_COLORTABLE_H
/*
 * $Id: colortable.h,v 1.3 2001/07/01 22:02:37 owagner Exp $
*/

#ifdef __GNUC__
struct MUI_PenSpec;
#endif

int findrgbname( char *name, ULONG *r, ULONG *g, ULONG *b );
void colspec2rgb( char *cs, struct MUI_PenSpec *spec );
void colspec2rgbvals( char *cs, ULONG *r, ULONG *g, ULONG *b );
ULONG colspec2rgb24( char *cs );
ULONG muipenspec2rgb24( APTR obj, char *penspec );

#endif /* VOYAGER_COLORTABLE_H */
