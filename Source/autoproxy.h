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


#ifndef VOYAGER_AUTOPROXY_H
#define VOYAGER_AUTOPROXY_H
/*
 * $Id: autoproxy.h,v 1.2 2001/07/01 22:02:35 owagner Exp $
*/

char *proxy_for_url( char *url, struct parsedurl *purl, int *proxyport );
void proxy_init( void );

#endif /* VOYAGER_AUTOPROXY_H */
