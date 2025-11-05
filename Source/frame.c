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
** $Id: frame.c,v 1.13 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/graphics.h>
#endif

/* private */
#include "classes.h"
#include "mui_func.h"

static struct MUI_CustomClass *lcc;

struct Data {
	ULONG y;
	UWORD x;
	UWORD xs, ys;
	ULONG size;
	ULONG p_shadow, p_shine;
};

DECNEW
{
	struct Data *data;
	INITASTORE;

	if( !( obj = (Object*)DoSuperNew( cl, obj, 
		MUIA_Background, MUII_BACKGROUND,
		MUIA_FillArea, FALSE, 
	TAG_DONE ) ) )
		return( 0 );

	data = INST_DATA( cl, obj );

	BEGINASTORE

		ASTORE( MA_DNode_X, x )
		ASTORE( MA_DNode_XS, xs )
		ASTORE( MA_DNode_Y, y )
		ASTORE( MA_DNode_YS, ys )
		ASTORE( MA_DNode_PenShadow, p_shadow )
		ASTORE( MA_DNode_PenShine, p_shine )
		ASTORE( MA_TFrame_Size, size )

	ENDASTORE

	return( (ULONG)obj );
}

DECMMETHOD( Draw )
{
	struct Data *data = INST_DATA( cl, obj );

	DOSUPER;

	if( msg->flags & MADF_DRAWOBJECT )
	{
		struct RastPort *rp = _rp( obj );
		// first, draw thick borders
		SetAPen( rp, data->p_shine );
		RectFill( rp, 
			_left( obj ), _top( obj ),
			_right( obj ), _top( obj ) + data->size - 1
		);
		RectFill( rp, 
			_left( obj ), _top( obj ),
			_left( obj ) + data->size - 1,
			_bottom( obj )
		);
		SetAPen( rp, data->p_shadow );
		RectFill( rp, 
			_right( obj ) - data->size + 1,
			_top( obj ) + 1,
			_right( obj ),
			_bottom( obj )
			
		);
		RectFill( rp, 
			_left( obj ) + 1,
			_bottom( obj ) - data->size + 1,
			_right( obj ),
			_bottom( obj )
		);
		if( data->size > 1 )
		{
			int c;

			SetAPen( rp, data->p_shine );
			for( c = 1; c < data->size; c++ )
			{
				RectFill( rp, 
					_right( obj ) - data->size + 1,
					_top( obj ) + c,
					_right( obj ) - c,
					_top( obj ) + c
				);
				RectFill( rp, 
					_left( obj ),
					_bottom( obj ) - c,
					_left( obj ) + c,
					_bottom( obj ) - c
				);
			}
		}
	}
	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	//DOSUPER;

	msg->MinMaxInfo = &muiAreaData( obj )->mad_MinMax;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMMETHOD( Draw )
DEFMMETHOD( AskMinMax )
ENDMTABLE

int create_frameclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "FrameClass";
#endif

	return( TRUE );
}

void delete_frameclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getframeclass( void )
{
	return( lcc->mcc_Class );
}
