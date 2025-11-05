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

	VLayout 2.0
	-----------
	V HTML Layout engine

	(C) 2000-2003 by Oliver Wagner, David Gerber, Jon Bright

	$Id: layout_main.c,v 1.72 2004/12/30 15:20:32 henes Exp $

*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#endif

/* private */
#include "layout.h"
#include "fontcache.h"
#include "colortable.h"
#include "prefs.h"
#include "malloc.h"
#include "network.h"
#include "classes.h"
#include "mui_func.h"

//
// -----------------------------------------------------------------------
//
// Create a new layout context
//
struct layout_ctx *layout_new( void )
{
	APTR pool;
	struct layout_ctx *n;

	pool = CreatePool( MEMF_ANY, 8192, 4096 );
	if( pool )
	{
		n = AllocPooled( pool, sizeof( *n ) );
		if( n )
		{
			memset( n, 0, sizeof( *n ) );
			n->pool = pool;

			NEWLIST( &n->l );
			InitSemaphore( &n->s );

			NEWLIST( &n->meta );
			NEWLIST( &n->pens );
			NEWLIST( &n->fontstyles );
			NEWLIST( &n->tablestack );
			NEWLIST( &n->listack );
			NEWLIST( &n->fetchlist );

			n->textbuffersize = TEXTBUFFER_INITIALSIZE;
			n->textbuffer = l_malloc( n, n->textbuffersize );

			n->meta_refresh_period = -1;

			n->htmlsourceline = 1;

#if USE_LIBUNICODE
			n->iso_charset = UCCS_LATIN1;
#endif

			return( n );
		}

		DeletePool( pool );
	}
	return( NULL );
}

//
// -----------------------------------------------------------------------
//
// Detach layout from a container (MUI class)
//
// Must be called synchronously from the main process
// The container group should be in a modifiable state (InitChange)
//
static void layout_detach( struct layout_ctx *ctx )
{
	if( ctx->is_attached )
	{
		APTR ostate, o;
		struct List *l;

		DoMethod( ctx->attached_to, MUIM_Group_InitChange );

		// Clear all objects...
		get( ctx->attached_to, MUIA_Group_ChildList, &l );

		ostate = l->lh_Head;

		// Skip the fake object
		NextObject( &ostate );

		while( o = NextObject( &ostate ) )
		{
			DoMethod( ctx->attached_to, OM_REMMEMBER, o );
			ADDTAIL( &ctx->l, _OBJECT( o ) );
		}

		DoMethod( ctx->attached_to, MUIM_Group_ExitChange );

		ctx->is_attached = FALSE;
	}
}

//
// -----------------------------------------------------------------------
//
// Dispose layout context
//
void layout_delete( struct layout_ctx *ctx )
{
	if( ctx )
	{
		APTR o;
		struct layout_fetchnode *fn;

		layout_detach( ctx );
		layout_freepens( ctx );

		while( fn = REMHEAD( &ctx->fetchlist ) )
			nets_close( fn->ns );

		while( o = REMHEAD( &ctx->l ) )
		{
			APTR obj = BASEOBJECT( o );

			// If this object is not under GC control, free it
			if( !DoMethod( obj, MM_JS_GetGCMagic ) )
			{
				MUI_DisposeObject( obj );
			}
		}

		ObtainSemaphore( &ctx->s );

#if USE_LIBUNICODE
		if( ctx->uconv )
			FreeCharConverter( ctx->uconv );
#endif
		DeletePool( ctx->pool );
	}
}

