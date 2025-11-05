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


#ifndef VOYAGER_URLPARSER_H
#define VOYAGER_URLPARSER_H
/*
 * $Id: urlparser.h,v 1.10 2003/04/25 19:13:56 zapek Exp $
 */

struct parsedurl {
	char *scheme;
	char *host;
	char *username;
	char *password;
	char *path;
	char *args;
	int port;
	char *fragment;
	int pathrelative;
};

int url_hasscheme( char *url );
void uri_split( char *url, struct parsedurl *out );
void uri_remerge( struct parsedurl *u, char *to );
void uri_mergeurl( char *from, char *add, char *to );
void uri_canon( char *from, char *to );
void uri_decode( char *url );

#define MAXURLSIZE 2048 /* TOFIX: make this dynamic, big work */

#endif /* VOYAGER_URLPARSER_H */
