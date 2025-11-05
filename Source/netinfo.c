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
** $Id: netinfo.c,v 1.38 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "voyager_cat.h"
#include "copyright.h"
#include "prefs.h"
#include "classes.h"
#include "network.h"
#include "methodstack.h"
#include "mui_func.h"

#define MAXCON 32
#define DNSNUM 4

APTR win_ni;
static char gaugeinfo[ MAXCON ][ 80 ];
static int maxvals[ MAXCON ], currentvals[ MAXCON ], parked[ MAXCON ];
static char sl[ MAXCON ][ 3 ];

struct Data {
	APTR gauge_prog[ MAXCON ];
	APTR bt_stop[ MAXCON ];
	ULONG maxcon;
};

DECNEW
{
	struct Data *data;
	APTR grp;
	APTR tmpgrp;
	int num = getprefslong( DSI_NET_MAXCON, 8 );
	int c;

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('N','E','T','I'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, GS( NETINFO_TITLE ),
		MUIA_Window_UseRightBorderScroller, TRUE,
		MUIA_Window_UseBottomBorderScroller, FALSE,
		MUIA_Window_NoMenus, TRUE,
		WindowContents, VGroup,
			Child, ScrollgroupObject,
				MUIA_Scrollgroup_UseWinBorder, TRUE,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_HorizBar, FALSE,
				MUIA_Scrollgroup_Contents, grp = VGroupV, MUIA_Font, MUIV_Font_Tiny, MUIA_Group_Spacing, 0, End,
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	for( c = 0; c < num; c++ )
	{
		sprintf( &sl[ c ][ 0 ], "%2u", c + 1 );
		
		tmpgrp = HGroup,
			Child, NewObject( getsmartlabelclass(), NULL,
				MA_Smartlabel_Text, &sl[ c ][ 0 ],
				MA_Smartlabel_MoreText, "666", /* }:> */
			End,
			Child, data->gauge_prog[ c ] = GaugeObject,
			GaugeFrame, MUIA_Gauge_Horiz, TRUE, MUIA_Gauge_InfoText, gaugeinfo[ c ],
				MUIA_Gauge_Current, currentvals[ c ],
				MUIA_Gauge_Max, maxvals[ c ],
				MUIA_Font, MUIV_Font_Tiny,
			End,

			Child, data->bt_stop[ c ] = TextObject,
				ButtonFrame,
				MUIA_Font, MUIV_Font_Tiny,
				MUIA_Text_Contents, "Stop",
				MUIA_Text_SetMax, TRUE,
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Disabled, TRUE,
				MUIA_UserData, 1,
			End,
		End;

		DoMethod( grp, OM_ADDMEMBER, ( ULONG )tmpgrp );
	
		DoMethod( data->gauge_prog[ c ], MUIM_Notify, MUIA_UserData, 0, ( ULONG )data->bt_stop[ c ], 3, MUIM_Set, MUIA_Disabled, TRUE );
		DoMethod( data->gauge_prog[ c ], MUIM_Notify, MUIA_UserData, 1, ( ULONG )data->bt_stop[ c ], 3, MUIM_Set, MUIA_Disabled, FALSE );
		DoMethod( data->bt_stop[ c ], MUIM_Notify, MUIA_Pressed, FALSE, ( ULONG )obj, 2, MM_Netinfo_Stop, ( ULONG )c );
	}

	//DoMethod( tmpgrp, OM_ADDMEMBER, ScaleObject, End ); weeee, if someone knows a way to add it without screwing the layout, call me

	data->maxcon = num;

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_Netinfo_Close
	);

	return( (ULONG)obj );
}


DECMMETHOD( Window_Setup )
{
	ULONG c;
	GETDATA;
	DOSUPER;

	for( c = 0; c < data->maxcon; c++ )
	{
		if( *gaugeinfo[ c ] && !parked[ c ] )
		{
			set( data->bt_stop[ c ], MUIA_Disabled, FALSE );
			set( data->bt_stop[ c ], MUIA_UserData, 0 );
		}
	}
 

	return( TRUE );
}

DECMETHOD( Netinfo_Close, ULONG )
{
	win_ni = NULL;
	killpushedmethods( obj );
	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );
	MUI_DisposeObject( obj );
	return( 0 );
}

DECMETHOD( Netinfo_SetMax, ULONG )
{
	GETDATA;
	//Printf( "max %ld = %ld\n", msg[ 1 ], maxvals[ msg[ 1 ] ] );
	set( data->gauge_prog[ msg[ 1 ] ], MUIA_Gauge_Max, maxvals[ msg[ 1 ] ] );
	return( 0 );
}

