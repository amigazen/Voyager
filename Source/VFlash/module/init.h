#ifndef VFLASH_INIT_H
#define VFLASH_INIT_H
/*
 * VFlash common init functions
 * ----------------------------
 *
 * $Id: init.h,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#ifdef __STDC__
struct FlashInfo;
#endif

int init_libs(void);
void remove_dosreq(void);
void enable_dosreq(void);
void close_libs(void);
struct FlashInfo * initstuff(void);
void closestuff(void);

#endif /* !VFLASH_INIT_H */
