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


#ifndef VOYAGER_MIME_H
#define VOYAGER_MIME_H
/*
 * $Id: mime.h,v 1.8 2001/09/23 22:42:20 zapek Exp $
*/

#if USE_VAT
#define mime_findbyextension VAT_MIME_FindByExtension
#define mime_findbytype VAT_MIME_FindByType
#else
enum {
	MT_ACTION_ASK,
	MT_ACTION_SAVE,
	MT_ACTION_VIEW,
	MT_ACTION_SAVE_AND_VIEW
};

#define MF_PIPE_STREAM 256
#define MF_VIEW_INLINE 512
int mime_findbyextension( char *filename, char *savedir, char *viewer, int *viewmode, char *mimetype );
int mime_findbytype( char *mimetype, char *savedir, char *viewer, int *viewmode );
#endif /* !USE_VAT */

void runmimeprefs( void );

#endif /* VOYAGER_MIME_H */
