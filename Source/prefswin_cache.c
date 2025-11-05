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
 * Cache settings
 * --------------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_cache.c,v 1.17 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#endif

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR str_cachedir;
	APTR str_cachesize;
	APTR str_cachemem;
	APTR cyc_cacheverify;
	APTR chk_cacheimg;
	APTR str_cachedays;
};

DECNEW
{
#if USE_DOS
	static STRPTR verify_opts[ 4 ];
	struct Data *data;
	APTR str_cachedir, str_cachedays;
#endif
	APTR bt_flushdisk = NULL, bt_flushmem = NULL;
	APTR str_cachesize = NULL, str_cachemem = NULL, cyc_cacheverify = NULL, chk_cacheimg = NULL;

#if USE_DOS

	fillstrarray( verify_opts, MSG_PREFSWIN_CACHE_VERIFY_1, 3 );

	obj = DoSuperNew( cl, obj,
		Child, ColGroup( 2 ), GroupFrameT( GS( PREFSWIN_CACHE_DISK_GFT ) ),
			Child, Label2( GS( PREFSWIN_CACHE_DISK_DIR ) ),
			Child, PopaslObject,
				MUIA_Popasl_Type, ASL_FileRequest,
				MUIA_Popstring_String, str_cachedir = pstring( DSI_CACHE_DIR, 256, GS( PREFSWIN_CACHE_DISK_DIR ) ),
				MUIA_Popstring_Button, PopButton( MUII_PopDrawer ),
				ASLFR_TitleText, GS( PREFSWIN_CACHE_DISK_DIR ),
				ASLFR_DrawersOnly, TRUE,
			End,

			Child, Label2( GS( PREFSWIN_CACHE_DISK_SIZE ) ),
			Child, HGroup,
				Child, str_cachesize = pinteger( DSI_CACHE_SIZE, GS( PREFSWIN_CACHE_DISK_SIZE ) ),
				Child, Label2( GS( PREFSWIN_CACHE_DISK_SIZE_KB ) ),
				Child, bt_flushdisk = button( MSG_PREFSWIN_CACHE_DISK_FLUSH, 0 ),
			End,

			Child, Label2( GS( PREFSWIN_CACHE_AUTOVERIFY ) ),
			Child, HGroup,
				Child, str_cachedays = pinteger( DSI_CACHE_AUTOVERIFY, GS( PREFSWIN_CACHE_AUTOVERIFY ) ),
				Child, Label2( GS( PREFSWIN_CACHE_VERIFY ) ),
				Child, cyc_cacheverify = pcycle( verify_opts, DSI_CACHE_VERIFY, GS( PREFSWIN_CACHE_VERIFY ) ),
			End,

		  End,

		Child, ColGroup( 2 ), GroupFrameT( GS( PREFSWIN_CACHE_MEM_GFT ) ),

			Child, Label2( GS( PREFSWIN_CACHE_DISK_SIZE ) ),
			Child, HGroup,
				Child, str_cachemem = pinteger( DSI_CACHE_MEMSIZE, GS( PREFSWIN_CACHE_DISK_SIZE ) ),
				Child, Label2( GS( PREFSWIN_CACHE_DISK_SIZE_KB ) ),
				Child, bt_flushmem = button( MSG_PREFSWIN_CACHE_MEM_FLUSH, 0 ),
			End,

			Child, Label2( GS( PREFSWIN_CACHE_IMAGES ) ),
			Child, HGroup,
				Child, chk_cacheimg = pcheck( DSI_CACHE_IMAGES, GS( PREFSWIN_CACHE_IMAGES ) ),
				Child, HSpace( 0 ),
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->str_cachedir = str_cachedir;
	data->str_cachesize = str_cachesize;
	data->str_cachemem = str_cachemem;
	data->cyc_cacheverify = cyc_cacheverify;
	data->chk_cacheimg = chk_cacheimg;
	data->str_cachedays = str_cachedays;
#endif

#if USE_DOS
	/* Help */
	set( str_cachedir, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_CACHEDIR ) );
	set( str_cachesize, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_CACHESIZE ) );
	set( bt_flushdisk, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_FLUSHDISK ) );
	set( str_cachedays, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_CACHEDAYS ) );
#endif
	set( cyc_cacheverify, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_CACHEVERIFY ) );
	set( str_cachemem, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_CACHEMEM ) );
	set( bt_flushmem, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_FLUSHMEM ) );
	set( chk_cacheimg, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_CACHEIMG ) );
	
	set( str_cachemem, MUIA_Weight, 50 );
	set( str_cachesize, MUIA_Weight, 50 );

	DoMethod( bt_flushmem, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MM_App_CacheFlush, MV_App_CacheFlush_Mem
	);
	DoMethod( bt_flushdisk, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MM_App_CacheFlush, MV_App_CacheFlush_Disk
	);

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

#if USE_DOS
	storeattr( data->str_cachesize, MUIA_String_Integer, DSI_CACHE_SIZE );
	storeattr( data->str_cachedays, MUIA_String_Integer, DSI_CACHE_AUTOVERIFY );
	storestring( data->str_cachedir, DSI_CACHE_DIR );
#endif
	storeattr( data->cyc_cacheverify, MUIA_Cycle_Active, DSI_CACHE_VERIFY );
	storeattr( data->str_cachemem, MUIA_String_Integer, DSI_CACHE_MEMSIZE );
	storeattr( data->chk_cacheimg, MUIA_Selected, DSI_CACHE_IMAGES );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_cacheclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_CacheClass";
#endif

	return( TRUE );
}

void delete_prefswin_cacheclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_cacheclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
