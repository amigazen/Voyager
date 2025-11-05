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

#ifndef SUR_GAUGE_H
#define SUR_GAUGE_H
/*
 * $Id: sur_gauge.h,v 1.2 2001/09/12 10:51:41 zapek Exp $
 */

#ifdef __GNUC__
struct unode;
#endif

void sur_gauge_reset( struct unode *un );
void sur_gauge_clear( struct unode *un );
void sur_gauge_report( struct unode *un );
void sur_txt( struct unode *un, char *txt );
void STDARGS sur_text( struct unode *un, char *fmt, ... );
void sur_led( struct unode *un, ULONG state );

#endif /* !SUR_GAUGE_H */
