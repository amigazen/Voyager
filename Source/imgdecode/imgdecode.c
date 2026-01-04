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
 * $Id: imgdecode.c,v 1.168 2003/09/14 18:18:54 olli Exp $
 */

#include "config.h"

#ifdef __SASC
#include "gst.h"
#endif /* __SASC */

#include "globals.h"

#if USE_CGX
#ifdef __MORPHOS__
#include <cybergraphx/cybergraphics.h>
#ifdef USE_VAT
#undef USE_VAT
#define USE_VAT 0 /* no VAT for MorphOS. It's pointless to use it, really */
#endif /* USE_VAT */
#endif /* __MORPHOS__ */
#endif /* USE_CGX */
#include <libraries/vimgdecode.h>

#include <graphics/gfxbase.h>

//#include <dos.h>
#include <errorreq.h>

#if USE_EXECUTIVE
#include "/executive_protos.h"
#endif /* USE_EXECUTIVE */

#include "imgcallback.h"
#include "network_callback.h"
#include "debug.h"

#ifdef __SASC // memcmp workaround
#undef memcmp
#endif /* __SASC */

#ifdef AMIGAOS
extern ASM mystoreds( void );
extern ASM mygetds( void );
#endif /* AMIGAOS */

#ifdef __MORPHOS__
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#endif


struct imgcallback *cbt;


int isppc;

#include <limits.h>

#ifdef __MORPHOS__
#include "rev.h"
char version[ ] = { "$VER: vimgdec_604e.vlib " LVERTAG };
/*
 * For MorphOS we use the following callback table where
 * we either provide direct jumps into V (V PPC) or our
 * function stubs (V 68k)
 */
struct imgcallback mcbt;
#define v_nets_open mcbt.nets_open
#define v_nets_close mcbt.nets_close
#define v_nets_release_buffer mcbt.nets_release_buffer
#define v_nets_getdocptr mcbt.nets_getdocptr
#define v_nets_getdocmem mcbt.nets_getdocmem
#define v_nets_lockdocmem mcbt.nets_lockdocmem
#define v_nets_unlockdocmem mcbt.nets_unlockdocmem
#define v_nets_state mcbt.nets_state
#define v_nets_settomem mcbt.nets_settomem
#define v_nets_url mcbt.nets_url
#define v_nets_redirecturl mcbt.nets_redirecturl
#define v_removeclone mcbt.removeclone
#define v_nets_errorstring mcbt.nets_errorstring
#define v_imgcallback_decode_hasinfo mcbt.imgcallback_decode_hasinfo
#define v_imgcallback_decode_gotscanline mcbt.imgcallback_decode_gotscanline
#define v_imgcallback_decode_done mcbt.imgcallback_decode_done

#else

/*
 * For 68k we use direct callback table.
 */
#define v_nets_open cbt->nets_open
#define v_nets_close cbt->nets_close
#define v_nets_release_buffer cbt->nets_release_buffer
#define v_nets_getdocptr cbt->nets_getdocptr
#define v_nets_getdocmem cbt->nets_getdocmem
#define v_nets_lockdocmem cbt->nets_lockdocmem
#define v_nets_unlockdocmem cbt->nets_unlockdocmem
#define v_nets_state cbt->nets_state
#define v_nets_settomem cbt->nets_settomem
#define v_nets_url cbt->nets_url
#define v_nets_redirecturl cbt->nets_redirecturl
#define v_removeclone cbt->removeclone
#define v_nets_errorstring cbt->nets_errorstring
#define v_imgcallback_decode_hasinfo cbt->imgcallback_decode_hasinfo
#define v_imgcallback_decode_gotscanline cbt->imgcallback_decode_gotscanline
#define v_imgcallback_decode_done cbt->imgcallback_decode_done
#endif /* !__MORPHOS__ */

#define BROKENBM ((struct BitMap*)-1)

// make jpeglib happy
#ifdef AMIGAOS
typedef APTR FILE;
#endif /* AMIGAOS */
#undef GLOBAL
#include "jpeglib.h"
#include "jversion.h"
#include "jerror.h"

#include "gif.h"

#define PNG_INTERNAL
#ifdef MBX
#define _STDLIB_H_ /* TOFIX!! HACK because of CaOS ! */
#define _STRING_H_ /* TOFIX!! too !! */
#endif /* MBX */
#include "png.h"

#if USE_CGX
struct Library *CyberGfxBase;
#endif /* USE_CGX */

#ifndef MBX
struct IntuitionBase *IntuitionBase;
struct DosLibrary *DOSBase;
struct ExecBase *SysBase;
#ifdef __MORPHOS__
struct UtilityBase *UtilityBase;
#else
struct Library *UtilityBase;
#endif
#if USE_VAT
struct Library *VATBase;
#endif /* USE_VAT */
struct GfxBase *GfxBase;
#endif /* !MBX */

#ifdef __MORPHOS__
struct EmulFunc mos_emulfunc;
struct TagItem mos_tags[ 9 ];
#endif /* __MORPHOS__ */

void free( void *ptr );

static int image_bgpen, image_framepen;
#ifndef MBX
static int image_shinepen, image_shadowpen;
#endif

#ifndef MBX
int db_level = 0;
#else
static int db_level = 0;
#endif

#include "c2p.h"

// prefs copy
long dsi_img_jpeg_dct = JDCT_ISLOW;
long dsi_img_jpeg_dither = JDITHER_NONE;
long dsi_img_jpeg_quant = FALSE;
long dsi_img_lamedecode = TRUE;
long dsi_jpeg_progressive = TRUE;
long dsi_img_gif_dither = TRUE;
long dsi_img_png_dither = TRUE;

static void flushunusednodes( void );

#define JPEG_ROWBUFFER_ROWS 4
static JSAMPROW jpeg_rowbuffer[ JPEG_ROWBUFFER_ROWS ];

#define MAXIMAGEWIDTH 4096

//
//
//

static struct MinList imglist, freelist;
static volatile struct Process *imgproc;
static APTR imgpool;
static struct SignalSemaphore imgsem;

struct myjpeg_error_mgr {
	struct jpeg_error_mgr je;
	jmp_buf jmpbuff;
};

typedef INT16 FSERROR;		/* 16 bits should be enough */
typedef int LOCFSERROR;		/* use 'int' for calculation temps */

enum {
	IMT_NONE,
	IMT_JPEG,
	IMT_GIF,
	IMT_PNG,
	IMT_XBM
};

struct imgnode {
	struct MinNode n;
	struct BitMap *bm;
	int state;
	int img_x, img_y;
	struct MinList clientlist;
	struct MinList imagelist;		// list of distinct images (for multipart images)
	struct nstream *urlnode;
	char *referer;
	int reloadflag;
	int number_of_redirects;
	int numpens;
	struct Screen *scr;				// unlock it!
	struct ColorMap *cm;
	struct imgframenode *currentimf;
	int readstate;
	char errormsg[ 128 ];
	UBYTE colmap[ 256 * 3 ];		// colormap in use

	int imagetype;		// IMT_#?
	int imgcount;		// number of frames

	int has_y;
	int min_touched_y, max_touched_y;

	int urlpos;
	int aborted;
	int do_reload;

	// jpeg stuff
	struct jpeg_decompress_struct *cinfo;
	struct myjpeg_error_mgr *jerr;
	int	jpeg_progressive;

	// gif stuff
	struct gifhandle *gifh;
	int local_xs, local_ys, local_xp, local_yp, local_mask;
	int local_mask_used, local_non_trans;
	int repeatcnt;
	int giftype;

	// PNG stuff
	png_structp png_ptr;
	png_infop png_info_ptr;
	png_bytep *pngrows;
	int pngoldpass;

	UBYTE *raw_data;

	char url[ 0 ];
};

enum {
	GS_DONE = -1,
	GS_SETTINGUP = 0,
	GS_URLWAIT,				// waiting for URL data...
	GS_READING,
	GS_HASIMGINFO,
	GS_READING2,
	GS_READING3,
	GS_NEXTIMAGE,

	GS_dummy
};

/*
 * Features
 */
#define FTB_TRUECOLOR 0	 /* true/hi-color system */
#define FTB_CYBERMAP  1  /* CyberGraphX bitmap */
#define FTB_ALPHA     2  /* CyberGraphX alpha support */

#ifndef MBX
#define FTF_TRUECOLOR (1UL << FTB_TRUECOLOR)
#if USE_CGX
#define FTF_CYBERMAP  (1UL << FTB_CYBERMAP)
#if USE_ALPHA
#define FTF_ALPHA     (1UL << FTB_ALPHA)
#endif /* USE_ALPHA */
#endif /* USE_CGX */
#endif /* !MBX */

static int features;

static struct Screen *destscreen;
static struct SignalSemaphore destscreensem;
#ifndef MBX
static char *destscreenname;
#endif /* !MBX */

APTR alloci( int size )
{
	APTR o;

	ObtainSemaphore( &imgsem );
	o = AllocPooled( imgpool, size );
	ReleaseSemaphore( &imgsem );

	return( o );
}

void freei( APTR block, int size )
{
	ObtainSemaphore( &imgsem );
	FreePooled( imgpool, block, size );
	ReleaseSemaphore( &imgsem );
}


/*
 * Frees a BitMap.
 */
static void free_bitmap( struct BitMap *bm )
{
#ifndef MBX
	int c;
#endif

	if( !bm || bm == BROKENBM )
		return;

#if USE_CGX
	if( features & FTF_CYBERMAP )
	{
		FreeBitMap( bm );
		return;
	}
#endif /* USE_CGX */

#ifdef MBX
	FreeBitMap( bm );
#else
	v_removeclone( bm );

	for( c = 0; c < bm->Depth; c++ )
	{
		if( bm->Planes[ c ] )
			FreeVec( bm->Planes[ c ] );
	}
	free( bm );
#endif /* !MBX */
}

/*
 * Allocates a BitMap (AGA-style)
 */
#ifndef MBX
static struct BitMap *alloc_bitmap( int xs, int ys, int depth )
{
	struct BitMap *bm = malloc( sizeof( *bm ) );

	if( bm )
	{
		int c;
		InitBitMap( bm, depth, xs, ys );

		for( c = 0; c < depth; c++ )
		{
			bm->Planes[ c ] = AllocVec( RASSIZE( xs, ys ), MEMF_CLEAR );
			if( !bm->Planes[ c ] )
			{
				free_bitmap( bm );
				return( NULL );
			}
		}
	}

	return( bm );
}
#endif /* !MBX */


/*
 * Copies lines, used for GIF.
 */
static void copylines( struct BitMap *bm, int fromline, int toline, int lines, int xs )
{
	//kprintf( "copyline from %ld to %ld lines %ld size %ld\r\n", fromline, toline, lines, xs );
#if USE_CGX
	if( features & FTF_CYBERMAP )
	{
		BltBitMap(
			bm, 0, fromline,
			bm, 0, toline,
			xs, lines,
			0xc0,
			-1,
			NULL
		);
	}
#endif
#ifdef MBX
	BltBitMap(
		bm, 0, fromline,
		bm, 0, toline,
		xs, lines,
		0xc0,
		-1,
		0
	); //TOFIX!! find out the minterm
#else
	else
	{
		int c;

		for( c = 0; c < bm->Depth; c++ )
		{
			int bpr = bm->BytesPerRow;
			int lc = lines;
			UBYTE *fromp = ((UBYTE*)bm->Planes[ c ]) + bpr * fromline;
			UBYTE *top = ((UBYTE*)bm->Planes[ c ]) + bpr * toline;

			while( lc-- )
			{
				CopyMem( fromp, top, bpr );
				fromp += bpr;
				top += bpr;
			}
		}
	}
#endif /* !MBX */
}


/*
 * Maskmode stuff
 */
enum {
	MASK_NONE,
	MASK_ALPHA,   /* Alpha mask */
	MASK_PLANAR   /* GIF or PNG-68k-Amiga planar mask */
};

/*
 * Allocates an imageframenode. This is used
 * for every image. Masking/alpha is allocated here too.
 */
static struct imgframenode *alloc_imfnode( struct imgnode *imn, ULONG maskmode, int xs, int ys, int xp, int yp )
{
	struct imgframenode *imf;
	int retried = FALSE;

	/*
	 * This is an AGA limitation. I don't know
	 * if FBlit removes that (neither does the author).
	 */
	//if( !( features & FTF_CYBERMAP ) && xs > 2048 )
	//
	// This is NOT an AGA limitation, but it's a limit
	// of about every image decoding subroutine used
	// further down. GRRR! Zapek, DIE!
	//
	if( xs > MAXIMAGEWIDTH )
	{
		sprintf( imn->errormsg, "Image too wide (%d pixels)", xs ); /* 128 chars */
		return( NULL );
	}


	imf = alloci( sizeof( *imf ) );
	if( !imf )
	{
		strcpy( imn->errormsg, "out of memory" );
		return( NULL );
	}

retry:

	ObtainSemaphore( &destscreensem );

	if( !destscreen )
	{
		/*
		 * We are iconified or something.
		 */
		ReleaseSemaphore( &destscreensem );
		return( NULL );
	}

	/*
	 * Allocate the bitmap + handle alpha
	 */
#if USE_CGX
	if( features & FTF_CYBERMAP )
	{
#if USE_ALPHA
		if( maskmode == MASK_ALPHA && features & FTF_ALPHA )
		{
			/*
			 * In that mode we need an ARGB source bitmap for
			 * alpha blitting.
			 */
			imf->bm = AllocBitMap(
				xs, ys,
				32,
				BMF_MINPLANES | BMF_SPECIALFMT | SHIFT_PIXFMT( PIXFMT_ARGB32 ), /* TOFIX: no vmem but that might be improved if one day a gfx card does that in hardware :) */
				NULL
			);
		}
		else
#endif /* USE_ALPHA */
		{
			imf->bm = AllocBitMap(
				xs, ys,
				GetBitMapAttr( destscreen->RastPort.BitMap, BMA_DEPTH ),
				BMF_MINPLANES,
				destscreen->RastPort.BitMap
			);
		}
		//kprintf( "imf->bm = %lx, modulo %ld\r\n", imf->bm, imf->bm->BytesPerRow );
	}
	else
#endif /* USE_CGX */
#ifdef MBX
	{
		UDWORD pixfmt;
		
		pixfmt = GetBitMapAttrs( destscreen->sc_RastPort->rp_BitMap, BMAV_PixelFormat );
		imf->bm = AllocBitMap(
			maskmode ? PIXFMT_RGB8888 : pixfmt,
			xs, ys,
			( maskmode == MASK_PLANAR ) ? BMF_ALPHABINARY : 0,
			NULL
		);
		if( maskmode == MASK_NONE && imf->bm )
		{
			SetAlphaChannelBM( imf->bm, 0, 0, xs, ys, 0xff );
		}
	}
#else
	{
		imf->bm = alloc_bitmap( xs, ys, GetBitMapAttr( destscreen->RastPort.BitMap, BMA_DEPTH ) );
	}
#endif /* !MBX */

	ReleaseSemaphore( &destscreensem );

	if( !imf->bm )
	{
		if( !retried++ )
		{
			flushunusednodes();
			goto retry;
		}

		freei( imf, sizeof( *imf ) );

		sprintf( imn->errormsg, "No memory for bitmap (%d x %d)", xs, ys );
		return( NULL );
	}

	if( maskmode )
	{
#if USE_CGX
		if( features & FTF_CYBERMAP )
		{
#if USE_ALPHA
			if( maskmode == MASK_ALPHA && features & FTF_ALPHA )
			{
				imf->maskbm = (APTR)-1;	/* just a marker */
			}
			else
#endif /* USE_ALPHA */
			{
				imf->maskbm = AllocBitMap(
					GetBitMapAttr( imf->bm, BMA_WIDTH ) , GetBitMapAttr( imf->bm, BMA_HEIGHT ),
					1,
					BMF_CLEAR,
					NULL
				);
				if( !imf->maskbm )
				{
					FreeBitMap( imf->bm );
					freei( imf, sizeof( *imf ) );
					sprintf( imn->errormsg, "No memory for mask (%d x %d)", xs, ys );
					return( NULL );
					
				}
				//kprintf( "imf->maskbm = %lx, modulo %ld\r\n", imf->maskbm, imf->maskbm->BytesPerRow );
			}
		}
		else
#endif /* USE_CGX */
#ifdef MBX
		{
			imf->maskbm = (APTR)-1; // This is just a marker really
		}
#else
		{
			imf->maskbm = alloc_bitmap( xs, ys, 1 );
		}
#endif /* !MBX */
		if( !imf->maskbm )
		{
			free_bitmap( imf->bm );

			if( !retried++ )
			{
				flushunusednodes();
				goto retry;
			}

			freei( imf, sizeof( *imf ) );

			sprintf( imn->errormsg, "No memory for mask bitmap (%d x %d)", xs, ys );

			return( NULL );
		}
	}

	imf->xs = xs;
	imf->ys = ys;
	imf->xp = xp;
	imf->yp = yp;

