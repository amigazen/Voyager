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


#ifndef VOYAGER_FRAMESET_H
#define VOYAGER_FRAMESET_H
/*
 * $Id: frameset.h,v 1.4 2001/07/01 22:02:43 owagner Exp $
 */

void fset_setsize( char *url, int iternum, int xs, int ys );
void fset_gethinfo( char *url, int iternum, char *weights );
void fset_getvinfo( char *url, int iternum, char *weights );
void fset_setweights( char *url, int iternum, int *x_weights, int *y_weights, int xnum, int ynum, int xs, int ys );
void fset_setaweight( char *url, int iternum, int irow, int icol, int orient, int delta );

void fsu_pushurl( char *topurl, char *name, char *url );
void fsu_addurl( char *topurl, char *name, char *url );
char *fsu_popurl( char *topurl, char *name );
int fsu_hasprev( char *topurl, char *name );
int fsu_hasnext( char *topurl, char *name );
void fsu_delall( char *topurl );
char *fsu_nexturl( char *topurl, char *name );

#endif /* VOYAGER_FRAMESET_H */
