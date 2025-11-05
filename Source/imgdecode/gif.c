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
 * $Id: gif.c,v 1.21 2003/07/06 16:51:35 olli Exp $
 */

#ifdef __SASC
#include "gst.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/timer.h>
#include <proto/rexxsyslib.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/iffparse.h>
#include <proto/layers.h>
#include <exec/execbase.h>

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <devices/keyboard.h>
#include <dos/dostags.h>
#include <datatypes/pictureclass.h>
#include <graphics/gfxmacros.h>
#define USE_BUILTIN_MATH
#include <string.h>
/*
 * Built in SAS/C function is bugged when being used with the
 * optimizer
 */
#undef memcmp
#endif /* __SASC */

#ifdef MBX
#include "mbx.h"
#endif /* MBX */

#include "globals.h"

#include <math.h>

#include "gif.h"
#include "gifdecode.h"

static void InitLZWCode( struct lzw_context *sinfo );
static void ReInitLZW( struct lzw_context *sinfo );
static int __inline GetCode( struct lzw_context *sinfo );
static int LZWReadByte( struct lzw_context *sinfo );

#define SWAB(x) ((((x)&0xff)<<8)|((x)>>8))

enum {
	GIFS_INIT,
	GIFS_GOT_HEADER
};

extern APTR alloci( int size );
extern void freei( APTR, int );

extern int gif_read_data( APTR ud, APTR buffer, int len );

// private functions

static int gi_fill_buffer( struct gifhandle *gifh )
{
	int rc;

	//kprintf( "enter fillbuffer restart %ld bp %ld brp %ld\n", gifh->restartmark, gifh->bp, gifh->brp );

	// move data to start of buffer
	if( ( gifh->restartmark > 0 ) && ( gifh->restartmark < gifh->bp ) )
	{
		memcpy( &gifh->buffer[ 0 ], &gifh->buffer[ gifh->restartmark ], gifh->bp - gifh->restartmark );
		gifh->bp -= gifh->restartmark;
		gifh->brp -= gifh->restartmark;
		gifh->restartmark = 0;
	}

	// buffer filled?
	if( gifh->bp == RBSIZE )
		return( 0 );

	// now, try to read data
	rc = gif_read_data( gifh->userdata, &gifh->buffer[ gifh->bp ], RBSIZE - gifh->bp );
	if( rc < 0 )
		return( -1 ); // End of stream

	gifh->bp += rc;

	//kprintf( "exit fillbuffer restart %ld bp %ld brp %ld\n", gifh->restartmark, gifh->bp, gifh->brp );

	return( 0 );
}

//
// read next byte
// return 0..255 if available
// return -1 if suspended
// return -2 if end of data
//

static int gi_read_byte( struct gifhandle *gifh )
{
	if( gifh->brp == gifh->bp )
	{
		if( gi_fill_buffer( gifh ) < 0 )
			return( -2 );
	}

	if( gifh->brp < gifh->bp )
	{
		// return buffered byte
		return( gifh->buffer[ gifh->brp++ ] );
	}

	// nothing available
	return( -1 );
}

static void gi_read_buffer( struct gifhandle *gifh, APTR dest, int size )
{
	memcpy( dest, &gifh->buffer[ gifh->brp ], size );
	gifh->brp += size;
}

static int gi_prefetch( struct gifhandle *gifh )
{
	if( gi_fill_buffer( gifh ) < 0 )
	{
		if( gifh->bp - gifh->brp == 0 )
			return( -2 );  // end of stream
	}
	return( gifh->bp - gifh->brp );
}

static void gi_set_restartmark( struct gifhandle *gifh )
{
	gifh->restartmark = gifh->brp;
}

static void gi_reset_to_restartmark( struct gifhandle *gifh )
{
	gifh->brp = gifh->restartmark;
}

// public functions

struct gifhandle *gif_init( APTR userdata )
{
	struct gifhandle *gifh = alloci( sizeof( *gifh ) );
	int c;

	if( !gifh )
		return( NULL );

	gifh->userdata = userdata;

	// preset global color table with grayscale
	for( c = 1; c < 256; c++ )
	{
		gifh->cmap_global[ c * 3 + 0 ] = 256 - c;
		gifh->cmap_global[ c * 3 + 1 ] = 256 - c;
		gifh->cmap_global[ c * 3 + 2 ] = 256 - c;
	}

	gifh->repeatcnt = -1;

	return( gifh );
}

void gif_free( struct gifhandle *gifh )
{
	freei( gifh, sizeof( *gifh ) );
}

