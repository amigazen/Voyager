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


#ifndef VOYAGER_AUTHIPC_H
#define VOYAGER_AUTHIPC_H
/*
 * $Id: authipc.h,v 1.8 2001/07/01 22:02:35 owagner Exp $
 */

/* doesn't really belong there, but well... */
struct authnode {
	struct MinNode n;
	char server[ 256 ];   // hostname
	char realm[ 256 ];    // realm
	char authdata[ 256 ]; // user+pass
	char authuser[ 128 ]; // user
	char authpass[ 128 ]; // password
	int save; 			  // save to disk ?
};

struct authmsg {
	struct Message m;
	int method;
	int quitme;
	char realm[ 256 ];
	char server[ 256 ];
	char authdata[ 80 ];
	int failedalready;
	int rc;
	int ftpmode;
};

extern ULONG authportsigmask;
extern struct MsgPort *authport;

int auth_query( struct parsedurl *purl, char *server, char *realm, int failedalready, char *authdata, int ftpmode );
void auth_process( void );
int pauth_get( char *server, char *realm, char *authdata, int ftpmode );
#endif /* VOYAGER_AUTHIPC_H */
