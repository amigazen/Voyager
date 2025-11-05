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


#ifndef VOYAGER_FASTCLASSES_H
#define VOYAGER_FASTCLASSES_H

/*
 * The purpose of this file is to add methods and tags while hacking
 * around and avoiding full rebuilds everytime something is changed.
 * This file is not taken into account into the dependencies generations.
 * Once you're done with your hacking session, please move the methods and tags
 * back into classes.h *cleanly*, then commit.
 *
 * Do not let stuff around for a long time.
 *
 * $Id: fastclasses.h,v 1.6 2001/09/30 22:51:03 zapek Exp $
 */

#ifdef __GNUC__
struct reqnode;
#endif

enum {
	MA_dummy_fast = MA_dummyend,

	/* put your crap here */
	MM_Downloadwin_SetState,
	MM_Downloadwin_RemoveEntry,
	MM_Downloadwin_FindEntry,

	MM_SmartReq_Enqueue,
	MM_SmartReq_Change,
	MM_SmartReq_Pressed,
	MM_SmartReq_Close,
	MA_SmartReq_Parent,
	MA_SmartReq_Ptr,

	MM_Downloadwin_InitReq,

	MA_SmartReq_MethodID,
	MM_SmartReq_Ask,

	MM_Downloadwin_ExitReq,
	MM_Downloadwin_FreeResume,

	MM_Downloadwin_AddFile,
	MM_Downloadwin_SelectFile,
	MM_Downloadwin_AddViewer,
	MM_Downloadwin_SelectViewer,
	MM_Downloadwin_View,

	MA_dummy_fast_end /* do not add any tag/method below ! */
};

struct MP_Downloadwin_SetState {
	ULONG MethodID;
	struct dlnode *dl;
	ULONG state;
};

struct MP_Downloadwin_RemoveEntry {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_Downloadwin_FindEntry {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_SmartReq_Enqueue {
	ULONG MethodID;
	struct reqnode *rn;
};

struct MP_SmartReq_Pressed {
	ULONG MethodID;
	LONG butnum;
	LONG userdata;
};

struct MP_SmartReq_Ask {
	ULONG MethodID;
	LONG butnum;
	LONG userdata;
};

struct MP_Downloadwin_InitReq {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_Downloadwin_ExitReq {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_Downloadwin_FreeResume {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_Downloadwin_AddFile {
	ULONG MethodID;
	struct dlnode *dl;
	STRPTR path;
	STRPTR filename;
};

struct MP_Downloadwin_AddViewer {
	ULONG MethodID;
	struct dlnode *dl;
	STRPTR path;
	STRPTR filename;
};

struct MP_Downloadwin_SelectFile {
	ULONG MethodID;
	struct dlnode *dl;
	STRPTR filename; /* without the path ! */
};

struct MP_Downloadwin_SelectViewer {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_Downloadwin_View {
	ULONG MethodID;
	struct dlnode *dl;
};

#endif /* VOYAGER_FASTCLASSES_H */
