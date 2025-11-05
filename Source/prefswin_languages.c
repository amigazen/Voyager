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
 * Languages
 * ---------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_languages.c,v 1.13 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/locale.h>
#endif

/* private */
#include "prefswin.h"
#include "countries.h"
#include "mui_func.h"


const STRPTR iso639table[] = {
	"*, |All",
	"da,|Dansk",
	"de,|Deutsch",
	"en,|English",
	"es,|Español",
	"fr,|Français",
	"it,|Italiano",
	"nl,|Nederlands",
	"no,|Norsk",
	"pt,|Portuguese",
	"sv,|Swedish",
	NULL
};

struct Data {
	APTR str_languages;
};

#if USE_POPPH
#define POPPH1 Child, str_languages = ppopph( DSI_MISC_LANGUAGES, 256, (char**)iso639table ),
#else
#define POPPH1
#endif

DECNEW
{
	APTR str_languages = NULL;
	APTR bt_getlocale;

	struct Data *data;

	obj = ( APTR )DoSuperNew( cl, obj,
		Child, HGroup, GroupFrameT( GS( PREFSWIN_LANGUAGE_TITLE ) ),
			Child, Label1( GS( PREFSWIN_LANGUAGE_LABEL ) ),
			POPPH1
			Child, bt_getlocale = TextObject,
				ButtonFrame,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_Contents, GS( PREFSWIN_LANGUAGE_LOCALE ),
				MUIA_Text_PreParse, "33c",
				MUIA_InputMode    , MUIV_InputMode_RelVerify,
				MUIA_Background   , MUII_ButtonBack,
				MUIA_CycleChain, TRUE,
				MUIA_Text_SetMax, TRUE,
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	data->str_languages = str_languages;

	set( str_languages, MUIA_ShortHelp, GS( SH_PREFSWIN_STR_LANGUAGES ) );
	set( bt_getlocale, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_GETLOCALE ) );
	
	DoMethod( bt_getlocale, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MM_Prefswin_Languages_GetLocale
	);
 
	return( ( ULONG )obj );
}


/*
 * Sets the Accept Languages string according to the current locale settings
 */
DECMETHOD( Prefswin_Languages_GetLocale, APTR )
{
	GETDATA;
	int i, j;
	extern struct Locale *locale;
	char buffer[ ISO639NUM * 4 + 2 ];
	buffer[ 0 ] = '\0';

	for( i = 0; i < 10 && locale->loc_PrefLanguages[ i ]; i++ )
	{
		for( j = 0; iso639table[ j ]; j++ )
		{
			if( !StrnCmp( locale, locale->loc_PrefLanguages[ i ], iso639table[ j ] + 4, -1, SC_ASCII ) )
			{
				strncat( buffer, iso639table[ j ], 3 );
				strcat( buffer, " " );
			}
		}
	}
	strcat( buffer, "*" );
	set( data->str_languages, MUIA_String_Contents, buffer );

	return( 0 );
}


DECDISPOSE
{
	GETDATA;
	char buffer[ 256 ];

	/* remove final ',' if any */
	STRPTR p = ( STRPTR )getv( data->str_languages, MUIA_String_Contents );

	if( *( p + strlen( p ) - 1 ) == ',' )
	{
		stccpy( buffer, p, strlen( p ) );
		set( data->str_languages, MUIA_String_Contents, buffer );
	}

	storestring( data->str_languages, DSI_MISC_LANGUAGES );
 
	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Prefswin_Languages_GetLocale )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_languagesclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_LanguagesClass";
#endif

	return( TRUE );
}

void delete_prefswin_languagesclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_languagesclass( void )
{
	return( mcc->mcc_Class );
}
