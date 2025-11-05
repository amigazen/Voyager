/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_V_PLUGIN_H
#define _PPCINLINE_V_PLUGIN_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef V_PLUGIN_BASE_NAME
#define V_PLUGIN_BASE_NAME VPluginBase
#endif /* !V_PLUGIN_BASE_NAME */

#define VPLUG_Cleanup() \
	LP0NR(0x48, VPLUG_Cleanup, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_FinalSetup() \
	LP0NR(0x4e, VPLUG_FinalSetup, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_FreeURLData(urlhandle) \
	LP1NR(0x36, VPLUG_FreeURLData, APTR, urlhandle, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_GetClass(mimetype) \
	LP1(0x3c, APTR, VPLUG_GetClass, STRPTR, mimetype, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_GetInfo(vpluginfo, nethandle) \
	LP2(0x5a, BOOL, VPLUG_GetInfo, struct VPlugInfo *, vpluginfo, a0, APTR, nethandle, a1, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_GetURLData(urlhandle) \
	LP1(0x2a, APTR, VPLUG_GetURLData, APTR, urlhandle, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_GetURLDataSize(urlhandle) \
	LP1(0x60, int, VPLUG_GetURLDataSize, APTR, urlhandle, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_GetURLMIMEType(urlhandle) \
	LP1(0x30, STRPTR, VPLUG_GetURLMIMEType, APTR, urlhandle, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_Hook_Prefs(method, prefsobj) \
	LP2NR(0x54, VPLUG_Hook_Prefs, ULONG, method, d0, struct vplug_prefs *, prefsobj, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_ProcessURLMethod(url) \
	LP1(0x24, APTR, VPLUG_ProcessURLMethod, STRPTR, url, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_ProcessURLString(string) \
	LP1(0x66, int, VPLUG_ProcessURLString, STRPTR, string, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_Query() \
	LP0(0x1e, struct TagItem *, VPLUG_Query, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define VPLUG_Setup(functable) \
	LP1(0x42, BOOL, VPLUG_Setup, struct vplug_functable *, functable, a0, \
	, V_PLUGIN_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_V_PLUGIN_H */