	if( ISLISTEMPTY( &imn->imagelist ) )
	{
		imn->bm = imf->bm;
		if( !imf->bm )
		{
			imn->bm = BROKENBM;
		}
#if USE_CGX
		else if( features & FTF_CYBERMAP )
		{
			/*
			 * Draw a rectangle around the image.
			 */
			struct RastPort rp;
			// first bitmap is always prefilled
			InitRastPort( &rp );
			rp.BitMap = imf->bm;
			SetAPen( &rp, image_bgpen );
			if( xs > 2 && ys > 2 )
			{
				RectFill( &rp, 1, 1, xs - 2, ys - 2 );
				SetAPen( &rp, image_shadowpen );
				RectFill( &rp, 0, 0, xs - 2, 0 );
				RectFill( &rp, 0, 1, 0, ys - 2 );
				SetAPen( &rp, image_shinepen );
				RectFill( &rp, xs - 1, 0, xs - 1, ys - 1 );
				RectFill( &rp, 0, ys - 1, xs - 2, ys - 1 );
			}
			else
			{
				RectFill( &rp, 0, 0, xs - 1, ys - 1 );
			}
		}
#endif /* USE_CGX */
	}

	ADDTAIL( &imn->imagelist, imf );

	imn->currentimf = imf;
	imn->imgcount++;

#ifndef MBX
	WaitBlit();
#endif /* !MBX */

	return( imf );
}

// FS dithering stuff

static LOCFSERROR *error_limit;

/*
 * Initialize the error-limiting transfer function (lookup table).
 * The raw F-S error computation can potentially compute error values of up to
 * +- MAXJSAMPLE.  But we want the maximum correction applied to a pixel to be
 * much less, otherwise obviously wrong pixels will be created.  (Typical
 * effects include weird fringes at color-area boundaries, isolated bright
 * pixels in a dark area, etc.)  The standard advice for avoiding this problem
 * is to ensure that the "corners" of the color cube are allocated as output
 * colors; then repeated errors in the same direction cannot cause cascading
 * error buildup.  However, that only prevents the error from getting
 * completely out of hand; Aaron Giles reports that error limiting improves
 * the results even with corner colors allocated.
 * A simple clamping of the error values to about +- MAXJSAMPLE/8 works pretty
 * well, but the smoother transfer function used below is even better.  Thanks
 * to Aaron Giles for this idea.
 */

static void init_fs_error_limit( void )
{
  int in, out;
  int *table;

  table = (int *) alloci( (MAXJSAMPLE*2+1) * sizeof(int));
	if( !table )
		return;

  table += MAXJSAMPLE;		/* so can index -MAXJSAMPLE .. +MAXJSAMPLE */

  error_limit = table;

#define STEPSIZE ((MAXJSAMPLE+1)/16)
  /* Map errors 1:1 up to +- MAXJSAMPLE/16 */
  out = 0;
  for (in = 0; in < STEPSIZE; in++, out++) {
	table[in] = out; table[-in] = -out;
  }
  /* Map errors 1:2 up to +- 3*MAXJSAMPLE/16 */
  for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1) {
	table[in] = out; table[-in] = -out;
  }
  /* Clamp the rest to final out value (which is (MAXJSAMPLE+1)/8) */
  for (; in <= MAXJSAMPLE; in++) {
	table[in] = out; table[-in] = -out;
  }
#undef STEPSIZE
}

#ifndef MBX
static UBYTE findmap( UBYTE r, UBYTE g, UBYTE b, UBYTE *cmap, int numpens )
{
	ULONG sr = (r>>2);
	ULONG sg = (g>>2);
	ULONG sb = (b>>2);
	ULONG lsq = 0xffffffff;
	ULONG sq,dr,dg,db,i;
	LONG p = -1;

	for(i=0;i<numpens;i++)
	{
		dr = abs(sr-((*cmap++)>>2));
		dg = abs(sg-((*cmap++)>>2));
		db = abs(sb-((*cmap++)>>2));

		sq = (dr*dr)+(dg*dg)+(db*db);

		if (sq<lsq)
		{
			lsq=sq;
			p=i;
			if (!lsq) break;
		}
	}

	return((UBYTE)p);
}
#endif /* !MBX */

#define RIGHT_SHIFT(x,shft)	((x) >> (shft))

#define HISTBITS 5
#define HISTSHIFT 3

#ifndef MBX
void do_dither(
	UBYTE *input,
	UBYTE *input_cmap,
	int xs, int ys,
	int numpens,
	struct ColorMap *cm,
	UBYTE *real_pens
)
{
	UBYTE output_cmap[ 3 * 256 ];
	int c;
	register LOCFSERROR cur0, cur1, cur2;	/* current error or pixel value */
	LOCFSERROR belowerr0, belowerr1, belowerr2; /* error for pixel below cur */
	LOCFSERROR bpreverr0, bpreverr1, bpreverr2; /* error for below/prev col */
	LOCFSERROR errors[ ( MAXIMAGEWIDTH + 2 ) * 3 ];
	int row, col;
	int dir;			/* +1 or -1 depending on direction */
	int dir3;			/* 3*dir, for advancing inptr & errorptr */
	LOCFSERROR *errorptr;
	UBYTE newcol;
	UBYTE *histo;
	int histix;

	//kprintf( "dithering %ld/%ld, np %ld\r\n", xs, ys, numpens );

	histo = alloci( 1L<<HISTBITS * 3 );
	if( !histo )
		return;

	memset( errors, 0, sizeof( errors ) );

	for( c = 0; c < numpens; c++ )
	{
		ULONG rgb[ 3 ];

		GetRGB32( cm, real_pens[ c ], 1, rgb );
		output_cmap[ c * 3 + 0 ] = rgb[ 0 ] >> 24;
		output_cmap[ c * 3 + 1 ] = rgb[ 1 ] >> 24;
		output_cmap[ c * 3 + 2 ] = rgb[ 2 ] >> 24;

		/*
		kprintf( "input %ld rgb %ld,%ld,%ld output %ld,%ld,%ld\r\n",
			c,
			input_cmap[ c * 3 + 0 ],
			input_cmap[ c * 3 + 1 ],
			input_cmap[ c * 3 + 2 ],
			output_cmap[ c * 3 + 0 ],
			output_cmap[ c * 3 + 1 ],
			output_cmap[ c * 3 + 2 ]
		);
		*/
	}

	for( row = 0; row < ys; row++ )
	{
		//kprintf( " row %ld\r\n", row );
		if( row & 1 )
		{
			input += xs - 1;
			dir = -1;
			dir3 = -3;
			errorptr = (LOCFSERROR*)&errors[ ( xs + 1 ) * 3 ];
		}
		else
		{
			errorptr = (LOCFSERROR*)errors;
			dir = 1;
			dir3 = 3;
		}

		/* Preset error values: no error propagated to first pixel from left */
		cur0 = cur1 = cur2 = 0;
		/* and no error propagated to row below yet */
		belowerr0 = belowerr1 = belowerr2 = 0;
		bpreverr0 = bpreverr1 = bpreverr2 = 0;

		for( col = xs; col > 0; col-- )
		{
			cur0 = RIGHT_SHIFT(cur0 + errorptr[dir3+0] + 8, 4);
			cur1 = RIGHT_SHIFT(cur1 + errorptr[dir3+1] + 8, 4);
			cur2 = RIGHT_SHIFT(cur2 + errorptr[dir3+2] + 8, 4);
		      /* Limit the error using transfer function set by init_error_limit.
		       * See comments with init_error_limit for rationale.
		       */
			cur0 = error_limit[cur0];
			cur1 = error_limit[cur1];
			cur2 = error_limit[cur2];

			/* Form pixel value + error, and range-limit to 0..MAXJSAMPLE.
			* The maximum error is +- MAXJSAMPLE (or less with error limiting);
			* this sets the required size of the range_limit array.
			*/
			cur0 += input_cmap[ *input * 3 + 0 ];
			cur1 += input_cmap[ *input * 3 + 1 ];
			cur2 += input_cmap[ *input * 3 + 2 ];

			if( cur0 < 0 )
				cur0 = 0;
			else if( cur0 > 255 )
				cur0 = 255;

			if( cur1 < 0 )
				cur1 = 0;
			else if( cur1 > 255 )
				cur1 = 255;

			if( cur2 < 0 )
				cur2 = 0;
			else if( cur2 > 255 )
				cur2 = 255;

			histix = (cur0>>HISTSHIFT)<<(HISTBITS*2) |
				(cur1>>HISTSHIFT)<<(HISTBITS*1) |
				(cur2>>HISTSHIFT)
			;
			if( !( newcol = histo[ histix ] ) )
			{
				newcol = findmap( cur0, cur1, cur2, output_cmap, numpens );
				histo[ histix ] = newcol;
			}

			*input = newcol;

			/* Compute representation error for this pixel */
			cur0 -= output_cmap[ newcol * 3 + 0 ];
			cur1 -= output_cmap[ newcol * 3 + 1 ];
			cur2 -= output_cmap[ newcol * 3 + 2 ];

			//kprintf( "  errors %ld %ld %ld\r\n", cur0, cur1, cur2 );

			/* Compute error fractions to be propagated to adjacent pixels.
			* Add these into the running sums, and simultaneously shift the
			* next-line error sums left by 1 column.
			*/
			{
				register LOCFSERROR bnexterr, delta;

				bnexterr = cur0;	/* Process component 0 */
				delta = cur0 * 2;
				cur0 += delta;		/* form error * 3 */
				errorptr[0] = (FSERROR) (bpreverr0 + cur0);
				cur0 += delta;		/* form error * 5 */
				bpreverr0 = belowerr0 + cur0;
				belowerr0 = bnexterr;
				cur0 += delta;		/* form error * 7 */
				bnexterr = cur1;	/* Process component 1 */
				delta = cur1 * 2;
				cur1 += delta;		/* form error * 3 */
				errorptr[1] = (FSERROR) (bpreverr1 + cur1);
				cur1 += delta;		/* form error * 5 */
				bpreverr1 = belowerr1 + cur1;
				belowerr1 = bnexterr;
				cur1 += delta;		/* form error * 7 */
				bnexterr = cur2;	/* Process component 2 */
				delta = cur2 * 2;
				cur2 += delta;		/* form error * 3 */
				errorptr[2] = (FSERROR) (bpreverr2 + cur2);
				cur2 += delta;		/* form error * 5 */
				bpreverr2 = belowerr2 + cur2;
				belowerr2 = bnexterr;
				cur2 += delta;		/* form error * 7 */
			}
			/* At this point curN contains the 7/16 error value to be propagated
			 * to the next pixel on the current line, and all the errors for the
			 * next line have been shifted over.  We are therefore ready to move on.
			 */
			input += dir;		/* Advance pixel pointers to next column */
			errorptr += dir3;		/* advance errorptr to current column */
		}

		/* Post-loop cleanup: we must unload the final error values into the
		* final fserrors[] entry.  Note we need not unload belowerrN because
		* it is for the dummy column before or after the actual array.
		*/
		errorptr[0] = (FSERROR) bpreverr0; /* unload prev errs into array */
		errorptr[1] = (FSERROR) bpreverr1;
		errorptr[2] = (FSERROR) bpreverr2;

		if( row & 1 )
			input += xs;
	}

	freei( histo, 1L<<HISTBITS * 3 );
}
#endif /* !MBX */

//
// JPeg sub module
//

typedef struct {
	struct jpeg_source_mgr pub;	/* public fields */
	struct imgnode *imgnode;
	int urlpos;					// last read pos
	int stilltoskip;
} my_src_mgr;

#define JMESSAGE(code,string)	string ,

const char * const jpeg_std_message_table[] = {
//#include "jerror.h"
  NULL
};

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_mgr *src = (my_src_mgr*)cinfo->src;
	int hlen = v_nets_getdocptr( src->imgnode->urlnode ) - src->urlpos;

	//kprintf( "entering fill_input_buffer hlen %ld pos %ld\r\n", hlen, src->urlpos );

	// Suspend -- no new data?
	if( hlen <= 0 )
		return FALSE;

	if( src->stilltoskip )
	{
		int wecan = min( src->stilltoskip, hlen );

		//kprintf( "we have to skip %ld\r\n", src->stilltoskip );

		src->urlpos += wecan;
		src->stilltoskip -= wecan;
		hlen -= wecan;

		// still excess data?
		if( src->stilltoskip || !hlen )
			return( FALSE );
	}

	src->pub.next_input_byte = v_nets_getdocmem( src->imgnode->urlnode ) + src->urlpos;
	src->pub.bytes_in_buffer = hlen;
	src->urlpos += hlen;

	return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_mgr *src = (my_src_mgr*)cinfo->src;
	int wecan;

	//kprintf( "skip %ld\r\n", num_bytes );

	// Nothing to do
	if( num_bytes <= 0 )
		return;

	// Determine number of bytes we can skip
	wecan = min( num_bytes, src->pub.bytes_in_buffer );

	//kprintf( "wecan %ld\r\n", wecan );

	src->pub.next_input_byte += wecan;
	src->pub.bytes_in_buffer -= wecan;

	src->stilltoskip += num_bytes - wecan;
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
	my_src_mgr *src = (my_src_mgr*)cinfo->src;

	if( src->imgnode->urlnode )
	{
		v_nets_release_buffer( src->imgnode->urlnode );
		v_nets_close( src->imgnode->urlnode );
		src->imgnode->urlnode = NULL;
	}
}

//
// JPeg error manager functions
//

METHODDEF(void)
error_exit (j_common_ptr cinfo)
{
	struct myjpeg_error_mgr *jerr = (struct myjpeg_error_mgr*)cinfo->err;

	//kprintf( "jpeg internal error!\r\n" );

	longjmp( jerr->jmpbuff, 1 );
  (*cinfo->err->output_message) (cinfo);

#if 0
  /* Always display the message */

  //kprintf( "errorexit\r\n" );

  /* Let the memory manager delete any temp files before we die */
  jpeg_destroy(cinfo);

  exit(EXIT_FAILURE);
#endif
}


/*
 * Actual output of an error or trace message.
 * Applications may override this method to send JPEG messages somewhere
 * other than stderr.
 */

METHODDEF(void)
output_message (j_common_ptr cinfo)
{
#if 0
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);

  /* Send it to stderr, adding a newline */
  //fprintf(stderr, "%s\r\n", buffer);
  kprintf( "%s\r\n", buffer );
#endif
}


/*
 * Decide whether to emit a trace or warning message.
 * msg_level is one of:
 *   -1: recoverable corrupt-data warning, may want to abort.
 *    0: important advisory messages (always display to user).
 *    1: first level of tracing detail.
 *    2,3,...: successively more detailed tracing messages.
 * An application might override this method if it wanted to abort on warnings
 * or change the policy about which messages to display.
 */

METHODDEF(void)
emit_message (j_common_ptr cinfo, int msg_level)
{
#if 0
  struct jpeg_error_mgr * err = cinfo->err;

  if (msg_level < 0) {
	/* It's a warning message.  Since corrupt files may generate many warnings,
	 * the policy implemented here is to show only the first warning,
	 * unless trace_level >= 3.
	 */
	if (err->num_warnings == 0 || err->trace_level >= 3)
	  (*err->output_message) (cinfo);
	/* Always count warnings in num_warnings. */
	err->num_warnings++;
  } else {
	/* It's a trace message.  Show it if trace_level >= msg_level. */
	if (err->trace_level >= msg_level)
	  (*err->output_message) (cinfo);
  }
#endif
}


/*
 * Format a message string for the most recent JPEG error or message.
 * The message is stored into buffer, which should be at least JMSG_LENGTH_MAX
 * characters.  Note that no '\n' character is added to the string.
 * Few applications should need to override this method.
 */

METHODDEF(void)
format_message (j_common_ptr cinfo, char * buffer)
{
#if 0
  struct jpeg_error_mgr * err = cinfo->err;
  int msg_code = err->msg_code;
  const char * msgtext = NULL;
  const char * msgptr;
  char ch;
  boolean isstring;

  /* Look up message string in proper table */
  if (msg_code > 0 && msg_code <= err->last_jpeg_message) {
	msgtext = err->jpeg_message_table[msg_code];
  } else if (err->addon_message_table != NULL &&
	     msg_code >= err->first_addon_message &&
	     msg_code <= err->last_addon_message) {
	msgtext = err->addon_message_table[msg_code - err->first_addon_message];
  }

  /* Defend against bogus message number */
  if (msgtext == NULL) {
	err->msg_parm.i[0] = msg_code;
	msgtext = err->jpeg_message_table[0];
  }

  /* Check for string parameter, as indicated by %s in the message text */
  isstring = FALSE;
  msgptr = msgtext;
  while ((ch = *msgptr++) != '\0') {
	if (ch == '%') {
	  if (*msgptr == 's') isstring = TRUE;
	  break;
	}
  }

  /* Format the message into the passed buffer */
  if (isstring)
	sprintf(buffer, msgtext, err->msg_parm.s);
  else
	sprintf(buffer, msgtext,
	    err->msg_parm.i[0], err->msg_parm.i[1],
	    err->msg_parm.i[2], err->msg_parm.i[3],
	    err->msg_parm.i[4], err->msg_parm.i[5],
	    err->msg_parm.i[6], err->msg_parm.i[7]);
#endif
}


/*
 * Reset error state variables at start of a new image.
 * This is called during compression startup to reset trace/error
 * processing to default state, without losing any application-specific
 * method pointers.  An application might possibly want to override
 * this method if it has additional error processing state.
 */

METHODDEF(void)
reset_error_mgr (j_common_ptr cinfo)
{
  cinfo->err->num_warnings = 0;
  /* trace_level is not reset since it is an application-supplied parameter */
  cinfo->err->msg_code = 0;	/* may be useful as a flag for "no error" */
}

