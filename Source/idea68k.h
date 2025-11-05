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


#ifndef VOYAGER_IDEA68K_H
#define VOYAGER_IDEA68K_H
/*
 * $Id: idea68k.h,v 1.4 2001/07/01 22:02:56 owagner Exp $
 */

#define IDEA_MODE_ENCRYPT 1
#define IDEA_MODE_DECRYPT 0

#define IDEA_SIZE_PASSWORD  16
#define IDEA_SIZE_SCHEDULE 216
#define IDEA_SIZE_IVEC       8

VOID STDARGS idea_key_schedule( APTR password,APTR schedule);
VOID STDARGS idea_cbc_encrypt(APTR src,APTR dst,int len,APTR keyschedule,APTR ivec,int mode);

#endif  /* VOYAGER_IDEA68K_H */
