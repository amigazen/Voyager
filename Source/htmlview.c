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
** This class visually embeds a HTML document
** and manages network operations
**
** $Id: htmlview.c,v 1.263 2004/06/09 22:50:32 zapek Exp $
**
*/
#define WHEEL_MOUSE_IS_FUCKED 0

/* XXX: I disabled smooth mode temporarily */
#define USE_SMOOTH_SCROLLING 0

#ifdef MBX
#include <modules/mui/features.h> /* HACK... sigh */
#endif

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/graphics.h>
#include <graphics/gfxmacros.h>
#include <proto/layers.h>
#endif

#include "classes.h"
#include "prefs.h"
#include "network.h"
#include "voyager_cat.h"
#if USE_PLUGINS
#include "plugins.h"
#endif
#include "htmlclasses.h"
#include "layout.h"
#ifndef MBX
#include "bitmapclone.h"
#else
#include <modules/mui/features.h>
#include <modules/mui/mui.h>
#endif
#include "classes.h"
#include "htmlclasses.h"
#include "malloc.h"
#include "mui_func.h"
#include <proto/vimgdecode.h>
#include "methodstack.h"
#include "js.h"
#include "sourceview.h"
#include "docinfowin.h"
#include "download.h"
#include "cookies.h"
#include "clip.h"
#include "win_func.h"
#ifndef __MORPHOS__
#include "newmouse.h"
#endif
#include "urlparser.h"

/*
	The following is internal MUI data
	The horror, the horror
*/

#ifdef __MORPHOS__
#pragma pack(2)
#endif /* __MORPHOS__ */

#define H    0
#define V    1
#define DIMS 2

struct LongRect
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
};

struct VirtgroupData
{
	APTR clip;						/* 0x00 */
	Object *scrollgroup;					/* 0x04 */

	LONG pos[DIMS];						/* 0x08 */
	LONG size[DIMS];					/* 0x10 */
	LONG oldpos[DIMS];					/* 0x18 */
	BYTE free[DIMS];					/* 0x20 */
	UWORD  Pad1;						/* 0x22 */
	LONG mx;						/* 0x24 */
	LONG my;						/* 0x28 */

	BOOL clipdraw;						/* 0x2c */
	BOOL input;						/* 0x2e */

	struct MUI_EventHandlerNode eh;				/* 0x30 */
	LONG imode;						/* 0x48 */

	#if defined(USE_SAMPLEGAG) || defined(USE_ABOUTMUIGAG)
	int PageID;						/* 0x4c */
	int SubPageID;						/* 0x50 */
	#endif

	#ifdef USE_SPECIALMIDMOVE
	struct MUI_InputHandlerNode midmove;			/* 0x50 */
	int midmove_active;					/* 0x68 */
	struct BitMap *pointerbitmap;				/* 0x6c */
	Object *pointerobj;					/* 0x70 */
	int pointerid;						/* 0x74 */
	ULONG mb_lastsecs;					/* 0x78 */
	ULONG mb_lastmics;					/* 0x7c */
	#endif
};								/* 0x80 */

struct VirtgroupData38
{
	APTR clip;
	Object *scrollgroup;

	LONG pos[DIMS];
	LONG size[DIMS];
	LONG oldpos[DIMS];
	BYTE free[DIMS];

	LONG mx;
	LONG my;

	/*
	#ifdef USE_HTMLGAG
	ULONG gagtype;
	#define MUISpecialBase (data->muispecialbase)
	struct Library *muispecialbase;
	struct Gag *gag;
	struct MUI_EventHandlerNode ehn;
	LONG gagleft;
	LONG gagtop;
	#endif
	*/

	unsigned clipdraw:1,
	         moving1:1,
	         input:1,
	         reqticks:1,
	         moving2:1,
				#ifdef USE_VIRTGROUPID
				PageID:8,
				SubPageID:8,
				#endif
				dummy:1;

	#ifdef USE_SPECIALMIDMOVE
	struct MUI_InputHandlerNode midmove;
	#endif
};

struct GroupData
{
	struct MinList ChildList; // muß an erster Stelle sein!	/* 0x00 */
	ULONG gidcmp;						/* 0x0c */
	struct BresenhamInfo *binfo[DIMS];			/* 0x10 */
	WORD dim[DIMS];						/* 0x18 */
	BYTE sspace[DIMS];					/* 0x1c */
	BYTE space[DIMS];					/* 0x1e */
	BYTE samedim[DIMS];					/* 0x20 */
	BYTE activepage;					/* 0x22 */
	BYTE change_count;					/* 0x23 */

	struct Hook *LayoutHook;				/* 0x24 */


	unsigned long						/* 0x28 */
				gtype        :2,
				pagemode     :1,
				freemode     :1,
				regiondraw   :1,
				pagemax      :1,
				stopget      :1,
				virtual      :1,
				hvirt        :1,
				vvirt        :1,
				virtdraw     :1,
				nochange     :1,
				unevenchilds :1,		/* 13 Bits */

				pad : 19;			/* to force sas to make an ULONG instead of an UWORD..sick */
};								/* 0x2c */

#ifdef __MORPHOS__
#pragma pack()
#endif /* __MORPHOS__ */

#define overlap(l1,t1,r1,b1,l2,t2,r2,b2) ( (r1)>=(l2) && (r2)>=(l1) && (b1)>=(t2) && (b2)>=(t1) )

LONG MUIG_ScrollRaster(struct RastPort *rp,WORD dx,WORD dy,WORD left,WORD top,WORD right,WORD bottom);
#if !defined( MBX ) && !defined( __MORPHOS__ )
extern struct Library *MUIGfxBase;
#pragma libcall MUIGfxBase MUIG_ScrollRaster f0 543210807
#endif

#ifdef __MORPHOS__
extern struct Library *MUIGfxBase;
#define MUIG_ScrollRaster(rp, dx, dy, left, top, right, bottom) \
	LP7(0xf0, LONG, MUIG_ScrollRaster, struct RastPort *, rp, a0, WORD, dx, d0, WORD, dy, d1, WORD, left, d2, WORD, top, d3, WORD, right, d4, WORD, bottom, d5, \
	, MUIGfxBase, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)
#endif /* __MORPHOS__ */

//
// Incremental layout layer/bitmap cache
//

static struct Layer_Info *layerinfo;
static struct Layer *layer;
static int oldlayer_w, oldlayer_h;
static struct BitMap *oldfriendbitmap;
static struct BitMap *oldbitmap;
static int oldbitmap_w, oldbitmap_h;
static int cachebminuse;

static struct BitMap *get_cachebitmap( struct BitMap *friend, int w, int h, struct RastPort **rp )
{
	if( cachebminuse )
	{
		#warning why it happens with only one window upon opaqueresize.. could you have a look, Olli ?
		//dprintf( "in use! aborting\n" );
		return( NULL );
	}

	if( oldfriendbitmap != friend || w != oldlayer_w || h != oldlayer_h )
	{
		if( layer )
		{
			DeleteLayer( NULL, layer );
			layer = NULL;
		}
		if( layerinfo )
		{
			DisposeLayerInfo( layerinfo );
			layerinfo = NULL;
		}
	}

	if( oldfriendbitmap != friend || w > oldbitmap_w || h > oldbitmap_h )
	{
		if( oldbitmap )
		{
			FreeBitMap( oldbitmap );
			oldbitmap = NULL;
		}
	}

	if( !oldbitmap )
	{
		oldbitmap = AllocBitMap( w, h, GetBitMapAttr( friend, BMA_DEPTH ), BMF_MINPLANES | BMF_DISPLAYABLE, friend );
		if( !oldbitmap )
		{
			return( NULL );
		}
		oldbitmap_w = w;
		oldbitmap_h = h;
		oldfriendbitmap = friend;
	}

	if( !layer )
	{
		layerinfo = NewLayerInfo();
		if( !layerinfo )
			return( NULL );
		layer = CreateUpfrontHookLayer( layerinfo, oldbitmap, 0, 0, w - 1, h - 1, LAYERSIMPLE, LAYERS_NOBACKFILL, NULL );
		if( !layer )
			return( NULL );
		oldlayer_w = w;
		oldlayer_h = h;
	}

	cachebminuse = 1;

	*rp = layer->rp;
	return( oldbitmap );
}

void cleanup_cachebitmap( void )
{
	if( layer )
		DeleteLayer( NULL, layer );
	if( oldbitmap )
		FreeBitMap( oldbitmap );
	if( layerinfo )
		DisposeLayerInfo( layerinfo );

	cachebminuse = FALSE;
}

struct Data {
	int xsize, ysize;
	ULONG xpos, ypos; /* to check if the user moved when we set the virtgroup position */
	int firstlayout;  /* ditto */
	struct nstream *doc;
	int lastoffset;
	int finished;
	int has_anchor, anchor_found;
	char anchorname[ 128 ];

