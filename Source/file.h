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


#ifndef VOYAGER_FILE_H
#define VOYAGER_FILE_H
/*
 * $Id: file.h,v 1.6 2001/07/01 22:02:43 owagner Exp $
*/

extern BPTR currentdir_lock;

void setcomment( STRPTR filename, STRPTR comment );
#ifdef __MORPHOS__
int STDARGS mySystemTags( char *, ... ) __attribute__((varargs68k));
#else
int STDARGS mySystemTags( char *, ... );
#endif /* !__MORPHOS__ */

#endif /* VOYAGER_FILE_H */
