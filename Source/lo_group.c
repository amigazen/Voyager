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
** $Id: lo_group.c,v 1.257 2004/06/09 22:50:32 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <intuition/extensions.h>
#include <graphics/gfxmacros.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#if USE_CGX
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#endif /* USE_CGX */
#endif
#include <limits.h>

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
#include "textfit.h"
#ifndef MBX
#include "bitmapclone.h"
#endif
#include "gfxcompat.h"
#include "methodstack.h"
#include "malloc.h"
#include "mui_func.h"
#include "menus.h"
#include "rexx.h"
#include "docinfowin.h"
#include "download.h"
#include "clip.h"

#include <stdio.h>

static struct MUI_CustomClass *lcc;

#define LISTINDENT 20

#define USE_FAST_LISTWALK 1 /* should speed up scrolling but is disabled for now */

/*
	Container node. This can be used for both text
	and for storing pointers to objects
*/
struct tn {
	struct MinNode n;
	int textlen;

	// This used to be a union, but gcc sucks ;)
	struct TextFont *font;
	APTR o;

	UBYTE *fontarray;
	struct layout_info *li;

	ULONG style;
	ULONG penspec;
	APTR anchor;

	unsigned char text[ 0 ];
};

/*
	Text view node -- text after splitting down
*/
struct tnr {
	struct MinNode n;
	int xp, yp;
	struct tn *tn;
	unsigned char *text;
	int textlen;
	int pixellen;
	ULONG pen;
	UBYTE redrawme, anchorselected;
	UBYTE anchoractive;
	UBYTE pad[ 1 ];
};

struct Data {
	struct layout_info li;
	APTR pool;
	struct MinList l;		// List of text fragments
	struct MinList v;		// List of text view nodes

	struct layout_ctx *ctx;

	ULONG bgcolor;
	ULONG bgcolor_pen;

	struct imgclient *bgimage;
	STRPTR bgimage_name;   /* name of the background for reloading */
	struct BitMap *bgbitmap;
	struct BitMap *bgmask;
	int bgbitmap_xs, bgbitmap_ys;
	int bgbitmap_has_mask;	// 1 == has mask, 2 == converted
	int bgbitmap_free;

	int changed;			// Group contents changed after last DoLayout

	int is_not_empty;

	APTR selected_anchor;
	APTR last_anchor;
	char last_anchor_url[ 240 ];

	int linealign;
	int templinealign;

	// Layout specifications
	char *widthspec;
	char *heightspec;
	int rowspan;
	int colspan;
	int row;
	int border;
	int topoffset;
	int default_linealign;

	int innerleft, innertop, innerbottom, innerright;

	// Incremental stuff
	int newobjects;

	// Border stuff
	int penspec_borderdark, penspec_borderlight;
	int pen_borderdark, pen_borderlight;

	// Flags
	UBYTE has_no_childs_with_backfilling;
	UBYTE nobackfill;
	UBYTE muibackground;
	UBYTE did_final_cleanup;

	// Context menu
	APTR cmenu;
	char last_cmenu_anchor[ 240 ]; /* anchor over which the context menu was pressed */

	// Text drawing speed up
	ULONG redrawlink;
	#if USE_FAST_LISTWALK
	struct tnr *last_tnr;
	#endif

	struct MUI_EventHandlerNode ehnode;
};

static struct tnr *find_tnr_by_xy( APTR obj, struct Data *data, int mousex, int mousey )
{
	struct tnr *tnr;

	mousex -= _left( obj );
	mousey -= _vtop( obj ) + data->topoffset;

	for( tnr = FIRSTNODE( &data->v ); NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
	{
		if( tnr->xp <= mousex && tnr->xp + tnr->pixellen > mousex &&
			tnr->yp - tnr->tn->font->tf_Baseline <= mousey && tnr->yp + tnr->tn->font->tf_YSize - tnr->tn->font->tf_Baseline > mousey
		)
		{
			return( tnr );
		}
	}

	return( NULL );
}

MUI_HOOK( layout, APTR grp, struct MUI_LayoutMsg *lm )
{
	struct Data *data = INST_DATA( lcc->mcc_Class, grp );

	switch( lm->lm_Type )
	{
		case MUILM_MINMAX:
			lm->lm_MinMax.MinWidth = 1;
			lm->lm_MinMax.MinHeight = 1;
			lm->lm_MinMax.DefWidth = 512;
			lm->lm_MinMax.DefHeight = 512;
			lm->lm_MinMax.MaxWidth = MUI_MAXMAX;
			lm->lm_MinMax.MaxHeight = MUI_MAXMAX;
			return( 0 );

		case MUILM_LAYOUT:
			{
				Object *cstate = (Object *)lm->lm_Children->mlh_Head;
				Object *child = NextObject( &cstate );
				int offsx = _left( grp );
				int offsy = _vtop( grp ) + data->topoffset;

				MUI_LayoutObj( child, offsx, offsy, 1, 1, 0 );

				while (child = NextObject(&cstate))
				{
					struct layout_info *li;
					int xs = 1, ys = 1;

					get( child, MA_Layout_Info, &li );

					if( li->xs > 8192 )
						xs = 8192;
					else if( li->xs > 0 )
						xs = li->xs;

					if( li->ys > 8192 )
						ys = 8192;
					else if( li->ys > 0 )
						ys = li->ys;

					// Hack-O-Kludge
					MUI_LayoutObj( child, li->xp + offsx, li->yp + offsy, xs, ys, 0 );
				}

				lm->lm_Layout.Height = data->li.ys;
				lm->lm_Layout.Width = data->li.xs;
			}
			return( TRUE );
	}

	return( MUILM_UNKNOWN );
}

static int doset( struct Data *data, APTR obj, struct TagItem *tags, int *sendup )
{
	struct TagItem *tag;
	int redraw = FALSE;

#define KILLT tag->ti_Tag = TAG_IGNORE

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_BGColor:
			data->bgcolor = tag->ti_Data;

			if( tag->ti_Data != LO_NOPEN )
				data->li.flags |= LOF_BACKFILLING;

			// We don't want this to be sent to our children
			data->bgcolor_pen = LO_NOPEN;
			redraw = TRUE;
			KILLT;
			break;

		case MA_Layout_Group_UseMUIBackground:
			data->muibackground = TRUE;
			KILLT;
			break;

		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			KILLT;
			break;

		/*
		 * Loads the background
		 */
		case MA_Layout_BGImageURL:
			if( data->bgimage )
				imgdec_close( data->bgimage );
			if( gp_loadimages_bg )
				data->bgimage = imgdec_open( (STRPTR)tag->ti_Data, obj, NULL, FALSE );
			KILLT;
			break;

		/*
		 * Store the URL of the background image so that
		 * we can reload it if needed/asked
		 */
		case MA_Layout_BGImageURLName:
			if( data->bgimage_name )
			{
				free( data->bgimage_name );
				data->bgimage_name = 0;
			}
			data->bgimage_name = strdup( ( STRPTR )tag->ti_Data );
			KILLT;
			break;

		case MA_Layout_LineAlign:
			data->linealign = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_TempLineAlign:
			data->templinealign = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_VAlign:
			data->li.valign = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_Align:
			data->li.align = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_Width:
			l_readstrtag( tag, &data->widthspec );
			KILLT;
			break;

		case MA_Layout_Height:
			l_readstrtag( tag, &data->heightspec );
			KILLT;
			break;

		case MA_Layout_Cell_Rowspan:
			data->rowspan = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_Cell_Row:
			data->row = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_Cell_Colspan:
			data->colspan = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_Cell_Border:
			data->border = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_BorderDark:
			data->penspec_borderdark = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_BorderLight:
			data->penspec_borderlight = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_MarginLeft:
			data->innerleft = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_MarginRight:
			data->innerright = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_MarginTop:
			data->innertop = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_MarginBottom:
			data->innerbottom = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_TopOffset:
			data->topoffset = tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_Cell_DefaultLineAlign:
			data->default_linealign = tag->ti_Data;
			KILLT;
			break;

		default:
			if( sendup )
				*sendup = TRUE;
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		MUIA_CustomBackfill, TRUE,
		MUIA_Group_LayoutHook, &layout_hook,
		Child, NewObject( getlodummyclass(), NULL, TAG_DONE ),
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->bgcolor = LO_NOPEN;
	data->bgcolor_pen = LO_NOPEN;
	data->pen_borderdark = LO_NOPEN;
	data->pen_borderlight = LO_NOPEN;

	data->li.flags = LOF_NEW;

	data->templinealign = -1;
	data->linealign = -1;

	doset( data, obj, msg->ops_AttrList, NULL );

	NEWLIST( &data->l );
	NEWLIST( &data->v );

	data->pool = CreatePool( 0, 1024, 512 );

	set( obj, MUIA_ContextMenu, 1 );

	return( (ULONG)obj );
}

DECDISP
{
	GETDATA;
	struct tn *tn;

	if( data->bgbitmap_free )
	{
		FreeBitMap( data->bgbitmap );
		if( data->bgmask > 0 && data->bgmask != ( APTR )-1 )
		{
			FreeBitMap( data->bgmask );
		}
	}

	if( data->bgimage )
		imgdec_close( data->bgimage );

	if( data->bgimage_name )
		free( data->bgimage_name );

	//
	// Remove any object which is under GC control
	// so it doesn't get removed during DISPOSE
	//
	for( tn = FIRSTNODE( &data->l ); NEXTNODE( tn ); tn = NEXTNODE( tn ) )
	{
		if( !tn->textlen )
		{
			if( DoMethod( tn->o, MM_JS_GetGCMagic ) )
			{
				DoMethod( obj, OM_REMMEMBER, tn->o );
			}
		}
	}

	DeletePool( data->pool );

	killpushedmethods( obj );

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		// Attributes we don't want to forward to childs
		case MA_Layout_FormElement_Form:
		case MA_Layout_MarginEnd_Left:
		case MA_Layout_MarginEnd_Right:
			return( FALSE );

		STOREATTR( MA_Layout_Info, &data->li )
		STOREATTR( MA_Layout_Cell_Row, data->row )
		STOREATTR( MA_Layout_Cell_Rowspan, data->rowspan )
		STOREATTR( MA_Layout_Cell_Colspan, data->colspan )
		STOREATTR( MA_Layout_Width, data->widthspec )
		STOREATTR( MA_Layout_Height, data->heightspec )
		STOREATTR( MA_Layout_Cell_Border, data->border )
		STOREATTR( MA_Layout_MarginLeft, data->innerleft )
		STOREATTR( MA_Layout_MarginRight, data->innerright )
		STOREATTR( MA_Layout_MarginTop, data->innertop )
		STOREATTR( MA_Layout_MarginBottom, data->innerbottom )
		STOREATTR( MA_Layout_LineAlign, data->linealign )
		STOREATTR( MA_Layout_Div_ClearMargin_Left, 0 )
		STOREATTR( MA_Layout_Div_ClearMargin_Right, 0 )
		STOREATTR( MA_Layout_Image_Anchor, NULL )
		STOREATTR( MA_Layout_Image_AnchorURL, NULL )
		STOREATTR( MA_Layout_Group_UseMUIBackground, data->muibackground )
		STOREATTR( MA_Layout_BGImageURL, data->bgimage )
		STOREATTR( MA_Layout_BGImageURLName, data->bgimage_name )
		STOREATTR( MA_Layout_Group_IsEmpty, !data->is_not_empty )

		case MA_Layout_TempLineAlign:
			if( data->templinealign >= 0 )
			{
				*msg->opg_Storage = data->templinealign;
				return( TRUE );
			}
			else
			{
				return( FALSE );
			}
			break;
	}

