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
** $Id: lo_image.c,v 1.158 2004/01/06 20:23:08 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <graphics/scale.h>
#include <proto/graphics.h>
#include <cybergraphx/cybergraphics.h>
#if USE_CGX
#include <proto/cybergraphics.h>
#endif
#endif

/* private */
#include "classes.h"
#include <proto/vimgdecode.h>
#include "prefs.h"
#include "voyager_cat.h"
#include "js.h"
#include "urlparser.h"
#include "htmlclasses.h"
#ifndef MBX
#include "bitmapclone.h"
#endif
#include "layout.h"
#include "methodstack.h"
#include "malloc.h"
#include "mui_func.h"
#include "menus.h"
#include "rexx.h"
#include "docinfowin.h"
#include "download.h"
#include "lo_image.h"
#include "clip.h"

static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *alttext;
	char *url;
	char *lowsrcurl;
	char *widthspec;
	char *heightspec;
	int border;
	APTR anchor;
	time_t anchor_visited;
	struct imgclient *img;

	ULONG qualifier;

	struct BitMap *bm;
	int ixs, iys;
	struct BitMap *maskbm;
	struct MinList *imglist;
	struct imgframenode *imf;   // current node
	struct BitMap *scaled_bm, *scaled_mask;

	int scaled;
	int newpartialys, newpartialye;

	int width, height;
	int percentwidth, percentheight;
	int marginleft, marginright, marginbottom, margintop;
	int mleft, mtop, mbottom, mright;

	// Form stuff
	APTR form;
	char *name;
	int meactive, me_x, me_y;

	// Image map
	char *usemap;
	APTR mapobjest;
	APTR activeareaobjest;
	int ismap;
	char *tempurl;
	int tempurlsize;

	// Anim updates
	struct MUI_InputHandlerNode ihn;
	int repeatcnt;

	// context menu
	APTR cmenu;
	LONG cmenu_anchor;
	ULONG split;

	struct MUI_EventHandlerNode ehnode;

	UBYTE frameupdate;
	UBYTE ihn_active;
	UBYTE broken_image;
	UBYTE play_anim;

	UBYTE width_given;
	UBYTE height_given;

	UBYTE fillbg;
	UBYTE is_blank;	// Image is completely transparent

	UBYTE done;

	UBYTE pressed;

	UBYTE pad[ 2 ];

	// Event handler function indices
	int ix_onclick, ix_mouseover, ix_mouseout;

};

int dest_is_interleaved;
struct Screen *destscreen;

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

#define MINTERM_B_AND_C     ABC | NABC
#define MINTERM_NOT_B_AND_C (ANBC | NANBC)
#define MINTERM_B_OR_C      (ABC | ABNC | NABC | NABNC | ANBC | NANBC)
#define MINTERM_B           (ABC | ABNC | NABC | NABNC)

#ifdef MBX
UDWORD __inline BltBitMapAlphaRastPort(BitMap_p srcBitMap, DWORD xSrc,
	DWORD ySrc, RastPort_p destRP, DWORD xDest,
	DWORD yDest, DWORD xSize, DWORD ySize, ARGB32 bgcolor)
{
	SetRastPortAttr(destRP, RPTAG_DRAWMODE, DRMD_ALPHA);
	BltBitMapRastPort( srcBitMap, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, 0xc0 );
	SetRastPortAttr(destRP, RPTAG_DRAWMODE, DRMD_JAM1);
	return(0);
}
#endif

static void sendcheckdone( struct Data *data )
{
	// Send a CHECKDONE to the htmlwin...
	if( data->ctx )
		pushmethod( data->ctx->dom_win, 1, MM_HTMLWin_CheckDone );
}

static void drawbackground( struct Data *data, APTR obj, int ystart, int yend )
{
	if( _parent( obj ) )
	{
		DoMethod( _parent( obj ), MM_Layout_Backfill,
			data->mleft, data->mtop + ystart,
			data->mright, data->mtop + yend
		);
	}
}

static void drawbackgroundxs( struct Data *data, APTR obj, int xp, int yp, int xs, int ys )
{
	if( _parent( obj ) )
	{
		DoMethod( _parent( obj ), MM_Layout_Backfill,
			data->mleft + xp, data->mtop + yp,
			data->mleft + xp + xs - 1, data->mtop + yp + ys - 1
		);
	}
}


#ifdef MBX

static BOOL __inline hastvbg(struct Data *data)
{
	ULONG pip=0;

	if (data->ctx && data->ctx->dom_win)
		get(data->ctx->dom_win,MA_HTMLWin_BGPip,&pip);

	return(pip ? TRUE : FALSE);
}

#else

#define hastvbg(data) 0

#endif


static void __inline blitnode( APTR obj, struct Data *data, struct imgframenode *imf )
{
	if( imf->maskbm && !hastvbg(data))
	{
#ifndef MBX
#if USE_ALPHA
		if( imf->maskbm == (APTR)-1 ) /* TOFIX: is that enough ? */
		{
			WritePixelArrayAlpha( imf->bm->Planes[ 0 ], 0, 0, GetCyberMapAttr( imf->bm, CYBRMATTR_XMOD ),
				_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
				imf->xs, imf->ys,
				0xffffffff
			);
		}
		else
#endif /* USE_ALPHA */
		{
			BltMaskBitMapRastPort( getclone( imf->bm, TRUE ), 0, 0,
				_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
				imf->xs, imf->ys,
				(ABC|ABNC|ANBC), getclone( imf->maskbm, TRUE )->Planes[ 0 ]
			);
		}
#else //TOFIX!!
//SetAlphaChannelBM(imf->bm,0,0,imf->xs,imf->ys,0xff);
//kprintf("  alphablit: %08lx %08lx %08lx\r\n",((ULONG *)imf->bm->bm_PixMap)[0],((ULONG *)imf->bm->bm_PixMap)[1],((ULONG *)imf->bm->bm_PixMap)[2] );
		BltBitMapAlphaRastPort( imf->bm, 0, 0,
			_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
			imf->xs, imf->ys,
			0xFFFFFFFF
		);
#endif
	}
	else
	{
		#ifdef MBX
		//SetAlphaChannelBM(imf->bm,0,0,imf->xs,imf->ys,0xff);
		//kprintf("noalphablit: %08lx %08lx %08lx\r\n",((ULONG *)imf->bm->bm_PixMap)[0],((ULONG *)imf->bm->bm_PixMap)[1],((ULONG *)imf->bm->bm_PixMap)[2] );
		#endif
		BltBitMapRastPort( getclone( imf->bm, FALSE ), 0, 0,
			_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
			imf->xs, imf->ys,
			0xc0
		);
	}
}

