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
 * Fonts
 * -----
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_fonts.c,v 1.33 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#include <proto/exec.h>
#endif

/* private */
#include "prefswin.h"
#include "mui_func.h"
#include "textinput.h"


#define FONTS_NUM 7

struct Data {
	APTR lv_faces;
	APTR chk_fontface;
	APTR pop_fonts[ FONTS_NUM ];
	APTR str_family;
	APTR bt_range;
	APTR bt_del_family;
	APTR txt_test;
	APTR str_step;
};
 

DECMETHOD( Prefswin_Fonts_SetFace, ULONG )
{
	GETDATA;

	int num;
	int c;

	get( data->lv_faces, MUIA_List_Active, &num );

	for( c = 0; c < FONTS_NUM; c++ )
		set( data->pop_fonts[ c ], MUIA_Disabled, num < 0 );

	if( num < 0 )
	{
		set( data->str_family, MUIA_Disabled, TRUE );
		set( data->bt_range, MUIA_Disabled, TRUE );
		set( data->bt_del_family, MUIA_Disabled, TRUE );
	}
	else if( num < 3 )
	{
		set( data->str_family, MUIA_Disabled, TRUE );
		set( data->bt_range, MUIA_Disabled, FALSE );
		set( data->bt_del_family, MUIA_Disabled, TRUE );
	}
	else
	{
		set( data->str_family, MUIA_Disabled, FALSE );
		set( data->bt_range, MUIA_Disabled, FALSE );
		set( data->bt_del_family, MUIA_Disabled, FALSE );
	}

	if( num >= 0 )
	{
		int c;
		char *p;

		for( c = 0; c < FONTS_NUM; c++ )
			nnset( data->pop_fonts[ c ], MUIA_String_Contents, getprefsstr( DSI_FONT_MAP( num, c ), "topaz/8" ) );

		DoMethod( data->lv_faces, MUIM_List_GetEntry, num, &p );

		nnset( data->str_family, MUIA_String_Contents, p );
	}

	return( 0 );
}


DECSMETHOD( Prefswin_Fonts_Test )
{
	GETDATA;
	int c;
	int num;
	extern struct TextFont *myopenfont( char *name, UBYTE **fontarray );

	D( db_gui, bug( "test func called, %ld\n", msg->obj ) );

	get( data->lv_faces, MUIA_List_Active, &num );
	if( num < 0 )
		return( 0 );

	for( c = 0; c < FONTS_NUM; c++ )
	{
		if( data->pop_fonts[ c ] == msg->obj || (APTR)getv( data->pop_fonts[ c ], MUIA_Popstring_String ) == msg->obj || (APTR)getv( data->pop_fonts[ c ], MUIA_Popstring_Button ) == msg->obj )
		{
			UBYTE *dummy;
			DoMethod( data->txt_test, MM_Fonttest_SetFont, myopenfont( getprefsstr( DSI_FONT_MAP( num, c ), "topaz/8" ), &dummy ) );
			break;
		}
	}
	
	return( 0 );
}


DECMETHOD( Prefswin_Fonts_Add, ULONG )
{
	GETDATA;
	int num;
	int c;

	DoMethod( data->lv_faces, MUIM_List_InsertSingle, "NewFace", MUIV_List_Insert_Bottom );

	get( data->lv_faces, MUIA_List_Entries, &num );

	for( c = 0; c < FONTS_NUM; c++ )
	{
		setprefsstr( DSI_FONT_MAP( num, c ), getprefs( DSI_FONT_MAP( 0, c ) ) );
	}	

	set( data->lv_faces, MUIA_List_Active, num );

	return( 0 );
}


DECMETHOD( Prefswin_Fonts_Del, ULONG )
{
	GETDATA;
	int num;
	int c;

	get( data->lv_faces, MUIA_List_Active, &num );
	if( num < 3 )
		return( 0 );

	for( c = num; c < getprefslong( DSI_FONT_FACE_NUM, 3 ) - 1; c++ )
	{
		int d;

		for( d = 0; d < FONTS_NUM; d++ )
			setprefsstr( DSI_FONT_MAP( c, d ), getprefs( DSI_FONT_MAP( c + 1, d ) ) );
	}

	setprefslong( DSI_FONT_FACE_NUM, getprefslong( DSI_FONT_FACE_NUM, 3 ) - 1 );

	DoMethod( data->lv_faces, MUIM_List_Remove, MUIV_List_Remove_Active );

	return( 0 );
}


DECMETHOD( Prefswin_Fonts_Name, ULONG )
{
	GETDATA;
	char *n1, *n2;
	int num;

	get( data->str_family, MUIA_String_Contents, ( STRPTR * )&n1 );
	get( data->lv_faces, MUIA_List_Active, &num );
	if( num < 3 )
		return( 0 );
	DoMethod( data->lv_faces, MUIM_List_GetEntry, num, &n2 );

	if( strcmp( n1, n2 ) )
	{
		strcpy( n2, n1 );
		DoMethod( data->lv_faces, MUIM_List_Redraw, num );
	}

	return( 0 );
}