	return( DOSUPER );
}

DECSET
{
	GETDATA;
	int sendup = FALSE;

	if( doset( data, obj, msg->ops_AttrList, &sendup ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	if( sendup )
		return( DOSUPER );
	else
		return( 0 );
}

DECSMETHOD( Layout_Group_AddText )
{
	GETDATA;
	struct tn *tn;

	if ( tn = AllocPooled( data->pool, sizeof( *tn ) + msg->textlen ) )
	{
		memcpy( tn->text, msg->text, msg->textlen );
		tn->textlen = msg->textlen;
		tn->font = msg->font;
		tn->fontarray = msg->fontarray;
		tn->penspec = msg->penspec;
		tn->style = msg->style;
		tn->anchor = msg->owning_anchor;

		ADDTAIL( &data->l, tn );

		data->is_not_empty = TRUE;

		DoMethod( obj, MUIM_Group_InitChange );
		data->changed = 1;
		DoMethod( obj, MUIM_Group_ExitChange2, TRUE );
		
		return( TRUE );
	}
	else
	{
		DB( ( "not enough memory to addtext\n" ) );
		return( FALSE );
	}
}

DECSMETHOD( Layout_Group_AddObject )
{
	GETDATA;
	struct tn *tn;

	if( !msg->obj )
	{
		reporterror( "Layout_Group_AddObject(NULL) called" );
		return( 0 );
	}

	if ( tn = AllocPooled( data->pool, sizeof( *tn ) ) )
	{
		tn->textlen = 0;
		tn->o = msg->obj;

		get( tn->o, MA_Layout_Info, &tn->li );
		tn->li->flags |= LOF_NEW;

		ADDTAIL( &data->l, tn );

		data->changed = 1;

		data->has_no_childs_with_backfilling = FALSE;

		data->is_not_empty = TRUE;

		DoMethod( obj, MUIM_Group_InitChange );
		DoMethod( obj, OM_ADDMEMBER, ( ULONG )tn->o );
		DoMethod( obj, MUIM_Group_ExitChange );

		return( TRUE );
	}
	else
	{
		DB( ( "not enough memory to addobj\n" ) );
		return( FALSE );
	}
}

#ifndef MBX

#ifndef MBX
#define RECTSIZEX(r) ((r)->MaxX - (r)->MinX + 1)
#define RECTSIZEY(r) ((r)->MaxY - (r)->MinY + 1)
#else
#define RECTSIZEX(r) ((r)->Max.X - (r)->Min.X + 1)
#define RECTSIZEY(r) ((r)->Max.Y - (r)->Min.Y + 1)
#endif

static void STDARGS CopyTiledBitMap(struct BitMap *Src,WORD SrcOffsetX,WORD SrcOffsetY,WORD SrcSizeX,WORD SrcSizeY,struct BitMap *Dst,struct Rectangle *DstBounds)
{
	WORD FirstSizeX;  // the width of the rectangle to blit as the first column
	WORD FirstSizeY;  // the height of the rectangle to blit as the first row
	WORD SecondMinX;  // the left edge of the second column
	WORD SecondMinY;  // the top edge of the second column
	WORD SecondSizeX; // the width of the second column
	WORD SecondSizeY; // the height of the second column
	WORD Pos;         // used as starting position in the "exponential" blit
	WORD Size;        // used as bitmap size in the "exponential" blit

	FirstSizeX = min(SrcSizeX-SrcOffsetX,RECTSIZEX(DstBounds)); // the width of the first tile, this is either the rest of the tile right to SrcOffsetX or the width of the dest rect, if the rect is narrow
	SecondMinX = DstBounds->MinX+FirstSizeX; // the start for the second tile (if used)
	SecondSizeX = min(SrcOffsetX,DstBounds->MaxX-SecondMinX+1); // the width of the second tile (we want the whole tile to be SrcSizeX pixels wide, if we use SrcSizeX-SrcOffsetX pixels for the left part we'll use SrcOffsetX for the right part)

	FirstSizeY = min(SrcSizeY-SrcOffsetY,RECTSIZEY(DstBounds)); // the same values are calculated for y direction
	SecondMinY = DstBounds->MinY+FirstSizeY;
	SecondSizeY = min(SrcOffsetY,DstBounds->MaxY-SecondMinY+1);

	BltBitMap(Src,SrcOffsetX,SrcOffsetY,Dst,DstBounds->MinX,DstBounds->MinY,FirstSizeX,FirstSizeY,0xC0,-1,NULL); // blit the first piece of the tile

	if (SecondSizeX>0) // if SrcOffset was 0 or the dest rect was to narrow, we won't need a second column
		BltBitMap(Src,0,SrcOffsetY,Dst,SecondMinX,DstBounds->MinY,SecondSizeX,FirstSizeY,0xC0,-1,NULL);
	if (SecondSizeY>0) // is a second row necessary?
	{
		BltBitMap(Src,SrcOffsetX,0,Dst,DstBounds->MinX,SecondMinY,FirstSizeX,SecondSizeY,0xC0,-1,NULL);
		if (SecondSizeX>0)
			BltBitMap(Src,0,0,Dst,SecondMinX,SecondMinY,SecondSizeX,SecondSizeY,0xC0,-1,NULL);
	}

	// this loop generates the first row of the tiles
	for (Pos = DstBounds->MinX+SrcSizeX,Size = min(SrcSizeX,DstBounds->MaxX-Pos+1);Pos<=DstBounds->MaxX;)
	{
		BltBitMap(Dst,DstBounds->MinX,DstBounds->MinY,Dst,Pos,DstBounds->MinY,Size,min(SrcSizeY,RECTSIZEY(DstBounds)),0xC0,-1,NULL);
		Pos += Size;
		Size = min(Size<<1,DstBounds->MaxX-Pos+1);
	}

	// this loop blit the first row down several times to fill the whole dest rect
	for (Pos = DstBounds->MinY+SrcSizeY,Size = min(SrcSizeY,DstBounds->MaxY-Pos+1);Pos<=DstBounds->MaxY;)
	{
		BltBitMap(Dst,DstBounds->MinX,DstBounds->MinY,Dst,DstBounds->MinX,Pos,RECTSIZEX(DstBounds),Size,0xC0,-1,NULL);
		Pos += Size;
		Size = min(Size<<1,DstBounds->MaxY-Pos+1);
	}
}

struct mybfhook {
	struct Hook h;
	int xo, yo;
	struct BitMap *bm;
	struct BitMap *maskbm;
	int bmx, bmy;
	struct RastPort drp;
};

struct VBackFillMsg
{
	struct Layer    *Layer;
	struct Rectangle Bounds;
	LONG             OffsetX;
	LONG             OffsetY;
};

#ifndef ABC
/* definitions for blitter control register 0 */
#define ABC    0x80
#define ABNC   0x40
#define ANBC   0x20
#define ANBNC  0x10
#define NABC   0x8
#define NABNC  0x4
#define NANBC  0x2
#define NANBNC 0x1
#endif

#ifdef __MORPHOS__
static LONG ASM SAVEDS bffunc_GATE( void );
static LONG bffunc_GATE2( struct mybfhook *h, struct RastPort *rp, struct VBackFillMsg *bfm );
struct EmulLibEntry bffunc = {
	TRAP_LIB, 0, ( void( * )( void ) )bffunc_GATE
};
static LONG ASM SAVEDS bffunc_GATE( void )
{
	return( bffunc_GATE2( ( void * )REG_A0, ( void * )REG_A2, ( void * )REG_A1 ) );
}
static struct Hook bffunc_hook = { 0, 0, ( void * )&bffunc };
static LONG bffunc_GATE2( struct mybfhook *h, struct RastPort *rp, struct VBackFillMsg *bfm )
#else
static void SAVEDS ASM bffunc( __reg( a0, struct mybfhook *h ), __reg( a2, struct RastPort *rp ), __reg( a1, struct VBackFillMsg *bfm ) )
#endif /* !__MORPHOS__ */
{
	int so_x, so_y;

	// Start offsets
	so_x = ( h->xo + bfm->OffsetX ) % h->bmx;
	so_y = ( h->yo + bfm->OffsetY ) % h->bmy;

#if USE_ALPHA
	if ( h->maskbm == ( APTR )-1 )
	{
		/*
		 * Alpha PNGs
		 */
		int xs, ys;
		int o_x = so_x, o_y;

		//DB( ( "mask == -1\n" ) );

		for( xs = bfm->Bounds.MinX; xs <= bfm->Bounds.MaxX; )
		{
			int thisxsize;

			thisxsize = min( h->bmx - o_x, bfm->Bounds.MaxX - xs +1 );

			o_y = so_y;
			for( ys = bfm->Bounds.MinY; ys <= bfm->Bounds.MaxY; )
			{
				int thisysize;

				thisysize = min( h->bmy - o_y, bfm->Bounds.MaxY - ys + 1 );

				WritePixelArrayAlpha( h->bm->Planes[ 0 ], o_x, o_y, GetCyberMapAttr( h->bm, CYBRMATTR_XMOD ),
					rp, xs, ys,
					thisxsize, thisysize,
					0xffffffff
				);
				ys += thisysize;
				o_y = 0;
			}

			xs += thisxsize;
			o_x = 0;
		}

	}
	else if ( h->maskbm != 0 )
#else
	if( h->maskbm )
#endif /* USE_ALPHA */
	{
		int xs, ys;
		int o_x = so_x, o_y;

		//DB( ( "mask > 0\n" ) );

		for( xs = bfm->Bounds.MinX; xs <= bfm->Bounds.MaxX; )
		{
			int thisxsize;

			thisxsize = min( h->bmx - o_x, bfm->Bounds.MaxX - xs +1 );

			o_y = so_y;
			for( ys = bfm->Bounds.MinY; ys <= bfm->Bounds.MaxY; )
			{
				int thisysize;

				thisysize = min( h->bmy - o_y, bfm->Bounds.MaxY - ys + 1 );

				h->drp.BitMap = rp->BitMap;
				BltMaskBitMapRastPort( h->bm, o_x, o_y,
					&h->drp, xs, ys,
					thisxsize, thisysize,
					(ABC|ABNC|ANBC), h->maskbm->Planes[ 0 ]
				);

				ys += thisysize;
				o_y = 0;
			}

			xs += thisxsize;
			o_x = 0;
		}
	}
	else
	{
		CopyTiledBitMap(
			h->bm,
			so_x,
			so_y,
			h->bmx,
			h->bmy,
			rp->rp_BitMap,
			&bfm->Bounds
		);
	}
}

#endif // !MBX

/*
DECMMETHOD( Setup )
{
	GETDATA;
	ULONG rc;

	rc = DOSUPER;

	return( rc );
}
*/

/*
	The contents of this group are a stream of text, and either
	floating or inlined objects. We need to iterate the list, and
	calculate the minimum and maximum widths of this object, and
	some suggested default width.

*/

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	struct tn *tn;
	int minwidth = 0;
	int lastwidth = 0;
	int totalwidth = 0;
	int maxtotalwidth = 0;
	int minheight = 0;
	int global_totalwidth = 0, global_maxtotalwidth = 0;
	int margin_left = 0;
	int margin_right = 0;

	DL( DEBUG_CHATTY, db_html, bug( "in calcminmax(%lx), sw=%ld, sh=%ld, frame %ld %ld %ld %ld\n", obj, msg->suggested_width, msg->suggested_height, data->innerleft, data->innertop, data->innerright, data->innerbottom ));

	// We eat up any spaces at the end
	if( !data->did_final_cleanup )
	{
		tn = LASTNODE( &data->l );
		if( PREVNODE( tn ) )
		{
			if( tn->textlen )
			{
				char *p = tn->text + tn->textlen - 1;

				while( p >= (char*)tn->text && isspace( *p ) )
				{
					*p-- = 0;
					tn->textlen--;
				}
				if( !tn->textlen )
					REMOVE( tn ); // Node no longer used
			}
		}

		data->did_final_cleanup = TRUE;
	}

	// All the text in this cell is supposed to be a single stream
	// so we find the minimum word length
	for( tn = FIRSTNODE( &data->l ); NEXTNODE( tn ); tn = NEXTNODE( tn ) )
	{
		char *p, *p2, *p3;
		int thiswidth;
		int spacewidth = -1;

		if( !tn->textlen )
		{
			struct layout_info *li = (APTR)DoMethod( tn->o, MM_Layout_CalcMinMax, msg->suggested_width - data->innerleft - data->innerright, msg->suggested_height - data->innerbottom - data->innertop, msg->window_width - data->innerbottom - data->innertop );

			minheight = max( minheight, li->minheight );

			// We can only add this to totalwidth if the object is inlined
			if( li->align == align_inline )
			{
				totalwidth += li->minwidth + margin_left + margin_right;
				maxtotalwidth += li->defwidth - li->minwidth;
				minwidth = max( minwidth, li->minwidth + lastwidth );
				lastwidth += li->minwidth;
			}
			else
			{
				// We need to cut off maximal widths here!!!
				global_totalwidth = max( totalwidth, global_totalwidth );
				global_maxtotalwidth = max( maxtotalwidth, global_maxtotalwidth );

				totalwidth = li->minwidth + margin_left + margin_right ;
				maxtotalwidth = li->defwidth - li->minwidth;
				minwidth = max( minwidth, li->minwidth + margin_left + margin_right );

				if( li->align == align_newrowrow )
				{
					global_totalwidth = max( totalwidth, global_totalwidth );
					global_maxtotalwidth = max( maxtotalwidth, global_maxtotalwidth );
					totalwidth = maxtotalwidth = 0;
					lastwidth = 0;
				}
				else if( li->align == align_newrow || li->align == align_newrowafter )
				{
					lastwidth = 0;
				}

				if( li->align == align_left )
					margin_left = li->minwidth;
				else if( li->align == align_right )
					margin_right = li->minwidth;
				else
				{
					margin_left = margin_right = 0;
				}
			}

			//if( li->xs > 0 )
			//	lastwidth = 0;

			// Inline object
			continue;
		}

		p = tn->text;
		p3 = p + tn->textlen;

		while( p < p3 )
		{
			p2 = p;

			if( tn->style & FSF_PRE )
			{
				p2 = p3;
			}
			else
			{
				while( p2 < p3 && !isspace( *p2 ) )
					p2++;
			}

			if( p == p2 )
			{
				// this was the space
				if( spacewidth < 0 )
				{
#ifdef MBX
					spacewidth = mbxtextlen( " ", 1, tn->font );
#else
					spacewidth = patextlen( " ", tn->fontarray, 1, tn->style );
#endif
				}
				thiswidth = spacewidth;

				p2++;
				lastwidth = 0;
			}
			else
			{
				// p->p2 now contains a word
#ifdef MBX
				thiswidth = mbxtextlen( p, p2-p, tn->font );
#else
				thiswidth = patextlen( p, tn->fontarray, p2 - p, tn->style );
#endif
			}

			minwidth = max( thiswidth + lastwidth, minwidth );
			minheight = max( minheight, tn->font->tf_YSize );

			totalwidth += thiswidth;
			lastwidth += thiswidth;

			p = p2;
		}
	}

	global_totalwidth = max( totalwidth, global_totalwidth );
	global_maxtotalwidth = max( maxtotalwidth, global_maxtotalwidth );

	minwidth += data->innerleft + data->innerright;
	minheight += data->innertop + data->innerbottom;

	data->li.minwidth = minwidth;
	data->li.maxwidth = global_totalwidth + data->innerleft + data->innerright + global_maxtotalwidth;
	data->li.defwidth = data->li.maxwidth;

	// Bogus, mostly
	data->li.minheight = minheight;
	data->li.defheight = msg->suggested_height;
	data->li.maxheight = msg->suggested_height;

	DL( DEBUG_CHATTY, db_html, bug( "finished calcminmax(%lx) -> %ld,%ld %ld,%ld %ld,%ld\n", obj, data->li.minwidth, data->li.minheight, data->li.defwidth, data->li.defheight, data->li.maxwidth, data->li.maxheight ));

	return( (ULONG)&data->li );
}

//
// This function "fixes" a line
//
// It aligns all inline objects (both objects and text)
// by the baseline (or whatever the alignment is)
// and returns the actual maximal line height
//

static int fixline( struct Data *data, struct tnr *linestart, int restwidth, int linealign, int innerrestwidth )
{
	int space_over_baseline = 0;
	int space_under_baseline = 0;
	int text_over_baseline = 0;
	int text_under_baseline = 0;
	struct tnr *tnr = linestart, *ntnr;
	int addx;

	if( !tnr || !NEXTNODE( tnr ) )
		return( 0 );

	if( linealign == linealign_right )
		addx = innerrestwidth;
	else if( linealign == linealign_center )
		addx = innerrestwidth / 2;
	else
		addx = 0;

	//Printf( "fixline - rest %ld, innerrest %ld, addx %ld\n", restwidth, innerrestwidth, addx );

	if( addx < 0 )
		addx = 0;

	// First, calculate space around baseline, text only
	while( ntnr = NEXTNODE( tnr ) )
	{
		if( tnr->textlen )
		{
			// Text node
			text_over_baseline = max( text_over_baseline, tnr->tn->font->tf_Baseline + 1 );
			text_under_baseline = max( text_under_baseline, tnr->tn->font->tf_YSize - tnr->tn->font->tf_Baseline -1 );
		}

		tnr = ntnr;
	}

	space_over_baseline = text_over_baseline;
	space_under_baseline = text_under_baseline;

	// Second pass, do text-relative objects
	tnr = linestart;
	while( ntnr = NEXTNODE( tnr ) )
	{
		if( !tnr->textlen )
		{
			// Inline object
			switch( tnr->tn->li->valign )
			{
				case valign_middle:
					// Center around mid of text (eek)
					{
						int totalfont = text_over_baseline + text_under_baseline;
						int base_diff = text_over_baseline - totalfont / 2;
						int ext_over = tnr->tn->li->ys / 2;
						int ext_under = ( tnr->tn->li->ys + 1 ) / 2;
						int have_over = text_over_baseline - base_diff;
						int have_under = text_under_baseline + base_diff;

						space_over_baseline = max( space_over_baseline, text_over_baseline + ( ext_over - have_over ) );
						space_under_baseline = max( space_under_baseline, text_under_baseline + ( ext_under - have_under ) );
					}
					break;

				case valign_texttop:
					// Extendens under_baseline
					space_under_baseline = max( space_under_baseline, tnr->tn->li->ys - text_over_baseline );
					break;

				case valign_bottom:
				case valign_baseline:
					// Baseline
					// In this case, object space goes over baseline
					space_over_baseline = max( space_over_baseline, tnr->tn->li->ys );
					break;

				case valign_special:
					// No size
					break;
			}
		}

		tnr = ntnr;
	}

	// Third pass, do line-relative objects
	tnr = linestart;
	while( ntnr = NEXTNODE( tnr ) )
	{
		if( !tnr->textlen )
		{
			// Inline object
			switch( tnr->tn->li->valign )
			{
				case valign_absmiddle:
					if( space_over_baseline + space_under_baseline < tnr->tn->li->ys )
					{
						int rest = tnr->tn->li->ys - ( space_over_baseline + space_under_baseline );
						int y1 = rest / 2;
						int y2 = ( rest + 1 ) / 2;

						space_over_baseline += y1;
						space_under_baseline += y2;
					}
					break;

				case valign_absbottom:
					if( space_over_baseline + space_under_baseline < tnr->tn->li->ys )
						space_over_baseline += tnr->tn->li->ys - ( space_over_baseline + space_under_baseline );
					break;

				case valign_top:
					if( space_over_baseline + space_under_baseline < tnr->tn->li->ys )
						space_under_baseline += tnr->tn->li->ys - ( space_over_baseline + space_under_baseline );
					break;

				case valign_special:
					// No size
					break;

				default:
					break;
			}
		}

		tnr = ntnr;
	}

	// Now correct YP so that the baselines match

	tnr = linestart;
	while( ntnr = NEXTNODE( tnr ) )
	{
		if( tnr->textlen )
		{
			// Text node
			tnr->yp += space_over_baseline - 1;
			tnr->xp += addx;
		}
		else
		{
			// Inline object
			switch( tnr->tn->li->valign )
			{
				case valign_absbottom:
					tnr->yp += ( space_over_baseline + space_under_baseline - tnr->tn->li->ys );
					break;

				case valign_absmiddle:
				case valign_middle:
					tnr->yp += ( space_over_baseline + space_under_baseline - tnr->tn->li->ys ) / 2;
					break;

				case valign_top:
					break;

				case valign_texttop:
					tnr->yp += ( space_over_baseline - text_over_baseline );
					break;

				case valign_special:
					// No size
					break;

				default: // Bottom/Baseline...
					tnr->yp += ( space_over_baseline - tnr->tn->li->ys );
					break;
			}

			if( tnr->tn->li->align != align_left && tnr->tn->li->align != align_right )
				tnr->tn->li->xp = tnr->xp + addx;
			else
				tnr->tn->li->xp = tnr->xp;
			tnr->tn->li->yp = tnr->yp;

			REMOVE( tnr );
			FreePooled( data->pool, tnr, sizeof( *tnr ) );
		}

		tnr = ntnr;
	}

	return( space_over_baseline + space_under_baseline );
}

struct marginnode {
	struct MinNode n;
	int width;	// Width of indention
	int stopat;	// Vertial end position
};

static int unrollmargin( struct MinList *marginlist, int yoffs, int endmode )
{
	int total = 0;
	struct marginnode *mn, *nn;
	int gotone = FALSE;

	for( mn = FIRSTNODE( marginlist ); nn = NEXTNODE( mn ); mn = nn )
	{
		if( ( yoffs >= mn->stopat && !gotone ) || ( endmode && mn->stopat == INT_MAX ) )
		{
			REMOVE( mn );
			free( mn );
		}
		else
		{
			total += mn->width;
			gotone = TRUE;
		}
	}

	return( total );
}

#define LINEBREAK \
	maxx = max( maxx, xoffs ); \
	yoffs += fixline( data, linestart, restwidth, linealign, restwidth + ( msg->outer_width - msg->suggested_width ) ); \
	linestart = NULL; \
	xoffs = marginwidth_left = unrollmargin( &marginfloat_left, yoffs, FALSE ) + data->innerleft; \
	restwidth = msg->suggested_width - xoffs - unrollmargin( &marginfloat_right, yoffs, FALSE ) - data->innerright; \

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	struct tn *tn;
	struct tnr *tnr, *linestart = NULL;
	int yoffs = data->innertop, xoffs = data->innerleft;
	int restwidth;
	int maxx = 0;
	struct MinList marginfloat_left, marginfloat_right;
#ifdef VDEBUG
	clock_t ts = 0;
#endif
	struct marginnode *left_margin = NULL, *right_margin = NULL;
	int linealign = data->default_linealign, newalign;
	int lastfit = 0;
	int marginwidth_left = xoffs;

	data->li.flags &= ~LOF_NEW;

	NEWLIST( &marginfloat_left );
	NEWLIST( &marginfloat_right );

	DL( DEBUG_CHATTY, db_html, bug( "in dolayout(%lx), destination width %ld, frames %ld %ld %ld %ld\n", obj, msg->suggested_width, data->innerleft, data->innertop, data->innerright, data->innerbottom, ts = clock() ) );

	if( msg->suggested_width < data->li.minwidth )
	{
		MUI_Request( app, _win( obj ), 0, "Error", "Skip cell", "Internal error:\nLayout width %ld, min %ld for object %lx", msg->suggested_width, data->li.minwidth, ( ULONG )obj );
		return( (ULONG)&data->li );
	}

	// Flush any existing text nodes
	while( tnr = REMHEAD( &data->v ) )
	{
		FreePooled( data->pool, tnr, sizeof( *tnr ) );
	}
	data->last_tnr=NULL;

	restwidth = msg->suggested_width - xoffs - data->innerright;

	for( tn = FIRSTNODE( &data->l ); NEXTNODE( tn ); tn = NEXTNODE( tn ) )
	{
		int thisfit;
		int remainlength = tn->textlen;
		char *txt = tn->text;
		int oldalign, clear;

		// Inline object
		if( !remainlength )
		{
			switch( tn->li->align )
			{
				case align_newrow:
				case align_newrowrow:
					// Object always in new row, no matter what
					LINEBREAK;
					if( get( tn->o, MA_Layout_LineAlign, &newalign ) && newalign != -1 )
						linealign = newalign;
					// Fallthrough
				case align_newrowafter:
					if( get( tn->o, MA_Layout_Div_ClearMargin_Left, &clear ) && clear )
					{
						// Clear any left margins
						while( !ISLISTEMPTY( &marginfloat_left ) )
						{
							yoffs += 1;
							unrollmargin( &marginfloat_left, yoffs, FALSE );
						}
					}
					if( get( tn->o, MA_Layout_Div_ClearMargin_Right, &clear ) && clear )
					{
						// Clear any right margins
						while( !ISLISTEMPTY( &marginfloat_right ) )
						{
							yoffs += 1;
							unrollmargin( &marginfloat_right, yoffs, FALSE );
						}
					}
					break;

				case align_left:
				case align_right:
					// Start new row
					LINEBREAK;
					break;

				default:
					// Inline...
					break;
			}

			if( tn->li->minwidth > restwidth )
			{
				LINEBREAK;
				while( tn->li->minwidth > restwidth )
				{
					yoffs++;
					LINEBREAK;
				}
			}


			// Handle object...
			tnr = AllocPooled( data->pool, sizeof( *tnr ) );

			tnr->xp = xoffs;
			tnr->yp = yoffs;
			tnr->textlen = 0;
			tnr->tn = tn;
			tnr->anchorselected = 0;
			tnr->anchoractive = 0;

			DoMethod( tn->o, MM_Layout_DoLayout, restwidth, msg->suggested_height - data->innertop - data->innerbottom, msg->outer_width - data->innerleft - data->innerright );

			ADDTAIL( &data->v, tnr );

			if( !linestart )
				linestart = tnr;

			switch( tn->li->align )
			{
				case align_newrowafter:
				case align_newrowrow:
					xoffs += tn->li->xs;
					restwidth -= tn->li->xs;
					// New row after object
					oldalign = linealign;
					get( tn->o, MA_Layout_TempLineAlign, &linealign );
					LINEBREAK;
					linealign = oldalign;
					break;

				default:
					xoffs += tn->li->xs;
					restwidth -= tn->li->xs;
					break;

				case align_left:
					left_margin = malloc( sizeof( *left_margin ) );
					memset( left_margin, '\0', sizeof( *left_margin ) ); /* TOFIX: maybe not needed + missing NULL check */
					ADDHEAD( &marginfloat_left, left_margin );
					left_margin->width = tn->li->xs;
					left_margin->stopat = yoffs + tn->li->ys;
					get( tn->o, MA_Layout_MarginEnd_Left, &left_margin->stopat );
					LINEBREAK;
					break;

				case align_right:
					{
						int innerrestwidth;

						innerrestwidth = restwidth + ( msg->outer_width - msg->suggested_width );
						innerrestwidth = max( innerrestwidth, tn->li->xs );

						tnr->xp += ( innerrestwidth - tn->li->xs );
						right_margin = malloc( sizeof( *right_margin ) );
						memset( right_margin, '\0', sizeof( *right_margin ) ); /* TOFIX: maybe not needed + missing NULL check */
						ADDHEAD( &marginfloat_right, right_margin );
						right_margin->width = tn->li->xs;
						right_margin->stopat = yoffs + tn->li->ys;
						get( tn->o, MA_Layout_MarginEnd_Right, &right_margin->stopat );
						LINEBREAK;
					}
					break;
			}

			continue;
		}

// !!! FIXME adapt the whole wordwrap loop (txt++ or txt-=tryfit etc for UTF8)
		while( remainlength > 0 )
		{
			int thislength;
			int tryfit;
			int didwrap = FALSE;

#ifndef MBX
			thisfit = patextfit( txt, tn->fontarray, remainlength, restwidth, tn->style );
#else
			thisfit = mbxtextfit( txt, remainlength, restwidth, tn->font );
#endif
			if( tn->style & FSF_PRE && thisfit < remainlength )
			{
				// If this is a pre-formatted segment, and we don't have
				// enough space to fit it completely, we need to wrap,
				// and retry
				LINEBREAK;

				if( !lastfit )
				{
					yoffs++;
				}
				lastfit = 0;
				continue;
			}

			if( thisfit < 0 )
			{
				char tmp[ 256 ];

				// Dump some sensible error message, then retry
				strncpy( tmp, txt, min( 256, remainlength + 1 ) );
				ALERT( ( "ERROR!!! textfit() returned %ld, remainlength=%ld, restwidth=%ld, font=%lx, txt='%s'\n", thisfit, remainlength, restwidth, tn->font, tmp ) );
				txt++;
				remainlength--;
				lastfit = 0;
				continue;
			}

			if( !thisfit )
			{
				LINEBREAK;
				while( remainlength && isspace( *txt ) )
					txt++, remainlength--;

				if( !lastfit )
				{
					yoffs++;
				}

				lastfit = 0;

				continue;
			}

			lastfit = thisfit;

			// Did we not fix everything into the line?
			if( thisfit < remainlength )
			{
				tryfit = thisfit;

				// Go back till there is a space
				while( tryfit-- && !isspace( txt[ tryfit ] ) );

				if( tryfit > 0 )
				{
					thisfit = tryfit;
					didwrap = TRUE;
				}
				else if( xoffs > marginwidth_left )
				{
					// If we're not at line start, wrap the complete word
					// CHECKME -- do we deal with &nbsp; properly here?
					while( remainlength && isspace( *txt ) )
						txt++, remainlength--;

					LINEBREAK;
					continue;
				}
			}

			// !!! FIXME NULLCHECK!
			tnr = AllocPooled( data->pool, sizeof( *tnr ) );

			tnr->xp = xoffs;
			tnr->yp = yoffs;
			tnr->pen = LO_NOPEN;
			tnr->tn = tn;
			tnr->text = txt;
			tnr->textlen = thisfit;
			tnr->redrawme = FALSE;
			tnr->anchorselected = 0;
			tnr->anchoractive = 0;

			ADDTAIL( &data->v, tnr );

			remainlength -= thisfit;
#ifndef MBX
			thislength = patextlen( txt, tn->fontarray, thisfit, tn->style );
#else
			thislength = mbxtextlen( txt, thisfit, tn->font );
#endif
			tnr->pixellen = thislength;
			restwidth -= thislength;
			xoffs += thislength;
			txt += thisfit;

			if( !linestart )
				linestart = tnr;

			if( didwrap )
			{
				// We wrapped -- skip the space
				remainlength--;
				txt++;
				LINEBREAK;
			}
		}
	}

	yoffs += fixline( data, linestart, restwidth, linealign, restwidth + ( msg->outer_width - msg->suggested_width ) );

	// Unroll any remaining margins...
	while( !ISLISTEMPTY( &marginfloat_left ) )
	{
		yoffs += 1;
		unrollmargin( &marginfloat_left, yoffs, TRUE );
	}
	while( !ISLISTEMPTY( &marginfloat_right ) )
	{
		yoffs += 1;
		unrollmargin( &marginfloat_right, yoffs, TRUE );
	}

	yoffs += data->innerbottom;

	data->li.xs = max( maxx + data->innerright, msg->suggested_width );
	data->li.ys = yoffs;

	DL( DEBUG_CHATTY, db_html, bug( "finished dolayout(%lx) -> %ld,%ld, took %ld ticks\n", obj, data->li.xs, data->li.ys, clock() - ts ));

	return( (ULONG)&data->li );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	data->bgcolor_pen = LO_NOPEN;

	// Enforce updating of status bar
	data->last_anchor = NULL;

	return( DOSUPER );
}