static void __inline blitnode_scaled( APTR obj, struct Data *data )
{
	if( data->maskbm && !hastvbg(data))
	{
		if( data->fillbg )
		{
			drawbackground( data, obj, 0, data->height - 1 );
			data->fillbg = FALSE;
		}
#ifndef MBX
#if USE_ALPHA
		if( data->maskbm == (APTR)-1 ) /* TOFIX: is that enough ? */
		{
			WritePixelArrayAlpha( data->bm->Planes[ 0 ], 0, 0, GetCyberMapAttr( data->bm, CYBRMATTR_XMOD ), /* hopeing that ixs is the right field :) */
				_rp( obj ), data->mleft, data->mtop,
				data->width, data->height,
				0xffffffff
			);
		}
		else
#endif /* USE_ALPHA */
		{
			BltMaskBitMapRastPort( getclone( data->bm, TRUE ), 0, 0,
				_rp( obj ), data->mleft, data->mtop,
				data->width, data->height,
				(ABC|ABNC|ANBC), getclone( data->maskbm, TRUE )->Planes[ 0 ]
			);
		}
#else //TOFIX!!
		BltBitMapAlphaRastPort( data->bm, 0, 0,
			_rp( obj ), data->mleft, data->mtop,
			data->width, data->height,
			0xFFFFFFFF
		);
#endif
	}
	else
	{
		BltBitMapRastPort( getclone( data->bm, FALSE ), 0, 0,
			_rp( obj ), data->mleft, data->mtop,
			data->width, data->height,
			0xc0
		);
	}
}


static void __inline blitnode_keepold( APTR obj, struct Data *data, struct imgframenode *imf )
{
	if( imf->maskbm && !hastvbg(data))
	{
#ifndef MBX
#if USE_ALPHA
		if( imf->maskbm == (APTR)-1 ) /* TOFIX: is that enough ? */
		{
			WritePixelArrayAlpha( imf->bm->Planes[ 0 ], 0, 0, GetCyberMapAttr( imf->bm, CYBRMATTR_XMOD ),
				_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
				imf->xs, imf->ys,
				0xffffffff
			);
		}
		else
		{
#endif
			BltMaskBitMapRastPort( getclone( imf->bm, TRUE ), 0, 0,
				_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
				imf->xs, imf->ys,
				(ABC|ABNC|ANBC), getclone( imf->maskbm, TRUE )->Planes[ 0 ]
			);
		}
#else
		{
			BltBitMapAlphaRastPort( getclone( imf->bm, TRUE ), 0, 0,
				_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
				imf->xs, imf->ys,
				0xFFFFFFFF
			);
		}
#endif /* MBX */
	}
	else
	{
		BltBitMapRastPort( getclone( imf->bm, FALSE ), 0, 0,
			_rp( obj ), data->mleft + imf->xp, data->mtop + imf->yp,
			imf->xs, imf->ys,
			0xc0
		);
	}
}


