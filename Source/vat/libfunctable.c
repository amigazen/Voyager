/*
 * Library function table
 * ----------------------
 *
 * $Id: libfunctable.c,v 1.1 2001/03/04 08:33:59 zapek Exp $
 */

#if defined( __MORPHOS__ )

struct Screen;
struct AsyncFile;
struct Library;
struct VATS_URL;

#include "globals.h"
#include "lib.h"

#include <time.h> /* for time_t */

void LIB_Open( void );
void LIB_Close( void );
void LIB_Expunge( void );
void LIB_Reserved( void );

int LIB_VAT_Initialize( void );
void LIB_VAT_Cleanup( void );
struct Library * LIB_VAT_OpenLibrary( void );
struct Library * LIB_VAT_OpenLibraryCode( void );
int LIB_VAT_CheckVATVersion( void );
struct AsyncFile * LIB_VAT_OpenAsync( void );
LONG LIB_VAT_CloseAsync( void );
LONG LIB_VAT_ReadAsync( void );
LONG LIB_VAT_ReadCharAsync( void );
LONG LIB_VAT_SeekAsync( void );
UBYTE * LIB_VAT_FGetsAsync( void );
APTR LIB_VAT_AllocPooled( void );
APTR LIB_VAT_AllocVecPooled( void );
void LIB_VAT_CalcMD5( void );
void LIB_VAT_CheckEcrypt( void );
void LIB_VAT_CheckProgramInPathFull( void );
APTR LIB_VAT_CreatePool( void );
void LIB_VAT_DeletePool( void );
int LIB_VAT_ExpandTemplateA( void );
int LIB_VAT_FGets( void );
UBYTE * LIB_VAT_FGetsAsyncNoLF( void );
void LIB_VAT_FreePooled( void );
void LIB_VAT_FreeURLList( void );
void LIB_VAT_FreeVecPooled( void );
LONG LIB_VAT_FtellAsync( void );
struct Screen * LIB_VAT_GetAppScreen( void );
STRPTR LIB_VAT_GetAppScreenName( void );
int LIB_VAT_GetDataType( void );
LONG LIB_VAT_GetFilesizeAsync( void );
int LIB_VAT_IsAmigaGuideFile( void );
int LIB_VAT_IsOnline( void );
void LIB_VAT_MD5Final( void );
void LIB_VAT_MD5Init( void );
void LIB_VAT_MD5Update( void );
int LIB_VAT_MIME_FindByExtension( void );
int LIB_VAT_MIME_FindByType( void );
void LIB_VAT_MPZFree( void );
void LIB_VAT_MPZPow( void );
void LIB_VAT_MultiSetA( void );
void LIB_VAT_NewShowRegUtil( void );
ULONG LIB_VAT_Random( void );
ULONG LIB_VAT_RandomByte( void );
void LIB_VAT_RandomStir( void );
void LIB_VAT_RemSpaces( void );
struct VATS_URL * LIB_VAT_ScanForURLS( void );
int LIB_VAT_SendRXMsg( void );
void LIB_VAT_SetFmtA( void );
void LIB_VAT_SetLastUsedDir( void );
void LIB_VAT_SetTxtFmtA( void );
void LIB_VAT_ShowRegUtil( void );
void LIB_VAT_ShowURL( void );
STRPTR LIB_VAT_StrDupPooled( void );
time_t LIB_VAT_Time( void );
time_t LIB_VAT_Timev( void );
void LIB_VAT_UnGetCAsync( void );
LONG LIB_VAT_VFPrintfAsync( void );
LONG LIB_VAT_WriteAsync( void );
LONG LIB_VAT_WriteCharAsync( void );

ULONG LibFuncTable[] =
{
	FUNCARRAY_32BIT_NATIVE,
	( ULONG )&LIB_Open,
	( ULONG )&LIB_Close,
	( ULONG )&LIB_Expunge,
	( ULONG )&LIB_Reserved,
	( ULONG )&LIB_VAT_Initialize,
	( ULONG )&LIB_VAT_Cleanup,
	( ULONG )&LIB_VAT_OpenLibrary,
	( ULONG )&LIB_VAT_OpenLibraryCode,
	( ULONG )&LIB_VAT_CheckVATVersion,
	( ULONG )&LIB_VAT_OpenAsync,
	( ULONG )&LIB_VAT_CloseAsync,
	( ULONG )&LIB_VAT_ReadAsync,
	( ULONG )&LIB_VAT_ReadCharAsync,
	( ULONG )&LIB_VAT_WriteAsync,
	( ULONG )&LIB_VAT_WriteCharAsync,
	( ULONG )&LIB_VAT_SeekAsync,
	( ULONG )&LIB_VAT_FGetsAsync,
	( ULONG )&LIB_VAT_VFPrintfAsync,
	( ULONG )&LIB_VAT_FtellAsync,
	( ULONG )&LIB_VAT_UnGetCAsync,
	( ULONG )&LIB_VAT_FGetsAsyncNoLF,
	( ULONG )&LIB_VAT_CreatePool,
	( ULONG )&LIB_VAT_DeletePool,
	( ULONG )&LIB_VAT_AllocPooled,
	( ULONG )&LIB_VAT_FreePooled,
	( ULONG )&LIB_VAT_AllocVecPooled,
	( ULONG )&LIB_VAT_FreeVecPooled,
	( ULONG )&LIB_VAT_StrDupPooled,
	( ULONG )&LIB_VAT_CalcMD5,
	( ULONG )&LIB_VAT_MD5Init,
	( ULONG )&LIB_VAT_MD5Final,
	( ULONG )&LIB_VAT_MD5Update,
	( ULONG )&LIB_VAT_RandomStir,
	( ULONG )&LIB_VAT_Random,
	( ULONG )&LIB_VAT_RandomByte,
	( ULONG )&LIB_VAT_MPZPow,
	( ULONG )&LIB_VAT_MPZFree,
	( ULONG )&LIB_VAT_ExpandTemplateA,
	( ULONG )&LIB_VAT_RemSpaces,
	( ULONG )&LIB_VAT_Time,
	( ULONG )&LIB_VAT_Timev,
	( ULONG )&LIB_VAT_SetLastUsedDir,
	( ULONG )&LIB_VAT_CheckProgramInPathFull,
	( ULONG )&LIB_VAT_SendRXMsg,
	( ULONG )&LIB_VAT_ShowURL,
	( ULONG )&LIB_VAT_GetFilesizeAsync,
	( ULONG )&LIB_VAT_ScanForURLS,
	( ULONG )&LIB_VAT_FreeURLList,
	( ULONG )&LIB_VAT_MultiSetA,
	( ULONG )&LIB_VAT_SetFmtA,
	( ULONG )&LIB_VAT_SetTxtFmtA,
	( ULONG )&LIB_VAT_GetAppScreen,
	( ULONG )&LIB_VAT_ShowRegUtil,
	( ULONG )&LIB_VAT_GetDataType,
	( ULONG )&LIB_VAT_IsAmigaGuideFile,
	( ULONG )&LIB_VAT_MIME_FindByExtension,
	( ULONG )&LIB_VAT_MIME_FindByType,
	( ULONG )&LIB_VAT_GetAppScreenName,
	( ULONG )&LIB_VAT_FGets,
	( ULONG )&LIB_VAT_NewShowRegUtil,
	( ULONG )&LIB_VAT_IsOnline,
	( ULONG )&LIB_VAT_CheckEcrypt,
	0xffffffff
};

#endif /* __MORPHOS__ */
