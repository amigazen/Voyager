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


#ifndef VOYAGER_BITMAPCLONE_H
#define VOYAGER_BITMAPCLONE_H
/*
 * $Id: bitmapclone.h,v 1.5 2001/07/01 22:02:35 owagner Exp $
 */

struct BitMap *getclone( struct BitMap *src, int masked );
UBYTE *clone_maskplane( struct BitMap *maskbm );
void markclonemodified( struct BitMap *src );
void ASM SAVEDS removeclone( __reg( a0, struct BitMap *src ) );
#if USE_CGX
int ensure_cybergfx( void );
int bitmap_is_cybergfx( struct BitMap *bm );
int screen_is_cybergfx( struct Screen *scr );
#endif

// Hack & Kludge
extern struct Screen *destscreen;
extern int iscybermap;
#endif /* VOYAGER_BITMAPCLONE_H */
