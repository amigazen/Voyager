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
** $Id: lo_li.c,v 1.22 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
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

//
//  List of Image pool
//
struct myli {
	struct MinNode n;
	UWORD size; // x == y
	UWORD modulo;
	UWORD *mem;
};

//
//  Levels:
//  0 = circle
//  1 = triangle
//  2 = square
//  3..5 = shape only
//
//
//

#define NUMSHAPES 6
static struct MinList mylis[ NUMSHAPES ];
static APTR mylispool;
#if USE_TMPRAS
static struct TmpRas tempras;
#endif

#ifdef MBX
#define RP rp
#else
#define RP &rp
#endif /* !MBX */

static int init_mylis( void )
{
	int c;

	for( c = 0; c < NUMSHAPES; c++ )
		NEWLIST( &mylis[ c ] );

	mylispool = CreatePool( MEMF_CHIP | MEMF_CLEAR, 2048, 1024 );
#if USE_TMPRAS
	InitTmpRas( &tempras, AllocPooled( mylispool, 256 ), 256 );
#endif

	return( ( int )mylispool );
}

static void cleanup_lispool( void )
{
	if( mylispool )
		DeletePool( mylispool );
}

static struct myli *getmyli( int size, int shape )
{
#ifdef MBX
return(NULL); //TOFIX!!!
#else

	struct myli *m;
	struct BitMap bm;
#ifdef MBX
	struct RastPort *rp;
#else
	struct RastPort rp;
#endif /* !MBX */

	size = max( 4, size );
	shape = min( NUMSHAPES - 1, shape );

	for( m = FIRSTNODE( &mylis[ shape ] ); NEXTNODE( m ); m = NEXTNODE( m ) )
	{
		if( m->size == size )
			return( m );
	}

	// We have to create one..
	m = malloc( sizeof( *m ) );
	m->mem = AllocPooled( mylispool, ( ( size + 15 ) / 16 ) * size * 4 );

	m->size = size;
	m->modulo = ( size + 15 ) / 16 * 2;

	ADDTAIL( &mylis[ shape ], m );

	InitBitMap( &bm, 1, size, size );
#ifdef MBX
	if( rp = AllocRastPort( TAG_DONE ) ) //TOFIX!!
	{
		SetRastPortAttr( rp, RPTAG_BITMAP, &bm );
	}
#else
	bm.Planes[ 0 ] = (PLANEPTR)m->mem;
	InitRastPort( &rp );
	rp.rp_BitMap = &bm;
#endif

#if USE_TMPRAS
	rp.rp_TmpRas = &tempras;
#endif

	SetABPenDrMd( RP, 1, 0, JAM2 );

	// Now, draw the Image
	switch( shape )
	{
		case 0: // Circle
			DrawEllipse( RP, size / 2, size / 2, size / 2, size / 2 );
			Flood( RP, 0, size / 2, size / 2 );
			break;

		case 1: // Triangle
			Move( RP, 0, 0 );
			Draw( RP, size - 1, size / 2 );
			Draw( RP, 0, size - 1 );
			Draw( RP, 0, 0 );
			Flood( RP, 0, 1, size / 2 );
			break;

		case 2: // Rectangle
			RectFill( RP, 0, 0, size - 1, size - 1 );
			break;

		case 3: 
			DrawEllipse( RP, size / 2, size / 2, size / 2, size / 2 );
			break;

		case 4: 
			Move( RP, 0, 0 );
			Draw( RP, size - 1, size / 2 );
			Draw( RP, 0, size - 1 );
			Draw( RP, 0, 0 );
		break;

		case 5: 
			Move( RP, 0, 0 );
			Draw( RP, size - 1, 0 );
			Draw( RP, size - 1, size - 1 );
			Draw( RP, 0, size - 1 );
			Draw( RP, 0, 0 );
			break;
	}

#ifdef MBX
	FreeRastPort( rp );
#endif /* MBX */

