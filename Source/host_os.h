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


#ifndef VOYAGER_HOST_OS_H
#define VOYAGER_HOST_OS_H
/*
 * $Id: host_os.h,v 1.3 2001/07/01 22:02:44 owagner Exp $
*/

extern char cpuid[ 12 ];
extern char hostos[ 8 ];

void find_host_os( void );

#endif /* VOYAGER_HOST_OS_H */
