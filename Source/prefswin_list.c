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
 * Prefswin list class
 * -------------------
 * - Needed for the displaying of the pictures in the listview
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_list.c,v 1.14 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR images[ PREFSWIN_NUMPAGES ];
	APTR iobjs[ PREFSWIN_NUMPAGES ];
};

struct Data *prefslistdata;

MUI_HOOK( prefslistdisp, STRPTR *array, STRPTR txt )
{
#ifndef MBX
	static char tmp[ 40 ];
	sprintf( tmp, "\033O[%08lx] %s", ( ULONG )prefslistdata->images[ (ULONG)array[ -1 ] ], txt );
	array[ 0 ] = tmp;
#else
	*array = txt;
#endif
	return( 0 );
}


DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		InputListFrame,
		MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
		MUIA_List_DestructHook, MUIV_List_DestructHook_String,
		MUIA_List_DisplayHook, &prefslistdisp_hook,
		MUIA_List_MinLineHeight, 17,
		MUIA_List_AdjustWidth, TRUE,
		MUIA_List_AutoVisible, TRUE,
	End;

	if( !obj )
	{
		return( 0 );
	}

	prefslistdata = data = INST_DATA( cl, obj );

	return( ( ULONG )obj );
}


DECMMETHOD( Setup )
{
#ifndef MBX
	int c;
	GETDATA;
#endif
	if( !DOSUPER )
		return( FALSE );

#ifndef MBX
	for( c = 0; c < PREFSWIN_NUMPAGES; c++ )
	{
		data->iobjs[ c ] = BitmapObject,
			MUIA_Bitmap_Bitmap, prefsgroups[ c ].bm,
			MUIA_Bitmap_Height, 14, MUIA_Bitmap_Width, 24,
			MUIA_FixHeight, 14, MUIA_FixWidth, 24,
			MUIA_Bitmap_SourceColors, prefsimages_cmap,
			MUIA_Bitmap_Transparent, 0,
		End;

		data->images[ c ] = (APTR)DoMethod( obj, MUIM_List_CreateImage, data->iobjs[ c ], 0 );
	}
#endif

	return( TRUE );
}

DECMMETHOD( Cleanup )
{
#ifndef MBX
	GETDATA;
	int c;

	for( c = 0; c < PREFSWIN_NUMPAGES; c++ )
	{
		DoMethod( obj, MUIM_List_DeleteImage, data->images[ c ] );
		MUI_DisposeObject( data->iobjs[ c ] );
	}
#endif

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_listclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_List, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "PrefsListClass";
#endif

	return( TRUE );
}

void delete_prefswin_listclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_listclass( void )
{
	return( mcc->mcc_Class );
}
