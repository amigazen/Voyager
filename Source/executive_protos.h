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


#ifndef CLIB_EXECUTIVE_PROTOS_H
#define CLIB_EXECUTIVE_PROTOS_H

/*
**  Executive Support
**
**  © 1999 by David Gerber <zapek@meanmachine.ch>
**  All Rights Reserved
**
**  $Id: executive_protos.h,v 1.4 2001/07/01 22:02:42 owagner Exp $
**
*/

#include "ExecutiveAPI.h"

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#include <proto/exec.h>

APTR InitExecutive(VOID);
LONG GetTaskPri(struct ExecutiveMessage *);
LONG SetNice(struct ExecutiveMessage *, LONG);
LONG GetNice(struct ExecutiveMessage *);
LONG SchedulerMode(struct ExecutiveMessage *, LONG, LONG);
VOID ExitExecutive(struct ExecutiveMessage *);

#endif /* CLIB_EXECUTIVE_PROTOS_H */
