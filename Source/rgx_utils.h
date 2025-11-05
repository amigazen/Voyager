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


/* utility definitions */
#ifdef _POSIX2_RE_DUP_MAX
#define	DUPMAX	_POSIX2_RE_DUP_MAX
#else
#define	DUPMAX	255
#endif
#define	INFINITY	(DUPMAX + 1)
#define	NC		(CHAR_MAX - CHAR_MIN + 1)
typedef unsigned char uch;

/* switch off assertions (if not already off) if no REDEBUG */
#ifndef REDEBUG
#ifndef NDEBUG
#define	NDEBUG	/* no assertions please */
#endif
#endif
#include <assert.h>

/* for old systems with bcopy() but no memmove() */
#ifdef USEBCOPY
#define	memmove(d, s, c)	bcopy(s, d, c)
#endif
