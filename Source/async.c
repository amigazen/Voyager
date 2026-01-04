#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#ifdef __SASC
#include <dos.h>
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/rexxsyslib.h>
#include <proto/muimaster.h>
#include <proto/graphics.h>
#include <intuition/intuitionbase.h>
#include <libraries/mui.h>
#include <rexx/storage.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "async.h"

/* External library bases */
extern struct Library *MUIMasterBase;

/* Utility library functions - implementations based on tbLib */
/* AllocVecPooled and FreeVecPooled implementations */
#define POOL_MAGIC 0xa0b18c9e

APTR AllocVecPooled(APTR poolheader, ULONG memsize)
{
	ULONG *result;
	ULONG totalsize;

	/* Allocate extra space for magic number and size */
	totalsize = memsize + sizeof(ULONG) * 2;
	result = (ULONG *)AllocPooled(poolheader, totalsize);
	
	if (result != NULL)
	{
		result[0] = POOL_MAGIC;
		result[1] = totalsize;
		return (APTR)&result[2];
	}
	else
	{
		return NULL;
	}
}

void FreeVecPooled(APTR poolheader, APTR memory)
{
	ULONG *tmp;

	if (memory != NULL)
	{
		tmp = (ULONG *)memory;
		/* Check magic number and free the actual allocation */
		if (tmp[-2] == POOL_MAGIC && tmp[-1] != 0)
		{
			FreePooled(poolheader, (APTR)&tmp[-2], tmp[-1]);
		}
	}
}

/*****************************************************************************/

/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);

/*****************************************************************************/

/* send out an async packet to the file system. */
static void SendPacket(struct AsyncFile *file, APTR arg2)
{
	file->af_Packet.sp_Pkt.dp_Port = &file->af_PacketPort;
	file->af_Packet.sp_Pkt.dp_Arg2 = (LONG)arg2;
	PutMsg(file->af_Handler, &file->af_Packet.sp_Msg);
	file->af_PacketPending = TRUE;
}

/*****************************************************************************/

/* this function waits for a packet to come back from the file system. If no
 * packet is pending, state from the previous packet is returned. This ensures
 * that once an error occurs, it state is maintained for the rest of the life
 * of the file handle.
 *
 * This function also deals with IO errors, bringing up the needed DOS
 * requesters to let the user retry an operation or cancel it.
 */
static LONG WaitPacket(struct AsyncFile *file)
{
	LONG bytes;

	if (file->af_PacketPending)
	{
		while (TRUE)
		{
			/* This enables signalling when a packet comes back to the port */
			file->af_PacketPort.mp_Flags = PA_SIGNAL;

			/* Wait for the packet to come back, and remove it from the message
			 * list. Since we know no other packets can come in to the port, we can
			 * safely use Remove() instead of GetMsg(). If other packets could come in,
			 * we would have to use GetMsg(), which correctly arbitrates access in such
			 * a case
			 */
			Remove((struct Node *)WaitPort(&file->af_PacketPort));

			/* set the port type back to PA_IGNORE so we won't be bothered with
			 * spurious signals
			 */
			file->af_PacketPort.mp_Flags = PA_IGNORE;

			/* mark packet as no longer pending since we removed it */
			file->af_PacketPending = FALSE;

			bytes = file->af_Packet.sp_Pkt.dp_Res1;
			if (bytes >= 0)
			{
				/* packet didn't report an error, so bye... */
				return(bytes);
			}

			/* see if the user wants to try again... */
			if (ErrorReport(file->af_Packet.sp_Pkt.dp_Res2,REPORT_STREAM,file->af_File,NULL))
				return(-1);

			/* user wants to try again, resend the packet */
			if (file->af_ReadMode)
				SendPacket(file,file->af_Buffers[file->af_CurrentBuf]);
			else
				SendPacket(file,file->af_Buffers[1 - file->af_CurrentBuf]);
		}
	}

	/* last packet's error code, or 0 if packet was never sent */
	SetIoErr(file->af_Packet.sp_Pkt.dp_Res2);

	return(file->af_Packet.sp_Pkt.dp_Res1);
}

/*****************************************************************************/

/* this function puts the packet back on the message list of our
 * message port.
 */
static void RequeuePacket(struct AsyncFile *file)
{
	AddHead(&file->af_PacketPort.mp_MsgList,&file->af_Packet.sp_Msg.mn_Node);
	file->af_PacketPending = TRUE;
}

/*****************************************************************************/

/* this function records a failure from a synchronous DOS call into the
 * packet so that it gets picked up by the other IO routines in this module
 */
static void RecordSyncFailure(struct AsyncFile *file)
{
	file->af_Packet.sp_Pkt.dp_Res1 = -1;
	file->af_Packet.sp_Pkt.dp_Res2 = IoErr();
}

/*****************************************************************************/