	APTR toplevelwin; // To send informs to etc..
	APTR htmlwin;

	int virtual_width;
	int is_frameset;

	// Various state information in the course of layout
	int gottitle;

	struct layout_ctx *lctx;
	struct ColorMap *lastcmap;

	struct MUI_InputHandlerNode ihn;
	int ihn_active;
	int ihn_period;

	// Image container mode
	int imagemode;
	int textmode;
	int in_layout; // For document.write(), and stuff

	struct imgclient *img; /* holds an imageclient in imagemode display (TOFIX: except plugins) */

	// Incremental refresh
	int layout_width, layout_height;
	#if USE_DBUF_RESIZE
	ULONG last_xs;
	ULONG last_ys;
	#endif
	int usedamageclip;

	// JS stuff
	struct MinList cpl;
	// for document write, push buffer
	struct MinList pushbuffer;
	ULONG gcmagic;
	APTR js_pips;
	APTR js_images;
	APTR js_forms;
	APTR js_links;
	APTR js_embeds;
	int ix_onload, ix_onunload;
#ifdef MBX
	int ix_ondiskchange;
#endif
	int js_isopen;	// Document.Open() called from JS code
	char *domain;	// document.domain stuff

#if USE_STB_NAV
	int crossdirs; // Scrollable directions
#endif

	struct MUI_EventHandlerNode ehn;
};

// This is used to hold fragments send by document.write(ln)
struct pushnode {
	struct MinNode n;
	char str[ 0 ]; // to be extended
};


static struct MUI_CustomClass *mcc;

MUI_HOOK( layoutfunc, APTR grp, struct MUI_LayoutMsg *lm )
{
	struct Data *data = INST_DATA( mcc->mcc_Class, grp );

	D( db_html, bug( "MUIM_Layout(%lx), action = %ld\n", grp, lm->lm_Type ));

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
				int maxx = 0, maxy = 0;

				if( data->virtual_width > 0 && !data->is_frameset )
					lm->lm_Layout.Width = data->virtual_width;

				while (child = NextObject(&cstate))
				{
					D( db_html, bug( "sending MM_Layout_CalcMinMax to child 0x%lx\n", child ) );
					DoMethod( child, MM_Layout_CalcMinMax, lm->lm_Layout.Width, lm->lm_Layout.Height, lm->lm_Layout.Width );
				}

				cstate = (Object *)lm->lm_Children->mlh_Head;
				child = NextObject( &cstate );

				// Layout dummy object
				MUI_Layout( child, 0, 0, 1, 1, 0 );

				while (child = NextObject(&cstate))
				{
					struct layout_info *li;

					get( child, MA_Layout_Info, &li );

					DoMethod( child, MM_Layout_DoLayout, max( lm->lm_Layout.Width, li->minwidth ), max( lm->lm_Layout.Height, li->minheight ), lm->lm_Layout.Width );

					li->ys = max( lm->lm_Layout.Height, li->ys );
					li->xs = max( lm->lm_Layout.Width, li->xs );

					MUI_Layout( child, li->xp, li->xp, min( 8191, li->xs ), min( 8192, li->ys ), 0 );

					maxx = max( li->xp + li->xs, maxx );
					maxy = max( li->yp + li->ys, maxy );
				}

				data->layout_height = lm->lm_Layout.Height = max( lm->lm_Layout.Height, maxy );
				data->layout_width = lm->lm_Layout.Width = max( lm->lm_Layout.Width, maxx );

				D( db_html, bug( "MUIM_Layout(%lx), final size %ld/%ld\n", grp, lm->lm_Layout.Width, lm->lm_Layout.Height ));
			}
			return( TRUE );
	}

	return( MUILM_UNKNOWN );
}

static int doset( APTR obj, struct Data *data, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;
#if USE_STB_NAV
	int crossupdate = FALSE;
#endif

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_HTMLView_TopLevelWin:
			data->toplevelwin = (APTR)tag->ti_Data;
			break;

		case MA_HTMLView_HTMLWin:
			data->htmlwin = (APTR)tag->ti_Data;
			break;

		case MA_HTMLView_InLayout:
			data->in_layout = (int)tag->ti_Data;
			break;

		case MA_HTMLWin_VirtualWidth:
			data->virtual_width = (int)tag->ti_Data;
			break;

		case MA_HTMLView_IsFrameset:
			data->is_frameset = (int)tag->ti_Data;
			break;


#if USE_STB_NAV
		case MUIA_Virtgroup_Left:
			crossupdate = TRUE;
			break;

		case MUIA_Virtgroup_Top:
			crossupdate = TRUE;
			break;
#endif
	}

#if USE_STB_NAV
	if( crossupdate )
	{
		pushmethod( obj, 2, MM_Cross_SetDir, NULL );
	}
#endif

	return( redraw );
}

DECSET
{
	GETDATA;

	doset( obj, data, msg->ops_AttrList );

	return( DOSUPER );
}

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		MUIA_CustomBackfill, TRUE,
		MUIA_Group_LayoutHook, (ULONG)&layoutfunc_hook,
		#if USE_SMOOTH_SCROLLING
		MUIA_Virtgroup_Smooth, gp_smooth_scroll,
		#endif
		Child, NewObject( getlodummyclass(), NULL, TAG_DONE ),
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	D( db_html, bug( "new HTMLView object 0x%lx\n", obj ) );

	data = INST_DATA( cl, obj );

	NEWLIST( &data->pushbuffer );
	NEWLIST( &data->cpl );

	data->domain = NULL;

	data->gcmagic = -1UL;

	doset( obj, data, msg->ops_AttrList );

	_flags( obj ) |= MADF_KNOWSACTIVE;

	return( (ULONG)obj );
}

DECDISPOSE
{
	GETDATA;

	if( data->img )
	{
		imgdec_close( data->img );
		data->img = NULL;
	}

	if( data->lctx )
		layout_delete( data->lctx );

	if( data->ihn_active )
		DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );

	if( data->domain )
		free(data->domain);

	cp_flush( &data->cpl );

	D( db_html, bug( "HTMLView object 0x%lx gone\n", obj ) );

	return( DOSUPER );
}

DECSMETHOD( HTMLView_ShowNStream )
{
	GETDATA;

	/*
	 * Cleanup possible images which were displayed in
	 * imagemode (full window).
	 */
	if( data->img )
	{
		imgdec_close( data->img );
		data->img = NULL;
	}

	// Is there an onUnload event handler?
	if( data->ix_onunload )
	{
		// Check for body onunload event handlers
		DoMethod( data->htmlwin, MM_HTMLWin_ExecuteEvent, jse_unload, data->ix_onunload, obj,
			TAG_DONE
		);
	}

	if( data->doc )
	{
		if( data->lctx->cache_no )
			nets_release_buffer( data->doc );
	}

	cp_flush( &data->cpl );

	data->ix_onload = 0;
	data->ix_onunload = 0;
#ifdef MBX
	data->ix_ondiskchange = 0;
#endif

	/*
	 * Store current document's scroll position
	 */
	if( data->doc )
	{
		DoMethod( data->htmlwin, MM_Historylist_StoreXY, nets_url( data->doc ), getv( obj, MUIA_Virtgroup_Left ), getv( obj, MUIA_Virtgroup_Top ) );
	}

	// Store form element values
	DoMethod( obj, MM_Layout_FormElement_Store );

	DoMethod( data->htmlwin, MUIM_Group_InitChange );

	if( data->lctx )
	{
		layout_delete( data->lctx );
		data->lctx = NULL;

	}

	if( data->ihn_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );
		data->ihn_active = FALSE;
	}

	// TOFIX: remove the following. after being sure it wasn't used as a workaround for something ( I think the whole can be removed anyway )
	//DoMethod( obj, MUIM_Group_InitChange );
	//SetAttrs( obj,
	//	  MUIA_Virtgroup_Top, 0,
	//	  MUIA_Virtgroup_Left, 0,
	//	  TAG_DONE
	//);
	//DoMethod( obj, MUIM_Group_ExitChange );


	data->doc = msg->ns;
	data->lastoffset = 0;
	data->finished = FALSE;
	data->gottitle = FALSE;

	data->lctx = layout_new();

	if( data->lastcmap )
		layout_setuppens( data->lctx, data->lastcmap );

	layout_attach( data->lctx, obj );
	layout_setdom( data->lctx, data->htmlwin, obj );

	layout_setmargins( data->lctx,
		getv( data->htmlwin, MA_Layout_MarginLeft ),
		getv( data->htmlwin, MA_Layout_MarginRight ),
		getv( data->htmlwin, MA_Layout_MarginTop ),
		getv( data->htmlwin, MA_Layout_MarginBottom )
	);

	// Run the garbage collector
	js_gc();