static void blitnodepartial( struct Data *data, APTR obj, struct imgframenode *imf, int ixp, int iyp, int ixs, int iys )
{
	int rox, roy;

	// Calculate offsets
	rox = ixp - imf->xp;
	if( rox < 0 )
	{
		rox = 0;
		ixp = imf->xp;
	}
	roy = iyp - imf->yp;
	if( roy < 0 )
	{
		roy = 0;
		iyp = imf->yp;
	}

	if( imf->maskbm && !hastvbg(data))
	{
#ifndef MBX
#if USE_ALPHA
		if( imf->maskbm == (APTR)-1 ) /* TOFIX: is that enough ? */
		{
			WritePixelArrayAlpha( imf->bm->Planes[ 0 ], rox, roy, GetCyberMapAttr( imf->bm, CYBRMATTR_XMOD ),
				_rp( obj ), data->mleft + ixp, data->mtop + iyp,
				ixs, iys,
				0xffffffff
			);
		}
		else
#endif /* USE_ALPHA */
		{
			BltMaskBitMapRastPort( getclone( imf->bm, TRUE ), rox, roy,
				_rp( obj ), data->mleft + ixp, data->mtop + iyp,
				ixs, iys,
				(ABC|ABNC|ANBC), getclone( imf->maskbm, TRUE )->Planes[ 0 ]
			);
		}
#else //TOFIX!!
		BltBitMapAlphaRastPort( getclone( imf->bm, TRUE ), rox, roy,
			_rp( obj ), data->mleft + ixp, data->mtop + iyp,
			ixs, iys,
			0xFFFFFFFF
		);
#endif
	}
	else
	{
		BltBitMapRastPort( getclone( imf->bm, FALSE ), rox, roy,
			_rp( obj ), data->mleft + ixp, data->mtop + iyp,
			ixs, iys,
			0xc0
		);
	}
}

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			break;

		case MA_Layout_Image_LowSrc:
			l_readstrtag( tag, &data->lowsrcurl );
			break;

		case MA_Layout_Image_Src:
			l_readstrtag( tag, &data->url );
			if( !gp_loadimages || ( gp_loadimages == 1 && ( data->ismap || data->usemap ) )	)
				DoMethod( obj, MM_Layout_Image_LoadImage );
			break;

		case MA_Layout_Image_Width:
			l_readstrtag( tag, &data->widthspec );
			break;

		case MA_Layout_Image_Height:
			l_readstrtag( tag, &data->heightspec );
			break;

		case MA_Layout_Image_Alttxt:
			l_readstrtag( tag, &data->alttext );
			if( data->alttext && *data->alttext )
				set( obj, MUIA_ShortHelp, data->alttext );
			break;

		case MA_Layout_Image_Anchor:
			data->anchor = (APTR)tag->ti_Data;
			break;

		case MA_Layout_Image_Anchor_Visited:
			data->anchor_visited = (time_t)tag->ti_Data;
			break;

		case MA_Layout_Image_Border:
			data->border = tag->ti_Data;
			break;

		case MA_Layout_VAlign:
			data->li.valign = tag->ti_Data;
			break;

		case MA_Layout_Align:
			data->li.align = tag->ti_Data;
			break;

		case MA_Layout_Image_UseMap:
			l_readstrtag( tag, &data->usemap );
			break;

		case MA_Layout_Image_IsMap:
			data->ismap = tag->ti_Data;
			break;

		case MA_Layout_FormElement_Form:
			data->form = (APTR)tag->ti_Data;
			break;

		case MA_Layout_FormElement_Name:
			l_readstrtag( tag, &data->name );
			break;

		case MA_Layout_MarginLeft:
			data->marginleft = tag->ti_Data;
			break;

		case MA_Layout_MarginTop:
			data->margintop = tag->ti_Data;
			break;

		case MA_Layout_MarginRight:
			data->marginright = tag->ti_Data;
			break;

		case MA_Layout_MarginBottom:
			data->marginbottom = tag->ti_Data;
			break;

		case (int)MUIA_Pressed:
		case (int)MUIA_Selected:
			// Forward to anchor
			if( data->usemap )
			{
				#ifdef MBX
				int x = _window( obj )->ww_MouseX - _left( obj ) - data->marginleft;
				int y = _window( obj )->ww_MouseY - _top( obj ) - data->margintop;
				#else
				int x = _window( obj )->MouseX - _left( obj ) - data->marginleft;
				int y = _window( obj )->MouseY - _top( obj ) - data->margintop;
				#endif

				if( !data->mapobjest )
					DoMethod( data->ctx->body, MM_Layout_Map_FindByName, data->usemap, &data->mapobjest );

				if( data->mapobjest )
				{
					APTR o = (APTR)DoMethod( data->mapobjest, MM_Layout_Map_FindArea, x, y );
					if( o )
					{
						set( o, tag->ti_Tag, tag->ti_Data );
						if( tag->ti_Tag == MUIA_Pressed && tag->ti_Data )
							data->activeareaobjest = o;
						else
							data->activeareaobjest = NULL;
					}
				}
			}
			else if( data->anchor )
			{
				// Need to modify the anchor URL?
				if( data->ismap )
				{
					if( data->tempurl )
						set( data->anchor, MA_Layout_Anchor_TempURL, data->tempurl );
				}
				set( data->anchor, MA_Layout_Anchor_Qualifier, data->qualifier );
				set( data->anchor, tag->ti_Tag, tag->ti_Data );
			}
			else if( data->form && tag->ti_Tag == MUIA_Pressed )
			{
				if( tag->ti_Data )
				{
					data->pressed = TRUE;
				}
				else if( data->pressed )
				{
					#ifdef MBX
					data->me_x = _window( obj )->ww_MouseX - _left( obj ) - data->marginleft;
					data->me_y = _window( obj )->ww_MouseY - _top( obj ) - data->margintop;
					#else
					data->me_x = _window( obj )->MouseX - _left( obj ) - data->marginleft;
					data->me_y = _window( obj )->MouseY - _top( obj ) - data->margintop;
					#endif
					data->meactive = TRUE;
					DoMethod( data->form, MM_Layout_Form_Submit, obj );
					data->meactive = FALSE;
					data->pressed = FALSE;
				}
			}
			redraw = TRUE;
			break;

	}

	return( redraw );
}

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		MUIA_CustomBackfill, TRUE,
		MUIA_Font, MUIV_Font_Tiny,
		MA_JS_ClassName, "Image",
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->width = 28;
	data->height = 30;

	doset( data, obj, msg->ops_AttrList );

	/*
	 * Find out the proper width/height, in pixel
	 * and in percent
	 */
	if( data->widthspec && *data->widthspec )
	{
		if( strchr( data->widthspec, '%' ) )
		{
			data->widthspec[ strlen( data->widthspec ) - 1 ] = '\0'; /* nuke the '%' */
			data->width_given = SGT_PERCENT;
			data->percentwidth = atoi( data->widthspec );
			data->percentwidth = max( 0, data->percentwidth );
			data->percentwidth = min( 100, data->percentwidth );
		}
		else
		{
			data->width_given = SGT_PIXEL;
			data->width = atoi( data->widthspec );
		}
	}
	if( data->heightspec && *data->heightspec )
	{
		if( strchr( data->heightspec, '%' ) )
		{
			data->heightspec[ strlen( data->heightspec ) - 1 ] = '\0'; /* nuke the '%' */
			data->height_given = SGT_PERCENT;
			data->percentheight = atoi( data->heightspec );
			data->percentheight = max( 0, data->percentheight );
			data->percentheight = min( 100, data->percentheight );
		}
		else
		{
			data->height_given = SGT_PIXEL;
			data->height = atoi( data->heightspec );
		}
	}

	/*
	 * If only percentage width or height is specified,
	 * the other takes the same value so
	 * that it's proportional.
	 *
	 * Alternatively, for pixel-specifications, the
	 * height takes 8/7ths of the specified width, or
	 * vice-versa with 7/8ths.  IE does this consistently across
	 * all resolutions.  God knows why that constant,
	 * though.  However, if the image later turns up,
	 * it's genuine width/height is taken in place of
	 * these numbers, so we don't change width/height_given
	 * from SGT_UNKNOWN
	 *
	 */
	if( data->width_given == SGT_PERCENT && data->height_given == SGT_UNKNOWN )
	{
		data->percentheight = data->percentwidth;
		data->height_given = SGT_PERCENT;
	}
	else if( data->height_given == SGT_PERCENT && data->width_given == SGT_UNKNOWN )
	{
		data->percentwidth = data->percentheight;
		data->width_given = SGT_PERCENT;
	}
	else if( data->width_given == SGT_PIXEL && data->height_given == SGT_UNKNOWN )
	{
		data->height = ( data->width * 8 ) / 7;
//		data->height_given = SGT_PIXEL;
	}
	else if( data->height_given == SGT_PIXEL && data->width_given == SGT_UNKNOWN )
	{
		data->width = ( data->height * 7 ) / 8;
//		data->width_given = SGT_PIXEL;
	}

	/*
	 * Some sensible values
	 */
	if( data->width < 1 )
		data->width = 1;
	if( data->height < 1 )
		data->height = 1;

	if( data->percentwidth < 1 )
		data->percentwidth = 1;
	if( data->percentheight < 1 )
		data->percentheight = 1;

	if( data->anchor || data->usemap || data->form )
		set( obj, MUIA_InputMode, MUIV_InputMode_RelVerify );

	set( obj, MUIA_ContextMenu, 1 );

	return( (ULONG)obj );
}

DECDISPOSE
{
	GETDATA;

	if( data->scaled_bm )
		FreeBitMap( data->scaled_bm );

	if( data->scaled_mask )
		FreeBitMap( data->scaled_mask );

	if( data->ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, &data->ihn );
	}

#if USE_MENUS
	if( data->cmenu )
	{
		MUI_DisposeObject( data->cmenu );
	}
#endif

	if( data->img )
		imgdec_close( data->img );

	free( data->widthspec );
	free( data->heightspec );
	free( data->alttext );
	free( data->url );
	free( data->lowsrcurl );
	free( data->usemap );
	free( data->tempurl );
	free( data->name );

	killpushedmethods( obj );

	return( DOSUPER );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( DOSUPER );
}

DECGET
{
	GETDATA;
	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_Layout_Info, &data->li );
		STOREATTR( MA_Layout_Image_Anchor, data->anchor );

		case MA_Layout_Image_AnchorURL:
			if( data->anchor || data->usemap )
			{
				#ifdef MBX
				int x = _window( obj )->ww_MouseX - _left( obj ) - data->marginleft;
				int y = _window( obj )->ww_MouseY - _top( obj ) - data->margintop;
				#else
				int x = _window( obj )->MouseX - _left( obj ) - data->marginleft;
				int y = _window( obj )->MouseY - _top( obj ) - data->margintop;
				#endif

				if( data->usemap )
				{
					if( !data->mapobjest )
						DoMethod( data->ctx->body, MM_Layout_Map_FindByName, data->usemap, &data->mapobjest );
					if( data->mapobjest )
					{
						APTR o = (APTR)DoMethod( data->mapobjest, MM_Layout_Map_FindArea, x, y );
						if( o )
						{
							return( (ULONG)get( o, MA_Layout_Anchor_URL, msg->opg_Storage ) );
						}
					}
					*msg->opg_Storage = (ULONG)NULL;
					return( TRUE );
				}
				else if( data->ismap )
				{
					char coords[ 32 ];
					char *baseurl;
					int urlsize;

					get( data->anchor, MA_Layout_Anchor_URL, &baseurl );

					sprintf( coords, "?%u,%u", x, y );

					urlsize = strlen( baseurl ) + strlen( coords ) + 1;

					if( urlsize > data->tempurlsize )
					{
						free( data->tempurl );
						data->tempurlsize = urlsize;
						data->tempurl = malloc( urlsize );
					}

					strcpy( data->tempurl, baseurl );
					strcat( data->tempurl, coords );

					*msg->opg_Storage = (ULONG)data->tempurl;

					return( TRUE );
				}
				return( (ULONG)get( data->anchor, MA_Layout_Anchor_URL, msg->opg_Storage ) );
			}
			return( FALSE );
	}

	return( DOSUPER );
}

