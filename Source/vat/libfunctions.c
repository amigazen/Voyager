/* Automatically generated file! Do not edit! */

#include "clib/vat_protos.h"

#include <emul/emulregs.h>

UBYTE * LIB_VAT_FGetsAsync(void)
{
	return (UBYTE *)VAT_FGetsAsync((struct AsyncFile *)REG_A0, (UBYTE *)REG_A1, (LONG )REG_D0);
}

LONG  LIB_VAT_ReadCharAsync(void)
{
	return (LONG )VAT_ReadCharAsync((struct AsyncFile *)REG_A0);
}

void  LIB_VAT_MultiSetA(void)
{
	VAT_MultiSetA((ULONG )REG_D0, (LONG )REG_D1, (APTR *)REG_A0);
}

void  LIB_VAT_CalcMD5(void)
{
	VAT_CalcMD5((APTR )REG_A0, (ULONG )REG_D0, (APTR )REG_A1);
}

int  LIB_VAT_FGets(void)
{
	return (int )VAT_FGets((BPTR )REG_A0, (STRPTR )REG_A1, (int )REG_D0);
}

LONG  LIB_VAT_ReadAsync(void)
{
	return (LONG )VAT_ReadAsync((struct AsyncFile *)REG_A0, (APTR )REG_A1, (LONG )REG_D0);
}

int  LIB_VAT_MIME_FindByExtension(void)
{
	return (int )VAT_MIME_FindByExtension((char *)REG_A0, (char *)REG_A1, (char *)REG_A2, (int *)REG_A3, (char *)REG_D0);
}

struct Screen * LIB_VAT_GetAppScreen(void)
{
	return (struct Screen *)VAT_GetAppScreen((APTR )REG_A0);
}

ULONG  LIB_VAT_Random(void)
{
	return (ULONG )VAT_Random();
}

void  LIB_VAT_FreeVecPooled(void)
{
	VAT_FreeVecPooled((APTR )REG_A0, (APTR )REG_A1);
}

void  LIB_VAT_RemSpaces(void)
{
	VAT_RemSpaces((STRPTR )REG_A0);
}

void  LIB_VAT_UnGetCAsync(void)
{
	VAT_UnGetCAsync((struct AsyncFile *)REG_A0);
}

void  LIB_VAT_FreePooled(void)
{
	VAT_FreePooled((APTR )REG_A0, (APTR )REG_A1, (ULONG )REG_D0);
}

STRPTR  LIB_VAT_StrDupPooled(void)
{
	return (STRPTR )VAT_StrDupPooled((APTR )REG_A0, (STRPTR )REG_A1);
}

time_t  LIB_VAT_Timev(void)
{
	return (time_t )VAT_Timev();
}

UBYTE * LIB_VAT_FGetsAsyncNoLF(void)
{
	return (UBYTE *)VAT_FGetsAsyncNoLF((struct AsyncFile *)REG_A0, (UBYTE *)REG_A1, (LONG )REG_D0);
}

LONG  LIB_VAT_WriteCharAsync(void)
{
	return (LONG )VAT_WriteCharAsync((struct AsyncFile *)REG_A0, (UBYTE )REG_D0);
}

struct Library * LIB_VAT_OpenLibrary(void)
{
	return (struct Library *)VAT_OpenLibrary((STRPTR )REG_A0, (ULONG )REG_D0);
}

void  LIB_VAT_Cleanup(void)
{
	VAT_Cleanup((APTR *)REG_A0);
}

LONG  LIB_VAT_WriteAsync(void)
{
	return (LONG )VAT_WriteAsync((struct AsyncFile *)REG_A0, (APTR )REG_A1, (LONG )REG_D0);
}

void  LIB_VAT_MD5Update(void)
{
	VAT_MD5Update((APTR )REG_A0, (APTR )REG_A1, (ULONG )REG_D0);
}

APTR  LIB_VAT_AllocVecPooled(void)
{
	return (APTR )VAT_AllocVecPooled((APTR )REG_A0, (ULONG )REG_D0);
}

void  LIB_VAT_DeletePool(void)
{
	VAT_DeletePool((APTR )REG_A0);
}

int  LIB_VAT_CheckProgramInPathFull(void)
{
	return (int )VAT_CheckProgramInPathFull((STRPTR )REG_A0, (STRPTR )REG_A1);
}

APTR  LIB_VAT_CreatePool(void)
{
	return (APTR )VAT_CreatePool((ULONG )REG_D0, (ULONG )REG_D1, (ULONG )REG_D2);
}

void  LIB_VAT_FreeURLList(void)
{
	VAT_FreeURLList((struct VATS_URL *)REG_A0);
}

struct VATS_URL * LIB_VAT_ScanForURLS(void)
{
	return (struct VATS_URL *)VAT_ScanForURLS((STRPTR )REG_A0);
}

ULONG  LIB_VAT_RandomByte(void)
{
	return (ULONG )VAT_RandomByte();
}

