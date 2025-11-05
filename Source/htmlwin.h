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


#ifndef VOYAGER_HTMLWIN_H
#define VOYAGER_HTMLWIN_H
/*
 * $Id: htmlwin.h,v 1.6 2001/07/01 22:02:55 owagner Exp $
 */

/* Global Window vars */
extern struct MinList winlist;
extern APTR lastactivewin;
extern APTR lastactivepanel;
extern APTR veryfirstwin;

APTR createwindow( char *url, char *referer );
APTR STDARGS newcreatewindow( char *url, char *referer, ... );

#endif /* VOYAGER_HTMLWIN_H */
