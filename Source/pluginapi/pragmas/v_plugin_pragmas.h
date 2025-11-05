#ifndef V_PLUGIN_PRAGMAS_H
#define V_PLUGIN_PRAGMAS_H
/*
 * $Id: v_plugin_pragmas.h,v 1.1.1.1 2001/04/16 15:53:09 zapek Exp $
 */

/* API V1 */
#pragma libcall VPluginBase VPLUG_Query 1e 0
#pragma libcall VPluginBase VPLUG_ProcessURLMethod 24 801
#pragma libcall VPluginBase VPLUG_GetURLData 2a 801
#pragma libcall VPluginBase VPLUG_GetURLMIMEType 30 801
#pragma libcall VPluginBase VPLUG_FreeURLData 36 801

/* API V2 additions */
#pragma libcall VPluginBase VPLUG_GetClass 3c 801
#pragma libcall VPluginBase VPLUG_Setup 42 801
#pragma libcall VPluginBase VPLUG_Cleanup 48 0
#pragma libcall VPluginBase VPLUG_FinalSetup 4e 0
#pragma libcall VPluginBase VPLUG_Hook_Prefs 54 8002

/* API V3 additions */
#pragma libcall VPluginBase VPLUG_GetInfo 5a 9802
#pragma libcall VPluginBase VPLUG_GetURLDataSize 60 801

/* API V4 additions */
#pragma libcall VPluginBase VPLUG_ProcessURLString 66 801

#endif /* !V_PLUGIN_PRAGMAS_H */
