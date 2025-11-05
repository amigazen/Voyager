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
** $Id: smartlabel.c,v 1.20 2003/07/06 16:51:34 olli Exp $
**
** smartlabel is a new simple right-aligned label class which can
** take up to 8 "MoreText" parameters and sets its minimum width
** to the maximum of all those labels
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/graphics.h>
#endif

/* private */
#include "classes.h"
#include "mui_func.h"

#define MAXMORE 8

struct Data {
	STRPTR contents;
	STRPTR morecontents[ MAXMORE ];
	int maintextlength;
};

DECNEW
{
	int cnts = 0;
	struct TagItem *tag, *tagp;
	struct Data *data;

	obj = (APTR)DOSUPER;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	tagp = msg->ops_AttrList;

	while( tag = NextTagItem( &tagp ) )
	{
		if( tag->ti_Tag == MA_Smartlabel_Text )
		{
			data->contents = (STRPTR)tag->ti_Data;
		}
		else if( tag->ti_Tag == MA_Smartlabel_MoreText )
		{
			if( cnts < MAXMORE )
				data->morecontents[ cnts++ ] = (STRPTR)tag->ti_Data;
		}
	}

	return( (ULONG)obj );
}

DECSET
{
	struct TagItem *tag;

	tag = FindTagItem( MA_Smartlabel_Text, msg->ops_AttrList );
	if( tag )
	{
		GETDATA;

		data->contents = (STRPTR)tag->ti_Data;

#ifdef MBX
		data->maintextlength = mbxtextlen(data->contents,strlen(data->contents),_font(obj));
#else
		{
			struct RastPort rp;
			InitRastPort( &rp );
			SetFont( &rp, _font( obj ) );
			data->maintextlength = TextLength( &rp, data->contents, strlen( data->contents ) );
		}
#endif /* !MBX */

		MUI_Redraw( obj, MADF_DRAWOBJECT );
	}
	return( DOSUPER );
}

DECMMETHOD( AskMinMax )
{
	struct MUI_MinMax *mix;
	GETDATA;
	int ml, c;
	#ifndef MBX
	struct RastPort rp;
	#endif

	DOSUPER;
	mix = msg->MinMaxInfo;

	#ifdef MBX
	data->maintextlength = mbxtextlen(data->contents,strlen(data->contents),_font(obj));
	#else
	InitRastPort( &rp );
	SetFont( &rp, _font( obj ) );
	data->maintextlength = TextLength( &rp, data->contents, strlen( data->contents ) );
	#endif

	ml = data->maintextlength;

	for( c = 0; c < MAXMORE; c++ )
	{
		int tl;

		if( !data->morecontents[ c ] )
			break;

		#ifdef MBX
		tl = mbxtextlen(data->morecontents[ c ], strlen( data->morecontents[ c ] ), _font(obj) );
		#else
		tl = TextLength( &rp, data->morecontents[ c ], strlen( data->morecontents[ c ] ) );
		#endif

		ml = max( tl, ml );
	}

	mix->MinWidth += ml;
	mix->MinHeight += _font( obj )->tf_YSize;
	mix->DefWidth += ml;
	mix->DefHeight += _font( obj )->tf_YSize;
	//mix->MaxWidth += 10000;
	//mix->MaxHeight += 10000;
	mix->MaxWidth += ml;
	//mix->MaxHeight += _font( obj )->tf_YSize;
	mix->MaxHeight += MUI_MAXMAX;

	return( 0 );
}

DECMMETHOD( Draw )
{
	GETDATA;

	DOSUPER;

	SetFont( _rp( obj ), _font( obj ) );
	SetAPen( _rp( obj ), _pens( obj )[ MPEN_TEXT ] );
	Move( _rp( obj ), _mright( obj ) - data->maintextlength + 1, _mtop( obj ) + _font( obj )->tf_Baseline + ( _mheight( obj ) - _font( obj )->tf_YSize ) / 2 );
	Text( _rp( obj ), data->contents, strlen( data->contents ) );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFSET
DEFMMETHOD( AskMinMax )
DEFMMETHOD( Draw )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_smartlabelclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "SmartLabelClass";
#endif

	return( TRUE );
}

void delete_smartlabelclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getsmartlabelclass( void )
{
	return( mcc->mcc_Class );
}
