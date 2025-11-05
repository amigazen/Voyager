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


#ifndef VOYAGER_REXX_H
#define VOYAGER_REXX_H
/*
 * $Id: rexx.h,v 1.9 2004/01/06 20:23:08 zapek Exp $
 */

/*
 * Name of the ARexx port and extentions
 */
#define VREXXPORT "VOYAGER"
#define VREXXEXT ".VRX"

/*
 * Maximum length of an ARexx command including the arguments
 */
#define VREXX_MAXLENGTH	2048 /* should be enough for everyone (tm) */

/*
 * Execution mode
 */
#define VREXX_FRAME  0  /* current frame, usualy context menus */
#define VREXX_WINDOW 1  /* current window, usualy toolbar and normal menus */

extern struct MUI_Command rexxcmds[];
extern ULONG command_runmode;
extern APTR rexx_obj;

int match_command( int type, STRPTR c, STRPTR s );
void execute_command( int type, STRPTR str, ULONG mode, STRPTR obj_url, STRPTR obj_link, STRPTR obj_window );

#endif /* VOYAGER_REXX_H */
