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
**
** $Id: mime.c,v 1.27 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif


/* private */
#include "voyager_cat.h"
#include "menus.h"
#include "mui_func.h"
#include "dos_func.h"

#if !USE_VAT
int mime_findbyextension( char *filename, char *savedir, char *viewer, int *viewmode, char *mimetype )
{
#if USE_DOS
	char buffer[ 512 ], *p, *p2 = 0, *p3 = 0;
	int found = FALSE;
#endif
	char lastclassdir[ 200 ], *ext;
	BPTR f;

	if( viewmode )
		*viewmode = 0;

	if( mimetype )
		*mimetype = 0;

	lastclassdir[ 0 ] = 0;

	ext = strrchr( filename, '.' );
	if( !ext )
		return( FALSE );
	ext++;

	//Printf( "checking for extension %s\n", ext ); 

#if USE_DOS
	f = Open( "ENV:MIME.prefs", MODE_OLDFILE );
#else
	f = 0;
#endif
	if( !f )
	{
#ifdef MBX
		/*
		 * TOFIX: this is ugly. There should be a separate
		 * way to enable this for all plugins. Once it works for
		 * non MBX builds, this should be removed.
		 */
		static STRPTR exttab[] = { "htm", "txt", "gif", "jpg", "jpeg", "jfif", "png", "xbm", "swf", NULL };
		static STRPTR mimetab[] = { "text/html", "text/plain", "image/gif", "image/jpeg", "image/jpeg", "image/jpeg", "image/png", "image/xbm", "application/x-shockwave-flash" };
#else
		static STRPTR exttab[] = { "htm", "txt", "gif", "jpg", "jpeg", "jfif", "png", "xbm", NULL };
		static STRPTR mimetab[] = { "text/html", "text/plain", "image/gif", "image/jpeg", "image/jpeg", "image/jpeg", "image/png", "image/xbm", };
#endif
		int c;

		if( savedir )
			*savedir = '\0';

		for( c = 0; exttab[ c ]; c++ )
		{
			if( !strnicmp( exttab[ c ], ext, strlen( exttab[ c ] ) ) )
			{
				if( mimetype )
					strcpy( mimetype, mimetab[ c ] );
				if( viewmode )
					*viewmode = ( 2 | 512 );

				return( TRUE );
			}
		}
		return( FALSE );
	}
#if USE_DOS
	while( !found && FGets( f, buffer, sizeof( buffer ) ) )
	{
		if( buffer[ 0 ] == ';' )
			continue;

		// find extension
		p = strchr( buffer, ',' );
		if( !p )
			continue;
		*p++ = 0;

		p2 = strchr( p, ',' );
		if( !p2 )
			continue;
		*p2++ = 0;

		p3 = strchr( p2, ',' );
		if( !p3 )
			continue;
		*p3++ = 0;

		if( strchr( buffer, '*' ) )
		{
			// found a class spec
			stccpy( lastclassdir, p2, sizeof( lastclassdir ) );
			continue;
		}

		//Printf( "matching against %s\n", p );

		Forbid();
		for( p = strtok( p, " " ); p; p = strtok( NULL, " " ) )
		{
			if( !stricmp( p, ext ) )
			{
				//Printf( "found match at %s\n", p );
				found = TRUE;
				break;
			}
		}
		Permit();
	}

	Close( f );

	if( !found )
	{
		return( FALSE );
	}

	stccpy( mimetype, buffer, 128 );

	// ok, we found it

	p = strchr( p3, ',' );
	if( !p )
		return( TRUE );

	*p++ = 0;
	// p3 now points to the viewer
	if( viewer )
		stccpy( viewer, p3, 200 );

	if( viewmode )
		*viewmode = atoi( p );

	p = strchr( p, ',' );
	if( !p )
		return( TRUE );
	
	p++;

	//Printf( "flag %ld dir %s\n", p, p2 );

	if( savedir )
	{
		if( atoi( p ) )
			strcpy( savedir, lastclassdir );
		else if( *p2 )
			stccpy( savedir, p2, 200 );
	}

	if( viewmode )
	{
		p = strchr( p, ',' );
		if( !p )
			return( TRUE );
		if( atoi( ++p ) )
			*viewmode |= 256;

		p = strchr( p, ',' );
		if( !p )
			return( TRUE );
		if( atoi( ++p ) )
			*viewmode |= 512;
	}

#endif /* USE_DOS */
	return( TRUE );
}

