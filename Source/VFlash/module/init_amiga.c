/*
 * VFlash portable initializations & cleanups
 * ------------------------------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: init_amiga.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#include "globals.h"
#include "init.h"
#include "support.h"

struct Library			*MUIMasterBase;
struct DosLibrary		*DOSBase;
struct IntuitionBase	*IntuitionBase;


int init_libs(void)
{
	if (MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19))
	{
		if (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39))
		{
			if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSNAME, 39))
			{
				return (TRUE);
			}
		}
	}
	return (FALSE);
}


void remove_dosreq(void)
{
#if defined(AMIGAOS) || defined(__MORPHOS__)
	struct Process *pr = (struct Process *)FindTask(NULL);
	oldwinptr = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR)-1;
#endif
}


void enable_dosreq(void)
{
#if defined(AMIGAOS) || defined(__MORPHOS__)
	if (oldwinptr)
	{
		struct Process *pr = (struct Process *)FindTask(NULL);
		pr->pr_WindowPtr = oldwinptr;
	}
#endif
}


void close_libs(void)
{
	if (DOSBase)
	{
		CloseLibrary((struct Library *)DOSBase);
	}

	if (MUIMasterBase)
	{
		CloseLibrary(MUIMasterBase);
	}

	if (IntuitionBase)
	{
		CloseLibrary((struct Library *)IntuitionBase);
	}
}
