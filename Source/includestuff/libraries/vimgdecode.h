#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#endif
#ifndef VIMGDECODE_H
#define VIMGDECODE_H
/*
 * Voyager Image Decoder
 * ---------------------
 *
 * $Id: vimgdecode.h,v 1.3 2001/06/07 19:03:00 zapek Exp $
 *
 */

#ifdef MBX
#ifdef __GNUC__
#pragma pack()
#pragma pack()
#endif
#include "mbx.h"
#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#endif
#else
#ifdef __GNUC__
#pragma pack()
#pragma pack()
#endif
#include <exec/types.h>
#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#pragma pack()
#pragma pack()
#endif
#include <exec/nodes.h>
#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#pragma pack()
#pragma pack()
#endif
#include <graphics/gfx.h>
#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#endif
#endif /* !MBX */

struct imgclient {
	struct MinNode n;
	APTR object;		// object to notify
	int privstate;
	struct imgnode *imgnode;
	int isspecial;		// 1..7
};

struct imgframenode {
	struct MinNode n;
	struct BitMap *bm;      // bitmap for this frame
	struct BitMap *maskbm;  // if applicable
	int delay;              // delay for this frame
	int disposal;           // disposal method
	int xp, yp, xs, ys;
	// private
	int numpens;            // number of pens allocated for this frame
	UBYTE pens[ 256 ];
};

#endif /* !VIMGDECODE_H */

#ifdef __GNUC__
#pragma pack()
#pragma pack()
#endif