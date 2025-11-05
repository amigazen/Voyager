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
** $Id: mybrush.c,v 1.18 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/iffparse.h>
#endif

/* private */
#include "mybrush.h"
#include "malloc.h"
#include "mui_func.h"
#include "dos_func.h"

struct myin {
	struct MinNode n;
	char name[ 256 ];
	struct MyBrush *b;
};

static struct MinList inlist;

static struct BitMap *allocbitmap( ULONG width, ULONG height, ULONG depth )
{
	struct BitMap *bm;

#ifdef __MORPHOS__
	bm = calloc( sizeof( *bm ), sizeof( BYTE ) );
#else
	bm = malloc( sizeof( *bm ) );
	memset( bm, '\0', sizeof( *bm ) );
#endif /* !__MORPHOS__ */

	if( bm )
	{
		int c;
		InitBitMap( bm, depth, width, height );
		for( c = 0; c < depth; c++ )
		{
#ifdef __MORPHOS__
			bm->Planes[ c ] = calloc( RASSIZE( width, height ), sizeof( BYTE ) );
#else
			bm->Planes[ c ] = AllocVec( RASSIZE( width, height ), MEMF_CLEAR | MEMF_CHIP );
#endif /* !__MORPHOS__ */
		}
	}
	return( bm );
}

void freebitmap( struct BitMap *bm )
{
	WaitBlit();
	if( bm )
	{
		int c;
		for( c = 0; c < bm->Depth; c++ )
		{
#ifdef __MORPHOS__
			free( bm->Planes[ c ] );
#else
			FreeVec( bm->Planes[ c ] );
#endif
		}
		free( bm );
	}
}

static ULONG shiftcol( ULONG col )
{
	return(
		( col << 24 ) |
		( col << 16 ) |
		( col << 8 ) |
		col
	);
}

/* IFF-Strukturen */
#ifdef __GNUC__
#pragma pack(2)
#endif /* __GNUC__ */
struct myBitMapHeader {
	UWORD w,h;
	WORD x,y;
	UBYTE nPlanes;
	UBYTE masking;
	UBYTE compression;
	UBYTE reserved1;
	UWORD transparentColor;
	UBYTE xAspect,yAspect;
	WORD pageWidth,pageHeight;
};
#ifdef __GNUC__
#pragma pack()
#endif /* __GNUC__ */

#define cmpNone 0
#define cmpByteRun1 1

struct BitMap *readbitmap( char *filename, ULONG *colmap32, int *xsize, int *ysize, int *depth )
{
	struct IFFHandle *iffh;
	BPTR file;
	struct BitMap *bm = NULL;

	if( file = Open( filename, MODE_OLDFILE ) ) 
	{
		if( iffh = AllocIFF() )
		{
			iffh->iff_Stream = (ULONG)file;
			InitIFFasDOS( iffh );
			if( !OpenIFF( iffh, IFFF_READ ) )
			{
				PropChunk( iffh, MAKE_ID('I','L','B','M' ), MAKE_ID( 'B','M','H','D' ) );
				PropChunk( iffh, MAKE_ID('I','L','B','M' ), MAKE_ID( 'C','M','A','P' ) );
				PropChunk( iffh, MAKE_ID('I','L','B','M' ), MAKE_ID( 'B','O','D','Y' ) );
				StopOnExit( iffh, MAKE_ID('I','L','B','M' ), MAKE_ID( 'F','O','R','M' ) );
				if( ParseIFF( iffh, IFFPARSE_SCAN ) == IFFERR_EOC )
				{
					struct StoredProperty *sp;
					struct myBitMapHeader *bmh;

					sp = FindProp( iffh, MAKE_ID( 'I', 'L', 'B', 'M' ), MAKE_ID( 'B', 'M', 'H', 'D' ) );
					if( sp )
					{
						bmh = ( struct myBitMapHeader * )sp->sp_Data;
						*depth = bmh->nPlanes;
						*xsize = bmh->w;
						*ysize = bmh->h;
						bm = allocbitmap( bmh->w, bmh->h, bmh->nPlanes );
						if( bm )
						{
							int c;
							int rasterwidth, rasterheight;

							sp = FindProp( iffh, MAKE_ID( 'I', 'L', 'B', 'M' ), MAKE_ID( 'C', 'M', 'A', 'P' ) );
							if( sp )
							{
								UBYTE *bodydata = (UBYTE*)sp->sp_Data;
						
								for( c = 0; c < (1L<<*depth ); c++ )
								{
									*colmap32++= shiftcol( bodydata[0] );
									*colmap32++= shiftcol( bodydata[1] );
									*colmap32++= shiftcol( bodydata[2] );
									bodydata += 3;
								}
							}

							rasterwidth = ( ( bmh->w + 15 ) / 16 ) * 2;
							rasterheight = bmh->h;

							sp = FindProp( iffh, MAKE_ID( 'I', 'L', 'B', 'M' ), MAKE_ID( 'B', 'O', 'D', 'Y' ) );
							if( sp )
							{
								UBYTE *bodydata = (UBYTE*)sp->sp_Data, *planedata;
								int d, e, f;

								for( c = 0; c < rasterheight; c++ )
								{
									for( d = 0; d < bmh->nPlanes; d++ )
									{
										planedata = ((UBYTE*)bm->Planes[ d ]) + ( c * bm->BytesPerRow );
										if( bmh->compression == cmpByteRun1 )
										{ /* Datas are compressed */
											for( e = 0; e < rasterwidth; )
											{
												signed char decodechar = *bodydata++;
												if( decodechar >=0 )
												{
													for( f=0; f < decodechar + 1; f++ )
														*planedata++ = *bodydata++;
													e += decodechar + 1; 
												}
												else if( decodechar != -128 )
												{
													for( f = 0; f < ( -decodechar ) + 1; f++ )
														*planedata++ = *bodydata; 
													e += ( -decodechar )+1;
													bodydata++; 
												}
											}
										}
										else 
										{
											for( e = 0; e < rasterwidth; e++ )
												*planedata++ = *bodydata++;
										}
									}
								}
							}
						}
					}
				}
				CloseIFF( iffh );
			}
			FreeIFF( iffh );
		}
		Close( file );
	}
	return( bm );
}