DECTMETHOD( Layout_Image_LoadImage )
{
	GETDATA;

	if( data->img )
		imgdec_close( data->img );

	if( data->scaled_bm )
	{
		FreeBitMap( data->scaled_bm );
		data->scaled_bm = NULL;
	}

	if( data->scaled_mask )
	{
		data->scaled_mask = NULL;
		FreeBitMap( data->scaled_mask );
	}

	data->done = FALSE;
	data->scaled = FALSE;
	data->newpartialys = data->newpartialye = 0;
	data->broken_image = FALSE;
	data->is_blank = FALSE;
	data->repeatcnt = 0;
	data->bm = NULL;
	data->maskbm = NULL;
	data->imglist = NULL;
	data->imf = NULL;

	if( data->url && data->url[ 0 ] )
	{
		data->img = imgdec_open( data->url, obj, data->ctx ? data->ctx->baseref : (char*)getv( obj, MA_JS_Object_Baseref ), FALSE );
	}
	else
	{
		data->done = TRUE;
		data->is_blank = TRUE;
		sendcheckdone( data );
	}

	if( data->ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, &data->ihn );
		data->ihn_active = FALSE;
	}

	return( FALSE );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	int iwidth, iheight;

	if( data->width_given == SGT_PERCENT )
	{
		data->width = msg->suggested_width * data->percentwidth / 100;
		// TOFIX! Kludge -- find out why width is 0 here
		if( data->width < 1 )
			data->width = 1;
	}

	if( data->height_given == SGT_PERCENT )
	{
		data->height = msg->suggested_height * data->percentheight / 100;
		// TOFIX! Kludge -- find out why height is 0 here
		if( data->height < 1 )
			data->height = 1;
	}

	iwidth = data->width;
	iheight = data->height;

	// Borders...
	iwidth += data->marginleft + data->marginright;
	iheight += data->margintop + data->marginbottom;

	data->li.xs = iwidth;
	data->li.ys = iheight;

	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	int iwidth, iheight;

	if( data->img )
	{
		if( !data->bm )
		{
			struct BitMap *bm; // dummy...

			if( imgdec_getinfo( data->img, &bm, &data->ixs, &data->iys ) && bm != ( (struct BitMap*)-1 ) )
			{
				if( data->width_given != SGT_PIXEL )
				{
					data->width = data->ixs;
				}

				if( data->height_given != SGT_PIXEL )
				{
					data->height = data->iys;
				}
			}
		}
		if( !data->done && imgdec_isdone( data->img ) )
		{
			struct BitMap *bm;
			int xs, ys;

			imgdec_getinfo( data->img, &bm, &xs, &ys );

			DoMethod( obj, MM_ImgDecode_HasInfo,
				bm, xs, ys,
				imgdec_getmaskbm( data->img ),
				imgdec_getimagelist( data->img )
			);
			DoMethod( obj, MM_ImgDecode_Done );
		}
	}

	iwidth = data->width;
	iheight = data->height;

	// Borders...
	iwidth += data->marginleft + data->marginright;
	iheight += data->margintop + data->marginbottom;

	data->li.minwidth = data->li.maxwidth = data->li.defwidth = iwidth;
	data->li.minheight = data->li.maxheight = data->li.defheight = iheight;

	return( (ULONG)&data->li );
}

static void draw_newscanlines( APTR obj, struct Data *data )
{
	if( data->bm && data->imglist )
	{
		struct imgframenode *imf = FIRSTNODE( data->imglist );

		if( data->maskbm && !hastvbg(data))
		{
			drawbackground( data, obj, data->newpartialys, data->newpartialye );
#ifndef MBX
#if USE_ALPHA
			if( data->maskbm == (APTR)-1 ) /* TOFIX: is that enough ? */
			{
				WritePixelArrayAlpha( imf->bm->Planes[ 0 ], 0, data->newpartialys, GetCyberMapAttr( imf->bm, CYBRMATTR_XMOD ),
					_rp( obj ), data->mleft + imf->xp, data->mtop + data->newpartialys + imf->yp,
					imf->xs, data->newpartialye - data->newpartialys + 1,
					0xffffffff
				);
			}
			else
#endif /* USE_ALPHA */
			{
				BltMaskBitMapRastPort( getclone( imf->bm, TRUE ), 0, data->newpartialys,
					_rp( obj ), data->mleft + imf->xp, data->mtop + data->newpartialys + imf->yp,
					imf->xs, data->newpartialye - data->newpartialys + 1,
					(ABC|ABNC|ANBC), getclone( imf->maskbm, TRUE )->Planes[ 0 ]
				);
			}
#else
			BltBitMapAlphaRastPort( getclone( imf->bm, TRUE ), 0, data->newpartialys,
				_rp( obj ), data->mleft + imf->xp, data->mtop + data->newpartialys + imf->yp,
				imf->xs, data->newpartialye - data->newpartialys + 1,
				0xFFFFFFFF
			);
#endif
		}
		else
		{
			BltBitMapRastPort( getclone( imf->bm, FALSE ), 0, data->newpartialys,
				_rp( obj ), data->mleft + imf->xp, data->mtop + data->newpartialys + imf->yp,
				imf->xs, data->newpartialye - data->newpartialys + 1,
				0xc0
			);
		}
	}
}

static void draw_newframe( APTR obj, struct Data *data )
{
	struct imgframenode *prev;

	if( data->imf == FIRSTNODE( data->imglist ) )
	{
		prev = LASTNODE( data->imglist );
		prev->disposal = 2;
	}
	else
		prev = PREVNODE( data->imf );

	switch( prev->disposal )
	{
		case 2: // restore to background
			drawbackgroundxs( data, obj, prev->xp, prev->yp, prev->xs, prev->ys );
			blitnode( obj, data, data->imf );
			break;

		case 3: // restore to previous
			blitnodepartial( data, obj, FIRSTNODE( data->imglist ), prev->xp, prev->yp, prev->xs, prev->ys );
			blitnode_keepold( obj, data, data->imf );
			break;

		default:
			// simply blit the new node
			blitnode_keepold( obj, data, data->imf );
			break;
	}
}

