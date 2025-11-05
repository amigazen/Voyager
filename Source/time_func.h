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


#ifndef VOYAGER_TIME_H
#define VOYAGER_TIME_H
/*
 * $Id: time_func.h,v 1.7 2001/08/29 14:05:23 owagner Exp $
*/

time_t timev( void );
long UtPack(const char *x);
char *datestamp2string( struct DateStamp *ds );
char *date2string( time_t t );
time_t convertrfcdate( char *uudate );
#ifdef MBX
#define timed timev
#define timedm() (timev()*1000)
#define getlocaltime GetSysTime
#else
time_t timed( void );
ULONG timedm( void );
void getlocaltime( struct timeval *dest );
#endif

#endif /* VOYAGER_TIME_H */
