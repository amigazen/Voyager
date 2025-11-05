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
** $Id: fonttestclass.c,v 1.9 2003/07/06 16:51:33 olli Exp $
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
	struct TextFont *font;
};

#define TT "The Quick Brown Fox Jumps Over The Lazy Dog"

DECMMETHOD( Draw )
{
	GETDATA;

	DOSUPER;

	if( data->font )
	{
		APTR clip;
		int tl;
		int base;

		clip = MUI_AddClipping( muiRenderInfo( obj ),
			_mleft( obj ), _mtop( obj ),
			_mwidth( obj ), _mheight( obj )
		);

		SetFont( _rp( obj ), data->font );
		tl = TextLength( _rp( obj ), TT, strlen( TT ) );

		base = _mbottom( obj ) - 1 - ( data->font->tf_YSize - data->font->tf_Baseline );

		// draw top/bottom font cell marks
		SetAPen( _rp( obj ), _pens( obj )[ MPEN_FILL ] );
		RectFill( _rp( obj ),
			_mleft( obj ) + 2,
			( base - data->font->tf_Baseline ) + 1,
			_mleft( obj ) + 2 + tl,
			( base - data->font->tf_Baseline ) + 1
		);
		RectFill( _rp( obj ),
			_mleft( obj ) + 2,
			base + ( data->font->tf_YSize - data->font->tf_Baseline ),
			_mleft( obj ) + 2 + tl,
			base + ( data->font->tf_YSize - data->font->tf_Baseline )
		);

		// draw baseline
		RectFill( _rp( obj ),
			_mleft( obj ) + 2,
			base,
			_mleft( obj ) + 2 + tl,
			base
		);

		SetAPen( _rp( obj ), _pens( obj )[ MPEN_TEXT ] );

		Move( _rp( obj ),
			_mleft( obj ) + 2,
			base
		);
		Text( _rp( obj ), TT, strlen( TT ) );

		MUI_RemoveClipping( muiRenderInfo( obj ), clip );
	}
	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;
	msg->MinMaxInfo->MinWidth += 36;
	msg->MinMaxInfo->MinHeight += 18;

	return( TRUE );
}

DECMETHOD( Fonttest_SetFont, APTR )
{
	GETDATA;

	data->font = msg[ 1 ];
	MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( 0 );
}

BEGINMTABLE
DEFMMETHOD( Draw )
DEFMETHOD( Fonttest_SetFont )
DEFMMETHOD( AskMinMax )
ENDMTABLE

int create_fonttestclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "FonttestClass";
#endif

	return( TRUE );
}

void delete_fonttestclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getfonttestclass( void )
{
	return( lcc->mcc_Class );
}
