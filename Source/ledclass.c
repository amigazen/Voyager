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
** $Id: ledclass.c,v 1.29 2003/11/18 10:37:40 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/graphics.h>
#endif


/* private */
#include "classes.h"
#include "prefs.h"
#include "methodstack.h"
#include "mui_func.h"


#define MAXLEDNUM 32

static struct MUI_CustomClass *lcc;

#define PEN0_R 0x3f
#define PEN0_G 0x39
#define PEN0_B 0x0a

#define PEN1_R 0x8f
#define PEN1_G 0
#define PEN1_B 0

#define PEN2_R 0xef
#define PEN2_G 0
#define PEN2_B 0

#define PEN3_R 0
#define PEN3_G 0xcf
#define PEN3_B 0

#define PEN4_R 0
#define PEN4_G 0x8f
#define PEN4_B 0

#define PEN5_R 127
#define PEN5_G 118
#define PEN5_B 137

#ifndef MBX
#define ALLOCPEN( r, g, b ) ObtainBestPen( scr->sc_ViewPort.vp_ColorMap, MAKE_ID( r, r, r, r ), MAKE_ID( g, g, g, g ), MAKE_ID( b, b, b, b ), OBP_Precision, PRECISION_EXACT, TAG_DONE )
#else
#define ALLOCPEN( r, g, b ) ((r<<16)|(g<<8)|b)
#endif

static UBYTE state[ MAXLEDNUM ];
static UBYTE changed[ MAXLEDNUM ];
static int usecount = 0;
static struct BitMap *bm = NULL;
static ULONG pens[ 7 ];
static struct MinList users;
static struct SignalSemaphore userssem;

static int ledcount;

struct Data {
	struct MinNode n;
	APTR obj;
	int yoff;
};

#define LEDXS 7
#define LEDYS 7

static void drawledframe( struct RastPort *rp, int x, int y, int shadowpen, int shinepen )
{
	SetAPen( rp, shadowpen );
	RectFill( rp, x + 2, y, x + 4, y );
	RectFill( rp, x + 1, y + 1, x + 1, y + 1 );
	RectFill( rp, x, y + 2, x, y + 4 );
	RectFill( rp, x + 1, y + 5, x + 1, y + 5 );
	SetAPen( rp, shinepen );
	RectFill( rp, x + 5, y + 1, x + 5, y + 1 );
	RectFill( rp, x + 6, y + 2, x + 6, y + 4 );
	RectFill( rp, x + 5, y + 5, x + 5, y + 5 );
	RectFill( rp, x + 2, y + 6, x + 4, y + 6 );
}

static void drawled( struct RastPort *rp, int x, int y, int penfill, int penwhite )
{
	SetAPen( rp, penfill );

	RectFill( rp, x + 1, y + 2, x + 5, y + 4 );
	RectFill( rp, x + 2, y + 1, x + 4, y + 1 );
	RectFill( rp, x + 2, y + 5, x + 4, y + 5 );

	SetAPen( rp, penwhite );
	//RectFill( rp, x + 2, y + 2, x + 2, y + 2 );
	WritePixel( rp, x + 2, y + 2 );
}

//#if USE_SSCREEN
static void initbitmap( APTR obj, struct Screen *scr, int shadowpen, int shinepen )
{
	struct RastPort *rp = NULL;
	#ifndef MBX
	struct RastPort tmprp;
	#endif

	int c;

	if( usecount++ )
		return;

	pens[ 0 ] = ALLOCPEN( PEN0_R, PEN0_G, PEN0_B );
	pens[ 1 ] = ALLOCPEN( PEN1_R, PEN1_G, PEN1_B );
	pens[ 2 ] = ALLOCPEN( PEN2_R, PEN2_G, PEN2_B );
	pens[ 3 ] = ALLOCPEN( PEN3_R, PEN3_G, PEN3_B );
	pens[ 4 ] = ALLOCPEN( PEN4_R, PEN4_G, PEN4_B );
	pens[ 5 ] = ALLOCPEN( PEN5_R, PEN5_G, PEN5_B );
	pens[ 6 ] = ALLOCPEN( 0xff, 0xff, 0xff );

	if ((bm=AllocBitMap(LEDXS*6,LEDYS,GetBitMapAttr(scr->sc_RastPort.rp_BitMap, BMA_DEPTH),BMF_MINPLANES,scr->sc_RastPort.rp_BitMap)))
	{
		InitRastPort( &tmprp );
		rp = &tmprp;
		rp->rp_BitMap = bm;
	}

	if (rp)
	{
		SetAPen( rp, _pens( obj )[ MPEN_BACKGROUND ] );
		RectFill( rp, 0, 0, LEDXS * 6 - 1, LEDYS - 1 );

		for( c = 0; c < 6; c++ )
		{
			drawledframe( rp, c * LEDXS, 0, shadowpen, shinepen );
			drawled( rp, c * LEDXS, 0,
				pens[ c ],
				pens[ 6 ]
			);
		}
	}
}