//
// return codes:
// 0 = suspended
// 1 = header ok
// -1 = not a GIF file, or EOF (if appropriate)
//

struct gifheader {
	char sigver[ 6 ];
	UWORD width, height;
	UBYTE flags;
	UBYTE backcolor;
	UBYTE ratio;
};

int gif_read_header( struct gifhandle *gifh, int *xs, int *ys )
{
	int rc;
	struct gifheader header;

	// not enough data for GIF header yet?
	gi_reset_to_restartmark( gifh );

	rc = gi_prefetch( gifh );

	if( rc < 0 )
		return( -1 );
	else if( rc < 13 )
		return( 0 );

	// we do have enough data
	gi_read_buffer( gifh, &header, 13 );

	if( memcmp( header.sigver, "GIF87a", 6 ) && memcmp( header.sigver, "GIF89a", 6 ) )
	{
		return( -1 );
	}

	gifh->width = SWAB( header.width );
	gifh->height = SWAB( header.height );

	gifh->cmap_global_depth = 1L << ( ( header.flags & 0x7 ) + 1 );

	//kprintf( "GIF: w = %ld h = %ld depth %ld flag %lx\n", gifh->width, gifh->height, gifh->cmap_global_depth, header.flags );

	// global color table?
	if( header.flags & 0x80 )
	{
		if( gi_prefetch( gifh ) < ( gifh->cmap_global_depth * 3 ) )
			return( 0 ); // not in memory yet
		gi_read_buffer( gifh, gifh->cmap_global, gifh->cmap_global_depth * 3 );
	}

	*xs = gifh->width;
	*ys = gifh->height;

	// set restart mark
	gi_set_restartmark( gifh );

	gifh->local_transparent = -1;

	return( 1 );
}

// Begin reading of image
// Read any extension blocks, skipping uninteresting ones
// returns:

// 1 = image read successfully
// 0 = suspended
// -1 = no more images
// -2 = other error

