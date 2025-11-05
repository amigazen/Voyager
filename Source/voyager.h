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


#ifndef VOYAGER_VOYAGER_H
#define VOYAGER_VOYAGER_H
/*
 * $Id: voyager.h,v 1.96 2001/08/15 20:14:33 zapek Exp $
 */

#include "config.h" /* This will include gst.h if appropriate */

#if defined( __MORPHOS__ ) || defined( AMIGAOS )
#include "globals.h"
#endif

#include "debug.h"

/*
 * Global static data
 */
extern APTR app, notify;
extern int iscybermap;

/*
 * Often used functions (do not put crap here! think twice!)
 */
void displaybeep( void );
void STDARGS reporterror( char *msg, ... );
#ifdef __SASC
int	snprintf(char *, unsigned int, const char *, ...);
#endif /* !snprintf */


/*
 * MUI-IDs (TOFIX: get rid of all MUI Ids. Bad coding style)
 */
enum {
	ID_dummy = 1,

	ID_CHECKWINREMOVE, //TOFIX!! removed
	ID_CHECKWINACTIVE,
	ID_CLOSEBOOKMARKS,
	ID_SAVECLOSEBOOKMARKS,

	ID_PM_FLUSH_URLS,
	ID_PM_FLUSH_CACHE, //TOFIX!! removed
	ID_PM_FLUSH_MEM,   //TOFIX!! removed

	ID_BM_OPEN
};


#endif /* VOYAGER_VOYAGER_H */

/*

// This is a dodgy, dodgy hack

static ULONG __stdargs myDoMethod( APTR obj, ULONG m, ... )
{
	if( !obj )
	{
		kprintf( "calling dm(0)!\n", *((char*)obj) );
	}
	return( DoMethodA( obj, &m ) );
}

#define DoMethod myDoMethod
*/
