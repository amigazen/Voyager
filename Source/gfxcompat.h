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


#ifndef VOYAGER_GFXCOMPAT_H
#define VOYAGER_GFXCOMPAT_H
/*
 * Graphics compatibility functions
 * $Id: gfxcompat.h,v 1.4 2001/07/01 22:02:44 owagner Exp $
 */

// AmigaOS versions for now

#define HLine(rp,x1,x2,y) RectFill(rp,x1,y,x2,y)
#define VLine(rp,x,y1,y2) RectFill(rp,x,y1,x,y2)

#endif /* VOYAGER_GFXCOMPAT_H */