DECMETHOD( Layout_CheckLayout, ULONG* )
{
	GETDATA;

	if( data->changed )
	{
		*msg[ 1 ] = TRUE;

		return( TRUE );
	}
	return( DOSUPER );
}

#ifndef MBX
#define USE_REGION_BACKFILL
#endif

#ifndef MBX
extern BOOL ASM _IsRectangleVisibleInLayer(__reg( a0, struct Layer *l ), __reg( d0, WORD x0 ), __reg( d1, WORD y0 ), __reg( d2, WORD x1 ), __reg( d3, WORD y1 ));
#define IsRectangleVisibleInLayer _IsRectangleVisibleInLayer
#endif // !MBX

#ifdef MBX
#ifdef __GNUC__
#define isvis(rp,x1,y1,x2,y2) ({ Rectangle_s coords = {{x1,y1},{x2,y2}}; IsRectangleVisible(rp,&coords); })
#else
#define isvis(rp,x1,y1,x2,y2) _isvis(rp,x1,y1,x2,y2)
static BOOL INLINE _isvis(RastPort_p rp,UDWORD x1,UDWORD y1,UDWORD x2,UDWORD y2)
{
	Rectangle_s coords;
	coords.Min.X = x1;
	coords.Min.Y = y1;
	coords.Max.X = x2;
	coords.Max.Y = y2;
	return IsRectangleVisible(rp,&coords);
}
#endif
#else
#define isvis(rp,layer,x1,y1,x2,y2) (!layer || IsRectangleVisibleInLayer(layer,x1,y1,x2,y2))
#endif

