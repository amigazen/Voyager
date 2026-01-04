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


#ifndef VOYAGER_TEAROFF_H
#define VOYAGER_TEAROFF_H
/*
 * $Id: tearoff.h,v 1.6 2001/07/01 22:03:32 owagner Exp $
 */

#if USE_TEAROFF
#include <mui/TearOffBay_mcc.h>
#include <mui/tearoffpanel_mcc.h>

/*
 * General
 */
extern APTR tearoff_dataspace;


/*
 * Functions
 */
void savetearoff( STRPTR filename );
void loadtearoff( STRPTR filename );
#endif /* USE_TEAROFF */

#endif /* VOYAGER_TEAROFF_H */
