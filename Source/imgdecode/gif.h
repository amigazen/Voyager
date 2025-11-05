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

/*
 * $Id: gif.h,v 1.7 2001/07/08 14:03:30 owagner Exp $
 */

#define RBSIZE 2048

#include "gifdecode.h"

struct gifhandle {
	int state;
	char buffer[ RBSIZE ];
	int bp;
	int brp;
	int restartmark;
	APTR userdata;

	int width;
	int height;

	UBYTE cmap_global[ 256 * 3 ];
	int cmap_global_depth;
	UBYTE cmap_local[ 256 * 3 ];
	int cmap_local_depth;

	int transparent_color;
	int gotdescriptor;
	
	int local_xs;
	int local_ys;
	int local_xp;
	int local_yp;
	
	int local_depth;
	int local_delay;
	int local_disposal;
	int local_transparent;
	int local_interlaced;
	int local_colortable;

	int gotbytes;
	int linecnt;
	int interlacepass;

	int repeatcnt;

	UBYTE linebuffer[ 2048 ];

	struct lzw_context context;
};

struct gifhandle *gif_init( APTR userdata );
int gif_read_header( struct gifhandle *gifh, int *xs, int *ys );
int gif_begin_image( struct gifhandle *gifh, int *xs, int *ys, int *xoffset, int *yoffset, int *depth, int *delay, int *disposal, int *mask, int *repeatcnt );
void gif_read_colormap( struct gifhandle *gifh, UBYTE *to );
void gif_endimage( struct gifhandle *gifh );
void gif_free( struct gifhandle *gifh );
int gif_readscanline( struct gifhandle *gifh, UBYTE *to, int *line );
