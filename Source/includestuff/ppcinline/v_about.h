/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_V_ABOUT_H
#define _PPCINLINE_V_ABOUT_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef V_ABOUT_BASE_NAME
#define V_ABOUT_BASE_NAME VAboutBase
#endif /* !V_ABOUT_BASE_NAME */

#define VABOUT_GetAboutIbeta() \
	LP0(0x36, STRPTR, VABOUT_GetAboutIbeta, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VABOUT_GetAboutPtr(revid, owner, imgdeclib) \
	LP3(0x1e, STRPTR, VABOUT_GetAboutPtr, STRPTR, revid, a0, STRPTR, owner, a1, STRPTR, imgdeclib, a2, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VABOUT_GetFlashLogo(sizeptr) \
	LP1(0x42, APTR, VABOUT_GetFlashLogo, int *, sizeptr, a0, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VABOUT_GetPNGLogo(sizeptr) \
	LP1(0x30, APTR, VABOUT_GetPNGLogo, int *, sizeptr, a0, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VABOUT_GetSSLLogo(sizeptr) \
	LP1(0x2a, APTR, VABOUT_GetSSLLogo, int *, sizeptr, a0, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VABOUT_GetV3Logo(sizeptr) \
	LP1(0x3c, APTR, VABOUT_GetV3Logo, int *, sizeptr, a0, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VABOUT_GetVLogo(sizeptr) \
	LP1(0x24, APTR, VABOUT_GetVLogo, int *, sizeptr, a0, \
	, V_ABOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_V_ABOUT_H */
