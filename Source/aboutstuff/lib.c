/*
 * $Id: lib.c,v 1.1 2003/06/22 23:04:48 zapek Exp $
 */

//#include "globals.h"

#include "rev.h"
#include "lib.h"

/*
 * Ah well, I'm tired of cut & past so define the stuff
 * and copy the file somewhere else.
 */
#define LIBNAME      "voyager_about.vlib"
#define LIBCOPYRIGHT "© 1998-2003 by Oliver Wagner & David Gerber"
#define LIBPRI 0
#define LIBBASE VAboutBase

char verstr[] = { "$VER: " LIBNAME " " LVERTAG " " LIBCOPYRIGHT };

extern ULONG LibFuncTable[];

struct Library*	LIB_Init(struct LibBase	*LIBBASE, BPTR SegList, struct ExecBase *SBase);

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
	(void (*)(void))&LIB_Init
};


struct Resident libresident = {
	RTC_MATCHWORD,
	&libresident,
	&libresident + 1,
	RTF_PPC | RTF_EXTENDED | RTF_AUTOINIT | RTF_AFTERDOS,
	VERSION,
	NT_LIBRARY,
	0,
	LIBNAME,
	LIBNAME " " LVERTAG " "LIBCOPYRIGHT,
	&libinit_struct,
	/* new fields */
	REVISION,
	NULL
};

/*
 * To tell the loader that this is a new emulppc elf and not
 * one for the ppc.library.
 */
ULONG __abox__ = 1;


/*
 * Library functions (system)
 */
struct Library*	LIB_Init(struct LibBase *LIBBASE, BPTR SegList, struct ExecBase *SBase)
{
	LIBBASE->SegList = SegList;
	LIBBASE->SBase = SBase;

	if (lib_init(SBase))
	{
		LIBBASE->Lib.lib_Node.ln_Pri = LIBPRI;

		return (&LIBBASE->Lib);
	}
	return (0);
}


/*
 * The following is needed because it's also called by LIB_Close() with
 * PPC args
 */
ULONG libexpunge(struct LibBase *LIBBASE)
{
	BPTR MySegment;

	MySegment =	LIBBASE->SegList;

	if (LIBBASE->Lib.lib_OpenCnt)
	{
		LIBBASE->Lib.lib_Flags |= LIBF_DELEXP;
		return (NULL);
	}

	Forbid();
	Remove((struct Node *)&LIBBASE->Lib);
	Permit();

	lib_cleanup();

	FreeMem((APTR)((ULONG)(LIBBASE) - (ULONG)(LIBBASE->Lib.lib_NegSize)),
		LIBBASE->Lib.lib_NegSize + LIBBASE->Lib.lib_PosSize);

	return ((ULONG)MySegment);
}


static ULONG opened;


ULONG LIB_Expunge(void)
{
	struct LibBase *LIBBASE = (struct LibBase *)REG_A6;

	opened = FALSE;

	return (libexpunge(LIBBASE));
}



struct Library * LIB_Open(void)
{
	struct LibBase	*LIBBASE = (struct LibBase *)REG_A6;

	if (!opened)
	{
		opened = TRUE;
		
		if (!lib_open())
		{
			lib_cleanup();
			return (0);
		}
	}

	LIBBASE->Lib.lib_Flags &= ~LIBF_DELEXP;
	LIBBASE->Lib.lib_OpenCnt++;
	return (&LIBBASE->Lib);
}


ULONG LIB_Close(void)
{
	struct LibBase	*LIBBASE = (struct LibBase *)REG_A6;

	if ((--LIBBASE->Lib.lib_OpenCnt) == 0)
	{
		if (LIBBASE->Lib.lib_Flags & LIBF_DELEXP)
		{
			return (libexpunge(LIBBASE));
		}
	}
	return (0);
}


ULONG LIB_Reserved(void)
{
	return (0);
}

