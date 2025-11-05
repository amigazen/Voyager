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


#ifndef VOYAGER_COOKIES_H
#define VOYAGER_COOKIES_H
/*
 * $Id: cookies.h,v 1.7 2001/07/01 22:02:37 owagner Exp $
 */

struct cookie {
	struct Node n;
	UWORD secure;
	char *name;
	char *data;
	char *server;
	char *path;
	time_t expires; // 0 = expires on end of session
	time_t age;
};

void cookie_process( struct cookiemsg *cm );
void cookie_set( char *line, char *realhost, char *realpath );
void cookie_checkclose( void );
void cookie_get( char *server, char *path, char *to, int isssecure );

#endif /* VOYAGER_COOKIES_H */
