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


#ifndef VOYAGER_ERRORWIN_H
#define VOYAGER_ERRORWIN_H
/*
 * Error handling
 * --------------
 *
 * All errors are handled through a function called puterror() which
 * is non-blocking and configurable by the user. See errorwin.c
 *
 * $Id: errorwin.h,v 1.6 2001/07/01 22:02:42 owagner Exp $
 *
*/

extern APTR errorwin;

extern void puterror( ULONG type, ULONG level, LONG errorcode, STRPTR url, STRPTR message );

/*
 * Message type.
 * Don't forget to add the types in voyager.cd
 * MSG_ERRORWIN_ERRORTYPE_...
 */
enum {
	LT_NET,
	LT_JS,
	LT_HTML,
	LT_INTERNAL,
	LT_IMAGE,
	LT_CACHE,
};

/*
 * Error log level.
 * Don't forget to add the levels in voyager.cd
 * MSG_ERRORWIN_ERRORLEVEL_...
 */
enum {
	LL_INFO,
	LL_WARNING,
	LL_ERROR,
};

#endif /* VOYAGER_ERRORWIN_H */
