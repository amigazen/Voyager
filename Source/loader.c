/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#include <proto/dos.h>
#define __USE_SYSBASE
#include <exec/execbase.h>
#include <proto/exec.h>
#include <workbench/startup.h>
#include <string.h>
#include <exec/memory.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>

#include "vpackdef.h"

typedef int ASM (*EPTR)(register __d0 int arg1, register __a0 int arg2);

static void ASM handlerfunc( void );

BPTR fh;
struct filedef filedefs[ NUMFILES ] = { "Testfile", 0, 4711 };
struct Process *handlerproc;
struct MsgPort *handlerport;

struct DosLibrary *DOSBase;
struct ExecBase *SysBase;

int ASM SAVEDS loader( register __d0 int arg1, register __a0 int arg2 )
{
	struct Process *myproc;
	struct WBStartup *wbm = NULL;
	BPTR oldcd;
	int rc = 20, rc2 = 0;
	BPTR seglist;
	struct DosList *devnode = 0;
	int nodeadded = FALSE;
	EPTR himem;

	SysBase = *((struct ExecBase**)4);
	myproc = (APTR)(SysBase->ThisTask);

	if( !myproc->pr_CLI )
	{
		// WB startup. Grab the fucking startup message
		while( !( wbm = (APTR)GetMsg( &myproc->pr_MsgPort ) ) )
			WaitPort( &myproc->pr_MsgPort );
	}

	DOSBase = (APTR)OpenLibrary( "dos.library", 37 );
	if( !DOSBase )
	{
		// oh fuck
		if( wbm )
		{
			Forbid();
			ReplyMsg( wbm );
		}
		return( -1 );
	}

	oldcd = CurrentDir( myproc->pr_HomeDir );

	if( wbm )
	{
		fh = Open( wbm->sm_ArgList->wa_Name, MODE_OLDFILE );
	}
	else
	{
		char cmdname[ 204 ];
		GetProgramName( cmdname, sizeof( cmdname ) );
		fh = Open( FilePart( cmdname ), MODE_OLDFILE );
	}

	CurrentDir( oldcd );

	if( !fh )
	{
		rc2 = ERROR_OBJECT_WRONG_TYPE;
		goto quitit;
	}

	// we're done with opening the file
	// now load the table
	if( Seek( fh, 0xac1daffe, OFFSET_BEGINNING ) < 0 )
		goto quitit;

	Read( fh, &filedefs, sizeof( filedefs ) );

	handlerproc = CreateNewProcTags( 
		NP_Entry, handlerfunc,
		NP_Name, "V³ Supervisor",
		NP_StackSize, 4096,
		NP_Priority, 5,
		TAG_DONE
	);

	if( !handlerproc )
		goto quitit;

	while( !handlerport )
		Delay( 1 );

	devnode = MakeDosEntry( "_VPACK", DLT_DEVICE );
	if( !devnode )
		goto quitit;

	devnode->dol_Task = handlerport;

	if( !AddDosEntry( devnode ) )
		goto quitit;

	nodeadded = TRUE;

	seglist = LoadSeg( "_VPACK:main" );

	if( seglist )
	{

		himem = (EPTR)BADDR( seglist + 1 );

		// we're ready to kick off...
		if( wbm )
		{
			PutMsg( &myproc->pr_MsgPort, wbm );
			wbm = NULL;
		}

		rc = himem( arg1, arg2 );
		rc2 = myproc->pr_Result2;

		UnLoadSeg( seglist );
	}

quitit:

	LockDosList( LDF_DEVICES | LDF_WRITE );

	if( nodeadded )
		RemDosEntry( devnode );
	if( devnode )
		FreeDosEntry( devnode );

	UnLockDosList( LDF_DEVICES | LDF_WRITE );

	while( handlerproc )
	{
		Signal( handlerproc, SIGBREAKF_CTRL_C );
		Delay( 1 );
	}

	if( fh )
		Close( fh );

	CloseLibrary( DOSBase );

	myproc->pr_Result2 = rc2;

	if( wbm )
	{
		Forbid();
		ReplyMsg( wbm );
	}

	return( rc );
}

struct filedef *findfile( BPTR nameptr )
{
	char *name = (char*)BADDR( nameptr ) + 1;
	int c;

	name = FilePart( name );

	//kprintf( "find: %s\n", name );

	for( c = 0; c < NUMFILES && filedefs[ c ].filename[ 0 ]; c++ )
	{
		if( !strcmp( filedefs[ c ].filename, name ) )
			return( &filedefs[ c ] );
	}

	return( 0 );
}

