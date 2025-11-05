#ifndef VIMGDECODE_PRAGMAS_H
#define VIMGDECODE_PRAGMAS_H
/*
 * $Id: vimgdecode_pragmas.h,v 1.2 2001/03/25 21:02:21 owagner Exp $
 */

#pragma libcall VIDBase imgdec_markforreload 1e 801
#pragma libcall VIDBase imgdec_abortload 24 801
#pragma libcall VIDBase imgdec_isdone 2a 801
#pragma libcall VIDBase imgdec_flushimages 30 0
#pragma libcall VIDBase imgdec_getmaskbm 36 801
#pragma libcall VIDBase imgdec_getimagelist 3c 801
#pragma libcall VIDBase imgdec_setclientobject 42 9802
#pragma libcall VIDBase imgdec_tick 48 0
#pragma libcall VIDBase imgdec_getinfo 4e BA9804
#pragma libcall VIDBase imgdec_close 54 801
#pragma libcall VIDBase imgdec_dowehave 5a 801
#pragma libcall VIDBase imgdec_open 60 0A9804
#pragma libcall VIDBase imgdec_setdestscreen 66 3210805
#pragma libcall VIDBase imgdec_libinit 6c 801
#pragma libcall VIDBase imgdec_libexit 72 0
#pragma libcall VIDBase imgdec_setprefs 78 654321007
#pragma libcall VIDBase imgdec_getrepeatcnt 7e 801

/* V10 */
#pragma libcall VIDBase imgdec_getinfostring 84 9802

/* V13 */
#pragma libcall VIDBase imgdec_errormsg 8a 801

/* V15 */
#pragma libcall VIDBase imgdec_setdebug 90 001

/* V16 */
#pragma libcall VIDBase imgdec_isblank 96 801

/* V18*/
#pragma libcall VIDBase imgdec_maskused 9c 801

#endif /* !VIMGDECODE_PRAGMAS_H */