#ifndef MBX
MUI_HOOK( construct, APTR pool, STRPTR str )
{
	STRPTR n = AllocPooled( pool, 32 );
	if( n )
	{
		strcpy( n, str );
	}
	return( (LONG)n );
}
#endif

DECMETHOD( Prefswin_Fonts_Update, ULONG )
{
	GETDATA;
	int num, c;

	get( data->lv_faces, MUIA_List_Active, &num );
	if( num >= 0 )
	{
		for( c = 0; c < FONTS_NUM; c++ )
			setprefsstr( DSI_FONT_MAP( num, c ), getstrp( data->pop_fonts[ c ] ) );
	}

	return( 0 );
}


DECMETHOD( Prefswin_Fonts_Range, ULONG )
{
	GETDATA;
	char *p = getstrp( data->pop_fonts[ 2 ] );
	char tmp1[ 32 ], tmp2[ 32 ];
	int step = getv( data->str_step, MUIA_String_Integer );
	int bsize = 0, size;
	int c;

	stccpy( tmp1, p, sizeof( tmp1 ) );
	p = strchr( tmp1, '/' );
	if( p )
	{
		*p++ = 0;
		bsize = atoi( p );
	}

	size = bsize;
	for( c = 1; c >= 0; c-- )
	{
		size -= step;
		if( size < 2 )
			size = 2;
		sprintf( tmp2, "%s/%d", tmp1, size );
		set( data->pop_fonts[ c ], MUIA_String_Contents, tmp2 );
	}

	size = bsize;
	for( c = 3; c < FONTS_NUM; c++ )
	{
		size += step;
		if( size < 2 )
			size = 2;
		sprintf( tmp2, "%s/%d", tmp1, size );
		set( data->pop_fonts[ c ], MUIA_String_Contents, tmp2 );
	}

	return( 0 );
}

extern APTR prefswindow;