static void exitbitmap( struct Screen *scr )
{
	int c;

	if( --usecount )
		return;

	for( c = 0; c < 7; c++ )
		ReleasePen( scr->sc_ViewPort.vp_ColorMap, pens[ c ] );

	if (bm)
	{
		FreeBitMap( bm );
		bm = NULL;
	}
}


DECMMETHOD( Setup )
{
	struct Data *data = INST_DATA( cl, obj );
	int shadowpen, shinepen;

	if( !DOSUPER )
		return( FALSE );

	ledcount = getprefslong( DSI_NET_MAXCON, 8 );

	shadowpen = _dri( obj )->dri_Pens[ SHADOWPEN ];
	shinepen = _dri( obj )->dri_Pens[ SHINEPEN ];
	initbitmap( obj, _screen( obj ), shadowpen, shinepen );

	ObtainSemaphore( &userssem );
	ADDTAIL( &users, &data->n );
	ReleaseSemaphore( &userssem );

	return( TRUE );
}

DECMMETHOD( Show )
{
	GETDATA;

	if( !DOSUPER )
		return( FALSE );
	data->yoff = _mtop( obj ) + ( _mheight( obj ) - 15 ) / 2;

	return( TRUE );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	ObtainSemaphore( &userssem );
	REMOVE( &data->n );
	ReleaseSemaphore( &userssem );

	exitbitmap( _screen( obj ) );

	killpushedmethods( obj );

	return( DOSUPER );
}

DECMMETHOD( Draw )
{
	int c;

	struct Data *data = INST_DATA(cl,obj);

	DOSUPER;

	/*
	** if MADF_DRAWOBJECT isn't set, we shouldn't draw anything.
	** MUI just wanted to update the frame or something like that.
	*/

	if( msg->flags & ( MADF_DRAWOBJECT | MADF_DRAWUPDATE ) )
	{
		int drawo = msg->flags & MADF_DRAWOBJECT;
		int noneven = ledcount & 1;

		for( c = 0; c < ledcount; c++ )
		{
			if( drawo || changed[ c ] )
			{
				int cnt2 = ( ledcount + 1 ) / 2;
				int myoff = data->yoff + ( c >= cnt2 ? LEDYS + 1 : 0 );

				if( changed[ c ] )
					changed[ c ]--;

				if( noneven && ( c == ( cnt2 - 1 ) ) )
					myoff += 3;

				if (bm)
				{
					BltBitMapRastPort(
						bm, state[ c ] * LEDXS, 0,
						_rp( obj ), _mleft( obj ) + ( c % cnt2 ) * ( LEDXS + 1 ), myoff,
						LEDXS, LEDYS,
						0xc0
					);
				}
			}
		}
	}

	return(0);
}

DECNEW
{
	struct Data *data;

	obj = ( Object * ) DOSUPER;
	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->obj = obj;

	return( (ULONG) obj );
}

DECMMETHOD( AskMinMax )
{
	struct MUI_MinMax *mmx;

	DOSUPER;

	mmx = msg->MinMaxInfo;
	mmx->MinWidth += ( ( ledcount + 1 ) / 2 ) * ( LEDXS + 1 ) - 1;
	mmx->MinHeight += LEDYS * 2 + 1;

	mmx->DefWidth = mmx->MinWidth;
	mmx->DefHeight += LEDYS * 2 + 1;

	mmx->MaxWidth = mmx->MinWidth;
	mmx->MaxHeight += MUI_MAXMAX;

	return( 0 );
}

DECMETHOD( Led_Update, APTR )
{
	MUI_Redraw( obj, MADF_DRAWUPDATE );
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMMETHOD( Draw )
DEFMMETHOD( Show )
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( AskMinMax )
DEFMETHOD( Led_Update )
ENDMTABLE

int create_ledclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &users );
	InitSemaphore( &userssem );
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
	{
		return( FALSE );
	}

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "LedClass";
#endif

	return( TRUE );
}

void delete_ledclass( void )
{
	if( lcc )
	{
		MUI_DeleteCustomClass( lcc );
		lcc = NULL; //TOFIX!! (I think)
	}
}

APTR getledclass( void )
{
	return( lcc->mcc_Class );
}

void setled( int led, int newstate )
{
	ObtainSemaphore( &userssem );
	if( changed[ led ] < usecount || state[ led ] != newstate )
	{
		struct Data *d;

		state[ led ] = newstate;
		changed[ led ] = usecount;

		for( d = FIRSTNODE( &users ); NEXTNODE( d ); d = NEXTNODE( d ) )
			pushmethod( d->obj, 1, MM_Led_Update );
	}
	ReleaseSemaphore( &userssem );
}
