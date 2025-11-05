/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


/*
 * Handles the locale settings
 *
 * $Id: locale.c,v 1.12 2003/07/06 16:51:33 olli Exp $
 */

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/locale.h>
#include <proto/exec.h>
#include <proto/locale.h>
//#include <locale.h>
#endif

/* private */
#include "config.h"
#include "v_locale.h"
#include "voyager_cat.h"

struct Locale *locale;
struct Catalog *catalog;
#ifndef MBX
#if (INCLUDE_VERSION >= 44) && !defined(__MORPHOS__)
struct LocaleBase *LocaleBase;
#else
struct Library *LocaleBase;
#endif /* (INCLUDE_VERSION >= 44) || defined(__MORPHOS__) */
#endif /* MBX */
int locale_timezone_offset;

int init_locale( void )
{
	D( db_init, bug( "initializing..\n" ) );

	/*
	 * We set a default locale base
	 */
#ifdef __MORPHOS__
//	  setlocale( LC_ALL, "C" );
#endif

#ifdef MBX
	if( !LocaleBase )
		return( TRUE );
#else
	LocaleBase = (struct LocaleBase *)OpenLibrary( "locale.library", 37 );
#endif
	if( !LocaleBase )
		return( FALSE );

	locale = OpenLocale( NULL );

	if( locale )
		locale_timezone_offset = -locale->loc_GMTOffset * 60;

	catalog = OpenCatalog(
		NULL, "Voyager.catalog",
		OC_BuiltInLanguage, "english",
		OC_Version, 3,
		TAG_DONE
	);

	if( catalog )
	{
		int c;

		for( c = 0; c < NUMCATSTRING; c++ )
			((char**)__stringtable)[ c ] = GetCatalogStr( catalog, c, (char*)__stringtable[ c ] );
	}
	return( TRUE );
}

void close_locale( void )
{
	D( db_init, bug( "cleaning up..\n" ) );

	if( locale )
	{
		CloseLocale( locale );
	}

	if( catalog )
	{
		CloseCatalog( catalog );
	}
#ifndef MBX
	if( LocaleBase )
	{
		CloseLibrary( (struct Library *)LocaleBase );
	}
#else
//TOFIX!!
#endif
}
