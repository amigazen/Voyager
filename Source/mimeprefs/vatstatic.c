#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <intuition/screens.h>
#include "mui.h"
#include <cl/lists.h>
#include "vatstatic.h"

extern struct Library *MUIMasterBase;

/*
 * exec.library did not export AllocVecPooled on the 3.1 proto
 * vat/sup_generic.c uses. Size is stored in the long before the
 * returned block, same layout as later exec.
 */
APTR AllocVecPooled( APTR poolheader, ULONG memsize )
{
	ULONG *block;

	block = AllocPooled( poolheader, memsize + sizeof( ULONG ) );
	if( !block )
		return( NULL );
	*block = memsize;
	return( block + 1 );
}

void FreeVecPooled( APTR poolheader, APTR memory )
{
	ULONG *block;

	if( !memory )
		return;
	block = (ULONG *)memory - 1;
	FreePooled( poolheader, block, *block + sizeof( ULONG ) );
}

APTR VUP_BeginCheckUpdate( ULONG product, ULONG version, STRPTR name )
{
	return( NULL );
}

void VUP_Quit( APTR handle )
{
}

struct Screen *VAT_GetAppScreen( APTR app )
{
	struct Screen *scr;
	char *pubname;
	struct List *l;
	APTR ostate, o;

	scr = NULL;
	if( MUIMasterBase && MUIMasterBase->lib_Version >= 13 )
	{
		get( app, MUIA_Application_WindowList, &l );
		ostate = l->lh_Head;
		while( o = NextObject( &ostate ) )
		{
			scr = NULL;
			get( o, MUIA_Window_Screen, &scr );
			if( scr )
				return( scr );
		}
	}

	pubname = NULL;
	get( app, MUIA_Application_PubScreenName, &pubname );
	scr = LockPubScreen( pubname );
	UnlockPubScreen( NULL, scr );
	return( scr );
}

STRPTR VAT_GetAppScreenName( APTR app )
{
	struct Screen *scr;
	struct List *psl;
	struct PubScreenNode *psn;

	scr = VAT_GetAppScreen( app );
	if( scr )
	{
		psl = LockPubScreenList();
		for( psn = FIRSTNODE( psl ); NEXTNODE( psn ); psn = NEXTNODE( psn ) )
		{
			if( psn->psn_Screen == scr )
				break;
		}
		UnlockPubScreenList();
		if( NEXTNODE( psn ) )
			return( psn->psn_Node.ln_Name );
	}
	return( "Workbench" );
}