static int init_as_jpeg( struct imgnode *imn )
{
	my_src_mgr *src;

	//kprintf( "init_as_jpeg url %s\r\n", imn->url );

	imn->cinfo = alloci( sizeof( *imn->cinfo ) );
	imn->jerr = alloci( sizeof( *imn->jerr ) );

	if( !imn->cinfo || !imn->jerr )
		return( FALSE );

	// Init error handler
	imn->jerr->je.error_exit = error_exit;
	imn->jerr->je.emit_message = emit_message;
	imn->jerr->je.output_message = output_message;
	imn->jerr->je.format_message = format_message;
	imn->jerr->je.reset_error_mgr = reset_error_mgr;

	imn->jerr->je.trace_level = 0;		/* default = no tracing */
	imn->jerr->je.num_warnings = 0;	/* no warnings emitted yet */
	imn->jerr->je.msg_code = 0;		/* may be useful as a flag for "no error" */

	/* Initialize message table pointers */
	imn->jerr->je.jpeg_message_table = jpeg_std_message_table;
	imn->jerr->je.last_jpeg_message = (int) JMSG_LASTMSGCODE - 1;

	imn->jerr->je.addon_message_table = NULL;
	imn->jerr->je.first_addon_message = 0;	/* for safety */
	imn->jerr->je.last_addon_message = 0;

	// Init source manager

	imn->cinfo->err = ( struct jpeg_error_mgr *)( imn->jerr );
	jpeg_create_decompress( imn->cinfo );

	/*
	 * We can see how GCC and SAS/C love
	 * each other :)
	 */
#ifdef __SASC
	imn->cinfo->src = src = alloci( sizeof( *src ) );
#else
	src = ( my_src_mgr * )alloci( sizeof( *src ) );
	imn->cinfo->src = ( struct jpeg_source_mgr * )src;
#endif /* !__SASC */

	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = term_source;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
	src->imgnode = imn;

	return( TRUE );
}

static void reporttoclients( struct imgnode *imn );

static void term_jpeg( struct imgnode *imn )
{
	if( imn->cinfo )
	{
		jpeg_destroy( ( j_common_ptr )imn->cinfo );
		imn->cinfo = NULL;
		if( imn->urlnode )
		{
			v_nets_release_buffer( imn->urlnode );
			v_nets_close( imn->urlnode );
			imn->urlnode = NULL;
		}
		SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
	}
}

#ifndef MBX
static void createjpegcmap( struct imgnode *imn )
{
	int c;

	if( !( features & FTF_TRUECOLOR ) )
	{
		imn->currentimf->numpens = imn->cinfo->actual_number_of_colors;
		ObtainSemaphore( &destscreensem );
		if( !destscreen )
		{
			ReleaseSemaphore( &destscreensem );
			return;
		}

		imn->cm = destscreen->ViewPort.ColorMap;
		for( c = 0; c < imn->currentimf->numpens; c++ )
		{
			if( imn->cinfo->out_color_space == JCS_GRAYSCALE )
			{
				imn->currentimf->pens[ c ] = ObtainBestPen( imn->cm,
					imn->cinfo->colormap[ 0 ][ c ] << 24,
					imn->cinfo->colormap[ 0 ][ c ] << 24,
					imn->cinfo->colormap[ 0 ][ c ] << 24,
					OBP_Precision, PRECISION_IMAGE,
					TAG_DONE
				);
			}
			else
			{
				imn->currentimf->pens[ c ] = ObtainBestPen( imn->cm,
					imn->cinfo->colormap[ 0 ][ c ] << 24,
					imn->cinfo->colormap[ 1 ][ c ] << 24,
					imn->cinfo->colormap[ 2 ][ c ] << 24,
					OBP_Precision, PRECISION_IMAGE,
					TAG_DONE
				);
			}
		}
		ReleaseSemaphore( &destscreensem );
	}
}
#endif /* !MBX */

static void procread_jpeg( struct imgnode *imn )
{
	int c;

	DBL( DEBUG_CHATTY, ( "locking...\n" ) );
	v_nets_lockdocmem();
	DBL( DEBUG_CHATTY, ( "done!\n" ) );

	if( !imn->readstate )
	{
		// still waiting for header...
		DBL( DEBUG_CHATTY, ( "before setjmp...\n" ) );
		if( setjmp( imn->jerr->jmpbuff ) )
		{
			DBL( DEBUG_CHATTY, ( "after setjmp...\n" ) );
			v_nets_unlockdocmem();
			strcpy( imn->errormsg, "JPEG decoding error (1)" );
			term_jpeg( imn );
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			reporttoclients( imn );
			return;
		}
		DBL( DEBUG_CHATTY, ( "setjmp failed\n" ) );

		if( jpeg_read_header( imn->cinfo, TRUE ) == JPEG_SUSPENDED )
		{
			DBL( DEBUG_CHATTY, ( "before v_nets_unlockdocmem(  ) now... \n" ) );
			v_nets_unlockdocmem();
			return;
		}
		DBL( DEBUG_CHATTY, ( "got headers\n" ) );
		// Got header
		imn->readstate = 1;

		// Allocate pens, bitmap and inform clients
		imn->img_x = imn->cinfo->image_width;
		imn->img_y = imn->cinfo->image_height;

#ifndef MBX
		if( !( features & FTF_TRUECOLOR ) )
		{
			imn->cinfo->quantize_colors = TRUE;

			ObtainSemaphore( &destscreensem );

			if( !destscreen )
			{
				ReleaseSemaphore( &destscreensem );
				longjmp( imn->jerr->jmpbuff, 1 );
			}


			if( imn->cinfo->jpeg_color_space == JCS_GRAYSCALE )
			{
				imn->cinfo->dither_mode = dsi_img_jpeg_dither;
				imn->cinfo->two_pass_quantize = dsi_img_jpeg_quant;
				imn->cinfo->desired_number_of_colors = 1L<<( max( destscreen->RastPort.BitMap->Depth - 1, 1 ) );
			}
			else
			{
				imn->cinfo->dither_mode = dsi_img_jpeg_dither;
				imn->cinfo->two_pass_quantize = dsi_img_jpeg_quant;
				imn->cinfo->desired_number_of_colors = 1L<<destscreen->RastPort.BitMap->Depth;
			}
			ReleaseSemaphore( &destscreensem );
		}
		else
#endif /* !MBX */
		{
			imn->cinfo->quantize_colors = FALSE;
		}

		imn->cinfo->dct_method = dsi_img_jpeg_dct;
	}
	if( imn->readstate == 1 )
	{
		DBL( DEBUG_CHATTY, ( "before setjmp ( readstate == 1 )\n" ) );
		if( setjmp( imn->jerr->jmpbuff ) )
		{
jpegfailed:
			DBL( DEBUG_CHATTY, ( "after\n" ) );
			v_nets_unlockdocmem();
			term_jpeg( imn );
			SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			reporttoclients( imn );
			return;
		}

		DBL( DEBUG_CHATTY, ( "CSID: %ld\r\n", imn->cinfo->jpeg_color_space ) );

		//if( imn->cinfo->jpeg_color_space != JCS_GRAYSCALE )
		//imn->cinfo->out_color_space = JCS_RGB;

		if( dsi_jpeg_progressive )
		{
			if( jpeg_has_multiple_scans( imn->cinfo ) )
			{
				imn->cinfo->buffered_image = TRUE;
				imn->jpeg_progressive = 2;
#ifndef MBX
				if( !( features & FTF_TRUECOLOR ) )
				{
					if( dsi_img_jpeg_quant || dsi_img_jpeg_dither == JDITHER_FS )
					{
						// Two pass quantize requested
						// use lame quantization first
						imn->cinfo->dither_mode = JDITHER_NONE;
						imn->cinfo->two_pass_quantize = FALSE;
						//imn->cinfo->desired_number_of_colors = 1L<<( max( destscreen->RastPort.BitMap->Depth - 1, 1 ) );
						imn->cinfo->dct_method = JDCT_IFAST;
						imn->jpeg_progressive = 1;
						imn->cinfo->enable_2pass_quant = TRUE;
					}
				}
#endif /* !MBX */
			}
		}

		if( !jpeg_start_decompress( imn->cinfo ) )
		{
			v_nets_unlockdocmem();
			return; 	// Suspended
		}

		v_nets_unlockdocmem();
		if( !alloc_imfnode( imn, MASK_NONE, imn->img_x, imn->img_y, 0, 0 ) )
		{
			v_nets_lockdocmem();
			goto jpegfailed;
		}
		v_nets_lockdocmem();

		if( imn->bm != BROKENBM )
		{
			// allocate colors for mapped image
#ifndef MBX
			if( !imn->jpeg_progressive )
				createjpegcmap( imn );
#endif /* !MBX */
			imn->state = GS_HASIMGINFO;
			//reporttoclients( imn );

			// tick process
			SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );

			if( imn->jpeg_progressive )
				imn->readstate = 2;
			else
				imn->readstate = 4;

			//imn->state = GS_READING2;
		}
		else
		{
			// failed
			imn->state = GS_DONE;
			reporttoclients( imn );
		}
	}
redo:
	if( imn->readstate == 2 )
	{
		int rc;

		DBL( DEBUG_CHATTY, ( "readstate == 2\n" ) );

		do {
			// Give network handler a chance to update
			// us with new data
			v_nets_unlockdocmem();
			v_nets_lockdocmem();
			rc = jpeg_consume_input( imn->cinfo );
			DBL( DEBUG_CHATTY, ( "ci = %ld\r\n", rc ) );
		} while( rc != JPEG_REACHED_EOI && rc != JPEG_SUSPENDED );

		imn->readstate = 3;

	}
	if( imn->readstate == 3 )
	{
		DBL( DEBUG_INFO, ( "input scan nummber %ld out %ld\r\n", imn->cinfo->input_scan_number, imn->cinfo->output_scan_number ) );

		if( jpeg_input_complete( imn->cinfo ) && imn->jpeg_progressive == 1 )
		{
			DBL( DEBUG_INFO, ( "switching up to high quality\r\n" ) );
			// switch decoding methods up to highest quality
			imn->jpeg_progressive = 2;

			imn->cinfo->dct_method = dsi_img_jpeg_dct;
			imn->cinfo->dither_mode = dsi_img_jpeg_dither;
			imn->cinfo->two_pass_quantize = dsi_img_jpeg_quant;
#ifdef MBX
			imn->cinfo->desired_number_of_colors = 1L<<( max( GetBitMapAttrs( destscreen->sc_RastPort->rp_BitMap, BMAV_BytesPerRow ) * 8 - 1, 1 ) );
#else
			ObtainSemaphore( &destscreensem );
			if( destscreen )
				imn->cinfo->desired_number_of_colors = 1L<<( max( destscreen->RastPort.BitMap->Depth - 1, 1 ) );
			ReleaseSemaphore( &destscreensem );
#endif /* !MBX */
			imn->cinfo->colormap = NULL;

			if( imn->cm )
			{
				for( c = 0; c < imn->currentimf->numpens; c++ )
					ReleasePen( imn->cm, imn->currentimf->pens[ c ] );
				imn->cm = NULL;
			}
		}

		// Restart
		if( !jpeg_start_output( imn->cinfo, imn->cinfo->input_scan_number ) )
		{
			DBL( DEBUG_CHATTY, ( "start_output suspended\r\n" ) );
			v_nets_unlockdocmem();
			return;
		}

		imn->has_y = 0;
		imn->min_touched_y = 0;
		imn->readstate = 4;
		DBL( DEBUG_INFO, ( "starting new JPEG pass %ld\r\n", imn->cinfo->input_scan_number ) );
#ifndef MBX
		if( !imn->cm )
			createjpegcmap( imn );
#endif /* !MBX */
	}
	if( imn->readstate == 4 )
	{
		int rc;
		int gotlines = 0;

#ifndef MBX
		struct RastPort rp;
		InitRastPort( &rp );
		rp.rp_BitMap = imn->bm;
#endif

		if( setjmp( imn->jerr->jmpbuff ) )
		{
			v_nets_unlockdocmem();
			strcpy( imn->errormsg, "JPEG decoding error (2)" );
			term_jpeg( imn );
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			reporttoclients( imn );
			return;
		}

		while( ( imn->has_y < imn->img_y ) && ( rc = jpeg_read_scanlines( imn->cinfo, jpeg_rowbuffer, JPEG_ROWBUFFER_ROWS ) ) > 0 )
		{
			int oldy = imn->has_y;

			imn->min_touched_y = min( imn->min_touched_y, imn->has_y );

			imn->has_y += rc;
			gotlines += rc;

			imn->max_touched_y = max( imn->max_touched_y, imn->has_y - 1 );

#ifndef MBX
			if( !( features & FTF_TRUECOLOR ) )
			{
				// Write ROW buffer
				for( c = 0; c < rc; c++ )
				{
					UBYTE linebuff[ MAXIMAGEWIDTH ];

					//for( d = 0; d < imn->img_x; d++ )
					//	linebuff[ d ] = imn->currentimf->pens[ jpeg_rowbuffer[ c ][ d ] ];
					penarrayconvert( jpeg_rowbuffer[ c ], linebuff, imn->currentimf->pens, imn->img_x, 0 );

					// linebuff now has the actual pen numbers to write
#if USE_CGX
					if( features & FTF_CYBERMAP )
					{
						WritePixelArray(
							linebuff, 0, 0, imn->img_x,
							&rp, 0, oldy + c,
							imn->img_x, 1,
							RECTFMT_LUT8
						);
					}
#endif /* USE_CGX */
					else
					{
						UBYTE *planes[ 8 ];
						int x;
						int add = imn->bm->BytesPerRow * ( oldy + c );

						for( x = 0; x < 8; x++ )
							planes[ x ] = imn->bm->Planes[ x ] + add;

						ObtainSemaphore( &destscreensem );
						if( destscreen )
							writechunky( linebuff, planes, imn->img_x, destscreen->BitMap.Depth );
						ReleaseSemaphore( &destscreensem );
					}
				}
			}
			else
#endif /* !MBX */
			{
/*
 * Oh fuck. I'm tired of this include mess
 */
#ifndef RECTFMT_GREY8
#define RECTFMT_GREY8	(4UL)
#endif
#ifndef RECTFMT_RGB
#define RECTFMT_RGB	(0UL)
#endif

#ifndef MBX
				int pixfmt = ( imn->cinfo->jpeg_color_space == JCS_GRAYSCALE ) ? RECTFMT_GREY8 : RECTFMT_RGB;
#endif /* !MBX */

				// Write ROW buffer
				for( c = 0; c < rc; c++ )
				{
#ifndef MBX
					WritePixelArray(
						jpeg_rowbuffer[ c ], 0, 0, imn->img_x,
						&rp, 0, oldy + c, imn->img_x, 1,
						pixfmt
					);
#else
					// FIXME! This is blatantly dead slow and ugly
					if( imn->cinfo->jpeg_color_space == JCS_GRAYSCALE )
					{
						// We must convert to RGB888 before -- gnah
						char tempbuffer[ MAXIMAGEWIDTH * 3 ], *ptb = tempbuffer;
						int x;

						for( x = 0; x < imn->img_x; x++ )
						{
							char ch = jpeg_rowbuffer[ c ][ x ];
							*ptb++ = ch;
							*ptb++ = ch;
							*ptb++ = ch;
						}
						WritePixelArrayBM(
							tempbuffer, 0, 0, imn->img_x * 3,
							imn->bm, 0, oldy + c, imn->img_x, 1,
							PIXFMT_RGB888, 0
						);
					}
					else
					{
						WritePixelArrayBM(
							jpeg_rowbuffer[ c ], 0, 0, imn->img_x * 3,
							imn->bm, 0, oldy + c, imn->img_x, 1,
							PIXFMT_RGB888, 0
						);
					}
#endif
				}
			}

			// Inform clients that we've got more data

			if( gotlines > 16 || ( dsi_img_lamedecode && gotlines ) )
			{
				reporttoclients( imn );
				gotlines = 0;
			}
		}

		if( gotlines )
			reporttoclients( imn );

		if( imn->has_y == imn->img_y )
		{
			if( !imn->jpeg_progressive )
			{
				imn->state = GS_DONE;
				reporttoclients( imn );

				term_jpeg( imn );
			}
			else
			{
				if( !jpeg_finish_output( imn->cinfo ) )
				{
					// suspended
					v_nets_unlockdocmem();
					return;
				}

				if( jpeg_input_complete( imn->cinfo ) && imn->cinfo->input_scan_number == imn->cinfo->output_scan_number && imn->jpeg_progressive == 2 )
				{
					imn->state = GS_DONE;
					reporttoclients( imn );

					term_jpeg( imn );
					DBL( DEBUG_INFO, ( "jpeg done\r\n" ) );
				}
				else
				{
					DBL( DEBUG_CHATTY, ( "finished output pass, back to scan\r\n" ) );
					imn->readstate = 2;
					// Give network handler a chance to update
					// us with new data
					v_nets_unlockdocmem();
					v_nets_lockdocmem();
					goto redo;
				}
			}
		}
	}
	DBL( DEBUG_CHATTY, ( "final v_nets_unlockdocmem(  )\n" ) );
	v_nets_unlockdocmem();
}

//
// GIF Module
//

int gif_read_data( struct imgnode *imn, APTR dest, int size )
{
	int hlen = v_nets_getdocptr( imn->urlnode ) - imn->urlpos;

	//kprintf( "gif_read_data(%s,%ld) hlen %ld\r\n", imn->url, size, hlen );

	if( hlen <= 0 )
	{
		if( v_nets_state( imn->urlnode ) )
			return( -1 );
		else
			return( 0 );
	}

	hlen = min( size, hlen );

	memcpy( dest, v_nets_getdocmem( imn->urlnode ) + imn->urlpos, hlen );

	imn->urlpos += hlen;
	return( hlen );
}

