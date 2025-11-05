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


#ifndef VOYAGER_TEXTFIT_H
#define VOYAGER_TEXTFIT_H
/*
 * $Id: textfit.h,v 1.4 2001/07/01 22:03:32 owagner Exp $
 */

extern ASM int patextfit( __reg(a0, STRPTR text), __reg(a1, UBYTE *warray), __reg(d0, UWORD textlen), __reg(d1, UWORD pixelsize), __reg(d2, UBYTE softstyles) );
extern ASM int patextlen( __reg(a0, STRPTR text), __reg(a1, UBYTE *warray), __reg(d0, UWORD textlen), __reg(d1, UBYTE softstyles) );

#endif /* VOYAGER_TEXTFIT_H */