#if USE_LO_PIP
	DoMethod( data->htmlwin, MM_HTMLWin_ResetPIPNum );
#endif

	if( data->doc )
	{
		char *p;
#if USE_LIBUNICODE
		data->lctx->iso_charset = charset_to_iso_code( nets_mimetype( data->doc ) );
#endif
		layout_setbaseref( data->lctx, nets_url( data->doc ) );

		p = strrchr( nets_fullurl( data->doc ), '#' );
		if( p )
		{
			stccpy( data->anchorname, p + 1, sizeof( data->anchorname ) );
			data->has_anchor = TRUE;
			data->anchor_found = FALSE;
		}

		if( !strnicmp( nets_mimetype( data->doc ), "image/", 6 ) )
		{
			/*
			 * generate a "fake HTML" page to display the picture
			 */
			char buffer[ MAXURLSIZE ];

			sprintf( buffer, "<IMG SRC=\"%s\">", nets_url( data->doc ) );

			data->img = imgdec_open( nets_url( data->doc ), data->htmlwin, nets_url( data->doc ), FALSE );
			set( data->htmlwin, MA_HTMLWin_Imgclient, data->img );
			layout_do( data->lctx, buffer, strlen( buffer ), 0, TRUE );

			DoMethod( data->htmlwin, MM_HTMLWin_CheckDone );

			//if( img )
			//	  imgdec_close( img );


			data->imagemode = 1;
		}
#if USE_PLUGINS
		else if( plugin_mimetype( nets_mimetype( data->doc ), nets_mimeextension( data->doc ) ) )
		{
			char buffer[ MAXURLSIZE ];
			sprintf( buffer, "<embed src=\"%s\" loop=\"false\" type=\"%s\" width=100%% height=100%%>", nets_url( data->doc ), nets_mimetype( data->doc ) );
			layout_do( data->lctx, buffer, strlen( buffer ), 0, TRUE );

			DoMethod( data->htmlwin, MM_HTMLWin_CheckDone );

			data->imagemode = 1;
		}
#endif /* USE_PLUGINS */
		else if( !strnicmp( nets_mimetype( data->doc ), "text/plain", 10 ) )
		{
			data->imagemode = 0;
			data->textmode = 1;
			DoMethod( obj, MM_NStream_GotData );
		}
		else
		{
			// set the document.domain property initially
			struct parsedurl purl;
			char *uri;

			uri = malloc( strlen( nets_url( data->doc ) ) + 1 );

			if (uri) // if this allocation fails, document.domain is fucked.
			{
				strcpy(uri, nets_url( data->doc ));
				uri_split( uri , &purl );

				free(data->domain);
				data->domain = NULL;

				if (purl.host) // URLs like "about:" wouldn't have a host :/
				{
					data->domain = malloc(strlen(purl.host) + 1);
					if (data->domain)	// we hope always TRUE
						strcpy(data->domain, purl.host);
				}

				free(uri);	// don't need this anymore
			}

			// .. and then carry on..
			data->imagemode = 0;
			data->textmode = 0;
			DoMethod( obj, MM_NStream_GotData );
		}
	}

	data->usedamageclip = FALSE;
	DoMethod( data->htmlwin, MUIM_Group_ExitChange );

	return( 0 );
}

DECTMETHOD( NStream_Done )
{
	GETDATA;
	ULONG x, y;

	/*
	 * This is a bit stupid but MM_NStream_Done and MM_NStream_GotData
	 * use a lot of common things so..
	 */
	DoMethod( obj, MM_NStream_GotData );

	/*
	 * Check if the user moved first
	 */
	if( getv( obj, MUIA_Virtgroup_Left ) == data->xpos && getv( obj, MUIA_Virtgroup_Top ) == data->ypos )
	{
	    if( DoMethod( data->htmlwin, MM_Historylist_GetXY, nets_url( data->doc ), &x, &y ) )
		{
			#if USE_SMOOTH_SCROLLING
			set_smooth_scroll( obj, FALSE );
			#endif
			SetAttrs( obj,
				MUIA_Virtgroup_Top, y,
				MUIA_Virtgroup_Left, x,
				TAG_DONE
			);
			#if USE_SMOOTH_SCROLLING
			set_smooth_scroll( obj, TRUE );
			#endif
		}
	}

	data->firstlayout = FALSE;

#if USE_LO_PIP
	{
		if( muiRenderInfo( obj ) && _win( obj ) )
		{
			APTR root;
			get( _win( obj ), MUIA_Window_RootObject, &root );
			DoMethod( root, MM_HTMLWin_CheckPIPs );
		}
	}
#endif

	return( 0 );
}


DECTMETHOD( NStream_GotData )
{
	GETDATA;
	int newoffset;
	int finished;
	// Parse new incoming data

	if( !data->doc || data->imagemode )
		return( 0 );

	newoffset = nets_getdocptr( data->doc );
	finished = nets_state( data->doc );

	if( newoffset > data->lastoffset )
	{
		nets_lockdocmem();
		if( !finished && data->lastoffset > 1024 && newoffset - data->lastoffset < 16384 )
		{
			// No need to relayout just yet
			nets_unlockdocmem();
			return( 0 );
		}

		if( !data->lastoffset && data->textmode && ( nets_sourceid( data->doc ) != 1 ) ) /* TOFIX: sux */
		{
			char *ptr;
			// Apply some guessing whether it's not really HTML we're looking at after all
			ptr = nets_getdocmem( data->doc );
			if( ptr )
			{
				char *srch = stpblk( ptr );
				if( !strnicmp( srch, "<HTML>", 6 ) || !strnicmp( srch, "<!DOCTYPE", 10 ) )
					data->textmode = 0;
			}
		}

		DoMethod( data->htmlwin, MUIM_Group_InitChange );
		if( data->textmode )
			data->lastoffset = layout_do_text( data->lctx, nets_getdocmem( data->doc ), newoffset, data->lastoffset, finished );
		else
			data->lastoffset = layout_do( data->lctx, nets_getdocmem( data->doc ), newoffset, data->lastoffset, finished );
		nets_unlockdocmem();

		// Did we get the document title?
		if( !data->gottitle && data->lctx->title[ 0 ] )
		{
			data->gottitle = TRUE;
			DoMethod( data->htmlwin, MM_HTMLWin_SetTitle, ( ULONG )data->lctx->title );
		}

		data->usedamageclip = TRUE;

		DoMethod( data->htmlwin, MUIM_Group_ExitChange );

		/*
		 * Record that the first layout is done
		 * and set the X/Y position of the virtgroup
		 * so that we can record if the user changes it.
		 */
		if( !data->firstlayout )
		{
			data->firstlayout = TRUE;
			data->xpos = getv( obj, MUIA_Virtgroup_Left );
			data->ypos = getv( obj, MUIA_Virtgroup_Top );
		}

		if( data->has_anchor && !data->anchor_found )
		{
			int ypos = 0;

			DoMethod( obj, MM_Layout_Anchor_FindByName, data->anchorname, &ypos );
			if( ypos > 0 )
			{
				#if USE_SMOOTH_SCROLLING
				set_smooth_scroll( obj, FALSE );
				#endif
				set( obj, MUIA_Virtgroup_Top, ypos );
				#if USE_SMOOTH_SCROLLING
				set_smooth_scroll( obj, TRUE );
				#endif
				data->anchor_found = TRUE;
			}
		}

		if( finished && layout_checkfetchnodes( data->lctx ) )
			finished = FALSE;
	}

	if( finished && !data->finished )
	{
		// Check for META Refresh tags
		if( data->lctx->meta_refresh_period >= 0 && !gp_metarefresh )
		{
			data->ihn.ihn_Object = obj;
			data->ihn.ihn_Flags = MUIIHNF_TIMER;
			data->ihn.ihn_Millis = 1000;
			data->ihn.ihn_Method = MM_HTMLView_RefreshTrigger;
			data->ihn_active = TRUE;
			data->ihn_period = data->lctx->meta_refresh_period;
			DoMethod( app, MUIM_Application_AddInputHandler, &data->ihn );
		}

		DoMethod( data->htmlwin, MM_HTMLWin_CheckDone );

		// Check for body onload event handlers
		if( data->ix_onload )
		{
			DoMethod( data->htmlwin, MM_HTMLWin_ExecuteEvent, jse_load, data->ix_onload, obj,
				TAG_DONE
			);
		}

		// Run the garbage collector
		js_gc();

		data->finished = TRUE;


#if USE_STB_NAV
		DoMethod( obj, MM_Cross_SetDir, NULL );
#endif

	}

	return( 0 );
}

