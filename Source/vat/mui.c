#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/rexxsyslib.h>
#include <proto/muimaster.h>
#include <proto/asl.h>
#include <exec/execbase.h>
#include <libraries/mui.h>
#include <devices/timer.h>
#include <string.h>
#include <dos/dostags.h>
#include <proto/openurl.h>
#include <ctype.h>
#include <macros/vapor.h>

#include "globals.h"

void ASM SAVEDS VAT_MultiSetA(
	__reg( d0, ULONG attr ),
	__reg( d1, ULONG val ),
	__reg( a0, APTR *objs )
)
{
	DB( ( "called\n" ) );
	
	while( *objs )
	{
		SetAttrs( *objs++, attr, val, TAG_DONE );
	}
}

extern UWORD fmtfunc[];

void ASM SAVEDS VAT_SetFmtA(
	__reg( a0, APTR obj ),
	__reg( d0, ULONG attr ),
	__reg( a1, STRPTR fmt ),
	__reg( a2, APTR args )
)
{
	char buffer[ 2048 ];

	DB( ( "called\n" ) );

	RawDoFmt( fmt, args, (APTR)fmtfunc, buffer );
	
	SetAttrs( obj, attr, ( ULONG )buffer, TAG_DONE );
}

void ASM SAVEDS VAT_SetTxtFmtA(
	__reg( a0, APTR obj ),
	__reg( a1, STRPTR fmt ),
	__reg( a2, APTR args )
)
{
	char buffer[ 2048 ];

	DB( ( "called\n" ) );
	RawDoFmt( fmt, args, (APTR)fmtfunc, buffer );

	SetAttrs( obj, MUIA_Text_Contents, ( ULONG )buffer, TAG_DONE );
}

struct Screen * ASM SAVEDS VAT_GetAppScreen( __reg( a0, APTR app ) )
{
	struct Screen *scr;
	char *pubname;

	DB( ( "called\n" ) );

	if( MUIMasterBase->lib_Version >= 13 )
	{
		struct List *l;
		APTR ostate, o;

		get( app, MUIA_Application_WindowList, &l );
		ostate = l->lh_Head;
		while( o = NextObject( &ostate ) )
		{
			scr = 0;
			get( o, MUIA_Window_Screen, &scr );
			if( scr )
				return( scr );
		}
	}

	// Fuck

	pubname = 0;
	get( app, MUIA_Application_PubScreenName, &pubname );
	scr = LockPubScreen( pubname );
	UnlockPubScreen( NULL, scr );

	return( scr );
}

char * ASM SAVEDS VAT_GetAppScreenName( __reg( a0, APTR app ) )
{
	struct Screen *scr = VAT_GetAppScreen( app );
	
	DB( ( "called\n" ) );
	
	if( scr )
	{
		struct List *psl = LockPubScreenList();
		struct PubScreenNode *psn;		

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