//
// -----------------------------------------------------------------------
//
// Attach layout to a container (MUI class)
//
// Must be called synchronously from the main process
// The container group should be in a modifiable state (InitChange)
//
void layout_attach( struct layout_ctx *ctx, APTR obj )
{
	APTR o;

	DoMethod( obj, MUIM_Group_InitChange );

	// This checks whether we're attached first anyway
	layout_detach( ctx );

	while( o = REMHEAD( &ctx->l ) )
	{
		DoMethod( obj, OM_ADDMEMBER, BASEOBJECT( o ) );
	}

	ctx->penspec_background = muipenspec2rgb24( obj, getprefs( DSI_COLORS + 0 ) );
	ctx->penspec_text = muipenspec2rgb24( obj, getprefs( DSI_COLORS + 1 ) );
	ctx->penspec_link = muipenspec2rgb24( obj, getprefs( DSI_COLORS + 2 ) );
	ctx->penspec_alink = muipenspec2rgb24( obj, getprefs( DSI_COLORS + 3 ) );
	ctx->penspec_vlink = muipenspec2rgb24( obj, getprefs( DSI_COLORS + 3 ) );

	ctx->attached_to = obj;
	ctx->is_attached = TRUE;

	DoMethod( obj, MUIM_Group_ExitChange );
}

//
// Add an object to the object tree
//
void layout_addobj( struct layout_ctx *ctx, APTR o )
{
	if( ctx->is_attached )
	{
		DoMethod( ctx->attached_to, MUIM_Group_InitChange );
		DoMethod( ctx->attached_to, OM_ADDMEMBER, o );
		DoMethod( ctx->attached_to, MUIM_Group_ExitChange );
	}
	else
		ADDTAIL( &ctx->l, _OBJECT( o ) );
}

//
// Remove an object from the object tree
//
void layout_remobj( struct layout_ctx *ctx, APTR o )
{
	if( ctx->is_attached )
	{
		DoMethod( ctx->attached_to, MUIM_Group_InitChange );
		DoMethod( ctx->attached_to, OM_REMMEMBER, o );
		DoMethod( ctx->attached_to, MUIM_Group_ExitChange );
	}
	else
		REMOVE( _OBJECT( o ) );
}

//
// Set DOM pointers
//
void layout_setdom( struct layout_ctx *ctx, APTR dom_win, APTR dom_document )
{
	ctx->dom_win = dom_win;
	ctx->dom_document = dom_document;
}


//
// Default margins
//
void layout_setmargins( struct layout_ctx *ctx, int l, int r, int t, int b )
{
	ctx->margin_left = l;
	ctx->margin_right = r;
	ctx->margin_top = t;
	ctx->margin_bottom = b;
}

//
// Parsing related stuff
//

void layout_setbaseref( struct layout_ctx *ctx, char *str )
{
	if( ctx->baseref )
		l_free( ctx, ctx->baseref );
	ctx->baseref = l_strdup( ctx, str );
}

void layout_setbasetarget( struct layout_ctx *ctx, char *str )
{
	if( ctx->basetarget )
		l_free( ctx, ctx->basetarget );
	ctx->basetarget = l_strdup( ctx, str );
}

//
// Meta object tags
//

char *layout_getmeta( struct layout_ctx *ctx, char *header, int num )
{
	struct layout_meta *m;

	for( m = FIRSTNODE( &ctx->meta ); NEXTNODE( m ); m = NEXTNODE( m ) )
	{
		if( !stricmp( m->header, header ) )
		{
			if( num-- <= 0 )
				return( m->value );
		}
	}

	return( NULL );
}


//
// -----------------------------------------------------------------------
//
// Memory management helper functions
//
APTR l_malloc( struct layout_ctx *ctx, int size )
{
	APTR n = AllocVecPooled( ctx->pool, size );
	if( !n )
	{
		reporterror( "l_malloc(%lx,%lu) failed", ctx, size );
		exit( 20 );
	}
	return( n );
}

APTR l_calloc( struct layout_ctx *ctx, int size )
{
	APTR n = AllocVecPooled( ctx->pool, size );
	if( !n )
	{
		reporterror( "l_calloc(%lx,%lu) failed", ctx, size );
		exit( 20 );
	}
	memset( n, 0, size );
	return( n );
}

