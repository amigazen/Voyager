#pragma pack(2)
#pragma pack(2)
#ifndef VAT_LIB_H
#define VAT_LIB_H
/*
 * Vapor Toolkit
 * -------------
 *
 * $Id: vat.h,v 1.5 2001/10/03 22:17:13 zapek Exp $
 */

#pragma pack()
#pragma pack()
#include <dos/dos.h>
#pragma pack(2)
#pragma pack(2)
#pragma pack()
#pragma pack()
#include <dos/dosextens.h>
#pragma pack(2)
#pragma pack(2)
#pragma pack()
#pragma pack()
#include <time.h>
#pragma pack(2)
#pragma pack(2)

#define VAT_VERSION 15

/*
 * Tool macros
 */
#define MYPROC ((struct Process*)(SysBase->ThisTask))

/*
 * Requirements for VAT_Initialize()
 */
#define VATIR_OS3 1
#define VATIR_020 2
#define VATIR_FPU 4

/*
 * Libvec: REMOVED
 *
 * People using Libvec must die.
 * If one of you does that he'll
 * be forced to eat a foam bat.
 */

/*
 * OpenLibraryCode() codes
 */
enum {
	VATOC_GFX,
	VATOC_UTIL,
	VATOC_WB,
	VATOC_ICON,
	VATOC_COMMODITIES,
	VATOC_LAYERS,
	VATOC_IFFPARSE,
	VATOC_CYBERGRAPHICS,
	VATOC_DATATYPES,
	VATOC_DISKFONT,
	VATOC_REXXSYS,
	VATOC_ASL,
	VATOC_INTUITION,
	VATOC_MATHTRANS,
	VATOC_MATHFFP,
	VATOC_MATHIEEEDOUBTRANS,
	VATOC_MATHIEEEDOUBBAS,
	VATOC_VAPOR_UPDATE
};

/*
 * URL finder
 */
struct VATS_URL {
	struct VATS_URL *next;
	APTR private;
	ULONG offset;
	ULONG len;
	char url[ 0 ]; // expanded dynamically
};

/*
 * MIME stuff
 */
enum {
	MT_ACTION_ASK,
	MT_ACTION_SAVE,
	MT_ACTION_VIEW,
	MT_ACTION_SAVE_AND_VIEW
};

#define MF_PIPE_STREAM 256
#define MF_VIEW_INLINE 512


/*
 * Asyncio stuff
 */
struct AsyncFile
{
	BPTR                  af_File;
	ULONG                 af_BlockSize;
	struct MsgPort       *af_Handler;
	APTR                  af_Offset;
	LONG                  af_BytesLeft;
	ULONG             af_BufferSize;
	APTR              af_Buffers[2];
	struct StandardPacket af_Packet;
	struct MsgPort        af_PacketPort;
	ULONG                 af_CurrentBuf;
	ULONG                 af_SeekOffset;
	LONG                 af_CurrentFileOffset;
	ULONG                 af_WriteError;
	UBYTE             af_PacketPending;
	UBYTE             af_ReadMode;
};

#define MODE_READ   0  /* read an existing file                             */
#define MODE_WRITE  1  /* create a new file, delete existing file if needed */
#define MODE_APPEND 2  /* append to end of existing file, or create new     */
#define MODE_SHAREDWRITE  3  /* create a new file, delete existing file if needed */

#ifndef NO_VAT_SHORTCUTS

#define OpenAsync VAT_OpenAsync
#define CloseAsync VAT_CloseAsync
#define ReadAsync VAT_ReadAsync
#define WriteAsync VAT_WriteAsync
#define ReadCharAsync VAT_ReadCharAsync
#define WriteCharAsync VAT_WriteCharAsync
#define SeekAsync VAT_SeekAsync
#define FGetsAsync VAT_FGetsAsync
#define FGetsAsyncNoLF VAT_FGetsAsyncNoLF
#define FPrintfAsync VAT_FPrintfAsync
#define FtellAsync VAT_FtellAsync
#define UnGetCAsync VAT_UnGetCAsync
#define GetFilesizeAsync VAT_GetFilesizeAsync

#undef CreatePool
#define CreatePool VAT_CreatePool
#undef DeletePool
#define DeletePool VAT_DeletePool
#undef AllocPooled
#define AllocPooled VAT_AllocPooled
#undef FreePooled
#define FreePooled VAT_FreePooled
#undef AllocVecPooled
#define AllocVecPooled VAT_AllocVecPooled
#undef FreeVecPooled
#define FreeVecPooled VAT_FreeVecPooled
#undef StrDupPooled
#define StrDupPooled VAT_StrDupPooled

#endif /* !NO_VAT_SHORTCUTS */

#endif /* !VAT_LIB_H */

#pragma pack()
#pragma pack()
