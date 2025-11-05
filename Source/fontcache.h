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


#ifndef VOYAGER_FONTCACHE_H
#define VOYAGER_FONTCACHE_H
/*
 * $Id: fontcache.h,v 1.5 2001/07/01 22:02:43 owagner Exp $
 */

struct TextFont *myopenfont( char *name, char **fontarray );
struct TextFont *getfont( char *facespec, int sizespec, UBYTE **cfa );
void cleanup_fontcache( void );

#endif /* VOYAGER_FONTCACHE_H */
