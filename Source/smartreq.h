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

/*
 * $Id: smartreq.h,v 1.2 2001/09/16 22:26:14 zapek Exp $
 */

#ifndef VOYAGER_SMARTREQ_H
#define VOYAGER_SMARTREQ_H

int STDARGS smartreq_request( APTR *obj, STRPTR title, APTR winobj, ULONG methodid, LONG userdata, STRPTR gadgets, STRPTR format, ... );

#endif /* VOYAGER_SMARTREQ_H */

