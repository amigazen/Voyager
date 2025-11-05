#ifndef VFLASH_MAIN_H
#define VFLASH_MAIN_H
/*
 * VFlash portable main functions
 * ------------------------------
 *
 * $Id: main.h,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#if !USE_REENTRANCY
extern struct FlashInfo 	   *fi;
extern APTR	      			   flashobj;
extern int					   signalexit;
extern struct FlashPrefs	   *vfp;
extern struct Process		   *pr;
#endif /* !USE_REENTRANCY */

#endif /* !VFLASH_MAIN_H */
