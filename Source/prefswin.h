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


#ifndef VOYAGER_PREFSWIN_H
#define VOYAGER_PREFSWIN_H
/*
 * $Id: prefswin.h,v 1.19 2001/10/10 20:42:48 zapek Exp $
 */

/*
 * Beware, including that header changes the behaviour of functions
 * manipulating preferences.
 */

#include "voyager_cat.h"

#include "prefs.h"
#include "classes.h"

/*
 * Number of prefs pages
 */
#if USE_NET
#define PREFSWIN_NUMPAGES 17
#else
#define PREFSWIN_NUMPAGES 8
#endif /* !USE_NET */

#define getprefs getprefs_clone
#define getprefslong getprefslong_clone
#define getprefsstr getprefsstr_clone
#define setprefs setprefs_clone
#define setprefslong setprefslong_clone
#define setprefsstr setprefsstr_clone
#define setprefsstr_menu setprefsstr_clone_menu

/*
 * Common functions
 */
void storestring( APTR strobj, ULONG pid );
void storeattr( APTR obj, ULONG attr, ULONG pid );
APTR pstring( ULONG pid, int maxlen, char *label );
APTR ppopph( ULONG pid, int maxlen, char **array );
APTR pinteger( ULONG pid, char * label );
APTR pcycle( STRPTR *opts, ULONG pid, char *label );
APTR pcheck( ULONG pid, char *label );
APTR pchecki( ULONG pid, char *label );
void fillstrarray( STRPTR *to, ULONG msg, int num );
void setupd( APTR o, ULONG attr );
void cleanupcerts( int doit );


/*
 * Common vars/structs
 */
typedef APTR (*GRPFUNC)(void);

struct prefsgroup {
	ULONG labelid;
	GRPFUNC class;
	struct BitMap *bm;
};

extern ULONG __far prefsimages_cmap[24];
extern int needdisplayupdate;
extern int num_buttons;
extern struct prefsgroup prefsgroups[ PREFSWIN_NUMPAGES ];

#endif /* VOYAGER_PREFSWIN_H */
