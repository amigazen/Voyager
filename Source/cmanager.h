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


#ifndef VOYAGER_CMANAGER_H
#define VOYAGER_CMANAGER_H

#if USE_CMANAGER

/*
 * $Id: cmanager.h,v 1.8 2001/07/01 22:02:36 owagner Exp $
 */

#ifdef __MORPHOS__
#include <libraries/CManager.h>
#include <proto/CManager.h>
#include <mui/CManager_mcc.h>
void magicfunc( void );
extern struct EmulLibEntry GATEmagicfunc;
#else
#include <cmanager/CManager.h>
#include <cmanager/Library/CManager_protos.h>
#include <cmanager/Library/CManager_lib.h>
#include <cmanager/MCC/CManager_mcc.h>
void ASM SAVEDS magicfunc( __reg( a0, STRPTR *str ), __reg( a1, ULONG *ver ), __reg( a2, ULONG **data ), __reg( a3, ULONG *datasize ) );
#endif /* !__MORPHOS__ */

#define MUIA_CManager_Magic     0xF76B0009

extern struct Library *CManagerBase;
extern APTR cm_obj;

void bm_load( char *filename );
void bm_save( int ask );
void bm_cleanup( void );
int bm_openwin( void );
int bm_create( void );

#endif /* USE_CMANAGER */

#endif /* VOYAGER_CMANAGER_H */