struct AsyncFile *OpenAsync(const STRPTR fileName, UBYTE accessMode, LONG bufferSize)
{
	struct AsyncFile  *file;
	struct FileHandle *fh;
	BPTR               handle;
	BPTR               lock;
	LONG               blockSize;
	ULONG              startOffset;
	D_S(struct InfoData,infoData);

	handle = NULL;
	file   = NULL;
	lock   = NULL;
	startOffset = 0;

	if (accessMode == MODE_READ)
	{
		if (handle = Open(fileName,MODE_OLDFILE))
			lock = Lock(fileName,ACCESS_READ);
	}
	else
	{
		if (accessMode == MODE_WRITE)
		{
			handle = Open(fileName,MODE_NEWFILE);
		}
		else if (accessMode == MODE_SHAREDWRITE)
		{
			handle = Open(fileName,MODE_NEWFILE);
			if( handle )
			{
				Close( handle );
				handle = Open( fileName, MODE_OLDFILE );
			}
		}
		else if (accessMode == MODE_APPEND)
		{
			/* in append mode, we open for writing, and then seek to the
			 * end of the file. That way, the initial write will happen at
			 * the end of the file, thus extending it
			 */

			if (handle = Open(fileName,MODE_READWRITE))
			{
				if (Seek(handle,0,OFFSET_END) < 0)
				{
					Close(handle);
					handle = NULL;
				}
				/* Obtain real offset */
				startOffset = Seek( handle, 0, OFFSET_CURRENT );
			}
		}

		/* we want a lock on the same device as where the file is. We can't
		 * use DupLockFromFH() for a write-mode file though. So we get sneaky
		 * and get a lock on the parent of the file
		 */
		if (handle)
			lock = ParentOfFH(handle);
	}

	if (handle)
	{
		/* if it was possible to obtain a lock on the same device as the
		 * file we're working on, get the block size of that device and
		 * round up our buffer size to be a multiple of the block size.
		 * This maximizes DMA efficiency.
		 */

		blockSize = 512;
		if (lock)
		{
			if (Info(lock,infoData))
			{
				blockSize = infoData->id_BytesPerBlock;
				bufferSize = (((bufferSize + blockSize - 1) / blockSize) * blockSize) * 2;
			}
			UnLock(lock);
		}
reallocate:

		/* now allocate the ASyncFile structure, as well as the read buffers.
		 * Add 15 bytes to the total size in order to allow for later
		 * quad-longword alignement of the buffers
		 */

		if (file = AllocVec(sizeof(struct AsyncFile) + bufferSize + 15,MEMF_ANY))
		{
			file->af_File      = handle;
			file->af_ReadMode  = (accessMode == MODE_READ);
			file->af_BlockSize = blockSize;
			file->af_CurrentFileOffset = startOffset;

			file->af_WriteError = FALSE;

			/* initialize the ASyncFile structure. We do as much as we can here,
			 * in order to avoid doing it in more critical sections
			 *
			 * Note how the two buffers used are quad-longword aligned. This
			 * helps performance on 68040 systems with copyback cache. Aligning
			 * the data avoids a nasty side-effect of the 040 caches on DMA.
			 * Not aligning the data causes the device driver to have to do
			 * some magic to avoid the cache problem. This magic will generally
			 * involve flushing the CPU caches. This is very costly on an 040.
			 * Aligning things avoids the need for magic, at the cost of at
			 * most 15 bytes of ram.
			 */

			fh                     = BADDR(file->af_File);
			file->af_Handler       = fh->fh_Type;
			file->af_BufferSize    = bufferSize / 2;
			file->af_Buffers[0]    = (APTR)(((ULONG)file + sizeof(struct AsyncFile) + 15) & 0xfffffff0);
			file->af_Buffers[1]    = (APTR)((ULONG)file->af_Buffers[0] + file->af_BufferSize);
			file->af_Offset        = file->af_Buffers[0];
			file->af_CurrentBuf    = 0;
			file->af_SeekOffset    = 0;
			file->af_PacketPending = FALSE;

			/* this is the port used to get the packets we send out back.
			 * It is initialized to PA_IGNORE, which means that no signal is
			 * generated when a message comes in to the port. The signal bit
			 * number is initialized to SIGB_SINGLE, which is the special bit
			 * that can be used for one-shot signalling. The signal will never
			 * be set, since the port is of type PA_IGNORE. We'll change the
			 * type of the port later on to PA_SIGNAL whenever we need to wait
			 * for a message to come in.
			 *
			 * The trick used here avoids the need to allocate an extra signal
			 * bit for the port. It is quite efficient.
			 */

			file->af_PacketPort.mp_MsgList.lh_Head     = (struct Node *)&file->af_PacketPort.mp_MsgList.lh_Tail;
			file->af_PacketPort.mp_MsgList.lh_Tail     = NULL;
			file->af_PacketPort.mp_MsgList.lh_TailPred = (struct Node *)&file->af_PacketPort.mp_MsgList.lh_Head;
			file->af_PacketPort.mp_Node.ln_Type        = NT_MSGPORT;
			file->af_PacketPort.mp_Flags               = PA_IGNORE;
			file->af_PacketPort.mp_SigBit              = SIGB_SINGLE;
			file->af_PacketPort.mp_SigTask             = FindTask(NULL);

			file->af_Packet.sp_Pkt.dp_Link          = &file->af_Packet.sp_Msg;
			file->af_Packet.sp_Pkt.dp_Arg1          = fh->fh_Arg1;
			file->af_Packet.sp_Pkt.dp_Arg3          = file->af_BufferSize;
			file->af_Packet.sp_Pkt.dp_Res1          = 0;
			file->af_Packet.sp_Pkt.dp_Res2          = 0;
			file->af_Packet.sp_Msg.mn_Node.ln_Name  = (STRPTR)&file->af_Packet.sp_Pkt;
			file->af_Packet.sp_Msg.mn_Node.ln_Type  = NT_MESSAGE;
			file->af_Packet.sp_Msg.mn_Length        = sizeof(struct StandardPacket);

			if (accessMode == MODE_READ)
			{
				/* if we are in read mode, send out the first read packet to
				 * the file system. While the application is getting ready to
				 * read data, the file system will happily fill in this buffer
				 * with DMA transfers, so that by the time the application
				 * needs the data, it will be in the buffer waiting
				 */

				file->af_Packet.sp_Pkt.dp_Type = ACTION_READ;
				file->af_BytesLeft             = 0;
				if (file->af_Handler)
					SendPacket(file,file->af_Buffers[0]);
			}
			else
			{
				file->af_Packet.sp_Pkt.dp_Type = ACTION_WRITE;
				file->af_BytesLeft             = file->af_BufferSize;
			}
		}
		else
		{
			if( bufferSize >= ( blockSize * 4 ) )
			{
				bufferSize /= 2;
				goto reallocate;
			}
			Close(handle);
		}
	}

