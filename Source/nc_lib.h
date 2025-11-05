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


#ifndef VOYAGER_NC_LIB_H
#define VOYAGER_NC_LIB_H
/*
 * $Id: nc_lib.h,v 1.4 2001/07/01 22:03:16 owagner Exp $
 */

extern struct Library *NetConnectBase;

#pragma libcall NetConnectBase NCL_OpenSocket 36 0
#pragma libcall NetConnectBase NCL_GetSerial 42 0
#pragma libcall NetConnectBase NCL_GetOwner 4e 0
#pragma libcall NetConnectBase NCL_CallMeSometimes 54 0
#pragma libcall NetConnectBase NCL_CallMeFrequently 5a 0

struct Library *NCL_OpenSocket( void );
char *NCL_GetSerial( void );
char *NCL_GetOwner( void );
void NCL_CallMeSometimes( void );
void NCL_CallMeFrequently( void );

#endif /* VOYAGER_NC_LIB_H */
