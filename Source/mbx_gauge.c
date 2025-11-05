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
** $Id: mbx_gauge.c,v 1.12 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"
#include "classes.h"
#include "mui_func.h"
#if USE_STB_NAV
#include <mbxgui_lib_calls.h>
#include <modules/mbxgui/classes.h>
#endif

#if USE_STB_NAV

/*
 * Gauge states
 */
#define GAUGE_EMPTY 0
#define GAUGE_PROGRESS 1
#define GAUGE_WAITING 2

struct Data {
	int mval, cval;
	int ihn_active;
	struct MUI_InputHandlerNode ihn;
};

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_Weight, 42,
		MUIA_Group_Horiz, TRUE,
		TAG_DONE
	);

	data = INST_DATA( cl, obj );

	data->mval = -1;
	data->cval = -1;

	return( (ULONG)obj );
}

DECTMETHOD( Gauge_Reset )
{
	GETDATA;

	set( obj, MUIA_Date_UpdateRate, MUIV_Date_UpdateRate_None );

	data->cval = 0;
	data->mval = 0;

	if( !data->ihn_active )
	{
		data->ihn.ihn_Object = obj;
		data->ihn.ihn_Flags = MUIIHNF_TIMER;
		data->ihn.ihn_Millis = 100;
		data->ihn.ihn_Method = MM_Gauge_Tick;
		data->ihn_active = TRUE;
		DoMethod( app, MUIM_Application_AddInputHandler, ( ULONG )&data->ihn );		
	}

	MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( 0 );
}

DECTMETHOD( Gauge_Clear )
{
	GETDATA;

	set( obj, MUIA_Date_UpdateRate, MUIV_Date_UpdateRate_Auto );

	if( data->ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );
		data->ihn_active = FALSE;
	}

	data->cval = -1;
	data->mval = -1;
	
	MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( 0 );
}

DECSMETHOD( Gauge_Set )
{
	GETDATA;

	if( msg->current != data->cval || data->mval != msg->max )
	{
		if( msg->max == 0 )
		{
			if( !data->ihn_active )
			{
				data->ihn.ihn_Object = obj;
				data->ihn.ihn_Flags = MUIIHNF_TIMER;
				data->ihn.ihn_Millis = 200;
				data->ihn.ihn_Method = MM_Gauge_Tick;
				data->ihn_active = TRUE;
				DoMethod( app, MUIM_Application_AddInputHandler, ( ULONG )&data->ihn );
			}
		}
		else
		{
			if( data->ihn_active )
			{
				DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );
				data->ihn_active = FALSE;
			}
			data->cval = msg->current;
		}

		data->mval = msg->max;

		MUI_Redraw( obj, MADF_DRAWOBJECT );
	}
	return( 0 );    
}

DECSMETHOD( Gauge_SetText )
{
	// Not used in this class
	return( 0 );
}

DECMMETHOD( Draw )
{
	GETDATA;
	int pos;

	// Draw...
	if( data->mval > 0 )
	{
		pos = ( _mwidth( obj ) * data->cval ) / data->mval;
		SetAPen( _rp( obj ), 0x0000ff );
		RectFill( _rp( obj ), _mleft( obj ), _mtop( obj ), _mleft( obj ) + pos, _mbottom( obj ) );
		SetAPen( _rp( obj ), 0x000000 );
		RectFill( _rp( obj ), _mleft( obj ) + pos, _mtop( obj ), _mright( obj ), _mbottom( obj ) );
	}
	else if( data->mval == 0 )
	{
		int c;
		int cnt = ( _mwidth( obj ) + 4 ) / 5;
		int pos = _mleft( obj );

		for( c = 0; c < cnt; c++ )
		{
			int thiswidth;
			
			thiswidth = _mright( obj ) - pos;
			if( thiswidth > 4 )
				thiswidth = 4;
			
			SetAPen( _rp( obj ), ( data->cval == ( c % 3 ) ) ? 0x0000ff : 0x000020 );
			RectFill( _rp( obj ), pos, _mtop( obj ), pos + thiswidth, _bottom( obj ) );
			pos += 5;
		}
	}
	else
	{
		return( DOSUPER );
	}
	return( 0 );
}

DECDEST
{
	GETDATA;

	if( data->ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );
	}

	return( DOSUPER );
}

DECTMETHOD( Gauge_Tick )
{
	GETDATA;

	if( !data->mval )
	{
		data->cval = ( data->cval + 1 ) % 3;
		MUI_Redraw( obj, MADF_DRAWUPDATE );
	}

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( Gauge_Set )
DEFSMETHOD( Gauge_SetText )
DEFMETHOD( Gauge_Reset )
DEFMETHOD( Gauge_Clear )
DEFMMETHOD( Draw )
DEFTMETHOD( Gauge_Tick )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_stbgaugeclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "stbgaugeclass";
#endif

	return( TRUE );
}

void delete_stbgaugeclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getstbgaugeclass( void )
{
	return( mcc->mcc_Class );
}

#endif