	return(file);
}

/*****************************************************************************/

LONG CloseAsync(struct AsyncFile *file)
{
	LONG result;

	if (file)
	{
		result = WaitPacket(file);
		if (result >= 0)
		{
			if (!file->af_ReadMode)
			{
				if( file->af_WriteError )
				{
					result = -1;
				}
				else
				{
					/* this will flush out any pending data in the write buffer */
					if (file->af_BufferSize > file->af_BytesLeft)
						result = Write(file->af_File,file->af_Buffers[file->af_CurrentBuf],file->af_BufferSize - file->af_BytesLeft);
				}
			}
		}

		Close(file->af_File);
		FreeVec(file);
	}
	else
	{
		SetIoErr(ERROR_INVALID_LOCK);
		result = -1;
	}

	return(result);
}

/*****************************************************************************/

LONG ReadAsync(struct AsyncFile *file, APTR buffer, LONG numBytes)
{
	LONG totalBytes;
	LONG bytesArrived;

	totalBytes = 0;

	/* if we need more bytes than there are in the current buffer, enter the
	 * read loop
	 */

	while (numBytes > file->af_BytesLeft)
	{
		/* drain buffer */
		CopyMem(file->af_Offset,buffer,file->af_BytesLeft);

		numBytes           -= file->af_BytesLeft;
		buffer              = (APTR)((ULONG)buffer + file->af_BytesLeft);
		totalBytes         += file->af_BytesLeft;
		file->af_BytesLeft  = 0;

		bytesArrived = WaitPacket(file);
		if (bytesArrived <= 0)
		{
			if (bytesArrived == 0)
			{
				file->af_CurrentFileOffset += totalBytes;
				return(totalBytes);
			}

			return(-1);
		}

		/* ask that the buffer be filled */
		SendPacket(file,file->af_Buffers[1-file->af_CurrentBuf]);

		if (file->af_SeekOffset > bytesArrived)
			file->af_SeekOffset = bytesArrived;

		file->af_Offset      = (APTR)((ULONG)file->af_Buffers[file->af_CurrentBuf] + file->af_SeekOffset);
		file->af_CurrentBuf  = 1 - file->af_CurrentBuf;
		file->af_BytesLeft   = bytesArrived - file->af_SeekOffset;
		file->af_SeekOffset  = 0;
	}

	CopyMem(file->af_Offset,buffer,numBytes);
	file->af_BytesLeft -= numBytes;
	file->af_Offset     = (APTR)((ULONG)file->af_Offset + numBytes);

	file->af_CurrentFileOffset += totalBytes + numBytes;
	return (totalBytes + numBytes);
}

/*****************************************************************************/

static LONG local_ReadCharAsync(struct AsyncFile *file)
{
	unsigned char ch;

	if (file->af_BytesLeft)
	{
		/* if there is at least a byte left in the current buffer, get it
		 * directly. Also update all counters
		 */

		ch = *(char *)file->af_Offset;
		file->af_BytesLeft--;
		file->af_Offset = (APTR)((ULONG)file->af_Offset + 1);
		file->af_CurrentFileOffset++;

		return((LONG)ch);
	}

	/* there were no characters in the current buffer, so call the main read
	 * routine. This has the effect of sending a request to the file system to
	 * have the current buffer refilled. After that request is done, the
	 * character is extracted for the alternate buffer, which at that point
	 * becomes the "current" buffer
	 */

	if (ReadAsync(file,&ch,1) > 0)
		return((LONG)ch);

	/* We couldn't read above, so fail */

	return(-1);
}