APTR l_realloc( struct layout_ctx *ctx, APTR old, int newsize )
{
	int oldsize = 0;
	APTR newdata = NULL;

	if( old )
	{
		// Dodgy Hack From Hell
		int *tempptr = old;
		tempptr--;
		oldsize = *tempptr;
	}

	if( newsize )
	{
		newdata = AllocVecPooled( ctx->pool, newsize );
		if( !newdata )
		{
			reporterror( "l_realloc(%lx,%lx,%lu) failed", ctx, old, newsize );
			exit( 20 );
		}
		if( old && oldsize )
			memcpy( newdata, old, min( newsize, oldsize - 4 ) );
	}
	if( old )
		FreeVecPooled( ctx->pool, old );

	return( newdata );
}

STRPTR l_strdup( struct layout_ctx *ctx, char *str )
{
	STRPTR n = AllocVecPooled( ctx->pool, strlen( str ) + 1 );
	if( !n )
	{
		reporterror( "l_strdup(%lx,%.256s) failed", ctx, str );
		exit( 20 );
	}
	strcpy( n, str );
	return( n );
}

void l_free( struct layout_ctx *ctx, APTR what )
{
	if( what )
		FreeVecPooled( ctx->pool, what );
}

void l_readstrtag( struct TagItem *tag, char **str )
{
	if( *str )
		free( *str );
	if( tag->ti_Data )
		*str = strdup( (STRPTR)tag->ti_Data ); /* TOFIX */
	else
		*str = NULL;
}

//
// -----------------------------------------------------------------------
//
// Pen cache
//

#ifndef MBX

void layout_setuppens( struct layout_ctx *ctx, struct ColorMap *cmap )
{
	ctx->cmap = cmap;
}

void layout_freepens( struct layout_ctx *ctx )
{
	struct layout_pen *p;

	while( p = REMHEAD( &ctx->pens ) )
	{
		ReleasePen( ctx->cmap, p->pen );
		l_free( ctx, p );
	}
}

ULONG layout_getpen( struct layout_ctx *ctx, ULONG rgb )
{
	struct layout_pen *p;
	ULONG r, g, b;

	for( p = FIRSTNODE( &ctx->pens ); NEXTNODE( p ); p = NEXTNODE( p ) )
	{
		if( p->rgb == rgb )
			return( p->pen );
	}

	p = l_malloc( ctx, sizeof( *p ) );
	p->rgb = rgb;

	r = rgb >> 16;
	g = ( rgb >> 8 ) & 0xff;
	b = rgb & 0xff;

	r = MAKE_ID( r, r, r, r );
	g = MAKE_ID( g, g, g, g );
	b = MAKE_ID( b, b, b, b );

	p->pen = ObtainBestPen( ctx->cmap,
		r,
		g,
		b,
		OBP_Precision, PRECISION_EXACT,
		TAG_DONE
	);

	ADDTAIL( &ctx->pens, p );

	return( p->pen );
}

#endif

void layout_make_shadow_shine( ULONG bgcolor, struct layout_ctx *ctx )
{
	int c;
	LONG vb;
	ULONG rgb[ 3 ];
	ULONG shadowrgb[ 3 ], shinergb[ 3 ];
	int shinesame = TRUE, shadowsame = TRUE;

	rgb[ 0 ] = bgcolor >> 16;
	rgb[ 1 ] = ( bgcolor >> 8 ) & 0xff;
	rgb[ 2 ] = bgcolor & 0xff;

	for( c = 0; c < 3; c++ )
	{
		vb = rgb[ c ];

		if( vb )
		{
			shadowsame = FALSE;
		}

		vb = max( 0, vb - 90 ); 
		shadowrgb[ c ] = vb;

		vb = rgb[ c ];
		if( vb < 255 )
			shinesame = FALSE;

		vb = min( 255, vb + 90 );   
		shinergb[ c ] = vb;
	}

	if( shadowsame )
	{
		shadowrgb[ 0 ] = 0x25;
		shadowrgb[ 1 ] = 0x25;
		shadowrgb[ 2 ] = 0x25;
	}
	if( shinesame )
	{
		shinergb[ 0 ] = 0xdd;
		shinergb[ 1 ] = 0xdd;
		shinergb[ 2 ] = 0xdd;
	}

	ctx->penspec_borderlight = shinergb[ 2 ] << 16 | shinergb[ 1 ] << 8 | shinergb[ 0 ];
	ctx->penspec_borderdark = shadowrgb[ 2 ] << 16 | shadowrgb[ 1 ] << 8 | shadowrgb[ 0 ];
}

