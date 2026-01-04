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
 * Simple snprintf implementation using utility.library VSNPrintf
 * $Id: snprintf.c,v 1.1 2003/07/06 16:51:33 olli Exp $
 */

#include <exec/types.h>
#include <proto/utility.h>
#include <stdarg.h>

#ifndef size_t
#define size_t ULONG
#endif

int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list args;
	int result;

	va_start(args, format);
	result = VSNPrintf(str, size, (const STRPTR)format, (APTR)args);
	va_end(args);

	return result;
}