/*****************************************************************************/

LONG ReadCharAsync(struct AsyncFile *file)
{
	unsigned char ch;

	if (file->af_BytesLeft)
	{
		/* if there is at least a byte left in the current buffer, get it
		 * directly. Also update all counters
		 */

		ch = *(char *)file->af_Offset;
		file->af_BytesLeft--;
		file->af_Offset = (APTR)((ULONG)file->af_Offset + 1);
		file->af_CurrentFileOffset++;

		return((LONG)ch);
	}

	/* there were no characters in the current buffer, so call the main read
	 * routine. This has the effect of sending a request to the file system to
	 * have the current buffer refilled. After that request is done, the
	 * character is extracted for the alternate buffer, which at that point
	 * becomes the "current" buffer
	 */

	if (ReadAsync(file,&ch,1) > 0)
		return((LONG)ch);

	/* We couldn't read above, so fail */

	return(-1);
}

/*****************************************************************************/

LONG WriteAsync(struct AsyncFile *file, APTR buffer, LONG numBytes)
{
	LONG totalBytes;

	totalBytes = 0;

	if( file->af_WriteError )
		return( -1 );

	while (numBytes > file->af_BytesLeft)
	{
		/* this takes care of NIL: */
		if (!file->af_Handler)
		{
			file->af_Offset    = file->af_Buffers[0];
			file->af_BytesLeft = file->af_BufferSize;
			return(numBytes);
		}

	if (file->af_BytesLeft)
	{
		CopyMem(buffer,file->af_Offset,file->af_BytesLeft);

		numBytes   -= file->af_BytesLeft;
		buffer      = (APTR)((ULONG)buffer + file->af_BytesLeft);
		totalBytes += file->af_BytesLeft;
	}

	if (WaitPacket(file) < 0)
	{
		file->af_WriteError = ( ( struct Process * ) FindTask( NULL ) ) -> pr_Result2;
		return(-1);
	}

	/* send the current buffer out to disk */
	SendPacket(file,file->af_Buffers[file->af_CurrentBuf]);

	file->af_CurrentBuf = 1 - file->af_CurrentBuf;
	file->af_Offset     = file->af_Buffers[file->af_CurrentBuf];
	file->af_BytesLeft  = file->af_BufferSize;
	}

	CopyMem(buffer,file->af_Offset,numBytes);
	file->af_BytesLeft -= numBytes;
	file->af_Offset     = (APTR)((ULONG)file->af_Offset + numBytes);

	file->af_CurrentFileOffset += totalBytes + numBytes;
	return (totalBytes + numBytes);
}

/*****************************************************************************/

LONG WriteCharAsync(struct AsyncFile *file, UBYTE ch)
{
	UBYTE ch2;

	if (file->af_BytesLeft)
	{
		/* if there's any room left in the current buffer, directly write
		 * the byte into it, updating counters and stuff.
		 */

		*( (UBYTE *)file->af_Offset ) = ch;
		file->af_BytesLeft--;
		file->af_Offset = (APTR)(((ULONG)file->af_Offset) + 1);

		/* one byte written */
		file->af_CurrentFileOffset++;
		return(1);
	}

	/* there was no room in the current buffer, so call the main write
	 * routine. This will effectively send the current buffer out to disk,
	 * wait for the other buffer to come back, and then put the byte into
	 * it.
	 */

	ch2 = ch;
	return(WriteAsync(file,&ch2,1));
}

/*****************************************************************************/

