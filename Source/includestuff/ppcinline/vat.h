/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_VAT_H
#define _PPCINLINE_VAT_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef VAT_BASE_NAME
#define VAT_BASE_NAME VATBase
#endif /* !VAT_BASE_NAME */

#define VAT_FGetsAsync(__p0, __p1, __p2) \
	LP3(102, UBYTE *, VAT_FGetsAsync, \
		struct AsyncFile *, __p0, a0, \
		UBYTE *, __p1, a1, \
		LONG , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_ReadCharAsync(__p0) \
	LP1(78, LONG , VAT_ReadCharAsync, \
		struct AsyncFile *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MultiSetA(__p0, __p1, __p2) \
	LP3NR(294, VAT_MultiSetA, \
		ULONG , __p0, d0, \
		LONG , __p1, d1, \
		APTR *, __p2, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_CalcMD5(__p0, __p1, __p2) \
	LP3NR(174, VAT_CalcMD5, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		APTR , __p2, a1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_FGets(__p0, __p1, __p2) \
	LP3(354, int , VAT_FGets, \
		BPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		int , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_ReadAsync(__p0, __p1, __p2) \
	LP3(72, LONG , VAT_ReadAsync, \
		struct AsyncFile *, __p0, a0, \
		APTR , __p1, a1, \
		LONG , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MIME_FindByExtension(__p0, __p1, __p2, __p3, __p4) \
	LP5(336, int , VAT_MIME_FindByExtension, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		int *, __p3, a3, \
		char *, __p4, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_GetAppScreen(__p0) \
	LP1(312, struct Screen *, VAT_GetAppScreen, \
		APTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_Random() \
	LP0(204, ULONG , VAT_Random, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_FreeVecPooled(__p0, __p1) \
	LP2NR(162, VAT_FreeVecPooled, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_RemSpaces(__p0) \
	LP1NR(234, VAT_RemSpaces, \
		STRPTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_UnGetCAsync(__p0) \
	LP1NR(120, VAT_UnGetCAsync, \
		struct AsyncFile *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_FreePooled(__p0, __p1, __p2) \
	LP3NR(150, VAT_FreePooled, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_StrDupPooled(__p0, __p1) \
	LP2(168, STRPTR , VAT_StrDupPooled, \
		APTR , __p0, a0, \
		STRPTR , __p1, a1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_Timev() \
	LP0(246, time_t , VAT_Timev, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_FGetsAsyncNoLF(__p0, __p1, __p2) \
	LP3(126, UBYTE *, VAT_FGetsAsyncNoLF, \
		struct AsyncFile *, __p0, a0, \
		UBYTE *, __p1, a1, \
		LONG , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_WriteCharAsync(__p0, __p1) \
	LP2(90, LONG , VAT_WriteCharAsync, \
		struct AsyncFile *, __p0, a0, \
		UBYTE , __p1, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_OpenLibrary(__p0, __p1) \
	LP2(42, struct Library *, VAT_OpenLibrary, \
		STRPTR , __p0, a0, \
		ULONG , __p1, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_Cleanup(__p0) \
	LP1NR(36, VAT_Cleanup, \
		APTR *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_WriteAsync(__p0, __p1, __p2) \
	LP3(84, LONG , VAT_WriteAsync, \
		struct AsyncFile *, __p0, a0, \
		APTR , __p1, a1, \
		LONG , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MD5Update(__p0, __p1, __p2) \
	LP3NR(192, VAT_MD5Update, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_AllocVecPooled(__p0, __p1) \
	LP2(156, APTR , VAT_AllocVecPooled, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_DeletePool(__p0) \
	LP1NR(138, VAT_DeletePool, \
		APTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_CheckProgramInPathFull(__p0, __p1) \
	LP2(258, int , VAT_CheckProgramInPathFull, \
		STRPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_CreatePool(__p0, __p1, __p2) \
	LP3(132, APTR , VAT_CreatePool, \
		ULONG , __p0, d0, \
		ULONG , __p1, d1, \
		ULONG , __p2, d2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_FreeURLList(__p0) \
	LP1NR(288, VAT_FreeURLList, \
		struct VATS_URL *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_ScanForURLS(__p0) \
	LP1(282, struct VATS_URL *, VAT_ScanForURLS, \
		STRPTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_RandomByte() \
	LP0(210, ULONG , VAT_RandomByte, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_CheckEcrypt(__p0) \
	LP1NR(372, VAT_CheckEcrypt, \
		time_t , __p0, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MPZFree(__p0) \
	LP1NR(222, VAT_MPZFree, \
		APTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_GetFilesizeAsync(__p0) \
	LP1(276, LONG , VAT_GetFilesizeAsync, \
		struct AsyncFile *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MD5Init(__p0) \
	LP1NR(180, VAT_MD5Init, \
		APTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_SendRXMsg(__p0, __p1, __p2) \
	LP3(264, int , VAT_SendRXMsg, \
		STRPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		STRPTR , __p2, a2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_GetDataType(__p0, __p1, __p2, __p3) \
	LP4(324, int , VAT_GetDataType, \
		STRPTR , __p0, a0, \
		ULONG *, __p1, a1, \
		ULONG *, __p2, a2, \
		STRPTR , __p3, a3, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_ExpandTemplateA(__p0, __p1, __p2, __p3) \
	LP4(228, int , VAT_ExpandTemplateA, \
		STRPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		ULONG , __p2, d0, \
		APTR , __p3, a2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_ShowRegUtil(__p0) \
	LP1NR(318, VAT_ShowRegUtil, \
		struct Screen *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_IsOnline() \
	LP0(366, int , VAT_IsOnline, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_OpenLibraryCode(__p0) \
	LP1(48, struct Library *, VAT_OpenLibraryCode, \
		ULONG , __p0, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_GetAppScreenName(__p0) \
	LP1(348, STRPTR , VAT_GetAppScreenName, \
		APTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_RandomStir() \
	LP0NR(198, VAT_RandomStir, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MIME_FindByType(__p0, __p1, __p2, __p3) \
	LP4(342, int , VAT_MIME_FindByType, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		int *, __p3, a3, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_SeekAsync(__p0, __p1, __p2) \
	LP3(96, LONG , VAT_SeekAsync, \
		struct AsyncFile *, __p0, a0, \
		LONG , __p1, d0, \
		BYTE , __p2, d1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_NewShowRegUtil(__p0, __p1) \
	LP2NR(360, VAT_NewShowRegUtil, \
		APTR , __p0, a0, \
		STRPTR , __p1, a1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_IsAmigaGuideFile(__p0) \
	LP1(330, int , VAT_IsAmigaGuideFile, \
		STRPTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_VFPrintfAsync(__p0, __p1, __p2) \
	LP3(108, LONG , VAT_VFPrintfAsync, \
		struct AsyncFile *, __p0, a0, \
		STRPTR , __p1, a1, \
		APTR , __p2, a2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MPZPow(__p0, __p1, __p2, __p3) \
	LP4NR(216, VAT_MPZPow, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_AllocPooled(__p0, __p1) \
	LP2(144, APTR , VAT_AllocPooled, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_CheckVATVersion(__p0) \
	LP1(54, int , VAT_CheckVATVersion, \
		ULONG , __p0, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_Time(__p0) \
	LP1(240, time_t , VAT_Time, \
		time_t *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_MD5Final(__p0, __p1) \
	LP2NR(186, VAT_MD5Final, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_OpenAsync(__p0, __p1, __p2) \
	LP3(60, struct AsyncFile *, VAT_OpenAsync, \
		const STRPTR , __p0, a0, \
		UBYTE , __p1, d0, \
		LONG , __p2, d1, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_SetLastUsedDir(__p0) \
	LP1NR(252, VAT_SetLastUsedDir, \
		STRPTR , __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_Initialize(__p0, __p1, __p2, __p3) \
	LP4(30, int , VAT_Initialize, \
		STRPTR , __p0, a0, \
		APTR *, __p1, a1, \
		STRPTR , __p2, a2, \
		ULONG , __p3, d0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_CloseAsync(__p0) \
	LP1(66, LONG , VAT_CloseAsync, \
		struct AsyncFile *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_ShowURL(__p0, __p1, __p2) \
	LP3NR(270, VAT_ShowURL, \
		STRPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		APTR , __p2, a2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_SetFmtA(__p0, __p1, __p2, __p3) \
	LP4NR(300, VAT_SetFmtA, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		STRPTR , __p2, a1, \
		APTR , __p3, a2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_SetTxtFmtA(__p0, __p1, __p2) \
	LP3NR(306, VAT_SetTxtFmtA, \
		APTR , __p0, a0, \
		STRPTR , __p1, a1, \
		APTR , __p2, a2, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VAT_FtellAsync(__p0) \
	LP1(114, LONG , VAT_FtellAsync, \
		struct AsyncFile *, __p0, a0, \
		, VAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#ifdef USE_INLINE_STDARG

#include <stdarg.h>

#define VAT_SetFmt(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	VAT_SetFmtA(__p0, __p1, __p2, (APTR )_tags);})

#define VAT_SetTxtFmt(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	VAT_SetTxtFmtA(__p0, __p1, (APTR )_tags);})

#define VAT_MultiSet(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	VAT_MultiSetA(__p0, __p1, (APTR *)_tags);})

#define VAT_ExpandTemplate(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	VAT_ExpandTemplateA(__p0, __p1, __p2, (APTR )_tags);})

#endif

#endif /* !_PPCINLINE_VAT_H */