//
// -----------------------------------------------------------------------
//
// Font style stack management
//

void layout_popstyle( struct layout_ctx *ctx )
{
	struct layout_style *style;

	style = REMTAIL( &ctx->fontstyles );

	if( ISLISTEMPTY( &ctx->fontstyles ) )
	{
		ADDTAIL( &ctx->fontstyles, style );
	}
	else
		l_free( ctx, style );

	ctx->currentstyle = LASTNODE( &ctx->fontstyles );
}

struct layout_style *layout_pushstyle( struct layout_ctx *ctx )
{
	struct layout_style *style, *prev;

	style = l_malloc( ctx, sizeof( *style ) );
	if( !ISLISTEMPTY( &ctx->fontstyles ) )
	{
		prev = LASTNODE( &ctx->fontstyles );
		*style = *prev;
	}

	ADDTAIL( &ctx->fontstyles, style );
	ctx->currentstyle = style;
	return( style );
}

void layout_styleopenfont( struct layout_ctx *ctx )
{
#if USE_LIBUNICODE
	// Evil hack-O-Rama FIXME FIXME FIXME
	if( ctx->iso_charset != UCCS_LATIN1 )
	{
		ctx->currentstyle->font = getfont( "Unicode", ctx->currentstyle->fontstepsize, &ctx->currentstyle->fontarray );
		return;
	}
#endif
	ctx->currentstyle->font = getfont( ctx->currentstyle->face, ctx->currentstyle->fontstepsize, &ctx->currentstyle->fontarray );
}

//
// Charset conversion stuff
//

//
// Helper function for specifying charset type
//

#if USE_LIBUNICODE

struct lcf {
	char *charset;
	int code;
} lcftable[] = {
	{ "UTF-8", 			UCCS_UTF8 },
	{ "UTF8", 			UCCS_UTF8 },
	{ "Java-Utf-8", 	UCCS_UTF8_JAVA },
/*
	// Can't use those since we can only deal with 8 bit chars
	{ "UCS2-big", 		UCCS_UTF16_BE },
	{ "UCS2-little", 	UCCS_UTF16_LE },
	{ "UCS2", 			UCCS_UTF16 },
	{ "UCS-2", 			UCCS_UTF16 },
	{ "UCS4-big", 		UCCS_UTF32_BE },
	{ "UCS4-little", 	UCCS_UTF32_LE },
	{ "UCS4-native", 	UCCS_UTF32 },
*/

	{ "ASCII", 			UCCS_ASCII },
	{ "iso-8859-1", 	UCCS_LATIN1 },
	{ "latin1", 		UCCS_LATIN1 },
	{ "iso-8859-2", 	UCCS_ISO_8859_2 },
	{ "iso-8859-3", 	UCCS_ISO_8859_3 },
	{ "iso-8859-4", 	UCCS_ISO_8859_4 },
	{ "iso-8859-5", 	UCCS_ISO_8859_5 },
	{ "iso-8859-6", 	UCCS_ISO_8859_6 },
	{ "iso-8859-7", 	UCCS_ISO_8859_7 },
	{ "iso-8859-8", 	UCCS_ISO_8859_8 },
	{ "iso-8859-9", 	UCCS_ISO_8859_9 },
	{ "iso-8859-10", 	UCCS_ISO_8859_10 },
	{ "iso-8859-14", 	UCCS_ISO_8859_14 },
	{ "iso-8859-15", 	UCCS_ISO_8859_15 },
	{ "cp1252", 		UCCS_WINDOWS_1252 },
	{ "windows-1252",	UCCS_WINDOWS_1252 },
	{ "koi8-r", 		UCCS_KOI8_R },
	{ "koi8-u", 		UCCS_KOI8_U },
	{ "tis620.2533-1", 	UCCS_TIS_620 },
	{ "armscii-8", 		UCCS_ARMSCII_8 },
	{ "georgian-academy", UCCS_GEORGIAN_AC },
	{ "georgian-ps", 	UCCS_GEORGIAN_PS },
	{ "Shift-JIS", 		UCCS_SJIS },
	{ "CP932", 			UCCS_CODEPAGE_932 },
	{ 0, 0 }
};

