/*
 * VFlash Amiga specific graphic subsystem
 * ---------------------------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: gfx_amiga.c,v 1.3 2004/02/28 18:51:07 zapek Exp $
 *
 */

#include "globals.h"
#include "gfx.h"
#include "mempool.h"
#include "main.h"

#if USE_PICASSO96
#include <libraries/Picasso96.h>
#endif /* USE_PICASSO96 */

#if USE_CGX
struct Library *CyberGfxBase;
#endif /* USE_CGX */



/*
 * First graphic initialization
 */
int init_gfx(void)
{
	if (fd = (struct FlashDisplay *)myAllocPooled(mempool, sizeof(struct FlashDisplay)))
	{
#if USE_CGX
		CyberGfxBase = OpenLibrary(CYBERGFXNAME, 39);
#endif /* USE_CGX */
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Graphic cleanup
 */
void cleanup_gfx(void)
{
#if USE_CGX
	if (CyberGfxBase)
	{
		CloseLibrary(CyberGfxBase);
	}
#endif /* USE_CGX */
}


/*
 * Initializes all the Amiga gfx stuffs
 */
long flash_graphic_init(FlashHandle fh)
{
#define USE_16BPP // <- 16 bits only eh ? bah
	int bpp;
	APTR morphcanvas; /* either a struct BitMap (CGFX) or a plain canvas (P96) */

	fd->width = getv(flashobj, MUIA_Width); /* width of the target zone in pixels */
	fd->height = getv(flashobj, MUIA_Height); /* height of the target zone in pixels */;

	/*
	 * Picasso96 supports
	 */
	switch (getv(flashobj, MA_Flash_GFXMode))
	{
#if USE_PICASSO96
		case MV_Flash_P96Mode:
		{
#ifdef USE_16BPP
			set(flashobj, MA_Flash_P96Format, RGBFB_R5G6B5);
			bpp = 2;
#else
			switch(getv(flashobj, MA_Flash_P96Format))
			{
				case RGBFB_CLUT 	: DB(("CLUT mode\n"));
									  /* add CLUT mode handling here */
									  bpp = 1;
		                              break;

				case RGBFB_R8G8B8	: DB(("TrueColor RGB (8 bit each)\n"));
									  set(flashobj, MA_Flash_P96Format, RGBFB_A8R8G8B8);
				case RGBFB_A8R8G8B8 : DB(("4 Byte TrueColor ARGB\n")); /* same as above */
									  ext->redmask   = 0x00ff0000;
									  ext->greenmask = 0x0000ff00;
									  ext->bluemask  = 0x000000ff;
									  bpp = 4;
									  break;

				case RGBFB_B8G8R8	: DB(("TrueColor BGR (8 bit each)\n"));
									  set(flashobj, MA_Flash_P96Format, RGBFB_A8B8G8R8);
				case RGBFB_A8B8G8R8 : DB(("4 Byte TrueColor ABGR\n"));
									  ext->redmask   = 0x000000ff;
									  ext->greenmask = 0x0000ff00;
									  ext->bluemask  = 0x00ff0000;
									  bpp = 4;
									  break;

				case RGBFB_R8G8B8A8 : DB(("4 Byte TrueColor RGBA\n"));
									  ext->redmask   = 0xff000000;
									  ext->greenmask = 0x00ff0000;
									  ext->bluemask  = 0x0000ff00;
									  bpp = 4;
									  break;

				case RGBFB_B8G8R8A8 : DB(("4 Byte TrueColor BGRA\n"));
									  ext->redmask   = 0x0000ff00;
									  ext->greenmask = 0x00ff0000;
									  ext->bluemask  = 0xff000000;
									  bpp = 4;
									  break;

				case RGBFB_B5G6R5PC : DB(("HiColor16 (5 bit R, 6 bit G, 5 bit B) (gggrrrrrbbbbbggg)\n"));
				case RGBFB_R5G6B5PC	: DB(("HiColor16 (5 bit R, 6 bit G, 5 bit B) (gggbbbbbrrrrrggg)\n"));
				case RGBFB_R5G6B5   : DB(("HiColor16 (5 bit R, 6 bit G, 5 bit B) (rrrrrggggggbbbbb)\n"));
									  set(flashobj, MA_Flash_P96Format, RGBFB_R5G6B5);
									  ext->redmask   = 0xf800;
									  ext->greenmask = 0x7e0;
									  ext->bluemask  = 0x1f;
									  bpp = 2;
									  break;

				case RGBFB_R5G5B5PC : DB(("HiColor15 (5 bit each) (gggbbbbb0rrrrrgg)\n"));
				case RGBFB_R5G5B5   : DB(("HiColor15 (5 bit each) (0rrrrrgggggbbbbb)\n"));
				case RGBFB_B5G5R5PC : DB(("HiColor15 (5 bit each) (gggrrrrr0bbbbbbgg)\n"));
									  set(flashobj, MA_Flash_P96Format, RGBFB_R5G5B5);
									  ext->redmask   = 0x7c00;
									  ext->greenmask = 0x3e0;
									  ext->bluemask  = 0x1f;
									  bpp = 2;
									  break;
			}
#endif /* !USE_16BPP */
		}
		break;
#endif /* USE_PICASSO96 */

#if USE_CGX
		case MV_Flash_CGFXMode:
		{
			/*
			* CyberGraphX modes support
			*/
#ifdef USE_16BPP
			bpp = 2;
#else
			switch(getv(flashobj, MA_Flash_CGFXFormat))
			{
				case PIXFMT_LUT8:
					DB(("PIXFMT_LUT8\n"));
					/* we have to set data->pixfmt back to the correct format, well cyfm could
					 * have used PIXFMT_* only :)
					 */
					set(flashobj, MA_Flash_CGFXFormat, RECTFMT_LUT8);
					bpp = 1;
					break;

				case PIXFMT_RGB15PC:
					DB(("PIXFMT_RGB15PC\n"));
				case PIXFMT_RGB15:
					DB(("PIXFMT_RGB15\n"));
					ext->redmask   = 0x7c;
					ext->greenmask = 0xe003;
					ext->bluemask  = 0x1f00;
					bpp = 2;
					break;

				case PIXFMT_BGR15PC:
					DB(("PIXFMT_BGR15PC\n"));
				case PIXFMT_BGR15:
					DB(("PIXFMT_BGR15\n"));
					//set(flashobj, MA_Flash_CGFXFormat, RECTFMT_RAW);
					ext->bluemask   = 0x7c00;
					ext->greenmask = 0x3e0;
					ext->redmask  = 0x1f;
					bpp = 2;
					break;

				case PIXFMT_RGB16PC:
					DB(("PIXFMT_RGB16PC\n"));
				case PIXFMT_RGB16:
					DB(("PIXFMT_RGB16\n"));
					//set(flashobj, MA_Flash_CGFXFormat, RECTFMT_RAW);
					ext->redmask   = 0xf800;
					ext->greenmask = 0x7e0;
					ext->bluemask  = 0x1f;
					bpp = 2;
					break;

				case PIXFMT_BGR16PC:
					DB(("PIXFMT_BGR16PC\n"));
				case PIXFMT_BGR16:
					DB(("PIXFMT_BGR16\n"));
					//set(flashobj, MA_Flash_CGFXFormat, RECTFMT_RAW);
					ext->bluemask  = 0xf800;
					ext->greenmask = 0x7e0;
					ext->redmask  = 0x1f;
					bpp = 2;
					break;

				case PIXFMT_ARGB32:
					DB(("PIXFMT_ARGB32\n"));
					set(flashobj, MA_Flash_CGFXFormat, RECTFMT_ARGB);
					ext->redmask   = 0x00ff0000;
					ext->greenmask = 0x0000ff00;
					ext->bluemask  = 0x000000ff;
					bpp = 4;
					break;

				case PIXFMT_BGR24:
					DB(("PIXFMT_BGR24\n"));
				case PIXFMT_BGRA32:
					DB(("PIXFMT_BGRA32\n"));
					set(flashobj, MA_Flash_CGFXFormat, RECTFMT_ARGB);
					ext->redmask   = 0x00ff0000;
					ext->greenmask = 0x0000ff00;
					ext->bluemask  = 0x000000ff;
					bpp = 4;
					break;

				case PIXFMT_RGB24:
					DB(("PIXFMT_RGB24\n"));
				case PIXFMT_RGBA32:
					DB(("PIXFMT_RGBA32\n"));
					set(flashobj, MA_Flash_CGFXFormat, RECTFMT_RGBA);
					ext->redmask   = 0xff000000;
					ext->greenmask = 0x00ff0000;
					ext->bluemask  = 0x0000ff00;
					bpp = 4;
					break;
			}
#endif /* !USE_16BPP */
		}
		break;
#endif /* USE_CGX */

#if USE_CLUT
		case MV_Flash_ChunkyCLUTMode:
		{
			bpp = 2;
		}
		break;
#endif /* USE_CLUT */

#if USE_PLANAR
		case MV_Flash_PlanarMode:
		{
			bpp = 2;
		}
		break;
#endif /* USE_PLANAR */
	}

	fd->bpl = fd->width * bpp;
	fd->depth = bpp * 8;

	set(flashobj, MA_Flash_bpp, bpp);

	if (!(morphcanvas = (APTR)domethod(flashobj, MM_Flash_AllocCanvas)))
	{
		DB(("argh! couldn't allocate the canvas (well, get it)\n"));
		return (0);
	}

	switch (getv(flashobj, MA_Flash_GFXMode))
	{
#if USE_CGX
		case MV_Flash_CGFXMode:
		{
			/*
			 * We are using CGFX so we need the base of the bitmap
			 * as we directly render into it.
			 */
			APTR handle;
			DB(("handling BitMap-style canvas\n"));

			if (handle = (APTR)LockBitMapTags((struct BitMap *)morphcanvas, LBMI_BASEADDRESS, (ULONG)&fd->pixels, TAG_DONE))
			{
				fd->bpl = GetCyberMapAttr((struct BitMap *)morphcanvas, CYBRMATTR_XMOD);
				UnLockBitMap(handle);
			}
			else
			{
				DB(("couldn't lock bitmap\n"));
				return(0);
			}
		}
		break;
#endif /* USE_CGX */

#if USE_PICASSO96
		case MV_Flash_P96Mode:
		{
			/*
			 * P96, normal plain canvas memory array
			 */
			DB(("handling normal canvas\n"));
			fd->pixels = morphcanvas;
			set(flashobj,  MA_Flash_P96Modulo, fd->bpl);
		}
		break;
#endif /* USE_PICASSO96 */

#if USE_CLUT
		case MV_Flash_ChunkyCLUTMode:
		{
			fd->pixels = morphcanvas;
		}
#endif /* USE_CLUT */

#if USE_PLANAR
		case MV_Flash_PlanarMode:
		{
			fd->pixels = morphcanvas;
		}
#endif /* USE_PLANAR */
	}

	DB(("calling FlashGraphicInit()...\n"));

	/*
	 * Clears the canvas
	 */
	domethod(flashobj, MM_Flash_ClearDrawArea);

	return (FlashGraphicInit(fh, fd));
}
