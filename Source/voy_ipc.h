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


#ifndef VOYAGER_VOY_IPC_H
#define VOYAGER_VOY_IPC_H
/*
 * Rexxless-Voyager-IPC
 *
 * $Id: voy_ipc.h,v 1.5 2001/07/01 22:03:32 owagner Exp $
 */

#define VCMD_GOTOURL 1
#define VCMD_GOTOURL_FLAG_NEWWIN (1<<0)

#define VIPCNAME "webbrowser_ipc"

struct voyager_msg {
	struct Message m;
	ULONG cmd;
	char *parms;
	ULONG flags;
	ULONG reserved[ 13 ];
};

#endif /* VOYAGER_VOY_IPC_H */