int gif_begin_image( struct gifhandle *gifh, int *xs, int *ys, int *xoffset, int *yoffset, int *depth, int *delay, int *disposal, int *mask, int *repeatcnt )
{
	int rc;
	UBYTE ext_data[ 256 ];
	UBYTE ext_data_2[ 256 ];

	gi_reset_to_restartmark( gifh );

	// we read data until we hit the trailer or an
	// image descriptor
	while( !gifh->gotdescriptor )
	{
		int ext_type, blk_size, ext_size;

		rc = gi_read_byte( gifh );

		if( rc == -1 )
			return( 0 );

		else if( rc == -2 )
		{
			//kprintf( "out of bytes\n" );
			return( -1 );
		}

		if( !rc )
			continue;

		//kprintf( "initbit %lx (%lc)\n", rc, rc );

		if( rc == 0x2c )
			break; // image descriptor hit, bail out
		else if( rc == 0x3b || !rc )
		{
			//kprintf( "eos rc = %ld\n", rc );
			return( -1 );	// end of data stream, goodbye
		}
		else if( rc != 0x21 )
		{
			//kprintf( "expected 0x21, got %lx\n", rc );
			//return( -2 );	// we expected a introducer, but got shit
		}

		ext_type = gi_read_byte( gifh );

		//kprintf( "ext_type = %lx\n", ext_type );

		if( ext_type == -1 )
			return( 0 );	// 2 bad
		else if( ext_type == -2 )
			return( -2 );	// we're nuked

		ext_size = 0;

		for(;;)
		{
			blk_size = gi_read_byte( gifh );

			//kprintf( "blk_size = %ld\n", blk_size );

			if( blk_size == -1 )
				return( 0 );	// 2 bad, suspended
			else if( blk_size == -2 )
				return( -2 );	// we're nuked
			if( !blk_size )
				break; // end of data blocks

			if( gi_prefetch( gifh ) < blk_size )
				return( 0 );	// we need more data to continue

			// we only read the first data block anyway, no matter what
			if( !ext_size )
			{
				ext_size = blk_size;
				gi_read_buffer( gifh, ext_data, min( 256, ext_size ) );
			}
			else
			{
				// skip dummy data
				gi_read_buffer( gifh, ext_data_2, min( 256, blk_size ) );
			}
		}

		// eat block terminator

		// when we're here, we hit a valid extension block and loaded
		// it's first data block
		switch( ext_type )
		{
			case 0xf9:	// graphic control block
				gifh->local_delay = ext_data[ 1 ] | ( ext_data[ 2 ] << 8 );
				if( ext_data[ 0 ] & 0x1 )
					gifh->local_transparent = ext_data[ 3 ];
				else
					gifh->local_transparent = -1;
				gifh->local_disposal = ( ext_data[ 0 ] >> 2 ) & 0x7;
				break;

			case 0xff: // Application
				if( !memcmp( &ext_data[ 0 ], "NETSCAPE2.0", 11 ) )
					gifh->repeatcnt = ext_data_2[ 1 ] | ( ext_data_2[ 2 ] << 8 );
				break;
		}

		// since the extension data has been read, we can dispose
		// any data
		gi_set_restartmark( gifh );
	}

	if( !gifh->gotdescriptor )
	{
		// If we're here, next data is the image_descriptor block
		if( gi_prefetch( gifh ) < 9 )
			return( 0 ); // sorry guys, but we really want to read it

		gi_read_buffer( gifh, ext_data, 9 );

		gifh->local_xp = ext_data[ 0 ] | ( ext_data[ 1 ] << 8 );
		gifh->local_yp = ext_data[ 2 ] | ( ext_data[ 3 ] << 8 );
		gifh->local_xs = ext_data[ 4 ] | ( ext_data[ 5 ] << 8 );
		gifh->local_ys = ext_data[ 6 ] | ( ext_data[ 7 ] << 8 );

		gifh->gotdescriptor = 1;

		gi_set_restartmark( gifh );

		gifh->local_interlaced = ext_data[ 8 ] & 0x40;
		if( ext_data[ 8 ] & 0x80 )
		{
			gifh->local_colortable = TRUE;
			gifh->cmap_local_depth = 1L << ( ( ext_data[ 8 ] & 0x7 ) + 1 );
		}
		else
		{
			// use global colortable
			memcpy( gifh->cmap_local, gifh->cmap_global, sizeof( gifh->cmap_local ) );
			gifh->cmap_local_depth = gifh->cmap_global_depth;
		}
	}

	if( gifh->local_colortable )
	{
		// we need to read the fucking colortable
		if( gi_prefetch( gifh ) < ( gifh->cmap_local_depth * 3 ) )
			return( 0 );	// sorry guys...
		gi_read_buffer( gifh, gifh->cmap_local, gifh->cmap_local_depth * 3 );
	}

	*xs = gifh->local_xs;
	*ys = gifh->local_ys;
	*xoffset = gifh->local_xp;
	*yoffset = gifh->local_yp;
	*depth = gifh->cmap_local_depth;
	*delay = gifh->local_delay;
	*disposal = gifh->local_disposal;
	*mask = gifh->local_transparent;
	*repeatcnt = gifh->repeatcnt;

	memset( &gifh->context, 0, sizeof( gifh->context ) );
	gifh->context.gifh = gifh;

	// We're ready to setup the decoder and read actually image data. Wee :)
	gifh->context.input_code_size = gi_read_byte( gifh );
	if( gifh->context.input_code_size < 0 )
	{
		return( 0 );
	}

	//kprintf( "decoder init, code size = %ld\n", gifh->context.input_code_size );

	InitLZWCode( &gifh->context );

	gi_set_restartmark( gifh );

	gifh->linecnt = 0;
	gifh->interlacepass = 0;
	gifh->gotbytes = 0;

	return( 1 );
}

void gif_read_colormap( struct gifhandle *gifh, UBYTE *to )
{
	memcpy( to, gifh->cmap_local, sizeof( gifh->cmap_local ) );
}

void gif_endimage( struct gifhandle *gifh )
{
	gi_set_restartmark( gifh );

	gifh->gotdescriptor = FALSE;
	gifh->local_transparent = -1;
	gifh->local_colortable = FALSE;
	gifh->local_disposal = FALSE;
}

