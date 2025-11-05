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


#ifndef VOYAGER_HTTP_H
#define VOYAGER_HTTP_H
/*
 * $Id: http.h,v 1.5 2001/09/09 19:31:21 zapek Exp $
 */

/* protos */
void un_setup_http( struct unode *un, char *proxy_host, int proxy_port );
void un_setup_https( struct unode *un );
void un_doprotocol_http( struct unode *un );

/*
 * HTTP protocol states
 */
enum {
	HTTP_SEND_REQUEST,
	HTTP_WAIT_REPLY,
	HTTP_READ_HEADER,
	HTTP_GOT_HEADER,
	HTTP_BEGIN_READ_DATA,
	HTTP_READ_DATA_10,
	HTTP_READ_CHUNK_11,
	HTTP_READ_NEXT_CHUNK_11,
	HTTP_READ_POST_CHUNK_11,
	HTTP_DO_SSL_HANDSHAKE = 20 /* no idea why it's 20.. shrug */
};

#endif /* VOYAGER_HTTP_H */
