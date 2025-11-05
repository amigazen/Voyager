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


#ifndef VOYAGER_MUI_FUNC_H
#define VOYAGER_MUI_FUNC_H
/*
 * Custom MUI functions and macros
 * -------------------------------
 * - This header should be included for any file using MUI related thing
 *
 * - Please don't add useless stuff here. Use direct MUI macros/functions
 * whenever possible because they're the same for all apps.
 *
 * $Id: mui_func.h,v 1.9 2003/04/25 19:13:55 zapek Exp $
 *
*/

#include <proto/intuition.h>

/* _DCC -- Hack to get MUIC_xxx defines as extern char[] */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#define _DCC
extern struct Library *MUIMasterBase;
#include <intuition/intuition.h>
#include <proto/muimaster.h>
#include <libraries/mui.h>
#ifdef AMIGAOS
#include <clib/alib_protos.h>
#endif
#ifdef __MORPHOS__
#include "alib_protos_replacement.h"
#endif
#include <proto/utility.h>
#undef _DCC
#endif


#include "mui_macros.h"

char *getpubname( APTR winobj );
APTR hbar( void );
APTR makebutton( ULONG stringid );
int getmenucheck( ULONG menuid );
STRPTR mui_getstrptr( APTR obj );
#define getstrp(o) mui_getstrptr(o)

ULONG mui_getv( APTR o, ULONG a );
#define getv(o,a) mui_getv(o,a)

char ParseHotKey(char *string_num);

APTR button( ULONG label, ULONG helpid );
APTR ebutton( ULONG label, ULONG helpid );
#ifdef __MORPHOS__
void STDARGS getobjectdimensions( APTR class, APTR wobj, int *xsize, int *ysize, ... ) __attribute__((varargs68k));
#else
void STDARGS getobjectdimensions( APTR class, APTR wobj, int *xsize, int *ysize, ... );
#endif /* !__MORPHOS__ */
APTR string( STRPTR def, int maxlen, int sh );
void tickapp( void );

#ifdef __MORPHOS__
APTR STDARGS DoSuperNew( struct IClass *cl, APTR obj, ... ) __attribute__((varargs68k));
#else
APTR STDARGS DoSuperNew( struct IClass *cl, APTR obj, ULONG tag1, ... );
#endif

char *filter_escapecodes( char *src );

#endif /* VOYAGER_MUI_FUNC_H */