static int init_as_gif( struct imgnode *imn )
{
	DBL( DEBUG_INFO, ( "init_as_gif url %s\r\n", imn->url) );
	imn->gifh = gif_init( imn );
	return( (int)imn->gifh );
}

static void term_gif( struct imgnode *imn )
{
	if( imn->urlnode )
	{
		v_nets_release_buffer( imn->urlnode );
		v_nets_close( imn->urlnode );
		imn->urlnode = NULL;
		SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
	}
	if( imn->gifh )
	{
		gif_free( imn->gifh );
		imn->gifh = NULL;
	}
	if( imn->raw_data )
	{
		freei( imn->raw_data, imn->local_xs * imn->local_ys );
		imn->raw_data = NULL;
	}
}

static void procread_gif( struct imgnode *imn )
{
#ifndef MBX
	int c;
#endif

	v_nets_lockdocmem();

	if( !imn->readstate )
	{
		int rc;

		// still waiting for header...
		rc = gif_read_header( imn->gifh, &imn->img_x, &imn->img_y );
		if( !rc )
		{
			v_nets_unlockdocmem();
			return;
		}

		if( rc < 0 )
		{
			//kprintf( "gif_read_header(%s) %ld\r\n", imn->url, rc );
			v_nets_unlockdocmem();
			strcpy( imn->errormsg, "GIF decoding error (1)" );
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			term_gif( imn );
			reporttoclients( imn );
			return;
		}

		// Got header
		imn->readstate = 1;
	}
retrybegin:
	if( imn->readstate == 1 )
	{
		int rc;
		int local_depth, local_delay, local_disposal;

		rc = gif_begin_image( imn->gifh, &imn->local_xs, &imn->local_ys, &imn->local_xp, &imn->local_yp, &local_depth, &local_delay, &local_disposal, &imn->local_mask, &imn->repeatcnt );

		//kprintf( "gif_begin_image(%s) = %ld\r\n", imn->url, rc );

		if( rc == -2 )
		{
			// something nuked
giffailed:
			v_nets_unlockdocmem();
			strcpy( imn->errormsg, "GIF decoding error (2)" );
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			term_gif( imn );
			reporttoclients( imn );
			return;
		}
		else if( rc == -1 )
		{
			v_nets_unlockdocmem();
			reporttoclients( imn );
			imn->state = GS_DONE;
			if( !imn->bm )
			{
				//kprintf( "gif_begin_image(%s) = %ld with no bitmap\r\n", imn->url, rc );
				imn->bm = BROKENBM;
			}
			strcpy( imn->errormsg, "GIF decoding error (3)" );
			term_gif( imn );
			reporttoclients( imn );
			return;
		}
		else if( !rc )
		{
			v_nets_unlockdocmem();
			return;	// suspended
		}

		//kprintf( "bi: xs %ld ys %ld xp %ld yp %ld depth %ld delay %ld mask %ld\r\n",
		//	imn->local_xs, imn->local_ys, imn->local_xp, imn->local_yp, local_depth, local_delay, imn->local_mask
		//);

		// at this point, we can create the bitmap and allocate colors

		v_nets_unlockdocmem();
		if( !alloc_imfnode( imn, ( imn->local_mask >= 0 ) ? MASK_PLANAR : MASK_NONE, imn->local_xs, imn->local_ys, imn->local_xp, imn->local_yp ) )
		{
			v_nets_lockdocmem();
			goto giffailed;
		}
		v_nets_lockdocmem();

		imn->currentimf->delay = local_delay;
		imn->currentimf->disposal = local_disposal;

		// report to clients only for the very first frame
		// they get a GS_DONE if the whole anim has been loaded
		if( imn->imgcount == 1 )
		{
			imn->state = GS_HASIMGINFO;
			//reporttoclients( imn );
			//imn->state = GS_READING2;
		}
		else
			imn->state = GS_READING3;

		gif_read_colormap( imn->gifh, imn->colmap );

#ifndef MBX
		if( !( features & FTF_TRUECOLOR ) )
		{
			imn->currentimf->numpens = local_depth;
			ObtainSemaphore( &destscreensem );
			if( !destscreen )
			{
				ReleaseSemaphore( &destscreensem );
				goto giffailed;
			}
			imn->cm = destscreen->ViewPort.ColorMap;
			// alloc pens
			for( c = 0; c < local_depth; c++ )
			{
				imn->currentimf->pens[ c ] = ObtainBestPen( imn->cm,
					imn->colmap[ c * 3 + 0 ] << 24,
					imn->colmap[ c * 3 + 1 ] << 24,
					imn->colmap[ c * 3 + 2 ] << 24,
					OBP_Precision, PRECISION_IMAGE,
					TAG_DONE
				);
			}
			ReleaseSemaphore( &destscreensem );
			if( dsi_img_gif_dither )
				imn->raw_data = alloci( imn->local_xs * imn->local_ys );
		}
#endif /* !MBX */
		imn->readstate = 2;
		imn->has_y = -1;
		SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
	}
	else if( imn->readstate == 2 )
	{
		int rc;
		int gotlines = 0;
		int oldpass = -1;
#ifndef MBX
		UBYTE linebuff[ MAXIMAGEWIDTH ];
		struct RastPort rp;

		InitRastPort( &rp );
		rp.rp_BitMap = imn->currentimf->bm;
#endif

		for(;;)
		{
			int yline;

			rc = gif_readscanline( imn->gifh, jpeg_rowbuffer[ 0 ], &yline );

			//kprintf( "GIF: read_scanline() returned %ld, line %ld\r\n", rc, yline );

			if( !rc )
				break;	// no data
			else if( rc == -1 )
			{
				// done with this image
				if( imn->raw_data )
				{
					if( gotlines )
						reporttoclients( imn );

#ifndef MBX
					do_dither(
						imn->raw_data,
						imn->colmap,
						imn->local_xs,
						imn->local_ys,
						imn->currentimf->numpens,
						imn->cm,
						imn->currentimf->pens
					);

					// Write back the modified lines
					penarrayconvert( imn->raw_data, imn->raw_data, imn->currentimf->pens, imn->local_xs * imn->local_ys, 0 );
#endif /* !MBX */

#if USE_CGX
					if( features & FTF_CYBERMAP )
					{
						WritePixelArray(
							imn->raw_data, 0, 0, imn->local_xs,
							&rp, 0, 0,
							imn->local_xs, imn->local_ys,
							RECTFMT_LUT8
						);
					}
					else
#endif /* USE_CGX */
					{
#ifdef MBX
					  WritePixelArrayBM(
						  imn->raw_data, 0, 0, imn->local_xs * 3,
						  imn->currentimf->bm, 0, 0,
						  imn->local_xs, imn->local_ys,
						  PIXFMT_RGB888, 0
					  );
#else
						for( c = 0; c < imn->local_ys; c++ )
						{
							UBYTE *planes[ 8 ];
							int x;
							int add = imn->currentimf->bm->BytesPerRow * ( c );

							for( x = 0; x < 8; x++ )
								planes[ x ] = imn->currentimf->bm->Planes[ x ] + add;

							ObtainSemaphore( &destscreensem );
							if( destscreen )
								writechunky( &imn->raw_data[ c * imn->local_xs ], planes, imn->local_xs, destscreen->BitMap.Depth );
							ReleaseSemaphore( &destscreensem );
						}
#endif /* !MBX */
					}

					imn->min_touched_y = 0;
					imn->max_touched_y = imn->local_ys - 1;
					gotlines = TRUE;

					freei( imn->raw_data, imn->local_xs * imn->local_ys );
					imn->raw_data = NULL;
				}

				gif_endimage( imn->gifh );

				if( gotlines )
					reporttoclients( imn );

				imn->readstate = 1;
				break;
			}
			else if( rc == -2 )
			{
				v_nets_unlockdocmem();
				// something broke
				imn->state = GS_DONE;
				reporttoclients( imn );
				term_gif( imn );
				return;
			}

			if( oldpass < 0 )
				oldpass = rc;

#ifndef MBX
			if( imn->local_mask >= 0 )
			{
				int rc;

				rc = makemaskline(
					jpeg_rowbuffer[ 0 ],
					imn->currentimf->maskbm->Planes[ 0 ] + ( imn->currentimf->maskbm->BytesPerRow * yline ),
					imn->local_xs,
					imn->local_mask
				);
				if( rc & 1 )
					imn->local_non_trans = TRUE;
				if( rc & 2 )
					imn->local_mask_used = TRUE;
			}

			gotlines++;

			if( !( features & FTF_TRUECOLOR ) )
			{
				//for( d = 0; d < imn->local_xs; d++ )
				//	linebuff[ d ] = imn->currentimf->pens[ jpeg_rowbuffer[ 0 ][ d ] ];
				if( imn->raw_data )
				{
					memcpy( &imn->raw_data[ yline * imn->local_xs ], jpeg_rowbuffer[ 0 ], imn->local_xs );
				}
				penarrayconvert( jpeg_rowbuffer[ 0 ], linebuff, imn->currentimf->pens, imn->local_xs, 0 );

				// linebuff now has the actual pen numbers to write
#if USE_CGX
				if( features & FTF_CYBERMAP )
				{
					WritePixelArray(
						linebuff, 0, 0, imn->local_xs,
						&rp, 0, yline,
						imn->local_xs, 1,
						RECTFMT_LUT8
					);
				}
				else
#endif /* USE_CGX */
				{
					UBYTE *planes[ 8 ];
					int x;
					int add = imn->currentimf->bm->BytesPerRow * ( yline );

					for( x = 0; x < 8; x++ )
						planes[ x ] = imn->currentimf->bm->Planes[ x ] + add;

					ObtainSemaphore( &destscreensem );
					if( destscreen )
						writechunky( linebuff, planes, imn->local_xs, destscreen->BitMap.Depth );
					ReleaseSemaphore( &destscreensem );
				}
			}
			else
#endif /* !MBX */
			{
#ifdef MBX
				UBYTE linebuff[ MAXIMAGEWIDTH * 4 ];
				if( imn->local_mask >= 0 )
				{
					rc = penarray2argb_trans( jpeg_rowbuffer[ 0 ], linebuff, imn->colmap, imn->local_xs, imn->local_mask );
					if( rc & 1 )
						imn->local_non_trans = TRUE;
					if( rc & 2 )
						imn->local_mask_used = TRUE;
					WritePixelArrayBM(
						linebuff, 0, 0, imn->local_xs * 4,
						imn->currentimf->bm, 0, yline, imn->local_xs, 1,
						PIXFMT_RGB8888, 0
					);
				}
				else
				{
					penarray2rgb( jpeg_rowbuffer[ 0 ], linebuff, imn->colmap, imn->local_xs );
					WritePixelArrayBM(
						linebuff, 0, 0, imn->local_xs * 3,
						imn->currentimf->bm, 0, yline, imn->local_xs, 1,
						PIXFMT_RGB888, 0
					);
				}
#else
				UBYTE linebuff[ MAXIMAGEWIDTH * 3 ];
				penarray2rgb( jpeg_rowbuffer[ 0 ], linebuff, imn->colmap, imn->local_xs );
				WritePixelArray(
					linebuff, 0, 0, imn->local_xs,
					&rp, 0, yline, imn->local_xs, 1,
					RECTFMT_RGB
				);
#endif /* MBX */
			}

			// Inform clients that we've got more data

			imn->min_touched_y = min( imn->min_touched_y, yline );
			imn->max_touched_y = max( imn->max_touched_y, yline );

			if( imn->imgcount == 1 )
			{
				// for interlace passes, duplicate data
				if( rc < 4 && ( yline < imn->local_ys - 2 ) && imn->local_xs < 976 ) // first pass
				{
					copylines( imn->bm, yline, yline + 1, 1, imn->local_xs );
					// Mask?
#ifndef MBX
					if( imn->currentimf->maskbm )
						copylines( imn->currentimf->maskbm, yline, yline + 1, 1, imn->local_xs );
#endif

					imn->max_touched_y = max( imn->max_touched_y, yline + 1 );
				}
				if( rc < 3 && ( yline < imn->local_ys - 4 ) && imn->local_xs < 976 )
				{
					copylines( imn->bm, yline, yline + 2, 2, imn->local_xs );
#ifndef MBX
					if( imn->currentimf->maskbm )
						copylines( imn->currentimf->maskbm, yline, yline + 2, 2, imn->local_xs );
#endif

					imn->max_touched_y = max( imn->max_touched_y, yline + 3 );
				}
				if( rc < 2 && ( yline < imn->local_ys - 8 ) && imn->local_xs < 976 )
				{
					copylines( imn->bm, yline, yline + 4, 4, imn->local_xs );
#ifndef MBX
					if( imn->currentimf->maskbm )
						copylines( imn->currentimf->maskbm, yline, yline + 4, 4, imn->local_xs );
#endif

					imn->max_touched_y = max( imn->max_touched_y, yline + 7 );
				}
				if( rc == 4 )
				{
					// on the forth pass, no full blit is required
					imn->has_y = yline;
					imn->max_touched_y = max( imn->max_touched_y, yline );
				}
			}

			if( oldpass != rc )
			{
				oldpass = rc;
				reporttoclients( imn );
				gotlines = 0;
			}
		}

		if( gotlines )
			reporttoclients( imn );

		if( imn->readstate == 1 )
			goto retrybegin;
	}

	v_nets_unlockdocmem();
}

static void png_user_error_fn( png_structp png_ptr, png_const_charp error_msg )
{
	//kprintf( "PNGERROR %s\r\n", error_msg );
	longjmp( png_ptr->jmpbuf, 1 );
}

static void png_user_warning_fn( png_structp png_ptr, png_const_charp warning_msg )
{
	//kprintf( "PNGWARN %s\r\n", warning_msg );
}

/* This function is called (as set by png_set_progressive_fn() above)
   when enough data has been supplied so all of the header has been read.
 */


static void term_png( struct imgnode *imn );

/* Hack!!! */

png_uint_32
png_get_valid(png_structp png_ptr, png_infop info_ptr, png_uint_32 flag)
{
   if (png_ptr != NULL && info_ptr != NULL)
	  return(info_ptr->valid & flag);
   else
	  return(0);
}

