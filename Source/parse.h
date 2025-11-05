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


#ifndef VOYAGER_PARSE_H
#define VOYAGER_PARSE_H
/*
 * $Id: parse.h,v 1.7 2001/07/01 22:03:16 owagner Exp $
*/

extern char *tokenargs_name, *tokenargs_id;
#define gettokenname() tokenargs_name
#define gettokenid() tokenargs_id

int gettokenarg_cnt( int cnt, char **name, char **val );
char *gettokenarg( char *from, char *what );
ULONG gettoken( char **textp, int *lineno );
char *getargs( char *token );
char *getargs_ne( char *token );
char *getargs_def( char *token, char *def );
char *getargsncv( char *token );
int getboolarg( char *token, int defval );
LONG STDARGS getstatearg( char *token, LONG default_not_there, LONG default_unknown, ... );
LONG getnumarg( char *token, LONG defval );
LONG getnumargp( char *token, LONG defval );
LONG getnumargmm( char *token, LONG defval, int minval, int maxval );
void convertentities( char *from, char *to );
void encodedata( char *from, char *to );

#if USE_LIBUNICODE
#include "layout.h"
void parse_setcurrentlayoutctx( struct layout_ctx *ctx );
#endif

#endif /* VOYAGER_PARSE_H */
