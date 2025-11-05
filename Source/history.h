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


#ifndef VOYAGER_HISTORY_H
#define VOYAGER_HISTORY_H
/*
 * $Id: history.h,v 1.2 2001/07/01 22:02:44 owagner Exp $
*/

void addurlhistory( char *url );
time_t checkurlhistory( char *url );
void flushurlhistory( void );
char *findurlhismatch( char *what );

#endif /* VOYAGER_HISTORY_H */