DECMMETHOD( Draw )
{
	GETDATA;
	struct RastPort *rp = _rp( obj );

	DOSUPER;

	/*{
		static int drawcnt;
		kprintf( "Draw called on %s (%ld)\n", data->url, drawcnt++ );
	}*/

	// Draw border?
	if( msg->flags & MADF_DRAWOBJECT && data->border )
	{
		int b = data->border;
		ULONG pen = data->ctx->penspec_link;

		if( data->anchor )
		{
			if( getv( obj, MUIA_Selected ) )
				pen = data->ctx->penspec_alink;
			else if( data->anchor_visited )
				pen = data->ctx->penspec_vlink;
		}
		SetAPen( rp, layout_getpen( data->ctx, pen ) );

		RectFill( rp, data->mleft - b, data->mtop - b, data->mright + b, data->mtop - 1 );
		RectFill( rp, data->mleft - b, data->mtop - b, data->mleft - 1, data->mbottom + b );
		RectFill( rp, data->mleft - b, data->mbottom + 1, data->mright + b, data->mbottom + b );
		RectFill( rp, data->mright + 1, data->mtop - b, data->mright + b, data->mbottom + b );
	}

	if( data->is_blank )
	{
		if( msg->flags & MADF_DRAWUPDATE )
		{
			// Fill background to get rid of placeholder
			drawbackground( data, obj, 0, data->height - 1 );
		}
		return( 0 );
	}

	if( data->bm && !data->scaled )
	{
		if( msg->flags & MADF_DRAWOBJECT )
		{
			/*
			 * This is broken. We always blit the first because it's
			 * the only frame that can be blited that way. Others frames
			 * depends on the previous one*s* background. Proper solution
			 * would be to blit on an offscreen bitmap, then blit it
			 * back from here.
			 * so, XXX :)
			 */
			struct imgframenode *firstn = FIRSTNODE( data->imglist );

			blitnode( obj, data, firstn );
		}
		else if( msg->flags & MADF_DRAWUPDATE )
		{
			if( data->frameupdate )
			{
				data->frameupdate = FALSE;
				draw_newframe( obj, data );
			}
			else
			{
				// New scanlines
				draw_newscanlines( obj, data );
			}
		}
	}
	else if( data->scaled_bm )
	{
		blitnode_scaled( obj, data );
	}
	else // Alt text / Placeholder
	{
		APTR cliphandle;
		char alttext[ 1024 ];
		int typ, rem;
		int raised;
		ULONG pen;
		int alttextoffs = 0;

		raised = FALSE;
		if( data->anchor && !getv( obj, MUIA_Selected ) )
			raised = TRUE;

		// Fill back
		if( data->fillbg )
		{
			drawbackground( data, obj, 0, data->height - 1 );
			data->fillbg = FALSE;
		}

		if ( gp_image_show_ph_border )
		{
			SetAPen( rp, layout_getpen( data->ctx, raised ? data->ctx->penspec_borderlight : data->ctx->penspec_borderdark ) );
			RectFill( rp, data->mleft, data->mtop, data->mright, data->mtop );
			RectFill( rp, data->mleft, data->mtop, data->mleft, data->mbottom );
			SetAPen( rp, layout_getpen( data->ctx, raised ? data->ctx->penspec_borderdark : data->ctx->penspec_borderlight ) );
			RectFill( rp, data->mright, data->mtop, data->mright, data->mbottom );
			RectFill( rp, data->mleft, data->mbottom, data->mright, data->mbottom );
		}

		if ( gp_image_show_alt_text )
		{
			if( data->width > 4 && data->height > 4 )
			{
				// Render Alttext
				cliphandle = MUI_AddClipping( muiRenderInfo( obj ), data->mleft + 1, data->mtop + 1, data->mright - data->mleft - 2, data->mbottom - data->mtop - 2 );

				SetFont( rp, _font( obj ) );

				pen = data->ctx->penspec_text;
				if( data->anchor )
				{
					if( getv( obj, MUIA_Selected ) )
						pen = data->ctx->penspec_alink;
					else if( data->anchor_visited )
						pen = data->ctx->penspec_vlink;
					else
						pen = data->ctx->penspec_link;
				}

				SetAPen( rp, layout_getpen( data->ctx, pen ) );

				Move( rp, data->mleft + 2, typ = data->mtop + 2 + _font( obj )->tf_Baseline );
				Text( rp, data->broken_image ? "×" : "·", 1 );

				rem = _mwidth( obj ) - 4 - ( rp->cp_x - data->mleft );

				stccpy( alttext, data->alttext ? data->alttext : "Image", sizeof( alttext ) );
				if( data->broken_image )
				{
					char *err = imgdec_errormsg( data->img );
					if( err )
					{
						int errlen = strlen( err ) + 5;
						alttext[ sizeof( alttext ) - errlen - 1 ] = 0;
						strins( alttext, "] " );
						strins( alttext, err );
						strins( alttext, "[" );
					}
				}

				while( strlen( &alttext[ alttextoffs ] ) )
				{
					struct TextExtent te;
					int fit = TextFit( rp, &alttext[ alttextoffs ], strlen( &alttext[ alttextoffs ] ), &te, NULL, 1, rem, 127 );
					if( fit < 1 )
						fit = strlen( &alttext[ alttextoffs ] );
					Text( rp, &alttext[ alttextoffs ], fit );
					alttextoffs += fit;
					typ += _font( obj )->tf_YSize;
					Move( rp, data->mleft + 2, typ );
					rem = _mwidth( obj ) - 4;
				}

				MUI_RemoveClipping( muiRenderInfo( obj ), cliphandle );
			}
		}
	}

	if( data->activeareaobjest )
		DoMethod( data->activeareaobjest, MM_Layout_Area_DrawFrame, data->mleft, data->mtop );

	return( 0 );
}

#define _isinobject(x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_bottom(obj)))
#define _between(a,x,b) ((x)>=(a) && (x)<=(b))

DECMMETHOD( HandleEvent )
{
	GETDATA;

	if( msg->imsg )
	{
		if( msg->imsg->Class == IDCMP_MOUSEBUTTONS )
		{
			if( _isinobject( msg->imsg->MouseX, msg->imsg->MouseY ) )
			{
				if( msg->imsg->Code == SELECTDOWN )
				{
					data->qualifier = msg->imsg->Qualifier;
				}
			}
		}
		else if( msg->imsg->Class == IDCMP_ACTIVEWINDOW )
		{
			if( gp_image_stop_animgif && data->play_anim )
			{
				DoMethod( obj, MM_Layout_Image_StartAnim );
			}
		}
		else if( msg->imsg->Class == IDCMP_INACTIVEWINDOW )
		{
			if( gp_image_stop_animgif && data->play_anim )
			{
				DoMethod( obj, MM_Layout_Image_StopAnim );
			}
		}
	}

	return( 0 );
}

