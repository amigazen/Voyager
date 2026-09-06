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
#include <exec/tasks.h>
#include <proto/exec.h>
#include <proto/locale.h>
//#include <locale.h>
#endif

/* private */
#include "config.h"
#include "v_locale.h"
/*
 * CatCompArray lives in this translation unit so GSI(id) can recover the
 * English builtin when the catalog is missing. VAT used to do that; the
 * empty-string fallback left every NewMenu title blank, so MUI built an
 * empty strip and the window never showed menus.
 */
#ifndef MBX
#define CATCOMP_ARRAY
#endif /* !MBX */
#include "voyager_cat.h"

struct Locale *locale;
struct Catalog *catalog;
#ifndef MBX
struct Catalog *CatalogBase;
static struct Task *catalog_owner;
#if (INCLUDE_VERSION >= 44) && !defined(__MORPHOS__)
struct LocaleBase *LocaleBase;
#else
struct Library *LocaleBase;
#endif /* (INCLUDE_VERSION >= 44) || defined(__MORPHOS__) */
#endif /* MBX */
int locale_timezone_offset;

#ifndef MBX
/*
 * GetCatalogStr() may still return NULL when the catalog is open but the
 * message id is missing; never pass that through to strcpy/printf.
 */
static STRPTR voyager_catalog_builtin( ULONG msgid )
{
	ULONG i;
	ULONG n;

	n = sizeof( CatCompArray ) / sizeof( CatCompArray[ 0 ] );
	for( i = 0; i < n; i++ )
	{
		if( (ULONG)CatCompArray[ i ].cca_ID == msgid )
			return( CatCompArray[ i ].cca_Str );
	}
	return( (STRPTR)"" );
}

STRPTR voyager_catalog_str( ULONG msgid, STRPTR builtin )
{
	STRPTR s;

	if( !builtin || !builtin[ 0 ] )
		builtin = voyager_catalog_builtin( msgid );

	if( !CatalogBase )
		return( builtin );

	/*
	 * locale.library on OS4 (newlib) is not safe from the nethandler
	 * child. GetCatalogStr() from that task DSI'd in strlen (r3 not a
	 * 68k address). English builtins are process-local constants.
	 */
	if( FindTask( NULL ) != catalog_owner )
		return( builtin );

	s = GetCatalogStr( CatalogBase, msgid, builtin );
	if( !s || !s[ 0 ] )
		return( builtin );

	return( s );
}
#endif /* !MBX */

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

#ifndef MBX
	catalog_owner = FindTask( NULL );
	CatalogBase = catalog;
#endif /* MBX */

	if( catalog )
	{
#ifdef MBX
		int c;

		for( c = 0; c < NUMCATSTRING; c++ )
			((char**)__stringtable)[ c ] = GetCatalogStr( catalog, c, (char*)__stringtable[ c ] );
#endif /* MBX */
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
	CatalogBase = NULL;
	catalog_owner = NULL;
	if( LocaleBase )
	{
		CloseLibrary( (struct Library *)LocaleBase );
	}
#else
//TOFIX!!
#endif
}
