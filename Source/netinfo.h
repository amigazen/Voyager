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


#ifndef VOYAGER_NETINFO_H
#define VOYAGER_NETINFO_H
/*
 * $Id: netinfo.h,v 1.4 2001/07/01 22:03:16 owagner Exp $
*/

extern APTR win_ni;
extern struct SignalSemaphore netinfosem; /* always use it when changing structures that are used by netinfo */

void netinfo_setmax( int m, int maxv );
void netinfo_setprogress( int m, int progv );
void netinfo_url( struct unode *un );
void netinfo_clear( int m );
void netinfo_parked( int m );

#endif /* VOYAGER_NETINFO_H */
