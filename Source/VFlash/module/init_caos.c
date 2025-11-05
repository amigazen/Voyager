/*
 * VFlash portable initializations & cleanups
 * ------------------------------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: init_caos.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#include "globals.h"
#include "init.h"
#include <z_lib_calls.h>

MUIBASE;
INSPIRATIONBASE;
ZBASE;

int init_libs(void)
{
	if (MUIBase = (MUIData_p)OpenModule(MUINAME, MUIVERSION))
	{
		if (ZBase = (VPTR)OpenModule(ZNAME, ZVERSION))
		{
			if (InspirationBase = (InspirationData_p)OpenModule(INSPIRATIONNAME, INSPIRATIONVERSION))
			{
				return (TRUE);
			}
		}
	}
	return (FALSE);
}


void remove_dosreq(void)
{
	return; /* nothing */
}


void enable_dosreq(void)
{
	return; /* nothing */
}


void close_libs(void)
{
	if (InspirationBase)
	{
		CloseModule((struct Module *)InspirationBase);
	}

	if (ZBase)
	{
		CloseModule((struct Module *)ZBase);
	}

	if (MUIBase)
	{
		CloseModule((struct Module *)MUIBase);
	}
}