static void ASM SAVEDS handlerfunc( void )
{
	int done = FALSE;
	handlerport = CreateMsgPort();

	//kprintf( "Hi1\n" );

	while( !done )
	{
		struct StandardPacket *dp;
		int r1, r2;
		struct MsgPort *rp;

		while( !( dp = (APTR)GetMsg( handlerport ) ) )
		{
			if( Wait( SIGBREAKF_CTRL_C | ( 1L << handlerport->mp_SigBit ) ) & SIGBREAKF_CTRL_C )
			{
				done = TRUE;
				break;
			}
		}

		if( !dp )
			continue;

		//kprintf( "got action %ld\n", dp->sp_Pkt.dp_Action );

		switch( dp->sp_Pkt.dp_Action )
		{
			case ACTION_LOCATE_OBJECT:
				{
					struct FileLock *lock;
					struct filedef *fd = findfile( dp->sp_Pkt.dp_Arg2 );
			
					if( fd )
					{
						lock = AllocVec( sizeof( *lock ), MEMF_CLEAR );
						lock->fl_Task = handlerport;
						lock->fl_Key = (ULONG)fd;

						r1 = (int)MKBADDR( lock );
						r2 = 0;
					}
					else
					{
						r1 = 0;
						r2 = ERROR_OBJECT_NOT_FOUND;
					}
				}
				break;

			case ACTION_FREE_LOCK:
				{
					APTR v = BADDR( dp->sp_Pkt.dp_Arg1 );
					if( v )
						FreeVec( v );
				}

			case ACTION_END:
				r1 = DOSTRUE;
				r2 = 0;
				break;

			case ACTION_EXAMINE_OBJECT:
				{
					struct FileInfoBlock *fib = BADDR( dp->sp_Pkt.dp_Arg2 );
					struct FileLock *lock = BADDR( dp->sp_Pkt.dp_Arg1 );

					memset( fib, 0, sizeof( *fib ) );

					if( !lock )
					{
						r1 = FALSE;
						r2 = ERROR_INVALID_LOCK;
					}
					else
					{
						struct filedef *fd = (APTR)lock->fl_Key;
						strcpy( &fib->fib_FileName[ 1 ], fd->filename ); 
						fib->fib_FileName[ 0 ] = strlen( fd->filename );
						fib->fib_Protection = FIBF_DELETE;
						fib->fib_DirEntryType = fib->fib_EntryType = ST_FILE;
						fib->fib_Size = fd->filesize;
						fib->fib_NumBlocks = 1;
						DateStamp( &fib->fib_Date );

						r1 = DOSTRUE;
						r2 = 0;
					}
				}
				break;

			case ACTION_FINDINPUT:  
				{
					struct filedef *fd = findfile( dp->sp_Pkt.dp_Arg3 );
					if( fd )
					{
						struct FileHandle *fh = BADDR( dp->sp_Pkt.dp_Arg1 );

						fh->fh_Arg1 = (LONG)fd;
						fd->readpos = 0;

						r1 = DOSTRUE;
						r2 = 0;
					}
					else
					{
						r1 = FALSE;
						r2 = ERROR_OBJECT_NOT_FOUND;
					}
				}
				break;

			case ACTION_READ:
				{
					struct filedef *fd = (APTR)dp->sp_Pkt.dp_Arg1;
					int rlen = dp->sp_Pkt.dp_Arg3;
					APTR dest = (APTR)dp->sp_Pkt.dp_Arg2;
					//kprintf( "read: %s, len %ld, rp %ld\n", fd->filename, rlen, fd->readpos );

					if( fd->readpos + rlen > fd->filesize )
						rlen = fd->filesize - fd->readpos;
					if( rlen )
					{
						Seek( fh, fd->fileoffset + fd->readpos, OFFSET_BEGINNING );
						r1 = Read( fh, dest, rlen );
						fd->readpos += r1;
						r2 = 0;
					}
					else
					{
						r1 = 0;
						r2 = 0;
					}
				}
				break;

			default:
				r1 = FALSE;
				r2 = ERROR_ACTION_NOT_KNOWN;
				break;
		}

		rp = dp->sp_Pkt.dp_Port;
		dp->sp_Pkt.dp_Res1 = r1;
		dp->sp_Pkt.dp_Res2 = r2;
		dp->sp_Msg.mn_Node.ln_Name = (char*)&dp->sp_Pkt;
		dp->sp_Pkt.dp_Port = handlerport;

		PutMsg( rp, ( struct Message * ) dp );
	}

	//kprintf( "bibi\n" );

done:
	DeleteMsgPort( handlerport );
	handlerproc = NULL;
}