//
// Return codes
// 0 = suspended
// 1 = read line, pass 1 of interlace
// 2 = read line, pass 2 of interlace
// 3 = read line, pass 3 of interlace
// 4 = read line, pass 4 of interlace
// -1 = image done
// -2 = error
int gif_readscanline( struct gifhandle *gifh, UBYTE *to, int *line )
{
	if( gifh->linecnt >= gifh->local_ys )
		return( -1 );

	//kprintf( "begin read line, offset %ld\n", gifh->gotbytes );

	while( gifh->gotbytes < gifh->local_xs )
	{
		int rc;
		rc = LZWReadByte( &gifh->context );

		if( rc == -1 )
			return( 0 ); // Suspended
		else if( rc == -3 )
			return( -1 );
		else if( rc == -2 ) // Error of some kind
		{
			return( -2 );
		}
		gifh->linebuffer[ gifh->gotbytes++ ] = rc;
	}
	memcpy( to, gifh->linebuffer, gifh->gotbytes );
	*line = gifh->linecnt;

	gifh->gotbytes = 0;

	if( gifh->local_interlaced )
	{
		int illine = gifh->interlacepass + 1;
		switch( gifh->interlacepass )
		{
			case 0:
				gifh->linecnt += 8;
				if( gifh->linecnt >= gifh->local_ys )
				{
					gifh->interlacepass++;
					if( gifh->local_ys == 2 )
						gifh->linecnt = 1;
					else
						gifh->linecnt = 4 % gifh->local_ys;
				}
				break;

			case 1:
				gifh->linecnt += 8;
				if( gifh->linecnt >= gifh->local_ys )
				{
					gifh->interlacepass++;
					gifh->linecnt = 2 % gifh->local_ys;
				}
				break;

			case 2:
				gifh->linecnt += 4;
				if( gifh->linecnt >= gifh->local_ys )
				{
					gifh->interlacepass++;
					gifh->linecnt = 1;
				}
				break;

			case 3:
				gifh->linecnt += 2;
				break;
		}
		return( illine );
	}
	else
	{
		gifh->linecnt++;
		return( 4 );
	}
}

//
// decoder module
//

static void ReInitLZW( struct lzw_context *sinfo )
/* (Re)initialize LZW state; shared code for startup and Clear processing */
{
  sinfo->code_size = sinfo->input_code_size + 1;
  sinfo->limit_code = sinfo->clear_code << 1;	/* 2^code_size */
  sinfo->max_code = sinfo->clear_code + 2;	/* first unused code value */
  sinfo->sp = sinfo->symbol_stack;		/* init stack to empty */
}


static void InitLZWCode( struct lzw_context *sinfo )
/* Initialize for a series of LZWReadByte (and hence GetCode) calls */
{
  /* GetCode initialization */
  sinfo->last_byte = 2;		/* make safe to "recopy last two bytes" */
  sinfo->last_bit = 0;		/* nothing in the buffer */
  sinfo->cur_bit = 0;		/* force buffer load on first call */
  sinfo->out_of_blocks = FALSE;

  /* LZWReadByte initialization: */
  /* compute special code values (note that these do not change later) */
  sinfo->clear_code = 1 << sinfo->input_code_size;
  sinfo->end_code = sinfo->clear_code + 1;
  sinfo->first_time = TRUE;

  ReInitLZW( sinfo );
}


static int __inline GetCode( struct lzw_context *sinfo )
/* Fetch the next code_size bits from the GIF data */
/* We assume code_size is less than 16 */
{
	register int accum;
	int offs, ret, count;
	static int code_mask[ 16 ] = {
		0,
		0x0001, 0x0003,
		0x0007, 0x000F,
		0x001F, 0x003F,
		0x007F, 0x00FF,
		0x01FF, 0x03FF,
		0x07FF, 0x0FFF,
		0x1fff, 0x3fff,
		0x7fff
	};

	while( ( sinfo->cur_bit + sinfo->code_size ) > sinfo->last_bit )
	{
	    /* Time to reload the buffer */
	    if( sinfo->out_of_blocks )
		{
			//WARNMS(sinfo->cinfo, JWRN_GIF_NOMOREDATA);
			return( sinfo->end_code );	/* fake something useful */
	    }
		/* preserve last two bytes of what we have -- assume code_size <= 16 */
		sinfo->code_buf[ 0 ] = sinfo->code_buf[ sinfo->last_byte - 2 ];
		sinfo->code_buf[ 1 ] = sinfo->code_buf[ sinfo->last_byte - 1 ];

		/* Load more bytes; set flag if we reach the terminator block */
		count = gi_read_byte( sinfo->gifh );

		//kprintf( "count %ld\n", count );

		if( count == -1 )
			return( -1 ); // Suspended
		else if( count == -2 )
			return( sinfo->end_code );

		if( !count )
		{
			sinfo->out_of_blocks = TRUE;
			return( sinfo->end_code );
		}

		ret = gi_prefetch( sinfo->gifh );
		//kprintf( "prefetch %ld\n", ret );

		if( ret < count )
		{
			gi_reset_to_restartmark( sinfo->gifh );
			return( -1 ); // Suspended
		}

		gi_read_buffer( sinfo->gifh, &sinfo->code_buf[ 2 ], count );
		gi_set_restartmark( sinfo->gifh );

	    /* Reset counters */
	    sinfo->cur_bit = (sinfo->cur_bit - sinfo->last_bit) + 16;
	    sinfo->last_byte = 2 + count;
	    sinfo->last_bit = sinfo->last_byte * 8;
	}

	/* Form up next 24 bits in accum */
	offs = sinfo->cur_bit >> 3;	/* byte containing cur_bit */
	accum = sinfo->code_buf[offs+2];
	accum <<= 8;
	accum |= sinfo->code_buf[offs+1];
	accum <<= 8;
	accum |= sinfo->code_buf[offs];

	/* Right-align cur_bit in accum, then mask off desired number of bits */
	accum >>= (sinfo->cur_bit & 7);
	//ret = ((int) accum) & ((1 << sinfo->code_size) - 1);
	ret = (int)accum & ( code_mask[ sinfo->code_size ] );

	sinfo->cur_bit += sinfo->code_size;

	return ret;
}


