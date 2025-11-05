/*
 * VFlash CaOS specific graphic subsystem
 * --------------------------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: gfx_caos.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#include "globals.h"
#include "gfx.h"
#include "main.h"
#include "mempool.h"
#include "support.h"


int init_gfx(void)
{
	if ((fd = (struct FlashDisplay *)myAllocPooled(mempool, sizeof(struct FlashDisplay))))
	{
		return (TRUE);
	}
	return (FALSE);
}


void cleanup_gfx(void)
{
	//...
}


long flash_graphic_init(FlashHandle fh)
{
	int bpp;
	BitMap_p morphcanvas;

	fd->width = getv(flashobj, MUIA_Width);
	fd->height = getv(flashobj, MUIA_Height);

	/*
	 * This is the (current) CaOS only
	 * supporte format (RGB16, rrrrrggggggbbbbb).
	 * Of course meanwhile we switched to 32-bits... sigh
	 */

	//ext->redmask = 0xf800;
	//ext->greenmask = 0x7e0;
	//ext->bluemask = 0x1f;

	bpp = 4;       // PIXMODE_ARGB32 // TOFIX: FIXME: Query screen mode

	fd->bpl = fd->width * bpp;
	fd->depth = bpp *8;

	set(flashobj, MA_Flash_bpp, bpp);

	if (!(morphcanvas = (APTR)DoMethod(flashobj, MM_Flash_AllocCanvas)))
	{
		return (0);
	}

	/* a row is a line... I should write that on the wall */
	fd->bpl = bpp * fd->width;
	fd->pixels = morphcanvas->bm_PixMap;

	/*
	 * Clears the canvas
	 */
	DoMethod(flashobj, MM_Flash_ClearDrawArea);

	return (FlashGraphicInit(fh, fd));
}