int charset_to_iso_code( char *mimetype )
{
	char buffer[ 256 ];
	char *p, *p2;
	struct lcf *lcf = lcftable;

	D( db_html, bug( "parsing content type '%s'\n", mimetype ) );

	stccpy( buffer, mimetype, sizeof( buffer ) );
	strlwr( buffer );

	p = strstr( buffer, "charset" );
	if( !p )
		return( UCCS_LATIN1 );
	p+= 7;
	p = stpblk( p );
	if( *p != '=' )
		return( 1 );
	p = stpblk( ++p );
	p2 = strpbrk( p, " ;," );
	if( p2 )
		*p2 = 0;

	// p now points to the charset spec
	while( lcf->charset )
	{
		if( !stricmp( lcf->charset, p ) )
			return( lcf->code );
		lcf++;
	}

	// Assume ISO-8859-1
	return( UCCS_LATIN1 );
}

int layout_charsetconv( struct layout_ctx *ctx, char *from, char *to, int fromsize )
{
	int tosize = 0;

	if( !ctx->uconv )
	{
		memcpy( to, from, fromsize );
		to[ fromsize ] = 0;
		return( fromsize );
	}
	#define CUCBLOCKSIZE 128

	while( fromsize > 0 )
	{
		int tmp;

		tmp = ConvertUCString( ctx->uconv, from, min( CUCBLOCKSIZE, fromsize ), to, 1024 );
		if( tmp < 0 )
		{
			D( db_html, bug( "ConvertUCString() failed! %ld\n", tmp ) );
			break;
		}
		from += CUCBLOCKSIZE;
		fromsize -= CUCBLOCKSIZE;

		to += tmp;
		tosize += tmp;
	}
	*to = 0;
	return( tosize );
}
#endif

//
// Fetch nodes -- sub documents
//
struct nstream *layout_hasdoc( struct layout_ctx *ctx, char *url )
{
	struct layout_fetchnode *fn;

	for( fn = FIRSTNODE( &ctx->fetchlist ); NEXTNODE( fn ); fn = NEXTNODE( fn ) )
	{
		if( !strcmp( fn->url, url ) )
		{
			if( nets_state( fn->ns ) )
				return( fn->ns );
			else
				return( NULL );
		}
	}

	fn = l_malloc( ctx, sizeof( *fn ) );
	fn->url = l_strdup( ctx, url );
	fn->ns = nets_open( 
		url,
		ctx->baseref,
		ctx->dom_document,
		NULL,
		NULL,
		0,
		0
	);
	nets_settomem( fn->ns );
	fn->waiting_for = TRUE;
	ADDTAIL( &ctx->fetchlist, fn );

	return( NULL );
}

int layout_checkfetchnodes( struct layout_ctx *ctx )
{
	struct layout_fetchnode *fn;

	for( fn = FIRSTNODE( &ctx->fetchlist ); NEXTNODE( fn ); fn = NEXTNODE( fn ) )
	{
		if( fn->waiting_for )
		{
			if( nets_state( fn->ns ) )
			{
				fn->waiting_for = FALSE;
			}
			else
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );
}