DECTMETHOD( HTMLView_RefreshTrigger )
{
	GETDATA;

	if( data->ihn_active )
	{
		if( --data->ihn_period <= 0 )
		{
			DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );
			data->ihn_active = FALSE;
			if( data->lctx->meta_refresh_url )
			{
				DoMethod( data->htmlwin,
					MM_HTMLWin_SetURL,
					data->lctx->meta_refresh_url,
					data->lctx->baseref,
					NULL,
					MF_HTMLWin_Reload | MF_HTMLWin_AddURL
				);
			}
			else
			{
				pushmethod( data->htmlwin, 1, MM_HTMLWin_Reload );
			}
		}
	}
	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	// since we're doing our own layout, we don't want
	// mui to fiddle with that. We still need Area class'
	// method to get the minmaxinfo
	DOSUPER;
	msg->MinMaxInfo->MinWidth = 1;
	msg->MinMaxInfo->MinHeight = 1;
//	CoerceMethodA( cl->cl_Super->cl_Super, obj, msg );

	return( 0 );
}

DECMMETHOD( Setup )
{
	ULONG rc;
	GETDATA;

	rc = DOSUPER;

#ifndef MBX
	data->lastcmap = _screen( obj )->sc_ViewPort.vp_ColorMap;
#endif

	if( data->lctx )
		layout_setuppens( data->lctx, data->lastcmap  );

	if( muiRenderInfo( obj )->mri_Screen )
	{
#if USE_CGX
		// Hack, Kludge
		iscybermap = imgdec_setdestscreen( destscreen = muiRenderInfo( obj )->mri_Screen, 0, 0, 0, 0 );
#else
		imgdec_setdestscreen( muiRenderInfo( obj )->mri_Screen, 0, 0, 0, 0 );
#endif
	}

	data->ehn.ehn_Class  = cl;
	data->ehn.ehn_Object = obj;
	data->ehn.ehn_Events = IDCMP_RAWKEY;
	data->ehn.ehn_Flags  = MUI_EHF_GUIMODE;
	#if WHEEL_MOUSE_IS_FUCKED
	data->ehn.ehn_Priority = 6;
	#else
	data->ehn.ehn_Priority = -4;
	#endif
	DoMethod(_win(obj),MUIM_Window_AddEventHandler,(ULONG) &data->ehn);

	return( rc );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	DoMethod(_win(obj),MUIM_Window_RemEventHandler,(ULONG) &data->ehn);

	if( data->lctx )
	{
		layout_freepens( data->lctx );
	}

	return( DOSUPER );
}

DECMMETHOD( Draw )
{
	GETDATA;
	int redraw_done = FALSE;
	ULONG V_GroupDraw( APTR obj, struct IClass *cl, struct MUIP_Draw *msg );
	LONG w,h;
	LONG left = _left(obj);
	LONG top = _top(obj);

	w = left + _width( obj );
	h = top + _height( obj );
	// EVIL! EVIL! 666! ( well, evil is good )
	#if USE_DBUF_RESIZE
	if( ( data->last_xs != w ) || ( data->last_ys != h ) || data->usedamageclip )
	#else
	if( data->usedamageclip )
	#endif
	{
		struct BitMap *bm;
		struct RastPort *rp, *oldrp;
		struct Window *oldwin;

		if ( ( bm = get_cachebitmap( _rp( obj )->BitMap, w, h, &rp ) ) )
		{
			oldrp = _rp( obj );
			oldwin = _window( obj );
			SetDrMd( rp, JAM1 );
			data->usedamageclip = FALSE;
			_rp( obj ) = rp;
			_window( obj ) = NULL;
			MUI_Redraw( obj, MADF_DRAWALL );
			_rp( obj ) = oldrp;
			_window( obj ) = oldwin;
			BltBitMapRastPort( bm, left, top, oldrp, _left( obj ), _top( obj ), _width( obj ), _height( obj ), 0xc0 );
			DoMethod( obj, MM_Layout_RefreshAfterIncrementalDump );
			redraw_done = TRUE;

			cachebminuse = FALSE;
			#if USE_DBUF_RESIZE
			data->last_xs = w;
			data->last_ys = h;
			#endif
		}
		else
		{
			#if USE_DBUF_RESIZE
			data->last_xs = 0;
			data->last_ys = 0;
			#endif
		}
	}

	if( !redraw_done )
	{
		#ifndef __MORPHOS__
		if( MUIMasterBase->lib_Version > 19 )
		#endif
		{
			struct VirtgroupData *data = INST_DATA(cl->cl_Super,obj);
			if( data->clipdraw )
			{
				MUIG_ScrollRaster(_rp(obj),data->pos[H]-data->oldpos[H],data->pos[V]-data->oldpos[V],_mleft(obj),_mtop(obj),_mright(obj),_mbottom(obj));
			}
		}
		#ifndef __MORPHOS__
		else
		{
			struct VirtgroupData38 *data = INST_DATA(cl->cl_Super,obj);

			if (data->clipdraw)
			{
				LONG pos[DIMS],size[DIMS];
				LONG add[DIMS];

				pos[H]  = _mleft(obj);
				pos[V]  = _mtop(obj);
				size[H] = _mwidth(obj);
				size[V] = _mheight(obj);

				add[H] = data->pos[H] - data->oldpos[H];
				add[V] = data->pos[V] - data->oldpos[V];

				/*
				** Optimiertes Zeichnen durch verschieben der Area um delta
				** und anschließendes Zeichnen durch eine ClipRegion ist nur
				** möglich, wenn das Objekt nicht selbst in einer VirtualGroup
				** ist. In diesem Fall kann ich nicht die Positionen herausfinden,
				** wo neu gezeichnet werden muß (_mleft, _mright etc. müssen ja
				** nicht unbedingt sichtbar sein).
				*/

				#define USE_VIRTGROUPFIX 1
				#ifdef USE_VIRTGROUPFIX
				if (abs(add[H])<size[H] && abs(add[V])<size[V] && (add[H] || add[V]))
				#else
				if (abs(add[H])<size[H] && abs(add[V])<size[V] && (add[H] || add[V]) && !_isinvirtual(obj))
				#endif
				{
					if (add[H]>=0)
					{
						if (add[V]>=0) ClipBlit(_rp(obj),pos[H]+add[H],pos[V]+add[V],_rp(obj),pos[H],pos[V]       ,size[H]-add[H],size[V]-add[V],0xc0);
						else           ClipBlit(_rp(obj),pos[H]+add[H],pos[V]       ,_rp(obj),pos[H],pos[V]-add[V],size[H]-add[H],size[V]+add[V],0xc0);
					}
					else
					{
						if (add[V]>=0) ClipBlit(_rp(obj),pos[H],pos[V]+add[V],_rp(obj),pos[H]-add[H],pos[V]       ,size[H]+add[H],size[V]-add[V],0xc0);
						else           ClipBlit(_rp(obj),pos[H],pos[V]       ,_rp(obj),pos[H]-add[H],pos[V]-add[V],size[H]+add[H],size[V]+add[V],0xc0);
					}

					if(_window(obj) && (_window(obj)->Flags & WFLG_SIMPLE_REFRESH))
					{
						if (_window(obj)->WLayer->front)
						{
							UBYTE oldmask = _rp(obj)->Mask;
							SetWrMsk(_rp(obj),0);
							ScrollRaster(_rp(obj),add[H],add[V],pos[H],pos[V],pos[H]+size[H]-1,pos[V]+size[V]-1);
							SetWrMsk(_rp(obj),oldmask);
						}
					}
				}
			}
		}
		#endif
		return( V_GroupDraw( obj, cl->cl_Super->cl_Super, msg ) );
	}

	return( 0 );
}

DECTMETHOD( Layout_Backfill )
{
	// We absolutely do not want this method to broadcast
	// to children ;)
	return( 0 );
}

DECTMETHOD( HTMLView_NewImageSizes )
{
	GETDATA;

	data->usedamageclip = TRUE;
	DoMethod( data->htmlwin, MUIM_Group_InitChange );
	DoMethod( data->htmlwin, MUIM_Group_ExitChange2, TRUE );

	return( 0 );
}

DECMMETHOD( Backfill )
{
	GETDATA;
	
	if( !data->lctx )
	{
		// If nothing to layout, simply backfill
		SetAPen( _rp( obj ), _pens( obj )[ MPEN_BACKGROUND ] );
		RectFill( _rp( obj ), _mleft( obj ), _mtop( obj ), _mright( obj ), _mbottom( obj ) );
	}
	// We do not want Virtgroup to backfill
	return( 0 );
}

//
// Javascript property/method handling
//

