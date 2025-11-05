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


#ifndef VOYAGER_DOWNLOAD_H
#define VOYAGER_DOWNLOAD_H
/*
 * $Id: download.h,v 1.5 2001/08/15 17:36:54 zapek Exp $
*/

void queue_download( STRPTR url, STRPTR referer, int dlonly, int askpath );
int open_downloadwin( void );

#endif /* VOYAGER_DOWNLOAD_H */