LONG SeekAsync(struct AsyncFile *file, LONG position, BYTE mode)
{
	LONG  current;
	LONG  target;
	LONG  minBuf;
	LONG  maxBuf;
	LONG  bytesArrived;
	LONG  diff;
	LONG  filePos;
	LONG  roundTarget;
	D_S(struct FileInfoBlock,fib);

	/* optimized Special cases */
	switch( mode )
	{
		case OFFSET_CURRENT:
			if( !position )
				return( file->af_CurrentFileOffset );
			break;

		case OFFSET_BEGINNING:
			if( position == file->af_CurrentFileOffset )
				return( file->af_CurrentFileOffset );
			break;
	}

	bytesArrived = WaitPacket(file);

	if (bytesArrived < 0)
		return(-1);

	if (file->af_ReadMode)
	{
		/* figure out what the actual file position is */
		filePos = Seek(file->af_File,OFFSET_CURRENT,0);
		if (filePos < 0)
		{
			RecordSyncFailure(file);
			return(-1);
		}

		/* figure out what the caller's file position is */
		current = filePos - (file->af_BytesLeft+bytesArrived) + file->af_SeekOffset;
		file->af_SeekOffset = 0;

		/* figure out the absolute offset within the file where we must seek to */
		if (mode == OFFSET_CURRENT)
		{
			target = current + position;
		}
		else if (mode == OFFSET_BEGINNING)
		{
			target = position;
		}
		else /* if (mode == OFFSET_END) */
		{
			if (!ExamineFH(file->af_File,fib))
			{
				RecordSyncFailure(file);
				return(-1);
			}

			target = fib->fib_Size + position;
		}

		file->af_CurrentFileOffset = target;

		/* figure out what range of the file is currently in our buffers */
		minBuf = current - (LONG)((ULONG)file->af_Offset - (ULONG)file->af_Buffers[ 1 - file->af_CurrentBuf]);
		maxBuf = current + file->af_BytesLeft + bytesArrived;  /* WARNING: this is one too big */

		diff = target - current;

		if ((target < minBuf) || (target >= maxBuf))
		{
			/* the target seek location isn't currently in our buffers, so
			 * move the actual file pointer to the desired location, and then
			 * restart the async read thing...
			 */

			/* this is to keep our file reading block-aligned on the device.
			 * block-aligned reads are generally quite a bit faster, so it is
			 * worth the trouble to keep things aligned
			 */
			roundTarget = (target / file->af_BlockSize) * file->af_BlockSize;

			if (Seek(file->af_File,roundTarget-filePos,OFFSET_CURRENT) < 0)
			{
				RecordSyncFailure(file);
				return(-1);
			}

			SendPacket(file,file->af_Buffers[0]);

			file->af_SeekOffset = target-roundTarget;
			file->af_BytesLeft  = 0;
			file->af_CurrentBuf = 0;
			file->af_Offset     = file->af_Buffers[0];
		}
		else if ((target < current) || (diff <= file->af_BytesLeft))
		{
			/* one of the two following things is true:
			 *
			 * 1. The target seek location is within the current read buffer,
			 * but before the current location within the buffer. Move back
			 * within the buffer and pretend we never got the pending packet,
			 * just to make life easier, and faster, in the read routine.
			 *
			 * 2. The target seek location is ahead within the current
			 * read buffer. Advance to that location. As above, pretend to
			 * have never received the pending packet.
			 */

			RequeuePacket(file);

			file->af_BytesLeft -= diff;
			file->af_Offset     = (APTR)((ULONG)file->af_Offset + diff);
		}
		else
		{
			/* at this point, we know the target seek location is within
			 * the buffer filled in by the packet that we just received
			 * at the start of this function. Throw away all the bytes in the
			 * current buffer, send a packet out to get the async thing going
			 * again, readjust buffer pointers to the seek location, and return
			 * with a grin on your face... :-)
			 */

			diff -= file->af_BytesLeft;

			SendPacket(file,file->af_Buffers[1-file->af_CurrentBuf]);

			file->af_Offset    = (APTR)((ULONG)file->af_Buffers[file->af_CurrentBuf] + diff);
			file->af_BytesLeft = bytesArrived - diff;
			file->af_CurrentBuf = 1 - file->af_CurrentBuf;
		}
	}
	else
	{
		if (file->af_BufferSize > file->af_BytesLeft)
		{
			if (Write(file->af_File,file->af_Buffers[file->af_CurrentBuf],file->af_BufferSize - file->af_BytesLeft) < 0)
			{
				RecordSyncFailure(file);
				return(-1);
			}
		}

		/* this will unfortunately generally result in non block-aligned file
		 * access. We could be sneaky and try to resync our file pos at a
		 * later time, but we won't bother. Seeking in write-only files is
		 * relatively rare (except when writing IFF files with unknown chunk
		 * sizes, where the chunk size has to be written after the chunk data)
		 */

		current = Seek(file->af_File,position,mode);

		if (current < 0)
		{
			RecordSyncFailure(file);
			return(-1);
		}

		switch( mode )
		{
			case OFFSET_BEGINNING:
				file->af_CurrentFileOffset = position;
				break;

			case OFFSET_CURRENT:
				file->af_CurrentFileOffset = current + position;
				break;

			case OFFSET_END:
				file->af_CurrentFileOffset = Seek( file->af_File, 0, OFFSET_CURRENT ) + position;
				break;
		}

		file->af_BytesLeft  = file->af_BufferSize;
		file->af_CurrentBuf = 0;
		file->af_Offset     = file->af_Buffers[0];
	}

	return(current);
}

/*****************************************************************************/

UBYTE *FGetsAsync(struct AsyncFile *file, UBYTE *buffer, int maxlen)
{
	int ch;
	int len = 0;

	while( --maxlen )
	{
		ch = local_ReadCharAsync( file );
		if( ch < 0 )
			break;
		buffer[ len++ ] = ch;
		if( ch == 10 )
			break;
	}

	buffer[ len ] = 0;
	return( len ? buffer : NULL );
}

/*****************************************************************************/

