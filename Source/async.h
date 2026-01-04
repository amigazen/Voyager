#ifndef ASYNC_H
#define ASYNC_H

/*****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif

/*****************************************************************************/

/* This structure is public only by necessity, don't muck with it yourself, or
 * you're looking for trouble
 */
struct AsyncFile
{
    BPTR                  af_File;
    ULONG                 af_BlockSize;
    struct MsgPort       *af_Handler;
    APTR                  af_Offset;
    LONG                  af_BytesLeft;
    ULONG                 af_BufferSize;
    APTR                  af_Buffers[2];
    struct StandardPacket af_Packet;
    struct MsgPort        af_PacketPort;
    ULONG                 af_CurrentBuf;
    ULONG                 af_SeekOffset;
    LONG                  af_CurrentFileOffset;
    ULONG                 af_WriteError;
    UBYTE                 af_PacketPending;
    UBYTE                 af_ReadMode;
};

/*****************************************************************************/

#define MODE_READ   0  /* read an existing file                             */
#define MODE_WRITE  1  /* create a new file, delete existing file if needed */
#define MODE_APPEND 2  /* append to end of existing file, or create new     */
#define MODE_SHAREDWRITE  3  /* create a new file, delete existing file if needed */

/*****************************************************************************/

struct AsyncFile *OpenAsync(const STRPTR fileName, UBYTE accessMode, LONG bufferSize);
LONG CloseAsync(struct AsyncFile *file);
LONG ReadAsync(struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG ReadCharAsync(struct AsyncFile *file);
LONG WriteAsync(struct AsyncFile *file, APTR buffer, LONG numBytes);
LONG WriteCharAsync(struct AsyncFile *file, UBYTE ch);
LONG SeekAsync(struct AsyncFile *file, LONG position, BYTE mode);
UBYTE *FGetsAsync(struct AsyncFile *file, UBYTE *buffer, int maxlen);
UBYTE *FGetsAsyncNoLF(struct AsyncFile *file, UBYTE *buffer, int maxlen);
LONG VFPrintfAsync(struct AsyncFile *file, char *string, APTR args);
LONG FPrintfAsync(struct AsyncFile *file, char *string, ...);
LONG FtellAsync(struct AsyncFile *file);
void UnGetCAsync(struct AsyncFile *file);
LONG GetFilesizeAsync(struct AsyncFile *file);

/*****************************************************************************/

/* Utility library functions - implementations based on tbLib */
APTR AllocVecPooled(APTR poolheader, ULONG memsize);
void FreeVecPooled(APTR poolheader, APTR memory);

/* VAT functions */
struct Screen *VAT_GetAppScreen(APTR app);
STRPTR VAT_GetAppScreenName(APTR app);
int VAT_SendRXMsg(STRPTR cmd, STRPTR basename, STRPTR suffix);

/*****************************************************************************/

#endif /* ASYNC_H */