DECMETHOD( Netinfo_SetProgress, ULONG )
{
	GETDATA;
	//Printf( "val %ld = %ld/%ld\n", msg[ 1 ], currentvals[ msg[ 1 ] ], maxvals[ msg[ 1 ] ] );
	set( data->gauge_prog[ msg[ 1 ] ], MUIA_Gauge_Current, currentvals[ msg[ 1 ] ] );
	return( 0 );
}

DECMETHOD( Netinfo_SetClear, ULONG )
{
	GETDATA;
	SetAttrs( data->gauge_prog[ msg[ 1 ] ], 
		MUIA_Gauge_Current, 0,
		MUIA_Gauge_Max, 0,
		MUIA_Gauge_InfoText, ( ULONG )"",
		MUIA_UserData, 0,
		TAG_DONE
	);
	return( 0 );
}

DECMETHOD( Netinfo_SetParked, ULONG )
{
	GETDATA;
	SetAttrs( data->gauge_prog[ msg[ 1 ] ], 
		MUIA_Gauge_Current, 0,
		MUIA_Gauge_Max, 0,
		MUIA_Gauge_InfoText, ( ULONG )GS( NETINFO_PARKED ),
		MUIA_UserData, 0,
		TAG_DONE
	);
	return( 0 );
}

DECSMETHOD( Netinfo_SetURL )
{
	GETDATA;
	int ledobj = msg->un->ledobjnum;

	if( ledobj != -1 )
	{
		set( data->bt_stop[ ledobj ], MUIA_UserData, msg->un );
	
		SetAttrs( data->gauge_prog[ ledobj ],
			MUIA_Gauge_Current, 0,
			MUIA_Gauge_InfoText, ( ULONG )gaugeinfo[ ledobj ],
			MUIA_UserData, 1,
			TAG_DONE
		);
	}

	return( 0 );
}

DECSMETHOD( Netinfo_Stop )
{
	extern struct Process *netproc;
	struct unode *un;
	GETDATA;

	un = (struct unode *)getv( data->bt_stop[ msg->button_number ], MUIA_UserData );
	if( un )
		un->net_abort = TRUE;

	/* it's currently impossible to do a stopxfer() for a w->doc_loading */

	Signal( ( struct Task * )netproc, SIGBREAKF_CTRL_D); // let netproc rescan its client list

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMMETHOD( Window_Setup )
DEFMETHOD( Netinfo_Close )
DEFMETHOD( Netinfo_SetMax )
DEFMETHOD( Netinfo_SetProgress )
DEFMETHOD( Netinfo_SetClear )
DEFMETHOD( Netinfo_SetParked )
DEFSMETHOD( Netinfo_SetURL )
DEFSMETHOD( Netinfo_Stop )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_netinfowinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "NetinfoWinClass";
#endif

	return( TRUE );
}

void delete_netinfowinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getnetinfowin( void )
{
	return( mcc->mcc_Class );
}

void netinfo_setmax( int m, int maxv )
{
	maxvals[ m ] += maxv;
	parked[ m ] = FALSE;

	if( win_ni )
		pushmethod( win_ni, 2, MM_Netinfo_SetMax, m );
}

void netinfo_setprogress( int m, int progv )
{
	currentvals[ m ] += progv;
	parked[ m ] = FALSE;

	if( win_ni )
		pushmethod( win_ni, 2, MM_Netinfo_SetProgress, m );
}

void netinfo_url( struct unode *un )
{
	char *p = gaugeinfo[ un->ledobjnum ];
	char *p2;

	strcpy( p, "\033l" );

	/* InfoText > 80 chars -> boom, MUI bug */
	if( strlen( un->url ) > 77 )
	{
		stccpy( p + 2, un->url, 73 );
		strcpy( p + 75, "..." );
	}
	else
	{
		strcpy( p + 2, un->url );
	}

	p2 = p;
	while( *p2 )
	{
		if( *p2 == '%' )
		{
			p[ 78 ] = 0;
			strins( p2, "%" );
			p2++;
		}
		p2++;
	}

	if( win_ni )
		pushmethod( win_ni, 2, MM_Netinfo_SetURL, un );
}

void netinfo_clear( int m )
{
	gaugeinfo[ m ][ 0 ] = 0;
	currentvals[ m ] = 0;
	maxvals[ m ] = 0;
	parked[ m ] = FALSE;

	if( win_ni )
		pushmethod( win_ni, 2, MM_Netinfo_SetClear, m );
}

void netinfo_parked( int m )
{
	currentvals[ m ] = 0;
	maxvals[ m ] = 0;
	parked[ m ] = TRUE;
	strcpy( gaugeinfo[ m ], GS( NETINFO_PARKED ) );

	if( win_ni )
		pushmethod( win_ni, 2, MM_Netinfo_SetParked, m );
}

#endif /* USE_NET */