static void png_info_callback( png_structp png_ptr, png_infop info_ptr )
{
	struct imgnode *imn = png_ptr->error_ptr; // hack, but who cares
	int c;

	//kprintf( "entering infocb w %ld h %ld np %ld\r\n", info_ptr->width, info_ptr->height, info_ptr->num_palette );

	/* Do any setup here, including setting any of the transformations
	   mentioned in the Reading PNG files section.  For now, you _must_
	   call either png_start_read_image() or png_read_update_info()
	   after all the transformations are set (even if you don't set
	   any).  You may start getting rows before png_process_data()
	   returns, so this is your last chance to prepare for that.
	 */

	imn->img_x = info_ptr->width;
	imn->img_y = info_ptr->height;

#ifndef MBX
	ObtainSemaphore( &destscreensem );
	if( !destscreen )
	{
		ReleaseSemaphore( &destscreensem );
		goto pngfailed;
	}
	imn->cm = destscreen->sc_ViewPort.vp_ColorMap;
	ReleaseSemaphore( &destscreensem );
#endif

	// setup PNG parameters

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	if( info_ptr->bit_depth == 16 )
	   png_set_strip_16( png_ptr );

	// convert gray to RGB (aren't we lazy? :)
	if( info_ptr->color_type == PNG_COLOR_TYPE_GRAY || info_ptr->color_type == PNG_COLOR_MASK_ALPHA )
	{
		png_set_gray_to_rgb( png_ptr );
	}

	if (info_ptr->valid & PNG_INFO_gAMA)
	   png_set_gamma(png_ptr, 2.2, info_ptr->gamma);

#ifndef MBX
	if( features & FTF_TRUECOLOR )
#endif /* !MBX */
	{
		int hasmask = FALSE;

		/* expand paletted colors into true RGB triplets */
		if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
		   png_set_expand(png_ptr);
		/* Expand RGB to RGBA */
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{
			png_set_expand(png_ptr);
			hasmask = TRUE;
		}

#if defined( MBX ) || USE_ALPHA
		// Swap RGBA to ARGB
		//png_set_invert_alpha( png_ptr );
		png_set_swap_alpha( png_ptr );
#endif

		//png_read_update_info( png_ptr, info_ptr );
		if( info_ptr->num_trans ||
			( imn->png_info_ptr->color_type & PNG_COLOR_MASK_ALPHA )
		)
		{
			hasmask = MASK_ALPHA;
		}

		if( !alloc_imfnode( imn,  hasmask, info_ptr->width, info_ptr->height, 0, 0 ) )
		{
pngfailed:
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			term_png( imn );
			reporttoclients( imn );
			return;
		}
	}
#ifndef MBX
	else
	{
		//if( !alloc_imfnode( imn, info_ptr->num_trans || ( imn->png_info_ptr->color_type & PNG_COLOR_MASK_ALPHA ), info_ptr->width, info_ptr->height, 0, 0 ) )
		if( !alloc_imfnode( imn, info_ptr->num_trans ? MASK_ALPHA : MASK_NONE, info_ptr->width, info_ptr->height, 0, 0 ) )
		{
			imn->state = GS_DONE;
			imn->bm = BROKENBM;
			term_png( imn );
			reporttoclients( imn );
			return;
		}


		/* pack multiple pixels with bit depths of 1, 2, and 4 into bytes
		   (useful only for paletted and grayscale images) */
		if (info_ptr->bit_depth < 8)
		   png_set_packing(png_ptr);

		// tricky dicky, we need to dither
		ObtainSemaphore( &destscreensem );
		if( !destscreen )
		{
			ReleaseSemaphore( &destscreensem );
			goto pngfailed;
		}
		if( info_ptr->valid & PNG_INFO_PLTE )
		{
			imn->currentimf->numpens = info_ptr->num_palette;
			// alloc pens
			for( c = 0; c < info_ptr->num_palette; c++ )
			{
				imn->currentimf->pens[ c ] = ObtainBestPen( imn->cm,
						info_ptr->palette[ c ].red << 24,
						info_ptr->palette[ c ].green << 24,
						info_ptr->palette[ c ].blue << 24,
						OBP_Precision, PRECISION_IMAGE,
						TAG_DONE
				);
			}
		}
		else
		{
			static png_color *defmap;
			int np = 1L<<destscreen->RastPort.BitMap->Depth;
			int levels;
			int r, g, b;

			if( !defmap )
				defmap = malloc( 256 * sizeof( png_color ) );

			// shit, we're FUCKED

			if( np < 8 )
			{
				if( np == 4 )
				{
					for( c = 0; c < 4; c++ )
					{
						imn->currentimf->pens[ c ] = ObtainBestPen(
							imn->cm,
							255 / ( c + 1 ),
							255 / ( c + 1 ),
							255 / ( c + 1 ),
							255 / ( c + 1 ),
							OBP_Precision,	PRECISION_IMAGE,
							TAG_DONE
						);
					}
				}
				else // 2
				{
					imn->currentimf->pens[ 0 ] = ObtainBestPen(
						imn->cm,
						0, 0, 0,
						OBP_Precision,	PRECISION_IMAGE,
						TAG_DONE
					);
					imn->currentimf->pens[ 1 ] = ObtainBestPen(
						imn->cm,
						0xff, 0xff, 0xff,
						OBP_Precision,	PRECISION_IMAGE,
						TAG_DONE
					);
				}
				imn->currentimf->numpens = np;
				info_ptr->num_palette = np;
			}
			else
			{
				int i;
				for( i = 6, levels = -1 ; i >= 2 ; i-- )
				{
					if( i * i * i <= np )
					{
						levels = i;
						break;
					}
				}
				c = 0;
				for( r = 0; r < levels; r++ )
				{
					for( g = 0; g < levels ; g++ )
					{
						for( b = 0 ; b < levels; b++ )
						{
							imn->currentimf->pens[ c++ ] = ObtainBestPen(
								imn->cm,
								( ( r * 255 ) / levels ) << 24,
								( ( g * 255 ) / levels ) << 24,
								( ( b * 255 ) / levels ) << 24,
								OBP_Precision,	PRECISION_IMAGE,
								TAG_DONE
							);
						}
					}
				}
				imn->currentimf->numpens = c;
				info_ptr->num_palette = c;
			}
			for( c = 0; c < imn->currentimf->numpens; c++ )
			{
				ULONG rgb[ 3 ];

				GetRGB32( imn->cm, imn->currentimf->pens[ c ], 1, rgb );
				defmap[ c ].red = rgb[ 0 ] >> 24;
				defmap[ c ].green = rgb[ 1 ] >> 24;
				defmap[ c ].blue = rgb[ 2 ] >> 24;
			}
			info_ptr->palette = defmap;
		}

		ReleaseSemaphore( &destscreensem );

		//kprintf( "before sd\r\n" );
		png_set_dither( png_ptr, info_ptr->palette, info_ptr->num_palette,
			info_ptr->num_palette, info_ptr->hist, info_ptr->color_type != PNG_COLOR_TYPE_PALETTE
		);
		//kprintf( "after sd\r\n" );
	}
#endif /* !MBX */

#if 0

	/* expand grayscale images to the full 8 bits */
	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY && info_ptr->bit_depth < 8)
	   png_set_expand(png_ptr);

	/* expand paletted or RGB images with transparency to full alpha channels
	 * so the data will be available as RGBA quartets */
	if (info_ptr->valid & PNG_INFO_tRNS)
	   png_set_expand(png_ptr);

	/* Set the background color to draw transparent and alpha
	   images over.  It is possible to set the red, green, and blue
	   components directly for paletted images. */

	png_color_16 my_background;

	if (info_ptr->valid & PNG_INFO_bKGD)
	   png_set_background(png_ptr, &(info_ptr->background),
	                      PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	else
	   png_set_background(png_ptr, &my_background,
	                      PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

	/* tell libpng to handle the gamma conversion for you.  We only
	   need the second call if the screen_gamma isn't the usual 2.2
	   or if it is controllable by the user.  It may also be a good
	   idea to allow the user to set the file gamma if it is unknown. */
	if (info_ptr->valid & PNG_INFO_gAMA)
	   png_set_gamma(png_ptr, screen_gamma, info_ptr->gamma);
	else
	   png_set_gamma(png_ptr, screen_gamma, 0.45);

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	if (info_ptr->bit_depth == 16)
	   png_set_strip_16(png_ptr);

	/* dither rgb files down to 8 bit palette & reduce palettes
	  to the number of colors available on your screen */
	if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
	{
	   if (info_ptr->valid & PNG_INFO_PLTE)
	      png_set_dither(png_ptr, info_ptr->palette, info_ptr->num_palette,
	                     max_screen_colors, info_ptr->histogram);
	   else
	   {
	      png_color std_color_cube[MAX_SCREEN_COLORS] =
	         {/* ... colors ... */};
	       png_set_dither(png_ptr, std_color_cube, MAX_SCREEN_COLORS,
	         MAX_SCREEN_COLORS, NULL);
	   }
	}

	/* invert monocrome files to have 0 as white and 1 as black */
	if (info_ptr->bit_depth == 1 && info_ptr->color_type == PNG_COLOR_GRAY)
	   png_set_invert(png_ptr);

	/* shift the pixels down to their true bit depth */
	if (info_ptr->valid & PNG_INFO_sBIT &&
	   info_ptr->bit_depth > info_ptr->sig_bit)
	   png_set_shift(png_ptr, &(info_ptr->sig_bit));

	/* pack multiple pixels with bit depths of 1, 2, and 4 into bytes
	   (useful only for paletted and grayscale images) */
	if (info_ptr->bit_depth < 8)
	   png_set_packing(png_ptr);

	/* flip the rgb pixels to bgr */
	if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
	   info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	   png_set_bgr(png_ptr);

	/* swap bytes of 16 bit files to least significant bit first */
	if (info_ptr->bit_depth == 16)
	   png_set_swap(png_ptr);

	/* add a filler byte to RGB files (before or after each RGB triplet) */
	if (info_ptr->bit_depth == 8 && info_ptr->color_type == PNG_COLOR_TYPE_RGB)
	   png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

#endif

	/* turn on interlace handling if you are not using png_read_image() */
	png_set_interlace_handling(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	// we need to allocate rows...
	imn->pngrows = malloc( info_ptr->height * 4 );
	for( c = 0; c < info_ptr->height; c++ )
		imn->pngrows[ c ] = malloc( info_ptr->rowbytes );

	//kprintf( "pnghasinfo\r\n" );
	imn->state = GS_HASIMGINFO;
	reporttoclients( imn );
	//kprintf( "pngpostinfo\r\n" );
}

/* This function is called when each row of image data is complete */
static void png_row_callback(png_structp png_ptr, png_bytep new_row,
	png_uint_32 row_num, int pass)
{
	struct imgnode *imn = png_ptr->error_ptr; // hack, but who cares

	if( !new_row )
		return;

	/* If the image is interlaced, and you turned on the interlace
	   handler, this function will be called for every row in every pass.
	   Some of these rows will not be changed from the previous pass.
	   When the row is not changed, the new_row variable will be NULL.
	   The rows and passes are called in order, so you don't really
	   need the row_num and pass, but I'm supplying them because it
	   may make your life easier.

	   For the non-NULL rows of interlaced images, you must call
	   png_progressive_combine_row() passing in the row and the
	   old row.  You can call this function for NULL rows (it will
	   just return) and for non-interlaced images (it just does the
	   memcpy for you) if it will make the code easier.  Thus, you
	   can just do this for all cases:
	 */

	png_progressive_combine_row( png_ptr, imn->pngrows[ row_num ], new_row );

	/* where old_row is what was displayed for previous rows.  Note
	   that the first pass (pass == 0, really) will completely cover
	   the old row, so the rows do not have to be initialized.  After
	   the first pass (and only for interlaced images), you will have
	   to pass the current row, and the function will combine the
	   old row and the new row.
	*/

#ifndef MBX
	if( features & FTF_TRUECOLOR )
#endif /* !MBX */
	{
#ifndef MBX
		struct RastPort rp;
		InitRastPort( &rp );
		rp.rp_BitMap = imn->currentimf->bm;
#endif

		if( imn->currentimf->maskbm )
		{
			int c, rc;
#if !defined( MBX )
			UBYTE linebuff[ MAXIMAGEWIDTH * 3 ]; /* TOFIX: sigh */
#endif

#ifdef MBX
			WritePixelArrayBM(
				imn->pngrows[ row_num ], 0, 0, imn->img_x * 4,
				imn->bm, 0, row_num, imn->img_x, 1,
				PIXFMT_RGB8888, 0
			);
			// Check Alpha channel for values >= 1
			if( !imn->local_mask_used )
			{
				for( c = 0; c < imn->img_x; c++ )
				{
					if( imn->pngrows[ row_num ][ c * 4 ] >= 1 )
					{
						imn->local_mask_used = TRUE;
						break;
					}
				}
				imn->local_non_trans = TRUE;
			}
#elif USE_ALPHA
			if( features & FTF_ALPHA )
			{
				WritePixelArray(
					imn->pngrows[ row_num ], 0, 0, imn->img_x * 4,
					&rp, 0, row_num, imn->img_x, 1,
					RECTFMT_ARGB
				);
				imn->local_non_trans = TRUE;
				imn->local_mask_used = TRUE;  /* XXX: sigh.. this is needed but not all of it.. sucks */
			}
			else
			{
				striprgba( imn->pngrows[ row_num ], linebuff, imn->img_x );
				WritePixelArray(
					linebuff, 0, 0, imn->img_x * 3,
					&rp, 0, row_num, imn->img_x, 1,
					RECTFMT_RGB
				);
			}
#else
			striprgba( imn->pngrows[ row_num ], linebuff, imn->img_x );
			WritePixelArray(
				linebuff, 0, 0, imn->img_x * 3,
				&rp, 0, row_num, imn->img_x, 1,
				RECTFMT_RGB
			);
#endif

#if !USE_ALPHA
			// parse alpha channel
			for( c = 0; c < imn->img_x; c++ )
				linebuff[ c ] = ( imn->pngrows[ row_num ][ c * 4 + 3 ] >= 1 ) ? 1 : 0;

			rc = makemaskline(
				linebuff,
				imn->currentimf->maskbm->Planes[ 0 ] + ( imn->currentimf->maskbm->BytesPerRow * row_num ),
				imn->img_x,
				0
			);
			if( rc & 1 )
				imn->local_non_trans = TRUE;
			if( rc & 2 )
				imn->local_mask_used = TRUE;
#endif /* !MBX */
		}
		else
		{
#ifdef MBX
			WritePixelArrayBM(
				imn->pngrows[ row_num ], 0, 0, imn->img_x * 3,
				imn->bm, 0, row_num, imn->img_x, 1,
				PIXFMT_RGB888, 0
			);
#else
			WritePixelArray(
				imn->pngrows[ row_num ], 0, 0, imn->img_x * 3,
				&rp, 0, row_num, imn->img_x, 1,
				RECTFMT_RGB
			);
#endif
		}
	}
#ifndef MBX
	else
	{
		UBYTE linebuff[ MAXIMAGEWIDTH ];

		//for( d = 0; d < imn->img_x; d++ )
		//	linebuff[ d ] = imn->currentimf->pens[ imn->pngrows[ row_num ][ d ] ];

		//kprintf( "row_num %ld, row %lx, pens %lx\r\n", row_num, imn->pngrows[ row_num ], imn->currentimf->pens );

		//kprintf( "nt %ld trns %ld\r\n", imn->png_info_ptr->num_trans, imn->png_info_ptr->trans[ 0 ] );

		if( imn->png_info_ptr->num_trans )
		{
			int d;
			int rc;

			/*kprintf( "row_num %ld rp %lx height %ld\r\n", row_num, imn->pngrows[ row_num ], imn->img_y );
			kprintf( "di %lx\r\n", png_ptr->dither_index );
			kprintf( "maskbm %lx\r\n", imn->currentimf->maskbm );*/

			memset( linebuff, 0, imn->img_x );

			for( d = 0; d < imn->png_info_ptr->num_trans; d++ )
			{
				UBYTE *in;
				int xp;

				if( imn->png_info_ptr->trans[ d ] )
					continue;

				in = imn->pngrows[ row_num ];

				for( xp = 0; xp < imn->img_x; xp++ )
				{
					if( in[ xp ] == d )
						linebuff[ xp ] = 1;
				}

			}


			rc = makemaskline(
				linebuff,
				imn->currentimf->maskbm->Planes[ 0 ] + ( imn->currentimf->maskbm->BytesPerRow * row_num ),
				imn->img_x,
				1
			);
			if( rc & 1 )
				imn->local_non_trans = TRUE;
			if( rc & 2 )
				imn->local_mask_used = TRUE;
		}

		penarrayconvert( imn->pngrows[ row_num ], linebuff, imn->currentimf->pens, imn->img_x, 0 );
#if USE_CGX
		if( features & FTF_CYBERMAP )
		{
			struct RastPort rp;

			InitRastPort( &rp );
			rp.BitMap = imn->currentimf->bm;

			WritePixelArray(
				linebuff, 0, 0, imn->img_x,
				&rp, 0, row_num,
				imn->img_x, 1,
				RECTFMT_LUT8
			);
		}
		else
#endif /* USE_CGX */
		{
			UBYTE *planes[ 8 ];
			int x;
			int add = imn->currentimf->bm->BytesPerRow * ( row_num );

			for( x = 0; x < 8; x++ )
				planes[ x ] = imn->currentimf->bm->Planes[ x ] + add;

			ObtainSemaphore( &destscreensem );
			if( destscreen )
				writechunky( linebuff, planes, imn->img_x, destscreen->BitMap.Depth );
			ReleaseSemaphore( &destscreensem );
		}
	}
#endif /* !MBX */

	imn->has_y = row_num;
	imn->readstate = 3;

	imn->min_touched_y = min( imn->min_touched_y, (int)row_num );
	imn->max_touched_y = max( imn->max_touched_y, (int)row_num );

	if( pass != imn->pngoldpass )
	{
		reporttoclients( imn );
		imn->pngoldpass = pass;
	}
}

static void png_end_callback( png_structp png_ptr, png_infop info )
{
	struct imgnode *imn = png_ptr->error_ptr; // hack, but who cares

	/* This function is called after the whole image has been read,
	   including any chunks after the image (up to and including
	   the IEND).  You will usually have the same info chunk as you
	   had in the header, although some data may have been added
	   to the comments and time fields.

	   Most people won't do much here, perhaps setting a flag that
	   marks the image as finished.
	 */
	reporttoclients( imn );
	imn->state = GS_DONE;
	reporttoclients( imn );
	imn->readstate = 4;
}


static int init_as_png( struct imgnode *imn )
{
	imn->png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
		(void *)imn, (png_error_ptr)png_user_error_fn, (png_error_ptr)png_user_warning_fn
	);
	imn->png_info_ptr = png_create_info_struct( imn->png_ptr );

	png_set_progressive_read_fn( imn->png_ptr, (void *)imn,
		png_info_callback, png_row_callback, png_end_callback
	);

	return( imn->png_ptr && imn->png_info_ptr );
}

static void term_png( struct imgnode *imn )
{
	int c;

	if( imn->pngrows )
	{
		for( c = 0; c < imn->img_y; c++ )
			free( imn->pngrows[ c ] );
		free( imn->pngrows );
		imn->pngrows = NULL;
	}
	if( imn->urlnode )
	{
		v_nets_release_buffer( imn->urlnode );
		v_nets_close( imn->urlnode );
		imn->urlnode = NULL;
		SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
	}
	if( imn->png_ptr && imn->png_info_ptr )
	{
		png_destroy_read_struct( &imn->png_ptr, &imn->png_info_ptr, (png_infopp)NULL);
	}
}

static void procread_png( struct imgnode *imn )
{
	int newdata;

	newdata = v_nets_getdocptr( imn->urlnode ) - imn->urlpos;
	if( newdata )
	{
		if( setjmp( imn->png_ptr->jmpbuf ) )
		{
			/* Free the png_ptr and info_ptr memory on error */
			strcpy( imn->errormsg, "PNG decoding error (1)" );
			imn->state = GS_DONE;
			//imn->bm = BROKENBM;
			//imn->img_x = 80;
			//imn->img_y = 20;
			term_png( imn );
			reporttoclients( imn );
			return;
		}
		png_process_data( imn->png_ptr, imn->png_info_ptr, v_nets_getdocmem( imn->urlnode ) + imn->urlpos, newdata );
		//kprintf( "after proc\r\n" );
		imn->urlpos += newdata;
	}

	switch( imn->readstate )
	{
		case 1: // got info
			break;

		case 3: // got data
			reporttoclients( imn );
			imn->readstate = 5;
			break;

		case 4: // done with image
			term_png( imn );
			break;
	}
}

static void procread_xbm( struct imgnode *imn )
{
	char *p;
	int width, height;
	int y;
	int bytesperrow;
	int ok = FALSE;
#ifndef MBX
	struct RastPort rp;
#endif
	int c = 0, d;

	// we only process xbm if we have the complete
	// data. xbm images are small by definition,
	// so this shouldn't be a problem

	if( !v_nets_state( imn->urlnode ) )
		return;

	p = v_nets_getdocmem( imn->urlnode );
	if( !p )
		return; // hm?!?

	// brute force decoding...
	p = strstr( p, "#define" );
	if( p )
	{
		p += 7;
		p = stpblk( p );
		while( *p && !isspace( *p ) )
			p++;
		width = atoi( p );
		p = strstr( p, "#define" );
		if( p )
		{
			p += 7;
			p = stpblk( p );
			while( *p && !isspace( *p ) )
				p++;
			height = atoi( p );

			// we've got the info...
			imn->img_x = width;
			imn->img_y = height;
			alloc_imfnode( imn, MASK_PLANAR, width, height, 0, 0 );
			imn->state = GS_HASIMGINFO;
#ifndef MBX
			ObtainSemaphore( &destscreensem );
			if( destscreen )
				imn->cm = destscreen->sc_ViewPort.vp_ColorMap;
			ReleaseSemaphore( &destscreensem );
#endif
			imn->currentimf->numpens = 2;
			imn->currentimf->pens[ 0 ] = ObtainBestPen( imn->cm, 0xffffffff, 0xffffffff, 0xffffffff, TAG_DONE );
			imn->currentimf->pens[ 1 ] = ObtainBestPen( imn->cm, 0, 0, 0, TAG_DONE );
			reporttoclients( imn );

			bytesperrow = ( width + 7 ) / 8;

#ifndef MBX
			InitRastPort( &rp );
			rp.rp_BitMap = imn->currentimf->bm;
#endif

			// now continue to read the data
			for( y = 0; y < height; y++ )
			{
				int rc;
				UBYTE *data = jpeg_rowbuffer[ 0 ];
				for( c = 0; c < bytesperrow; c++ )
				{
					char *val = strstr( p, "0x" );
					long v;
					if( !val )
						break; // something broke
					p = val + 2;
					stch_l( p, &v );
					for( d = 0; d < 8; d++ )
					{
						if( v & (1L<<d) )
							*data++ = imn->currentimf->pens[ 1 ];
						else
							*data++ = imn->currentimf->pens[ 0 ];
					}
				}
				// zeile fettich
#ifndef MBX
				rc = makemaskline(
					jpeg_rowbuffer[ 0 ],
					imn->currentimf->maskbm->Planes[ 0 ] + ( imn->currentimf->maskbm->BytesPerRow * y ),
					width,
					imn->currentimf->pens[ 0 ]
				);
				if( rc & 1 )
					imn->local_non_trans = TRUE;
				if( rc & 2 )
					imn->local_mask_used = TRUE;
#endif /* !MBX */
//TOFIX!! add CaOS function

#if USE_CGX
				if( features & FTF_CYBERMAP )
				{
					WritePixelArray(
						jpeg_rowbuffer[ 0 ], 0, 0, width,
						&rp, 0, y,
						width, 1,
						RECTFMT_LUT8
					);
				}
#endif /* USE_CGX */
#ifndef MBX
				else
				{
					UBYTE *planes[ 8 ];
					int x;
					int add = imn->currentimf->bm->BytesPerRow * ( y );

					for( x = 0; x < 8; x++ )
						planes[ x ] = imn->currentimf->bm->Planes[ x ] + add;

					ObtainSemaphore( &destscreensem );
					if( destscreen )
						writechunky( jpeg_rowbuffer[ 0 ], planes, width, destscreen->BitMap.Depth );
					ReleaseSemaphore( &destscreensem );
				}
#endif /* !MBX */
			}
			if( y == height && c == bytesperrow )
				ok = TRUE;
		}
	}

	if( imn->urlnode )
	{
		v_nets_release_buffer( imn->urlnode );
		v_nets_close( imn->urlnode );
		imn->urlnode = 0;
	}

	if( !ok )
	{
		// 'decoding' failed...
		strcpy( imn->errormsg, "XBM decoding error (1)" );
		imn->bm = BROKENBM;
	}
	imn->state = GS_DONE;
	reporttoclients( imn );
}

//
// General modules
//

static void reporttoclients( struct imgnode *imn )
{
	struct imgclient *client;
	int deststate = 0;

	DBL( DEBUG_CHATTY, ( "reportoclients '%s' state %ld\r\n", imn->url, imn->state ) );

	if( imn->state == GS_HASIMGINFO )
	{
		deststate = 1;
		imn->state = GS_READING2;
	}
	else if( imn->state == GS_READING2 )
		deststate = 2;
	else if( imn->state == GS_READING3 )
		deststate = 3;
	else if( imn->state == GS_DONE )
		deststate = 4;

	if( !deststate )
		return;

	if( deststate == 2 && ( imn->min_touched_y == INT_MAX ) )
		return;

	ObtainSemaphore( &imgsem );

	DBL( DEBUG_CHATTY, ( "iterating clients\r\n" ) );

	for( client = FIRSTNODE( &imn->clientlist ); NEXTNODE( client ); client = NEXTNODE( client ) )
	{
		if( !client->object )
			continue;

		DBL( DEBUG_CHATTY, ( "inform cl %lx o %lx ds %ld ps %ld\r\n", client, client->object, deststate, client->privstate ) );

		if( ( deststate >= 1 ) && ( client->privstate < 1 ) )
		{
			struct imgframenode *imf = FIRSTNODE( &imn->imagelist );
			client->privstate = 1;
			v_imgcallback_decode_hasinfo( client->object, imn->bm, imn->img_x, imn->img_y, imf->maskbm, &imn->imagelist );
		}
		if( ( deststate == 2 ) && ( client->privstate <= 2 ) )
		{
			client->privstate = 2;
			v_imgcallback_decode_gotscanline( client->object, imn->min_touched_y, imn->max_touched_y );
		}
		if( ( deststate == 4 ) && ( client->privstate < 4 ) )
		{
			client->privstate = 4;
			v_imgcallback_decode_done( client->object );
		}
	}

	DBL( DEBUG_CHATTY, ( "done clients\r\n" ) );

	ReleaseSemaphore( &imgsem );

	if( deststate == 2 )
	{
		imn->max_touched_y = INT_MIN;
		imn->min_touched_y = INT_MAX;
	}

	DBL( DEBUG_CHATTY, ( "done reporttoclients\r\n" ) );
}

static int isjfif( char *data, int size )
{
	if( size >= 2 )
		if( data[ 0 ] == 0xff && data[ 1 ] == 0xd8 )
			return( TRUE );
	return( FALSE );
}

static int setupnodebytype( struct imgnode *imn )
{
	STRPTR bf = v_nets_getdocmem( imn->urlnode );

	imn->min_touched_y = INT_MAX;
	imn->max_touched_y = INT_MIN;

	if( bf && v_nets_state( imn->urlnode ) >= 0 )
	{
		if( !memcmp( bf, "GIF8", 4 ) )
		{
			imn->imagetype = IMT_GIF;
			if( !init_as_gif( imn ) )
				goto error;
			imn->giftype = bf[ 4 ];
			return( FALSE );
		}
		else if( png_check_sig( bf, 8 ) )
		{
			imn->imagetype = IMT_PNG;
			if( !init_as_png( imn ) )
				goto error;
			return( FALSE );
		}
		else if( isjfif( bf, v_nets_getdocptr( imn->urlnode ) ) )
		{
			imn->imagetype = IMT_JPEG;
			if( !init_as_jpeg( imn ) )
				goto error;
			return( FALSE );
		}
		else if( strstr( bf, "#define" ) )
		{
			imn->imagetype = IMT_XBM;
			//init_as_xbm( imn );
			return( FALSE );
		}
		strcpy( imn->errormsg, "Unknown image type" );
	}
	else
	{
		sprintf( imn->errormsg, "Net error: %-.50s", v_nets_errorstring( imn->urlnode ) );
	}

error:

	// unknown image type
	v_nets_release_buffer( imn->urlnode );
	v_nets_close( imn->urlnode );
	imn->urlnode = 0;
	return( TRUE );
}

static void cleanupnode( struct imgnode *imn )
{
	struct imgframenode *imf;

	//kprintf( "cleanupnode %s\r\n", imn->url );

	switch( imn->imagetype )
	{
		case IMT_JPEG:
			term_jpeg( imn );
			break;

		case IMT_GIF:
			term_gif( imn );
			break;
	}

	// url-node?
	//kprintf( "freeing urlnode %lx\r\n", imn->urlnode );
	if( imn->urlnode )
	{
		v_nets_release_buffer( imn->urlnode );
		v_nets_close( imn->urlnode );
		SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
	}

	//kprintf( "freeing bitmap list\r\n" );
	while( ( imf = REMHEAD( &imn->imagelist ) ) )
	{
		// Bitmaps?
		free_bitmap( imf->bm );
#ifndef MBX
		free_bitmap( imf->maskbm );
#endif

		// Pens?
		if( imf->numpens )
		{
			int c;

			for( c = 0; c < imf->numpens; c++ )
				ReleasePen( imn->cm, imf->pens[ c ] );
		}
	}

	//kprintf( "after free list\r\n" );

#ifndef MBX
	if( imn->scr )
	{
		UnlockPubScreen( NULL, imn->scr );
	}
#endif /* !MBX */

	//kprintf( "exiting\r\n" );
}

static void processnode( struct imgnode *imn )
{
	DBL( DEBUG_CHATTY, ( "processnode '%s' state %ld\r\n", imn->url, imn->state ) );
	if( imn->aborted == 1 )
	{
		switch( imn->imagetype )
		{
			case IMT_JPEG:
				DBL( DEBUG_CHATTY, ( "before term_jpeg\r\n" ) );
				term_jpeg( imn );
				DBL( DEBUG_CHATTY, ( "after term_jpeg\r\n" ) );
				break;

			case IMT_GIF:
				DBL( DEBUG_CHATTY, ( "before term_gif\r\n" ) );
				term_gif( imn );
				DBL( DEBUG_CHATTY, ( "after term_gif\r\n" ) );
				break;

			case IMT_PNG:
				term_png( imn );
				break;
		}

		// url-node?
		DBL( DEBUG_INFO, ( "freeing urlnode %lx\r\n", imn->urlnode ) );
		if( imn->urlnode )
		{
			v_nets_release_buffer( imn->urlnode );
			v_nets_close( imn->urlnode );
			SetSignal( SIGBREAKF_CTRL_F, SIGBREAKF_CTRL_F );
			imn->urlnode = NULL;
		}

		if( !imn->bm )
			imn->bm = BROKENBM;

		strcpy( imn->errormsg, "User aborted." );

		imn->aborted = 2;

		imn->state = GS_DONE;
		reporttoclients( imn );

		return;
	}

	switch( imn->state )
	{
		/*case GS_DONE:
			reporttoclients( imn );
			break;*/

		case GS_SETTINGUP:
			DBL( DEBUG_CHATTY, ( "setting up..\n" ) );
			imn->urlnode = v_nets_open(
				imn->url,
				imn->referer,
				(APTR)-1,
				NULL,
				NULL,
				0,
				imn->reloadflag ? NOF_RELOAD: 0
			);
			DBL( DEBUG_INFO, ( "done setting up, imn->urlnode == %ld\n", imn->urlnode ) );
			if( imn->urlnode )
			{
				v_nets_settomem( imn->urlnode );
				imn->state = GS_URLWAIT;
			}
			else
			{
				break;
			}

		case GS_URLWAIT:
			DBL( DEBUG_CHATTY, ( "GS_URLWAIT state\n" ) );
			if( v_nets_redirecturl( imn->urlnode ) )
			{
				struct nstream *nnew;

				if( imn->number_of_redirects++ > 20 )
				{
					strcpy( imn->errormsg, "redirect loop" );
					imn->bm = BROKENBM;
					imn->img_x = 80;
					imn->img_y = 20;
					imn->state = GS_DONE;
					reporttoclients( imn );
					break;
				}

				nnew = v_nets_open(
					v_nets_redirecturl( imn->urlnode ),
					v_nets_url( imn->urlnode ),
					(APTR)-1,
					NULL,
					NULL,
					0,
					imn->reloadflag ? NOF_RELOAD : 0
				);
				if( !nnew )
					break;

				if( nnew )
				{
					v_nets_release_buffer( imn->urlnode );
					v_nets_close( imn->urlnode );
					v_nets_settomem( nnew );
					imn->urlnode = nnew;
				}
			}

			if( !v_nets_state( imn->urlnode ) && v_nets_getdocptr( imn->urlnode ) < 128 )
				break;

			if( setupnodebytype( imn ) )
			{
				// unknown image type
				DBL( DEBUG_WARNING, ( "unknown image type\n" ) );
				// fill in broken image and be done
				imn->bm = BROKENBM;
				imn->img_x = 80;
				imn->img_y = 20;
				imn->state = GS_DONE;
				reporttoclients( imn );
				break;
			}
			else
			{
				imn->state = GS_READING;
			}

			// drop through

		case GS_READING:
		case GS_READING2:
		case GS_READING3:
			switch( imn->imagetype )
			{
				case IMT_JPEG:
					DBL( DEBUG_CHATTY, ( "calling procread_jpeg..\n" ) );
					procread_jpeg( imn );
					break;

				case IMT_GIF:
					DBL( DEBUG_CHATTY, ( "calling procread_gif..\n" ) );
					procread_gif( imn );
					break;

				case IMT_PNG:
					DBL( DEBUG_CHATTY, ( "calling procread_png..\n" ) );
					procread_png( imn );
					break;

				case IMT_XBM:
					DBL( DEBUG_CHATTY, ( "calling procread_xbm..\n" ) );
					procread_xbm( imn );
					break;
			}
			break;
	}
	DBL( DEBUG_CHATTY, ( "leaving processnode '%s' state %ld\r\n", imn->url, imn->state ) );
}

static void processall( void )
{
	struct imgnode *imn, *next;

	DBL( DEBUG_CHATTY, ( "there\n" ) );

	ObtainSemaphore( &imgsem );
	imn = FIRSTNODE( &imglist );
	ReleaseSemaphore( &imgsem );
	for(;;)
	{
		ObtainSemaphore( &imgsem );
		next = NEXTNODE( imn );
		ReleaseSemaphore( &imgsem );

		if( !next )
			break;

		DBL( DEBUG_CHATTY, ( "before processnode\r\n" ) );
		processnode( imn );
		DBL( DEBUG_CHATTY, ( "after processnode\r\n" ) );

		ObtainSemaphore( &imgsem );
		next = NEXTNODE( imn );

		if( imn->aborted )
		{
			if( ISLISTEMPTY( &imn->clientlist ) )
			{
				REMOVE( imn );
				cleanupnode( imn );
			}
		}

		ReleaseSemaphore( &imgsem );

		imn = next;

#ifndef MBX
		if( CheckSignal( SIGBREAKF_CTRL_C ) )
		{
			// reset signal
			SetSignal( SIGBREAKF_CTRL_C, SIGBREAKF_CTRL_C );
			break;
		}
#endif /* !MBX */
	}

	DBL( DEBUG_CHATTY, ( "doing inform\r\n" ) );

	ObtainSemaphore( &imgsem );
	for( imn = FIRSTNODE( &imglist ); NEXTNODE( imn ); imn = NEXTNODE( imn ) )
		reporttoclients( imn );
	ReleaseSemaphore( &imgsem );

	DBL( DEBUG_CHATTY, ( "done\r\n" ) );
}

static void flushunusednodes( void )
{
	struct imgnode *imn, *next;

	DBL( DEBUG_CHATTY, ( "entering flushunused\r\n" ) );

	ObtainSemaphore( &imgsem );

	while( ( imn = REMHEAD( &freelist ) ) )
		cleanupnode( imn );

	for( imn = FIRSTNODE( &imglist ); ( next = NEXTNODE( imn ) ); imn = next )
	{
		if( ISLISTEMPTY( &imn->clientlist ) )
		{
			REMOVE( imn );
			cleanupnode( imn );
		}
	}

	ReleaseSemaphore( &imgsem );

	DBL( DEBUG_CHATTY, ( "exiting flushunused\r\n" ) );
}

static void imghandlerfunc( void )
{
	int Done = FALSE;
	struct imgnode *imn;
#if USE_EXECUTIVE
	APTR executivemsg;
#endif /* USE_EXECUTIVE */

#ifdef AMIGAOS
	mygetds();
#endif /* AMIGAOS */

	DBL( DEBUG_INFO, ( "imgdecoder process ready\r\n" ) );

#if USE_EXECUTIVE
	executivemsg = InitExecutive();
	if( executivemsg )
	{
		SetNice( executivemsg, 15 ); /* TOFIX! should be configurable one day */
		ExitExecutive( executivemsg );
	}
#endif /* USE_EXECUTIVE */

	while( !Done )
	{
		ULONG sigs;

		DBL( DEBUG_CHATTY, ( "now waiting...\r\n" ) );

		sigs = Wait( SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F );

		DBL( DEBUG_CHATTY, ( "WAKE UP!!\r\n" ) );

		if( sigs & SIGBREAKF_CTRL_C )
			Done = TRUE;

		processall();

		if( (sigs & SIGBREAKF_CTRL_E ) || AvailMem( 0 ) < ( 256 * 1024 ) )
		{
			flushunusednodes();
			processall();
		}
	}

	// clean up any data
	while( ( imn = REMHEAD( &imglist ) ) )
		cleanupnode( imn );

	imgproc = NULL;
	// fallthrough to exit...
}

#ifdef __MORPHOS__
static int memhandlerfunc( void );

struct IntData
{
	struct EmulLibEntry InterruptFunc;
	struct Interrupt Interrupt;
	struct ExecBase *SysBase;
} myintdata;


static int memhandlerfunc( void )
#else
#ifdef MBX
static STATUS memhandlerfunc( SystemData_p sysbase, MemHandler_p mh, MemHandlerData_p mhd )
#else
static int ASM memhandlerfunc( __reg( a0,  struct MemHandlerData *mhd ) )
#endif
#endif /* !__MORPHOS__ */
{
	int rc = MEM_DID_NOTHING;

#ifdef AMIGAOS
	putreg( REG_A4, rc );
	putreg( REG_A6, rc );

	mygetds();
#endif /* AMIGAOS */

#ifdef __MORPHOS__
	struct ExecBase *SysBase = myintdata.SysBase;
#endif /* __MORPHOS__ */

	//kprintf( "in memhandler\n" );

	// try to get hold of the list semaphore (to be save)
	if( AttemptSemaphore( &imgsem ) )
	{
		struct imgnode *imn, *next;

		//kprintf( "got semaphore\n" );
		for( imn = FIRSTNODE( &imglist ); ( next = NEXTNODE( imn ) ); imn = next )
		{
			//kprintf( "checking %s\n", imn->url );
			if( ( imn->state == GS_DONE ) && ISLISTEMPTY( &imn->clientlist ) )
			{
				struct imgframenode *imf;

				REMOVE( imn );
				ADDTAIL( &freelist, imn );

				//kprintf( "freeing\n" );

				for( imf = FIRSTNODE( &imn->imagelist ); NEXTNODE( imf ); imf = NEXTNODE( imf ) )
				{
					free_bitmap( imf->bm );
#ifndef MBX
					free_bitmap( imf->maskbm );
#endif
					imf->bm = NULL;
					imf->maskbm = NULL;
				}
			}
		}

		ReleaseSemaphore( &imgsem );
		rc = MEM_ALL_DONE;
	}

	//kprintf( "leaving memhandler, rc = %ld, free %ld/%ld\n", rc, AvailMem(0),AvailMem(MEMF_LARGEST) );

#ifndef MBX
	Signal( imgproc, SIGBREAKF_CTRL_E );
#else
	Signal( (Process_p)imgproc, SIGBREAKF_CTRL_E );
#endif
	return( rc );
}

#define IMGDECODE_MEMHANDLER_NAME "V's Image Cache Low Mem Handler"
#define IMGDECODE_MEMHANDLER_PRI 20

#ifdef MBX
static MemHandler_p imgdecode_memhandler;
#endif /* MBX */

#ifdef AMIGAOS
typedef void (*INTFUNC)();

static struct Interrupt memhandlerint = {
	0, 0,
	NT_INTERRUPT,
	IMGDECODE_MEMHANDLER_PRI,
	IMGDECODE_MEMHANDLER_NAME,
	NULL,
	(INTFUNC)memhandlerfunc
};
static int memhandleractive;

#endif /* AMIGAOS */

#ifdef __MORPHOS__
static int memhandleractive;
#endif /* __MORPHOS__ */

//
// Special bitmap handling
//

static int special_usecnt;
#ifndef MBX
static int special_setup;
#endif
static struct imgframenode specialfn[ 7 ];
static struct MinList specialfnl[ 7 ];
#ifndef MBX
static ULONG specialpens[ 24 ];
#endif

extern struct BitMap mimebrushBitMap;
extern ULONG mimebrushCMap32[];

#define SPECIAL_XS 13
#define SPECIAL_YS 17

#ifndef MBX //TOFIX!! should be replaced with about:xx references
static void setupspecials( void )
{
	int c;
	struct RastPort srp, drp, mrp;

	if( special_usecnt++ )
		return;

	special_setup = TRUE;

	for( c = 1; c < 24; c++ )
	{
		specialpens[ c ] = ObtainBestPen( destscreen->sc_ViewPort.vp_ColorMap,
			mimebrushCMap32[ c * 3 + 0 ],
			mimebrushCMap32[ c * 3 + 1 ],
			mimebrushCMap32[ c * 3 + 2 ],
			OBP_Precision, PRECISION_IMAGE,
			TAG_DONE
		);
	}

	InitRastPort( &srp );
	srp.BitMap = &mimebrushBitMap;
	InitRastPort( &drp );
	InitRastPort( &mrp );

	// we've got the pens
	for( c = 0; c < 7; c++ )
	{
		int xp, yp;

		specialfn[ c ].bm = AllocBitMap( SPECIAL_XS, SPECIAL_YS, destscreen->BitMap.Depth, BMF_CLEAR, NULL );
		specialfn[ c ].maskbm = AllocBitMap( SPECIAL_XS, SPECIAL_YS, 1, BMF_CLEAR, NULL );
		specialfn[ c ].xs = SPECIAL_XS;
		specialfn[ c ].ys = SPECIAL_YS;

		NEWLIST( &specialfnl[ c ] );
		ADDTAIL( &specialfnl[ c ], &specialfn[ c ] );

		drp.BitMap = specialfn[ c ].bm;
		mrp.BitMap = specialfn[ c ].maskbm;
		SetAPen( &mrp, 1 );

		for( xp = 0; xp < SPECIAL_XS; xp++ )
		{
			for( yp = 0; yp < SPECIAL_YS; yp++ )
			{
				int x = ReadPixel( &srp, xp + c * SPECIAL_XS, yp );
				if( x )
				{
					SetAPen( &drp, specialpens[ x ] );
					WritePixel( &drp, xp, yp );
					WritePixel( &mrp, xp, yp );
				}
			}
		}
	}
}

static void cleanupspecials( void )
{
	int c;

	if( special_usecnt || !special_setup )
		return;

	special_setup = FALSE;

	for( c = 1; c < 24; c++ )
		ReleasePen( destscreen->ViewPort.ColorMap, specialpens[ c ] );

	for( c = 0; c < 7; c++ )
	{
		FreeBitMap( specialfn[ c ].bm );
		FreeBitMap( specialfn[ c ].maskbm );
	}
}
#endif /* !MBX */

int ASM SAVEDS imgdec_setdestscreen(
	__reg( a0, struct Screen *scr ),
	__reg( d0, int bgpen ),
	__reg( d1, int framepen ),
	__reg( d2, int shadowpen ),
	__reg( d3, int shinepen )
)
{
	image_bgpen = bgpen;
	image_framepen = framepen;

	if( !imgproc )
		return( 0 );

	DBL( DEBUG_CHATTY, ( "scr = %lx, destscreen = %lx\n", scr, destscreen ) );

	if( scr != destscreen )
	{
#ifndef MBX
		struct List *pslist;
		struct PubScreenNode *psn;
#endif

		// get images flushed
		if( destscreen )
		{
			int c;

			DBL( DEBUG_CHATTY, ( "before obtainsemaphore\n" ) );
			ObtainSemaphore( &destscreensem );
			DBL( DEBUG_CHATTY, ( "after obtainsemaphore\n" ) );

#ifndef MBX
			cleanupspecials();

			Signal( imgproc, SIGBREAKF_CTRL_E );
#else
			Signal( (Process_p) imgproc, SIGBREAKF_CTRL_E );
#endif /* !MBX */
			for( c = 0; c < 10; c++ )
			{
				DBL( DEBUG_CHATTY, ( "before obtainsemaphore imgsem\n" ) );
				ObtainSemaphore( &imgsem );
				DBL( DEBUG_CHATTY, ( "after obtainsemaphore imgsem\n" ) );
				if( ISLISTEMPTY( &imglist ) )
				{
					ReleaseSemaphore( &imgsem );
					break;
				}
				/*{
					struct imgnode *imn;
					for( imn = FIRSTNODE( &imglist ); NEXTNODE( imn ); imn = NEXTNODE( imn ) )
					{
						int c = 0;
						struct imgclient *imc;

						for( imc = FIRSTNODE( &imn->clientlist ); NEXTNODE( imc ); imc = NEXTNODE( imc ) )
							c++;

						//Printf( "busy: %ld, %s\r\n", c, imn->url );
					}
				}*/

				ReleaseSemaphore( &imgsem );
				Delay( 2 );
			}
		}

//TOFIX!! this is a fucking mess
#ifndef MBX
		DBL( DEBUG_CHATTY, ( "iterating pslist\n" ) );
		pslist = ( struct List * )LockPubScreenList();
		for( psn = FIRSTNODE( pslist ); NEXTNODE( psn ); psn = NEXTNODE( psn ) )
		{
			if( psn->psn_Screen == scr )
			{
				destscreenname = psn->psn_Node.ln_Name;
				break;
			}
		}
		UnlockPubScreenList();
#endif /* !MBX */

		destscreen = scr;
#ifndef MBX
		features &= ~FTF_TRUECOLOR;
#endif /* !MBX */

		if( scr )
		{
#if USE_CGX
			features &= ~FTF_CYBERMAP;
			if( CyberGfxBase )
			{
				if( GetCyberMapAttr( scr->RastPort.BitMap, CYBRMATTR_ISCYBERGFX ) )
				{
					features |= FTF_CYBERMAP;
					if( GetCyberMapAttr( scr->RastPort.BitMap, CYBRMATTR_PIXFMT ) != PIXFMT_LUT8 )
					{
						features |= FTF_TRUECOLOR;
#if USE_ALPHA
						if( CyberGfxBase->lib_Version >= 43 )
						{
							features |= FTF_ALPHA;
						}
#endif /* USE_ALPHA */
					}
				}
			}
#endif /* USE_CGX */
			DBL( DEBUG_CHATTY, ( "release\n" ) );
			ReleaseSemaphore( &destscreensem );
		}

		// Nota bene -- the semaphore is only freed if we set the screen
		// to something != NULL -- the idea is that the semaphore remains
		// locked!
	}

#if USE_CGX
	return( features & FTF_CYBERMAP );
#else
	return( FALSE );
#endif /* USE_CGX */
}

//
// return:
// 0 = not available, started
// 1 = already getting
//
APTR ASM SAVEDS imgdec_open(
	__reg( a0, char *url ),
	__reg( a1, APTR clientobject ),
	__reg( a2, char *referer ),
	__reg( d0, int reloadflag )
)
{
	struct imgnode *imn;
	int result = FALSE;
	struct imgclient *client;
	char *p;

	//DB( ("imgdec_open(%s,%lx,%s,%ld)\r\n",url,clientobject,referer?referer:"NULL",reloadflag) );

	ObtainSemaphore( &imgsem );

	client = AllocPooled( imgpool, sizeof( *client ) );
	client->object = clientobject;

	// check if they want an internal image
	if( ( p = strstr( url, "internal-gopher-" ) ) )
	{
		p += 16;
		if( !strncmp( p, "menu", 4 ) )
			client->isspecial = 7;
		else if( !strncmp( p, "bin", 3 ) )
			client->isspecial = 1;
		else if( !strncmp( p, "text", 4 ) )
			client->isspecial = 2;
		else if( !strncmp( p, "image", 5 ) )
			client->isspecial = 3;
		else if( !strncmp( p, "audio", 5 ) )
			client->isspecial = 4;
		else if( !strncmp( p, "video", 5 ) )
			client->isspecial = 5;
		else
			client->isspecial = 6;

#ifndef MBX
		setupspecials();
#endif /* !MBX */

		ReleaseSemaphore( &imgsem );
		return( client );
	}

	DBL( DEBUG_INFO, ( "open url %s co %lx cl %lx\r\n", url, clientobject, client ) );

	/*
	 * Check if we already exist
	 */
	for( imn = FIRSTNODE( &imglist ); NEXTNODE( imn ); imn = NEXTNODE( imn ) )
	{
		if( !strcmp( url, imn->url ) )
		{

			if( !imn->do_reload && !imn->aborted )
			{
				result = TRUE;
			}

			if( imn->reloadflag )
			{
				/*
				 * new image node will fetch directly from the server
				 */
				reloadflag = TRUE;
				imn->reloadflag = FALSE;
			}

			break;
		}
	}

	DBL( DEBUG_CHATTY, ( "open %s res %ld\r\n", url, result ) );

	if( !result )
	{
		/*
		 * Reloading
		 */
		int reflen = referer ? strlen( referer ) + 1 : 0;

		imn = AllocPooled( imgpool, sizeof( *imn ) + strlen( url ) + reflen + 1 );
		strcpy( imn->errormsg, "(unspecified error)" );
		//kprintf( "creating new IMN %lx\r\n", imn );
		strcpy( imn->url, url );
		if( referer )
		{
			imn->referer = strchr( imn->url, 0 ) + 1;
			strcpy( imn->referer, referer );
		}
		imn->reloadflag = reloadflag;
		NEWLIST( &imn->clientlist );
		NEWLIST( &imn->imagelist );
		ADDTAIL( &imglist, imn );

#ifndef MBX
		if( destscreenname )
			imn->scr = ( struct Screen * )LockPubScreen( destscreenname );
#endif /* !MBX */
	}

	client->imgnode = imn;
	ADDTAIL( &imn->clientlist, client );

	ReleaseSemaphore( &imgsem );

	// notify server process
#ifndef MBX
	Signal( imgproc, ( AvailMem( MEMF_CHIP ) < ( 128 * 1024 ) ) ? ( SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F ) : SIGBREAKF_CTRL_F );
#else
	Signal( (Process_p) imgproc, ( AvailMem( MEMF_CHIP ) < ( 128 * 1024 ) ) ? ( SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F ) : SIGBREAKF_CTRL_F );
#endif

	return( client );
}

int ASM SAVEDS imgdec_dowehave( __reg( a0, char *url ) )
{
	struct imgnode *imn;
	int result = FALSE;

	if( strstr( url, "internal-gopher-" ) )
		return( TRUE );

	ObtainSemaphore( &imgsem );

	for( imn = FIRSTNODE( &imglist ); NEXTNODE( imn ); imn = NEXTNODE( imn ) )
	{
		if( !strcmp( url, imn->url ) && !imn->do_reload )
		{
			result = TRUE;
			break;
		}
	}

	ReleaseSemaphore( &imgsem );

	return( result );
}


void ASM SAVEDS imgdec_close( __reg( a0, struct imgclient *client ) )
{
	ObtainSemaphore( &imgsem );
	if( !client->isspecial )
		REMOVE( client );
	else
		special_usecnt--;
	FreePooled( imgpool, client, sizeof( *client ) );
	ReleaseSemaphore( &imgsem );
}

int ASM SAVEDS imgdec_getinfo(
	__reg( a0, struct imgclient *client ),
	__reg( a1, struct BitMap **bm ),
	__reg( a2, int *xsize ),
	__reg( a3, int *ysize )
)
{
	int result = FALSE;
	struct imgnode *imn;

	if( client->isspecial )
	{
		*bm = specialfn[ client->isspecial - 1 ].bm;
		*xsize = SPECIAL_XS;
		*ysize = SPECIAL_YS;
		return( TRUE );
	}

	imn = client->imgnode;

	if( imn && imn->bm )
	{
		*bm = imn->bm;
		*xsize = imn->img_x;
		*ysize = imn->img_y;

		result = TRUE;
	}

	return( result );
}

int ASM SAVEDS imgdec_getrepeatcnt( __reg( a0, struct imgclient *client ) )
{
	if( client->imgnode )
		return( client->imgnode->repeatcnt );
	else
		return( 0 );
}


void ASM SAVEDS imgdec_tick( void )
{
#ifndef MBX
	if( imgproc )
		Signal( imgproc, SIGBREAKF_CTRL_F );
#else
	if( imgproc )
		Signal( (Process_p) imgproc, SIGBREAKF_CTRL_F );
#endif
	//kprintf( "sending tick\r\n" );
}

void ASM SAVEDS imgdec_setclientobject(
	__reg( a0, struct imgclient *client ),
	__reg( a1, APTR object )
)
{
	ObtainSemaphore( &imgsem );
	client->object = object;
#ifndef MBX
	Signal( imgproc, SIGBREAKF_CTRL_F );
#else
	Signal( (Process_p) imgproc, SIGBREAKF_CTRL_F );
#endif
	ReleaseSemaphore( &imgsem );
}

struct MinList * ASM SAVEDS imgdec_getimagelist( __reg( a0, struct imgclient *client ) )
{
	if( client->isspecial )
		return( &specialfnl[ client->isspecial - 1 ] );
	else
		return( &client->imgnode->imagelist );
}

struct BitMap * ASM SAVEDS imgdec_getmaskbm( __reg( a0, struct imgclient *client ) )
{
	if( client->isspecial )
		return( NULL );
	else
		return( ((struct imgframenode*)FIRSTNODE( &client->imgnode->imagelist ))->maskbm  );
}

void ASM SAVEDS imgdec_flushimages( void )
{
	if( imgproc )
	{
#ifndef MBX
		Signal( imgproc, SIGBREAKF_CTRL_E );
#else
		Signal( (Process_p) imgproc, SIGBREAKF_CTRL_E );
#endif
		// crude handshaking
		Delay( 2 );
	}
}

int ASM SAVEDS imgdec_isdone( __reg( a0, struct imgclient *client ) )
{
	if( client->isspecial )
		return( TRUE );
	else
		return( client->imgnode->state == GS_DONE );
}

void ASM SAVEDS imgdec_abortload( __reg( a0, struct imgclient *client ) )
{
	Forbid();
	if( client->imgnode )
	{
		if( client->imgnode->state != GS_DONE )
		{
			//Printf( "yep, aborting\r\n" );
			client->imgnode->aborted = TRUE;
		}
	}
	Permit();
}

void ASM SAVEDS imgdec_markforreload(
	__reg( a0, struct imgclient *client )
)
{
	Forbid();
	if( client->imgnode )
	{
		client->imgnode->do_reload = TRUE;
		client->imgnode->reloadflag = TRUE;
	}
	Permit();
}

char * ASM SAVEDS imgdec_errormsg(
	__reg( a0, struct imgclient *client )
)
{
	if( client->imgnode )
		return( client->imgnode->errormsg );
	else
		return( "" );
}

int ASM SAVEDS imgdec_isblank(
	__reg( a0, struct imgclient *client )
)
{
	if( client->imgnode )
		return( !client->imgnode->local_non_trans );
	else
		return( FALSE );
}

int ASM SAVEDS imgdec_maskused(
	__reg( a0, struct imgclient *client )
)
{
	if( client->imgnode )
		return( client->imgnode->local_mask_used );
	else
		return( FALSE );
}

#ifdef AMIGAOS
/*
 * sprintf() replacement
 */
UWORD fmtfunc[] = { 0x16c0, 0x4e75 };
void __stdargs sprintf( char *to, const char *fmt, ... )
{
	RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, to );
}
#endif /* AMIGAOS */

