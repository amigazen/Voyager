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
	$Id: bitmapclone.c,v 1.17 2003/11/18 10:37:40 zapek Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <graphics/displayinfo.h>
#include <intuition/screens.h>
#if USE_CGX
#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#endif
#endif

/* private */
#include "mui_func.h"

static int initclonelist_done;
static struct MinList clonelist;
static struct SignalSemaphore clonesem;
static int clonecount;
static APTR clonepool;

struct bmclone {
	struct MinNode n;
	struct BitMap *orig_bm;
	struct BitMap *real_bm;
	int planesize;
	int xs, ys;
	UWORD modified;
	UWORD interleaved;
};

struct BitMap fakebitmap;
int fblitinstalled;
int iscybermap;
#if USE_CGX
struct Library *CyberGfxBase;
#endif
extern struct Screen *destscreen;

#if USE_CGX
/*
 * Do not OpenLibrary cybergraphics during init. A machine with the library
 * and one without then take the same init path.
 *
 * GetCyberMapAttr on a planar/AGA bitmap gurus classic CGX. Gate screens
 * with IsCyberModeID first. Do not require BMF_SPECIALFMT: OS4 graphics V54
 * RTG maps are CyberMode but the 68k BitMap flags often omit that bit, and
 * treating them as planar BltBitMapRastPort DSI's in the native kernel
 * (DAR 0x70, GrimReaper on Sam460ex).
 */
int ensure_cybergfx( void )
{
	if( CyberGfxBase )
		return( TRUE );

	CyberGfxBase = OpenLibrary( "cybergraphics.library", 40 );
	if( !CyberGfxBase )
	{
		VoyLog(( "[CGX] cybergraphics.library v40+ not opened\n" ));
		VoyFlush();
		return( FALSE );
	}
	VoyLog(( "[CGX] opened cybergraphics.library v%ld.%ld at %lx\n",
		(long)CyberGfxBase->lib_Version, (long)CyberGfxBase->lib_Revision,
		(ULONG)CyberGfxBase ));
	VoyFlush();
	return( TRUE );
}

int bitmap_is_cybergfx( struct BitMap *bm )
{
	if( !bm )
		return( FALSE );
	if( !ensure_cybergfx() )
		return( FALSE );
	if( !( bm->Flags & BMF_SPECIALFMT ) && !iscybermap )
		return( FALSE );
	return( GetCyberMapAttr( bm, CYBRMATTR_ISCYBERGFX ) != 0 );
}

int screen_is_cybergfx( struct Screen *scr )
{
	ULONG modeid;

	if( !scr || !scr->RastPort.BitMap )
		return( FALSE );
	if( !ensure_cybergfx() )
		return( FALSE );
	modeid = GetVPModeID( &scr->ViewPort );
	if( modeid != INVALID_ID && !IsCyberModeID( modeid ) )
		return( FALSE );
	return( GetCyberMapAttr( scr->RastPort.BitMap, CYBRMATTR_ISCYBERGFX ) != 0 );
}
#endif

void init_fakebitmap( void )
{
	D( db_init, bug( "initializing..\n" ) );
	VoyLog(( "[INIT] init_fakebitmap() entry\n" ));
	VoyFlush();

#if USE_CGX
	VoyLog(( "[INIT] cybergraphics.library deferred until a CGX screen is used\n" ));
	VoyFlush();
#endif

	VoyLog(( "[INIT] Calling InitBitMap()...\n" ));
	VoyFlush();
	InitBitMap( &fakebitmap, 1, 1024, 1024 );
	VoyLog(( "[INIT] InitBitMap() complete\n" ));
	VoyFlush();

	/* FBlit is optional - check for it but don't fail if not present */
	fblitinstalled = FALSE;
	VoyLog(( "[INIT] Checking for FBlit message port...\n" ));
	VoyFlush();
	if( SysBase != NULL )
	{
		/*
		 * FindPort() on a public port must run inside Forbid/Permit so the
		 * port cannot vanish while we look it up. Do not Printf/Flush here:
		 * DOS inside Forbid can stall the scheduler.
		 */
		Forbid();
		if( FindPort( "FBlit" ) != NULL )
			fblitinstalled = TRUE;
		Permit();
		if( fblitinstalled )
			VoyLog(( "[INIT] FBlit found, fblitinstalled=TRUE\n" ));
		else
			VoyLog(( "[INIT] FBlit not found, fblitinstalled=FALSE\n" ));
		VoyFlush();
	}
	else
	{
		VoyLog(( "[INIT] SysBase is NULL, skipping FBlit check\n" ));
		VoyFlush();
	}
	VoyLog(( "[INIT] init_fakebitmap() complete\n" ));
	VoyFlush();
}

void close_cybergfx( void )
{
	D( db_init, bug( "cleaning up..\n" ) );
#if USE_CGX
	if( CyberGfxBase )
	{
		CloseLibrary( CyberGfxBase );
		CyberGfxBase = NULL;
	}
#endif
}


