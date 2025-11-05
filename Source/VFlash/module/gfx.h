#ifndef VFLASH_GFX_H
#define VFLASH_GFX_H
/*
 * VFlash portable gfx functions
 * -----------------------------
 *
 * $Id: gfx.h,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

extern struct FlashDisplay *fd;

int init_gfx(void);
void cleanup_gfx(void);
long flash_graphic_init(FlashHandle fh);

#endif /* !VFLASH_GFX_H */
