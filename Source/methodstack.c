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


/*
 * Own Method handlings
 * --------------------
 * - MUI's pushmethods are a bit lame because they use
 * a fixed sized array. Those are better suited for V which
 * makes an extensive use of them.
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: methodstack.c,v 1.20 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#endif

/* private */
#include "methodstack.h"
#include "mui_func.h"

static struct SignalSemaphore mssem;
static struct v_Task *mstask;
struct MinList methodlist;
static APTR methodpool;

struct pushedmethod {
	struct MinNode n;
	ULONG size;
	APTR obj;
	ULONG m[ 0 ];
};

struct syncmethod {
	struct MinNode n;
	ULONG flag; // well be ~0 for a sync method
	APTR obj;
	APTR msg;
	ULONG result;
	struct Message m;
};

int init_methodstack( void )
{
	D( db_init, bug( "initializing..\n" ) );

	InitSemaphore( &mssem );
	mstask = FindTask( NULL );
	NEWLIST( &methodlist );
	methodpool = CreatePool( MEMF_ANY, 2048, 1024 );
	return( ( int )methodpool );
}

void cleanup_methodstack( void )
{
	D( db_init, bug( "cleaning up..\n" ) );

	if( methodpool )
	{
		// Semaphore is kept till shutdown
		ObtainSemaphore( &mssem );
		DeletePool( methodpool );
		mstask = NULL;
	}
}

void STDARGS SAVEDS pushmethod( APTR obj, ULONG cnt, ... )
{
	struct pushedmethod *pm;
	LONG i = 0;
	va_list va;

	if( !mstask )
		return;

	va_start( va, cnt );

	ObtainSemaphore( &mssem );

	pm = AllocPooled( methodpool, sizeof( *pm ) + cnt * sizeof( LONG ) );
	if( pm )
	{
		pm->obj = obj;
		pm->size = cnt;

		while( cnt-- )
		{
			pm->m[ i ] = va_arg( va, ULONG );
			i++;
		}
		ADDTAIL( &methodlist, pm );
	}

	va_end( va );

	ReleaseSemaphore( &mssem );
	Signal( mstask, SIGBREAKF_CTRL_E );
}

ULONG SAVEDS pushsyncmethod( APTR obj, APTR msg )
{
	struct syncmethod pm;
	struct Process *thisproc = (APTR)FindTask( NULL );
#ifndef MBX
	struct MsgPort *replyport = &(thisproc->pr_MsgPort);
#else
	struct MsgPort *replyport = NULL;
#endif
	if( !mstask )
		return( -1UL );

	if( thisproc == (APTR)mstask )
	{
		return( DoMethodA( obj, ( Msg )msg ) );
	}

	pm.obj = obj;
	pm.msg = msg;
	pm.flag = (ULONG)~0;
#ifdef MBX
	replyport = CreateMsgQueue( "spotme", 0, TRUE );
	if( !replyport )
		return( -1UL );  //Simulate a failure
	InitMessage( &pm.m, replyport, "syncmethod", sizeof( Message_s ) );
#else
	pm.m.mn_ReplyPort = replyport;
#endif

	ObtainSemaphore( &mssem );
	ADDTAIL( &methodlist, &pm );
	ReleaseSemaphore( &mssem );

	Signal( mstask, SIGBREAKF_CTRL_E );

#ifdef MBX
	{
	FLAGS wsigs, rsigs;
		wsigs  = 1UL << replyport->mqu_SignalBit;
		D(db_init, kprintf("*** Wait for %08lx \n", wsigs); )
		wsigs |= SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F;
		do
		{
			rsigs = Wait( wsigs );
			D(db_init, kprintf("*** Received %08lx from Wait()\n", rsigs); )
			if ( rsigs & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F) )
			{
				D(db_init,  kprintf( "*** Got CTRL-x while waiting for pushsyncmethod() to complete\n"
													 "***   Waiter is: %08lx %s RecSigs: %08lx\n",
														 thisproc, thisproc->pr_Node.ln_Name,rsigs ); );
			}
		}
		while ( !(rsigs & (1UL << replyport->mqu_SignalBit)) );
	}
	DeleteMsgQueue( pm.m.mn_ReplyPort, TRUE );
#else
	WaitPort( replyport );
	GetMsg( replyport );
#endif
	return( pm.result );
}

void killpushedmethods( APTR obj )
{
	struct pushedmethod *pm, *next;

	//kprintf( "killpushmethods(%lx)\n", obj );

	ObtainSemaphore( &mssem );

	for( pm = FIRSTNODE( &methodlist ); next = NEXTNODE( pm ); pm = next )
	{
		if( pm->obj == obj )
		{
			//kprintf( "killing method on %lx\n", obj );
			pm->obj = NULL;
		}
	}

	ReleaseSemaphore( &mssem );
}

void checkmethods( void )
{
	struct pushedmethod *pm = NULL;

	for(;;)
	{
		ObtainSemaphore( &mssem );
		if( pm )
			FreePooled( methodpool, pm, pm->size * 4 + sizeof( *pm ) );
		pm = REMHEAD( &methodlist );
		ReleaseSemaphore( &mssem );
		if( !pm )
			break;
		if( pm->size == (ULONG)~0 )
		{
			struct syncmethod *sm = (APTR)pm;
			if( sm->obj )
				sm->result = DoMethodA( sm->obj, sm->msg );
			else
				sm->result = -1;
			ReplyMsg( &sm->m );
			pm = NULL;
		}
		else
		{
			if( pm->obj )
			{
				//kprintf( "Executing method %lx on %lx (%s)\n", pm->m[ 0 ], pm->obj, OCLASS( pm->obj )->cl_ID );
#ifdef MBX
				// TOFIX! Debugging hack
				DoMethod( pm->obj, pm->m[ 0 ], pm->m[ 1 ], pm->m[ 2 ], pm->m[ 3 ], pm->m[ 4 ], pm->m[ 5 ], pm->m[ 6 ], pm->m[ 7 ], pm->m[ 8 ], pm->m[ 9 ], pm->m[ 10 ] );
#else
				DoMethodA( pm->obj, (Msg)&pm->m[ 0 ] );
#endif
			}
		}
	}
}