DECMMETHOD( Backfill )
{
	GETDATA;
	struct tnr *tnr;
	struct RastPort *rp = _rp( obj );
	int offs_x = _left( obj );
	int offs_y = _vtop( obj ) + data->topoffset;
	ULONG oldpen = -1, oldstyle = -1;
	struct TextFont *oldfont = NULL;
	struct Layer *layer = rp->Layer;
#ifdef MBX
	int fixedbg = FALSE;
#endif
#ifdef USE_REGION_BACKFILL
	struct Region *clipregion = NULL;
	APTR cliphandle = NULL;
#endif

	//DB( ( "doing backfill\n" ) );

	if( data->nobackfill )
	{
		return( 0 );
	}

	/* TOFIX: this is temporary */
	if( OCLASS( obj ) == data->ctx->dom_document )
	{
		msg->left = _mleft( data->ctx->dom_document );
		msg->top = _mtop( data->ctx->dom_document );
		msg->right = _mright( data->ctx->dom_document );
		msg->bottom = _mbottom( data->ctx->dom_document );
	}
	else
	{
		/*
		 * This is the sucky part.. only used for subgroups now.
		 */
		msg->left = _left( obj );
		msg->top = _vtop( obj );
		msg->right = msg->left + data->li.xs - 1;
		msg->bottom = msg->top + data->li.ys - 1;

		msg->left = max( 0, msg->left );
		msg->top = max( 0, msg->top );
		msg->right = min( 4096, msg->right );
		msg->bottom = min( 4096, msg->bottom );
	}

#ifdef USE_REGION_BACKFILL
	if( ( data->bgcolor != LO_NOPEN || data->bgbitmap ) && _window( obj ) && !data->has_no_childs_with_backfilling )
	{
		int region_used = FALSE;

		clipregion = NewRegion();
		if( clipregion )
		{
			struct Rectangle r;

			r.MinX = msg->left;
			r.MinY = msg->top;
			r.MaxX = msg->right;
			r.MaxY = msg->bottom;

			if( OrRectRegion( clipregion, &r ) )
			{
				// Add childs
				struct tn *tn;

				for( tn = FIRSTNODE( &data->l ); NEXTNODE( tn ); tn = NEXTNODE( tn ) )
				{
					if( !tn->textlen && ( tn->li->flags & LOF_BACKFILLING ) && tn->li->xs > 64 && tn->li->ys > 64 )
					{
						r.MinX = offs_x + tn->li->xp;
						r.MinY = offs_y + tn->li->yp;
						r.MaxX = offs_x + tn->li->xp + tn->li->xs - 1;
						r.MaxY = offs_y + tn->li->yp + tn->li->ys - 1;
						ClearRectRegion( clipregion, &r );
						region_used = TRUE;
					}
				}
			}

			if( region_used )
			{
				cliphandle = MUI_AddClipRegion( muiRenderInfo( obj ), clipregion );
			}
			else
			{
				DisposeRegion( clipregion );
				clipregion = NULL;
				data->has_no_childs_with_backfilling = TRUE;
			}
		}
	}
#endif
	
	if( data->muibackground )
	{
		DoMethod( obj, MUIM_DrawBackground, msg->left, msg->top, msg->right - msg->left + 1, msg->bottom - msg->top + 1, msg->left, msg->right, 0 );
#ifdef MBX
		fixedbg = FALSE;
#endif
	}
	else if( data->bgcolor != LO_NOPEN && data->bgcolor != LO_GENLOCK && ( !data->bgbitmap || data->bgbitmap_has_mask ) )
	{
		if( data->bgcolor_pen == LO_NOPEN )
			data->bgcolor_pen = layout_getpen( data->ctx, data->bgcolor );
		SetAPen( rp, data->bgcolor_pen );
#ifdef MBX
		SetBPen( rp, data->bgcolor_pen );
		fixedbg = TRUE;
#endif
		if( msg->left <= msg->right && msg->top <= msg->bottom )
			RectFill( rp, msg->left, msg->top, msg->right, msg->bottom );
	}
#ifdef MBX
	else if( data->bgcolor == LO_GENLOCK )
	{
		if( msg->left <= msg->right && msg->top <= msg->bottom )
		{
			SetAPen( rp, 0xffffff);
			SetBPen( rp, 0xffffff);
			fixedbg = TRUE;
			RectFill( rp, msg->left, msg->top, msg->right, msg->bottom );
			SetAlphaChannel( rp, msg->left, msg->top, msg->right - msg->left + 1, msg->bottom - msg->top + 1, 0x00 );
		}
	}
#endif
	if( data->bgbitmap )
	{
#ifndef MBX
		struct mybfhook mbfh;
		struct Rectangle rect;

		rect.MinX = msg->left;
		rect.MinY = msg->top;
		rect.MaxX = msg->right;
		rect.MaxY = msg->bottom;

#ifdef __MORPHOS__
		mbfh.h.h_Entry = ( void * )&bffunc;
#else
		mbfh.h.h_Entry = (HOOKFUNC)bffunc;
#endif
		//mbfh.xo = -msg->left;
		//mbfh.yo = -msg->top;
		mbfh.xo = - _left( obj );
		mbfh.yo = - _vtop( obj );

		mbfh.bm = getclone( data->bgbitmap, ( data->bgmask > 0 && data->bgmask != ( APTR )-1 ) ? TRUE : FALSE );

		//DB( ( "data->bgmask: 0x%lx\n", data->bgmask ) );

		if( data->bgmask > 0 && data->bgmask != ( APTR )-1 )
			mbfh.maskbm = getclone( data->bgmask, TRUE );
		else
			mbfh.maskbm = data->bgmask;

		mbfh.bmx = data->bgbitmap_xs;
		mbfh.bmy = data->bgbitmap_ys;

		InitRastPort( &mbfh.drp );

		DoHookClipRects( &mbfh, rp, &rect );
#else
		BltBitMapRastPortTiled( data->bgbitmap, 0, 0, data->bgbitmap_xs, data->bgbitmap_ys,
			msg->left - _mleft(obj), msg->top - _mtop(obj),
			_rp( obj ),
			msg->left, msg->top,
			msg->right - msg->left + 1, msg->bottom - msg->top + 1
		);
		fixedbg = FALSE;
#endif
	}

#ifdef USE_REGION_BACKFILL
	if( clipregion )
	{
		MUI_RemoveClipRegion( muiRenderInfo( obj ), cliphandle );
	}
#endif

	// Draw texts. Text is guaranteed to be sorted in top-bottom order,
	// so we can abort rendering if we exceed our bottom

	#if USE_FAST_LISTWALK
	if( data->last_tnr ) /* XXX: kill it if that's a new page !! */
	{
		int ty;

		/*
		 * Find which direction we have to go.
		 */
		tnr = data->last_tnr;

		while( offs_y + tnr->yp + 30 > msg->top )
		{
			if(tnr)
				tnr=PREVNODE(tnr);
			if(!tnr)
			{
				tnr=FIRSTNODE(&data->v);
				break;
			}
		}
		data->last_tnr = tnr;
	}
	else
	{
		int ty;

		for ( tnr = FIRSTNODE( &data->v ); NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
		{
			ty = offs_y + tnr->yp;

			if ( ty + 10 < msg->top )
				continue;

			data->last_tnr = tnr;
			break;
		}
	}

	for ( tnr = data->last_tnr; tnr && NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
	#else
	for( tnr = FIRSTNODE( &data->v ); tnr && NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
	#endif
	{
		int ty, tx;
		int newstyle, fullstyle;

		// TOFIX!! -- we need some max text extent values
		// stored somewhere

		ty = offs_y + tnr->yp;
		tx = offs_x + tnr->xp;

		if( ty + 10 < msg->top ) /* XXX: remove that once everything works */
		{
			continue;
		}
		else if( ty - 30 > msg->bottom )
		{
			break;
		}

#ifdef MBX
		if( !isvis( rp,
			tx, ty - tnr->tn->font->tf_Baseline,
			tx + tnr->pixellen - 1, ty - tnr->tn->font->tf_Baseline + tnr->tn->font->tf_YSize
		))
#else
		if( !isvis( rp, layer,
			tx, ty - tnr->tn->font->tf_Baseline,
			tx + tnr->pixellen - 1, ty - tnr->tn->font->tf_Baseline + tnr->tn->font->tf_YSize
		))
#endif // !MBX
		{
			continue;
		}

		if( tnr->pen == LO_NOPEN )
			tnr->pen = layout_getpen( data->ctx, tnr->tn->penspec );

		if( oldpen != tnr->pen )
		{
			SetAPen( rp, tnr->pen );
			oldpen = tnr->pen;
		}
		if( oldfont != tnr->tn->font )
		{
			SetFont( rp, tnr->tn->font );
			oldfont = tnr->tn->font;
			oldstyle = -1;
		}
#ifdef MBX
		if( fixedbg )
			oldfont->tf_Flags |= FPF_ALPHA_BGPEN;
		else
			oldfont->tf_Flags &= ~FPF_ALPHA_BGPEN;
#endif

#define REALSTYLEMASK (FSF_BOLD|FSF_UNDERLINED|FSF_ITALIC)

		fullstyle = tnr->tn->style;
		newstyle = fullstyle & REALSTYLEMASK;
		if( oldstyle != newstyle )
		{
			SetSoftStyle( rp, newstyle, REALSTYLEMASK );
			oldstyle = newstyle;
		}

		Move( rp, tx, ty );
#ifdef USE_LIBUNICODE
		mbxText( rp, tnr->text, tnr->textlen );
#else
		Text( rp, tnr->text, tnr->textlen );
#endif

		if( fullstyle & FSF_STRIKE )
		{
			int yo = offs_y + tnr->yp - ( tnr->tn->font->tf_Baseline - 2 ) / 2;

			Move( rp, offs_x + tnr->xp, yo );
			Draw( rp, offs_x + tnr->xp + tnr->pixellen, yo );
		}

		if( tnr->tn->anchor )
		{
			if( gp_underline_links )
			{
				static UWORD areaptr[ ] = { 0xcccc, 0xcccc };
				int linky = offs_y + tnr->yp + 2;

				if( fullstyle & FSF_VISITEDLINK )
				{
					SetAfPt( rp, areaptr, 1 );
					RectFill( rp,
						offs_x + tnr->xp, linky,
						offs_x + tnr->xp + tnr->pixellen - 1, linky
					);
					SetAfPt( rp, NULL, 0 );
				}
				else
				{
					RectFill( rp,
						offs_x + tnr->xp, linky,
						offs_x + tnr->xp + tnr->pixellen - 1, linky
					);
				}
			}
		}
	}

	return( TRUE );
}

DECTMETHOD( ImgDecode_Done )
{
		GETDATA;
		int rc;

		if( data->bgbitmap_free )
		{
			FreeBitMap( data->bgbitmap );
			if( data->bgmask > 0 && data->bgmask != ( APTR )-1 )
			{
				FreeBitMap( data->bgmask );
			}
			data->bgbitmap_free = FALSE;
		}

		rc = imgdec_getinfo( data->bgimage, &data->bgbitmap, &data->bgbitmap_xs, &data->bgbitmap_ys );

		if( data->bgbitmap == (APTR)-1 )
			data->bgbitmap = NULL;

		data->bgmask = imgdec_getmaskbm( data->bgimage );

		//DB( ( "data->bgmask: 0x%lx\n", data->bgmask ) );

		if( rc && data->bgmask > 0 )
		{
			data->bgbitmap_has_mask = 1; /* TOFIX: sigh.. */
		}

		// If the background image is transparent and completely
		// blank, just drop it
		if( data->bgmask && imgdec_isblank( data->bgimage ) )
		{
			imgdec_close( data->bgimage );
			data->bgimage = NULL;
			data->bgbitmap = NULL;
			data->bgmask = NULL;
			return( 0 );
		}
		// If the background mask is not used, drop it
		if( data->bgmask && !imgdec_maskused( data->bgimage ) )
		{
			data->bgmask = NULL;
		}

		// If the background image is transparent and
		// very small, copy it and blow it up, to speed
		// up blitting
		if( data->bgmask && ( data->bgbitmap_xs < 128 || data->bgbitmap_ys < 128 ) )
		{
			int new_xs = data->bgbitmap_xs;
			int new_ys = data->bgbitmap_ys;
			struct BitMap *new_bm;
			struct BitMap *new_mask;
			struct BitMap *src_friend;

			//DB( ( "doing blit-o-mess, data->bgmask: 0x%lx\n", data->bgmask ) );

			while( new_xs < 128 )
				new_xs *= 2;
			while( new_ys < 128 )
				new_ys *= 2;

#if USE_ALPHA
			if( data->bgmask == ( APTR )-1 )
			{
				/*
				 * PNG alphas
				 */
				//DB( ( "doing PNG alpha..\n" ) );
				new_bm = AllocBitMap(
					new_xs,
					new_ys,
					32,
					BMF_MINPLANES | BMF_SPECIALFMT | SHIFT_PIXFMT( PIXFMT_ARGB32 ),
					NULL
				);
			}
			else
#endif /* USE_ALPHA */
			{
				//DB( ( "normal blitting\n" ) );
				if( destscreen )
				{
					src_friend = GetBitMapAttr( destscreen->sc_RastPort.rp_BitMap, BMA_FLAGS ) & BMF_INTERLEAVED ? NULL : destscreen->sc_RastPort.rp_BitMap;
				}
				else
				{
					src_friend = NULL;
				}

				new_bm = AllocBitMap(
					new_xs,
					new_ys,
					GetBitMapAttr( data->bgbitmap, BMA_DEPTH ),
					BMF_MINPLANES | BMF_DISPLAYABLE,
					src_friend
				);

				new_mask = AllocBitMap(
					new_xs,
					new_ys,
					1,
					0,
					NULL
				);
			}

			if( new_bm )
			{
				// Copy over old image to new image
				int xp, yp;

#if USE_ALPHA
				if( data->bgmask == ( APTR )-1 )
				{
					struct RastPort rp;
					InitRastPort( &rp );
					rp.BitMap = new_bm;

					for( xp = 0; xp < new_xs; xp += data->bgbitmap_xs )
					{
						for( yp = 0; yp < new_ys; yp += data->bgbitmap_ys )
						{
							WritePixelArrayAlpha( data->bgbitmap->Planes[ 0 ],
								0, 0, GetCyberMapAttr( data->bgbitmap, CYBRMATTR_XMOD ),
								&rp, xp, yp,
								data->bgbitmap_xs, data->bgbitmap_ys,
								0xffffffff
							);
						}
					}
				}
				else
#endif
				{
					//DB( ( "normal copy\n" ) );
					for( xp = 0; xp < new_xs; xp += data->bgbitmap_xs )
					{
						for( yp = 0; yp < new_ys; yp += data->bgbitmap_ys )
						{
							BltBitMap(
								data->bgbitmap, 0, 0,
								new_bm, xp, yp,
								data->bgbitmap_xs, data->bgbitmap_ys,
								0xc0, -1, NULL
							);
							BltBitMap(
								data->bgmask, 0, 0,
								new_mask, xp, yp,
								data->bgbitmap_xs, data->bgbitmap_ys,
								0xc0, -1, NULL
							);
						}
					}
					data->bgmask = new_mask;
				}
				data->bgbitmap = new_bm;
				data->bgbitmap_free = TRUE;
				data->bgbitmap_xs = new_xs;
				data->bgbitmap_ys = new_ys;
			}
		}

		if( rc && imgdec_isdone( data->bgimage ) )
		{
			data->li.flags |= LOF_BACKFILLING;
			MUI_Redraw( obj, MADF_DRAWOBJECT );
		}

		return( 0 );
}

#define _isinobject(x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_top(obj)+data->li.ys-1))
#define _isinobject2(obj,x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_bottom(obj)))
#define _between(a,x,b) ((x)>=(a) && (x)<=(b))

DECMMETHOD( HandleEvent )
{
	GETDATA;

	if( msg->imsg )
	{
		if( msg->imsg->Class == IDCMP_INTUITICKS )
		{
			if( _isinobject2( data->ctx->dom_document, msg->imsg->MouseX, msg->imsg->MouseY ) && _isinobject( msg->imsg->MouseX, msg->imsg->MouseY ) )
			{
				struct tnr *xtnr = find_tnr_by_xy( obj, data, msg->imsg->MouseX, msg->imsg->MouseY );
				APTR anchor = NULL;
				char *url = NULL;
				char tmp[ 256 ];

				if( xtnr )
				{
					anchor = xtnr->tn->anchor;
				}
				else
				{
					APTR o = NULL;
					APTR optr, ostate;
					struct List *l;
					//o = (APTR)DoMethod( obj, MUIM_WhichObject, msg->imsg->MouseX, msg->imsg->MouseY );
					get( _win( obj ), MUIA_Window_MouseObject, &o );
					get( obj, MUIA_Group_ChildList, &l );
					ostate = l->lh_Head;
					#if 1
					if ( _parent( o ) == obj )
					{
						get( o, MA_Layout_Image_Anchor, &anchor );
						get( o, MA_Layout_Image_AnchorURL, &url );
					}
					#else
					while( optr = NextObject( &ostate ) )
					{
						if( optr == o )
						{
							get( o, MA_Layout_Image_Anchor, &anchor );
							get( o, MA_Layout_Image_AnchorURL, &url );
							break;
						}
					}
					#endif
				}

				if( anchor != data->last_anchor )
				{
					if( data->last_anchor )
						DoMethod( data->last_anchor, MM_Layout_Anchor_HandleMouseEvent, 2, msg->imsg->MouseX, msg->imsg->MouseY );

					if( anchor )
					{
						//DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );

						if( DoMethod( anchor, MM_Layout_Anchor_HandleMouseEvent, 1, msg->imsg->MouseX, msg->imsg->MouseY ) )
						{
							goto anchordone;
						}

						if( !url )
							get( anchor, MA_Layout_Anchor_URL, &url );

						if( url )
						{
							DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_SELECTLINK );
							stccpy( data->last_anchor_url, url, sizeof( data->last_anchor_url ) );
							sprintf( tmp, "Goto \033b%.240s", url );
							DoMethod( data->ctx->dom_win, MM_HTMLWin_SetTempStatus, ( ULONG )tmp );
						}
						else
						{
							DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );
							DoMethod( data->ctx->dom_win, MM_HTMLWin_ResetStatus );
						}
					}
					else
					{
						DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );
						DoMethod( data->ctx->dom_win, MM_HTMLWin_ResetStatus );
					}
				}
				#if 0
				else if( !anchor )
				{
					if( url && strcmp( data->last_anchor_url, url ) )
					{
						DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_SELECTLINK );
						sprintf( tmp, "Goto \033b%.240s", url );
						DoMethod( data->ctx->dom_win, MM_HTMLWin_SetTempStatus, ( ULONG )tmp );
						stccpy( data->last_anchor_url, url, sizeof( data->last_anchor_url ) );
					}
					else if( !url && data->last_anchor_url[ 0 ] )
					{
						DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );
						data->last_anchor_url[ 0 ] = 0;
						DoMethod( data->ctx->dom_win, MM_HTMLWin_ResetStatus );
					}
				}
				#endif

anchordone:
				data->last_anchor = anchor;
			}
			else
			{
				/*
				 * XXX: when switching quickly from object 1 to object 2 without
				 * having an event ending in lo_group, we get that case and
				 * it clears the status. This is all wrong [zapek]
				 * I think we should send a MA_Layout_Image_Anchor etc.. like above
				 */
				if( data->last_anchor )
				{
					DoMethod( data->ctx->dom_win, MM_HTMLWin_SetPointer, POINTERTYPE_SELECTLINK );
					if( !DoMethod( data->last_anchor, MM_Layout_Anchor_HandleMouseEvent, 2, msg->imsg->MouseX, msg->imsg->MouseY ) )
					{
						DoMethod( data->ctx->dom_win, MM_HTMLWin_ResetStatus ); /* XXX: what's the point ?? [zapek] */
					}

					data->last_anchor = NULL;
					DoMethod( data->ctx->dom_win, MM_HTMLWin_ResetStatus );
				}
			}
		}
		else if( msg->imsg->Class == IDCMP_MOUSEBUTTONS )
		{
			if( _isinobject2( data->ctx->dom_document, msg->imsg->MouseX, msg->imsg->MouseY ) && _isinobject( msg->imsg->MouseX, msg->imsg->MouseY ) )
			{
				struct tnr *xtnr = find_tnr_by_xy( obj, data, msg->imsg->MouseX, msg->imsg->MouseY );

				if( msg->imsg->Code == SELECTDOWN )
				{
					if( xtnr && xtnr->tn->anchor )
					{
						set( _win( obj ), MUIA_Window_ActiveObject, xtnr->tn->anchor );
						set( xtnr->tn->anchor, MA_Layout_Anchor_Qualifier, msg->imsg->Qualifier );
						SetAttrs( xtnr->tn->anchor, MUIA_Pressed, TRUE, MUIA_Selected, TRUE, TAG_DONE );
						data->selected_anchor = xtnr->tn->anchor;
						return( MUI_EventHandlerRC_Eat );
					}
				}
				else if( msg->imsg->Code == SELECTUP && data->selected_anchor )
				{
					if( xtnr && xtnr->tn->anchor && xtnr->tn->anchor == data->selected_anchor )
					{
						if( DoMethod( xtnr->tn->anchor, MM_Layout_Anchor_HandleMouseEvent, 0, msg->imsg->MouseX, msg->imsg->MouseY ) )
							return( MUI_EventHandlerRC_Eat );

						set( data->selected_anchor, MUIA_Pressed, FALSE );
					}
					set( data->selected_anchor, MUIA_Selected, FALSE );
					data->selected_anchor = NULL;
				}
			}
			else if( msg->imsg->Code == SELECTUP )
			{
				if( data->selected_anchor )
				{
					set( data->selected_anchor, MUIA_Selected, FALSE );
					data->selected_anchor = NULL;
				}
			}
		}
		else if( msg->imsg->Class == IDCMP_RAWKEY )
		{
			// Broadcast rawkeys for accesskey handling
			if( !( msg->imsg->Qualifier & IEQUALIFIER_LSHIFT ) )
				DoSuperMethod( cl, obj, MM_Layout_Anchor_HandleAccessKey, msg->imsg->Code );
		}
	}

	return( 0 );
}