UBYTE *FGetsAsyncNoLF(struct AsyncFile *file, UBYTE *buffer, int maxlen)
{
	int ch;
	int len = 0;

	while( --maxlen )
	{
		ch = local_ReadCharAsync( file );
		if( ch < 0 )
			break;
		if( ch == 13 )
			continue;
		if( ch == 10 )
			break;
		buffer[ len++ ] = ch;
	}

	buffer[ len ] = 0;
	return( len ? buffer : NULL );
}

/*****************************************************************************/

struct fpi
{
	ULONG a6;
	struct AsyncFile *to;
	int error;
	int count;
};

/* Callback for RawDoFmt - writes one character at a time */
/* RawDoFmt calls this with: buff = pointer to character, result = unused, userdata = fpi */
static void dofpi_callback(STRPTR buff, LONG *result, struct fpi *fpi)
{
	char ch;

	if (!buff || !fpi)
		return;

	ch = *buff;
	if (!ch)
		return;

	if (fpi->error)
		return;

	fpi->count++;

	if (WriteCharAsync(fpi->to, (UBYTE)ch) != 1)
	{
		fpi->error = 1;
	}
}

/*****************************************************************************/

LONG VFPrintfAsync(struct AsyncFile *file, char *string, APTR args)
{
	struct fpi fpi;

	fpi.error = fpi.count = 0;
	fpi.to = file;
	fpi.a6 = 0;

	RawDoFmt(string, args, (void (*)())dofpi_callback, &fpi);

	return(fpi.error ? -1 : fpi.count);
}

/*****************************************************************************/

LONG FPrintfAsync(struct AsyncFile *file, char *string, ...)
{
	APTR args;
	LONG result;

	args = (APTR)(&string + 1);
	result = VFPrintfAsync(file, string, args);

	return(result);
}

/*****************************************************************************/

LONG FtellAsync(struct AsyncFile *file)
{
	if( file )
		return( file->af_CurrentFileOffset );
	else
	{
		SetIoErr(ERROR_INVALID_LOCK);
		return( -1 );
	}
}

/*****************************************************************************/

void UnGetCAsync(struct AsyncFile *file)
{
	if( file->af_Offset != file->af_Buffers[ 1 - file->af_CurrentBuf ] )
	{
		file->af_BytesLeft++;
		file->af_Offset = (APTR)((ULONG)file->af_Offset - 1);
		file->af_CurrentFileOffset--;
	}
	else
		SeekAsync( file, -1, OFFSET_CURRENT );
}

/*****************************************************************************/

LONG GetFilesizeAsync(struct AsyncFile *file)
{
	D_S(struct FileInfoBlock,fib);

	if( !file )
		return( -1 );

	if (!ExamineFH(file->af_File,fib))
		return( -1 );

	return( fib->fib_Size );
}

/*****************************************************************************/
/* VAT_ functions - hardcoded implementations */

#define MYPROC ((struct Process*)(SysBase->ThisTask))

/* Library code array for VAT_OpenLibraryCode */
static char *vat_libarray[] = {
	"graphics",
	"utility",
	"workbench",
	"icon",
	"commodities",
	"layers",
	"iffparse",
	"cybergraphics",
	"datatypes",
	"diskfont",
	"rexxsyslib",
	"asl",
	"intuition",
	"mathtrans",
	"mathffp",
	"mathieeedoubtrans",
	"mathieeedoubbas"
};

/* Library code enum values */
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
	VATOC_MATHIEEEDOUBBAS
};

/* Pool functions */
APTR VAT_CreatePool(ULONG flag, ULONG puddlesize, ULONG threshsize)
{
	return CreatePool(flag, puddlesize, threshsize);
}

void VAT_DeletePool(APTR poolheader)
{
	DeletePool(poolheader);
}

APTR VAT_AllocPooled(APTR poolheader, ULONG memsize)
{
	return AllocPooled(poolheader, memsize);
}

void VAT_FreePooled(APTR poolheader, APTR memory, ULONG memsize)
{
	FreePooled(poolheader, memory, memsize);
}

APTR VAT_AllocVecPooled(APTR poolheader, ULONG memsize)
{
	return (APTR)AllocVecPooled(poolheader, memsize);
}

void VAT_FreeVecPooled(APTR poolheader, APTR memory)
{
	FreeVecPooled(poolheader, memory);
}

/* OpenLibrary functions */
struct Library *VAT_OpenLibrary(STRPTR libname, ULONG libversion)
{
	struct Library *l;
	char tpath[128];
	int triedflush = FALSE;

retry:
	l = OpenLibrary(libname, libversion);
	if (!l && !strpbrk(libname, ":/"))
	{
		Strncpy(tpath, "LIBS:", sizeof(tpath) - 1);
		Strncpy(tpath + 5, libname, sizeof(tpath) - 6);
		tpath[sizeof(tpath) - 1] = '\0';
		l = OpenLibrary(tpath, libversion);
	}
	if (!l && !strpbrk(libname, ":/"))
	{
		Strncpy(tpath, "LIBS/", sizeof(tpath) - 1);
		Strncpy(tpath + 5, libname, sizeof(tpath) - 6);
		tpath[sizeof(tpath) - 1] = '\0';
		l = OpenLibrary(tpath, libversion);
	}

