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


#ifndef VOYAGER_NLIST_H
#define VOYAGER_NLIST_H
/*
 * $Id: nlist.h,v 1.4 2001/07/01 22:03:16 owagner Exp $
 */

extern char *listclass;
extern char *listviewclass;
extern int nlist;

#if USE_NLIST

void check_for_nlist( void );

#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#endif /* USE_NLIST */

#endif /* VOYAGER_NLIST_H */