void  LIB_VAT_CheckEcrypt(void)
{
	VAT_CheckEcrypt((time_t )REG_D0);
}

void  LIB_VAT_MPZFree(void)
{
	VAT_MPZFree((APTR )REG_A0);
}

LONG  LIB_VAT_GetFilesizeAsync(void)
{
	return (LONG )VAT_GetFilesizeAsync((struct AsyncFile *)REG_A0);
}

void  LIB_VAT_MD5Init(void)
{
	VAT_MD5Init((APTR )REG_A0);
}

int  LIB_VAT_SendRXMsg(void)
{
	return (int )VAT_SendRXMsg((STRPTR )REG_A0, (STRPTR )REG_A1, (STRPTR )REG_A2);
}

int  LIB_VAT_GetDataType(void)
{
	return (int )VAT_GetDataType((STRPTR )REG_A0, (ULONG *)REG_A1, (ULONG *)REG_A2, (STRPTR )REG_A3);
}

int  LIB_VAT_ExpandTemplateA(void)
{
	return (int )VAT_ExpandTemplateA((STRPTR )REG_A0, (STRPTR )REG_A1, (ULONG )REG_D0, (APTR )REG_A2);
}

void  LIB_VAT_ShowRegUtil(void)
{
	VAT_ShowRegUtil((struct Screen *)REG_A0);
}

int  LIB_VAT_IsOnline(void)
{
	return (int )VAT_IsOnline();
}

struct Library * LIB_VAT_OpenLibraryCode(void)
{
	return (struct Library *)VAT_OpenLibraryCode((ULONG )REG_D0);
}

STRPTR  LIB_VAT_GetAppScreenName(void)
{
	return (STRPTR )VAT_GetAppScreenName((APTR )REG_A0);
}

void  LIB_VAT_RandomStir(void)
{
	VAT_RandomStir();
}

int  LIB_VAT_MIME_FindByType(void)
{
	return (int )VAT_MIME_FindByType((char *)REG_A0, (char *)REG_A1, (char *)REG_A2, (int *)REG_A3);
}

LONG  LIB_VAT_SeekAsync(void)
{
	return (LONG )VAT_SeekAsync((struct AsyncFile *)REG_A0, (LONG )REG_D0, (BYTE )REG_D1);
}

void  LIB_VAT_NewShowRegUtil(void)
{
	VAT_NewShowRegUtil((APTR )REG_A0, (STRPTR )REG_A1);
}

int  LIB_VAT_IsAmigaGuideFile(void)
{
	return (int )VAT_IsAmigaGuideFile((STRPTR )REG_A0);
}

LONG  LIB_VAT_VFPrintfAsync(void)
{
	return (LONG )VAT_VFPrintfAsync((struct AsyncFile *)REG_A0, (STRPTR )REG_A1, (APTR )REG_A2);
}

void  LIB_VAT_MPZPow(void)
{
	VAT_MPZPow((APTR )REG_A0, (APTR )REG_A1, (APTR )REG_A2, (APTR )REG_A3);
}

APTR  LIB_VAT_AllocPooled(void)
{
	return (APTR )VAT_AllocPooled((APTR )REG_A0, (ULONG )REG_D0);
}

int  LIB_VAT_CheckVATVersion(void)
{
	return (int )VAT_CheckVATVersion((ULONG )REG_D0);
}

time_t  LIB_VAT_Time(void)
{
	return (time_t )VAT_Time((time_t *)REG_A0);
}

void  LIB_VAT_MD5Final(void)
{
	VAT_MD5Final((APTR )REG_A0, (APTR )REG_A1);
}

struct AsyncFile * LIB_VAT_OpenAsync(void)
{
	return (struct AsyncFile *)VAT_OpenAsync((const STRPTR )REG_A0, (UBYTE )REG_D0, (LONG )REG_D1);
}

void  LIB_VAT_SetLastUsedDir(void)
{
	VAT_SetLastUsedDir((STRPTR )REG_A0);
}

int  LIB_VAT_Initialize(void)
{
	return (int )VAT_Initialize((STRPTR )REG_A0, (APTR *)REG_A1, (STRPTR )REG_A2, (ULONG )REG_D0);
}

LONG  LIB_VAT_CloseAsync(void)
{
	return (LONG )VAT_CloseAsync((struct AsyncFile *)REG_A0);
}

void  LIB_VAT_ShowURL(void)
{
	VAT_ShowURL((STRPTR )REG_A0, (STRPTR )REG_A1, (APTR )REG_A2);
}

void  LIB_VAT_SetFmtA(void)
{
	VAT_SetFmtA((APTR )REG_A0, (ULONG )REG_D0, (STRPTR )REG_A1, (APTR )REG_A2);
}

void  LIB_VAT_SetTxtFmtA(void)
{
	VAT_SetTxtFmtA((APTR )REG_A0, (STRPTR )REG_A1, (APTR )REG_A2);
}

LONG  LIB_VAT_FtellAsync(void)
{
	return (LONG )VAT_FtellAsync((struct AsyncFile *)REG_A0);
}

