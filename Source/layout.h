/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_LAYOUT_H
#define VOYAGER_LAYOUT_H
/*

	VLayout 2.0
	-----------
	V HTML Layout engine

	(C) 2000-2001 by Oliver Wagner, David Gerber, Jon Bright

	$Id: layout.h,v 1.58 2001/07/01 22:02:58 owagner Exp $

*/

#include "voyager.h"

#define TEXTBUFFER_INITIALSIZE 1024
#define TEXTBUFFER_INCREMENT 8192

#define LO_NOPEN (~0)
#define LO_GENLOCK ((ULONG)-2)

// Special fake styles

#define FSF_VISITEDLINK 0x100

#ifndef FSF_STRIKE
#define FSF_STRIKE 0x200
#endif

#ifndef FSF_PRE
#define FSF_PRE 0x400
#endif

//
// This defines a node in a style tree
// The parent pointer is used for inheritation
//
struct layout_style {
	struct MinNode n;
	struct TextFont *font;
	UBYTE *fontarray;
	char face[ 256 ];
	int fontstepsize;
	ULONG color;
	ULONG fontstyle;
};

struct layout_meta {
	struct MinNode n;
	char *header;
	char *value;
};

struct layout_pen {
	struct MinNode n;
	ULONG rgb;
	ULONG pen;
};

struct layout_li {
	struct MinNode n;
	int type;
	int shape;
	int count;
	int depth;
};

#define LINEALIGNSTACKSIZE 256
#define LINEALIGNTYPES 4

enum linealign_types {
	latype_div,
	latype_p,
	latype_center,
	latype_h
};

struct layout_button {
	APTR previous_container;
};

struct layout_table {
	struct MinNode n;
	
	struct MinList previous_stylestack;
	struct MinList previous_listack;
	APTR tableobject;
	APTR previous_container;
	APTR previous_anchor;
	int previous_lastwasblank;
	int previous_linealign[ LINEALIGNTYPES ][ LINEALIGNSTACKSIZE ];
	int previous_linealignptr[ LINEALIGNTYPES ];
	int previous_current_linealign;
	int previous_mode_pre, previous_mode_xmp;
	int previous_active_p, previous_active_div;
	int previous_mode_nobr;

	int cellpadding, cellspacing, border;

	int penspec_border_dark, penspec_border_light;

	int rowcount;
	int inrow;

	char *tr_bgcolor, *tr_bgimage, *tr_valign, *tr_align;
};

struct layout_ctx {
	int is_attached;
	APTR attached_to;

	APTR dom_win, dom_document;

	struct MinList l;	// Object list (when not attached)

	struct MinList meta;	// Document meta information
	char *meta_refresh_url;
	int meta_refresh_period;

	struct MinList tablestack;
	struct layout_table *currenttable;

	struct MinList listack;
	struct layout_li *current_li;

	struct layout_button *current_button;

	APTR currentform;
	APTR currentselect;

	struct MinList fontstyles;	// Font style stack
	struct layout_style *currentstyle;

	struct MinList pens;
	struct ColorMap *cmap;	// Colormap for pens

	APTR pool;
	struct SignalSemaphore s;

	char title[ 256 ];
	int htmlsourceline;

	APTR body;			// Body container object
	APTR current_container;
	int seen_body;

	APTR current_anchor;
	time_t current_anchor_visited;

	APTR current_imagemap;

	// Parsing modes
	int mode_xmp, mode_pre, mode_nobr;
	int lastwasblank;

	int linealign[ LINEALIGNSTACKSIZE ][ LINEALIGNTYPES ];
	int linealign_ptr[ LINEALIGNTYPES ];
	int current_linealign;
	int active_p, active_div;
	int seen_text;

	char *textbuffer;
	int textbuffersize;
	int textbufferoffset;

	int done;

	// HTML related stuff
	char *baseref;
	char *basetarget;

	// Default styles
	ULONG penspec_background;
	ULONG penspec_text;
	ULONG penspec_link;
	ULONG penspec_alink;
	ULONG penspec_vlink;
	ULONG penspec_borderdark;
	ULONG penspec_borderlight;

	int	margin_left, margin_top, margin_right, margin_bottom;

	// Form IDs
	int form_id;		// Current form ID
	int form_eid;		// Current form element ID

	// font specification
	int iso_charset;

	// Objects being waited for (<... src=xxx>
	struct MinList fetchlist;

	// Cache management
	time_t cache_expires;
	int cache_no;

#if USE_LIBUNICODE
	// Currently allocated font converter
	UCHandle_p uconv;
	// For which iso_charset source is the font converter allocated?
	int uconv_source;
#endif
};

struct layout_fetchnode {
	struct MinNode n;
	int waiting_for;
	struct nstream *ns;
	char *url;
};

//
// Functions
//

void layout_attach( struct layout_ctx *ctx, APTR obj );
void layout_setdom( struct layout_ctx *ctx, APTR dom_win, APTR dom_document );
struct layout_ctx *layout_new( void );
void layout_delete( struct layout_ctx *ctx );
void layout_addobj( struct layout_ctx *ctx, APTR o );
void layout_remobj( struct layout_ctx *ctx, APTR o );
void layout_setbaseref( struct layout_ctx *ctx, char *baseref );
void layout_setbasetarget( struct layout_ctx *ctx, char *basetarget );
void layout_setmargins( struct layout_ctx *ctx, int l, int r, int t, int b );
char *layout_getmeta( struct layout_ctx *ctx, char *header, int num );

STRPTR l_strdup( struct layout_ctx *ctx, char *str );
APTR l_calloc( struct layout_ctx *ctx, int size );
APTR l_malloc( struct layout_ctx *ctx, int size );
void l_free( struct layout_ctx *ctx, APTR what );
void l_readstrtag( struct TagItem *tag, char **str );
APTR l_realloc( struct layout_ctx *ctx, APTR old, int newsize );

int layout_do( struct layout_ctx *ctx, STRPTR text, int textlen, int offset, int is_complete );
int layout_do_text( struct layout_ctx *ctx, STRPTR text, int textlen, int offset, int is_complete );

#ifndef MBX
ULONG layout_getpen( struct layout_ctx *ctx, ULONG rgb );
void layout_freepens( struct layout_ctx *ctx );
void layout_setuppens( struct layout_ctx *ctx, struct ColorMap *cmap );
#else
#define layout_freepens(ctx)
#define layout_setuppens(ctx,cmap)
#define layout_getpen(ctx,rgb) rgb
#endif

void layout_make_shadow_shine( ULONG bgcolor, struct layout_ctx *ctx );

void layout_popstyle( struct layout_ctx *ctx );
struct layout_style *layout_pushstyle( struct layout_ctx *ctx );
void layout_styleopenfont( struct layout_ctx *ctx );

int charset_to_iso_code( char *mimetype );
int layout_charsetconv( struct layout_ctx *ctx, char *from, char *to, int fromsize );

struct nstream *layout_hasdoc( struct layout_ctx *ctx, char *url );
int layout_checkfetchnodes( struct layout_ctx *ctx );

#endif /* VOYAGER_LAYOUT_H */