DECSMETHOD( Layout_Group_HighliteAnchor )
{
	GETDATA;
	struct tnr *tnr;

	data->nobackfill = TRUE;
	for( tnr = FIRSTNODE( &data->v ); NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
	{
		if( tnr->tn->anchor == msg->anchor )
		{
			tnr->redrawme = TRUE;
			data->redrawlink = TRUE;
			tnr->anchorselected = msg->state;
		}
	}

	MUI_Redraw( obj, MADF_DRAWUPDATE );
	data->nobackfill = FALSE;

	return( 0 );
}

DECSMETHOD( Layout_Group_ActiveAnchor )
{
	GETDATA;
	struct tnr *tnr;

	data->nobackfill = TRUE;
	for( tnr = FIRSTNODE( &data->v ); NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
	{
		if( tnr->tn->anchor == msg->anchor )
		{
			tnr->redrawme = TRUE;
			data->redrawlink = TRUE;
		}
	}

	if( !msg->state )
		MUI_Redraw( obj, MADF_DRAWUPDATE );

	for( tnr = FIRSTNODE( &data->v ); NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
	{
		if( tnr->tn->anchor == msg->anchor )
		{
			tnr->anchoractive = msg->state;
		}
	}

	if( msg->state )
		MUI_Redraw( obj, MADF_DRAWUPDATE );
	data->nobackfill = FALSE;

	return( 0 );
}

DECMMETHOD( Draw )
{
	ULONG oldpen = -1, oldstyle = -1;
	struct TextFont *oldfont = NULL;
	GETDATA;
	struct tnr *tnr;
	struct RastPort *rp = _rp( obj );
	int offs_x = _left( obj );
	int offs_y = _vtop( obj ) + data->topoffset;
	ULONG alink_pen;
	ULONG V_GroupDraw( APTR obj, struct IClass *cl, struct MUIP_Draw *msg );

	// Calling super method...
	V_GroupDraw( obj, cl->cl_Super, msg );

	if( msg->flags & MADF_DRAWOBJECT )
	{
		if( data->border && !ISLISTEMPTY( &data->l ) )
		{
			int x1, x2, y1, y2;
			int c;

			x1 = _left( obj );
			x2 = _right( obj );
			y1 = _vtop( obj );
			y2 = _vbottom( obj );

			if( data->pen_borderdark == LO_NOPEN )
			{
				data->pen_borderdark = layout_getpen( data->ctx, data->penspec_borderdark );
				data->pen_borderlight = layout_getpen( data->ctx, data->penspec_borderlight );
			}

			SetAPen( rp, data->pen_borderlight );
			for( c = 0; c < data->border; c++ )
			{
				HLine( rp, x1 + c, x2 - c, y1 + c );
				VLine( rp, x1 + c, y1 + c, y2 - c );
			}
			SetAPen( rp, data->pen_borderdark );
			for( c = 0; c < data->border; c++ )
			{
				HLine( rp, x1 + c, x2, y2 - c );
				VLine( rp, x2 - c, y1 + c, y2 - c );
			}

		}
	}

	if( !( msg->flags & MADF_DRAWUPDATE ) )
		return( 0 );

	alink_pen = layout_getpen( data->ctx, data->ctx->penspec_alink );

	// Draw texts. Text is guaranteed to be sorted in top-bottom order,
	// so we can abort rendering if we exceed our bottom
	// This part is used to redraw links (selected/unselected)
	if ( data->redrawlink )
	{
		data->redrawlink = FALSE;

		for( tnr = FIRSTNODE( &data->v ); NEXTNODE( tnr ); tnr = NEXTNODE( tnr ) )
		{
			ULONG pen;

			if( !tnr->redrawme )
				continue;

			tnr->redrawme = FALSE;

			if( tnr->anchorselected )
			{
				pen = alink_pen;
			}
			else
			{
				if( tnr->pen == LO_NOPEN )
					tnr->pen = layout_getpen( data->ctx, tnr->tn->penspec );
				pen = tnr->pen;
			}

			if( oldpen != pen )
			{
				SetAPen( rp, pen );
				oldpen = pen;
			}
			if( oldfont != tnr->tn->font )
			{
				SetFont( rp, tnr->tn->font );
				oldfont = tnr->tn->font;
				oldstyle = -1;
			}
			if( oldstyle != tnr->tn->style )
			{
				SetSoftStyle( rp, tnr->tn->style, FSF_BOLD | FSF_UNDERLINED | FSF_ITALIC );
				oldstyle = tnr->tn->style;
			}

			Move( rp, offs_x + tnr->xp, offs_y + tnr->yp );
	#if USE_LIBUNICODE
			mbxText( rp, tnr->text, tnr->textlen );
	#else
			Text( rp, tnr->text, tnr->textlen );
	#endif

			if( tnr->tn->anchor )
			{
				static UWORD areaptr[ ] = { 0xcccc, 0xcccc };
				if( gp_underline_links )
				{
					int linky = offs_y + tnr->yp + 2;

					if( tnr->tn->style & FSF_VISITEDLINK )
					{
						SetAfPt( rp, areaptr, 1 );
						RectFill( rp,
							offs_x + tnr->xp, linky,
							offs_x + tnr->xp + tnr->pixellen - 1, linky
						);
						SetAfPt( rp, NULL, 0 );
					}
					else
					{
						RectFill( rp,
							offs_x + tnr->xp, linky,
							offs_x + tnr->xp + tnr->pixellen - 1, linky
						);
					}
				}

				if( tnr->anchoractive )
				{
					// Draw active frame around anchor
					SetDrMd( rp, JAM2 | COMPLEMENT );
					SetAfPt( rp, areaptr, 1 );
					HLine( rp, offs_x + tnr->xp, offs_x + tnr->xp + tnr->pixellen - 1, offs_y + tnr->yp - tnr->tn->font->tf_Baseline );
					HLine( rp, offs_x + tnr->xp, offs_x + tnr->xp + tnr->pixellen - 1, offs_y + tnr->yp - tnr->tn->font->tf_Baseline + tnr->tn->font->tf_YSize );
					VLine( rp, offs_x + tnr->xp, offs_y + tnr->yp - tnr->tn->font->tf_Baseline, offs_y + tnr->yp - tnr->tn->font->tf_Baseline + tnr->tn->font->tf_YSize );
					VLine( rp, offs_x + tnr->xp + tnr->pixellen - 1, offs_y + tnr->yp - tnr->tn->font->tf_Baseline, offs_y + tnr->yp - tnr->tn->font->tf_Baseline + tnr->tn->font->tf_YSize );
					SetAfPt( rp, NULL, 0 );
					SetDrMd( rp, JAM1 );
				}
			}
		}
	}

	return( 0 );
}
DECSMETHOD( Layout_Backfill )
{
	GETDATA;
	int didrender = FALSE;

	if( data->bgcolor != LO_NOPEN && ( !data->bgbitmap || data->bgbitmap_has_mask ) )
	{
		if( data->bgcolor_pen == LO_NOPEN )
			data->bgcolor_pen = layout_getpen( data->ctx, data->bgcolor );
		SetAPen( _rp( obj ), data->bgcolor_pen );
		if( msg->left <= msg->right && msg->top <= msg->bottom )
			RectFill( _rp( obj ), msg->left, msg->top, msg->right, msg->bottom );

		didrender = TRUE;
	}
	if( data->bgbitmap )
	{
#ifndef MBX
		struct mybfhook mbfh;
		struct Rectangle rect;

		rect.MinX = msg->left;
		rect.MinY = msg->top;
		rect.MaxX = msg->right;
		rect.MaxY = msg->bottom;

#ifdef __MORPHOS__
		mbfh.h.h_Entry = ( void * )&bffunc;
#else
		mbfh.h.h_Entry = (HOOKFUNC)bffunc;
#endif
		mbfh.xo = -_mleft(obj);//-msg->left;
		mbfh.yo = -_mtop(obj);//-msg->top;

		mbfh.bm = getclone( data->bgbitmap, ( ( data->bgmask > 0 && data->bgmask != ( APTR )-1 ) ? TRUE : FALSE ) );

		//DB( ( "data->bgmask: 0x%lx\n", data->bgmask ) );

		if( data->bgmask > 0 && data->bgmask != ( APTR )-1 )
			mbfh.maskbm = getclone( data->bgmask, TRUE );
		else
			mbfh.maskbm = data->bgmask;

		mbfh.bmx = data->bgbitmap_xs;
		mbfh.bmy = data->bgbitmap_ys;

		InitRastPort( &mbfh.drp );

		DoHookClipRects( &mbfh, _rp( obj ), &rect );
#else
		BltBitMapRastPortTiled( data->bgbitmap, 0, 0, data->bgbitmap_xs, data->bgbitmap_ys,
			msg->left - _mleft(obj), msg->top - _mtop(obj),
			_rp( obj ),
			msg->left, msg->top,
			msg->right - msg->left + 1, msg->bottom - msg->top + 1
		);

#endif

		didrender = TRUE;
	}

	if( !didrender && _parent( obj ) )
		DoMethodA( _parent( obj ), (Msg)msg );

	return( 0 );
}

DECMMETHOD( ContextMenuBuild )
{
#if USE_MENUS
	GETDATA;

	D( db_gui, bug( "showing context menu\n" ) );

	if( data->cmenu )
	{
		MUI_DisposeObject( data->cmenu );
	}

	if( data->cmenu = MenustripObject, End )
	{
		/*
		 * Find out if we are over an anchor
		 */
		struct tnr *xtnr = find_tnr_by_xy( obj, data, msg->mx, msg->my );

		if( xtnr && xtnr->tn->anchor )
		{
			/* store that so that ContextMenuChoice knows what to do */
			stccpy( data->last_cmenu_anchor, (STRPTR)getv( xtnr->tn->anchor, MA_Layout_Anchor_URL ), sizeof( data->last_cmenu_anchor ) );
		}
		else
		{
			data->last_cmenu_anchor[ 0 ] = '\0';
		}

		return( ( ULONG )build_context_menu( data->last_cmenu_anchor[ 0 ] ? VMT_LINKMODE : VMT_PAGEMODE, data->cmenu, NULL ) );
	}
#endif

	displaybeep();

	return( 0 );
}

DECMMETHOD( ContextMenuChoice )
{
#if USE_MENUS
	GETDATA;

	if( data->last_cmenu_anchor[ 0 ] )
	{
		rexx_obj = obj;
		execute_command( getprefslong( DSI_CMENUS_LINK_ACTION + muiUserData( msg->item ), 0 ), getprefsstr( DSI_CMENUS_LINK_ARGS + muiUserData( msg->item ), "" ), VREXX_FRAME, ( STRPTR )data->last_cmenu_anchor, ( STRPTR )data->last_cmenu_anchor, ( STRPTR )getv( data->ctx->dom_win, MA_JS_Window_URL ) );
	}
	else
	{
		rexx_obj = data->ctx->dom_document;
		execute_command( getprefslong( DSI_CMENUS_PAGE_ACTION + muiUserData( msg->item ), 0 ), getprefsstr( DSI_CMENUS_PAGE_ARGS + muiUserData( msg->item ), "" ), VREXX_FRAME, ( STRPTR )"", ( STRPTR )"", ( STRPTR )getv( data->ctx->dom_win, MA_JS_Window_URL ) ); /* TOFIX: NYI ! */
	}
#endif /* USE_MENUS */

	return( 0 );
}

DECSMETHOD( JS_FindByName )
{
	APTR ostate, o, or;
	struct List *l;

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		char *name = NULL;

		if( ( or = (APTR)DoMethodA( o, ( Msg )msg ) ) )
		{
			return( (ULONG)or );
		}

		get( o, MA_JS_Name, &name );
		if( name && !strcmp( name, msg->name ) )
			return( (ULONG)or );
	}
	return( 0 );
}

DECSMETHOD( JS_FindByID )
{
	APTR ostate, o, or;
	struct List *l;

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		char *name = NULL;

		if( ( or = (APTR)DoMethodA( o, ( Msg )msg ) ) )
		{
			return( (ULONG)or );
		}

		get( o, MA_JS_ID, &name );
		if( name && !strcmp( name, msg->name ) )
			return( (ULONG)or );
	}
	return( 0 );
}

/*
 * The following methods are mostly forwarded
 */
DECMETHOD( HTMLWin_Backward, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLWin_Backward ) );
	}
	return( FALSE );
}