int mime_findbytype( char *mimetype, char *savedir, char *viewer, int *viewmode )
{
#if USE_DOS
	char buffer[ 512 ], *p2 = 0, *p3 = 0;
	BPTR f;
	int found = FALSE;
#endif
	char lastclassdir[ 200 ], *p;
	char cmptype[ 128 ];

	stccpy( cmptype, mimetype, sizeof( cmptype ) );
	p = strchr( cmptype, ';' );
	if( p )
		*p = 0;

	if( viewmode )
		*viewmode = 0;

	lastclassdir[ 0 ] = 0;

	//Printf( "checking for type %s\n", cmptype ); 

#if !USE_DOS
	// Hack, to get images inline in MBX
	if( !strnicmp( mimetype, "image/", 6 ) || !strnicmp( mimetype, "text/", 5 ) )
	{
		*viewmode = 512;
		return( FALSE );
	}
	return( FALSE );
#else
	f = Open( "ENV:MIME.prefs", MODE_OLDFILE );
	if( !f )
		return( FALSE );

	while( !found && FGets( f, buffer, sizeof( buffer ) ) )
	{
		if( buffer[ 0 ] == ';' )
			continue;

		// find extension
		p = strchr( buffer, ',' );
		if( !p )
			continue;
		*p++ = 0;

		p2 = strchr( p, ',' );
		if( !p2 )
			continue;
		*p2++ = 0;

		p3 = strchr( p2, ',' );
		if( !p3 )
			continue;
		*p3++ = 0;

		if( strchr( buffer, '*' ) )
		{
			// found a class spec
			stccpy( lastclassdir, p2, sizeof( lastclassdir ) );
			continue;
		}

		//Printf( "matching against %s\n", p );

		if( !stricmp( buffer, cmptype ) )
		{
			found = TRUE;
			break;
		}
	}

	Close( f );

	if( !found )
	{
		return( FALSE );
	}

	// ok, we found it

	p = strchr( p3, ',' );
	if( !p )
		return( TRUE );

	*p++ = 0;
	// p3 now points to the viewer
	if( viewer )
		stccpy( viewer, p3, 200 );

	if( viewmode )
		*viewmode = atoi( p );

	p = strchr( p, ',' );
	if( !p )
		return( TRUE );
	
	p++;

	//Printf( "flag %ld dir %s\n", p, p2 );

	if( savedir )
	{
		if( atoi( p ) )
			strcpy( savedir, lastclassdir );
		else if( *p2 )
			stccpy( savedir, p2, 200 );
	}

	if( viewmode )
	{
		p = strchr( p, ',' );
		if( !p )
			return( TRUE );
		if( atoi( ++p ) )
			*viewmode |= 256;

		p = strchr( p, ',' );
		if( !p )
			return( TRUE );
		if( atoi( ++p ) )
			*viewmode |= 512;
	}

	return( TRUE );
#endif /* USE_DOS */
}
#endif /* !USE_VAT */


void probe_mimeprefs( void )
{
#if USE_DOS
	BPTR l = Lock( "ENV:MIME.prefs", SHARED_LOCK );
	
	D( db_init, bug( "initializing..\n" ) );
	if( l )
		UnLock( l );
	else
#endif
		DoMethod( app, MUIM_Application_ReturnID, MENU_SET_MIME );

}

#if USE_DOS
static int probemimeprefs( char *p )
{
	BPTR l;

	if( l = Lock( p, SHARED_LOCK ) )
	{
		char path[ 256 ];
		char *ps = (char*)getv( app, MUIA_Application_PubScreenName );

		NameFromLock( l, path, sizeof( path ) );
		strins( path, "\"" );

		sprintf( strchr( path, 0 ), "\" PubScreen=\"%s\" AppName=MD2",
			( ps && *ps ) ? ps : "Workbench"
		);

		strcat( path, "\"" );

		SystemTags( path,
			SYS_Asynch, TRUE,
			SYS_Input, Open( "NIL:", MODE_OLDFILE ),
			SYS_Output, Open( "NIL:", MODE_OLDFILE ),
			TAG_DONE
		);

		UnLock( l );

		return( 0 );
	}
	return( 1 );
}
#endif /* USE_DOS */


void runmimeprefs( void )
{
#if USE_DOS
	if( probemimeprefs( "PROGDIR:MIMEPrefs" ) )
	{
		if( probemimeprefs( "SYS:Prefs/MIMEPrefs" ) )
		{
			MUI_Request( app, NULL, 0, NULL, GS( CANCEL ), GS( MIMEPREFS_NOTFOUND ), 0 );
		}
	}
#endif
}