// Flush clones
static void flushclones( void )
{
	struct bmclone *bmc;

	while( clonecount > 2 )
	{
		bmc = REMHEAD( &clonelist );
		if( bmc->real_bm != &fakebitmap )
			FreeBitMap( bmc->real_bm );
		FreePooled( clonepool, bmc, sizeof( *bmc ) );
		clonecount--;
	}
}

#define IMGBUFFER_MEMHANDLER_NAME "V's Image Buffer Low Mem Handler"
#define IMGBUFFER_MEMHANDLER_PRI 30

#ifdef AMIGAOS
static int ASM SAVEDS memhandlerfunc( __reg( a0, struct MemHandlerData *mhd ) );

typedef void (*INTFUNC)();

static struct Interrupt memhandlerint = {
	0, 0,
	NT_INTERRUPT,
	IMGBUFFER_MEMHANDLER_PRI,
	IMGBUFFER_MEMHANDLER_NAME,
	NULL,
	(INTFUNC)memhandlerfunc
};
#endif /* AMIGAOS */

#ifdef __MORPHOS__
static int memhandlerfunc( void );

static struct IntData
{
	struct EmulLibEntry InterruptFunc;
	struct Interrupt Interrupt;
	struct ExecBase *SysBase;
} mosintdata;

#endif /* __MORPHOS__ */


#ifdef __MORPHOS__
static int memhandlerfunc( void )
#else
static int ASM SAVEDS memhandlerfunc( __reg( a0, struct MemHandlerData *mhd ) )
#endif /* !__MORPHOS__ */
{
	int rc = MEM_DID_NOTHING;

#ifdef __MORPHOS__
	struct ExecBase *SysBase = mosintdata.SysBase;
#endif /* __MORPHOS__ */
	
	// try to get hold of the list semaphore (to be save)
	if( AttemptSemaphore( &clonesem ) )
	{
		flushclones();
		ReleaseSemaphore( &clonesem );
		rc = MEM_ALL_DONE;
	}

	return( rc );
}


int	init_clonelist( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( clonepool = CreatePool( MEMF_ANY, sizeof( struct bmclone ) * 32, sizeof( struct bmclone ) * 16 ) )
	{
		InitSemaphore( &clonesem );
		NEWLIST( &clonelist );

#ifdef __MORPHOS__
		mosintdata.SysBase = SysBase;
		mosintdata.InterruptFunc.Trap = TRAP_LIB;
		mosintdata.InterruptFunc.Extension = 0;
		mosintdata.InterruptFunc.Func = ( void ( * )( void ) )memhandlerfunc;
		mosintdata.Interrupt.is_Node.ln_Type = NT_INTERRUPT;
		mosintdata.Interrupt.is_Node.ln_Pri = IMGBUFFER_MEMHANDLER_PRI;
		mosintdata.Interrupt.is_Node.ln_Name = IMGBUFFER_MEMHANDLER_NAME;
		mosintdata.Interrupt.is_Data = &mosintdata;
		mosintdata.Interrupt.is_Code = ( void ( * )( void ) )&mosintdata.InterruptFunc;

		AddMemHandler( &mosintdata.Interrupt );
#endif
#ifdef AMIGAOS
		AddMemHandler( &memhandlerint );
#endif /* AMIGAOS */
		initclonelist_done = TRUE;
		return( TRUE );
	}
	else
	{
		D( db_init, bug( "oops, couldn't create the pool\n" ) );
		return( FALSE );
	}
}

void cleanup_clonelist( void )
{
	struct bmclone *bmc;

	if( initclonelist_done )
	{
#ifdef AMIGAOS
		RemMemHandler( &memhandlerint );
#endif
#ifdef __MORPHOS__
		RemMemHandler( &mosintdata.Interrupt );
#endif

		while( bmc = REMHEAD( &clonelist ) )
		{
			if( bmc->real_bm != &fakebitmap )
				FreeBitMap( bmc->real_bm );
		}

		DeletePool( clonepool );
	}
}

static void __inline copyplane( UBYTE *srcplane, UBYTE *destplane, int srcbytes, int destbytes, int rows )
{
	while( rows-- )
	{
		CopyMem( srcplane, destplane, srcbytes );
		srcplane += srcbytes;
		destplane += destbytes;
	}
}

void ASM __far removeclone( __reg( a0, struct BitMap *src ) )
{
	struct bmclone *bm;

	if( iscybermap || fblitinstalled )
		return;

	ObtainSemaphore( &clonesem );

	for( bm = FIRSTNODE( &clonelist ); NEXTNODE( bm ); bm = NEXTNODE( bm ) )
	{
		if( bm->orig_bm == src )
		{
			REMOVE( bm );
			if( bm->real_bm != &fakebitmap )
			{
				WaitBlit();
				FreeBitMap( bm->real_bm );
			}
			FreePooled( clonepool, bm, sizeof( *bm ) );
			clonecount--;
			break;
		}
	}

	ReleaseSemaphore( &clonesem );
}

