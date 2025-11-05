/*
 * VFlash global defines
 * ---------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: globals.h,v 1.7 2004/02/29 17:04:14 zapek Exp $
 *
 */

#include "config.h"
#include "../debug.h"

extern struct Library		   *MUIMasterBase;
extern struct DosLibrary	   *DOSBase;
extern struct IntuitionBase	   *IntuitionBase;


#include <exec/exec.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <dos/exall.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>

#ifdef POWERUP
#if (DEBUG == 0)
#include <clib/alib_stdio_protos.h>
#endif
#else /* !POWERUP */
#ifndef __MORPHOS__
#include <clib/alib_stdio_protos.h>
#endif
#endif /* !POWERUP */

#include <cybergraphx/cybergraphics.h>
#include <libraries/mui.h>
#include <intuition/intuition.h>

#ifdef POWERUP
#include <powerup/gcclib/powerup_protos.h>
#include <powerup/ppclib/interface.h>
#include <powerup/ppclib/tasks.h>
#include <powerup/ppclib/memory.h>
#include <powerup/ppclib/message.h>
#endif


/*
 * Some handy macros
 */
#ifdef POWERUP
#define myCreatePool PPCCreatePool
#define myAllocPooled PPCAllocPooled
#define myFreePooled PPCFreePooled
#define myDeletePool PPCDeletePool
#else /* !POWERUP */
#define myCreatePool CreatePool
#define myAllocPooled AllocPooled
#define myFreePooled FreePooled
#define myDeletePool DeletePool
#endif /* !POWERUP */


#include "rev.h"

/*
 * OS dependent parts
 */
#include "flash_mcc_private.h"
#include "flash.h"

ULONG domethoda(APTR obj, ...);

extern struct vplug_functable *ft;

#define domethod(_obj, ...) \
	({ULONG _msg[] = { __VA_ARGS__}; \
	ft->vplug_domethoda(_obj, _msg);})

#undef set

#define set(_obj, _attr, _val) \
	({struct opSet _s; \
	struct TagItem _tags[2]; \
	_tags[0].ti_Tag = _attr; \
	_tags[0].ti_Data = _val; \
	_tags[1].ti_Tag = TAG_DONE; \
	_s.MethodID = OM_SET; \
	_s.ops_AttrList = _tags; \
	ft->vplug_domethoda(_obj, &_s);})

#if USE_REENTRANCY
#include "reentrant.h"
#endif /* USE_REENTRANCY */