DECMETHOD( HTMLWin_Forward, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLWin_Forward ) );
	}
	return( FALSE );
}

DECMETHOD( HTMLRexx_LoadBG, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLRexx_LoadBG ) );
	}
	return( FALSE );
}

DECSMETHOD( HTMLRexx_SetURLFromObject )
{
	GETDATA;

	if( data->last_cmenu_anchor[ 0 ] )
	{
		if( msg->newwin )
		{
			win_create( "", data->last_cmenu_anchor, NULL /* TOFIX!! where to get the referer ? */, NULL, FALSE, msg->reload, FALSE );
		}
		else
		{
			//TOFIX!!
			return( FALSE );
		}
	}
	return( FALSE );
}

DECTMETHOD( HTMLRexx_SetClipFromObject )
{
	GETDATA;

	if( data->last_cmenu_anchor[ 0 ] )
	{
		return( (ULONG)storetoclip( data->last_cmenu_anchor ) );
	}
	return( FALSE );
}

DECMETHOD( HTMLRexx_OpenSourceView, ULONG )
{
	//TOFIX!! NYI

	return( 0 );
}

DECSMETHOD( HTMLRexx_OpenDocInfo )
{
	GETDATA;

	if( msg->url )
	{
		return( ( ULONG )createdocinfowin( msg->url ) );
	}
	else
	{
		if( data->last_anchor_url )
		{
			return( ( ULONG )createdocinfowin( data->last_anchor_url ) );
		}
	}

	return( FALSE );
}