BEGINPTABLE
DPROP( URL,         string )
DPROP( title,       string )
DPROP( cookie,	    string )
DPROP( write,       funcptr )
DPROP( open,		funcptr )
DPROP( close,		funcptr )
DPROP( getElementById, funcptr )
DPROP( referrer,    string )
DPROP( writeln,     funcptr )
DPROP( images,      obj )
#if USE_LO_PIP
DPROP( pips,        obj )
#endif
DPROP( document,    obj )
DPROP( location,	obj )
DPROP( forms,		obj )
DPROP( links,		obj )
DPROP( frames,		obj )
DPROP( anchors,		obj )
DPROP( embeds,		obj )
DPROP( plugins,		obj )
DPROP( history,		obj )
DPROP( all,			obj )
DPROP( lastModified, string )
DPROP( onload,		funcptr )
DPROP( onunload,	funcptr )
#ifdef MBX
DPROP( ondiskchange, funcptr )
#endif
DPROP( domain,		string )
DPROP( width,		real )
DPROP( height,		real )
ENDPTABLE

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;
	struct customprop *cp;
	GETDATA;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	// Check for direct childs
	if( DoMethod( obj, MM_JS_FindByName, msg->propname ) )
		return( expt_obj );

	cp = cp_find( &data->cpl, msg->propname );
	if( cp )
		return( expt_obj );

	return( 0 );
}

DECSMETHOD( JS_SetProperty )
{
	GETDATA;

	//Printf( "setprop: %s\n", msg->propname );

	/* TOFIX: security: check originator's URL/domain for all property sets*/

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_onload:
			data->ix_onload = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onunload:
			//data->ix_onunload = *((int*)msg->dataptr);
			return( TRUE );

#ifdef MBX
		case JSPID_ondiskchange:
			data->ix_ondiskchange = *((int*)msg->dataptr);
			return( TRUE );
#endif

		case JSPID_title:
			DoMethod( data->htmlwin, MM_HTMLWin_SetTitle, msg->dataptr );
			return( TRUE );

		case JSPID_location:
			return( DoMethodA( data->htmlwin, (Msg)msg ) );

		case JSPID_domain:
			{
				/* TOFIX

				we should check the originating URL for this
				operation, for security - or any JS script on any
				domain can coerce a script to run under it's
				domain (thus completely negating security)

				*/

				int datalen = 0, domainlen = 0, freeflag = FALSE;
				char *uri, *p;
				struct parsedurl purl;

				// check string lengths
				if (msg->dataptr)
					datalen = strlen( msg->dataptr );
				if (data->domain)
					domainlen = strlen( data->domain );

				uri = malloc( strlen( nets_url( data->doc ) ) + 1 );
				if (uri)	// this must work ;)
				{
					strcpy(uri, nets_url( data->doc ) );
					uri_split( uri, &purl );

					if ( strcmp(purl.scheme, "file") )
					{
						// CHECKING HERE!!!!!
						// www.server.domain.com -> server.domain.com -> domain.com is valid
						// www.server.domain.com -> tits.ass.com INVALID (NULL returned)

						// Netscape JS docs say domain.com -> server.domain.com INVALID
						// IE implementation shows that this is VALID (we copy IE for maximum effect)

						// check for that last part
						p = strstr(purl.host, msg->dataptr);
						if ( p )
						{
							if ( !strcmp(p, msg->dataptr) )
							{
								free(data->domain);
								data->domain = malloc( datalen + 1 );
								if (data->domain) // we hope always TRUE
									strcpy( data->domain, msg->dataptr );
							}
							else // last segment of the URL doesn't match (unsecure)
								freeflag = TRUE;
						}
						else // URL doesn't match at all (unsecure)
							freeflag = TRUE;
					}
					else // is a file: url
						freeflag = TRUE;

					free(uri);	// don't need this anymore
				}
				else // uri allocation failed
					freeflag = TRUE;

				if (freeflag)
				{
					free(data->domain);
					data->domain = NULL;
				}
			}
			return( TRUE );

#if USE_NET
		case JSPID_cookie:
			{
				char cookiebuffer[ 16384 ];
				char urlbuffer[ MAXURLSIZE ];
				struct parsedurl purl;
				long oldcset1 = getprefslong( DSI_SECURITY_ASK_COOKIE_PERM, 1 );
				long oldcset2 = getprefslong( DSI_SECURITY_ASK_COOKIE_TEMP, 1 );

				strcpy( urlbuffer, nets_url( data->doc ) );
				uri_split( urlbuffer, &purl );

				stccpy( cookiebuffer, msg->dataptr, min( sizeof( cookiebuffer ), msg->datasize ) );

				// HACK! -- if we don't do this, we will deadlock
				// with askcookie() in cookie_set()

				setprefslong( DSI_SECURITY_ASK_COOKIE_PERM, 2 );
				setprefslong( DSI_SECURITY_ASK_COOKIE_TEMP, 2 );

				cookie_set( cookiebuffer, purl.host, purl.path );

				setprefslong( DSI_SECURITY_ASK_COOKIE_PERM, oldcset1 );
				setprefslong( DSI_SECURITY_ASK_COOKIE_TEMP, oldcset2 );

				return( TRUE );
			}
			break;
#endif /* USE_NET */

		case 0:
			cp_set( &data->cpl, msg->propname, *(APTR*)msg->dataptr );
			return( TRUE );
	}


	return( FALSE );
}

static void recbuildobjarray( Object *obj, struct MinList *cpl, APTR class )
{
	APTR o,ostate;
	struct List *l;

	if( !get( obj, MUIA_Group_ChildList, &l ) )
		return;
	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		if( OCLASS( o ) == class )
		{
			char *name;
			char tmpname[ 32 ];

			name = 0;
			get( o, MA_JS_Name, &name );
			if( !name || !*name )
			{
				sprintf( tmpname, "unnamed%x", (int)o );
				name = tmpname;
			}
			cp_set( cpl, name, o );
		}
		else
			recbuildobjarray( o, cpl, class );
	}
}

