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

#ifndef LINEREAD_H
#define LINEREAD_H

#ifdef __GNUC__
struct unode;
#endif /* __GNUC__ */

#define LINEREADBUFFERSIZE 2048

void initlineread( struct unode *un );
void initlineread_pasv( struct unode *un );
void exitlineread( struct unode *un );
void reinitlineread( struct unode *un );
void dolineread( struct unode *un );
void purgedata( struct unode *un, int amount );
void purgeline( struct unode *un );


#endif /* !LINEREAD_H */
