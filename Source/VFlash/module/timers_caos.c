/*
 * VFlash CaOS timers
 * ------------------
 *
 * © 2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: timers_caos.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#include "globals.h"
#include "timers.h"

#include <system_lib_calls.h>
#include <timer_drv_calls.h>

#if 0

MsgQueue_p queue;
TimeRequest_p iotr;
TIMERBASE;


/*
 * Initializes the timer
 */
int init_timer(void)
{
	DB(("in init_timer()...\n"));

	if ((queue = CreateMsgQueue("VFlash Timer", 0, TRUE)))
	{
		if ((iotr = (TimeRequest_p)CreateExtIO(queue, sizeof(TimeRequest_s))))
		{
			if (OpenDriver(TIMERNAME, UNIT_MICROHZ, (DrvIOReq_p)iotr, 0) == 0)
			{
				TimerBase = (TimerData_p)iotr->tr_DrvIOReq.ior_Driver;
				timersig = 1L << queue->mqu_SignalBit;
				DB(("timersig is %lu\n", timersig));
				timerqueued = FALSE;
				return (TRUE);
			}
		}
	}
	return (FALSE);
}


/*
 * Resets the timer
 */
void rtimer(void)
{
	if (timerqueued)
	{
		if ( AbortIO((DrvIOReq_p)iotr) == NULL )
			WaitIO((DrvIOReq_p)iotr);
		timerqueued = FALSE;
	}
}


/*
 * Waits for the completion of a timer
 */
void wtimer(void)
{
	if (timerqueued)
	{
		WaitIO((DrvIOReq_p)iotr);
		timerqueued = FALSE;
	}
}


/*
 * Exits
 */
void cleanup_timer(void)
{
	DB(("cleaning up...\n"));

	rtimer();

	if (TimerBase)
	{
		CloseDriver((DrvIOReq_p)iotr);
	}

	if (iotr)
	{
		DeleteExtIO((DrvIOReq_p)iotr);
	}

	if (queue)
	{
		DeleteMsgQueue(queue, TRUE); //TOFIX: check the true at the end
	}
}


/*
 * Queues a timer
 */
void qtimer(struct timeval *tv)
{
	DB(("start..\n"));

	SetSignal(0, timersig);

	iotr->tr_Time.tv_Micros = (tv->tv_sec * SYSTICKS_PER_SECOND) + tv->tv_usec;
	iotr->tr_DrvIOReq.ior_Command = TR_ADDREQUEST;
	SendIO((DrvIOReq_p)iotr);
	timerqueued = TRUE;
	DB(("sent\n"));
}

#else
void cleanup_timer(void)
{
}
#endif