static int LZWReadByte( struct lzw_context *sinfo )
/* Read an LZW-compressed byte */
{
	register int code;		/* current working code */
	int incode;			/* saves actual input code */

	/* First time, just eat the expected Clear code(s) and return next code, */
	/* which is expected to be a raw byte. */
	if( sinfo->first_time )
	{
		sinfo->first_time = FALSE;
		code = sinfo->clear_code;	/* enables sharing code with Clear case */
	}
	else
	{
		/* If any codes are stacked from a previously read symbol, return them */
		if( sinfo->sp > sinfo->symbol_stack )
			return (int) *(-- sinfo->sp);

		/* Time to read a new symbol */
		code = GetCode(sinfo);

		if( code < 0 )
			return( code );
	}

	if( code == sinfo->clear_code )
	{
		/* Reinit state, swallow any extra Clear codes, and */
		/* return next code, which is expected to be a raw byte. */
		ReInitLZW( sinfo );
		do {
			code = GetCode(sinfo);
			if( code < 0 )
			{
				sinfo->first_time = TRUE;
				return( code );
			}
		} while( code == sinfo->clear_code );

		if( code > sinfo->clear_code )
		{ /* make sure it is a raw byte */
			//WARNMS(sinfo->cinfo, JWRN_GIF_BADDATA);
			code = 0;			/* use something valid */
		}
		/* make firstcode, oldcode valid! */
		sinfo->firstcode = sinfo->oldcode = code;
		return code;
	}

	if( code == sinfo->end_code )
	{
		/* Skip the rest of the image, unless GetCode already read terminator */
		if( !sinfo->out_of_blocks )
		{
			GetCode( sinfo );
			//SkipDataBlocks(sinfo);
			sinfo->out_of_blocks = TRUE;
		}
		/* Complain that there's not enough data */
		//WARNMS(sinfo->cinfo, JWRN_GIF_ENDCODE);
		/* Pad data with 0's */
		return -3;
	}

	/* Got normal raw byte or LZW symbol */
	incode = code;		/* save for a moment */

	if( code >= sinfo->max_code )
	{ /* special case for not-yet-defined symbol */
		/* code == max_code is OK; anything bigger is bad data */
		if( code > sinfo->max_code )
		{
			//WARNMS(sinfo->cinfo, JWRN_GIF_BADDATA);
			incode = 0;		/* prevent creation of loops in symbol table */
		}
		/* this symbol will be defined as oldcode/firstcode */
		*(sinfo->sp++) = (UBYTE) sinfo->firstcode;
		code = sinfo->oldcode;
	}

	/* If it's a symbol, expand it into the stack */
	while (code >= sinfo->clear_code)
	{
		*(sinfo->sp++) = sinfo->symbol_tail[code]; /* tail is a byte value */
		code = sinfo->symbol_head[code]; /* head is another LZW symbol */
	}
	/* At this point code just represents a raw byte */
	sinfo->firstcode = code;	/* save for possible future use */

	/* If there's room in table, */
	if( ( code = sinfo->max_code ) < LZW_TABLE_SIZE )
	{
	   /* Define a new symbol = prev sym + head of this sym's expansion */
		sinfo->symbol_head[ code ] = sinfo->oldcode;
		sinfo->symbol_tail[ code ] = (UBYTE) sinfo->firstcode;
		sinfo->max_code++;
		/* Is it time to increase code_size? */
		if ((sinfo->max_code >= sinfo->limit_code) &&
			(sinfo->code_size < MAX_LZW_BITS))
		{
			sinfo->code_size++;
			sinfo->limit_code <<= 1;	/* keep equal to 2^code_size */
		}
	}

	sinfo->oldcode = incode;	/* save last input symbol for future use */
	return sinfo->firstcode;	/* return first byte of symbol's expansion */
}
