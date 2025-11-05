/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_REQTOOLS_H
#define _PPCINLINE_REQTOOLS_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef REQTOOLS_BASE_NAME
#define REQTOOLS_BASE_NAME ReqToolsBase
#endif /* !REQTOOLS_BASE_NAME */

#define rtAllocRequestA(type, taglist) \
	LP2(0x1e, APTR, rtAllocRequestA, ULONG, type, d0, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtAllocRequest(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtAllocRequestA((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtChangeReqAttrA(req, taglist) \
	LP2(0x30, LONG, rtChangeReqAttrA, APTR, req, a1, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtChangeReqAttr(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtChangeReqAttrA((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtCloseWindowSafely(win) \
	LP1NR(0x96, rtCloseWindowSafely, struct Window *, win, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtEZRequestA(bodyfmt, gadfmt, reqinfo, argarray, taglist) \
	LP5(0x42, ULONG, rtEZRequestA, char *, bodyfmt, a1, char *, gadfmt, a2, struct rtReqInfo *, reqinfo, a3, APTR, argarray, a4, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtEZRequest(a0, a1, a2, a3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtEZRequestA((a0), (a1), (a2), (a3), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtFileRequestA(filereq, file, title, taglist) \
	LP4(0x36, APTR, rtFileRequestA, struct rtFileRequester *, filereq, a1, char *, file, a2, char *, title, a3, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtFileRequest(a0, a1, a2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtFileRequestA((a0), (a1), (a2), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtFontRequestA(fontreq, title, taglist) \
	LP3(0x60, ULONG, rtFontRequestA, struct rtFontRequester *, fontreq, a1, char *, title, a3, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtFontRequest(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtFontRequestA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtFreeFileList(filelist) \
	LP1NR(0x3c, rtFreeFileList, struct rtFileList *, filelist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtFreeReqBuffer(req) \
	LP1NR(0x2a, rtFreeReqBuffer, APTR, req, a1, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtFreeRequest(req) \
	LP1NR(0x24, rtFreeRequest, APTR, req, a1, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtGetLongA(longptr, title, reqinfo, taglist) \
	LP4(0x4e, ULONG, rtGetLongA, ULONG *, longptr, a1, char *, title, a2, struct rtReqInfo *, reqinfo, a3, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtGetLong(a0, a1, a2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtGetLongA((a0), (a1), (a2), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtGetStringA(buffer, maxchars, title, reqinfo, taglist) \
	LP5(0x48, ULONG, rtGetStringA, UBYTE *, buffer, a1, ULONG, maxchars, d0, char *, title, a2, struct rtReqInfo *, reqinfo, a3, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtGetString(a0, a1, a2, a3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtGetStringA((a0), (a1), (a2), (a3), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtGetVScreenSize(screen, widthptr, heightptr) \
	LP3(0x78, ULONG, rtGetVScreenSize, struct Screen *, screen, a0, ULONG *, widthptr, a1, ULONG *, heightptr, a2, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtLockWindow(win) \
	LP1(0x9c, APTR, rtLockWindow, struct Window *, win, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtPaletteRequestA(title, reqinfo, taglist) \
	LP3(0x66, LONG, rtPaletteRequestA, char *, title, a2, struct rtReqInfo *, reqinfo, a3, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtPaletteRequest(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtPaletteRequestA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtReqHandlerA(handlerinfo, sigs, taglist) \
	LP3(0x6c, ULONG, rtReqHandlerA, struct rtHandlerInfo *, handlerinfo, a1, ULONG, sigs, d0, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtReqHandler(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtReqHandlerA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtScreenModeRequestA(screenmodereq, title, taglist) \
	LP3(0x90, ULONG, rtScreenModeRequestA, struct rtScreenModeRequester *, screenmodereq, a1, char *, title, a3, struct TagItem *, taglist, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define rtScreenModeRequest(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; rtScreenModeRequestA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define rtScreenToFrontSafely(screen) \
	LP1NR(0x8a, rtScreenToFrontSafely, struct Screen *, screen, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtSetReqPosition(reqpos, newwindow, screen, window) \
	LP4NR(0x7e, rtSetReqPosition, ULONG, reqpos, d0, struct NewWindow *, newwindow, a0, struct Screen *, screen, a1, struct Window *, window, a2, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtSetWaitPointer(window) \
	LP1NR(0x72, rtSetWaitPointer, struct Window *, window, a0, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtSpread(posarray, sizearray, length, min, max, num) \
	LP6NR(0x84, rtSpread, ULONG *, posarray, a0, ULONG *, sizearray, a1, ULONG, length, d0, ULONG, min, d1, ULONG, max, d2, ULONG, num, d3, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define rtUnlockWindow(win, winlock) \
	LP2NR(0xa2, rtUnlockWindow, struct Window *, win, a0, APTR, winlock, a1, \
	, REQTOOLS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_REQTOOLS_H */
