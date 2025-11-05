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


#ifndef VOYAGER_FTP_H
#define VOYAGER_FTP_H
/*
 * $Id: ftp.h,v 1.7 2001/09/09 19:31:21 zapek Exp $
 */

/* protos */
void un_setup_ftp( struct unode *un );
void un_doprotocol_ftp( struct unode *un );

/*
 * FTP protocol states
 */
enum {
	FPT_INIT,
	FPT_GOTINIT,
	FPT_USERREPLY,
	FPT_PWREPLY,
	FPT_PWDREPLY,
	FPT_PREABORREPLY,
	FPT_ABORREPLY,
	FPT_POSTABORREPLY,
	FPT_PASVREPLY,
	FPT_RESTREPLY,
	FPT_RETRREPLY,
	FPT_LISTREPLY,
	FPT_LISTREAD,
	FPT_CWDREPLY,
	FPT_BEGINREADDATA,
	FPT_READDATA,
	FPT_TYPEREPLY,
	FPT_REINIT_CWD,
	FPT_DOPARK
};

#endif /* VOYAGER_FTP_H */