DECSMETHOD( ImgDecode_HasInfo )
{
	GETDATA;
	int sendinfo = FALSE;

	if( data->done )
		return( 0 );

	data->bm = msg->bm;
	data->ixs = msg->img_x;
	data->iys = msg->img_y;

	if( data->bm == (struct BitMap*)-1 )
	{
		data->bm = NULL;
		data->broken_image = TRUE;
		data->fillbg = TRUE;
		MUI_Redraw( obj, MADF_DRAWOBJECT );
		sendcheckdone( data );
		return( 0 );
	}

	markclonemodified( data->bm );

	/*
	 * If the size isn't pixel specified,
	 * restart the layout with the (now known) size
	 */
	switch( data->width_given )
	{
		case SGT_PIXEL:
			/* no need to recompute that one */
			break;

		case SGT_UNKNOWN:
			if( data->height_given == SGT_PIXEL )
			{
				data->width = ( data->ixs * ( ( data->height * 100 ) / data->iys ) ) / 100;
				data->width_given = SGT_PIXEL;
				sendinfo = TRUE;
			}
			else if( data->width != data->ixs )
			{
				data->width = data->ixs;
				sendinfo = TRUE;
			}
			break;
	}

	switch( data->height_given )
	{
		case SGT_PIXEL:
			/* no need to recompute that one */
			break;

		case SGT_UNKNOWN:
			if( data->width_given == SGT_PIXEL )
			{
				data->height = ( data->iys * ( ( data->width * 100 ) / data->ixs ) ) / 100;
				data->height_given = SGT_PIXEL;
				sendinfo = TRUE;
			}
			else if( data->height != data->iys )
			{
				data->height = data->iys;
				sendinfo = TRUE;
			}
			break;
	}

	if( data->ixs != data->width || data->iys != data->height )
	{
		data->scaled = TRUE;
	}

	data->maskbm = msg->maskbm;
	data->imglist = msg->imagelist;

	if( data->imglist && !ISLISTEMPTY( data->imglist ) )
		data->imf = FIRSTNODE( data->imglist );

	if( sendinfo )
	{
		if( data->ctx )
			DoMethod( data->ctx->dom_document, MM_HTMLView_NewImageSizes );
	}

	return( 0 );
}

static void scalebitmap( APTR obj, struct Data *data )
{
#ifndef MBX
	struct BitScaleArgs bsa, bsa2;
#endif
	struct BitMap *newbm;
	struct imgframenode *imf;
	int src_depth;
#ifndef MBX
	struct BitMap *src_friend;
#else
	IPTR src_pixfmt;
#endif

	imf = FIRSTNODE( data->imglist );
	if( !NEXTNODE( imf ) )
		return;

#ifndef MBX
	markclonemodified( data->bm );
	markclonemodified( imf->maskbm );
#endif

	src_depth = GetBitMapAttr( data->bm, BMA_DEPTH );
#ifndef MBX
	if( destscreen )
		src_friend = GetBitMapAttr( destscreen->sc_RastPort.rp_BitMap, BMA_FLAGS ) & BMF_INTERLEAVED ? NULL : destscreen->sc_RastPort.rp_BitMap;
	else
		src_friend = NULL;

	newbm = AllocBitMap(
		data->width + 1,
		data->height + 1,
		src_depth,
		BMF_MINPLANES,
		src_friend
	);

	if( !newbm )
		return;

	memset( &bsa, 0, sizeof( bsa ) );

	bsa.bsa_SrcWidth = imf->xs;
	bsa.bsa_SrcHeight = imf->ys;

	bsa.bsa_XSrcFactor = imf->xs;
	bsa.bsa_XDestFactor = data->width;

	bsa.bsa_YSrcFactor = imf->ys;
	bsa.bsa_YDestFactor = data->height;

	bsa2 = bsa;

	bsa.bsa_SrcBitMap = getclone( data->bm, TRUE );
	bsa.bsa_DestBitMap = newbm;

	BitMapScale( &bsa );

#else
	src_pixfmt = GetBitMapAttr( data->bm, BMAV_PixelFormat );

	newbm = AllocBitMap(
		src_pixfmt,
		data->width + 1,
		data->height + 1,
		0,
		NULL
	);

	if( !newbm )
		return;

	ScaleBitMap( data->bm, newbm );

#endif /* !MBX */

	data->bm = newbm;
	data->scaled_bm = newbm;

#ifndef MBX
	// mask, too?
	if( data->maskbm > 0 && data->maskbm != ( APTR )-1 )
	{
		newbm = AllocBitMap(
			data->width + 1,
			data->height + 1,
			1,
			0,
			NULL
		);
		if( !newbm )
			return;

		bsa2.bsa_SrcBitMap = getclone( data->maskbm, TRUE );
		bsa2.bsa_DestBitMap = newbm;

		BitMapScale( &bsa2 );

		data->maskbm = newbm;
		data->scaled_mask = newbm;

		data->fillbg = TRUE;
	}
#endif
}

DECTMETHOD( ImgDecode_Done )
{
	GETDATA;

	if( data->broken_image || data->done )
	{
		return( 0 );
	}

	if( data->maskbm && imgdec_isblank( data->img ) )
	{
		data->is_blank = TRUE;
		MUI_Redraw( obj, MADF_DRAWUPDATE );
		sendcheckdone( data );
		return( TRUE );
	}

	if( data->maskbm && !imgdec_maskused( data->img ) )
	{
		data->maskbm = NULL;
	}

	if( data->scaled )
	{
		// Scale bitmap
		scalebitmap( obj, data );
		MUI_Redraw( obj, MADF_DRAWOBJECT );
		sendcheckdone( data );
		return( 0 );
	}

	if( data->newpartialye < data->iys - 1 )
	{
		data->newpartialye = data->iys - 1;
		MUI_Redraw( obj, MADF_DRAWUPDATE );
	}

	if( data->imf && NEXTNODE( NEXTNODE( data->imf ) ) && !data->scaled )
	{
		data->play_anim = TRUE;
		DoMethod( obj, MM_Layout_Image_StartAnim );
	}

	if( !data->maskbm && !data->broken_image && !data->marginleft && !data->margintop )
	{
		data->li.flags |= LOF_BACKFILLING;
	}

	sendcheckdone( data );

	data->done = TRUE;

	return( 0 );
}

DECTMETHOD( Layout_Image_StartAnim )
{
	GETDATA;

	if( data->imf && !data->ihn_active )
	{
		data->ihn.ihn_Object = obj;
		data->ihn.ihn_Flags = MUIIHNF_TIMER;
		data->ihn.ihn_Millis = min( 65535, data->imf->delay * 10 );
		data->ihn.ihn_Method = MM_Layout_Image_DoAnim;
		data->ihn_active = TRUE;
		DoMethod( app, MUIM_Application_AddInputHandler, &data->ihn );
	}
	return( 0 );
}

DECTMETHOD( Layout_Image_StopAnim )
{
	GETDATA;

	if( data->ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, &data->ihn );
		data->ihn_active = FALSE;
		data->imf = FIRSTNODE( data->imglist );
		MUI_Redraw( obj, MADF_DRAWOBJECT );
	}
	return( 0 );
}