/// LoadBrush
struct MyBrush *LoadBrush( STRPTR File )
{
	struct MyBrush *Brush = NULL;
	ULONG cmap32[ 256 * 3 ];
	int xsize, ysize, depth;
	struct BitMap *bm;
	
	if( bm = readbitmap( File, cmap32, &xsize, &ysize, &depth ) )
	{
#ifdef __MORPHOS__
		if( ( Brush = calloc( sizeof( struct MyBrush ), sizeof( BYTE ) ) ) )
		{
#else
		if( Brush = malloc( sizeof( struct MyBrush ) ) )
		{
#endif /* !__MORPHOS__ */
			int err = TRUE;
			int i;

#ifndef __MORPHOS__
			memset( Brush, '\0', sizeof( struct MyBrush ) );
#endif /* !__MORPHOS__ */

			Brush->Width  = xsize;
			Brush->Height = ysize;
			Brush->BitMap = bm;

			i = ( 1L << depth ) * 3;

			if( ( Brush->Colors = malloc( i * sizeof( ULONG ) ) ) )
			{
				memcpy( Brush->Colors, cmap32, i * 4 );
				err = FALSE;
			}

			if( err ) 
			{
				freebitmap( bm );
				free( Brush );
				Brush = NULL;
			}
		}
	}

	return( Brush );
}
///
/// FreeBrush
void FreeBrush( struct MyBrush *Brush )
{
	if( Brush )
	{

		freebitmap( Brush->BitMap );

		free( Brush->Colors );

		free( Brush );
	}
}
///

struct MyBrush *CreateEmptyBrush( int width, int height )
{
	struct MyBrush *Brush;
	
#ifdef __MORPHOS__
	if( ( Brush = calloc( sizeof( struct MyBrush ), sizeof( BYTE ) ) ) )
	{
#else
	if( ( Brush = malloc( sizeof( struct MyBrush ) ) ) )
	{
		memset( Brush, '\0', sizeof( BYTE ) );
#endif /* !__MORPHOS__ */
		Brush->Width  = width;
		Brush->Height = height;

		Brush->BitMap = allocbitmap( Brush->Width, Brush->Height, 1 );
		Brush->Colors = malloc( 2 * 3 * 4 );
		memset( Brush->Colors, '\0', 2 * 3 * 4 );
	}

	return( Brush );
}


void init_inlist( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &inlist );
}


void cleanup_inlist( void )
{
	struct myin *in;

	D( db_init, bug( "cleaning up..\n" ) );

	while( in = (APTR)REMHEAD( &inlist ) )
	{
#ifndef MBX
		FreeBrush( in->b );
#endif //TOFIX!!
		free( in );
	}
}

struct MyBrush *b_load( char *name )
{
	struct myin *in;

	for( in = FIRSTNODE( &inlist ); NEXTNODE( in ); in = NEXTNODE( in ) )
	{
		if( !stricmp( in->name, name ))
			return( in->b );
	}

	in = malloc( sizeof( *in ) );
	strcpy( in->name, name );
#ifndef MBX
	in->b = LoadBrush( name );
#endif //TOFIX!!

	ADDTAIL( &inlist, in );

	return( in->b );
}