DECSMETHOD( HTMLRexx_SaveURL )
{
	GETDATA;

#if USE_NET
	if( msg->url )
	{
		queue_download( msg->url, NULL, TRUE, msg->ask ); //TOFIX!! name is ignored
		return( TRUE );
	}
	else
	{
		if( data->last_anchor_url )
		{
			queue_download( data->last_anchor_url, NULL, TRUE, msg->ask );/* TOFIX!! where's the referer ? */ //TOFIX name ignored
			return( TRUE );
		}
	}
#endif /* USE_NET */
	return( FALSE );
}

DECMMETHOD( Show )
{
	struct MinList *childlist;
	APTR o, ostate;
	extern int V_ShowClipped( APTR obj, struct LongRect *clip ); // from htmlview.c
	GETDATA;

	get( obj, MUIA_Group_ChildList, &childlist );
	ostate = childlist->mlh_Head;
	NextObject( &ostate ); // Skip Dummy
	while( o = NextObject( &ostate ) )
	{
		if( _flags( o ) & MADF_SHOWME )
			V_ShowClipped( o, msg->clip );
	}

	data->ehnode.ehn_Object = obj;
	data->ehnode.ehn_Class = cl;
	data->ehnode.ehn_Events = IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS | IDCMP_RAWKEY;
	data->ehnode.ehn_Priority = 1;
	data->ehnode.ehn_Flags = MUI_EHF_GUIMODE;
	DoMethod( _win( obj ), MUIM_Window_AddEventHandler, ( ULONG )&data->ehnode );

	return(TRUE);
}

