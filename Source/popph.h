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


#ifndef VOYAGER_POPPH_H
#define VOYAGER_POPPH_H
/*
 * $Id: popph.h,v 1.2 2001/07/01 22:03:17 owagner Exp $
 */

#if USE_POPPH
#ifdef __MORPHOS__
#include <mui/popplaceholder_mcc.h>
#else
#include <popph/popplaceholder_mcc.h>
#endif /* !__MORPHOS__ */
#endif /* USE_POPPH */

#endif /* VOYAGER_POPPH_H */
