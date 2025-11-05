#ifndef CLIB_VAT_PROTOS_H
#define CLIB_VAT_PROTOS_H

struct Screen;
struct VATS_URL;
struct AsyncFile;


/*
 * Vapor Toolkit protos
 * --------------------
 *
 * © 2000-2001 by VaporWare
 *
 * $Id: vat_protos.h,v 1.1.1.1 2001/01/21 19:33:05 zapek Exp $
 *
 */

#include <libraries/vat.h>

/* Init functions */
int VAT_Initialize( STRPTR appid, APTR *libvec, STRPTR ctypevec, ULONG requirements );
void VAT_Cleanup( APTR *libvec );
int VAT_CheckVATVersion( ULONG version );
struct Library *VAT_OpenLibrary( STRPTR libname, ULONG libversion );
struct Library *VAT_OpenLibraryCode( ULONG libcode );

/* Async I/O */
struct AsyncFile *VAT_OpenAsync(const STRPTR fileName, UBYTE accessMode, LONG bufferSize);
LONG VAT_CloseAsync(struct AsyncFile *file);
LONG VAT_ReadAsync(struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG VAT_ReadCharAsync(struct AsyncFile *file);
LONG VAT_WriteAsync(struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG VAT_WriteCharAsync(struct AsyncFile *file, UBYTE ch);
LONG VAT_SeekAsync(struct AsyncFile *file, LONG position, BYTE mode);
UBYTE *VAT_FGetsAsync( struct AsyncFile *, UBYTE *, LONG );
UBYTE *VAT_FGetsAsyncNoLF( struct AsyncFile *, UBYTE *, LONG );
LONG VAT_VFPrintfAsync( struct AsyncFile *, STRPTR, APTR  );
LONG VAT_FPrintfAsync( struct AsyncFile *, STRPTR, ... );
LONG VAT_FtellAsync( struct AsyncFile * );
void VAT_UnGetCAsync( struct AsyncFile *file );
LONG VAT_GetFilesizeAsync( struct AsyncFile *file );

/* Pools */
APTR VAT_CreatePool( ULONG, ULONG, ULONG );
void VAT_DeletePool( APTR  );
APTR VAT_AllocPooled( APTR , ULONG );
APTR VAT_AllocVecPooled( APTR , ULONG );
void VAT_FreePooled( APTR , APTR , ULONG );
void VAT_FreeVecPooled( APTR , APTR  );
STRPTR VAT_StrDupPooled( APTR , STRPTR  );

/* Crypto */
void VAT_CalcMD5( APTR in, ULONG inlen, APTR out );
void VAT_MD5Init( APTR ctx );
void VAT_MD5Final( APTR digest, APTR ctx );
void VAT_MD5Update( APTR ctx, APTR data, ULONG datalen );
void VAT_RandomStir( void );
ULONG VAT_Random( void );
ULONG VAT_RandomByte( void );
void VAT_MPZPow( APTR od, APTR id, APTR pn, APTR pe );
void VAT_MPZFree( APTR od );

/* Strings */
int VAT_ExpandTemplateA( STRPTR from, STRPTR to, ULONG tosize, APTR args );
int VAT_ExpandTemplate( STRPTR from, STRPTR to, ULONG tosize, ... );
void VAT_RemSpaces( STRPTR str );

/* Time */
time_t VAT_Time( time_t *tptr );
time_t VAT_Timev( void );

/* Misc (oh well, it's a mess from here) */
void VAT_SetLastUsedDir( STRPTR appid );
int VAT_CheckProgramInPathFull( STRPTR name, STRPTR fullpath );
int VAT_SendRXMsg( STRPTR cmd, STRPTR basename, STRPTR suffix );
void VAT_ShowURL( STRPTR url, STRPTR rexxsuffix, APTR muiapp );

struct VATS_URL *VAT_ScanForURLS( STRPTR str );
void VAT_FreeURLList( struct VATS_URL *list );

void VAT_MultiSetA( ULONG attr, LONG val, APTR *objlist );
void VAT_MultiSet( ULONG attr, LONG val, ... );

void VAT_SetFmtA( APTR obj, ULONG attr, STRPTR fmt, APTR args );
void VAT_SetFmt( APTR obj, ULONG attr, STRPTR fmt, ... );
void VAT_SetTxtFmtA( APTR obj, STRPTR fmt, APTR args );
void VAT_SetTxtFmt( APTR obj, STRPTR fmt, ... );

struct Screen * VAT_GetAppScreen( APTR app );

void VAT_ShowRegUtil( struct Screen *scr );

int VAT_GetDataType( STRPTR filename, ULONG *gid, ULONG *id, STRPTR namebuff );
int VAT_IsAmigaGuideFile( STRPTR filename );

int VAT_MIME_FindByExtension(
	char *filename,
	char *savedir,
	char *viewer,
	int *viewmode,
	char *mimetype
);
int VAT_MIME_FindByType(
	char *mimetype,
	char *savedir,
	char *viewer,
	int *viewmode
);

STRPTR VAT_GetAppScreenName( APTR app );

int VAT_FGets( BPTR fh, STRPTR buffer, int size );

void VAT_NewShowRegUtil( APTR muiapp, STRPTR prodname );

int VAT_IsOnline( void );

void VAT_CheckEcrypt( time_t when );

#endif /* !CLIB_VAT_PROTOS_H */