void ASM SAVEDS imgdec_getinfostring(
	__reg( a0, struct imgclient *client ),
	__reg( a1, STRPTR buffer )
)
{
	//static STRPTR colorspace[] = { "?", "GRAY", "RGB", "YCbCr", "CMYK", "YCCK" };

	strcpy( buffer, "(unknown)" );
	Forbid();
	if( client->imgnode )
	{
		switch( client->imgnode->imagetype )
		{
			case IMT_JPEG:
				sprintf( buffer, "JPEG, %ld x %ld%s",
					(long)client->imgnode->img_x,
					(long)client->imgnode->img_y,
					//colorspace[ client->imgnode->cinfo->jpeg_color_space ],
					client->imgnode->jpeg_progressive ? ", PRG" : ""
				);
				break;

			case IMT_GIF:
				sprintf( buffer, "GIF8%lca, %ld x %ld, %ld frame%s",
					(long)client->imgnode->giftype,
					(long)client->imgnode->img_x,
					(long)client->imgnode->img_y,
					(long)client->imgnode->imgcount,
					client->imgnode->imgcount > 1 ? "s" : ""
				);
				break;

			case IMT_PNG:
				sprintf( buffer, "PNG, %ld x %ld",
					(long)client->imgnode->img_x,
					(long)client->imgnode->img_y
				);
				break;

			case IMT_XBM:
				sprintf( buffer, "XBM, %ld x %ld",
					(long)client->imgnode->img_x,
					(long)client->imgnode->img_y
				);
		}
	}
	Permit();
}


