#ifndef IMG_GLOBALS_H
#define IMG_GLOBALS_H
/*
 * MorphOS support
 * ---------------
 * Mostly there to replace gst.h for GCC
 *
 * $Id: globals.h,v 1.13 2003/04/25 19:13:59 zapek Exp $
 */

#ifndef MBX
#include "macros/compilers.h"
#endif /* !MBX */

/*
 * Includes
 */
#if defined( __GNUC__ ) && defined( __MORPHOS__ )

#include <exec/types.h>
#include "debug.h"

#include <exec/execbase.h>

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <devices/keyboard.h>
#include <dos/dostags.h>
#include <datatypes/soundclass.h>
#include <datatypes/pictureclass.h>
#include <graphics/gfxmacros.h>

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif /* min */

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif /* max */

//#include <ppcinline/cybergraphics.h>

#include <macros/vapor.h>

#endif /* __GNUC__ && __MORPHOS__ */


/*
 * That sucks but well... Should go in a separate common file
 * eventually..
 */
#ifndef MBX
#define rp_BitMap BitMap
#endif /* !MBX */
#define sc_ViewPort ViewPort
#define vp_ColorMap ColorMap


#endif /* !IMG_GLOBALS_H */
