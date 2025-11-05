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


#ifndef VOYAGER_MYBRUSH_H
#define VOYAGER_MYBRUSH_H
/*
 * $Id: mybrush.h,v 1.7 2001/07/01 22:03:16 owagner Exp $
 */

#ifdef __GNUC__
#pragma pack(2)
#endif /* __GNUC__ */

struct MyImage {
		UWORD           Width;
		UWORD           Height;
		UWORD          *ImageData;
		APTR            Datatype;
		ULONG           BytesPerRow;
		UBYTE           Depth;
		UBYTE          *CReg;
		UBYTE          *Planes[8];
};

struct MyBrush {
		UWORD           Width;
		UWORD           Height;
		struct BitMap  *BitMap;
		ULONG          *Colors;
};

extern struct MyBrush *LoadBrush( STRPTR );
extern void FreeBrush( struct MyBrush * );
extern struct MyBrush *CreateEmptyBrush( int width, int height );
extern struct MyBrush *b_load( char *name );

#ifdef __GNUC__
#pragma pack()
#endif /* __GNUC__ */

#endif /* VOYAGER_MYBRUSH_H */