//
// ***********************
//
APTR pool = NULL;

#ifdef AMIGAOS
void *malloc( size_t size )
{
	return( ( void * )VAT_AllocVecPooled( pool, size ) );
}

void *calloc( size_t cnt, size_t size )
{
	APTR mem = ( APTR )VAT_AllocVecPooled( pool, size * cnt );
	if( mem )
		memset( mem, 0, cnt * size );
	return( mem  );
}

void free( void *ptr )
{
	if( ptr )
		VAT_FreeVecPooled( pool, ptr );
}
#endif /* AMIGAOS */
#ifdef __MORPHOS__
void *malloc(size_t size)
{
	ULONG *ptr;

	if (!size)
		return(NULL);

	if (!(ptr = AllocPooled(pool, size + 4)))
		return(NULL);

	*ptr++ = size + 4;

	return(ptr);
}


void *calloc( size_t num, size_t size)
{
	void *memblock;

	size = num * size;

	if (memblock = malloc(size))
	{
		memset(memblock, 0L, size);
		return(memblock);
	}
	else
		return(NULL);
}


void free(void *data)
{
	if (data)
	{
		ULONG *ptr = data;
		FreePooled(pool, --ptr, *ptr);
	}
}
#endif /* __MORPHOS__ */

#ifdef WITH_REALLOC
void *realloc( void *ptr, size_t size)
{
	ULONG *nptr = malloc( size ), *xptr = ptr;
	if( nptr )
	{
		memcpy( nptr, xptr, xptr[ -1 ] - 4 );
		free( ptr );
	}
	return( nptr );
}
#endif

