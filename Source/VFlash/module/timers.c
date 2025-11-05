/*
 * VFlash portable timers
 * ----------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: timers.c,v 1.1.1.1 2002/10/07 12:52:16 zapek Exp $
 *
 */

#include "globals.h"
#include "timers.h"

ULONG timersig;
int timerqueued;
