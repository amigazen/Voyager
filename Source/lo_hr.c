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
** $Id: lo_hr.c,v 1.19 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/graphics.h>
#endif

/* private */
#include "classes.h"
#include <proto/vimgdecode.h>
#include "prefs.h"
#include "voyager_cat.h"
#include "js.h"
#include "urlparser.h"
#include "htmlclasses.h"
#include "layout.h"
#include "fontcache.h"
#include "gfxcompat.h"
#include "mui_func.h"

static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *widthspec;
	int noshade;
	int size;
	int penspec1, penspec2;
	int pen1, pen2;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			break;

		case MA_Layout_Width:
			l_readstrtag( tag, &data->widthspec );
			break;

		case MA_Layout_HR_NoShade:
			data->noshade = tag->ti_Data;
			break;

		case MA_Layout_HR_Penspec1:
			data->penspec1 = tag->ti_Data;
			break;

		case MA_Layout_HR_Penspec2:
			data->penspec2 = tag->ti_Data;
			break;
			
		case MA_Layout_HR_Size:
			data->size = tag->ti_Data;
			break;

	}

	return( redraw );
}

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->li.valign = valign_top;
	data->li.align = align_newrowrow;
	data->pen1 = LO_NOPEN;
	data->pen2 = LO_NOPEN;

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_Layout_Info:
			*msg->opg_Storage = (ULONG)&data->li;
			return( TRUE );

		case MA_Layout_TempLineAlign:
			*msg->opg_Storage = (ULONG)linealign_center;
			return( TRUE );
	}

	return( DOSUPER );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( DOSUPER );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	int width = msg->suggested_width;

	if( data->widthspec )
	{
		if( strchr( data->widthspec, '%' ) )
		{
			width = msg->suggested_width * atoi( data->widthspec ) / 100;
		}
		else
			width = atoi( data->widthspec );
	}

	data->li.minwidth = 1;
	data->li.maxwidth = width;
	data->li.defwidth = width;
	data->li.minheight = data->size;

	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	int width = msg->suggested_width;

	if( data->widthspec )
	{
		if( strchr( data->widthspec, '%' ) )
		{
			width = msg->suggested_width * atoi( data->widthspec ) / 100;
		}
		else
			width = atoi( data->widthspec );
	}

	data->li.ys = data->size + _subheight( obj );
	data->li.xs = width;

	return( (ULONG)&data->li );
}

DECMMETHOD( Draw )
{
	GETDATA;
	struct RastPort *rp = _rp( obj );
	
	DOSUPER;
	
	if( data->pen1 == LO_NOPEN )
	{
		data->pen1 = layout_getpen( data->ctx, data->penspec1 );
	}

	SetAPen( rp, data->pen1 );

	if( data->noshade )
	{
		RectFill( rp, _mleft( obj ), _mtop( obj ), _mright( obj ), _mbottom( obj ) );
	}
	else
	{
		if( data->pen2 == LO_NOPEN )
			data->pen2 = layout_getpen( data->ctx, data->penspec2 );
		HLine( rp, _mleft( obj ), _mright( obj ), _mtop( obj ) );
		VLine( rp, _mleft( obj ), _mtop( obj ), _mbottom( obj ) );
		SetAPen( rp, data->pen2 );
		HLine( rp, _mleft( obj ), _mright( obj ), _mbottom( obj ) );
		VLine( rp, _mright( obj ), _mtop( obj ), _mbottom( obj ) ); 
	}
		
	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

BEGINMTABLE
DEFCONST
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( Draw )
DEFMMETHOD( AskMinMax )
ENDMTABLE

int create_lohrclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lohrClass";
#endif

	return( TRUE );
}

void delete_lohrclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlohrclass( void )
{
	return( lcc->mcc_Class );
}