#ifdef __SASC
void __stdargs _CXFERR( int code )
{
	_FPERR = code;
}
#endif /* __SASC */

// Lib management

void ASM SAVEDS removeclone( __reg( a0, struct BitMap *src ) );//TOFIX!! sux..

int ASM SAVEDS imgdec_libinit(
	__reg( a0, struct imgcallback *cbtptr )
)
{
	int c;
	cbt = cbtptr;
	
	DBL( DEBUG_INFO, ( "initializing image decoder.. callback table 0x%lx passed\n", cbt ) );

	NEWLIST( &imglist );
	NEWLIST( &freelist );
	InitSemaphore( &imgsem );
	InitSemaphore( &destscreensem );
	ObtainSemaphore( &destscreensem );
	
	/*
	 * Check for correct V version.
	 */

	/*
	 * Please leave that check here!
	 * add a new one *afterwards* if needed.
	 * The reason is explained in the errorreq() call.
	*/
#ifndef MBX
	if( cbt->v_major < 4 )
	{
		if( cbt->v_minor < 4 )
		{
			if( cbt->v_build < 112 )
			{
				errorreq( "V Image decoder", "You can't use that image decoder version with V.\nThe API is different. Please upgrade V (3.3.112+) or\ndowngrade the imagedecoders (18-).", "Oh well" );
				errorreq( "V Image decoder", "Additionally, the V you're using can't know that\nthe image decoder is incompatible.\nV will now hang forever, sorry.", "Argh" );
				while( 1 )
				{
					errorreq( "V Image decoder", "You'd better reboot.", "Grr" );
				}
			}
		}
	}
#endif

	/* new checks go here */

	if( ( imgpool = CreatePool( MEMF_CLEAR, 4096, 2048 ) ) )
	{
#ifdef __MORPHOS__
		/*
		 * Set PPC mode
		 */
		isppc = cbt->v_isppc;

		if( cbt->v_major < 4 )
		{
			if( cbt->v_minor < 4 )
			{
				if( cbt->v_build < 88 )
				{
					isppc = FALSE;
				}
			}
		}

		if( isppc )
		{
			/*
			 * We could use a struct copy here but:
			 *
			 * <laire> structcopies are evil
			 *
			 * and besides gcc still doesn't align
			 * them properly and there's no alignement
			 * emulation in MorphOS and I heard you
			 * don't get laid if you use them.
			 */
			mcbt.nets_open = cbt->nets_open;
			mcbt.nets_state = cbt->nets_state;
			mcbt.nets_close = cbt->nets_close;
			mcbt.nets_getdocmem = cbt->nets_getdocmem;
			mcbt.nets_getdocptr = cbt->nets_getdocptr;
			mcbt.nets_settomem = cbt->nets_settomem;
			mcbt.nets_url = cbt->nets_url;
			mcbt.nets_redirecturl = cbt->nets_redirecturl;
			mcbt.nets_lockdocmem = cbt->nets_lockdocmem;
			mcbt.nets_unlockdocmem = cbt->nets_unlockdocmem;
			mcbt.nets_release_buffer = cbt->nets_release_buffer;
			mcbt.removeclone = cbt->removeclone;
			mcbt.nets_errorstring = cbt->nets_errorstring;
			mcbt.imgcallback_decode_hasinfo = cbt->imgcallback_decode_hasinfo;
			mcbt.imgcallback_decode_gotscanline = cbt->imgcallback_decode_gotscanline;
			mcbt.imgcallback_decode_done = cbt->imgcallback_decode_done;
		}
		else
		{
			/*
			 * Normal 68k gates
			 */
			mcbt.nets_open = nets_open;
			mcbt.nets_state = nets_state;
			mcbt.nets_close = nets_close;
			mcbt.nets_getdocmem = nets_getdocmem;
			mcbt.nets_getdocptr = nets_getdocptr;
			mcbt.nets_settomem = nets_settomem;
			mcbt.nets_url = nets_url;
			mcbt.nets_redirecturl = nets_redirecturl;
			mcbt.nets_lockdocmem = nets_lockdocmem;
			mcbt.nets_unlockdocmem = nets_unlockdocmem;
			mcbt.nets_release_buffer = nets_release_buffer;
			mcbt.removeclone = removeclone;
			mcbt.nets_errorstring = nets_errorstring;
			mcbt.imgcallback_decode_hasinfo = imgcallback_decode_hasinfo;
			mcbt.imgcallback_decode_gotscanline = imgcallback_decode_gotscanline;
			mcbt.imgcallback_decode_done = imgcallback_decode_done;
		}

#endif /* __MORPHOS__ */

		// Allocate JPEG row buffer
		for( c = 0; c < JPEG_ROWBUFFER_ROWS; c++ )
		{
			jpeg_rowbuffer[ c ] = alloci( MAXIMAGEWIDTH * 3 );
			
			if( !jpeg_rowbuffer[ c ] )
			{
				goto libinit_fail;
			}
		}

		init_fs_error_limit();

		imgproc = CreateNewProcTags(
			NP_Entry, ( ULONG )imghandlerfunc,
			NP_Name, ( ULONG )"V's Image Decoder Process",
			NP_CodeType, CODETYPE_PPC,
			NP_StackSize, 64 * 1024 * 2, /* XXX: fix for 68k */
			NP_Priority, -1,
			NP_Input, NULL,
			NP_CloseInput, FALSE,
			NP_Output, NULL,
			NP_CloseOutput, FALSE,
			NP_CopyVars, FALSE,
			TAG_DONE
		);

		if( !imgproc )
		{
			goto libinit_fail;
		}

		DBL( DEBUG_INFO, ( "CreateNewProc() done, image decoder process started at 0x%lx\r\n", imgproc ) );

		/*
		 * Now let's add the memhandler
		 */
#ifdef __MORPHOS__
		myintdata.SysBase = SysBase;
		myintdata.InterruptFunc.Trap = TRAP_LIB;
		myintdata.InterruptFunc.Extension = 0;
		myintdata.InterruptFunc.Func = ( void ( * )( void ) )memhandlerfunc;
		myintdata.Interrupt.is_Node.ln_Type = NT_INTERRUPT;
		myintdata.Interrupt.is_Node.ln_Pri = IMGDECODE_MEMHANDLER_PRI;
		myintdata.Interrupt.is_Node.ln_Name = IMGDECODE_MEMHANDLER_NAME;
		myintdata.Interrupt.is_Data = &myintdata;
		myintdata.Interrupt.is_Code = ( void ( * )( void ) )&myintdata.InterruptFunc;

		AddMemHandler( &myintdata.Interrupt );
		memhandleractive = TRUE;
#endif /* __MORPHOS__ */

#ifdef AMIGAOS
		AddMemHandler( &memhandlerint );
		memhandleractive = TRUE;
#endif /* AMIGAOS */

#ifdef MBX
		imgdecode_memhandler =
			AddMemHandler( (STRPTR)IMGDECODE_MEMHANDLER_NAME, (DWORD)IMGDECODE_MEMHANDLER_PRI, (VPTR)memhandlerfunc, 0xdeadbeef );
#endif /* MBX */
	
		return( TRUE );

	}
libinit_fail: /* TOFIX: a bit sucky.. */
	return( FALSE );
}

void ASM SAVEDS imgdec_libexit( void )
{
#ifdef __MORPHOS__
	if( memhandleractive )
	{
		RemMemHandler( &myintdata.Interrupt );
		memhandleractive = FALSE;
	}
#endif /* __MORPHOS__ */

#ifdef AMIGAOS
	if( memhandleractive )
	{
		RemMemHandler( &memhandlerint );
		memhandleractive = FALSE;
	}
#endif /* AMIGAOS */

#ifdef MBX
	if( imgdecode_memhandler )
	{
		RemMemHandler( imgdecode_memhandler );
	}
#endif /* MBX */

	if( !destscreen )
		ReleaseSemaphore( &destscreensem );

	Forbid();
#ifndef MBX
	if( imgproc )
		SetTaskPri( imgproc, 0 );
#endif
	while( imgproc )
	{
#ifndef MBX
		Signal( imgproc, SIGBREAKF_CTRL_C );
#else
		Signal( (Process_p) imgproc, SIGBREAKF_CTRL_C );
#endif
		Delay( 2 );
	}
	Permit();

	if( !destscreen )
		ObtainSemaphore( &destscreensem );

	if( imgpool )
	{
		DeletePool( imgpool );
		imgpool = 0;
	}

#ifndef MBX
	cleanupspecials();
#endif /* !MBX */
}

void ASM SAVEDS imgdec_setprefs(
	__reg( d0, long img_jpeg_dct ),
	__reg( d1, long img_jpeg_dither ),
	__reg( d2, long img_jpeg_quant ),
	__reg( d3, long img_lamedecode ),
	__reg( d4, long img_progressive_jpeg ),
	__reg( d5, long img_gif_dither ),
	__reg( d6, long img_png_dither )
)
{
	dsi_img_jpeg_dct = img_jpeg_dct;
	dsi_img_jpeg_dither = img_jpeg_dither;
	dsi_img_gif_dither = img_gif_dither;
	dsi_img_png_dither = img_png_dither;
	dsi_img_jpeg_quant = img_jpeg_quant;
	dsi_img_lamedecode = img_lamedecode;
	dsi_jpeg_progressive = img_progressive_jpeg;
}

void ASM SAVEDS imgdec_setdebug( __reg( d0, int lvl ) )
{
	db_level = lvl;
}

#ifndef MBX

#ifdef _FFP
struct Library *MathBase;
#endif
#ifdef _IEEE
#ifndef __MORPHOS__  // gcc morphos workaround ?
struct Library *MathIeeeDoubBasBase;
#endif /* !__MORPHOS__ */
#endif

void lib_cleanup( void );

int	lib_init( struct ExecBase *SBase )
{
	SysBase = SBase;

	if( !( DOSBase = (struct DosLibrary*)OpenLibrary( "dos.library", 37 ) ) )
	{
		return( FALSE );
	}

	IntuitionBase = ( struct IntuitionBase * )OpenLibrary( "intuition.library", 0 );

#if USE_VAT
	if( !( VATBase = OpenLibrary( "vapor_toolkit.library", VAT_VERSION ) ) )
	{
		errorreq( "V Image decoder", "Couldn't open vapor_toolkit.library.", "Ok" );
		lib_cleanup();
		return( FALSE );
	}
#endif /* USE_VAT */

#if USE_VAT
	UtilityBase = VAT_OpenLibraryCode( VATOC_UTIL );
	GfxBase = (APTR)VAT_OpenLibraryCode( VATOC_GFX );
#else
	UtilityBase = ( struct UtilityBase * )OpenLibrary( "utility.library", 0 );
	GfxBase = ( struct GfxBase * )OpenLibrary( "graphics.library", 0 );
#endif

	if( !UtilityBase || !GfxBase )
	{
		lib_cleanup();
		return( FALSE );
	}

#ifdef _FFP
	if( !( MathBase = VAT_OpenLibraryCode( VATOC_MATHFFP ) ) )
	{
		lib_cleanup();
		return( FALSE );
	}
#endif

#ifdef _IEEE
#ifndef __MORPHOS__
	if( !( MathIeeeDoubBasBase = VAT_OpenLibraryCode( VATOC_MATHIEEEDOUBBAS ) ) )
	{
		lib_cleanup();
		return( FALSE );
	}
#endif /* !__MORPHOS__ */
#endif

#if USE_CGX
	CyberGfxBase = OpenLibrary( "cybergraphics.library", 0 );
#endif /* USE_CGX */

	return( TRUE );
}

#if defined( AMIGAOS ) && defined( __SASC )
long SAVEDS ASM __UserLibInit( __reg( a6, struct Library *libbase ) )
{
	if( lib_init(*((struct ExecBase**)4)) )
	{
		libbase->lib_Node.ln_Pri = -128;

		if( pool = VAT_CreatePool( 0, 4096, 2048 ) )
		{
			mystoreds();
			return( 0 ); /* success */
		}
	}
	return( -1 );
}
#endif /*AMIGAOS */

void lib_cleanup( void )
{
	if( pool )
	{
		DeletePool( pool );
	}

#ifdef _FFP
	CloseLibrary( MathBase );
#endif

#ifdef _IEEE
#ifndef __MORPHOS__
	CloseLibrary( MathIeeeDoubBasBase );
#endif /* !__MORPHOS__ */
#endif

#if USE_CGX
	CloseLibrary( CyberGfxBase );
#endif /* USE_CGX */
#if USE_VAT
	CloseLibrary( VATBase );
#endif /* USE_VAT */
	CloseLibrary( (APTR)DOSBase );
	CloseLibrary( (APTR)GfxBase );
	CloseLibrary( (APTR)IntuitionBase );
	CloseLibrary( (APTR)UtilityBase );
}

#if defined( AMIGAOS ) && defined( __SASC )
void SAVEDS ASM __UserLibCleanup(void)
{
	lib_cleanup();
}
#endif /* AMIGAOS */

#endif /* !MBX */
