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


#ifndef VOYAGER_CERTS_H
#define VOYAGER_CERTS_H
/*
 * $Id: certs.h,v 1.7 2003/04/25 19:13:53 zapek Exp $
 */

#include "vssl.h"

/*
 * The following is needed because
 * VSSLBase isn't used globally
 * everywhere.
 */
#ifdef __MORPHOS__
#define USE_INLINE_STDARG
#include "vssl_inline.h"
#undef USE_INLINE_STDARG
#endif

#include "network.h"

void certreq_process( struct certmsg *cm );
void certreq_checkclose( void );
int certreq_ask( X509 *cert, char *error, VSSLBASE, struct unode *un );
void cert_getinfo( X509 *cert, char *to, VSSLBASE );
#endif /* VOYAGER_CERTS_H */