static APTR buildobjarray( APTR obj, APTR class )
{
	struct MinList *cpl;
	APTR o;

	o = JSNewObject( getjs_array(), MA_JS_Object_TerseArray, TRUE, TAG_DONE );
	if( o )
	{
		get( o, MA_JS_Object_CPL, &cpl );
		recbuildobjarray( obj, cpl, class );
	}

	return( o );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	struct customprop *cp;
	GETDATA;

	if( !pt )
	{
		APTR o = (APTR)DoMethod( obj, MM_JS_FindByName, msg->propname );
		if( o )
		{
			storeobjprop( msg, o );
			return( TRUE );
		}
		cp = cp_find( &data->cpl, msg->propname );
		if( cp )
		{
			//Printf( "found custom prop %lx / %lx\n", cp, cp->obj );
			storeobjprop( msg, cp->obj );
			return( TRUE );
		}

		return( FALSE );

	}

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}

	switch( pt->id )
	{
		case JSPID_URL:
			/* TOFIX: security: check originator's URL/domain */
			storestrprop( msg, nets_url( data->doc ) );
			return( TRUE );

		case JSPID_domain:
			/* TOFIX: security: check originator's URL/domain */
			storestrprop( msg, data->domain ? data->domain : "" );
			return( TRUE );

		case JSPID_referrer:
			/* TOFIX: security: check originator's URL/domain */
			storestrprop( msg, nets_referer( data->doc ) );
			return( TRUE );

		case JSPID_lastModified:
			/* TOFIX: security: check originator's URL/domain */
			storestrprop( msg, nets_getheader( data->doc, "LAST-MODIFIED" ) );
			return( TRUE );

		case JSPID_title:
			storestrprop( msg, data->lctx ? data->lctx->title : "" );
			return( TRUE );

		case JSPID_document:
		case JSPID_all: // Hack, kludge
			storeobjprop( msg, obj );
			return( TRUE );

		case JSPID_location:
		case JSPID_history:
		case JSPID_frames:
			return( DoMethodA( data->htmlwin, (Msg)msg ) );

#if USE_LO_PIP
		case JSPID_pips:
			data->js_pips = buildobjarray( obj, getlopipclass() );
			storeobjprop( msg, data->js_pips );
			return( TRUE );
#endif

		case JSPID_images:
			data->js_images = buildobjarray( obj, getloimageclass() );
			storeobjprop( msg, data->js_images );
			return( TRUE );

		case JSPID_forms:
			/* TOFIX: security: check originator's URL/domain ??? */
			data->js_forms = buildobjarray( obj, getloformclass() );
			storeobjprop( msg, data->js_forms );
			return( TRUE );

		case JSPID_links:
		case JSPID_anchors:
			/* TOFIX: security: check originator's URL/domain */
			data->js_links = buildobjarray( obj, getloanchorclass() );
			storeobjprop( msg, data->js_links );
			return( TRUE );

		case JSPID_embeds:
			/* TOFIX: security: check originator's URL/domain (not plugins) */
		case JSPID_plugins:
			data->js_embeds = buildobjarray( obj, getloembedclass() );
			storeobjprop( msg, data->js_embeds );
			return( TRUE );

#if USE_NET
		case JSPID_cookie:
			/* TOFIX: security: check originator's URL/domain */
			{
				char cookiebuffer[ 16384 ];
				char urlbuffer[ MAXURLSIZE ];
				struct parsedurl purl;
				char *p;

				strcpy( urlbuffer, nets_url( data->doc ) );
				uri_split( urlbuffer, &purl );

				cookiebuffer[ 0 ]=0;
				cookie_get( purl.host, purl.path, cookiebuffer,! strcmp( purl.scheme, "https" ) );
				p = strstr( cookiebuffer, "\r\n" );
				if( p )
					*p = 0;
				storestrprop( msg, cookiebuffer[ 0 ] ? &cookiebuffer[7] : "" );
				return( TRUE );
			}
			break;
#endif /* USE_NET */

		case JSPID_width:
			storerealprop( msg, (double) data->layout_width );
			return( TRUE );

		case JSPID_height:
			storerealprop( msg, (double) data->layout_height );
			return( TRUE );
	}

	return( FALSE );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;

	switch( msg->pid )
	{
		case JSPID_open:
			if( !data->in_layout )
				data->js_isopen = TRUE;
			return( TRUE );

		case JSPID_close:
			if( data->js_isopen && !data->in_layout )
			{
				char *buffer;
				char oldbaseref[ MAXURLSIZE ];

				oldbaseref[ 0 ] = '\0';

				data->js_isopen = FALSE;

				DoMethod( data->htmlwin, MUIM_Group_InitChange );

				if( data->lctx )
				{
					ASSERT( data->lctx->baseref );
					strcpy( oldbaseref, data->lctx->baseref );
					layout_delete( data->lctx );
				}

				// Run the garbage collector
				js_gc();

				data->finished = FALSE;
				data->gottitle = FALSE;

				data->lctx = layout_new();
				if( data->lastcmap )
					layout_setuppens( data->lctx, data->lastcmap );

				layout_attach( data->lctx, obj );
				layout_setdom( data->lctx, data->htmlwin, obj );
				layout_setbaseref( data->lctx, oldbaseref );

				layout_setmargins( data->lctx,
					getv( data->htmlwin, MA_Layout_MarginLeft ),
					getv( data->htmlwin, MA_Layout_MarginRight ),
					getv( data->htmlwin, MA_Layout_MarginTop ),
					getv( data->htmlwin, MA_Layout_MarginBottom )
				);

				buffer = (char*)DoMethod( obj, MM_HTMLView_GetPushedData );

				if( buffer )
					layout_do( data->lctx, buffer, strlen( buffer ), 0, TRUE );
				else
					layout_do( data->lctx, "", 0, 0, TRUE );

				DoMethod( data->htmlwin, MM_HTMLWin_CheckDone );
				DoMethod( data->htmlwin, MUIM_Group_ExitChange );
			}
			return( TRUE );

		case JSPID_write:
		case JSPID_writeln:
			{
				int strl;
				struct pushnode *pn;

				while( msg->argcnt-- > 0 )
				{
					exprs_get_type( msg->es, 0, &strl );
					pn = malloc( sizeof( *pn ) + 2 + strl );
					memset( pn, '\0', sizeof( *pn ) + 2 + strl );
					exprs_pop_as_string( msg->es, pn->str, 0 );
					if( msg->pid == JSPID_writeln )
						strcpy( &pn->str[ strl ], "\n" );
					else
						pn->str[ strl ] = 0;
					ADDTAIL( &data->pushbuffer, pn );
				}
			}
			return( TRUE );

		case JSPID_getElementById:
			{
				char *name;
				static APTR dval;

				if( msg->argcnt-- < 0 )
					return( FALSE );

				name = exprs_peek_as_string( msg->es, 0 );
				dval = (APTR)DoMethod( obj, MM_JS_FindByID, name );
				exprs_pop( msg->es );

				*msg->typeptr = expt_obj;
				*msg->dataptr = dval;
				*msg->datasize = sizeof( dval );

				return( TRUE );
			}
			break;
	}

	// no super class..
	return( 0 );
}

DOM_LISTPROP

DECTMETHOD( HTMLView_GetPushedData )
{
	GETDATA;
	int totalsize = 0;
	struct pushnode *pn;
	char *res = NULL;

	for( pn = FIRSTNODE( &data->pushbuffer ); NEXTNODE( pn ); pn = NEXTNODE( pn ) )
		totalsize += strlen( pn->str );

	if( totalsize )
	{
		res = malloc( totalsize + 1 );
		if( res )
		{
			res[ 0 ] = 0;
			while( pn = REMHEAD( &data->pushbuffer ) )
			{
				strcat( res, pn->str );
				free( pn );
			}
		}
	}

	return( (ULONG)res );
}

DECGET
{
	switch( (int)msg->opg_AttrID )
	{
		case MA_JS_ClassName:
			*msg->opg_Storage = (ULONG)"document";
			return( TRUE );
	}

	return( DOSUPER );
}

DECSMETHOD( JS_FindByName )
{
	APTR ostate, o, or;
	struct List *l;

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		if( ( or = (APTR)DoMethodA( o, ( Msg )msg ) ) )
		{
			return( (ULONG)or );
		}
	}
	return( 0 );
}

DECSMETHOD( JS_ToString )
{
	js_tostring( "document", msg );
	return( TRUE );
}

