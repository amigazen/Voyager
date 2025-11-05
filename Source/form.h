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


#ifndef VOYAGER_FORM_H
#define VOYAGER_FORM_H
/*
 * $Id: form.h,v 1.2 2001/07/01 22:02:43 owagner Exp $
*/

int formp_storedata( char *data, int len, int enctype );
char *formp_getdata( int id, int *lenp, int *inctype );
void formstore_add( char *url, int formelementid, int formsubid, char *data, int size );
char *formstore_get( char *url, int formelementid, int formsubid, int *size );

#endif /* VOYAGER_FORM_H */
