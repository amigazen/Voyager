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


#define VABOUT_NAME "PROGDIR:Plugins/voyager_about.vlib"
#define VABOUT_VERSION 3

#ifdef __SASC
#pragma libcall VAboutBase VABOUT_GetAboutPtr 1e A9803
#pragma libcall VAboutBase VABOUT_GetVLogo 24 801
#pragma libcall VAboutBase VABOUT_GetSSLLogo 2a 801
#pragma libcall VAboutBase VABOUT_GetPNGLogo 30 801
#pragma libcall VAboutBase VABOUT_GetAboutIbeta 36 0
#pragma libcall VAboutBase VABOUT_GetV3Logo 3c 801
#pragma libcall VAboutBase VABOUT_GetFlashLogo 42 801
#endif

STRPTR VABOUT_GetAboutPtr( STRPTR revid, STRPTR owner, STRPTR imgdeclib );
APTR VABOUT_GetVLogo( int *sizeptr );
APTR VABOUT_GetV3Logo( int *sizeptr );
APTR VABOUT_GetFlashLogo( int *sizeptr );
APTR VABOUT_GetSSLLogo( int *sizeptr );
APTR VABOUT_GetPNGLogo( int *sizeptr );
STRPTR VABOUT_GetAboutIbeta( void );