DECTMETHOD( Layout_Image_DoAnim )
{
	GETDATA;

	if( !data->ihn_active )
		return( 0 );

	data->imf = NEXTNODE( data->imf );

	if( !NEXTNODE( data->imf ) )
	{
		int v;

		data->repeatcnt++;

		v = imgdec_getrepeatcnt( data->img );
		if( v )
		{
			if( data->repeatcnt >= v )
			{
				if( data->ihn_active )
				{
					DoMethod( app, MUIM_Application_RemInputHandler, &data->ihn );
					data->ihn_active = FALSE;
					data->play_anim = FALSE;
					return( 0 );
				}
			}
		}

		data->imf = FIRSTNODE( data->imglist );
	}

	if( _isvisible( obj ) )
	{
		data->frameupdate = TRUE;
		MUI_Redraw( obj, MADF_DRAWUPDATE );
	}

	#ifndef __MORPHOS__
	if( MUIMasterBase->lib_Version < 20 ) DoMethod( app, MUIM_Application_RemInputHandler, &data->ihn );
	#endif
	data->ihn.ihn_Millis = min( 65535, data->imf->delay * 10 );
	#ifndef __MORPHOS__
	if( MUIMasterBase->lib_Version < 20 ) DoMethod( app, MUIM_Application_AddInputHandler, &data->ihn );
	#endif

	return( 0 );
}

DECSMETHOD( ImgDecode_GotScanline )
{
	GETDATA;

	if( data->scaled || !data->bm )
		return( 0 );

	markclonemodified( data->bm );

	data->newpartialys = msg->min_touched_y;
	data->newpartialye = msg->max_touched_y;

	MUI_Redraw( obj, MADF_DRAWUPDATE );

	return( 0 );
}

DECMMETHOD( Setup )
{
	ULONG rc;
	GETDATA;

	rc = DOSUPER;

#ifndef MBX
	destscreen = _screen( obj );
	dest_is_interleaved = GetBitMapAttr( _screen( obj )->RastPort.BitMap, BMA_FLAGS ) & BMF_INTERLEAVED;
#endif

	data->ehnode.ehn_Object = obj;
	data->ehnode.ehn_Class = cl;
	data->ehnode.ehn_Events = IDCMP_MOUSEBUTTONS | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW;
	data->ehnode.ehn_Priority = 1;
	data->ehnode.ehn_Flags = MUI_EHF_GUIMODE;
	DoMethod( _win( obj ), MUIM_Window_AddEventHandler, ( ULONG )&data->ehnode );

	return( rc );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	DoMethod( _win( obj ), MUIM_Window_RemEventHandler, ( ULONG )&data->ehnode );

	return( DOSUPER );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

DECSMETHOD( Layout_FormElement_ReportValue )
{
	GETDATA;
	char buffer[ 128 ], buffer2[ 64 ];

	// Only report my value when I was clicked
	if( msg->whichform != data->form || !data->meactive )
		return( 0 );

	if( getv( obj, MUIA_Disabled ) )
		return( 0 );

	if( !data->name )
		return( 0 );

	// Submit X position
	sprintf( buffer, "%.100s.x", data->name );
	sprintf( buffer2, "%d", data->me_x );

	DoMethod( data->form, MM_Layout_Form_AttachValue,
		buffer, buffer2,
		-1,
		NULL
	);

	// Submit Y position
	sprintf( buffer, "%.100s.y", data->name );
	sprintf( buffer2, "%d", data->me_y );

	DoMethod( data->form, MM_Layout_Form_AttachValue,
		buffer, buffer2,
		-1,
		NULL
	);

	return( 0 );
}

DECMMETHOD( Show )
{
	GETDATA;
	ULONG rc;

	rc = DOSUPER;

	data->mleft = _left( obj ) + data->marginleft;
	data->mright = _right( obj ) - data->marginright;
	data->mtop = _top( obj ) + data->margintop;
	data->mbottom = _bottom( obj ) - data->marginbottom;

	return( rc );
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
		APTR mstrip;
		/*
		 * We want to show up the imagelink over imagemaps too.
		 */
		mstrip = build_context_menu( ( getv( obj, MA_Layout_Image_Anchor ) || ( data->ismap || data->usemap && getv( obj, MA_Layout_Image_AnchorURL ) ) ) ? VMT_IMAGELINKMODE : VMT_IMAGEMODE, data->cmenu, &data->split );
		if( mstrip )
		{
			static char buffer[ 128 ];

			if( data->img )
			{
				char buffer2[ 64 ];
				struct MinList *ml;
				APTR mit, mit2;

				get( mstrip, MUIA_Family_List, &ml );
				mit2 = FIRSTNODE( ml );
				mit = NextObject( &mit2 );

				imgdec_getinfostring( data->img, buffer2 );
				sprintf( buffer, "%s (%s)", (char*)getv( mit, MUIA_Menu_Title ), buffer2 );
				set( mit, MUIA_Menu_Title, buffer );
			}
		}
		return( (ULONG)mstrip );
	}
#endif

	displaybeep();

	return( 0 );
}

DECMMETHOD( ContextMenuChoice )
{
#if USE_MENUS
	GETDATA;
#endif

	rexx_obj = obj;

#if USE_MENUS
	if( getv( obj, MA_Layout_Image_Anchor ) && muiUserData( msg->item ) > data->split )
	{
		data->cmenu_anchor = TRUE; /* to know if the action is to be done on the link or on the image */
		execute_command( getprefslong( DSI_CMENUS_LINK_ACTION + muiUserData( msg->item ) - data->split, 0 ), getprefsstr( DSI_CMENUS_LINK_ARGS + muiUserData( msg->item ) - data->split, "" ), VREXX_FRAME, ( STRPTR )data->url, ( STRPTR )getv( data->anchor, MA_Layout_Anchor_URL ), ( STRPTR )getv( data->ctx->dom_win, MA_JS_Window_URL ) );
	}
	else
	{
		data->cmenu_anchor = FALSE;
		execute_command( getprefslong( DSI_CMENUS_IMAGE_ACTION + muiUserData( msg->item ), 0 ), getprefsstr( DSI_CMENUS_IMAGE_ARGS + muiUserData( msg->item ), "" ), VREXX_FRAME, ( STRPTR )data->url, ( STRPTR )getv( data->anchor, MA_Layout_Anchor_URL ), ( STRPTR )getv( data->ctx->dom_win, MA_JS_Window_URL ) );
	}
#endif /* USE_MENUS */

	return( 0 );
}

DECSMETHOD( Layout_CalcUnloadedObjects )
{
	GETDATA;
	if( data->img )
	{
		if( !imgdec_isdone( data->img ) )
		{
			*msg->cnt_img = *msg->cnt_img + 1;
		}
	}
	return( 0 );
}

DECTMETHOD( HTMLView_AbortLoad )
{
	GETDATA;

	if( data->img )
		imgdec_abortload( data->img );

	return( 0 );
}

