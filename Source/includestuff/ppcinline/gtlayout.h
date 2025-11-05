/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_GTLAYOUT_H
#define _PPCINLINE_GTLAYOUT_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef GTLAYOUT_BASE_NAME
#define GTLAYOUT_BASE_NAME GTLayoutBase
#endif /* !GTLAYOUT_BASE_NAME */

#define LT_Activate(handle, id) \
	LP2NR(0xae, LT_Activate, struct LayoutHandle *, handle, a0, LONG, id, d0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_AddA(handle, type, label, id, tagList) \
	LP5NR(0x5a, LT_AddA, struct LayoutHandle *, handle, a0, LONG, type, d0, STRPTR, label, d1, LONG, id, d2, struct TagItem *, tagList, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_Add(a0, a1, a2, a3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_AddA((a0), (a1), (a2), (a3), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_BeginRefresh(handle) \
	LP1NR(0x42, LT_BeginRefresh, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_BuildA(handle, tagParams) \
	LP2(0xcc, struct Window *, LT_BuildA, struct LayoutHandle *, handle, a0, struct TagItem *, tagParams, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_Build(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_BuildA((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_CatchUpRefresh(handle) \
	LP1NR(0x10e, LT_CatchUpRefresh, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_CreateHandle(screen, font) \
	LP2(0x2a, struct LayoutHandle *, LT_CreateHandle, struct Screen *, screen, a0, struct TextAttr *, font, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_CreateHandleTagList(screen, tagList) \
	LP2(0x30, struct LayoutHandle *, LT_CreateHandleTagList, struct Screen *, screen, a0, struct TagItem *, tagList, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_CreateHandleTags(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_CreateHandleTagList((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_DeleteHandle(handle) \
	LP1NR(0x24, LT_DeleteHandle, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_DeleteWindowLock(window) \
	LP1NR(0xa2, LT_DeleteWindowLock, struct Window *, window, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_DisposeMenu(menu) \
	LP1NR(0xde, LT_DisposeMenu, struct Menu *, menu, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_EndGroup(handle) \
	LP1NR(0x66, LT_EndGroup, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_EndRefresh(handle, complete) \
	LP2NR(0x48, LT_EndRefresh, struct LayoutHandle *, handle, a0, LONG, complete, d0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_FindMenuCommand(menu, msgCode, msgQualifier, msgGadget) \
	LP4(0xfc, struct MenuItem *, LT_FindMenuCommand, struct Menu *, menu, a0, ULONG, msgCode, d0, ULONG, msgQualifier, d1, struct Gadget *, msgGadget, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_GetAttributesA(handle, id, tagList) \
	LP3(0x4e, LONG, LT_GetAttributesA, struct LayoutHandle *, handle, a0, LONG, id, d0, struct TagItem *, tagList, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_GetAttributes(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_GetAttributesA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_GetCode(msgQualifier, msgClass, msgCode, msgGadget) \
	LP4(0xba, LONG, LT_GetCode, ULONG, msgQualifier, d0, ULONG, msgClass, d1, ULONG, msgCode, d2, struct Gadget *, msgGadget, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_GetIMsg(handle) \
	LP1(0xc0, struct IntuiMessage *, LT_GetIMsg, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_GetMenuItem(menu, id) \
	LP2(0xf6, struct MenuItem *, LT_GetMenuItem, struct Menu *, menu, a0, ULONG, id, d0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_GetWindowUserData(window, defaultValue) \
	LP2(0x114, APTR, LT_GetWindowUserData, struct Window *, window, a0, APTR, defaultValue, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_HandleInput(handle, msgQualifier, msgClass, msgCode, msgGadget) \
	LP5NR(0x3c, LT_HandleInput, struct LayoutHandle *, handle, a0, ULONG, msgQualifier, d0, ULONG *, msgClass, a1, UWORD *, msgCode, a2, struct Gadget **, msgGadget, a3, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_LabelChars(handle, label) \
	LP2(0x90, LONG, LT_LabelChars, struct LayoutHandle *, handle, a0, STRPTR, label, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_LabelWidth(handle, label) \
	LP2(0x8a, LONG, LT_LabelWidth, struct LayoutHandle *, handle, a0, STRPTR, label, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_LayoutA(handle, title, bounds, extraWidth, extraHeight, idcmp, align, tagParams) \
	LP8(0x6c, struct Window *, LT_LayoutA, struct LayoutHandle *, handle, a0, STRPTR, title, a1, struct IBox *, bounds, a2, LONG, extraWidth, d0, LONG, extraHeight, d1, ULONG, idcmp, d2, LONG, align, d3, struct TagItem *, tagParams, a3, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_Layout(a0, a1, a2, a3, a4, a5, a6, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_LayoutA((a0), (a1), (a2), (a3), (a4), (a5), (a6), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_LayoutMenusA(handle, menuTemplate, tagParams) \
	LP3(0x72, struct Menu *, LT_LayoutMenusA, struct LayoutHandle *, handle, a0, struct NewMenu *, menuTemplate, a1, struct TagItem *, tagParams, a2, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_LayoutMenus(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_LayoutMenusA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_LevelWidth(handle, levelFormat, dispFunc, min, max, maxWidth, maxLen, fullCheck) \
	LP8NR(0x1e, LT_LevelWidth, struct LayoutHandle *, handle, a0, STRPTR, levelFormat, a1, APTR, dispFunc, a2, LONG, min, d0, LONG, max, d1, LONG *, maxWidth, a3, LONG *, maxLen, a5, LONG, fullCheck, d2, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_LockWindow(window) \
	LP1NR(0x96, LT_LockWindow, struct Window *, window, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_MenuControlTagList(window, intuitionMenu, tags) \
	LP3NR(0xf0, LT_MenuControlTagList, struct Window *, window, a0, struct Menu *, intuitionMenu, a1, struct TagItem *, tags, a2, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_MenuControlTags(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_MenuControlTagList((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_NewA(handle, tagList) \
	LP2NR(0x60, LT_NewA, struct LayoutHandle *, handle, a0, struct TagItem *, tagList, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_New(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_NewA((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_NewLevelWidth(handle, levelFormat, dispFunc, min, max, maxWidth) \
	LP6NR(0x102, LT_NewLevelWidth, struct LayoutHandle *, handle, a0, STRPTR, levelFormat, a1, APTR, dispFunc, a2, LONG, min, d0, LONG, max, d1, LONG *, maxWidth, a3, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_NewMenuTagList(tagList) \
	LP1(0xea, struct Menu *, LT_NewMenuTagList, struct TagItem *, tagList, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_NewMenuTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_NewMenuTagList((struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_NewMenuTemplate(screen, textAttr, amigaGlyph, checkGlyph, error, menuTemplate) \
	LP6(0xe4, struct Menu *, LT_NewMenuTemplate, struct Screen *, screen, a0, struct TextAttr *, textAttr, a1, struct Image *, amigaGlyph, a2, struct Image *, checkGlyph, a3, LONG *, error, d0, struct NewMenu *, menuTemplate, d1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_PressButton(handle, id) \
	LP2(0xb4, BOOL, LT_PressButton, struct LayoutHandle *, handle, a0, LONG, id, d0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_Rebuild(handle, bounds, extraWidth, extraHeight, clear) \
	LP5(0x36, BOOL, LT_Rebuild, struct LayoutHandle *, handle, a0, struct IBox *, bounds, a1, LONG, extraWidth, a2, LONG, extraHeight, d0, LONG, clear, d1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_RebuildTagList(handle, clear, tags) \
	LP3(0xd2, BOOL, LT_RebuildTagList, struct LayoutHandle *, handle, a0, LONG, clear, d0, struct TagItem *, tags, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_RebuildTags(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_RebuildTagList((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_Redraw(handle, id) \
	LP2NR(0x11a, LT_Redraw, struct LayoutHandle *, handle, a0, LONG, id, d0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_Refresh(handle) \
	LP1NR(0x108, LT_Refresh, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_ReplyIMsg(msg) \
	LP1NR(0xc6, LT_ReplyIMsg, struct IntuiMessage *, msg, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_SetAttributesA(handle, id, tagList) \
	LP3NR(0x54, LT_SetAttributesA, struct LayoutHandle *, handle, a0, LONG, id, d0, struct TagItem *, tagList, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LT_SetAttributes(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; LT_SetAttributesA((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define LT_ShowWindow(handle, activate) \
	LP2NR(0xa8, LT_ShowWindow, struct LayoutHandle *, handle, a0, LONG, activate, a1, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_UnlockWindow(window) \
	LP1NR(0x9c, LT_UnlockWindow, struct Window *, window, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LT_UpdateStrings(handle) \
	LP1NR(0xd8, LT_UpdateStrings, struct LayoutHandle *, handle, a0, \
	, GTLAYOUT_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_GTLAYOUT_H */