DECMMETHOD( Hide )
{
	GETDATA;

	DoMethod( _win( obj ), MUIM_Window_RemEventHandler, ( ULONG )&data->ehnode );

	return( DOSUPER );
}

BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
//DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( Draw )
DEFMMETHOD( Show )
DEFMMETHOD( Hide )
DEFMMETHOD( HandleEvent )
DEFMMETHOD( ContextMenuBuild )
DEFMMETHOD( ContextMenuChoice )
DEFSMETHOD( Layout_Group_AddText )
DEFSMETHOD( Layout_Group_AddObject )
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CheckLayout )
DEFSMETHOD( Layout_Group_HighliteAnchor )
DEFSMETHOD( Layout_Group_ActiveAnchor )
DEFMMETHOD( Backfill ) //TOFIX
DEFSMETHOD( Layout_Backfill )
DEFTMETHOD( ImgDecode_Done )
DEFSMETHOD( JS_FindByName )
DEFSMETHOD( JS_FindByID )
DEFSMETHOD( HTMLRexx_SetURLFromObject )
DEFSMETHOD( HTMLRexx_OpenDocInfo )
DEFSMETHOD( HTMLRexx_SaveURL )
DEFMETHOD( HTMLRexx_OpenSourceView )
DEFMETHOD( HTMLWin_Backward )
DEFMETHOD( HTMLWin_Forward )
DEFMETHOD( HTMLRexx_LoadBG )
DEFTMETHOD( HTMLRexx_SetClipFromObject )

case MM_ImgDecode_HasInfo:
case MM_ImgDecode_GotScanline:
case MM_Layout_Anchor_HandleAccessKey:
	return( 0 ); // Eat them, we don't forwarding to childs!

case MM_JS_GetGCMagic:
	// We're not under GC control
	// Note that SetGCMagic is intentionally being broadcasted
	// to all child objects
	return( 0 );

ENDMTABLE

int create_logroupclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "logroupClass";
#endif

	return( TRUE );
}

void delete_logroupclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlogroupclass( void )
{
	return( lcc->mcc_Class );
}

APTR getlogroupmcc( void )
{
	return( lcc );
}


