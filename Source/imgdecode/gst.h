/*
 * GST support, SAS/C only
 * -----------------------
 *
 * $Id: gst.h,v 1.8 2001/11/20 20:55:59 owagner Exp $
 */
#define __USE_SYSBASE
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
#include <exec/execbase.h>

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <proto/muimaster.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <devices/keyboard.h>
#include <dos/dostags.h>
#include <datatypes/soundclass.h>
#include <datatypes/pictureclass.h>
#include <graphics/gfxmacros.h>

#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>

#include <macros/vapor.h>

#define USE_BUILTIN_MATH
#include <time.h>
#include <string.h>
#include <constructor.h>
#include <ctype.h>
#include <stdlib.h>
#include <setjmp.h>

#include <proto/vat.h>

void __stdargs sprintf( char *to, const char *fmt, ... );

#define MAKE_ID(a,b,c,d)	\
	((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
					   type *name = (type *)((LONG)(a_##name+3) & ~3);

#define __callback __asm __saveds
#define _reg(x) register __##x