	if (!l && !triedflush)
	{
		APTR x;
		triedflush = TRUE;
		x = AllocVec(0xfffffff, 0);
		if (x)
			FreeVec(x);
		goto retry;
	}

	return l;
}

struct Library *VAT_OpenLibraryCode(ULONG libcode)
{
	char buffer[32];
	ULONG len;

	if (libcode >= sizeof(vat_libarray) / sizeof(vat_libarray[0]))
		return NULL;

	Strncpy(buffer, vat_libarray[libcode], sizeof(buffer) - 8);
	len = strlen(buffer);
	Strncpy(buffer + len, ".library", sizeof(buffer) - len - 1);
	buffer[sizeof(buffer) - 1] = '\0';
	return VAT_OpenLibrary(buffer, 0);
}

/* SetLastUsedDir function */
void VAT_SetLastUsedDir(STRPTR appid)
{
	char buff[128];
	char path[128];
	BPTR f;
	int envarcchanged = TRUE;

	NameFromLock(GetProgramDir(), buff, sizeof(buff));

	if (!(f = Lock("ENV:Vapor", SHARED_LOCK)))
		f = CreateDir("ENV:Vapor");
	UnLock(f);
	if (!(f = Lock("ENVARC:Vapor", SHARED_LOCK)))
		f = CreateDir("ENVARC:Vapor");
	UnLock(f);

	Strncpy(path, "ENVARC:Vapor/", sizeof(path) - 1);
	Strncpy(path + 13, appid, sizeof(path) - 14);
	Strncpy(path + 13 + strlen(appid), "_LASTUSEDDIR", sizeof(path) - 13 - strlen(appid) - 1);
	path[sizeof(path) - 1] = '\0';

	f = Open(path, MODE_OLDFILE);
	if (f)
	{
		char buff2[128];
		if (Read(f, buff2, sizeof(buff2)) == strlen(buff))
		{
			if (!memcmp(buff, buff2, strlen(buff)))
				envarcchanged = FALSE;
		}
		Close(f);
	}

	if (envarcchanged)
	{
		f = Open(path, MODE_NEWFILE);
		if (f)
		{
			Write(f, buff, strlen(buff));
			Close(f);
		}
	}

	Strncpy(path, "ENV:Vapor/", sizeof(path) - 1);
	Strncpy(path + 10, appid, sizeof(path) - 11);
	Strncpy(path + 10 + strlen(appid), "_LASTUSEDDIR", sizeof(path) - 10 - strlen(appid) - 1);
	path[sizeof(path) - 1] = '\0';
	f = Open(path, MODE_NEWFILE);
	if (f)
	{
		Write(f, buff, strlen(buff));
		Close(f);
	}
}

/* IsOnline function - simplified version */
int VAT_IsOnline(void)
{
	struct Library *l;

	/* Check for bsdsocket.library */
	if (l = OpenLibrary("bsdsocket.library", 3))
	{
		CloseLibrary(l);
		return TRUE;
	}

	return FALSE;
}

/* Random number generator - simplified implementation */
#define RANDOM_STATE_BYTES 1024

typedef struct
{
	unsigned char state[RANDOM_STATE_BYTES];
	unsigned int next_available_byte;
	unsigned int add_position;
} RandomState;

static struct SignalSemaphore vat_randsem;
static RandomState vat_random_state;
static int vat_rand_isinit = FALSE;
static int vat_randsem_init = FALSE;

static void vat_random_stir(void)
{
	unsigned int i;
	for (i = 0; i < RANDOM_STATE_BYTES; i++)
	{
		vat_random_state.state[i] ^= (unsigned char)(SysBase->IdleCount + i);
		vat_random_state.state[i] ^= (unsigned char)(SysBase->DispCount + i);
	}
	vat_random_state.next_available_byte = 0;
}

static unsigned int vat_random_get_byte(void)
{
	if (vat_random_state.next_available_byte >= RANDOM_STATE_BYTES)
	{
		vat_random_stir();
	}
	return vat_random_state.state[vat_random_state.next_available_byte++];
}

static void vat_random_initialize(void)
{
	struct timeval tv;
	BPTR f;
	int rc;
	char buffer[1024];

	vat_random_state.next_available_byte = 0;
	vat_random_state.add_position = 0;

	/* Initialize with system state */
	GetSysTime(&tv);
	memcpy(vat_random_state.state, &tv, sizeof(tv));
	memcpy(&vat_random_state.state[sizeof(tv)], SysBase, sizeof(struct ExecBase));

	/* Try to load saved seed */
	f = Open("S:VaporToolkit.Randseed", MODE_OLDFILE);
	if (f)
	{
		rc = Read(f, buffer, sizeof(buffer));
		Close(f);
		if (rc > 0)
		{
			unsigned int i;
			for (i = 0; i < rc && i < RANDOM_STATE_BYTES; i++)
				vat_random_state.state[i] ^= buffer[i];
		}
	}

	vat_random_stir();
	vat_rand_isinit = TRUE;
}

