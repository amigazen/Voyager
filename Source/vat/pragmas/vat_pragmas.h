#ifndef VAT_PRAGMAS_H
#define VAT_PRAGMAS_H
/*
 * $Id: vat_pragmas.h,v 1.2 2001/03/04 08:34:03 zapek Exp $
 */

#pragma libcall VATBase VAT_Initialize 1e 0A9804
#pragma libcall VATBase VAT_Cleanup 24 801
#pragma libcall VATBase VAT_OpenLibrary 2a 0802
#pragma libcall VATBase VAT_OpenLibraryCode 30 001
#pragma libcall VATBase VAT_CheckVATVersion 36 001
#pragma libcall VATBase VAT_OpenAsync 3c 10803
#pragma libcall VATBase VAT_CloseAsync 42 801
#pragma libcall VATBase VAT_ReadAsync 48 09803
#pragma libcall VATBase VAT_ReadCharAsync 4e 801
#pragma libcall VATBase VAT_WriteAsync 54 09803
#pragma libcall VATBase VAT_WriteCharAsync 5a 0802
#pragma libcall VATBase VAT_SeekAsync 60 10803
#pragma libcall VATBase VAT_FGetsAsync 66 09803
#pragma libcall VATBase VAT_VFPrintfAsync 6c A9803
#pragma tagcall VATBase VAT_FPrintfAsync 6c A9803
#pragma libcall VATBase VAT_FtellAsync 72 801
#pragma libcall VATBase VAT_UnGetCAsync 78 801
#pragma libcall VATBase VAT_FGetsAsyncNoLF 7e 09803
#pragma libcall VATBase VAT_CreatePool 84 21003
#pragma libcall VATBase VAT_DeletePool 8a 801
#pragma libcall VATBase VAT_AllocPooled 90 0802
#pragma libcall VATBase VAT_FreePooled 96 09803
#pragma libcall VATBase VAT_AllocVecPooled 9c 0802
#pragma libcall VATBase VAT_FreeVecPooled a2 9802
#pragma libcall VATBase VAT_StrDupPooled a8 9802
#pragma libcall VATBase VAT_CalcMD5 ae 90803
#pragma libcall VATBase VAT_MD5Init b4 801
#pragma libcall VATBase VAT_MD5Final ba 9802
#pragma libcall VATBase VAT_MD5Update c0 09803
#pragma libcall VATBase VAT_RandomStir c6 0
#pragma libcall VATBase VAT_Random cc 0
#pragma libcall VATBase VAT_RandomByte d2 0
#pragma libcall VATBase VAT_MPZPow d8 BA9804
#pragma libcall VATBase VAT_MPZFree de 801
#pragma libcall VATBase VAT_ExpandTemplateA e4 A09804
#pragma tagcall VATBase VAT_ExpandTemplate e4 A09804
#pragma libcall VATBase VAT_RemSpaces ea 801
#pragma libcall VATBase VAT_Time f0 801
#pragma libcall VATBase VAT_Timev f6 0
#pragma libcall VATBase VAT_SetLastUsedDir fc 801
#pragma libcall VATBase VAT_CheckProgramInPathFull 102 9802
#pragma libcall VATBase VAT_SendRXMsg 108 A9803
#pragma libcall VATBase VAT_ShowURL 10e A9803
#pragma libcall VATBase VAT_GetFilesizeAsync 114 801
#pragma libcall VATBase VAT_ScanForURLS 11a 801
#pragma libcall VATBase VAT_FreeURLList 120 801
#pragma libcall VATBase VAT_MultiSetA 126 81003
#pragma tagcall VATBase VAT_MultiSet 126 81003
#pragma libcall VATBase VAT_SetFmtA 12c A90804
#pragma libcall VATBase VAT_SetTxtFmtA 132 A9803
#pragma tagcall VATBase VAT_SetFmt 12c A90804
#pragma tagcall VATBase VAT_SetTxtFmt 132 A9803
#pragma libcall VATBase VAT_GetAppScreen 138 801
#pragma libcall VATBase VAT_ShowRegUtil 13e 801

/* V8 additions*/
#pragma libcall VATBase VAT_GetDataType 144 BA9804
#pragma libcall VATBase VAT_IsAmigaGuideFile 14a 801
#pragma libcall VATBase VAT_MIME_FindByExtension 150 0BA9805
#pragma libcall VATBase VAT_MIME_FindByType 156 BA9804
#pragma libcall VATBase VAT_GetAppScreenName 15c 801
#pragma libcall VATBase VAT_FGets 162 09803

/* V11 additions*/
#pragma libcall VATBase VAT_NewShowRegUtil 168 9802

/* V12 additions*/
#pragma libcall VATBase VAT_IsOnline 16e 0
#pragma libcall VATBase VAT_CheckEcrypt 174 001

#endif /* !VAT_PRAGMAS_H */
