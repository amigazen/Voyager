/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_SCREENNOTIFY_H
#define _PPCINLINE_SCREENNOTIFY_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef SCREENNOTIFY_BASE_NAME
#define SCREENNOTIFY_BASE_NAME ScreenNotifyBase
#endif /* !SCREENNOTIFY_BASE_NAME */

#define AddCloseScreenClient(screen, port, pri) \
	LP3(0x1e, APTR, AddCloseScreenClient, struct Screen *, screen, a0, struct MsgPort *, port, a1, BYTE, pri, d0, \
	, SCREENNOTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AddPubScreenClient(port, pri) \
	LP2(0x2a, APTR, AddPubScreenClient, struct MsgPort *, port, a0, BYTE, pri, d0, \
	, SCREENNOTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AddWorkbenchClient(port, pri) \
	LP2(0x36, APTR, AddWorkbenchClient, struct MsgPort *, port, a0, BYTE, pri, d0, \
	, SCREENNOTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RemCloseScreenClient(handle) \
	LP1(0x24, BOOL, RemCloseScreenClient, APTR, handle, a0, \
	, SCREENNOTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RemPubScreenClient(handle) \
	LP1(0x30, BOOL, RemPubScreenClient, APTR, handle, a0, \
	, SCREENNOTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RemWorkbenchClient(handle) \
	LP1(0x3c, BOOL, RemWorkbenchClient, APTR, handle, a0, \
	, SCREENNOTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_SCREENNOTIFY_H */
