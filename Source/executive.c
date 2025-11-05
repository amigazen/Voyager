/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


/*
**  Executive Support
**
**  © 1999 by David Gerber <zapek@meanmachine.ch>
**  All Rights Reserved
**
**  $Id: executive.c,v 1.13 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"
#include "ExecutiveAPI.h"

#if USE_EXECUTIVE

#define PICASSO96_SUX /* workaround for a P96 stupid behaviour */

/*
** This bug appears on some configs when there are many steps to launch applications.
** The launcher can have a negative priority (scheduled by Executive). The launched
** process gets the same priority. There are some tricks in Executive to work around
** this but it fails most of the time (at least on my config). This workaround seems
** to help in some cases but not all. Anyway, it helps for people using Executive.
**
** Update: Picasso96 was the culprit, define renamed :)
*/

/* support functions */

static BOOL SendMessage(struct ExecutiveMessage *message)
{
	struct MsgPort *port;

	Forbid();

	if(port = FindPort(EXECUTIVEAPI_PORTNAME))
	{
		PutMsg(port, (struct Message *) message);
		Permit();
		WaitPort(message->message.mn_ReplyPort);
		while(GetMsg(message->message.mn_ReplyPort));
		if(!message->error)
			return(TRUE);
	}
	else
	{
		/** Executive is not running **/
		Permit();
	}

	return(FALSE);
}


/* ------------------------------- ExitExecutive -------------------------------

 Bye bye

*/
void ExitExecutive(struct ExecutiveMessage *msg)
{
	/* remove the client */
	if (msg && (msg->message.mn_ReplyPort))
	{
		msg->command = EXAPI_CMD_REM_CLIENT;

		SendMessage(msg);
	}

	/* delete the message */
	if (msg)
	{
		if (msg->message.mn_ReplyPort)
			DeleteMsgPort(msg->message.mn_ReplyPort);

		FreeVec(msg);
	}
}

/* ------------------------------- InitExecutive -------------------------------

  Become an Executive client and prevent it to exit

*/
APTR InitExecutive(VOID)
{
	struct ExecutiveMessage *msg;
	struct MsgPort *port;
	if(!(msg = AllocVec(sizeof(struct ExecutiveMessage), MEMF_PUBLIC | MEMF_CLEAR)))
	{
		ExitExecutive(msg);
		return(FALSE);
	}

	if(!(msg->message.mn_ReplyPort = CreateMsgPort()))
	{
		ExitExecutive(msg);
		return(FALSE);
	}

#ifdef PICASSO96_SUX
	Forbid();
	if(port = FindPort(EXECUTIVEAPI_PORTNAME))
	{
		Permit();
		SetTaskPri(FindTask(NULL), 0);
	}
	else
	{
		Permit();
	}
#endif /* PICASSO96_SUX */

	msg->message.mn_Node.ln_Type = NT_MESSAGE;
	msg->message.mn_Length       = sizeof(struct ExecutiveMessage);
	msg->ident = 0;

	/* Current program as Executive client to prevent him exiting */
	msg->command = EXAPI_CMD_ADD_CLIENT;

	if(!(SendMessage(msg)))
	{
		ExitExecutive(msg);
		return(NULL);
	}

	return(msg);
}


/* -------------------------------- GetTaskPri ---------------------------------

 Get current task's real priority (Exec Priority the task  was created with, not
 the scheduled one)

*/
#if 0

LONG GetTaskPri(struct ExecutiveMessage *msg)
{
	msg->command = EXAPI_CMD_GET_PRIORITY;
	msg->task    = FindTask(NULL);

	if(!(SendMessage(msg)))
	{
		return(-129); /* bogus value to indicate it went wrong */
	}

	return(msg->value1);
}
#endif

/* ---------------------------------- SetNice ----------------------------------

 Set the nice value of the current task (range from -20 to +20), returns the old
 nice value (remember to restore it on your program's exit)

*/
LONG SetNice(struct ExecutiveMessage *msg, LONG nicevalue)
{
	msg->command = EXAPI_CMD_SET_NICE;
	msg->task    = FindTask(NULL);
	msg->value1  = nicevalue;

	if(!(SendMessage(msg)))
	{
		return(-21); /* bogus value to indicate it went wrong */
	}

	return(msg->value1);
}

#if 0
/* ---------------------------------- GetNice ----------------------------------

 Get the nice value of the current task (range from -20 to +20)

*/
LONG GetNice(struct ExecutiveMessage *msg)
{
	msg->command = EXAPI_CMD_GET_NICE;
	msg->task    = FindTask(NULL);

	if(!(SendMessage(msg)))
	{
		return(-21); /* bogus value to indicate it went wrong */
	}

	return(msg->value1);
}

#endif


/* ------------------------------- SchedulerMode -------------------------------

 Change the scheduling mode

 mode- EXAPI_TYPE_SCHEDULE    - Schedule this task / childtasks (normal mode)
	   EXAPI_TYPE_NOSCHEDULE  - Don't schedule this task / childtasks
	   EXAPI_TYPE_RELATIVE    - Childtasks' priority relative to parent's

*/
LONG SchedulerMode(struct ExecutiveMessage *msg, LONG mode, LONG option)
{
	msg->command = EXAPI_CMD_WATCH;
	msg->task    = FindTask(NULL);
	msg->value1  = EXAPI_WHICH_TASK; /* current task only */
	msg->value2  = mode;
	msg->value3  = option;

	if(!(SendMessage(msg)))
	{
		return(-2);
	}

	if (msg->error == EXAPI_ERROR_ALREADY_WATCHED)
		return(-1); // entry was already added, better just ignore it
	else
		return(0);
}
#endif /* USE_EXECUTIVE */
