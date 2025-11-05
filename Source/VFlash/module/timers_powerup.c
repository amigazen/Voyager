/*
 * VFlash PowerUP timers
 * ---------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: timers_powerup.c,v 1.1.1.1 2002/10/07 12:52:15 zapek Exp $
 *
 */

#include "globals.h"
#include "timers.h"

extern struct ExecBase *SysBase;
#include <powerup/ppcinline/exec.h>
#include <powerup/ppclib/time.h>
static void *ppctimerobject;
static ULONG ppctimersig_handle;


/*
 * Initializes the timer
 */
int	init_timer(void)
{
	DB(("in inittimer()...\n"));

	if ((ppctimersig_handle = PPCAllocSignal(-1)) != -1)
	{
		DB(("timersig allocated signal number %ld\n", ppctimersig_handle));
		timersig = 1L << ppctimersig_handle;
		DB(("computed timersig looks like %lu\n", timersig));

		timerqueued = FALSE;
		return(TRUE);
	}
	return(FALSE);
}


/*
 * Resets a timer
 */
void rtimer(void)
{
	if (ppctimerobject)
	{
		DB(("old ppctimerobject deleted\n"));
		PPCDeleteTimerObject(ppctimerobject);
		ppctimerobject = NULL;
	}
	
	timerqueued = FALSE;
}


/*
 * Waits for the completion of a timer
 */
void wtimer(void)
{
	if (timerqueued)
	{
		PPCWait(timersig);
		timerqueued = FALSE;
	}
}


/*
 * Exits
 */
void cleanup_timer(void)
{
	if (ppctimerobject)
	{
		PPCDeleteTimerObject(ppctimerobject);
		ppctimerobject = NULL;
	}

	PPCFreeSignal(ppctimersig_handle);
}


/*
 * Queues a timer
 */
void qtimer(struct timeval *tv)
{
	struct TagItem 	ppctimertags[4];

	DB(("qtimer()...start\n"));

	if (ppctimerobject)
	{
		DB(("old ppctimerobject deleted\n"));
		PPCDeleteTimerObject(ppctimerobject);
	}

	DB(("tv_secs == %lu, tv_micro == %lu\n", tv->tv_secs, tv->tv_micro));
	DB(("computed interval == %lu ticks\n", tv->tv_secs * 50 + tv->tv_micro / 20000));

	PPCSetSignal(0, timersig);

	ppctimertags[0].ti_Tag	= PPCTIMERTAG_50HZ;
	ppctimertags[0].ti_Data = tv->tv_secs * 50 + tv->tv_micro / 20000;
	ppctimertags[1].ti_Tag	= PPCTIMERTAG_SIGNALMASK;
	ppctimertags[1].ti_Data	= timersig;
	ppctimertags[2].ti_Tag	= PPCTIMERTAG_AUTOREMOVE;
	ppctimertags[2].ti_Data	= TRUE;
	ppctimertags[3].ti_Tag	= TAG_END;

	ppctimerobject = PPCCreateTimerObject(ppctimertags);

	DB(("ppctimerobject started, timerqueued == TRUE\n"));
	timerqueued = TRUE;
}
