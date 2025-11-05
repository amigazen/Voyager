/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_MALLOC_H
#define VOYAGER_MALLOC_H
/*
 * $Id: malloc.h,v 1.14 2003/04/25 19:13:55 zapek Exp $
*/

#if USE_MALLOC
#ifndef MBX
void *malloc( size_t size );
void free( void *memb );
void *realloc( void *old, size_t nsize );
char *strdup( const char *string );
#endif
#else
#include <stdlib.h>
#endif

STRPTR StrDupPooled( APTR pool, STRPTR instring );

#endif /* VOYAGER_MALLOC_H */