BEGINPTABLE
DPROP( onclick,  	funcptr )
DPROP( onmouseover,	funcptr )
DPROP( onmouseout,	funcptr )
DPROP( border, 		real )
DPROP( complete,	bool )
DPROP( height,		real )
DPROP( hspace,		real )
DPROP( src,			string )
DPROP( lowsrc,		string )
DPROP( vspace,		real )
DPROP( width,		real )
ENDPTABLE

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DECSMETHOD( JS_GetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_border:
			storeintprop( msg, data->border );
			return( TRUE );

		case JSPID_hspace:
			storeintprop( msg, data->marginright );
			return( TRUE );

		case JSPID_vspace:
			storeintprop( msg, data->margintop );
			return( TRUE );

		case JSPID_width:
			storeintprop( msg, data->width );
			return( TRUE );

		case JSPID_height:
			storeintprop( msg, data->height );
			return( TRUE );

		case JSPID_complete:
			storeintprop( msg, data->done );
			return( TRUE );

		case JSPID_src:
			storestrprop( msg, data->url );
			return( TRUE );

		case JSPID_lowsrc:
			storestrprop( msg, data->lowsrcurl );
			return( TRUE );
	}
	return( DOSUPER );
}

DECSMETHOD( JS_SetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_onclick:
			data->ix_onclick = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onmouseover:
			data->ix_mouseover = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onmouseout:
			data->ix_mouseout = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_src:
			{
				char newurl[ MAXURLSIZE ];
				uri_mergeurl( data->ctx ? data->ctx->baseref : (char*)getv( obj, MA_JS_Object_Baseref ), msg->dataptr, newurl );
				set( obj, MA_Layout_Image_Src, newurl );
			}
			return( TRUE );

		case JSPID_lowsrc:
			{
				char newurl[ MAXURLSIZE ];
				uri_mergeurl( data->ctx ? data->ctx->baseref : (char*)getv( obj, MA_JS_Object_Baseref ), msg->dataptr, newurl );
				set( obj, MA_Layout_Image_LowSrc, newurl );
			}
			return( TRUE );
	}

	return( DOSUPER );
}

/*
 * The following methods can be forwarded to the HTMLView object
 * when needed.
 */

DECMETHOD( HTMLWin_Backward, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLWin_Backward ) );
	}
	else
	{
		return( FALSE );
	}
}

DECMETHOD( HTMLWin_Forward, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLWin_Forward ) );
	}
	else
	{
		return( FALSE );
	}
}

DECMETHOD( HTMLRexx_LoadBG, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLRexx_LoadBG ) );
	}
	else
	{
		return( FALSE );
	}
}

DECSMETHOD( HTMLRexx_SetURLFromObject )
{
	GETDATA;

	/* link */
	if( data->cmenu_anchor )
	{
		//same TOFIX!! as below	:)
		if( msg->newwin )
		{
			win_create( "", ( STRPTR )getv( data->anchor, MA_Layout_Anchor_URL ), "", NULL, FALSE, msg->reload, FALSE );
			return( TRUE );
		}
		else
		{
			//TOFIX!!
			return( FALSE );
		}
	}

	/* image */
	if( data->url )
	{
		if( msg->newwin )
		{
			//TOFIX!! find a way to fetch the referer maybe
			win_create( "", data->url, "", NULL, FALSE, msg->reload, FALSE );
			return( TRUE );
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

	/* link */
	if( data->cmenu_anchor )
	{
		return( (ULONG)storetoclip( ( STRPTR )getv( data->anchor, MA_Layout_Anchor_URL ) ) );
	}

	/* image */
	if( data->url )
	{
		return( (ULONG)storetoclip( data->url ) );
	}

	return( FALSE );
}

DECMETHOD( HTMLRexx_OpenSourceView, ULONG )
{
	GETDATA;

	if( data->ctx && data->ctx->dom_document )
	{
		return( DoMethod( data->ctx->dom_document, MM_HTMLRexx_OpenSourceView ) );
	}
	else
	{
		return( FALSE );
	}
}

DECSMETHOD( HTMLRexx_OpenDocInfo )
{
	GETDATA;

	if( msg->url )
	{
		if( data->ctx && data->ctx->dom_document )
		{
			return( DoMethod( data->ctx->dom_document, MM_HTMLRexx_OpenDocInfo, msg->url ) );
		}
		else
		{
			return( FALSE );
		}
	}
	else
	{
		if( data->url )
		{
			return( ( ULONG )createdocinfowin( data->url ) );
		}
		else
		{
			return( FALSE );
		}
	}
}

DECSMETHOD( HTMLRexx_SaveURL )
{
	GETDATA;

#if USE_NET

	if( msg->url )
	{
		queue_download( msg->url, NULL, TRUE, msg->ask );//TOFIX!! fix the name which is blatantly ignored atm
		return( TRUE );
	}
	else
	{
		/* link */
		if( data->cmenu_anchor )
		{
			queue_download( ( STRPTR )getv( data->anchor, MA_Layout_Anchor_URL ), NULL, TRUE, msg->ask ); // name here too ?
			return( TRUE );
		}

		if( data->url )
		{
			queue_download( data->url, NULL, TRUE, msg->ask );//TOFIX!! fix the name which is blatantly ignored atm
			//TOFIX!! find the referer
			return( TRUE );
		}
	}
#endif /* USE_NET */
	return( FALSE );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSET
DEFGET
DEFDISPOSE
DEFTMETHOD( Layout_Image_LoadImage )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_CalcUnloadedObjects )
DEFTMETHOD( ImgDecode_Done )
DEFSMETHOD( ImgDecode_HasInfo )
DEFSMETHOD( ImgDecode_GotScanline )
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( Draw )
DEFMMETHOD( Show )
DEFMMETHOD( AskMinMax )
DEFMMETHOD( ContextMenuBuild )
DEFMMETHOD( ContextMenuChoice )
DEFMMETHOD( HandleEvent )
DEFTMETHOD( Layout_Image_StopAnim )
DEFTMETHOD( Layout_Image_StartAnim )
DEFTMETHOD( Layout_Image_DoAnim )
DEFTMETHOD( Layout_FormElement_ReportValue )
DEFTMETHOD( HTMLView_AbortLoad )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
DEFMETHOD( HTMLWin_Backward )
DEFMETHOD( HTMLWin_Forward )
DEFMETHOD( HTMLRexx_LoadBG )
DEFSMETHOD( HTMLRexx_SetURLFromObject )
DEFMETHOD( HTMLRexx_OpenSourceView )
DEFSMETHOD( HTMLRexx_OpenDocInfo )
DEFSMETHOD( HTMLRexx_SaveURL )
DEFTMETHOD( HTMLRexx_SetClipFromObject )
ENDMTABLE

int create_loimageclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loimageClass";
#endif

	return( TRUE );
}

void delete_loimageclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloimageclass( void )
{
	return( lcc->mcc_Class );
}