void VAT_RandomStir(void)
{
	if (!vat_randsem_init)
	{
		InitSemaphore(&vat_randsem);
		vat_randsem_init = TRUE;
	}
	ObtainSemaphore(&vat_randsem);
	if (!vat_rand_isinit)
		vat_random_initialize();
	vat_random_stir();
	ReleaseSemaphore(&vat_randsem);
}

ULONG VAT_RandomByte(void)
{
	ULONG x;

	if (!vat_randsem_init)
	{
		InitSemaphore(&vat_randsem);
		vat_randsem_init = TRUE;
	}
	ObtainSemaphore(&vat_randsem);
	if (!vat_rand_isinit)
		vat_random_initialize();
	x = vat_random_get_byte();
	ReleaseSemaphore(&vat_randsem);

	return x;
}

/* GetAppScreen function */
struct Screen *VAT_GetAppScreen(APTR app)
{
	struct Screen *scr;
	char *pubname;

	if (MUIMasterBase && MUIMasterBase->lib_Version >= 13)
	{
		struct List *l;
		APTR ostate, o;

		get(app, MUIA_Application_WindowList, &l);
		ostate = l->lh_Head;
		while (o = NextObject(&ostate))
		{
			scr = 0;
			get(o, MUIA_Window_Screen, &scr);
			if (scr)
				return scr;
		}
	}

	pubname = 0;
	get(app, MUIA_Application_PubScreenName, &pubname);
	scr = LockPubScreen(pubname);
	UnlockPubScreen(NULL, scr);

	return scr;
}

/* GetAppScreenName function */
STRPTR VAT_GetAppScreenName(APTR app)
{
	struct Screen *scr = VAT_GetAppScreen(app);

	if (scr)
	{
		struct List *psl = LockPubScreenList();
		struct PubScreenNode *psn;

		for (psn = FIRSTNODE(psl); NEXTNODE(psn); psn = NEXTNODE(psn))
		{
			if (psn->psn_Screen == scr)
				break;
		}
		UnlockPubScreenList();

		if (NEXTNODE(psn))
			return psn->psn_Node.ln_Name;
	}
	return "Workbench";
}

/* SendRXMsg function - simplified version */
static struct MsgPort *vat_rxhandler_rxport = NULL;

int VAT_SendRXMsg(STRPTR cmd, STRPTR basename, STRPTR suffix)
{
	struct RexxMsg *rm;
	struct MsgPort *rexxhost;
	TEXT buffer[1024];

	/* Initialize port if needed */
	if (!vat_rxhandler_rxport)
	{
		vat_rxhandler_rxport = CreateMsgPort();
		if (!vat_rxhandler_rxport)
			return -1;
	}

	rm = CreateRexxMsg(vat_rxhandler_rxport, suffix, basename);
	if (!rm)
		return -1;

	if (*cmd != '"' && *cmd != 39) /* is command not rexx code? */
	{
		STRPTR mark = strchr(cmd, ' ');
		BPTR l;

		if (mark)
			*mark = 0; /* split file/args */

		strcpy(buffer, "CALL \"PROGDIR:Rexx/");
		strcat(buffer, cmd);
		if (!strchr(buffer, '.'))
		{
			strcat(buffer, ".");
			strcat(buffer, suffix);
		}
		l = Lock(&buffer[6], SHARED_LOCK);
		if (l)
		{
			NameFromLock(l, &buffer[6], 256);
			UnLock(l);
		}
		else
			strcpy(&buffer[6], cmd);

		strcat(buffer, "\"(\"");

		if (mark) /* do we have any arguments behind filename */
		{
			STRPTR p = &buffer[strlen(buffer)];
			*p = *mark++ = ' ';

			/* copy args doubling any quotes for rexx func call */
			while (*mark)
			{
				*p = *mark++;
				if (*p++ == '"')
					*p++ = '"';
			}
			*p = 0;
		}

		strcat(buffer, "\")");

		rm->rm_Args[0] = buffer;
		FillRexxMsg(rm, 1, 0);
		rm->rm_Action = RXCOMM | RXFF_NOIO | RXFF_STRING;
	}
	else
	{
		rm->rm_Args[0] = cmd;
		FillRexxMsg(rm, 1, 0);
		rm->rm_Action = RXCOMM | RXFF_NOIO;
	}

	Forbid();
	rexxhost = FindPort("REXX");
	if (rexxhost)
	{
		PutMsg(rexxhost, (APTR)rm);
	}
	else
	{
		ClearRexxMsg(rm, 1);
		DeleteRexxMsg(rm);
	}
	Permit();

	return 0;
}