/*
 * PNG alpha uses maskbm == (APTR)-1. That is not a BitMap. Returning it
 * from getclone() when iscybermap is set made callers do
 * ((APTR)-1)->Planes[0] and jump to 0xfffffffe (OS4 68k exception
 * 0x80000007, graphics.library / muimaster).
 */
struct BitMap *getclone( struct BitMap *src, int masked )
{
	struct bmclone *bm;
	int xs, ys, depth;
	int c;

	if( !src || src == (struct BitMap *)-1 )
		return( NULL );

	if( iscybermap || fblitinstalled )
		return( src );

	if( !src->Planes[ 0 ] )
		return( src );

	if( ( TypeOfMem( src->Planes[ 0 ] ) & MEMF_CHIP ) == MEMF_CHIP )
		return( src );

	xs = GetBitMapAttr( src, BMA_WIDTH );
	ys = GetBitMapAttr( src, BMA_HEIGHT );

	ObtainSemaphore( &clonesem );
	for( bm = FIRSTNODE( &clonelist ); NEXTNODE( bm ); bm = NEXTNODE( bm ) )
	{
		if( bm->orig_bm == src && bm->xs == xs && bm->ys == ys )
			break;
	}

	if( NEXTNODE( bm ) )
	{
		int c;

		// Back on list
		REMOVE( bm );
		ADDTAIL( &clonelist, bm );

		if( bm->modified && bm->real_bm != &fakebitmap )
		{
			for( c = 0; c < src->Depth; c++ )
			{
				if( bm->interleaved )
					copyplane( src->Planes[ c ], bm->real_bm->Planes[ c ], src->BytesPerRow, bm->real_bm->BytesPerRow, src->Rows );
				else
					CopyMem( src->Planes[ c ], bm->real_bm->Planes[ c ], bm->planesize );
			}
			bm->modified = FALSE;
		}

		ReleaseSemaphore( &clonesem );

		return( bm->real_bm );
	}

	// we have to allocate a new bm
	bm = AllocPooled( clonepool, sizeof( *bm ) );
	bm->orig_bm = src;
	bm->xs = xs;
	bm->ys = ys;
	depth = GetBitMapAttr( src, BMA_DEPTH );

	#ifndef __MORPHOS__
	while( AvailMem( MEMF_CHIP ) < ( 768 * 1024 ) )
	{
		struct bmclone *bmc;

		if( clonecount < 3 )
			break;

		//Printf( "freeing a clone, cc %ld\n", clonecount );

		WaitBlit();

		bmc = REMHEAD( &clonelist );
		if( bmc->real_bm != &fakebitmap )
			FreeBitMap( bmc->real_bm );
		FreePooled( clonepool, bmc, sizeof( *bmc ) );
		clonecount--;
	}
	#endif

	ReleaseSemaphore( &clonesem );

	/* When destscreen not set yet (no lo_image Setup run), use NULL friend to avoid crash */
	if( !destscreen && !masked )
	{
		VoyLog(( "[CLONE] getclone destscreen=NULL using NULL friend\n" ));
		VoyFlush();
	}
	bm->real_bm = AllocBitMap( xs, ys, depth, 0, ( masked || !destscreen ) ? NULL : destscreen->RastPort.BitMap );
	if( !bm->real_bm )
	{
		bm->real_bm = &fakebitmap;
	}
	else
	{
		bm->planesize = RASSIZE( xs, ys );
		bm->interleaved = GetBitMapAttr( bm->real_bm, BMA_FLAGS ) & BMF_INTERLEAVED;
		for( c = 0; c < src->Depth; c++ )
		{
			if( bm->interleaved )
				copyplane( src->Planes[ c ], bm->real_bm->Planes[ c ], src->BytesPerRow, bm->real_bm->BytesPerRow, src->Rows );
			else
				CopyMem( src->Planes[ c ], bm->real_bm->Planes[ c ], bm->planesize );
		}
	}

	ObtainSemaphore( &clonesem );
	ADDTAIL( &clonelist, bm );
	ReleaseSemaphore( &clonesem );

	clonecount++;

	return( bm->real_bm );
}

UBYTE *clone_maskplane( struct BitMap *maskbm )
{
	struct BitMap *cl;

	if( !maskbm || maskbm == (struct BitMap *)-1 )
		return( NULL );
	cl = getclone( maskbm, TRUE );
	if( !cl || cl == (struct BitMap *)-1 || !cl->Planes[ 0 ] )
		return( NULL );
	return( cl->Planes[ 0 ] );
}

void markclonemodified( struct BitMap *src )
{
	struct bmclone *bm;
	ObtainSemaphore( &clonesem );
	for( bm = FIRSTNODE( &clonelist ); NEXTNODE( bm ); bm = NEXTNODE( bm ) )
	{
		if( bm->orig_bm == src )
		{
			bm->modified++;
			ReleaseSemaphore( &clonesem );
			return;
		}
	}
	ReleaseSemaphore( &clonesem );
}
