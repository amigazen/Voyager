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
 * Security
 * --------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_spoof.c,v 1.15 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

#if USE_NET

/* private */
#include "prefswin.h"
#include "host_os.h"
#include "copyright.h"
#include "mui_func.h"

#define PWS_ENTRIES 3

struct Data {
	APTR str_spoof[ PWS_ENTRIES ];
	APTR str_spoof_an[ PWS_ENTRIES ];
	APTR str_spoof_ac[ PWS_ENTRIES ];
	APTR str_spoof_av[ PWS_ENTRIES ];
};


DECNEW
{
	struct Data *data;

	APTR str_spoof[ PWS_ENTRIES ];
	APTR str_spoof_ac[ PWS_ENTRIES ], str_spoof_an[ PWS_ENTRIES ], str_spoof_av[ PWS_ENTRIES ];
	char defapp[ 128 ];
	char defuser[ 128 ];
	int c;

	sprintf( defapp, VERSIONSTRING " (%s; %s)", hostos, cpuid );
	strcpy( defuser, "AmigaVoyager/" );
	strcat( defuser, defapp );

	obj	= DoSuperNew( cl, obj,

		Child, VGroup, GroupFrameT( GS( PREFSWIN_COMPAT_TITLE ) ),

			Child, ColGroup( 2 ),

				Child, Label2( ( STRPTR )"Real:" ),
				Child, TextObject, MUIA_Text_Contents, defuser, TextFrame, MUIA_Background, MUII_TextBack, MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L1 ), MUIA_Text_SetMin, FALSE, End,

				Child, RectangleObject, MUIA_Weight, 0, End,
				Child, HGroup,
					Child, TextObject, MUIA_Text_Contents, "AmigaVoyager", TextFrame, MUIA_Background, MUII_TextBack, MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L2 ), MUIA_Text_SetMin, FALSE, End,
					Child, TextObject, MUIA_Text_Contents, "AmigaVoyager", TextFrame, MUIA_Background, MUII_TextBack, MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L3 ), MUIA_Text_SetMin, FALSE, End,
					Child, TextObject, MUIA_Text_Contents, defapp, TextFrame, MUIA_Background, MUII_TextBack, MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L4 ), MUIA_Text_SetMin, FALSE, End,
				End,

				Child, Label2( ( STRPTR )"_1" ),
				Child, str_spoof[ 0 ] = pstring( DSI_NET_SPOOF_AS_1 + 0, 64, "_1" ),	
				Child, RectangleObject, MUIA_Weight, 0, End,	
				Child, HGroup,	
					Child, str_spoof_an[ 0 ] = pstring( DSI_NET_SPOOF_AS_1_AN + 0, 64, "" ),	
					Child, str_spoof_ac[ 0 ] = pstring( DSI_NET_SPOOF_AS_1_AC + 0, 64, "" ),	
					Child, str_spoof_av[ 0 ] = pstring( DSI_NET_SPOOF_AS_1_AV + 0, 64, "" ),	
				End,	

				Child, Label2( ( STRPTR )"_2" ),
				Child, str_spoof[ 1 ] = pstring( DSI_NET_SPOOF_AS_1 + 1, 64, "_2" ),	
				Child, RectangleObject, MUIA_Weight, 0, End,	
				Child, HGroup,	
					Child, str_spoof_an[ 1 ] = pstring( DSI_NET_SPOOF_AS_1_AN + 1, 64, "" ),	
					Child, str_spoof_ac[ 1 ] = pstring( DSI_NET_SPOOF_AS_1_AC + 1, 64, "" ),	
					Child, str_spoof_av[ 1 ] = pstring( DSI_NET_SPOOF_AS_1_AV + 1, 64, "" ),	
				End,	

				Child, Label2( ( STRPTR )"_3" ),
				Child, str_spoof[ 2 ] = pstring( DSI_NET_SPOOF_AS_1 + 2, 64, "_3" ),	
				Child, RectangleObject, MUIA_Weight, 0, End,	
				Child, HGroup,	
					Child, str_spoof_an[ 2 ] = pstring( DSI_NET_SPOOF_AS_1_AN + 2, 64, "" ),	
					Child, str_spoof_ac[ 2 ] = pstring( DSI_NET_SPOOF_AS_1_AC + 2, 64, "" ),	
					Child, str_spoof_av[ 2 ] = pstring( DSI_NET_SPOOF_AS_1_AV + 2, 64, "" ),	
				End,	
			End,
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	for( c = 0; c < PWS_ENTRIES; c++ )
	{
		data->str_spoof[ c ] = str_spoof[ c ];
		data->str_spoof_an[ c ] = str_spoof_an[ c ];
		data->str_spoof_ac[ c ] = str_spoof_ac[ c ];
		data->str_spoof_av[ c ] = str_spoof_av[ c ];
		
		/* Help */
		set( str_spoof[ c ], MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L1 ) );
		set( str_spoof_an[ c ], MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L2 ) );
		set( str_spoof_ac[ c ], MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L3 ) );
		set( str_spoof_av[ c ], MUIA_ShortHelp, GS( PREFSWIN_COMPAT_SPOOF_L4 ) );
	}

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;
	int c;

	for( c = 0; c < PWS_ENTRIES; c++ )
	{
		storestring( data->str_spoof[ c ], DSI_NET_SPOOF_AS_1 + c );
		storestring( data->str_spoof_an[ c ], DSI_NET_SPOOF_AS_1_AN + c );
		storestring( data->str_spoof_ac[ c ], DSI_NET_SPOOF_AS_1_AC + c );
		storestring( data->str_spoof_av[ c ], DSI_NET_SPOOF_AS_1_AV + c );
	}

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_spoofclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_SpoofClass";
#endif

	return( TRUE );
}

void delete_prefswin_spoofclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_spoofclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
