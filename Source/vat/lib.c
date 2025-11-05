/*
 * MorphOS library support
 * -----------------------
 *
 * © 2001 by David Gerber <zapek@vapor.com>
 * All rights reserved
 *
 * $Id: lib.c,v 1.7 2002/10/28 14:35:52 zapek Exp $
*/

#if defined( __MORPHOS__ )

#include "lib.h"
#include "rev.h"
#include "vat_debug.h"

char verstr[] = { "$VER: vapor_toolkit.library " LVERTAG " PPC version by David Gerber" };

extern ULONG LibFuncTable[];

struct Library*	LIB_Init(struct LibBase	*VATBase, BPTR SegList, struct ExecBase *SBase);

struct libinitstruct
{
	ULONG	LibSize;
	void	*FuncTable;
	void	*DataTable;
	void	(*InitFunc)(void);
};

struct libinitstruct libinit_struct =
{
	sizeof(struct LibBase),
	LibFuncTable,
	NULL,
	(void (*)(void)) &LIB_Init
};


struct Resident libresident = {
	RTC_MATCHWORD,
	&libresident,
	&libresident + 1,
	RTF_PPC | RTF_AUTOINIT,
	VERSION,
	NT_LIBRARY,
	0,
	"vapor_toolkit.library",
	"vapor_toolkit.library " LVERTAG " (PPC version by David Gerber)",
	&libinit_struct
};

/*
 * To tell the loader that this is a new emulppc elf and not
 * one for the ppc.library.
 */
ULONG __amigappc__ = 1;
ULONG __abox__ = 1;


/*
 * Library functions (system)
 */
struct Library*	LIB_Init(struct LibBase *VATBase, BPTR SegList, struct ExecBase *SBase)
{
	VATBase->SegList = SegList;
	VATBase->SBase = SBase;

	if( lib_init( SBase ) )
	{
		VATBase->Lib.lib_Node.ln_Pri = -111;
		VATBase->Lib.lib_Revision = REVISION;

		return( &VATBase->Lib );
	}
	return( 0 );
}


/*
 * The following is needed because it's also called by LIB_Close() with
 * PPC args
 */
ULONG libexpunge( struct LibBase *VATBase )
{
	BPTR MySegment;

	MySegment =	VATBase->SegList;

	if( VATBase->Lib.lib_OpenCnt )
	{
		VATBase->Lib.lib_Flags |= LIBF_DELEXP;
		return(NULL);
	}

	Forbid();
	Remove(&VATBase->Lib);
	Permit();

	lib_cleanup();

	FreeMem((APTR)((ULONG)(VATBase) - (ULONG)(VATBase->Lib.lib_NegSize)),
		VATBase->Lib.lib_NegSize + VATBase->Lib.lib_PosSize);

	return( (ULONG)MySegment );
}


ULONG LIB_Expunge(void)
{
	struct LibBase *VATBase = (struct LibBase *)REG_A6;
	ULONG rc;

	DB( ( "called\n" ) );

	rc = libexpunge( VATBase );

	return( rc );
}


extern struct Library *TimerBase;

struct Library * LIB_Open( void )
{
	struct LibBase	*VATBase = (struct LibBase *)REG_A6;
	
	//DB( ( "called (task == %s)\n", FindTask( NULL )->tc_Node.ln_Name ) );

	if( !TimerBase )
	{
		DB( ( "calling lib open\n" ) );
		if( !lib_open() )
		{
			DB( ( "libopen failed\n" ) );
			lib_cleanup();
			return( 0 );
		}
		DB( ( "successfully opened\n" ) );
	}

	DB( ( "already opened\n" ) );

	VATBase->Lib.lib_Flags &= ~LIBF_DELEXP;
	VATBase->Lib.lib_OpenCnt++;
	
	return(&VATBase->Lib);
}


ULONG LIB_Close( void )
{
	struct LibBase	*VATBase = (struct LibBase *)REG_A6;

	DB( ( "called (task == %s)\n", FindTask( NULL )->tc_Node.ln_Name ) );

	if ((--VATBase->Lib.lib_OpenCnt) == 0)
	{
		if (VATBase->Lib.lib_Flags & LIBF_DELEXP)
		{
			return(libexpunge(VATBase));
		}
	}
	return( 0 );
}


ULONG LIB_Reserved( void )
{
	return( 0 );
}

#endif /* __MORPHOS__ */
