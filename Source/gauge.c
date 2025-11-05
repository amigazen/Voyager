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
** $Id: gauge.c,v 1.25 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "classes.h"
#include "mui_func.h"
#include "busy.h"

/*
 * Gauge states
 */
#define GAUGE_EMPTY 0
#define GAUGE_PROGRESS 1
#define GAUGE_WAITING 2

struct Data {
	char txt[ 72 ];
	APTR gauge;
	int page;
};

DECNEW
{
	struct Data *data;
	APTR gauge;
	#if USE_BUSY
	APTR busy;
	#endif

	gauge = GaugeObject,
		GaugeFrame,
		MUIA_Font, MUIV_Font_Tiny,
		MUIA_Gauge_InfoText, "",
		MUIA_Gauge_Horiz, TRUE,
	End;
	
	if( !gauge )
	{
		return( 0 );
	}

	#if USE_BUSY
	busy = BusyObject,
		GaugeFrame,
		MUIA_Background, MUII_BACKGROUND,
	End;
	
	if( !busy )
	{
		MUI_DisposeObject( gauge );
		return ( 0 );
	}
	#endif

	obj = DoSuperNew( cl, obj,
		MUIA_Group_PageMode, TRUE,
		MUIA_Font, MUIV_Font_Tiny,
		MUIA_Weight, 42,
		Child, RectangleObject, End,
		Child, gauge,
		#if USE_BUSY
		Child, busy,
		#endif
		TAG_DONE
	);

	if( !obj )
	{
		MUI_DisposeObject( gauge );
		#if USE_BUSY
		MUI_DisposeObject( busy );
		#endif
		return ( 0 );
	}
	data = INST_DATA( cl, obj );

	data->gauge = gauge;

	return( (ULONG)obj );
}

DECMETHOD( Gauge_Reset, APTR )
{
	GETDATA;

#if USE_BUSY
	SetSuperAttrs( cl, obj,
		MUIA_Group_ActivePage, GAUGE_WAITING,
		TAG_DONE
	);
	SetAttrs( data->gauge,
		MUIA_Gauge_InfoText, "",
		MUIA_Gauge_Current, 0,
		TAG_DONE
	);
#else
	SetSuperAttrs( cl, obj,
		MUIA_Group_ActivePage, GAUGE_EMPTY,
		TAG_DONE
	);
	SetAttrs( data->gauge,
		MUIA_Gauge_InfoText, ( ULONG )"Waiting...", //TOFIX!!
		MUIA_Gauge_Current, 0,
		TAG_DONE
	);
#endif /* !USE_BUSY */

	data->page = GAUGE_WAITING;

	return( 0 );
}

DECMETHOD( Gauge_Clear, APTR )
{
	GETDATA;

	SetSuperAttrs( cl, obj, 
		MUIA_Group_ActivePage, GAUGE_EMPTY,
		TAG_DONE
	);
	SetAttrs( data->gauge,
		MUIA_Gauge_Current, 0, 
		MUIA_Gauge_InfoText, ( ULONG )"",
		TAG_DONE 
	);
	
	data->page = GAUGE_EMPTY;

	return( 0 );
}

DECSMETHOD( Gauge_Set )
{
	GETDATA;

	stccpy( data->txt, msg->txt, sizeof( data->txt ) );
	SetAttrs( data->gauge,
		MUIA_Gauge_Max, msg->max,
		MUIA_Gauge_Current, msg->current,
		MUIA_Gauge_InfoText, data->page == GAUGE_PROGRESS ? ( ULONG )data->txt : ( ULONG )"",
		TAG_DONE
	);
	if( data->page != GAUGE_PROGRESS )
	{
		SetSuperAttrs( cl, obj, MUIA_Group_ActivePage, GAUGE_PROGRESS, TAG_DONE );
		set( data->gauge, MUIA_Gauge_InfoText, data->txt );
		data->page = GAUGE_PROGRESS;
	}
	return( 0 );    
}

DECSMETHOD( Gauge_SetText )
{
	GETDATA;
	stccpy( data->txt, msg->txt, sizeof( data->txt ) );
	SetAttrs( data->gauge, MUIA_Gauge_InfoText, ( ULONG )data->txt, TAG_DONE );
	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	GETDATA;

	DOSUPER;

	// Hack-O-Kludge against Gauge class minwidth suckness
	_minwidth( data->gauge ) = 32;
	msg->MinMaxInfo->MinWidth = 32;

	return( 0 );
}

BEGINMTABLE
DEFNEW
//DEFSET
DEFSMETHOD( Gauge_Set )
DEFSMETHOD( Gauge_SetText )
DEFMETHOD( Gauge_Reset )
DEFMETHOD( Gauge_Clear )
DEFMMETHOD( AskMinMax )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_gaugeclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "GaugeClass";
#endif

	return( TRUE );
}

void delete_gaugeclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getgaugeclass( void )
{
	return( mcc->mcc_Class );
}
