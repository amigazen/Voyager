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


#ifndef VOYAGER_HISTORYLIST_H
#define VOYAGER_HISTORYLIST_H
/*
 * $Id: historylist.h,v 1.5 2001/07/01 22:02:44 owagner Exp $
 */

/* history related */

/*
 * A mainpage is that is added to the HistorylistObject.
 * They are unique within it. If there are frames, there's a
 * frameset structure which keeps a list of frame.
 * Everytime the user changes a frame, a new frameset structure
 * is created.
 */

struct history_mainpage {
	struct MinNode node;
	char *url;                  /* URL */
	ULONG x;					/* virtgroup X position */
	ULONG y;                	/* virtgroup Y position */
	struct MinList frameset;
};

struct history_frameset {
	struct MinNode node;
	ULONG x;
	ULONG y;
	struct MinList framelist;
};

struct history_frame {
	struct MinNode node;
	char *name;
	char *url;
	ULONG x;
	ULONG y;
};

#endif /* VOYAGER_HISTORYLIST_H */
