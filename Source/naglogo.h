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


#ifndef VOYAGER_NAGLOGO_H
#define VOYAGER_NAGLOGO_H
/*
 * $Id: naglogo.h,v 1.6 2001/07/01 22:03:16 owagner Exp $
 */

extern const ULONG __far reglogo_colors[96];

#define REGLOGO_WIDTH        64
#define REGLOGO_HEIGHT       43
#define REGLOGO_DEPTH         5
#define REGLOGO_COMPRESSION   1
#define REGLOGO_MASKING       2

extern const struct BitMapHeader __far reglogo_header;

extern const UBYTE __far reglogo_body[1755];

#endif /* VOYAGER_NAGLOGO_H */
