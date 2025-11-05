/*
 * MorphOS library support
 * -----------------------
 *
 * © 2000 by David Gerber <zapek@vapor.com>
 * All rights reserved
 *
 * $Id: lib.c,v 1.7 2003/04/25 19:13:59 zapek Exp $
*/

#if defined( __MORPHOS__ )

#include "lib.h"
#include "rev.h"

extern ULONG LibFuncTable[];
extern APTR pool;

struct Library*	LIB_Init(struct LibBase	*VimgdecodeBase, BPTR SegList, struct ExecBase *SBase);

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
	"vimgdec_604e.vlib", //TOFIX!!
	"vimgdec_604e.vlib " LVERTAG,
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
struct Library*	LIB_Init(struct LibBase *VimgdecodeBase, BPTR SegList, struct ExecBase *SBase)
{
	VimgdecodeBase->SegList = SegList;
	VimgdecodeBase->SBase = SBase;
	
	if( lib_init( SBase ) )
	{
		VimgdecodeBase->Lib.lib_Node.ln_Pri = -128;
		VimgdecodeBase->Lib.lib_Revision = REVISION;

		if( pool = CreatePool( 0, 4096, 2048 ) )
		{
			return( &VimgdecodeBase->Lib );
		}
	}
	return( 0 );
}


/*
 * The following is needed because it's also called by LIB_Close() with
 * PPC args
 */
ULONG libexpunge( struct LibBase *VimgdecodeBase )
{
	BPTR MySegment;

	MySegment =	VimgdecodeBase->SegList;

	if( VimgdecodeBase->Lib.lib_OpenCnt )
	{
		VimgdecodeBase->Lib.lib_Flags |= LIBF_DELEXP;
		return(NULL);
	}

	Forbid();
	Remove(&VimgdecodeBase->Lib);
	Permit();

	lib_cleanup();

	FreeMem((APTR)((ULONG)(VimgdecodeBase) - (ULONG)(VimgdecodeBase->Lib.lib_NegSize)),
		VimgdecodeBase->Lib.lib_NegSize + VimgdecodeBase->Lib.lib_PosSize);

	return( (ULONG)MySegment );
}


ULONG LIB_Expunge(void)
{
	struct LibBase *VimgdecodeBase = (struct LibBase *)REG_A6;

	return( libexpunge(VimgdecodeBase));
}


struct Library * LIB_Open( void )
{
	struct LibBase	*VimgdecodeBase = (struct LibBase *)REG_A6;

	VimgdecodeBase->Lib.lib_Flags &= ~LIBF_DELEXP;
	VimgdecodeBase->Lib.lib_OpenCnt++;
	return(&VimgdecodeBase->Lib);
}


ULONG LIB_Close( void )
{
	struct LibBase	*VimgdecodeBase = (struct LibBase *)REG_A6;

	if ((--VimgdecodeBase->Lib.lib_OpenCnt) > 0)
	{
		//close done
	}
	else
	{
		if (VimgdecodeBase->Lib.lib_Flags & LIBF_DELEXP)
		{
			return(libexpunge(VimgdecodeBase));
		}
	}
	return( 0 );
}


ULONG LIB_Reserved( void )
{
	return( 0 );
}

#endif /* __MORPHOS__ */
