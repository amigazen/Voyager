/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_MUIGFX_H
#define _PPCINLINE_MUIGFX_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef MUIGFX_BASE_NAME
#define MUIGFX_BASE_NAME MUIGfxBase
#endif /* !MUIGFX_BASE_NAME */

#define MUIG_ObtainBestPenA(__p0, __p1, __p2, __p3, __p4) \
	LP5(90, LONG , MUIG_ObtainBestPenA, \
		struct ColorMap *, __p0, a0, \
		ULONG , __p1, d1, \
		ULONG , __p2, d2, \
		ULONG , __p3, d3, \
		struct TagItem *, __p4, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_ReleaseMUIPen(__p0, __p1) \
	LP2NR(108, MUIG_ReleaseMUIPen, \
		struct MUI_ScreenInfo *, __p0, a0, \
		LONG , __p1, d0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DrawImage(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8NR(132, MUIG_DrawImage, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct MUI_Image *, __p1, a1, \
		LONG , __p2, d0, \
		LONG , __p3, d1, \
		LONG , __p4, d2, \
		LONG , __p5, d3, \
		ULONG , __p6, d4, \
		ULONG , __p7, d5, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_RemapBitMap(__p0, __p1, __p2) \
	LP3(54, struct BitMap *, MUIG_RemapBitMap, \
		struct BitMap *, __p0, a0, \
		int , __p1, d0, \
		UBYTE *, __p2, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_TextPrint(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9NR(210, MUIG_TextPrint, \
		struct MUI_RenderInfo *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		char *, __p5, a1, \
		LONG , __p6, d4, \
		char *, __p7, a2, \
		LONG , __p8, d5, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_StringToRGB(__p0, __p1) \
	LP2(216, BOOL , MUIG_StringToRGB, \
		char *, __p0, a0, \
		ULONG *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetBitMapAttr(__p0, __p1) \
	LP2(42, ULONG , MUIG_GetBitMapAttr, \
		struct BitMap *, __p0, a0, \
		ULONG , __p1, d0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_FreeBitMap(__p0) \
	LP1NR(36, MUIG_FreeBitMap, \
		struct BitMap *, __p0, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_ScrollRaster(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(240, LONG , MUIG_ScrollRaster, \
		struct RastPort *, __p0, a0, \
		WORD , __p1, d0, \
		WORD , __p2, d1, \
		WORD , __p3, d2, \
		WORD , __p4, d3, \
		WORD , __p5, d4, \
		WORD , __p6, d5, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetRGB32(__p0, __p1, __p2) \
	LP3NR(228, MUIG_GetRGB32, \
		struct ColorMap *, __p0, a0, \
		ULONG , __p1, d0, \
		ULONG *, __p2, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_ReleasePen(__p0, __p1) \
	LP2NR(96, MUIG_ReleasePen, \
		struct ColorMap *, __p0, a0, \
		LONG , __p1, d0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_AllocBitMap(__p0, __p1, __p2, __p3, __p4) \
	LP5(30, struct BitMap *, MUIG_AllocBitMap, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		LONG , __p2, d2, \
		LONG , __p3, d3, \
		struct BitMap *, __p4, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_AllocBitMapFromBody(__p0, __p1) \
	LP2(144, struct BitMap *, MUIG_AllocBitMapFromBody, \
		struct BitMapHeader *, __p0, a0, \
		UBYTE *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetFrame(__p0, __p1) \
	LP2(180, struct Frame *, MUIG_GetFrame, \
		LONG , __p0, d0, \
		BOOL , __p1, d1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_ObtainMUIPen(__p0, __p1) \
	LP2(102, LONG , MUIG_ObtainMUIPen, \
		struct MUI_ScreenInfo *, __p0, a0, \
		struct MUI_PenSpec *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetRenderInfo(__p0, __p1) \
	LP2(168, struct MUI_RenderInfo *, MUIG_GetRenderInfo, \
		struct Screen *, __p0, a0, \
		Object *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_TextOffset(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8(264, LONG , MUIG_TextOffset, \
		struct MUI_RenderInfo *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		char *, __p5, a1, \
		LONG , __p6, d4, \
		char *, __p7, a2, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_SetRGB32(__p0, __p1, __p2) \
	LP3NR(222, MUIG_SetRGB32, \
		struct ViewPort *, __p0, a0, \
		ULONG , __p1, d0, \
		ULONG *, __p2, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_AllocMRIBuffer(__p0, __p1, __p2, __p3) \
	LP4(192, struct MUI_RenderInfo *, MUIG_AllocMRIBuffer, \
		Object *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		ULONG , __p3, d2, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_FreeRenderInfo(__p0) \
	LP1NR(174, MUIG_FreeRenderInfo, \
		struct MUI_RenderInfo *, __p0, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_ScrollDamage(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(234, BOOL , MUIG_ScrollDamage, \
		struct RastPort *, __p0, a0, \
		WORD , __p1, d0, \
		WORD , __p2, d1, \
		WORD , __p3, d2, \
		WORD , __p4, d3, \
		WORD , __p5, d4, \
		WORD , __p6, d5, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DoColorImposeHook(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(246, MUIG_DoColorImposeHook, \
		struct CGXHook *, __p0, a0, \
		struct RastPort *, __p1, a1, \
		LONG , __p2, d0, \
		LONG , __p3, d1, \
		LONG , __p4, d2, \
		LONG , __p5, d3, \
		ULONG , __p6, d4, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_BltBitMapTiled(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(72, MUIG_BltBitMapTiled, \
		struct BitMap *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		struct BitMap *, __p5, a1, \
		struct Rectangle *, __p6, a2, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DummyWasObtainPen(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6NR(84, MUIG_DummyWasObtainPen, \
		struct ColorMap *, __p0, a0, \
		LONG , __p1, d0, \
		ULONG , __p2, d1, \
		ULONG , __p3, d2, \
		ULONG , __p4, d3, \
		ULONG , __p5, d4, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DrawFrame(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(186, MUIG_DrawFrame, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct Frame *, __p1, a1, \
		LONG , __p2, d0, \
		LONG , __p3, d1, \
		LONG , __p4, d2, \
		LONG , __p5, d3, \
		LONG , __p6, d4, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetImageSize(__p0, __p1) \
	LP2NR(138, MUIG_GetImageSize, \
		struct MUI_Image *, __p0, a0, \
		struct MUI_MinMax *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetImageAttr(__p0, __p1, __p2) \
	LP3(126, ULONG , MUIG_GetImageAttr, \
		struct MUI_Image *, __p0, a0, \
		ULONG , __p1, d0, \
		struct MUI_RenderInfo *, __p2, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_TextDim(__p0, __p1, __p2, __p3, __p4) \
	LP5(204, LONG , MUIG_TextDim, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct TextFont *, __p1, a1, \
		char *, __p2, a2, \
		LONG , __p3, d0, \
		char *, __p4, a3, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DrawDisablePattern(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(78, MUIG_DrawDisablePattern, \
		struct MUI_RenderInfo *, __p0, a0, \
		LONG , __p1, d0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		LONG , __p4, d3, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetMBR(__p0, __p1, __p2, __p3) \
	LP4(66, struct BitMap *, MUIG_GetMBR, \
		struct MUI_RenderInfo *, __p0, a0, \
		struct BitMapHeader *, __p1, a1, \
		UBYTE *, __p2, a2, \
		struct BitMap **, __p3, a3, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetImage(__p0, __p1, __p2) \
	LP3(114, struct MUI_Image *, MUIG_GetImage, \
		struct MUI_RenderInfo *, __p0, a0, \
		char *, __p1, a1, \
		ULONG , __p2, d0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_ReleaseImage(__p0) \
	LP1NR(120, MUIG_ReleaseImage, \
		struct MUI_Image *, __p0, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_FreeMRIBuffer(__p0) \
	LP1NR(198, MUIG_FreeMRIBuffer, \
		struct MUI_RenderInfo *, __p0, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_OpenFont(__p0, __p1) \
	LP2(162, struct TextFont *, MUIG_OpenFont, \
		STRPTR , __p0, a0, \
		struct TextAttr *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_MaskBitMap(__p0, __p1) \
	LP2(60, struct BitMap *, MUIG_MaskBitMap, \
		struct BitMap *, __p0, a0, \
		LONG , __p1, d0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DoBalanceHook(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9NR(258, MUIG_DoBalanceHook, \
		struct CGXHook *, __p0, a0, \
		struct RastPort *, __p1, a1, \
		LONG , __p2, d0, \
		LONG , __p3, d1, \
		LONG , __p4, d2, \
		LONG , __p5, d3, \
		LONG , __p6, d4, \
		LONG , __p7, d5, \
		LONG , __p8, d6, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_GetScreenInfo(__p0) \
	LP1(150, struct MUI_ScreenInfo *, MUIG_GetScreenInfo, \
		struct Screen *, __p0, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_FreeScreenInfo(__p0) \
	LP1NR(156, MUIG_FreeScreenInfo, \
		struct MUI_ScreenInfo *, __p0, a0, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_CloneBitMap(__p0, __p1) \
	LP2(48, struct BitMap *, MUIG_CloneBitMap, \
		struct BitMap *, __p0, a0, \
		struct BitMap *, __p1, a1, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MUIG_DoBitMapImposeHook(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8, __p9, __p10, __p11) \
	LP12NR(252, MUIG_DoBitMapImposeHook, \
		struct CGXHook *, __p0, a0, \
		struct BitMap *, __p1, a1, \
		struct BitMap *, __p2, a2, \
		struct RastPort *, __p3, a3, \
		ULONG , __p4, d0, \
		ULONG , __p5, d1, \
		ULONG , __p6, d2, \
		ULONG , __p7, d3, \
		LONG , __p8, d4, \
		LONG , __p9, d5, \
		ULONG , __p10, d6, \
		ULONG , __p11, d7, \
		, MUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_MUIGFX_H */
