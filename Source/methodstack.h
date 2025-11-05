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


#ifndef VOYAGER_METHODSTACK_H
#define VOYAGER_METHODSTACK_H
/*
 * $Id: methodstack.h,v 1.4 2001/07/01 22:03:16 owagner Exp $
*/

void STDARGS pushmethod( APTR obj, ULONG cnt, ... );
ULONG SAVEDS pushsyncmethod( APTR obj, APTR msg );
void killpushedmethods( APTR obj );
void checkmethods( void );

#endif /* VOYAGER_METHODSTACK_H */
