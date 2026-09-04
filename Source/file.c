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
 * DOS support functions
 * ---------------------
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: file.c,v 1.12 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "file.h"
#include "dos_func.h"
#if USE_WBSTART
#include <proto/wbstart.h>
#endif /* USE_WBSTART */

static BPTR oldprogdir;
BPTR currentdir_lock;

/*
 * Sets the comment of a file to 80 chars. This stripping
 * is necessary because FFS <= 40.4 will trash the filesystem
 * if more than 80 chars are used.
 */
void setcomment( STRPTR filename, STRPTR comment )
{
#if USE_DOS
	char com[ 80 ];

	stccpy( com, comment, 80 );
	SetComment( filename, com );
#endif
}


static int testpath( BPTR lock, char *cmd, char *to )
{
#if USE_DOS
	BPTR oldcd, test;

	oldcd = CurrentDir( lock );
	test = Lock( cmd, SHARED_LOCK );
	CurrentDir( oldcd );
	if( test && to )
		NameFromLock( test, to, 256 );
	UnLock( test );

	return( test ? TRUE : FALSE );
#else
	return( FALSE );
#endif
}
static int testpathname( char *pn, char *cmd )
{
#if USE_DOS
	BPTR l = Lock( pn, SHARED_LOCK );
	int rc;

	if( !l )
		return( FALSE );
	rc = testpath( l, cmd, NULL );
	UnLock( l );
	return( rc );
#else
	return FALSE;
#endif
}


#define MY_SYSTEMTAGS_MAX_TAGS 32

/*
 * extended SystemTags() with WB support.
 * Builds a proper TagItem array from varargs (va_list as TagItem* is undefined behaviour).
 */
int STDARGS mySystemTags( char *cmd, ... )
{
	struct pn {
		BPTR next;
		BPTR lock;
	} *pn;
	char ncmd[256], *p;
	struct Process *wb;
	struct CommandLineInterface *wbcli;
	va_list va;
	struct TagItem tags[ MY_SYSTEMTAGS_MAX_TAGS ];
	ULONG tag, data;
	int i;
	int result;

	va_start( va, cmd );

	i = 0;
	do
	{
		tag = va_arg( va, ULONG );
		if( tag == TAG_DONE )
		{
			tags[ i ].ti_Tag = TAG_DONE;
			tags[ i ].ti_Data = 0;
			break;
		}
		data = va_arg( va, ULONG );
		tags[ i ].ti_Tag = tag;
		tags[ i ].ti_Data = data;
		i++;
	} while( i < MY_SYSTEMTAGS_MAX_TAGS );

	if( i >= MY_SYSTEMTAGS_MAX_TAGS )
	{
		tags[ MY_SYSTEMTAGS_MAX_TAGS - 1 ].ti_Tag = TAG_DONE;
		tags[ MY_SYSTEMTAGS_MAX_TAGS - 1 ].ti_Data = 0;
	}

	if (!Cli())
	{
		/*
		 * We're from Workbench.
		 */
		stccpy(ncmd, cmd, sizeof(ncmd));
		p = stpbrk(ncmd, " \t ");
		if (p)
		{
	        *p = 0;
		}

		/*
		 * Check default path
		 */
		if (!testpathname("", ncmd) || !testpathname("C:", ncmd))
		{
			if (wb = (struct Process *) FindTask( "Workbench"))
			{
				if (wbcli = BADDR(wb->pr_CLI))
				{
					pn = BADDR(wbcli->cli_CommandDir);
					while (pn)
					{
						if (testpath(pn->lock, ncmd, ncmd))
						{
							if (p = stpbrk(cmd, " \t "))
							{
								strcat(strchr(ncmd, '\0'), p);
							}
							result = SystemTagList( ncmd, tags );
							va_end( va );
							return result;
						}
						pn = BADDR(pn->next);
					}
				}
			}
		}
	}
	result = SystemTagList( cmd, tags );
	va_end( va );
	return result;
}


/*
 * Puts the correct PROGDIR:
 */
void init_progdir( void )
{
#if USE_DOS
	char progdirname[ 256 ];

	if( GetVar( "VOYAGERDIR", progdirname, sizeof( progdirname ), 0 ) > 0 )
	{
		currentdir_lock = Lock( progdirname, SHARED_LOCK );
		if( currentdir_lock )
		{
			oldprogdir = SetProgramDir( currentdir_lock );
		}
	}
#endif
}

void restore_progdir( void )
{
#if USE_DOS
	if( oldprogdir )
	{
		D( db_init, bug( "restoring PROGDIR:..\n" ) );
		UnLock( SetProgramDir( oldprogdir ) );
	}
#endif
}


#if USE_WBSTART
void cleanup_wbstart( void )
{
	if( WBStartBase )
	{
		D( db_init, bug( "cleaning up..\n" ) );
		CloseLibrary( WBStartBase );
	}
}
#endif /* USE_WBSTART */
