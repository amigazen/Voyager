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
 * Fastlinks
 * ---------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_fastlinks.c,v 1.17 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "prefswin.h"
#include "mui_func.h"
#include "textinput.h"


struct Data {
	APTR lv_fastlinks;
	APTR chk_striptext;
	APTR str_fastlink_label;
	APTR str_fastlink_url;
	APTR bt_fastlink_del;
};
 

struct sfl {
	char label[ 80 ];
	char url[ 512 ];
};

MUI_HOOK( fldisp, STRPTR *array, struct sfl *sfl )
{
	if( !sfl )
	{
		*array++ = GS( PREFSWIN_FASTLINKS_L_LABEL );
		*array = GS( PREFSWIN_FASTLINKS_L_URL );
	}
	else
	{
		*array++ = sfl->label;
		*array = sfl->url;
	}

	return( 0 );
}

MUI_HOOK( flcons, APTR pool, struct sfl *sfl )
{
	struct sfl *new = AllocPooled( pool, sizeof( *new ) );
	if( new )
	{
		if( sfl )
			*new = *sfl;
		else
			memset( new, 0, sizeof( *new ) );
	}
	return( (LONG)new );
}

DECMETHOD( Prefswin_Fastlinks_SetStrings, ULONG )
{
	GETDATA;
	struct sfl *sfl;
	
	DoMethod( data->lv_fastlinks, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &sfl );
	if( sfl )
	{
		nnset( data->str_fastlink_label, MUIA_String_Contents, sfl->label );
		nnset( data->str_fastlink_url, MUIA_String_Contents, sfl->url );
	}
	set( data->str_fastlink_label, MUIA_Disabled, !sfl );
	set( data->str_fastlink_url, MUIA_Disabled, !sfl );
	set( data->bt_fastlink_del, MUIA_Disabled, !sfl );

	return( 0 );
}


DECMETHOD( Prefswin_Fastlinks_Copy, ULONG )
{
	GETDATA;
	struct sfl *sfl;

	DoMethod( data->lv_fastlinks, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &sfl );
	if( sfl )
	{
		strcpy( sfl->label, getstrp( data->str_fastlink_label ) );
		strcpy( sfl->url, getstrp( data->str_fastlink_url ) );
		DoMethod( data->lv_fastlinks, MUIM_List_Redraw, MUIV_List_Redraw_Active );
	}

	return( 0 );
}




DECNEW
{
	struct Data *data;
	APTR lv_fastlinks, str_fastlink_label, str_fastlink_url, bt_fastlink_add, bt_fastlink_del, chk_striptext;
	int num, c;

	obj	= DoSuperNew( cl, obj,
		
		GroupFrameT( GS( PREFSWIN_FASTLINKS_GFT ) ),

		Child, lv_fastlinks = ListviewObject, MUIA_CycleChain, 1,
			MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
			MUIA_Listview_List, ListObject, InputListFrame,
				MUIA_List_DragSortable, TRUE,
				MUIA_List_ConstructHook, &flcons_hook,
				MUIA_List_DisplayHook, &fldisp_hook,
				MUIA_List_Title, TRUE,
				MUIA_List_Format, "BAR,",
			End,
		End,

		Child, ColGroup( 2 ),
			Child, Label2( GS( PREFSWIN_FASTLINKS_LABEL ) ),
			Child, str_fastlink_label = TextinputObject, StringFrame, MUIA_CycleChain, 1, End,

			Child, Label2( GS( PREFSWIN_FASTLINKS_URL ) ),
			Child, str_fastlink_url = NewObject( geturlstringclass(), NULL, MUIA_CycleChain, 1, End,
		End,

		Child, HGroup,
			Child, bt_fastlink_add = button( MSG_PREFSWIN_BT_ADD, 0 ),
			Child, bt_fastlink_del = button( MSG_PREFSWIN_BT_DEL, 0 ),
		End,

		Child, HGroup,
			Child, HSpace( 0 ),
				Child, Label2( GS( PREFSWIN_FASTLINKS_STRIPTEXT ) ),
				Child, chk_striptext = pchecki( DSI_FLAGS + VFLG_FASTLINKS_STRIPTEXT, GS( PREFSWIN_FASTLINKS_STRIPTEXT ) ),
			Child, HSpace( 0 ),
		End,

	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->lv_fastlinks = lv_fastlinks;
	data->chk_striptext = chk_striptext;
	data->str_fastlink_label = str_fastlink_label;
	data->str_fastlink_url = str_fastlink_url;
	data->bt_fastlink_del = bt_fastlink_del;

	DoMethod( bt_fastlink_del, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_fastlinks, 2, MUIM_List_Remove, MUIV_List_Remove_Active
	);
	DoMethod( bt_fastlink_add, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_fastlinks, 3, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom
	); // grrr, NList doesn't handle that !
	DoMethod( bt_fastlink_add, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_fastlinks, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom
	);
#if 0 //TOFIX!! dunno why it doesn't work
	DoMethod( bt_fastlink_add, MUIM_Notify, MUIA_Pressed, FALSE,
		prefswindow, 2, MUIA_Window_ActiveObject, str_fastlink_label
	);
#endif

	DoMethod( lv_fastlinks, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Fastlinks_SetStrings
	);

	DoMethod( str_fastlink_label, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Fastlinks_Copy
	);
	DoMethod( str_fastlink_url, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Fastlinks_Copy
	);

	DoMethod( obj, MM_Prefswin_Fastlinks_SetStrings );

	num = getprefslong( DSI_FASTLINKS_NUM, 8 );

	for( c = 0; c < num; c++ )
	{
		struct sfl sfl;

		strcpy( sfl.label, getprefsstr( DSI_FASTLINKS_LABELS + c, "" ) );
		strcpy( sfl.url, getprefsstr( DSI_FASTLINKS_URLS + c, "" ) );

		DoMethod( lv_fastlinks, MUIM_List_InsertSingle, &sfl, MUIV_List_Insert_Bottom );
	}
	
	/* Help */
	set( lv_fastlinks, MUIA_ShortHelp, GS( SH_PREFSWIN_LV_FASTLINKS ) );
	set( str_fastlink_label, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_FASTLINK_LABEL ) );
	set( str_fastlink_url, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_FASTLINK_URL ) );
	set( bt_fastlink_add, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_FASTLINK_ADD ) );
	set( bt_fastlink_del, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_FASTLINK_DEL ) );
	set( chk_striptext, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_STRIPTEXT ) );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	struct sfl *sfl;
	int num = 0;
	int c;

	for( c = 0;;c++)
	{
		DoMethod( data->lv_fastlinks, MUIM_List_GetEntry, c, &sfl );
		if( !sfl )
			break;

		if( !sfl->label[ 0 ] && !sfl->url[ 0 ] )
			continue;

		setprefsstr_clone( DSI_FASTLINKS_LABELS + num, sfl->label );
		setprefsstr_clone( DSI_FASTLINKS_URLS + num, sfl->url );

		num++;
	}
	setprefslong_clone( DSI_FASTLINKS_NUM, num );
	storeattr( data->chk_striptext, MUIA_Selected, DSI_FLAGS + VFLG_FASTLINKS_STRIPTEXT );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Prefswin_Fastlinks_SetStrings )
DEFMETHOD( Prefswin_Fastlinks_Copy )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_fastlinksclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_FastlinksClass";
#endif

	return( TRUE );
}

void delete_prefswin_fastlinksclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_fastlinksclass( void )
{
	return( mcc->mcc_Class );
}
