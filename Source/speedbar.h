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


#ifndef VOYAGER_SPEEDBAR_H
#define VOYAGER_SPEEDBAR_H
/*
 * $Id: speedbar.h,v 1.2 2001/07/01 22:03:32 owagner Exp $
 */

#if USE_SPEEDBAR
/*
 * The SpeedBar/SpeedButton headers ship in includestuff/mui, which is on the
 * include path. The old <speedbar/...> layout is not present in this tree.
 */
#include <mui/SpeedBar_mcc.h>
#include <mui/SpeedButton_mcc.h>
#endif /* USE_SPEEDBAR */

#endif /* VOYAGER_SPEEDBAR_H */
