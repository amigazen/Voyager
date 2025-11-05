/*
 * Timer controls
 *
 * $Id: timers_amiga.c,v 1.2 2003/04/27 03:13:10 zapek Exp $
 */

#include "globals.h"
#include "mempool.h"
#include "../debug.h"
#include <proto/exec.h>
#include <devices/timer.h>
#include "timers.h"


struct Library *TimerBase;
static struct timerequest *TimerIO;


/*
 * Initializes the timer
 */
int	init_timer(void)
{
	DB(("in inittimer()...\n"));

	if (TimerIO = (struct timerequest *)myAllocPooled(mempool, sizeof(struct timerequest)))
	{
		DB(("memory allocated for timer\n"));
		memset(TimerIO, 0, sizeof(TimerIO));
		
		DB(("creating msgport..\n"));
		if (TimerIO->tr_node.io_Message.mn_ReplyPort = CreateMsgPort())
		{
			DB(("creating timer..\n"));
			if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest*)TimerIO, 0) == 0)
			{
				DB(("timer.device opened\n"));
				TimerBase = (struct Library *)TimerIO->tr_node.io_Device;
				timersig = 1L << TimerIO->tr_node.io_Message.mn_ReplyPort->mp_SigBit;
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
		AbortIO((struct IORequest*)TimerIO);
		WaitIO((struct IORequest*)TimerIO);
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
		WaitIO((struct IORequest *)TimerIO);
		timerqueued = FALSE;
	}
}


/*
 * Exits
 */
void cleanup_timer(void)
{
	rtimer();
	if (TimerBase)
	{
		CloseDevice((struct IORequest*)TimerIO);
		DeleteMsgPort(TimerIO->tr_node.io_Message.mn_ReplyPort);
	}
}


/*
 * Queues a timer
 */
void qtimer(struct timeval *tv)
{
	DB(("qtimer()...start\n"));
	
	SetSignal(0, timersig);

	TimerIO->tr_time = *tv;
	TimerIO->tr_node.io_Command = TR_ADDREQUEST;
	SendIO((struct IORequest*)TimerIO);
	timerqueued = TRUE;
	DB(("sent\n"));
}
