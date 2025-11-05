#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

// TOFIX: cleanup
//#include <clib/exec_protos.h>
//#include <clib/dos_protos.h>
//#include <pragmas/exec_pragmas.h>
//#include <pragmas/dos_pragmas.h>

#ifdef __SASC
#include <dos.h>
#endif

#include <proto/exec.h>
#include <proto/dos.h>

#include <time.h>

#include "libraries/vat.h"

#include "globals.h"

/*
 * TOFIX: this should really be upgraded to the latest
 * asyncio.lib package but there are custom extentions now.
 */

/****** vapor_toolkit.library/--asyncio-- ******************************************
* 
*   FUNCTION
*   The "vapor_toolkit.library" offers a set of high performance I/O
*   routines based on Martin Tailliefers "FastIO" package.
*   See the original documentation for using this routines,
*   only extensions will be mentioned here.
*
******************************************************************************
*/


/*****************************************************************************/



/*****************************************************************************/


/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);


/*****************************************************************************/


/* send out an async packet to the file system. */
static void __inline SendPacket(struct AsyncFile *file, APTR arg2)
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


struct AsyncFile * ASM SAVEDS VAT_OpenAsync(__reg( a0, const STRPTR fileName ), __reg( d0, UBYTE accessMode ), __reg( d1, LONG bufferSize ) )
{
	struct AsyncFile  *file;
	struct FileHandle *fh;
	BPTR               handle;
	BPTR               lock;
	LONG               blockSize;
	ULONG              startOffset;
	D_S(struct InfoData,infoData);

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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


LONG ASM SAVEDS VAT_CloseAsync( __reg( a0, struct AsyncFile *file ) )
{
	LONG result;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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


LONG ASM SAVEDS __inline VAT_ReadAsync( __reg( a0, struct AsyncFile *file ), __reg( a1, APTR buffer ), __reg( d0, LONG numBytes ) )
{
	LONG totalBytes;
	LONG bytesArrived;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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


LONG ASM SAVEDS __inline VAT_ReadCharAsync( __reg( a0, struct AsyncFile *file ) )
{
	unsigned char ch;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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

	if (VAT_ReadAsync(file,&ch,1) > 0)
		return((LONG)ch);

	/* We couldn't read above, so fail */

	return(-1);
}

static LONG __inline local_ReadCharAsync(struct AsyncFile *file)
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

	if (VAT_ReadAsync(file,&ch,1) > 0)
		return((LONG)ch);

	/* We couldn't read above, so fail */

	return(-1);
}


/*****************************************************************************/


LONG ASM SAVEDS __inline VAT_WriteAsync( __reg( a0, struct AsyncFile *file ), __reg( a1, APTR buffer ), __reg( d0, LONG numBytes) )
{
	LONG totalBytes;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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


LONG ASM SAVEDS __inline VAT_WriteCharAsync( __reg( a0, struct AsyncFile *file ), __reg( d0, UBYTE ch) )
{
	UBYTE ch2;
	
#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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
	return(VAT_WriteAsync(file,&ch2,1));
}


/*****************************************************************************/


LONG ASM SAVEDS VAT_SeekAsync( __reg( a0, struct AsyncFile *file ), __reg( d0, LONG position ), __reg( d1, BYTE mode) )
{
	LONG  current, target;
	LONG  minBuf, maxBuf;
	LONG  bytesArrived;
	LONG  diff;
	LONG  filePos;
	LONG  roundTarget;
	D_S(struct FileInfoBlock,fib);

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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

#if 0
		if( file->af_Received > 1 )
		{
			/* MH: Bugfix. Prevents some seeks from being slow (some buffers
			 * would be read twice, since the code would think the data
			 * wasn't in our buffers when it really was).
			 */
			minBuf -= file->af_BufferSize;
		}
#endif


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

/****** vapor_toolkit.library/VAT_FGetsAsync ******************************************
* 
*   NAME	
*   VAT_FGetsAsync -- asyncio version of fgets()
* 
*   SYNOPSIS
*   buffer = VAT_FGetsAsync( asyncfile, buffer, maxlen )
*   D0                       A0         A1      D0
*
*   UBYTE * VAT_FGetsAsync( struct AsyncFile *, UBYTE *, LONG );
*
*   FUNCTION
*   Reads a string upto maxlen bytes or until EOF or a LF is
*   hit.
* 
*   INPUTS
*   asyncfile     - an pointer to a asyncfile as returned by
*                   VAT_OpenAsync()
*
*   buffer        - buffer to receive line
*
*   maxlen        - length of buffer
*
*	
*   RESULT
*   buffer - if everything went ok, the original buffer pointer
*            is returned, else NULL
* 
*   EXAMPLE
*
*   NOTES
*
*   BUGS
* 
*   SEE ALSO
* 
******************************************************************************
*/

UBYTE ASM SAVEDS *VAT_FGetsAsync( __reg( a0, struct AsyncFile *file ), __reg( a1, UBYTE *buffer ), __reg( d0, int maxlen ) )
{
	int ch;
	int len = 0;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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

UBYTE ASM SAVEDS *VAT_FGetsAsyncNoLF( __reg( a0, struct AsyncFile *file ), __reg( a1, UBYTE *buffer ), __reg( d0, int maxlen ) )
{
	int ch;
	int len = 0;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

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


/****** vapor_toolkit.library/VAT_VFPrintfAsync ******************************************
* 
*   NAME	
*   VAT_VFPrintfAsync -- asyncio version of vfprintf()
*   VAT_FPrintfasync -- varargs stub for VAT_VFPrintfAsync
* 
*   SYNOPSIS
*   len = VAT_VFPrintfAsync( asyncfile, fmtstring, args )
*   D0                       A1         A1         A2
*
*   LONG VAT_VFPrintfAsync( struct AsyncFile *, UBYTE *, APTR );
*   LONG VAT_FPrintfAsync( struct AsyncFile *, UBYTE, ... );
*
*   FUNCTION
*   Formats and prints a string to an async file.
* 
*   INPUTS
*   asyncfile     - an pointer to a asyncfile as returned by
*                   VAT_OpenAsync()
*
*   fmtstring     - standard printf()-like formating string
*
*   args          - optional args as described by fmtstring
*
*	
*   RESULT
*   len    - number of bytes written.
* 
*   EXAMPLE
*
*   NOTES
*   This call uses exec.library/RawDoFmt(), so the usual
*   restrictions apply.
*
*   BUGS
* 
*   SEE ALSO
*   exec.library/RawDoFmt
* 
******************************************************************************
*/
struct fpi
{
	ULONG a6;
	struct AsyncFile *to;
	int error;
	int count;
};

/*
 * With SASC there's a tagcall
 * and libcall in the pragma magic
 * so there's only one function for
 * both calls. For MorphOS, fd2inline
 * will take care of it. Don't forget to
 * use a version with VAT_VFPrintfAsync in
 * its internal table though.
 */

#ifndef __MORPHOS__
extern void __asm call_dofpi( void );
#endif /* !__MORPHOS__ */

#ifdef __MORPHOS__
void dofpi( void )
{
	GETEMULHANDLE
	char ch = REG_D0;
	struct fpi *fpi = ( struct fpi * )REG_A3;
	struct EmulCaos MyCaos;
#else
void ASM SAVEDS dofpi( __reg( d0, char ch ), __reg( a3, struct fpi *fpi ) )
{
#endif /* __MORPHOS__ */
	if( !ch )
		return;
	
	if( fpi->error )
		return;
	fpi->count++;

	if( VAT_WriteCharAsync( fpi->to, ch ) != 1 )
	{
		fpi->error = 1;
	}
}

#ifdef __MORPHOS__
struct EmulLibEntry GATE_dofpi = {
	TRAP_LIB, 0, ( void( * )( void ) )dofpi
};
#endif

int ASM SAVEDS VAT_VFPrintfAsync( __reg( a0, struct AsyncFile *file ), __reg( a1, char *string ), __reg( a2, APTR args ) )
{
	struct fpi fpi;

#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

	fpi.error = fpi.count = 0;
	fpi.to = file;
#ifdef __MORPHOS__
	fpi.a6 = REG_A6;
	RawDoFmt( string, args, ( void * )&GATE_dofpi, &fpi );
#else
	fpi.a6 = getreg( REG_A6 );
	RawDoFmt( string, args, call_dofpi, &fpi );
#endif /* !__MORPHOS__ */

	return( fpi.error ? -1 : fpi.count );
}

/****** vapor_toolkit.library/VAT_FtellAsync ******************************************
* 
*   NAME	
*   VAT_FtellAsync -- tell position of asyncfile
* 
*   SYNOPSIS
*   pos = VAT_FtellAsync( asyncfile )
*   D0                    A0
*
*   LONG VAT_FtellAsync( struct AsyncFile * );
*
*   FUNCTION
*   Returns the current position of an asyncfile.
* 
*   INPUTS
*   asyncfile     - an pointer to a asyncfile as returned by
*                   VAT_OpenAsync()
*
*   RESULT
*   pos - file position or -1 for error
* 
*   EXAMPLE
*
*   NOTES
*   The clutil version of the asyncio routines maintain a
*   special offset counter which allows for inquiring the
*   offset without disturbing any I/O in progress.
*
*   Always use this call, not VAT_SeekAsync(f,0,MODE_CURRENT)!
*
*   BUGS
* 
*   SEE ALSO
* 
******************************************************************************
*/
LONG ASM SAVEDS VAT_FtellAsync( __reg( a0, struct AsyncFile *file ) )
{
#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

	if( file )
		return( file->af_CurrentFileOffset );
	else
	{
		SetIoErr(ERROR_INVALID_LOCK);
		return( -1 );
	}
}

/****** vapor_toolkit.library/VAT_UnGetCAsync ******************************************
* 
*   NAME	
*   VAT_UnGetCAsync -- seek back one char
* 
*   SYNOPSIS
*   VAT_UnGetCAsync( asyncfile )
*                      A0
*
*   void VAT_UnGetCAsync( struct AsyncFile * );
*
*   FUNCTION
*   Seeks back one position.
* 
*   INPUTS
*   asyncfile     - an pointer to a asyncfile as returned by
*                   VAT_OpenAsync()
*
*   RESULT
*   None.
* 
*   EXAMPLE
*
*   NOTES
*   This function generally has a much less severe overhead
*   than SeekAsync(f,-1,OFFSET_CURRENT).
*
*   BUGS
* 
*   SEE ALSO
* 
******************************************************************************
*/

void ASM SAVEDS VAT_UnGetCAsync( __reg( a0, struct AsyncFile *file ) )
{
#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

	if( file->af_Offset != file->af_Buffers[ 1 - file->af_CurrentBuf ] )
	{
		file->af_BytesLeft++;
		file->af_Offset = (APTR)((ULONG)file->af_Offset - 1);
		file->af_CurrentFileOffset--;
	}
	else
		VAT_SeekAsync( file, -1, OFFSET_CURRENT );
}

LONG ASM SAVEDS VAT_GetFilesizeAsync( __reg( a0, struct AsyncFile *file ) )
{
	D_S(struct FileInfoBlock,fib);
	
#if DEBUG_ASYNCIO
	DB( ( "called\n" ) );
#endif

	if( !file )
		return( -1 );

	if (!ExamineFH(file->af_File,fib))
		return( -1 );

	return( fib->fib_Size );
}
