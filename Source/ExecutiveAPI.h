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


#ifndef VOYAGER_EXECUTIVE_API_H
#define VOYAGER_EXECUTIVE_API_H
/*
**      $VER: ExecutiveAPI.h 1.00 (03.09.96)
**      ExecutiveAPI Release 1.00
**
**      ExecutiveAPI definitions
**
**      Copyright © 1996-97 Petri Nordlund. All rights reserved.
**
**      $Id: ExecutiveAPI.h,v 1.4 2001/07/01 22:02:35 owagner Exp $
**
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif /* EXEC_PORTS_H */


/*
 * Public message port to send messages to
 *
 */
#define EXECUTIVEAPI_PORTNAME 	"Executive_server"


/*
 * ExecutiveMessage
 *
 */
struct ExecutiveMessage {
	struct Message	message;
	WORD		ident;		/* This must always be 0		*/

	WORD		command;	/* Command to be sent, see below	*/
	struct Task	*task;		/* Task address				*/
	STRPTR		taskname;	/* Task name				*/
	LONG		value1;		/* Depends on command			*/
	LONG		value2;		/* Depends on command			*/
	LONG		value3;		/* Depends on command			*/
	LONG		value4;		/* Depends on command			*/
	WORD		error;		/* Non-zero if error, see below		*/

	LONG		reserved[4];	/* Reserved for future use		*/
};


/*
 * Commands
 *
 */
enum {
	EXAPI_CMD_ADD_CLIENT = 0,	/* Add new client				*/
	EXAPI_CMD_REM_CLIENT,		/* Remove client				*/

	EXAPI_CMD_GET_NICE,		/* Get nice-value				*/
	EXAPI_CMD_SET_NICE,		/* Set nice-value				*/

	EXAPI_CMD_GET_PRIORITY,		/* Get task's correct (not scheduling) priority	*/

	EXAPI_CMD_WATCH,		/* Schedle, don't schedle etc. See below	*/
};


/*
 * These are used with EXAPI_CMD_WATCH
 *
 */

/* --> value1 */
enum {
	EXAPI_WHICH_TASK = 0,		/* Current task					*/
	EXAPI_WHICH_CHILDTASKS,		/* Childtasks of this task			*/
};

/* --> value2 */
enum {
	EXAPI_TYPE_SCHEDULE = 0,	/* Schedule	this task / childtasks		*/
	EXAPI_TYPE_NOSCHEDULE,		/* Don't schedule this task / childtasks	*/
	EXAPI_TYPE_RELATIVE,		/* Childtasks' priority relative to parent's	*/
					/* priority.					*/
};

/* --> value3 */
/* These are only used with EXAPI_TYPE_NOSCHEDULE */
enum {
	EXAPI_PRI_LEAVE_ALONE = 0,	/* Ignore task priority				*/
	EXAPI_PRI_ABOVE,		/* Task's priority kept above scheduled tasks	*/
	EXAPI_PRI_BELOW,		/* Task's priority kept below scheduled tasks	*/
	EXAPI_PRI_SET			/* Set priority to given value (value4)		*/
};


/*
 * Errors
 *
 */
enum {
	EXAPI_OK = 0,			/* No error					*/
	EXAPI_ERROR_TASK_NOT_FOUND,	/* Specified task wasn't found			*/
	EXAPI_ERROR_NO_SERVER,		/* Server not available (quitting)		*/
	EXAPI_ERROR_INTERNAL,		/* Misc. error (e.g. no memory)			*/
	EXAPI_ERROR_ALREADY_WATCHED,	/* Task is already being watched, meaning that	*/
					/* user has put the task to "Executive.prefs".	*/
};

#endif /* VOYAGER_EXECUTIVE_API_H */