	return( m );
#endif // MBX
}

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	int size, shape, index, color, type;
	int pen;
	struct myli *image;
	int reallength, textlength;
	int inlist;
	int depth;
	char txt[ 16 ];
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

		case MA_Layout_Listitem_Size:
			data->size = tag->ti_Data;
			break;

		case MA_Layout_Listitem_Shape:
			data->shape = tag->ti_Data;
			break;

		case MA_Layout_Listitem_Index:
			data->index = tag->ti_Data;
			sprintf( data->txt, "%u.", data->index );
			break;

		case MA_Layout_Listitem_Type:
			data->type = tag->ti_Data;
			break;

		case MA_Layout_Listitem_Color:
			data->color = tag->ti_Data;
			break;

		case MA_Layout_Listitem_InList:
			data->inlist = tag->ti_Data;
			break;

		case MA_Layout_Listitem_Depth:
			data->depth = tag->ti_Data;
			break;
	}

	return( redraw );
}

DECCONST
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

	data->li.valign = valign_middle;
	data->li.align = align_newrow;

	data->pen = LO_NOPEN;

	doset( data, obj, msg->ops_AttrList );

#if 0
	{
		char buffer[ 128 ];

		sprintf( buffer, "%u/%ld", data->inlist, data->depth );
		set( obj, MUIA_ShortHelp, strdup( buffer ) );
	}
#endif

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

		case MA_Layout_MarginEnd_Left:
			*msg->opg_Storage = 0;
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

	if( data->type )
	{
#ifdef MBX
		struct RastPort *rp;
		if( !( rp = AllocRastPort( TAG_DONE ) ) ) //TOFIX!!
		{
			//TOFIX!!
		}
#else
		struct RastPort rp;
		InitRastPort( &rp );
#endif /* !MBX */

		SetFont( RP, _font( obj ) );
		data->textlength = TextLength( RP, data->txt, strlen( data->txt ) );
		data->reallength = TextLength( RP, "888", 3 ) + 4;

		data->li.minwidth = data->reallength;
		data->li.maxwidth = data->reallength;
		data->li.defwidth = data->reallength;
		data->li.minheight = _font( obj )->tf_YSize;
	}
	else
	{
		data->li.minwidth = data->size;
		data->li.maxwidth = data->size;
		data->li.defwidth = data->size;
		data->li.minheight = data->size;
	}

	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;

	if( data->type )
	{
		data->li.ys = _font( obj )->tf_YSize;
		data->li.xs = data->reallength;
	}
	else
	{
		data->li.ys = data->size;
		data->li.xs = data->size;
	}

	return( (ULONG)&data->li );
}

DECMMETHOD( Draw )
{
	GETDATA;
	
	DOSUPER;

	if( !( msg->flags & MADF_DRAWOBJECT ) )
		return( 0 );

	if( data->pen == LO_NOPEN )
		data->pen = layout_getpen( data->ctx, data->color );


	if( data->type )
	{
		SetAPen( _rp( obj ), data->pen );
		SetFont( _rp( obj ), _font( obj ) );
		Move( _rp( obj ), _left( obj ) + data->reallength - data->textlength - 4, _top( obj ) + _font( obj )->tf_Baseline );
		Text( _rp( obj ), data->txt, strlen( data->txt ) );
	}
	else
	{
		int offset = ( data->size ) / 4;

		if( !data->image )
			data->image = getmyli( ( data->size - 2 ) / 2, data->shape );

		if( !data->image )
			return( 0 );

		SetAPen( _rp( obj ), data->pen );
		BltTemplate( (PLANEPTR)data->image->mem, 0, data->image->modulo, 
			_rp( obj ), _left( obj ) + 1, _top( obj ) + 1 + offset, 
			data->image->size, data->image->size 
		);
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

DECMMETHOD( Cleanup )
{
	GETDATA;

	data->pen = LO_NOPEN;

	return( DOSUPER );
}

BEGINMTABLE
DEFCONST
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( Draw )
DEFMMETHOD( AskMinMax )
DEFMMETHOD( Cleanup )
ENDMTABLE

int create_loliclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !init_mylis() )
		return( FALSE );

	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loliclass";
#endif

	return( TRUE );
}

void delete_loliclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
	cleanup_lispool();
}

APTR getloliclass( void )
{
	return( lcc->mcc_Class );
}