DECNEW
{
	struct Data *data;

	APTR lv_faces, pop_fonts[ FONTS_NUM ];
	APTR txt_test;
	APTR str_family, bt_add_family, bt_del_family, bt_range, str_step;
	APTR chk_fontface;
 
	int c;

#ifndef MBX
#define FGR(x,t,s) \
			Child, Label2(t),\
			Child, Label2(s),\
			Child, pop_fonts[x] = PopaslObject,	\
				MUIA_CycleChain, 1,	\
				MUIA_Popasl_Type, ASL_FontRequest,	\
				MUIA_Popstring_String, TextinputObject, StringFrame, End,	\
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),	\
				ASLFO_TitleText, "Amiga Font name & size:",	\
			End,	\

	
	obj	= DoSuperNew( cl, obj,
		Child, HGroup,

			Child, lv_faces = ListviewObject, MUIA_HorizWeight, 50,
				MUIA_Listview_List, ListObject,
					InputListFrame,
					MUIA_List_ConstructHook, &construct_hook,
				End,
			End,

			Child, VGroup,

				Child, VSpace( 0 ),

				Child, ColGroup( 3 ),

					/* TOFIX: localize */
					FGR( 0, ( STRPTR )"Smallest", ( STRPTR )"1 (-2)" )
					FGR( 1, ( STRPTR )"", ( STRPTR )"2 (-1)" )
					FGR( 2, ( STRPTR )"Default", ( STRPTR )"3 (±0)" )
					FGR( 3, ( STRPTR )"", ( STRPTR )"4 (+1)" )
					FGR( 4, ( STRPTR )"", ( STRPTR )"5 (+2)" )
					FGR( 5, ( STRPTR )"", ( STRPTR )"6 (+3)" )
					FGR( 6, ( STRPTR )"Largest", ( STRPTR )"7 (+4)" )

				End,

				Child, VSpace( 0 ),

			End,

		End,

		Child, HGroup,
			
			Child, HSpace( 0 ),
			
			Child, Label1( GS( PREFSWIN_FONT_FONTFACE ) ),
			Child, chk_fontface = pchecki( DSI_FLAGS + VFLG_FONTFACE, GS( PREFSWIN_FONT_FONTFACE ) ),
			
			Child, HSpace( 0 ),

		End,

		Child, hbar(),

		Child, HGroup,
			Child, str_family = TextinputObject, StringFrame, MUIA_CycleChain, 1, MUIA_Weight, 200, MUIA_String_MaxLen, 32, End,
			Child, bt_add_family = ebutton( MSG_PREFS_FONT_FAMILY_ADD, 0 ),
			Child, bt_del_family = ebutton( MSG_PREFS_FONT_FAMILY_DEL, 0 ),
			Child, bt_range = ebutton( MSG_PREFS_FONT_FAMILY_RANGE, 0 ),
			Child, Label2( GS( PREFS_FONT_STEP ) ),
			Child, str_step = TextinputObject, StringFrame, MUIA_Textinput_IsNumeric, TRUE, MUIA_Textinput_MinVal, 1, MUIA_Textinput_Contents, "2", MUIA_CycleChain, 1, MUIA_Weight, 30, End,
		End,

		Child, BalanceObject, End,

		Child, txt_test = NewObject( getfonttestclass(), NULL, MUIA_Background, MUII_TextBack, MUIA_Weight, 25, TextFrame, End,

	End;

#else
	obj = NULL; //TOFIX!!
#endif /* !MBX */

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->lv_faces = lv_faces;
	data->chk_fontface = chk_fontface;
	for( c = 0; c < FONTS_NUM; c++ )
	{
		data->pop_fonts[ c ] = pop_fonts[ c ];
	}
	data->str_family = str_family;
	data->bt_range = bt_range;
	data->bt_del_family = bt_del_family;
	data->txt_test = txt_test;
	data->str_step = str_step;

	DoMethod( lv_faces, MUIM_List_InsertSingle, "(Default)", MUIV_List_Insert_Bottom );
	DoMethod( lv_faces, MUIM_List_InsertSingle, "(Fixed)", MUIV_List_Insert_Bottom );
	DoMethod( lv_faces, MUIM_List_InsertSingle, "(Template)", MUIV_List_Insert_Bottom );

	for( c = 3; c < getprefslong( DSI_FONT_FACE_NUM, 3 ); c++ )
	{
		DoMethod( lv_faces, MUIM_List_InsertSingle, getprefsstr( DSI_FONT_FACENAME( c ), "" ), MUIV_List_Insert_Bottom );
	}

	DoMethod( lv_faces, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Fonts_SetFace
	);

	DoMethod( prefswindow, MUIM_Notify, MUIA_Window_ActiveObject, MUIV_EveryTime,
		obj, 2, MM_Prefswin_Fonts_Test, MUIV_TriggerValue
	);

	DoMethod( prefswindow, MUIM_Notify, MUIA_Window_MouseObject, MUIV_EveryTime,
		obj, 2, MM_Prefswin_Fonts_Test, MUIV_TriggerValue
	);

	DoMethod( bt_add_family, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Fonts_Add
	);

	DoMethod( bt_del_family, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Fonts_Del
	);

	DoMethod( bt_range, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Fonts_Range
	);

	DoMethod( str_family, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Fonts_Name
	);

	for( c = 0; c < FONTS_NUM; c++ )
	{
		DoMethod( pop_fonts[ c ], MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
			obj, 1, MM_Prefswin_Fonts_Update
		);
	}

	DoMethod( obj, MM_Prefswin_Fonts_SetFace );

	/* Help */
	set( lv_faces, MUIA_ShortHelp, GS( SH_PREFSWIN_LV_FACES ) );
	set( chk_fontface, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_FONTFACE ) );
	set( str_family, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_FAMILY ) );
	set( bt_add_family, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_ADD_FAMILY ) );
	set( bt_del_family, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_DEL_FAMILY ) );
	set( bt_range, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_RANGE ) );
	set( str_step, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_STEP ) );
	set( txt_test, MUIA_ShortHelp, GS( SH_PREFSWIN_TXT_TEST ) );

	setupd( chk_fontface, MUIA_Selected );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;
	int c;

	D( db_gui, bug( "removed fonts panel\n" ) );
	
	set( prefswindow, MUIA_UserData, 42 ); /* MUI bug workaround */
	DoMethod( prefswindow, MUIM_KillNotify, MUIA_Window_ActiveObject );
	DoMethod( prefswindow, MUIM_KillNotify, MUIA_Window_MouseObject );

	for( c = 0; ; c++ )
	{
		char *p;

		DoMethod( data->lv_faces, MUIM_List_GetEntry, c, &p );
		if( !p )
			break;

		setprefsstr( DSI_FONT_FACENAME( c ), p );
	}

	setprefslong( DSI_FONT_FACE_NUM, c );

	storeattr( data->chk_fontface, MUIA_Selected, DSI_FLAGS + VFLG_FONTFACE );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( Prefswin_Fonts_Test )
DEFMETHOD( Prefswin_Fonts_SetFace )
DEFMETHOD( Prefswin_Fonts_Add )
DEFMETHOD( Prefswin_Fonts_Del )
DEFMETHOD( Prefswin_Fonts_Name )
DEFMETHOD( Prefswin_Fonts_Update )
DEFMETHOD( Prefswin_Fonts_Range )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_fontsclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_FontsClass";
#endif

	return( TRUE );
}

void delete_prefswin_fontsclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_fontsclass( void )
{
	return( mcc->mcc_Class );
}
