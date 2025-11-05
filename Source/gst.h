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


#ifndef VOYAGER_GST_H
#define VOYAGER_GST_H
/*
 * Global Symbol Table
 * -------------------
 * - used to speed up SAS/C.
 *
 * $Id: gst.h,v 1.66 2003/04/25 19:13:53 zapek Exp $
 */

#define __USE_SYSBASE

/*
 * Normal SAS/C protos
 */
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/timer.h>
#include <proto/rexxsyslib.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/iffparse.h>
#include <proto/layers.h>
#include <proto/keymap.h>
#include <proto/commodities.h>
#include <proto/input.h>
#include <proto/wbstart.h>
#include <proto/battclock.h>
#if USE_CGX
#include <cybergraphx/cybergraphics.h>
//#include <proto/cybergraphics>
#endif /* USE_CGX */

/*
 * Common stuff
 */
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <intuition/classes.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <clib/alib_protos.h>

/* _DCC -- Hack to get MUIC_xxx defines as extern char[] */
#define _DCC
extern struct Library *MUIMasterBase;
#include "//morphos/mui/muimaster_protos.h"
#include "//morphos/mui/muimaster_pragmas.h"
#include "//morphos/mui/mui.h"
#undef _DCC

#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/locale.h>

#include <dos/dostags.h>
#include <dos/datetime.h>
#include <dos/dosextens.h>
#include <dos/doshunks.h>
#include <dos/exall.h>

#include <datatypes/pictureclass.h>
#if INCLUDE_VERSION < 44
#include <datatypes/pictureclassext.h>
#endif
#include <graphics/gfxmacros.h>
#include <graphics/scale.h>
#include <graphics/clip.h>
#include <graphics/text.h>


#define USE_BUILTIN_MATH
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>

/* MUI classes */
#include "listtree.h"
#include "busy.h"
#include "textinput.h"
#include "speedbar.h"
#include "nlist.h"
#include "popph.h"


#if USE_EXECUTIVE
#include "executive_protos.h"
#endif /* USE_EXECUTIVE */

#define VAPOR_H_BROKEN
#include <macros/vapor.h>

/*
 * SAS/C bug workaround. Only happens on 68020+
 */
#if ( __SASC && __M68020 )
#undef memcmp
#endif

#endif /* VOYAGER_GST_H */
