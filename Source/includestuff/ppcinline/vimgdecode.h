/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_VIMGDECODE_H
#define _PPCINLINE_VIMGDECODE_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef VIMGDECODE_BASE_NAME
#define VIMGDECODE_BASE_NAME VIDBase
#endif /* !VIMGDECODE_BASE_NAME */

#define imgdec_abortload(ic) \
	LP1NR(0x24, imgdec_abortload, struct imgclient *, ic, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_close(ic) \
	LP1NR(0x54, imgdec_close, struct imgclient *, ic, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_dowehave(url) \
	LP1(0x5a, int, imgdec_dowehave, char *, url, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_errormsg(client) \
	LP1(0x8a, char *, imgdec_errormsg, struct imgclient *, client, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_flushimages() \
	LP0NR(0x30, imgdec_flushimages, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_getimagelist(ic) \
	LP1(0x3c, struct MinList *, imgdec_getimagelist, struct imgclient *, ic, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_getinfo(ic, bmp, xp, yp) \
	LP4(0x4e, int, imgdec_getinfo, struct imgclient *, ic, a0, struct BitMap **, bmp, a1, int *, xp, a2, int *, yp, a3, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_getinfostring(client, buffer) \
	LP2NR(0x84, imgdec_getinfostring, struct imgclient *, client, a0, STRPTR, buffer, a1, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_getmaskbm(ic) \
	LP1(0x36, struct BitMap *, imgdec_getmaskbm, struct imgclient *, ic, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_getrepeatcnt(client) \
	LP1(0x7e, int, imgdec_getrepeatcnt, struct imgclient *, client, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_isblank(client) \
	LP1(0x96, int, imgdec_isblank, struct imgclient *, client, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_isdone(ic) \
	LP1(0x2a, int, imgdec_isdone, struct imgclient *, ic, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_libexit() \
	LP0NR(0x72, imgdec_libexit, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_libinit(cbtable) \
	LP1(0x6c, int, imgdec_libinit, APTR, cbtable, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_markforreload(ic) \
	LP1NR(0x1e, imgdec_markforreload, struct imgclient *, ic, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_maskused(client) \
	LP1(0x9c, int, imgdec_maskused, struct imgclient *, client, a0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_open(url, client, ref, reload) \
	LP4(0x60, APTR, imgdec_open, char *, url, a0, APTR, client, a1, char *, ref, a2, int, reload, d0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_setclientobject(ic, obj) \
	LP2NR(0x42, imgdec_setclientobject, struct imgclient *, ic, a0, APTR, obj, a1, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_setdebug(level) \
	LP1NR(0x90, imgdec_setdebug, int, level, d0, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_setdestscreen(scr, bgpen, framepen, shadowpen, shinepen) \
	LP5(0x66, int, imgdec_setdestscreen, struct Screen *, scr, a0, int, bgpen, d0, int, framepen, d1, int, shadowpen, d2, int, shinepen, d3, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_setprefs(dct, dither, quant, dec, progjpeg, dithergif, ditherpng) \
	LP7NR(0x78, imgdec_setprefs, int, dct, d0, int, dither, d1, int, quant, d2, int, dec, d3, int, progjpeg, d4, int, dithergif, d5, int, ditherpng, d6, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define imgdec_tick() \
	LP0NR(0x48, imgdec_tick, \
	, VIMGDECODE_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_VIMGDECODE_H */
