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


#ifndef VOYAGER_COPYRIGHT_H
#define VOYAGER_COPYRIGHT_H
/*
 * Copyright and version infos
 * ---------------------------
 * - This files contain all the copyright infos,
 *   and version strings.
 *
 * $Id: copyright.h,v 1.2 2001/07/01 22:02:37 owagner Exp $
 */

#if defined( MBX )
#define APPNAME "WWW"
#elif defined( __MORPHOS__ )
#define APPNAME "Voyager PPC"
#else
#define APPNAME "Voyager"
#endif

#ifndef DEPEND
#include "rev.h"
#endif /* !DEPEND */

#endif /* VOYAGER_COPYRIGHT_H */
