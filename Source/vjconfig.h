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


#ifndef VOYAGER_JCONFIG_H
#define VOYAGER_JCONFIG_H
/*
 * $Id: vjconfig.h,v 1.2 2001/07/01 22:03:32 owagner Exp $
 */

/* jconfig.sas --- jconfig.h for Amiga systems using SAS C 6.0 and up. */
/* see jconfig.doc for explanations */

#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
/* #define void char */
/* #define const */
#undef CHAR_IS_UNSIGNED
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS
#undef NEED_SHORT_EXTERNAL_NAMES
#undef INCOMPLETE_TYPES_BROKEN

#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED

#define TEMP_DIRECTORY "JPEGTMP:"   /* recommended setting for Amiga */

#define NO_MKTEMP       /* SAS C doesn't have mktemp() */

#define SHORTxSHORT_32      /* produces better DCT code with SAS C */

#endif /* JPEG_INTERNALS */

#ifdef JPEG_CJPEG_DJPEG

#define BMP_SUPPORTED       /* BMP image file format */
#define GIF_SUPPORTED       /* GIF image file format */
#define PPM_SUPPORTED       /* PBMPLUS PPM/PGM image file format */
#undef RLE_SUPPORTED        /* Utah RLE image file format */
#define TARGA_SUPPORTED     /* Targa image file format */

#define TWO_FILE_COMMANDLINE
#define NEED_SIGNAL_CATCHER
#undef DONT_USE_B_MODE
#undef PROGRESS_REPORT      /* optional */

#define NO_GETENV

#endif /* JPEG_CJPEG_DJPEG */

#endif /* VOYAGER_JCONFIG_H */
