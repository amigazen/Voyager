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
 * Wiz's super duper crashy CManager
 * ---------------------------------
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: cmanager.c,v 1.14 2003/11/21 09:09:18 zapek Exp $
 *
*/

#include "voyager.h"

#if USE_CMANAGER

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

#include "cmanager.h"
#include "classes.h"
#include "htmlclasses.h" /* TOFIX: grr */
#include "voyager_cat.h"
#include "mui_func.h"
#include "dos_func.h"

struct Library *CManagerBase;
APTR cm_obj;
static APTR win_bm;

/*
 * Call before calling a CManager (CM_#?) function.
 */
int init_cmanagerbase( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !CManagerBase )
	{
		CManagerBase = VAT_OpenLibrary( "CManager.library", 10 );
	}
	return( ( int )CManagerBase );
}

/*
 * Call after you're done calling CM_#? functions.
 */
void cleanup_cmanagerbase( void )
{
	if( CManagerBase )
	{
		CloseLibrary( CManagerBase );
		CManagerBase = NULL;
	}
}

MUI_HOOK( setfrombookmark, int dummy, APTR msg )
{
	struct CMWWW *cmw = msg;
	struct CMFTP *cmf = (APTR)cmw;

	if( cmw->Type == CME_FTP )
	{
		char url[ 1024 ];
		sprintf( url, "ftp://%s", cmf->FTP );
		DoMethod( app, MM_DoLastActiveWin, MM_HTMLWin_SetURL, url, NULL, NULL, MF_HTMLWin_AddURL );
	}
	else
	{
		DoMethod( app, MM_DoLastActiveWin, MM_HTMLWin_SetURL, cmw->WWW, NULL, NULL, MF_HTMLWin_AddURL );
	}
	return( 0 );
}


/*
 * Create the CManager object if needed. Call it
 * everytime you need a CM function since it's
 * dynamic.
 */
int bm_create( void )
{
	if( win_bm )
	{
		return( TRUE );
	}

	win_bm = WindowObject,
		MUIA_Window_ID   , MAKE_ID('B','O','O','K'),
		MUIA_Window_Title, GS( BOOK_TITLE ),
		MUIA_Window_RootObject, VGroup,
			Child, cm_obj = CManagerObject,
				MUIA_CManager_HideUsers, TRUE,
				MUIA_CManager_HideChat, TRUE,
				MUIA_CManager_HideTelnet, TRUE,
				MUIA_CManager_AppDoubleClick, &setfrombookmark_hook,
			End,
		End,
	End;
		
	if (win_bm)
	{
		DoMethod( app, OM_ADDMEMBER, win_bm );
	}
	else
	{
		displaybeep(); /* TOFIX: display something, or try to */
		return( FALSE );
	}

	DoMethod( cm_obj, MUIM_CManager_LoadData, NULL, NULL );

	DoMethod( cm_obj, MUIM_Notify, MUIA_CManager_Changed, TRUE,
		cm_obj, 3, MUIM_CManager_SaveData, FALSE, NULL
	);

	DoMethod( win_bm, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 6, MUIM_Application_PushMethod, win_bm, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);
}

/*
 * Opens the bookmark window.
 */
int bm_openwin( void )
{
	if( bm_create() )
	{
		set( win_bm, MUIA_Window_Open, TRUE );

		return( (int)getv( win_bm, MUIA_Window_Open ) );
	}
	else
	{
		return( FALSE );
	}
}

/* Not needed anymore since CManager loads things from the current user anyway */
void bm_load( char *filename )
{
	FileLock_p l;
	if( l = Lock( filename, SHARED_LOCK ) )
	{
		UnLock( l );
		
		if( bm_create() )
		{
			DoMethod( cm_obj, MUIM_CManager_LoadData, NULL, filename );
		}
	}
}

void bm_save( int ask )
{
	if( ask )
	{
		/*
		 * No point in saving if nothing is modified,
		 * really.
		 */
		if( !bm_create() )
		{
			return;
		}
	}

	if( cm_obj )
	{
		DoMethod( cm_obj, MUIM_CManager_SaveData, ask, NULL );
	}
}

/*
 * To call before exiting.
 */
void bm_cleanup( void )
{
	if( cm_obj )
	{
		bm_save( FALSE );
		DoMethod( cm_obj, MUIM_CManager_Cleanup, FALSE ); /* ranieri broke this call by adding a fucking parameter.. */
	}
}

void addtobookmarks( char *url, char *title )
{
	struct CMWWW *cmw;

	if( init_cmanagerbase() )
	{
		if( bm_create() )
		{
			if( cmw = CM_AllocEntry( CME_WWW ) )
			{
				stccpy( cmw->WWW, url, sizeof( cmw->WWW ) );
				stccpy( cmw->Name, title, sizeof( cmw->Name ) );

				DoMethod( cm_obj, MUIM_CManager_AddEntry, cmw, MUIV_CManager_AddEntry_CurrentGroup );
			}
		}
		cleanup_cmanagerbase();
	}
}

#ifdef __MORPHOS__
void magicfunc( void )
#else
void ASM SAVEDS magicfunc(
	__reg( a0, STRPTR *str ),
	__reg( a1, ULONG *ver ),
	__reg( a2, ULONG **data ),
	__reg( a3, ULONG *datasize ) )
#endif /* !__MORPHOS__ */
{
	static char md5[ 16 ];
#ifdef __MORPHOS__
	STRPTR *str = ( STRPTR *)REG_A0;
	ULONG *ver = ( ULONG *)REG_A1;
	ULONG **data = ( ULONG **)REG_A2;
	ULONG *datasize = ( ULONG *)REG_A3;
#endif /* __MORPHOS__ */

	*str = "Voyager";
	*ver = 1;

	VAT_CalcMD5( *data, *datasize, md5 );

	*data = (APTR)md5;
	*datasize = 16;
}

#ifdef __MORPHOS__
struct EmulLibEntry GATEmagicfunc =
{
	TRAP_LIB, 0, ( void ( * )( void ) )magicfunc
};
#endif /* __MORPHOS__ */

#endif /* USE_CMANAGER */
