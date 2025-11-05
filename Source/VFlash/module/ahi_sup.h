/*
 * AHI support stuffs
 *
 * $Id: ahi_sup.h,v 1.1.1.1 2002/10/07 12:52:15 zapek Exp $
 */

#include <devices/ahi.h>

/*
 * AHI sound support
 */
extern struct MsgPort 		   *AHImp;
extern struct AHIRequest 	   *AHIio;
extern struct AHIRequest 	   *AHIio2;
extern struct AHIRequest	   *AHIlink;
extern int    				   AHIDevice = -1;
extern int    				   AHIavail;

/*
 * Sound buffer size
 */
extern int    buffersize = AHIBUFFERSIZE;

/*
 * We currently use tripple buffering. Sound.cc needs a rewrite
 */
extern char ahimixbuffer1[AHIBUFFERSIZE];
extern char ahimixbuffer2[AHIBUFFERSIZE];
extern char *ahibuf1;
extern char *ahibuf2;
