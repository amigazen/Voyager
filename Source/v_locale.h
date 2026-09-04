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


#ifndef VOYAGER_LOCALE_H
#define VOYAGER_LOCALE_H
/*
 * $Id: v_locale.h,v 1.1 2001/07/04 16:59:40 zapek Exp $
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

extern struct Locale *locale;
extern struct Catalog *catalog;

#ifndef MBX
STRPTR voyager_catalog_str( ULONG msgid, STRPTR builtin );
#endif /* !MBX */

#endif /* VOYAGER_LOCALE_H */
