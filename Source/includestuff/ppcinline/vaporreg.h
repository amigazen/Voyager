/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_VAPORREG_H
#define _PPCINLINE_VAPORREG_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef VAPORREG_BASE_NAME
#define VAPORREG_BASE_NAME VaporRegBase
#endif /* !VAPORREG_BASE_NAME */

#define VREG_ShowReg(screen) \
	LP1(0x1e, int, VREG_ShowReg, struct Screen *, screen, a0, \
	, VAPORREG_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VREG_ShowRegProduct(screen, product) \
	LP2(0x24, int, VREG_ShowRegProduct, struct Screen *, screen, a0, STRPTR, product, a1, \
	, VAPORREG_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_VAPORREG_H */
