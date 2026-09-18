/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

**************************************************************************/

/*
 * Bounded sprintf. utility.library SNPrintf/VSNPrintf are V47+ only.
 */

#include "voyager.h"
#include <stdarg.h>

#ifdef __SASC
#include <proto/exec.h>

struct snbuf
{
	char *p;
	char *end;
};

static void ASM sn_putc( __reg( d0, UBYTE c ), __reg( a3, struct snbuf *s ) )
{
	if( s->p < s->end )
		*s->p++ = (char)c;
}

int voy_vsnprintf( char *str, unsigned int size, const char *format, APTR args )
{
	struct snbuf s;

	if( !str || size == 0 )
		return( 0 );
	s.p = str;
	s.end = str + size - 1;
	RawDoFmt( (STRPTR)format, args, (void (*)())sn_putc, &s );
	*s.p = 0;
	return( (int)( s.p - str ) );
}

int STDARGS snprintf( char *str, unsigned int size, const char *format, ... )
{
	va_list args;
	int n;

	va_start( args, format );
	n = voy_vsnprintf( str, size, format, (APTR)args );
	va_end( args );
	return( n );
}

#else /* !__SASC */

#include <proto/utility.h>

int voy_vsnprintf( char *str, unsigned int size, const char *format, APTR args )
{
	return( VSNPrintf( str, size, (const STRPTR)format, args ) );
}

int snprintf( char *str, unsigned int size, const char *format, ... )
{
	va_list args;
	int result;

	va_start( args, format );
	result = VSNPrintf( str, size, (const STRPTR)format, (APTR)args );
	va_end( args );
	return( result );
}

#endif /* __SASC */