DECSMETHOD( JS_SetGCMagic )
{
	GETDATA;

	if( data->gcmagic != msg->magic )
	{
		struct customprop *cp;

		data->gcmagic = msg->magic;
		for( cp = FIRSTNODE( &data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
		{
			if( cp->obj )
				DoMethodA( cp->obj, (Msg)msg );
		}
		return( DOSUPER );
	}
	return( TRUE );
}

int V_ShowClipped( APTR obj, struct LongRect *clip )
{
	struct LongRect clipbuf;

	if (!clip && _isinvirtual(obj))
	{
		MUIP_GetVirtualRect(_parent(obj),&clipbuf);
		clip = &clipbuf;
	}

	_flags(obj) &= ~MADF_PARTIAL;

	if (clip)
	{
		struct layout_info *li = 0;

		get( obj, MA_Layout_Info, &li );

		if( li )
		{
			int x1 = _vleft( obj );
			int y1 = _vtop( obj );
			int x2 = x1 + li->xs - 1;
			int y2 = y1 + li->ys - 1;

			if (!overlap(x1,y1,x2,y2,clip->left,clip->top,clip->right,clip->bottom))
			{
				return(FALSE);
			}

			if (x1 < clip->left || y1 < clip->top || x2 > clip->right || y2 > clip->bottom)
				_flags(obj) |= MADF_PARTIAL;
		}
		else
		{
			return( FALSE );
		}
	}

	if( DoMethod(obj,MUIM_Show,(ULONG) clip) )
	{
		_flags(obj) |= MADF_VISIBLE;
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

#ifndef MBX
extern BOOL ASM _IsRectangleVisibleInLayer(__reg( a0, struct Layer *l ), __reg( d0, WORD x0 ), __reg( d1, WORD y0 ), __reg( d2, WORD x1 ), __reg( d3, WORD y1 ));
#define IsRectangleVisibleInLayer _IsRectangleVisibleInLayer
#endif // !MBX

#ifdef MBX
#ifdef __GNUC__
#define isvis(rp,l,x1,y1,x2,y2) ({ Rectangle_s coords = {{x1,y1},{x2,y2}}; IsRectangleVisible(rp,&coords); })
#else
#define isvis(rp,l,x1,y1,x2,y2) _isvis(rp,x1,y1,x2,y2)
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

ULONG V_GroupDraw( APTR obj, struct IClass *cl, struct MUIP_Draw *msg )
{
	//struct GroupData *data = INST_DATA(cl,obj);
#if USE_RESIZEBUFFER
	struct MUI_RenderInfo *mri = muiRenderInfo(obj);
#endif


	/*
	#ifdef AMIGAOS
	struct Layer *layer = mri->mri_Window ? mri->mri_Window->WLayer : NULL;
	#else
	int layer = 0;
	#endif
	*/

	LONG pos[DIMS],size[DIMS];
	
	#ifdef USE_RESIZEBUFFER
	if ((mri->mri_Flags & MUIMRI_RESIZEREDRAW) && _areadata(obj)->lastpos==_areadata(obj)->mad.mad_Box)
	{
		D(DBF_LAYOUT,bug("skipped redraw on group %08lx\n",obj));
		return(FALSE);
	}
	#endif

	pos[H]  = _left(obj);
	pos[V]  = _top(obj);
	size[H] = _width(obj);
	size[V] = _height(obj);

	msg->flags = _flags(obj) & MADF_DRAWMASK;
	_flags(obj) &= ~MADF_DRAWMASK;

	//if(!(msg->flags & (MADF_DRAWOBJECT|MADF_DRAWCHILD)))
	//	return(0);

	if (_isvirtual(obj))
	{
		if (!DoMethod(obj,MUIM_Virtgroup_InstallClip))
		{
			return(FALSE);
		}
	}

	if (msg->flags & MADF_DRAWOBJECT)
	{
		DoMethod(obj, MUIM_Backfill, pos[H],pos[V],pos[H]+size[H]-1,pos[V]+size[V]-1, 0, 0 );
	}

	if (msg->flags & (MADF_DRAWOBJECT|MADF_DRAWCHILD))
	{
		struct MinList *childlist;
		APTR child, ostate;

		get( obj, MUIA_Group_ChildList, &childlist );
		ostate = childlist->mlh_Head;

		// Skip the dummy object
		NextObject( &ostate );

		while( child = NextObject( &ostate ) )
		{
			if(_isvisible(child))
			{
				struct layout_info *li = 0;

				get( child, MA_Layout_Info, &li );
				if( !li )
					continue;

				//if( isvis(mri->mri_RastPort,layer,_left(obj),_top(obj),(_left(obj)+li->xs-1),(_top(obj)+li->ys-1) ) )
				{
					if (msg->flags & MADF_DRAWCHILD)
					{
						if (_flags(child) & MADF_DRAWMASK)
						{
							DoMethod(child,MUIM_Draw,0);
						}
					}
					else
					{
						_flags(child) |= MADF_DRAWALL;
						DoMethod(child,MUIM_Draw,0);
					}
				}
			}
		}
	}

	if (_isvirtual(obj)) DoMethod(obj,MUIM_Virtgroup_RemoveClip);

	return(0);
}

DECMMETHOD( Virtgroup_Update )
{
	struct VirtgroupData *data = INST_DATA(cl->cl_Super,obj);
	BYTE d;
	LONG add[DIMS];

	for (d=0;d<DIMS;d++)
	{
		add[d] = data->pos[d];
		if (!msg->layout) add[d] -= data->oldpos[d];
	}

	if (add[H] || add[V])
	{
		struct MinList *childlist;

		Object *child;
		Object *cstate;

		get( obj, MUIA_Group_ChildList, &childlist );

		for (cstate=(Object *)childlist->mlh_Head;child=NextObject(&cstate);)
		{
			MUI_Offset(child,-add[H],-add[V]);
		}

		if (_isvisible(obj))
		{
			struct LongRect lr;

			MUIP_GetVirtualRect(obj,&lr);

			for (cstate=(Object *)childlist->mlh_Head;child=NextObject(&cstate);)
			{
				if (_isvisible(child))
					MUI_Hide(child);

				V_ShowClipped(child,&lr);
			}

#ifndef MBX
			if( MUIMasterBase->lib_Version > 19 )
			{
#endif
				data->clipdraw = TRUE;
				MUI_Redraw(obj,MADF_DRAWOBJECT);
				data->clipdraw = FALSE;
#ifndef MBX
			}
			else
			{
				struct VirtgroupData38 *data = INST_DATA(cl->cl_Super,obj);
				data->clipdraw = TRUE;
				MUI_Redraw(obj,MADF_DRAWOBJECT);
				data->clipdraw = FALSE;
			}
#endif

#ifndef MBX
			if (MUI_BeginRefresh(muiRenderInfo(obj),0))
			{
				MUI_Redraw(obj,MADF_DRAWOBJECT);
				MUI_EndRefresh(muiRenderInfo(obj),0);
			}
#endif

		}
	}

	data->oldpos[H] = data->pos[H];
	data->oldpos[V] = data->pos[V];

	return(0);
}

DECMMETHOD( Show )
{
	struct MinList *childlist;
	APTR o, ostate;
	extern int V_ShowClipped( APTR obj, struct LongRect *clip ); // from htmlview.c
	struct LongRect *old;
	struct LongRect buf;

	MUIP_GetVirtualRect(obj,&buf);

	old = msg->clip;
	msg->clip = &buf;

	get( obj, MUIA_Group_ChildList, &childlist );
	ostate = childlist->mlh_Head;
	NextObject( &ostate ); // Skip dummy
	while( o = NextObject( &ostate ) )
	{
		if( _flags( o ) & MADF_SHOWME)
			V_ShowClipped( o, msg->clip );
	}

	msg->clip = old;

	return(TRUE);
}

#ifdef MBX
DECTMETHOD( JS_CDPlayer_DiskChange )
{
	GETDATA;

	if( data->ix_ondiskchange )
	{
		DoMethod( data->htmlwin, MM_HTMLWin_ExecuteEvent, jse_diskchange, data->ix_ondiskchange, obj,
			TAG_DONE
		);
	}

	// Broadcast to possible child windows
	return( DOSUPER );
}
#endif

#if USE_STB_NAV
DECSMETHOD( Cross_SetDir )
{
	GETDATA;
	int vl = getv( obj, MUIA_Virtgroup_Left );
	int vt = getv( obj, MUIA_Virtgroup_Top );

	if( vl > 0 )
		data->crossdirs |= CROSSDIR_WEST;
	else
		data->crossdirs &= ~CROSSDIR_WEST;

	if( vt > 0 )
		data->crossdirs |= CROSSDIR_NORTH;
	else
		data->crossdirs &= ~CROSSDIR_NORTH;

	if( getv( obj, MUIA_Virtgroup_Width ) - vl > _mwidth ( obj ) )
		data->crossdirs |= CROSSDIR_EAST;
	else
		data->crossdirs &= ~CROSSDIR_EAST;

	if( getv( obj, MUIA_Virtgroup_Height ) - vt > _mheight( obj ) )
		data->crossdirs |= CROSSDIR_SOUTH;
	else
		data->crossdirs &= ~CROSSDIR_SOUTH;

	if( data->htmlwin )
		DoMethod( data->htmlwin, MM_Cross_SetDir, data->crossdirs );

	return( 0 );
}
#endif

#define DK(k) {DoMethod( _parent( obj ), fwdmethod, NULL, MUIKEY_##k );return(MUI_EventHandlerRC_Eat);}

DECMMETHOD( HandleEvent )
{

#ifndef MBX
	ULONG fwdmethod = ( MUIMasterBase->lib_Version > 19 ) ? MUIM_HandleEvent : MUIM_HandleInput;
	if( msg->imsg->Class == IDCMP_RAWKEY )
	{
		int ctrl = msg->imsg->Qualifier & ( IEQUALIFIER_CONTROL );
		int shift = msg->imsg->Qualifier & ( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT );
		int alt = msg->imsg->Qualifier & ( IEQUALIFIER_LALT | IEQUALIFIER_RALT );

		//Printf( "raw code = %ld\n", msg->imsg->Code );

		switch( msg->imsg->Code )
		{
			#if 0
			case NM_WHEEL_UP:
			case NM_WHEEL_DOWN:
				{
					int pageheight = _mheight( obj );
					APTR sb = NULL;

					get( _parent( obj ), MUIA_Scrollgroup_VertBar, &sb );

					if( sb )
					{
						if( ctrl )
							pageheight /= 32;
						else if( !shift )
							pageheight /= 8;

						set( sb, MUIA_Prop_First, getv( obj, MUIA_Virtgroup_Top ) + ( ( msg->imsg->Code == NM_WHEEL_UP ) ? -pageheight : pageheight ) );
					}
				}
				break;

			case NM_WHEEL_LEFT:
				DK( LEFT );

			case NM_WHEEL_RIGHT:
				DK( LEFT );
			#endif
			#if WHEEL_MOUSE_IS_FUCKED
			case NM_WHEEL_UP:
			case NM_WHEEL_DOWN:
			case NM_WHEEL_LEFT:
			case NM_WHEEL_RIGHT:
				return ( MUI_EventHandlerRC_Eat );
			#endif

			case 0x4c:	// cup
				if( ctrl )
					DK( TOP );
				if( shift )
					DK( PAGEUP );
				DK( UP );

			case 0x3e:	// nk-8
				DK( UP );

			case 0x4d:	// cdown
				if( ctrl )
					DK( BOTTOM );
				if( shift )
					DK( PAGEDOWN );

			case 0x1e:	// nk-8
				DK( DOWN );

			case 0x4f: // left
			case 0x2d: // nk-4
				if( alt )
				{
					DoMethod( app, MUIM_Application_PushMethod, _win( obj ), 1, MM_HTMLWin_Backward );
					return( 0 );
				}
				if( shift || ctrl )
					DK( LINESTART );
				DK( LEFT );

			case 0x4e: // right
			case 0x2f: // nk-6
				if( alt )
				{
					DoMethod( app, MUIM_Application_PushMethod, _win( obj ), 1, MM_HTMLWin_Forward );
					return( 0 );
				}
				if( shift || ctrl )
					DK( LINEEND );
				DK( RIGHT );

			case 0x3d: // nk-home
				DK( TOP );

			case 0x1d: // nk-end
				DK( BOTTOM );

			case 0x41: // backspace
			case 0x3f: // nk-pageup
			case 0x48: // pageup
				DK( PAGEUP );

			case 0x40: // space
			case 0x1f: // nk-pagedown
			case 0x49: // pagedown
				DK( PAGEDOWN );

			case 0x4a: // nk-minus
				DoMethod( app, MUIM_Application_PushMethod, _win( obj ), 1, MM_HTMLWin_Backward );
				return( 0 );

			case 0x5e: // nk-plus
				DoMethod( app, MUIM_Application_PushMethod, _win( obj ), 1, MM_HTMLWin_Forward );
				return( 0 );

			case 0x43:	// nk-enter

			default:
				return( 0 );
		}
	}
#else
// MBX Special version
	if( msg->imsg->Class == IDCMP_RAWKEY )
	{
		int shift = msg->imsg->Qualifier & ( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | IEQUALIFIER_CONTROL | IEQUALIFIER_LALT | IEQUALIFIER_RALT );
		int dx = 0, dy = 0;

#define D_LINE 20
#define D_PAGE ((_height(obj)*80)/100)
#define DX_COL 20

		switch( msg->imsg->Code )
		{
			case 0xb2:	// cup
				if( shift )
					dy = -D_PAGE;
				else
					dy = -D_LINE;
				break;

			case 0xd2:	// cdown
				if( shift )
					dy = D_PAGE;
				else
					dy = D_LINE;
				break;

			case 0xd1: // left
				if( shift )
					dx = -getv( obj, MUIA_Virtgroup_Width );
				else
					dx = -DX_COL;
				break;

			case 0xd3: // right
				if( shift )
					dx = getv( obj, MUIA_Virtgroup_Width );
				else
					dx = DX_COL;
				break;

			default:
				return( 0 );
		}

		if( dx )
			set( obj, MUIA_Virtgroup_Left, getv( obj, MUIA_Virtgroup_Left ) + dx );
		if( dy )
			set( obj, MUIA_Virtgroup_Top, getv( obj, MUIA_Virtgroup_Top ) + dy );
	}

#endif

	return( 0 );
}

/*
 * The following methods can be forwarded to the HTMLWin object
 * when needed. They're called HTMLRexx_ to avoid confusion. An
 * HTMLRexx command should be available on images, links and
 * frames (HTMLView) objects. They might either be forwarded to
 * frames in most cases or act directly.
 */

DECMETHOD( HTMLWin_Backward, ULONG )
{
	GETDATA;

	//TOFIX!! check if shift is pressed. if it is, apply as a whole
	return( DoMethod( data->htmlwin, MM_HTMLWin_Backward ) ); //TOFIX!! handle frames instead of window only
}

DECMETHOD( HTMLWin_Forward, ULONG )
{
	GETDATA;

	//TOFIX!! check if shift is pressed. if it is, apply as a whole

	return( DoMethod( data->htmlwin, MM_HTMLWin_Forward ) ); //TOFIX!! handle frames instead of window only
}

/*
 * Load (or reload) a background
 */
DECMETHOD( HTMLRexx_LoadBG, ULONG )
{
	GETDATA;

	if( getv( data->lctx->body, MA_Layout_BGImageURL ) )
	{
		/* reload */
		set( data->lctx->body, MA_Layout_BGImageURL, ( STRPTR )getv( data->lctx->body, MA_Layout_BGImageURLName ) ); //TOFIX!! check if that actually works..
	}
	else
	{
		STRPTR n;

		/* we didn't load it yet */
		if( n = ( STRPTR )getv( data->lctx->body, MA_Layout_BGImageURLName ) )
		{
			set( data->lctx->body, MA_Layout_BGImageURL, n );
		}
	}

	return( 0 );
}

/*
 * Opens the current's frame URL in a new window or in the
 * current frame
 */
DECSMETHOD( HTMLRexx_SetURLFromObject )
{
	GETDATA;

	if( data->doc && data->doc->url )
	{
		if( msg->newwin )
		{
			win_create( "", data->doc->url, data->doc->referer, NULL, FALSE, msg->reload, FALSE );
			return( TRUE );
		}
		else
		{
			pushmethod( data->htmlwin, 1, MM_HTMLWin_Reload );
			return( TRUE );
		}
	}
	else if( msg->newwin )
	{
		/*
		 * No URL given but NEW argument given. We open a new
		 * empty window then.
		 */
		win_create( "", "", NULL, NULL, FALSE, FALSE, FALSE );
		return ( TRUE );
	}

	return( FALSE );
}

/*
 * Copies the current frame's URL to the clipboard.
 */
DECTMETHOD( HTMLRexx_SetClipFromObject )
{
	GETDATA;

	if( data->doc && data->doc->url )
	{
		return( (ULONG)storetoclip( data->doc->url ) );
	}

	return( FALSE );
}

/*
 * Opens the source viewer window and display the current URL
 * TOFIX!! perhaps make it open empty and kind of "link" it with the
 * frame it was opened over. If the frame is gone, open a new window
 * that way the user has a cheap HTML editor.
 */
DECMETHOD( HTMLRexx_OpenSourceView, ULONG )
{
	GETDATA;

	D( db_gui, bug( "called\n" ) );

	if( data->doc )
	{
		return( ( ULONG )opensourceview( data->doc ) );
	}
	return( FALSE );
}

/*
 * Opens the DocInfoWindow (tm)
 */
DECSMETHOD( HTMLRexx_OpenDocInfo )
{
	GETDATA;

	if( msg->url )
	{
		return( ( ULONG )createdocinfowin( msg->url ) );
	}
	else
	{
		if( data->doc && data->doc->url )
		{
			return( ( ULONG )createdocinfowin( data->doc->url ) );
		}
	}

	return( FALSE );
}

/*
 * Saves the URL to disk
 */
DECSMETHOD( HTMLRexx_SaveURL )
{
	GETDATA;

#if USE_NET
	if( msg->url )
	{
		queue_download( msg->url, NULL, TRUE, msg->ask ); //TOFIX!! fix the name which is blatantly ignored atm
		return( TRUE );
	}
	else
	{
		if( data->doc && data->doc->url )
		{
			queue_download( data->doc->url, data->doc->referer, TRUE, msg->ask ); //TOFIX!! fix the name which is blatantly ignored atm
			return( TRUE );
		}
	}
#endif /* USE_NET */
	return( FALSE );
}

/* --- DO NOT ADD ANY METHOD BELOW BUT ABOVE HTMLWin_Backward for the sake of readability ! --- */

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSET
DEFGET
DEFMMETHOD( AskMinMax )
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( Draw )
DEFMMETHOD( Backfill )
DEFMMETHOD( Show )
DEFTMETHOD( Layout_Backfill )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
case MM_JS_FindByID:
DEFSMETHOD( JS_FindByName )
DEFSMETHOD( JS_ToString )
DEFTMETHOD( HTMLView_NewImageSizes )
DEFTMETHOD( HTMLView_RefreshTrigger )
DEFTMETHOD( HTMLView_GetPushedData )
DEFSMETHOD( HTMLView_ShowNStream )
DEFSMETHOD( HTMLRexx_SetURLFromObject )
DEFSMETHOD( HTMLRexx_OpenDocInfo )
DEFSMETHOD( HTMLRexx_SaveURL )
DEFMETHOD( HTMLRexx_OpenSourceView )
DEFMETHOD( HTMLWin_Backward )
DEFMETHOD( HTMLWin_Forward )
DEFMETHOD( HTMLRexx_LoadBG )

case MM_JS_GetGCMagic:
	return( 0 ); // We're not under GC control

DEFTMETHOD( JS_SetGCMagic )

DEFMMETHOD( Virtgroup_Update )

#ifdef MBX
DEFTMETHOD( JS_CDPlayer_DiskChange )
#endif
DEFTMETHOD( NStream_GotData )
DEFTMETHOD( NStream_Done )
DEFTMETHOD( HTMLRexx_SetClipFromObject )
#if USE_STB_NAV
DEFSMETHOD( Cross_SetDir )
#endif

DEFMMETHOD( HandleEvent )
ENDMTABLE

int create_htmlviewclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Virtgroup, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "HtmlViewClass";
#endif

	return( TRUE );
}

void delete_htmlviewclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR gethtmlviewclass( void )
{
	return( mcc->mcc_Class );
}
