/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_MUIMASTER_H
#define _PPCINLINE_MUIMASTER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef MUIMASTER_BASE_NAME
#define MUIMASTER_BASE_NAME MUIMasterBase
#endif /* !MUIMASTER_BASE_NAME */

#define MUI_RequestIDCMP(__p0, __p1) \
	LP2NR(90, MUI_RequestIDCMP, \
		Object *, __p0, a0, \
		ULONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_FreeClass(__p0) \
	LP1NR(84, MUI_FreeClass, \
		struct IClass *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ExportNumber(__p0, __p1, __p2) \
	LP3(408, ULONG , MUIP_ExportNumber, \
		Object *, __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_AddClipping(__p0, __p1, __p2, __p3, __p4) \
	LP5(168, APTR , MUI_AddClipping, \
		struct MUI_RenderInfo *, __p0, a0, \
		WORD , __p1, d0, \
		WORD , __p2, d1, \
		WORD , __p3, d2, \
		WORD , __p4, d3, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_TestIntuiMsg(__p0, __p1) \
	LP2(396, LONG , MUIP_TestIntuiMsg, \
		Object *, __p0, a0, \
		struct IntuiMessage *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_ObtainPen(__p0, __p1, __p2) \
	LP3(156, LONG , MUI_ObtainPen, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_PenSpec *, __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_RemoveClipping(__p0, __p1) \
	LP2NR(174, MUI_RemoveClipping, \
		struct MUI_RenderInfo *, __p0, a0, \
		APTR , __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_ReleasePen(__p0, __p1) \
	LP2NR(162, MUI_ReleasePen, \
		struct MUI_RenderInfo *, __p0, a0, \
		LONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetVirtualRect(__p0, __p1) \
	LP2NR(240, MUIP_GetVirtualRect, \
		Object *, __p0, a0, \
		struct LongRect *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_EndRefresh(__p0, __p1) \
	LP2NR(198, MUI_EndRefresh, \
		struct MUI_RenderInfo *, __p0, a0, \
		ULONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_RegUser() \
	LP0(288, struct RegUser *, MUIP_RegUser, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetImageAttr(__p0, __p1, __p2) \
	LP3(390, ULONG , MUIP_GetImageAttr, \
		struct MUI_Image *, __p0, a0, \
		ULONG , __p1, d0, \
		struct MUI_RenderInfo *, __p2, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ObtainImage(__p0, __p1, __p2) \
	LP3(204, struct MUI_Image *, MUIP_ObtainImage, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_ImageSpec *, __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_FreeMem(__p0, __p1) \
	LP2NR(138, MUIP_FreeMem, \
		APTR , __p0, a1, \
		ULONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_RegStatus() \
	LP0(282, LONG , MUIP_RegStatus, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ShowClipped(__p0, __p1) \
	LP2(312, ULONG , MUIP_ShowClipped, \
		Object *, __p0, a0, \
		struct LongRect *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_AllocAslRequest(__p0, __p1) \
	LP2(48, APTR , MUI_AllocAslRequest, \
		unsigned long , __p0, d0, \
		struct TagItem *, __p1, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ReleaseImage(__p0, __p1) \
	LP2NR(210, MUIP_ReleaseImage, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_Image *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_GetClass(__p0) \
	LP1(78, struct IClass *, MUI_GetClass, \
		STRPTR , __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_TestDebug(__p0) \
	LP1(258, BOOL , MUIP_TestDebug, \
		ULONG , __p0, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_IsInVirtualRect(__p0, __p1) \
	LP2(246, BOOL , MUIP_IsInVirtualRect, \
		Object *, __p0, a0, \
		struct LongRect *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_AllocMem(__p0) \
	LP1(132, APTR , MUIP_AllocMem, \
		ULONG , __p0, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetRenderInfo(__p0, __p1, __p2) \
	LP3(330, struct MUI_RenderInfo *, MUIP_GetRenderInfo, \
		struct Screen *, __p0, a0, \
		Object *, __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_Show(__p0) \
	LP1(216, ULONG , MUI_Show, \
		Object *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_IMsgToChar(__p0, __p1, __p2) \
	LP3(360, LONG , MUIP_IMsgToChar, \
		struct IntuiMessage *, __p0, a0, \
		ULONG , __p1, d0, \
		ULONG , __p2, d1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ImportString(__p0, __p1, __p2) \
	LP3(426, ULONG , MUIP_ImportString, \
		Object *, __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ParseVersionString(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(366, BOOL , MUIP_ParseVersionString, \
		STRPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		int , __p2, d0, \
		STRPTR *, __p3, a2, \
		STRPTR *, __p4, a3, \
		STRPTR *, __p5, a4, \
		STRPTR *, __p6, a5, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_RejectIDCMP(__p0, __p1) \
	LP2NR(96, MUI_RejectIDCMP, \
		Object *, __p0, a0, \
		ULONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_FreeAslRequest(__p0) \
	LP1NR(60, MUI_FreeAslRequest, \
		APTR , __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_DisposeObject(__p0) \
	LP1NR(36, MUI_DisposeObject, \
		Object *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ScrollRaster(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(354, LONG , MUIP_ScrollRaster, \
		struct RastPort *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		LONG , __p5, d4, \
		LONG , __p6, d5, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ImportNumber(__p0, __p1, __p2) \
	LP3(420, ULONG , MUIP_ImportNumber, \
		Object *, __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetFrameAttr(__p0, __p1, __p2) \
	LP3(342, LONG , MUIP_GetFrameAttr, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_FrameSpec *, __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ConfigItemAvailable(__p0) \
	LP1(270, BOOL , MUIP_ConfigItemAvailable, \
		struct ConfigItem *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_MakeObjectA(__p0, __p1) \
	LP2(120, Object *, MUI_MakeObjectA, \
		LONG , __p0, d0, \
		ULONG *, __p1, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_SetError(__p0) \
	LP1(72, LONG , MUI_SetError, \
		LONG , __p0, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_RemoveClipRegion(__p0, __p1) \
	LP2NR(186, MUI_RemoveClipRegion, \
		struct MUI_RenderInfo *, __p0, a0, \
		APTR , __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_Hide(__p0) \
	LP1(222, ULONG , MUI_Hide, \
		Object *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_AllocVecPooled(__p0, __p1, __p2) \
	LP3(318, APTR , MUIP_AllocVecPooled, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		int , __p2, a4, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_AllocImage(__p0, __p1, __p2) \
	LP3(372, struct MUI_Image *, MUIP_AllocImage, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_ImageSpec *, __p1, a1, \
		struct TagItem *, __p2, a2, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_FreeRenderInfo(__p0) \
	LP1NR(336, MUIP_FreeRenderInfo, \
		struct MUI_RenderInfo *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_Layout(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(126, BOOL , MUI_Layout, \
		Object *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		ULONG , __p5, d4, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_DrawImage(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(384, MUIP_DrawImage, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_Image *, __p1, a1, \
		LONG , __p2, d0, \
		LONG , __p3, d1, \
		LONG , __p4, d2, \
		LONG , __p5, d3, \
		struct TagItem *, __p6, a2, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_NewObjectA(__p0, __p1) \
	LP2(30, Object *, MUI_NewObjectA, \
		STRPTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_DeleteCustomClass(__p0) \
	LP1(114, BOOL , MUI_DeleteCustomClass, \
		struct MUI_CustomClass *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetApplistObject() \
	LP0(264, Object *, MUIP_GetApplistObject, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_Redraw(__p0, __p1) \
	LP2NR(102, MUI_Redraw, \
		Object *, __p0, a0, \
		ULONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_AslRequest(__p0, __p1) \
	LP2(54, BOOL , MUI_AslRequest, \
		APTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_FreeVec(__p0) \
	LP1NR(150, MUIP_FreeVec, \
		APTR , __p0, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_BeginRefresh(__p0, __p1) \
	LP2(192, BOOL , MUI_BeginRefresh, \
		struct MUI_RenderInfo *, __p0, a0, \
		ULONG , __p1, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_AllocVec(__p0) \
	LP1(144, APTR , MUIP_AllocVec, \
		ULONG , __p0, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_LayoutObj(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(228, BOOL , MUI_LayoutObj, \
		Object *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		ULONG , __p5, d4, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_Get020Base() \
	LP0(402, struct Library *, MUIP_Get020Base, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_SetTestMode(__p0) \
	LP1NR(294, MUIP_SetTestMode, \
		BOOL , __p0, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_CreateCustomClass(__p0, __p1, __p2, __p3, __p4) \
	LP5(108, struct MUI_CustomClass *, MUI_CreateCustomClass, \
		struct Library *, __p0, a0, \
		STRPTR , __p1, a1, \
		struct MUI_CustomClass *, __p2, a2, \
		int , __p3, d0, \
		APTR , __p4, a3, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetConfigItemStructure() \
	LP0(300, struct ConfigItem *, MUIP_GetConfigItemStructure, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_Offset(__p0, __p1, __p2) \
	LP3NR(234, MUI_Offset, \
		Object *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ExportString(__p0, __p1, __p2) \
	LP3(414, ULONG , MUIP_ExportString, \
		Object *, __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetCclistObject() \
	LP0(306, Object *, MUIP_GetCclistObject, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_GetMasterConfigObject() \
	LP0(276, Object *, MUIP_GetMasterConfigObject, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_DrawFrame(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(348, MUIP_DrawFrame, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_FrameSpec *, __p1, a1, \
		LONG , __p2, d0, \
		LONG , __p3, d1, \
		LONG , __p4, d2, \
		LONG , __p5, d3, \
		ULONG , __p6, d4, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_Error() \
	LP0(66, LONG , MUI_Error, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_RequestA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(42, LONG , MUI_RequestA, \
		APTR , __p0, d0, \
		APTR , __p1, d1, \
		ULONG , __p2, d2, \
		STRPTR , __p3, a0, \
		STRPTR , __p4, a1, \
		STRPTR , __p5, a2, \
		APTR , __p6, a3, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_ReadConfigItem(__p0) \
	LP1(252, struct ConfigItem *, MUIP_ReadConfigItem, \
		LONG , __p0, d0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_FreeVecPooled(__p0, __p1, __p2) \
	LP3(324, APTR , MUIP_FreeVecPooled, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		int , __p2, a4, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUI_AddClipRegion(__p0, __p1) \
	LP2(180, APTR , MUI_AddClipRegion, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct Region *, __p1, a1, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIP_FreeImage(__p0) \
	LP1NR(378, MUIP_FreeImage, \
		struct MUI_Image *, __p0, a0, \
		, MUIMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#ifdef USE_INLINE_STDARG

#include <stdarg.h>

#define MUI_AslRequestTags(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MUI_AslRequest(__p0, (struct TagItem *)_tags);})

#define MUI_MakeObject(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MUI_MakeObjectA(__p0, (ULONG *)_tags);})

#define MUI_NewObject(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MUI_NewObjectA(__p0, (struct TagItem *)_tags);})

#define MUI_AllocAslRequestTags(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MUI_AllocAslRequest(__p0, (struct TagItem *)_tags);})

#define MUI_Request(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MUI_RequestA(__p0, __p1, __p2, __p3, __p4, __p5, (APTR )_tags);})

#endif

#endif /* !_PPCINLINE_MUIMASTER_H */
