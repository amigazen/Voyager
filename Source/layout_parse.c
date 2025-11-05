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
	V HTML4 Layout engine

	(C) 2000-2003 by Oliver Wagner, David Gerber, Jon Bright

	$Id: layout_parse.c,v 1.297 2004/01/06 20:23:08 zapek Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif
#include <limits.h>

#ifdef MBX
#include <modules/mbxgui/classes.h>
#endif

/* private */
#include "layout.h"
#include "html.h"
#include "classes.h"
#include "htmlclasses.h"
#include "fontcache.h"
#include "urlparser.h"
#include "parse.h"
#include "malloc.h"
#include "history.h"
#include "prefs.h"
#include "colortable.h"
#include "js.h"
#include "form.h"
#include "mui_func.h"
#include "network.h"
#include "textinput.h"

//
// JS support functions
//

void js_doeventhandlers( struct layout_ctx *ctx, APTR obj )
{
	static STRPTR tab[] = {
		"ONABORT",
		"ONBLUR",
		"ONCHANGE",
		"ONCLICK",
		"ONDBLCLICK",
		"ONDRAGDROP",
		"ONERROR",
		"ONFOCUS",
		"ONKEYDOWN",
		"ONKEYUP",
		"ONLOAD",
		"ONMOUSEDOWN",
		"ONMOUSEMOVE",
		"ONMOUSEOUT",
		"ONMOUSEOVER",
		"ONMOUSEUP",
		"ONMOVE",
		"ONRESET",
		"ONRESIZE",
		"ONSELECT",
		"ONSUBMIT",
		"ONUNLOAD",
#ifdef MBX
		"ONDISKCHANGE",
#endif
		NULL
	};
	int c;

#if USE_JS
	if( !gp_javascript )
#endif
		return;

	if( !obj )
		return;

	for( c = 0; tab[ c ]; c++ )
	{
		char *p = getargs( tab[ c ] );
		int funcindex;
		char tmp[ 32 ];

		if( !p )
			continue;

		strcpy( tmp, tab[ c ] );
		strlwr( tmp );

		if( DoMethod( obj, MM_JS_HasProperty, tmp ) != expt_funcptr )
			continue;

		// Lame Webdesigner Mega-Kludge
		if( !strnicmp( p, "javascript:", 11 ) )
			p += 11;

		//Printf( "JS: compiling %s handler for object %lx\n", tab[ c ], obj );

		funcindex = js_compile(
			ctx->dom_win,
			cjsol, p, strlen( p ),
			ctx->htmlsourceline,
			ctx->baseref
		);
		if( funcindex >= 0 )
		{
				DoMethod( obj, MM_JS_SetProperty, tmp, &funcindex, sizeof( funcindex ), expt_funcptr );
		}
	}
}

/* case ht_frameset: */
static void do_frameset( APTR container, struct layout_ctx *ctx, char **data, int defframeborder, int defborder )
{
	char *colspec = getargs( "COLS" );
	char *rowspec = getargs( "ROWS" );
	int ch;
	APTR fso;
	int frameborder = getboolarg( "FRAMEBORDER", defframeborder );
	char *bordercolor = getargs_ne( "BORDERCOLOR" );
	ULONG bgrgb = 0x999999;
	int border = getnumargp( "BORDER", getnumargp( "FRAMESPACING", defborder ) );

	if( bordercolor )
	{
		bgrgb = colspec2rgb24( bordercolor );
	}

#if USE_LO_PIP
	{
		char *bgimage = getargs( "BACKGROUND" );
		if( bgimage && !stricmp( bgimage, "tv:" ) )
		{
			APTR pip;
			char *chname = getargs( "CHANNELNAME" );
			int chid = getnumarg( "CHANNELID", -1 );

			set( ctx->body, MA_Layout_BGColor, LO_GENLOCK );

			get( ctx->dom_win, MA_HTMLWin_BGPip, &pip );

			if( pip )
			{
				SetAttrs( pip,
					chname ? MUIA_Pip_ChannelName : TAG_IGNORE, chname,
					chid >= 0 ? MUIA_Pip_ChannelId : TAG_IGNORE, chid,
					( chname || chid >= 0 ) ? MUIA_Pip_AutoTune : TAG_IGNORE, TRUE,
 					TAG_DONE
 				);
			}
			else
			{
				pip = NewObject( getlopipclass(), NULL,
					MUIA_Pip_ChannelName, chname,
					MUIA_Pip_AutoTune, TRUE,
					MA_Layout_Context, ctx,
					MUIA_Pip_Background, TRUE,
					MA_Layout_Image_Width, 1,
					MA_Layout_Image_Height, 1,
					TAG_DONE
				);
				set( ctx->dom_win, MA_HTMLWin_BGPip, pip );
			}
		}
		else
			set( ctx->dom_win, MA_HTMLWin_BGPip, NULL );
	}
#endif

	// Create the body container object
	fso = NewObject( getloframesetclass(), NULL,
		MA_Layout_MarginLeft, 0,
		MA_Layout_MarginRight, 0,
		MA_Layout_MarginTop, 0,
		MA_Layout_MarginBottom, 0,
		MA_Layout_Context, ctx,
		MA_Layout_BGColor, bgrgb,
		MA_Layout_Frame_Horizspec, colspec ? colspec : "*",
		MA_Layout_Frame_Vertspec, rowspec ? rowspec : "*",
		MA_Layout_Frame_Frameborder, frameborder,
		MA_Layout_Frame_Border, border,
		TAG_DONE
	);
	DoMethod( container, MM_Layout_Group_AddObject, fso );

	// Read individual frames, and possibly sub-framesets
	while( ( ch = gettoken( data, &ctx->htmlsourceline ) ) )
	{
		if( !ch )
			break;
		if( ch == ( ht_frameset | HTF_NEGATE ) )
		{
			// End of this frameset
			break;
		}
		if( ch == ht_frame )
		{
			// Handle one frame
			char *name = getargs( "NAME" );
			char *src = getargs( "SRC" );
			char *scrolling = getargs( "SCROLLING" );
			char urlbuffer[ MAXURLSIZE ];
			APTR o;
			int scrollval = 0;
			int thisframeborder = getboolarg( "FRAMEBORDER", frameborder );
			int marginwidth = getnumargp( "MARGINWIDTH", 10 );
			int marginheight = getnumargp( "MARGINHEIGHT", 15 );

			if( !src || !*src )
				src = "";
			if( !name )
				name = "_unnamed";

			if( scrolling )
			{
				if( !stricmp( scrolling, "YES" ) )
					scrollval = 1;
				else if( !stricmp( scrolling, "NO" ) )
					scrollval = 2;
			}

			uri_mergeurl( ctx->baseref, src, urlbuffer );

			if(!strcmp(ctx->baseref,urlbuffer))
			{
				// If there's a loop, skip
				src="";
			}


			//kprintf( "creating frame src=%s name=%s %ld %ld\n", urlbuffer, name, *row, *col );
			// TOFIX! Owner?
			o = win_create( name, *src ? urlbuffer : "", ctx->baseref, NULL, TRUE, FALSE, FALSE );
			SetAttrs( o,
				MA_HTMLWin_OrigBaseref, ctx->baseref,
				MA_HTMLWin_Scrolling, scrollval,
				MA_HTMLWin_DrawFrame, thisframeborder ? -1 : 0,
				MA_Layout_MarginLeft, marginwidth,
				MA_Layout_MarginRight, marginwidth,
				MA_Layout_MarginTop, marginheight,
				MA_Layout_MarginBottom, marginheight,
				TAG_DONE
			);
			DoMethod( fso, MM_Layout_Group_AddObject, o );
		}
		if( ch == ht_frameset )
		{
			// Sub frameset
			do_frameset( fso, ctx, data, frameborder, border );
		}
	}
}

static int do_title( struct layout_ctx *ctx, char **data )
{
	char *olddata = *data;
	int ch;
	int offs = 0;
	int oldline = ctx->htmlsourceline;

	while( ch = gettoken( data, &ctx->htmlsourceline ) )
	{
		if( ch > 255 )
			break;
		if( ch == '\n' )
			ctx->htmlsourceline++;
		if( isspace( ch ) )
			ch = ' ';
		if( offs < sizeof( ctx->title ) - 1 )
			ctx->title[ offs++ ] = ch;
	}

	while( offs )
	{
		if( isspace( ctx->title[ offs - 1 ] ) )
			ctx->title[ --offs ] = 0;
		else
			break;
	}

#if USE_LIBUNICODE
	{
		char tmp[ 1024 ];

		layout_charsetconv( ctx, ctx->title, tmp, strlen( ctx->title ) );
		stccpy( ctx->title, tmp, sizeof( ctx->title ) );
	}
#endif

	if( ch < 256 )
	{
		*data = olddata;
		ctx->title[ 0 ] = 0;
		ctx->htmlsourceline = oldline;
		return( TRUE ); // Incomplete, abort
	}
	else
	{
		return( FALSE );
	}
}

/* case ht_option: */
static int do_option( struct layout_ctx *ctx, char **data )
{
	char *olddata = *data;
	int ch;
	int offs = 0;
	char optionname[ 256 ];
	char *beforedata = olddata;
#if USE_LIBUNICODE
	char optionname2[ 1024 ];
#endif
	char *val = getargs( "VALUE" );
	int selected = getargs( "SELECTED" ) ? 1 : 0;
	int oldline = ctx->htmlsourceline;

	// Need to copy it, as gettoken() will destroy the value
	if( val )
		val = l_strdup( ctx, val );

	while( ch = gettoken( data, &ctx->htmlsourceline ) )
	{
		if( ch > 255 )
		{
			ch &= ~HTF_NEGATE;

			if( ch != ht_b && ch != ht_i && ch != ht_u )
				break;
			else
				continue;
		}
		if( ch == '\n' )
			ctx->htmlsourceline++;
		if( isspace( ch ) )
			ch = ' ';
		if( offs < sizeof( optionname ) - 1 )
			optionname[ offs++ ] = ch;

		beforedata = *data;
	}

	while( offs )
	{
		if( isspace( optionname[ offs - 1 ] ) )
			--offs;
		else
			break;
	}

	optionname[ offs ] = 0;

#if USE_LIBUNICODE
	layout_charsetconv( ctx, optionname, optionname2, strlen( optionname ) );
#endif

	if( ch < 256 )
	{
		*data = olddata;
		ctx->htmlsourceline = oldline;
		return( TRUE ); // Incomplete, abort
	}

#if USE_LIBUNICODE
	DoMethod( ctx->currentselect, MM_Layout_FormCycle_AddOption,
		stpblk( optionname2 ),
		val,
		selected
	);
#else
	DoMethod( ctx->currentselect, MM_Layout_FormCycle_AddOption,
		stpblk( optionname ),
		val,
		selected
	);
#endif /* !USE_LIBUNICODE */

	*data = beforedata;

	return( FALSE );
}

static int do_skip( struct layout_ctx *ctx, char **data, char *endtag )
{
	char *olddata = *data;
	int ch;
	int endlen = strlen( endtag );
	int oldline = ctx->htmlsourceline;

	for(;;)
	{
		ch = **data;
		*data = *data + 1;
		if( !ch )
		{
			// Incomplete
			*data = olddata;
			ctx->htmlsourceline = oldline;
			return( TRUE );
		}
		if( ch == '\n' )
			ctx->htmlsourceline++;
		if( ch == '<' )
		{
			if( !strnicmp( *data, endtag, endlen ) )
				break; // End of tag
		}
	}

	// Skip closing tag
	*data = *data + endlen;
	return( FALSE );
}

//
// Used for both <SCRIPT> and <STYLE> now
//
static int do_script( struct layout_ctx *ctx, char **data, char **script, char *endtag, int doquotes )
{
	char *olddata = *data;
	int ch, prevch = 0;
	int buffersize = 1024;
	int bufferuse = 0;
	char *buffer = l_malloc( ctx, buffersize );
	int endlen = strlen( endtag );
	int oldline = ctx->htmlsourceline;
	int inquote = FALSE;

	for(;;)
	{
		ch = **data;
		*data = *data + 1;
		if( !ch )
		{
			// Incomplete
			*data = olddata;
			l_free( ctx, buffer );
			ctx->htmlsourceline = oldline;
			return( TRUE );
		}
		if( doquotes && ( ch == '\'' || ch == '\"' ) )
		{
			if( prevch != '\\' )
			{
				if( inquote && ( inquote == ch ) )
					inquote = 0;
				else if( !inquote )
					inquote = ch;
			}
		}
		if( ch == '<' && !inquote )
		{
			if( !strnicmp( *data, endtag, endlen ) )
				break; // End of script/style stuff
		}
		if( bufferuse == buffersize - 2 )
		{
			buffersize += 1024;
			buffer = l_realloc( ctx, buffer, buffersize );
		}
		buffer[ bufferuse++ ] = ch;
		if( ch == '\n' )
		{
			ctx->htmlsourceline++;
			inquote = FALSE;
		}

		prevch = ch;
	}

	buffer[ bufferuse ] = 0;
	*script = buffer;

	// Skip closing tag
	*data = *data + endlen;
	return( FALSE );
}

/* case ht_textarea: */
static int do_textarea( struct layout_ctx *ctx, char **data )
{
	char *olddata = *data;
	int ch;
	APTR o;
	char *tmp, *tmp2, *tmp3;
#if USE_LIBUNICODE
	char *tmp4;
#endif
	int dsize = 0;
	char *storedtext = formstore_get( ctx->baseref, ctx->form_eid, ctx->form_id, NULL );
	int oldline = ctx->htmlsourceline;

	for(;;)
	{
		ch = **data;
		*data = *data + 1;
		if( !ch )
		{
			// Incomplete
			*data = olddata;
			ctx->htmlsourceline = oldline;
			return( TRUE );
		}
		if( ch == '<' )
		{
			if( !strnicmp( *data, "/TEXTAREA>", 10 ) )
				break; // End of contents
		}
		if( ch == '\n' )
			ctx->htmlsourceline++;
		dsize++;
	}

	tmp = malloc( dsize + 1 );
	tmp2 = malloc( dsize + 1 );
	stccpy( tmp, olddata, dsize + 1 );
	convertentities( tmp, tmp2 );

	if( isspace( *tmp2 ) )
		tmp3 = tmp2 + 1;
	else
		tmp3 = tmp2;

#if USE_LIBUNICODE
	tmp4 = malloc( strlen( tmp3 ) * 4 );
	layout_charsetconv( ctx, tmp3, tmp4, strlen( tmp3 ) );
	tmp3 = tmp4;
#endif

	o = JSNewObject( getloformtextfieldclass(),
		MUIA_CycleChain, 1,
		MUIA_Disabled, getargs( "DISABLED" ),
		MUIA_Textinput_NoInput, getargs( "READONLY" ),
		MA_Layout_Context, ctx,
		MA_Layout_FormTextarea_Rows, getnumargp( "ROWS", 8 ),
		MA_Layout_FormTextarea_Cols, getnumargp( "COLS", 40 ),
		MA_Layout_FormElement_Form, ctx->currentform,
		MA_Layout_FormElement_Name, gettokenname(),
		MA_Layout_FormElement_Value, storedtext ? storedtext : tmp3,
		MA_Layout_FormElement_DefaultValue, tmp3,
		MA_Layout_FormElement_ID, ctx->form_id,
		MA_Layout_FormElement_EID, ctx->form_eid++,
		TAG_DONE
	);

	free( tmp2 );
	free( tmp );

	if( o )
	{
		js_doeventhandlers( ctx, o );
		DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
	}

#if USE_LIBUNICODE
	free( tmp4 );
#endif

	// Skip closing tag
	*data = *data + 10;
	return( FALSE );
}

static int get_valign( int def )
{
	char *args = getargs( "VALIGN" );

	if( !args )
		return( def );

	// Hm -- modifying calling data. Who cares ;)
	strupr( args );

	if( !strcmp( args, "BASELINE" ) )
		return( valign_baseline );
	else if( !strcmp( args, "ABSMIDDLE" ) )
		return( valign_absmiddle );
	else if( !strcmp( args, "BOTTOM" ) )
		return( valign_bottom );
	else if( !strcmp( args, "ABSBOTTOM" ) )
		return( valign_absbottom );
	else if( !strcmp( args, "MIDDLE" ) || !strcmp( args, "CENTER" ) )
		return( valign_middle );
	else if( !strcmp( args, "TEXTTOP" ) )
		return( valign_texttop );
	else if( !strcmp( args, "TOP" ) )
		return( valign_top );
	else
		return( def );
}

// You can't have nested buttons, so "unroll" isn't quite the right name. But it's consistent.
static void unrollbutton( struct layout_ctx *ctx )
{
	struct layout_button *b = ctx->current_button;
	ctx->current_container = b->previous_container;
	l_free( ctx, b );
	ctx->current_button = NULL;
}

static void unrolltable( struct layout_ctx *ctx )
{
	struct layout_table *t = ctx->currenttable;
	struct layout_style *st;
	struct layout_li *li;

	free( ctx->currenttable->tr_bgcolor );
	free( ctx->currenttable->tr_bgimage );
	free( ctx->currenttable->tr_valign );
	free( ctx->currenttable->tr_align );

	while( st = REMHEAD( &ctx->fontstyles ) )
		l_free( ctx, st );

	while( st = REMHEAD( &t->previous_stylestack ) )
		ADDTAIL( &ctx->fontstyles, st );

	ctx->currentstyle = LASTNODE( &ctx->fontstyles );

	NEWLIST( &ctx->listack );
	while( li = REMHEAD( &t->previous_listack ) )
		ADDTAIL( &ctx->listack, li );
	if( ISLISTEMPTY( &ctx->listack ) )
		ctx->current_li = NULL;
	else
		ctx->current_li = LASTNODE( &ctx->listack );

	ctx->current_container = t->previous_container;
	ctx->current_anchor = t->previous_anchor;
	ctx->lastwasblank = t->previous_lastwasblank;
	ctx->mode_xmp = t->previous_mode_xmp;
	ctx->mode_pre = t->previous_mode_pre;
	ctx->mode_nobr = t->previous_mode_nobr;

	memcpy( ctx->linealign, t->previous_linealign, sizeof( ctx->linealign ) );
	memcpy( ctx->linealign_ptr, t->previous_linealignptr, sizeof( ctx->linealign_ptr ) );
	ctx->current_linealign = t->previous_current_linealign;
	ctx->active_p = t->previous_active_p;
	ctx->active_div = t->previous_active_div;

	REMOVE( t );
	l_free( ctx, t );

	if( ISLISTEMPTY( &ctx->tablestack ) )
		ctx->currenttable = NULL;
	else
		ctx->currenttable = LASTNODE( &ctx->tablestack );
}

static void push_linealign( struct layout_ctx *ctx, int newalign, int type )
{
	if( ctx->linealign_ptr[ type ] < LINEALIGNSTACKSIZE - 1 )
	{
		ctx->linealign[ ctx->linealign_ptr[ type ]++ ][ type ] = ctx->current_linealign;
	}
	ctx->current_linealign = newalign;
}

static int pop_linealign( struct layout_ctx *ctx, int type )
{
	if( ctx->linealign_ptr[ type ] )
	{
		ctx->current_linealign = ctx->linealign[ --ctx->linealign_ptr[ type ] ][ type ];
	}
	return( ctx->current_linealign );
}

int layout_do(
	struct layout_ctx *ctx,
	STRPTR data,
	int datasize,
	int offset,
	int is_complete
)
{
	STRPTR endofdata = data + datasize;
	int ch;
	int aborthere = FALSE;
	char *begindata = data;
	char *olddata = data;
	char urlbuffer[ MAXURLSIZE ];
	int isformimage = FALSE; // for <input type=image> hack
#ifdef VDEBUG
	clock_t ts = 0;
#endif

	D( db_html, bug( "layout_do offset %ld, totalsize %ld, is_complete %ld\n", offset, datasize, is_complete, ts = clock() ));

	if( ctx->done )
		return( offset );

	if( layout_checkfetchnodes( ctx ) )
		return( offset );

	cjsol = (APTR)getv( ctx->dom_win, MA_HTMLWin_CJSOL );

#if USE_LIBUNICODE
	// Check whether we need to allocate a charset converter
	parse_setcurrentlayoutctx( ctx );

redocharsetconverter:
	D( db_html, bug( "iso charset %ld, uconv_source %ld\n", ctx->iso_charset, ctx->uconv_source ) );
	if( ctx->iso_charset != ctx->uconv_source )
	{
		FreeCharConverter( ctx->uconv );
		ctx->uconv = AllocCharConverter( ctx->iso_charset, UCCS_UTF8 );
		ctx->uconv_source = ctx->iso_charset;

		if( ctx->iso_charset != UCCS_LATIN1 )
		{
			// Create default style for body
			layout_pushstyle( ctx );
			strcpy( ctx->currentstyle->face, "Unicode" );
			ctx->currentstyle->fontstepsize = 2;
			ctx->currentstyle->fontstyle = 0;
			ctx->currentstyle->color = ctx->penspec_text;
			layout_styleopenfont( ctx );
			// Hack topmost style into submission (for following table nodes)
			if( ctx->body )
			{
				struct layout_style *topstyle = FIRSTNODE( &ctx->fontstyles );
				topstyle->font = ctx->currentstyle->font;
				strcpy( topstyle->face, "Unicode" );
			}
		}
	}

#endif

	data += offset;

	if( !ctx->body )
	{
		ULONG rgb = ctx->penspec_background;

		// Create the body container object
		ctx->body = NewObject( getlogroupclass(), NULL,
			MA_Layout_MarginLeft, ctx->margin_left,
			MA_Layout_MarginRight, ctx->margin_right,
			MA_Layout_MarginTop, ctx->margin_top,
			MA_Layout_MarginBottom, ctx->margin_bottom,
			MA_Layout_Context, ctx,
			MA_Layout_BGColor, rgb,
			TAG_DONE
		);

		if( !ctx->body )
		{
			reporterror( "can't create body object" );
			ctx->done = TRUE;
			return( offset );
		}

		layout_addobj( ctx, ctx->body );
		ctx->current_container = ctx->body;

		ctx->lastwasblank = TRUE;

		// Create default style for body
		layout_pushstyle( ctx );
		strcpy( ctx->currentstyle->face, "(Default)" );
		ctx->currentstyle->fontstepsize = 2;
		ctx->currentstyle->fontstyle = 0;
		ctx->currentstyle->color = ctx->penspec_text;
		layout_styleopenfont( ctx );
		layout_make_shadow_shine( rgb, ctx );

		set( ctx->dom_document, MA_HTMLView_IsFrameset, FALSE );
	}

	while( data < endofdata && !aborthere )
	{
		char *this_name = NULL;

		olddata = data;

		if( ctx->mode_xmp )
		{
			if( !strnicmp( data, "</xmp>", 6 ) )
			{
				ctx->mode_xmp = FALSE;
				continue;
			}
			ch = *data;
			data++;
		}
		else
		{
			ch = gettoken( (char**)&data, &ctx->htmlsourceline );
		}

		if( ch < 256 )
		{
			if( !ch )
			{
				if( !is_complete )
				{
					aborthere = TRUE;
					break;
				}
				data++;
				continue;
			}

			if( !ctx->mode_xmp && !ctx->mode_pre )
			{
				if( isspace( ch ) )
				{
					if( ctx->lastwasblank )
						continue;
					ctx->lastwasblank = TRUE;
					ch = ' ';
				}
				else
					ctx->lastwasblank = FALSE;
			}
			else if( ctx->mode_pre )
			{
				if( ch == 10 )
				{
					ch = ht_br;
					goto dotext;
				}
				else if( ch == 13 )
				{
					continue;
				}
				else if( ch == '\t' )
				{
					int addspaces = 8 - ( ctx->textbufferoffset ) % 8;

					while( addspaces-- )
					{
						if( ctx->textbufferoffset == ctx->textbuffersize )
						{
							int ns = ctx->textbuffersize + TEXTBUFFER_INCREMENT;
							char *ntb = l_malloc( ctx, ns );
							memcpy( ntb, ctx->textbuffer, ctx->textbuffersize );
							l_free( ctx, ctx->textbuffer );
							ctx->textbuffer = ntb;
							ctx->textbuffersize = ns;
						}
						ctx->textbuffer[ ctx->textbufferoffset++ ] = ' ';
					}
					continue;
				}
			}

			// Add this character to the current text buffer
			if( ctx->textbufferoffset == ctx->textbuffersize )
			{
				int ns = ctx->textbuffersize + TEXTBUFFER_INCREMENT;
				char *ntb = l_malloc( ctx, ns );
				memcpy( ntb, ctx->textbuffer, ctx->textbuffersize );
				l_free( ctx, ctx->textbuffer );
				ctx->textbuffer = ntb;
				ctx->textbuffersize = ns;
			}

			ctx->textbuffer[ ctx->textbufferoffset++ ] = ch;

			continue;
		}

dotext:
		if( ctx->textbufferoffset )
		{
#if USE_LIBUNICODE
			char *tmp;
			int newlen;

			tmp = malloc( ctx->textbufferoffset * 4 );
			newlen = layout_charsetconv( ctx, ctx->textbuffer, tmp, ctx->textbufferoffset );
			// Create a text object out of the current text buffer
			DoMethod( ctx->current_container,
				MM_Layout_Group_AddText,
				tmp, newlen,
				ctx->currentstyle->font,
				ctx->currentstyle->fontarray,
				ctx->currentstyle->color,
				ctx->currentstyle->fontstyle | ( ( ctx->mode_pre || ctx->mode_nobr ) ? FSF_PRE : 0 ),
				ctx->current_anchor
			);
			ctx->textbufferoffset = 0;
			free( tmp );
#else
			// Create a text object out of the current text buffer
			DoMethod( ctx->current_container,
				MM_Layout_Group_AddText,
				ctx->textbuffer, ctx->textbufferoffset,
				ctx->currentstyle->font,
				ctx->currentstyle->fontarray,
				ctx->currentstyle->color,
				ctx->currentstyle->fontstyle | ( ( ctx->mode_pre || ctx->mode_nobr ) ? FSF_PRE : 0 ),
				ctx->current_anchor
			);
			ctx->textbufferoffset = 0;
#endif
		}

		this_name = gettokenname();

		switch( ch )
		{
			case ht_title:
				if( do_title( ctx, (char**)&data ) )
					aborthere = TRUE;
				break;

			case ht_meta:
				{
					char *content = getargs( "CONTENT" );
					char *http_equiv = getargs( "HTTP-EQUIV" );

					if( http_equiv && content )
					{
						strupr( http_equiv );
						if( !strcmp( http_equiv, "REFRESH" ) )
						{
							char *url;

							ctx->meta_refresh_period = atoi( content );

							url = strstr( content, "URL=" );
							if( !url )
								url = strstr( content, "url=" );
							if( !url )
								url = strstr( content, "Url=" );
							if( url )
							{
								uri_mergeurl( ctx->baseref, stpblk( url + 4 ), urlbuffer );
								ctx->meta_refresh_url = l_strdup( ctx, urlbuffer );
								//Printf( "setting up refresh period %ld url %s\n", ctx->meta_refresh_period, ctx->meta_refresh_url );
							}
						}
						else if( !strcmp( http_equiv, "PRAGMA" ) )
						{
							if( !stricmp( content, "NO-CACHE" ) )
								ctx->cache_no = TRUE;
						}
#if USE_LIBUNICODE
						else if( !strcmp( http_equiv, "CONTENT-TYPE" ) )
						{
							int iso_charset;
							iso_charset = charset_to_iso_code( content );
							if( iso_charset != ctx->iso_charset )
							{
								ctx->iso_charset = iso_charset;
								goto redocharsetconverter;
							}
						}
#endif
					}
					if( this_name )
					{
						struct layout_meta *lm;

						lm = l_malloc( ctx, sizeof( *lm ) );
						lm->header = l_strdup( ctx, this_name );
						if( content )
							lm->value = l_strdup( ctx, content );

						D( db_html, bug( "got meta header = %s, value = %s\n", this_name, content ) );

						ADDTAIL( &ctx->meta, lm );
					}
				}
				break;

			case ht_base:
				{
					char *href = getargs( "HREF" );
					char *target = getargs( "TARGET" );

					if( href )
					{
						uri_mergeurl( ctx->baseref, href, urlbuffer );
						layout_setbaseref( ctx, urlbuffer );
					}
					if( target )
					{
					 	layout_setbasetarget( ctx, target );
					}
				}
				break;

			case ht_body:
				{
					char *bgcolor = getargs_ne( "BGCOLOR" );
					char *bgimage = getargs( "BACKGROUND" );
					char *tcolor = getargs_ne( "TEXT" );
					char *lcolor = getargs_ne( "LINK" );
					char *vcolor = getargs_ne( "VLINK" );
					char *acolor = getargs_ne( "ALINK" );
					char *leftmargin = getargs( "LEFTMARGIN" );
					char *topmargin = getargs( "TOPMARGIN" );
					char *rightmargin = getargs( "RIGHTMARGIN" );
					char *bottommargin = getargs( "BOTTOMMARGIN" );
					// Netscape crap
					char *marginwidth = getargs( "MARGINWIDTH" );
					char *marginheight = getargs( "MARGINHEIGHT" );
					int l = -1, r = -1, t = -1, b = -1;

					if( ctx->seen_body )
						break;

					ctx->seen_body = TRUE;

					if( bgcolor )
					{
						ULONG rgb = colspec2rgb24( bgcolor );
						set( ctx->body, MA_Layout_BGColor, rgb );
						layout_make_shadow_shine( rgb, ctx );
					}

					if( tcolor )
					{
						ctx->penspec_text = colspec2rgb24( tcolor );
						ctx->currentstyle->color = ctx->penspec_text;
					}
					if( acolor )
						ctx->penspec_alink = colspec2rgb24( acolor );
					if( vcolor )
						ctx->penspec_vlink = colspec2rgb24( vcolor );
					if( lcolor )
						ctx->penspec_link = colspec2rgb24( lcolor );

#if USE_LO_PIP
					if( !bgcolor && !bgimage )
					{
						APTR haspip = NULL;

						get( ctx->dom_win, MA_HTMLWin_BGPip, &haspip );
						if( haspip )
						{
							set( ctx->body, MA_Layout_BGColor, LO_GENLOCK );
						}
					}
#endif

					if( bgimage )
					{
#if USE_LO_PIP
						if( !stricmp( bgimage, "tv:" ) )
						{
							APTR pip;
							char *chname = getargs( "CHANNELNAME" );

							set( ctx->body, MA_Layout_BGColor, LO_GENLOCK );

							get( ctx->dom_win, MA_HTMLWin_BGPip, &pip );

							if( pip )
							{
								SetAttrs( pip,
									MUIA_Pip_ChannelName, chname,
									MUIA_Pip_AutoTune, TRUE,
									TAG_DONE
								);
							}
							else
							{
								pip = NewObject( getlopipclass(), NULL,
									MUIA_Pip_ChannelName, chname,
									MUIA_Pip_AutoTune, TRUE,
									MA_Layout_Context, ctx,
									MUIA_Pip_Background, TRUE,
									MA_Layout_Image_Width, 1,
									MA_Layout_Image_Height, 1,
									TAG_DONE
								);
								set( ctx->dom_win, MA_HTMLWin_BGPip, pip );
							}
						}
						else
#endif
						if( getflag( VFLG_LOAD_IMAGES ) )
						{
							uri_mergeurl( ctx->baseref, bgimage, urlbuffer );
							set( ctx->body, MA_Layout_BGImageURL, urlbuffer );
							set( ctx->body, MA_Layout_BGImageURLName, urlbuffer );
#if USE_LO_PIP
							set( ctx->dom_win, MA_HTMLWin_BGPip, NULL );
#endif

						}
						else
						{
#if USE_LO_PIP
							set( ctx->dom_win, MA_HTMLWin_BGPip, NULL );
#endif

							set( ctx->body, MA_Layout_BGImageURLName, urlbuffer );
						}
					}

					// marginwidth sets left and right margins
					if( marginwidth )
					{
						l = atoi( marginwidth );
						r = l;
					}

					// marginheight sets top and bottom margins
					if( marginheight )
					{
						t = atoi( marginheight );
						b = t;
					}

					// leftmargin sets left margin, and if rightmargin is not
					// set then it acts like marginwidth. if marginwidth is set
					// then leftmargin will override it here.
					if( leftmargin )
					{
						l = atoi( leftmargin );
						r = l;
					}
					else
						l = r = -1;

					// rightmargin will override the previous assignment
					if( rightmargin )
						r = atoi( rightmargin );

					// topmargin sets the top margin, and if bottommargin is
					// not set then it acts like marginheight. if marginheight
					// is set them topmargin will override it here.
					if( topmargin )
					{
						t = atoi( topmargin );
						b = t;
					}
					else
						t = b = -1; // Internet Explorer default

					// bottommargin will override the previous assignment
					if( bottommargin )
						b = atoi( bottommargin );

					SetAttrs( ctx->body,
							(l > -1) ? MA_Layout_MarginLeft : TAG_IGNORE, l,
							(r > -1) ? MA_Layout_MarginRight : TAG_IGNORE, r,
							(t > -1) ? MA_Layout_MarginTop : TAG_IGNORE, t,
							(b > -1) ? MA_Layout_MarginBottom : TAG_IGNORE, b,
							TAG_DONE
					);

					js_doeventhandlers( ctx, ctx->dom_document );
				}
				break;

			case ht_script:
				{
					char *script_src = NULL, *script = NULL;
					char *src = getargs( "SRC" );
					char *lang = getargs( "LANGUAGE" );
					char *pd;
					int startline = ctx->htmlsourceline;
					char *docurl = ctx->baseref;


					if( !lang )
						lang = "JAVASCRIPT";


					if ( src )
					{
						struct nstream *ns;
						char urlbuffer[ MAXURLSIZE ];

						#if 0
						/*
							if the document url is not file://.. and the
							script is loaded from file:// (i.e. remote
							pages accessing local ones) then disallow it
							for security reasons.

							netscape has a hidden option to allow this..
						*/

						struct parsed_url docsrc, splitsrc;

						uri_split(docurl, &docsrc);
						uri_split(src, &splitsrc);

						if ( strncmp(docsrc.scheme, "file", 4 && !strcmp(splitsrc.scheme, "file", 4))
						{
							aborthere = TRUE;
							break;
						}

						/*
							carry on as normal
						*/
						#endif

						uri_mergeurl( ctx->baseref, src, urlbuffer );
						ns = layout_hasdoc( ctx, urlbuffer );
						if( ns )
						{
							if( nets_state( ns ) > 0 )
							{
								script_src = nets_getdocmem( ns );
								startline = 1;
								docurl = nets_url( ns );
							}
							else
							{
								// hm, can't fetch SRC
								break;
							}
						}
						else
						{
							aborthere = TRUE;
							break;
						}
					}

					if( do_script( ctx, (char**)&data, &script, "/SCRIPT>", TRUE ) )
					{
						aborthere = TRUE;
						break;
					}

					// Process JScript
					// TOFIX -- reference objects
#if USE_JS
					if( gp_javascript && !strnicmp( lang, "JAVASCRIPT", 10 ) )
					{
						set( ctx->dom_document, MA_HTMLView_InLayout, TRUE );
						if( script_src && *script_src )
						{
							js_run(
								ctx->dom_win,
								//ctx->dom_document,
								ctx->dom_win,
								ctx->baseref,
								script_src, strlen( script_src ),
								startline,
								urlbuffer, sizeof( urlbuffer ),
								docurl
							);
						}
						if( script && *script )
						{
							js_run(
								ctx->dom_win,
								//ctx->dom_document,
								ctx->dom_win,
								ctx->baseref,
								script, strlen( script ),
								startline,
								urlbuffer, sizeof( urlbuffer ),
								docurl
							);
						}
						set( ctx->dom_document, MA_HTMLView_InLayout, FALSE );
						pd = (char*)DoMethod( ctx->dom_document, MM_HTMLView_GetPushedData );
						if( pd )
						{
							int oldlineno = ctx->htmlsourceline;
							layout_do( ctx, pd, strlen( pd ), 0, 2 );
							ctx->htmlsourceline = oldlineno;
							free( pd );
						}

					}
#endif /* USE_JS */
				}
				break;

			case ht_noscript:
#if USE_JS
				if( gp_javascript )
				{
					if( do_skip( ctx, (char**)&data, "/noscript>" ) )
					{
						aborthere = TRUE;
					}
				}
#endif /* USE_JS */
				break;

			case ht_noframes:
				if( do_skip( ctx, (char**)&data, "/noframes>" ) )
				{
					aborthere = TRUE;
				}
				break;

			case ht_noembed:
				if( do_skip( ctx, (char**)&data, "/noembed>" ) )
				{
					aborthere = TRUE;
				}
				break;

			case ht_style:
				{
					char *script;
					char *src = getargs( "SRC" );

					if( src )
						break;

					if( do_script( ctx, (char**)&data, &script, "/STYLE>", FALSE ) )
					{
						aborthere = TRUE;
						break;
					}
					// Process Style stuff
					// TOFIX!
				}
				break;

			// Line breaks, all kind of

			case ht_hr:
				{
					APTR o;
					char *widthspec = getargs( "WIDTH" );
					int size = getnumargp( "SIZE", 2 );
					char *noshade = getargs( "NOSHADE" );
					char *penspec = getargs_ne( "COLOR" );

					o = NewObject( getlohrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_HR_NoShade, noshade,
						MA_Layout_HR_Penspec1, penspec ? colspec2rgb24( penspec ) : ctx->penspec_borderdark,
						MA_Layout_HR_Penspec2, penspec ? colspec2rgb24( penspec ) : ctx->penspec_borderlight,
						MA_Layout_HR_Size, size,
						MA_Layout_Width, widthspec,
						MUIA_InnerTop, ctx->currentstyle->font->tf_YSize / 2,
						MUIA_InnerBottom, ctx->currentstyle->font->tf_YSize / 2,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
				}
				break;

			case ht_div:
				{
					APTR o;
					char *align = getargs( "ALIGN" );
					int linealign = -1;

					if( align )
					{
						strupr( align );

						if( !strcmp( align, "LEFT" ) )
							linealign = linealign_left;
						else if( !strcmp( align, "RIGHT" ) )
							linealign = linealign_right;
						else if( !strcmp( align, "CENTER" ) || !strcmp( align, "MIDDLE" ) )
							linealign = linealign_center;
					}

					if( ctx->active_div )
						pop_linealign( ctx, latype_div );

					push_linealign( ctx, linealign, latype_div );
					ctx->active_div = TRUE;

					// And another one...
					o = JSNewObject( getlodivclass(),
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, align_newrow,
						MA_Layout_LineAlign, linealign,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_br:
				{
					APTR o;
					char *clear;
					int clear_left = 0, clear_right = 0;

					clear = getargs( "CLEAR" );
					if( clear )
					{
						strupr( clear );
						if( !strcmp( clear, "LEFT" ) )
							clear_left = 1;
						else if( !strcmp( clear, "RIGHT" ) )
							clear_right = 1;
						else if( !strcmp( clear, "ALL" ) )
							clear_left = clear_right = 1;
					}

					// And another one...
					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Height, ( clear_left || clear_right ) ? 0 : ctx->currentstyle->font->tf_YSize,
						MA_Layout_Div_Align, align_newrowafter,
						MA_Layout_Div_ClearMargin_Left, clear_left,
						MA_Layout_Div_ClearMargin_Right, clear_right,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_p:
				{
					APTR o;
					char *align = getargs( "ALIGN" );
					int linealign = ctx->current_linealign;

					if( align )
					{
						strupr( align );

						if( !strcmp( align, "LEFT" ) )
							linealign = linealign_left;
						else if( !strcmp( align, "RIGHT" ) )
							linealign = linealign_right;
						else if( !strcmp( align, "CENTER" ) || !strcmp( align, "MIDDLE" ) )
							linealign = linealign_center;

					}

					if( ctx->active_p )
						pop_linealign( ctx, latype_p );

					push_linealign( ctx, linealign, latype_p );
					ctx->active_p = TRUE;

					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, ( getv( ctx->current_container, MA_Layout_Group_IsEmpty ) ) ? align_newrow : align_newrowrow,
						MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
						MA_Layout_LineAlign, linealign,
						TAG_DONE
					);
					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_p | HTF_NEGATE:
			case ht_div | HTF_NEGATE:
				{
					APTR o;

					if( ch == ht_p | HTF_NEGATE )
						ctx->active_p = FALSE;
					else
						ctx->active_div = FALSE;

					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Div_Align, align_newrow,
						MA_Layout_Context, ctx,
						MA_Layout_LineAlign, pop_linealign( ctx, ( ch == ( ht_p | HTF_NEGATE ) ) ? latype_p : latype_div ),
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_center:
				// Legacy kludge
				{
					APTR o;

					push_linealign( ctx, linealign_center, latype_center );

					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, align_newrow,
						MA_Layout_LineAlign, linealign_center,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_center | HTF_NEGATE:
				// Legacy kludge
				{
					APTR o;
					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, align_newrow,
						MA_Layout_LineAlign, pop_linealign( ctx, latype_center ),
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			//
			// Images
			//

			case ht_map:
				{
					if( this_name )
					{
						ctx->current_imagemap = JSNewObject( getlomapclass(),
							MA_Layout_Map_Name, this_name,
							TAG_DONE
						);
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, ctx->current_imagemap );
					}
				}
				break;

			case ht_map | HTF_NEGATE:
				ctx->current_imagemap = NULL;
				break;

			case ht_area:
				if( ctx->current_imagemap )
				{
					char *shape = getargs( "SHAPE" );
					char *href = getargs( "HREF" );
					char *target = getargs( "TARGET" );
					char *coords = getargs( "COORDS" );
					char *accesskey = getargs( "ACCESSKEY" );
					int type = IMAPAREA_RECT;
					time_t visited = 0;
					APTR o;

					if( !href )
						break; // Nonsense...

					if( !target )
						target = ctx->basetarget;

					uri_mergeurl( ctx->baseref, href, urlbuffer );
					visited = checkurlhistory( urlbuffer );

					if( shape )
					{
						strupr( shape );
						if( !strcmp( shape, "DEFAULT" ) )
						{
							set( ctx->current_imagemap, MA_Layout_Map_DefaultURL, urlbuffer );
							if( target )
								set( ctx->current_imagemap, MA_Layout_Map_DefaultTarget, target );
							break;
						}
						else if( !strcmp( shape, "CIRCLE" ) )
						{
							type = IMAPAREA_CIRCLE;
						}
						else if( !strcmp( shape, "POLY" ) || !strcmp( shape, "POLYGON" ) )
						{
							type = IMAPAREA_POLY;
						}
					}

					if( !coords )
						break;

					o = JSNewObject( getloareaclass(),
						MA_Layout_Context, ctx,
						MA_Layout_Anchor_Name, this_name,
						MA_Layout_Anchor_Target, target,
						MA_Layout_Anchor_Title, getargs( "ALT" ),
						MA_Layout_Anchor_URL, urlbuffer,
						MA_Layout_Anchor_AccessKey, accesskey,
						TAG_DONE
					);

					DoMethod( o, MM_Layout_Area_SetCoords, type, coords );

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
					DoMethod( ctx->current_imagemap, MM_JS_Array_AddMember, o );

					js_doeventhandlers( ctx, o );
				}
				break;

			case ht_embed:
				{
					APTR o;
					char *type = getargs( "TYPE" );
					char *src = getargs( "SRC" );
					char *width = getargs( "WIDTH" );
					char *height = getargs( "HEIGHT" );
					int hspace = getnumargp( "HSPACE", 0 );
					int vspace = getnumargp( "VSPACE", 0 );
					int border = getnumargp( "BORDER", 0 );
					char *align = getargs( "ALIGN" );
					int alignval = align_inline;
					int valignval = get_valign( valign_baseline );
					//char *bgcolor = getargs_ne( "BGCOLOR" ); /* TOFIX: do something with that one day */

					if( !src )
						break; // No source? Hrmpf

					if( align )
					{
						if( !stricmp( align, "LEFT" ) )
							alignval = align_left, valignval = valign_special;
						else if( !stricmp( align, "RIGHT" ) )
							alignval = align_right, valignval = valign_special;
					}

					uri_mergeurl( ctx->baseref, src, urlbuffer );

					o = NewObject( getloembedclass(), NULL,
						MA_JS_Name, this_name,
						MA_Layout_MarginLeft, hspace + border,
						MA_Layout_MarginRight, hspace + border,
						MA_Layout_MarginTop, vspace + border,
						MA_Layout_MarginBottom, vspace + border,
						MA_Layout_Image_Border, border,
						MUIA_Font, ctx->currentstyle->font,
						MA_Layout_Context, ctx,
						MA_Layout_VAlign, valignval,
						MA_Layout_Align, alignval,
						MA_Layout_Embed_Src, urlbuffer,
						MA_Layout_Embed_Type, type,
						width ? MA_Layout_Image_Width : TAG_IGNORE, width,
						height ? MA_Layout_Image_Height : TAG_IGNORE, height,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
				}
				break;

			case ht_img:
				doformimage: // Ugly hack
				{
					APTR o;
					char *alt = getargs( "ALT" );
					char *src = getargs( "SRC" );
					char *lowsrc = getargs( "LOWSRC" );
					char *width = getargs( "WIDTH" );
					char *height = getargs( "HEIGHT" );
					int hspace = getnumargp( "HSPACE", 0 );
					int vspace = getnumargp( "VSPACE", 0 );
					int border = getnumargp( "BORDER", ctx->current_anchor ? 2 : 0 );
					char *align = getargs( "ALIGN" );
					int alignval = align_inline;
					int valignval = get_valign( valign_baseline );
					char *ismap = getargs( "ISMAP" );
					char *usemap = getargs( "USEMAP" );
					char urlbuffer2[ MAXURLSIZE ];
					APTR class = getloimageclass();
#if USE_LO_PIP
					char *chname = NULL;
#endif

					if( src )
						uri_mergeurl( ctx->baseref, src, urlbuffer );
					else
						urlbuffer[ 0 ]=0;

					if( lowsrc )
						uri_mergeurl( ctx->baseref, lowsrc, urlbuffer2 );

					if( align )
					{
						if( !stricmp( align, "LEFT" ) )
							alignval = align_left, valignval = valign_special;
						else if( !stricmp( align, "RIGHT" ) )
							alignval = align_right, valignval = valign_special;
					}

#if USE_LO_PIP
					if( !strnicmp( urlbuffer, "tv:", 3 ) )
					{
						chname = getargs( "CHANNELNAME" );
						class = getlopipclass();
						DoMethod( ctx->dom_win, MM_HTMLWin_IncPIPNum );
					}
#endif

					o = JSNewObject( class,
						isformimage ? MUIA_CycleChain : TAG_IGNORE, 1,
						MA_Layout_FormElement_Form, isformimage ? ctx->currentform : NULL,
						MA_Layout_FormElement_Name, this_name,
						MA_JS_Name, this_name,
						MA_Layout_MarginLeft, hspace + border,
						MA_Layout_MarginRight, hspace + border,
						MA_Layout_MarginTop, vspace + border,
						MA_Layout_MarginBottom, vspace + border,
						MA_Layout_Image_Border, border,
						MUIA_Font, ctx->currentstyle->font,
						MA_Layout_Context, ctx,
						MA_Layout_VAlign, valignval,
						MA_Layout_Align, alignval,
						MA_Layout_Image_Alttxt, alt,
						MA_Layout_Image_Anchor, ctx->current_anchor,
						MA_Layout_Image_Anchor_Visited, ctx->current_anchor_visited,
						MA_Layout_Image_IsMap, ismap ? TRUE : FALSE,
						MA_Layout_Image_UseMap, usemap,
						MA_Layout_Image_Src, urlbuffer,
						MA_Layout_Image_LowSrc, lowsrc ? urlbuffer2 : NULL,
						width ? MA_Layout_Image_Width : TAG_IGNORE, width,
						height ? MA_Layout_Image_Height : TAG_IGNORE, height,
#if USE_LO_PIP
						MUIA_Pip_ChannelName, chname,
						MUIA_Pip_AutoTune, TRUE,
#endif
						TAG_DONE
					);

					isformimage = FALSE;

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank=FALSE;
				}
				break;

			case ht_table:
				{
					APTR o;
					char *bgcolor = getargs_ne( "BGCOLOR" );
					char *bgimage = getargs( "BACKGROUND" );
					struct layout_table *table;
					struct layout_style *st;
					int cellpadding = getnumargp( "CELLPADDING", 1 );
					int cellspacing = getnumargp( "CELLSPACING", 2 );
					char *borderspec = getargs( "BORDER" );
					int border = 0;
					char *width = getargs( "WIDTH" );
					char *height = getargs( "HEIGHT" );
					char *bcdark = getargs_ne( "BORDERCOLORDARK" );
					char *bclight = getargs_ne( "BORDERCOLORLIGHT" );
					char *bclight2 = getargs_ne( "BORDERCOLOR" );
					char *align = getargs( "ALIGN" );
					int alignval = align_newrowrow;
					int linealign = -1;
					int valignval = valign_baseline;
					struct layout_li *li;

					if( !bclight && bclight2 )
						bclight = bclight2;

					if( borderspec )
					{
						if( !*borderspec )
							border = 1;
						else
							border = atoi( borderspec );

						border = max( 0, border );
					}

#ifdef VDEBUG
					if( db_forceborder && border < 2 )
						border = 2;
#endif

					if( align )
					{
						strupr( align );
						if( !strcmp( align, "LEFT" ) )
							alignval = align_left, valignval = valign_special;
						else if( !strcmp( align, "RIGHT" ) )
							alignval = align_right, valignval = valign_special;
						else if( !strcmp( align, "CENTER" ) || !strcmp( align, "MIDDLE" ) ) // BAH!
							linealign = linealign_center;
					}

					table = l_calloc( ctx, sizeof( *table ) );

					NEWLIST( &table->previous_stylestack );
					while( st = REMHEAD( &ctx->fontstyles ) )
						ADDTAIL( &table->previous_stylestack, st );
					table->previous_container = ctx->current_container;
					table->previous_anchor = ctx->current_anchor;
					table->previous_lastwasblank = ctx->lastwasblank;
					table->previous_mode_xmp = ctx->mode_xmp;
					table->previous_mode_pre = ctx->mode_pre;
					table->previous_mode_nobr = ctx->mode_nobr;
					table->border = border;
					table->cellpadding = cellpadding;
					table->cellspacing = cellspacing;

					memcpy( table->previous_linealign, ctx->linealign, sizeof( ctx->linealign ) );
					memcpy( table->previous_linealignptr, ctx->linealign_ptr, sizeof( ctx->linealign_ptr ) );
					table->previous_current_linealign = ctx->current_linealign;
					table->previous_active_p = ctx->active_p;
					table->previous_active_div = ctx->active_div;

					table->penspec_border_light = bclight ? colspec2rgb24( bclight ) : ctx->penspec_borderlight;
					table->penspec_border_dark = bcdark ? colspec2rgb24( bcdark ) : ctx->penspec_borderdark;

					NEWLIST( &table->previous_listack );
					while( li = REMHEAD( &ctx->listack ) )
						ADDTAIL( &table->previous_listack, li );

					// Create a copy of the "root" style
					st = l_malloc( ctx, sizeof( *st ) );
					memcpy( st, FIRSTNODE( &table->previous_stylestack ), sizeof( *st ) );
					ctx->currentstyle = st;
					ADDTAIL( &ctx->fontstyles, st );

					// Create a table object, and push it on the table stack

					o = NewObject( getlotableclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Table_Cellpadding, cellpadding,
						MA_Layout_Table_Cellspacing, cellspacing,
						MA_Layout_Table_Border, border,
						MA_Layout_Width, width,
						MA_Layout_Height, height,
						MA_Layout_BorderDark, table->penspec_border_dark,
						MA_Layout_BorderLight, table->penspec_border_light,
						MA_Layout_Cell_Border, table->border,
						MA_Layout_Align, alignval,
						MA_Layout_VAlign, valignval,
						MA_Layout_TempLineAlign, linealign,
						MA_Layout_MarginTop, border,
						MA_Layout_MarginBottom, border,
						MA_Layout_MarginLeft, border,
						MA_Layout_MarginRight, border,
						TAG_DONE
					);

					if( bgcolor )
					{
						set( o, MA_Layout_BGColor, colspec2rgb24( bgcolor ) );
					}

					if( bgimage )
					{
#ifdef MBX
						if( !stricmp( bgimage, "tv:" ) )
						{
							set( o, MA_Layout_BGColor, LO_GENLOCK );
						}
						else
#endif
						{
							uri_mergeurl( ctx->baseref, bgimage, urlbuffer );
							set( o, MA_Layout_BGImageURL, urlbuffer );
						}
					}

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					table->tableobject = o;

					ADDTAIL( &ctx->tablestack, table );
					ctx->currenttable = table;
				}
				break;

			case ht_table | HTF_NEGATE:
				if( ctx->currenttable )
					unrolltable( ctx );
				break;

			case ht_td:
			case ht_th:
				{
					APTR o;
					char *bgcolor = getargs_ne( "BGCOLOR" );
					char *bgimage = getargs( "BACKGROUND" );
					char *width = getargs( "WIDTH" );
					char *height = getargs( "HEIGHT" );
					int rowspan = getnumargmm( "ROWSPAN", 1, 1, INT_MAX );
					int colspan = getnumargmm( "COLSPAN", 1, 1, INT_MAX );
					char *valign = getargs( "VALIGN" );
					char *align = getargs( "ALIGN" );
					int innerframe;
					int valign_val = valign_middle;
					int linealign = linealign_left;
					struct layout_style *st;
					char *nowrap = getargs( "NOWRAP" );

					if( !ctx->currenttable )
						break; // Not in table, forget about it

					if( !ctx->currenttable->inrow )
					{
						free( ctx->currenttable->tr_bgcolor );
						free( ctx->currenttable->tr_bgimage );
						free( ctx->currenttable->tr_valign );
						free( ctx->currenttable->tr_align );
						ctx->currenttable->tr_bgcolor = 0;
						ctx->currenttable->tr_bgimage = 0;
						ctx->currenttable->tr_valign = 0;
						ctx->currenttable->tr_align = 0;
						ctx->lastwasblank = TRUE;
						ctx->currenttable->rowcount++;
						ctx->currenttable->inrow = 1;
					}

					if( !bgcolor )
						bgcolor = ctx->currenttable->tr_bgcolor;
					if( !bgimage )
						bgimage = ctx->currenttable->tr_bgimage;
					if( !valign )
						valign = ctx->currenttable->tr_valign;
					if( !align )
						align = ctx->currenttable->tr_align;

					ctx->mode_nobr = ( nowrap && !width ) ? TRUE : FALSE;

					if( valign )
					{
						strupr( valign );
						if( !strcmp( valign, "TOP" ) )
							valign_val = valign_top;
						else if( !strcmp( valign, "BOTTOM" ) )
							valign_val = valign_bottom;
					}

					// <TH> -> bold, centered
					if( ch == ht_th )
						linealign = linealign_center;

					if( align )
					{
						strupr( align );
						if( !strcmp( align, "RIGHT" ) )
							linealign = linealign_right;
						else if( !strcmp( align, "LEFT" ) )
							linealign = linealign_left;
						else if( !strcmp( align, "CENTER" ) || !strcmp( align, "MIDDLE" ) )
							linealign = linealign_center;
					}

					// Create a copy of the "root" style
					while( st = REMHEAD( &ctx->fontstyles ) )
						l_free( ctx, st );

					st = l_malloc( ctx, sizeof( *st ) );
					memcpy( st, FIRSTNODE( &ctx->currenttable->previous_stylestack ), sizeof( *st ) );
					ctx->currentstyle = st;
					ADDTAIL( &ctx->fontstyles, st );

					if( ch == ht_th )
						st->fontstyle |= FSF_BOLD;

					NEWLIST( &ctx->listack );
					ctx->current_li = NULL;

					innerframe = ctx->currenttable->cellpadding + ( ctx->currenttable->border ? 1 : 0 );

					ctx->current_container = o = NewObject( getlogroupclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Width, width,
						MA_Layout_Height, height,
						MA_Layout_VAlign, valign_val,
						MA_Layout_MarginTop, innerframe,
						MA_Layout_MarginBottom, innerframe,
						MA_Layout_MarginLeft, innerframe,
						MA_Layout_MarginRight, innerframe,
						MA_Layout_Cell_Colspan, colspan,
						MA_Layout_Cell_Rowspan, rowspan,
						MA_Layout_Cell_Row, ctx->currenttable->rowcount,
						MA_Layout_Cell_Border, ctx->currenttable->border ? 1 : 0,
						MA_Layout_Cell_DefaultLineAlign, linealign,
						// Swapped for inner cell borders!
						MA_Layout_BorderLight, ctx->currenttable->penspec_border_dark,
						MA_Layout_BorderDark, ctx->currenttable->penspec_border_light,
						TAG_DONE
					);

					if( !o )
					{
						reporterror( "can't create table cell object" );
						ctx->done = TRUE;
						return( offset );
					}

					if( bgcolor )
						set( o, MA_Layout_BGColor, colspec2rgb24( bgcolor ) );

					if( bgimage )
					{
#ifdef MBX
						if( !stricmp( bgimage, "tv:" ) )
							set( o, MA_Layout_BGColor, LO_GENLOCK );
						else
#endif
						uri_mergeurl( ctx->baseref, bgimage, urlbuffer );
						set( o, MA_Layout_BGImageURL, urlbuffer );
					}

					DoMethod( ctx->currenttable->tableobject, MM_Layout_Group_AddObject, o );

					memset( &ctx->linealign_ptr, 0, sizeof( ctx->linealign_ptr ) );
					ctx->current_linealign = linealign;
					ctx->lastwasblank = TRUE;
					ctx->mode_xmp = FALSE;
					ctx->mode_pre = FALSE;
				}
				break;

			case ht_tr:
				{
					if( ctx->currenttable )
					{
						char *bgcolor = getargs_ne( "BGCOLOR" );
						char *bgimage = getargs( "BACKGROUND" );
						char *valign = getargs( "VALIGN" );
						char *align = getargs( "ALIGN" );
						/* Adding more of these properties?  Please also look at the
						   'implied' TR code in hd_td, above */

						free( ctx->currenttable->tr_bgcolor );
						free( ctx->currenttable->tr_bgimage );
						free( ctx->currenttable->tr_valign );
						free( ctx->currenttable->tr_align );

						if( bgcolor )
							ctx->currenttable->tr_bgcolor = strdup( bgcolor ); /* TOFIX */
						else
							ctx->currenttable->tr_bgcolor = 0;
						if( bgimage )
							ctx->currenttable->tr_bgimage = strdup( bgimage ); /* TOFIX */
						else
							ctx->currenttable->tr_bgimage = 0;
						if( valign )
							ctx->currenttable->tr_valign = strdup( valign ); /* TOFIX */
						else
							ctx->currenttable->tr_valign = 0;
						if( align )
							ctx->currenttable->tr_align = strdup( align ); /* TOFIX */
						else
							ctx->currenttable->tr_align = 0;

						ctx->current_container = ctx->currenttable->previous_container;
						ctx->lastwasblank = TRUE;
						ctx->currenttable->rowcount++;
						ctx->currenttable->inrow = 1;
					}
				}
				break;

			case ht_td | HTF_NEGATE:
			case ht_tr | HTF_NEGATE:
			case ht_th | HTF_NEGATE:
				{
					if( ctx->currenttable )
					{
						ctx->current_container = ctx->currenttable->previous_container;
						ctx->lastwasblank = TRUE;
						ctx->mode_nobr = FALSE;
						if( ch == ( ht_tr | HTF_NEGATE ) )
							ctx->currenttable->inrow = 0;
					}
				}
				break;

			//
			// Anchors
			//
			// Embedded, invisible objects
			//

			case ht_a:
				{
					char *href = getargs( "HREF" );
					char *title = getargs( "TITLE" );
					char *target = getargs( "TARGET" );
					char *accesskey = getargs( "ACCESSKEY" );
					APTR o;
					struct layout_style *style = layout_pushstyle( ctx );
					time_t visited = 0;

					if( !target )
						target = ctx->basetarget;

					if( href )
					{
						uri_mergeurl( ctx->baseref, href, urlbuffer );
						visited = checkurlhistory( urlbuffer );
						ctx->current_anchor_visited = visited;
						style->color =  visited ? ctx->penspec_vlink : ctx->penspec_link;
						if( visited )
							style->fontstyle |= FSF_VISITEDLINK;
						//D( db_html, bug( "adding link to %s, target %s, visited %lx\n", urlbuffer, target ? target : "(none)", visited ));
					}

					o = JSNewObject( getloanchorclass(),
						MA_Layout_Anchor_URL, href ? urlbuffer : NULL,
						MA_Layout_Anchor_Name, this_name,
						MA_JS_Name, this_name,
						MA_Layout_Anchor_Title, title,
						MA_Layout_Anchor_Target, target,
						MA_Layout_Anchor_Visited, visited,
						MA_Layout_Anchor_AccessKey, accesskey,
						MA_Layout_Context, ctx,
						TAG_DONE
					);

					if( href )
						ctx->current_anchor = o;

					js_doeventhandlers( ctx, o );

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
				}
				break;

			case ht_a | HTF_NEGATE:
				layout_popstyle( ctx );
				ctx->current_anchor = NULL;
				break;

			//
			// Text style modification tags
			//
			// All those effectivly push a new style node,
			// which is automatically cloned from the previous node,
			// and modify it accordingly

			case ht_b:
			case ht_strong:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					style->fontstyle |= FSF_BOLD;
				}
				break;

			case ht_i:
			case ht_em:
			case ht_cite:
			case ht_dfn:
			case ht_var:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					style->fontstyle |= FSF_ITALIC;
				}
				break;

			case ht_u:
			case ht_ins:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					style->fontstyle |= FSF_UNDERLINED;
				}
				break;

			case ht_s:
			case ht_del:
			case ht_strike:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					style->fontstyle |= FSF_STRIKE;
				}
				break;

			case ht_tt:
			case ht_code:
			case ht_samp:
			case ht_kbd:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					stccpy( style->face, "(Fixed)", sizeof( style->face ) );
					style->font = getfont( style->face, style->fontstepsize, &style->fontarray );
				}
				break;

			case ht_font:
			case ht_small:
			case ht_big:
			case ht_basefont:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					char *color = getargs_ne( "COLOR" );
					char *size = getargs( "SIZE" );
					char *face = getargs( "FACE" );

					if( ch == ht_small )
						size = "-1";
					else if( ch == ht_big )
						size = "+1";

					if( color )
					{
						style->color = colspec2rgb24( color );
					}

					if( size )
					{
						int ps;

						if( *size == '+' )
						{
							size++;
							ps = atoi( size ) + 2;
						}
						else if( *size == '-' )
						{
							ps = 2 + atoi( size );
						}
						else
						{
							ps = atoi( size ) - 1;
						}

						if( ps < 0 )
							ps = 0;
						else if( ps > 6 )
							ps = 6;

						style->fontstepsize = ps;
					}

					if( gp_fontface && face )
					{
						stccpy( style->face, face, sizeof( style->face ) );
					}

					if( ( gp_fontface && face ) || size )
					{
						style->font = getfont( style->face, style->fontstepsize, &style->fontarray );
					}

					// If basefont, replace default style
					if( ch == ht_basefont )
					{
						struct layout_style *first;

						REMOVE( style );
						while( ( first = REMHEAD( &ctx->fontstyles ) ) )
							l_free( ctx, first );

						ADDHEAD( &ctx->fontstyles, style );
					}
				}
				break;

			case ht_h1:
			case ht_h2:
			case ht_h3:
			case ht_h4:
			case ht_h5:
			case ht_h6:
				{
					char *align = getargs( "ALIGN" );
					int linealign = -1;
					struct layout_style *style = layout_pushstyle( ctx );
					APTR o;

					style->fontstepsize = 5 - ( ch - ht_h1 );
					style->fontstyle = FSF_BOLD;
					style->font = getfont( style->face, style->fontstepsize, &style->fontarray );

					if( align )
					{
						if( !stricmp( align, "LEFT" ) )
							linealign = linealign_left;
						else if( !stricmp( align, "RIGHT" ) )
							linealign = linealign_right;
						else if( !stricmp( align, "CENTER" ) || !stricmp( align, "MIDDLE" ) )
							linealign = linealign_center;
					}

					push_linealign( ctx, linealign, latype_h );

					// And another one...
					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, align_newrowrow,
						MA_Layout_LineAlign, linealign,
						MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_h1 | HTF_NEGATE:
			case ht_h2 | HTF_NEGATE:
			case ht_h3 | HTF_NEGATE:
			case ht_h4 | HTF_NEGATE:
			case ht_h5 | HTF_NEGATE:
			case ht_h6 | HTF_NEGATE:
				{
					APTR o;

					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_LineAlign, pop_linealign( ctx, latype_h ),
						MA_Layout_Div_Align, align_newrowrow,
						MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize / 2,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					layout_popstyle( ctx );

					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_b | HTF_NEGATE:
			case ht_strong | HTF_NEGATE:

			case ht_i | HTF_NEGATE:
			case ht_em | HTF_NEGATE:
			case ht_cite | HTF_NEGATE:
			case ht_dfn | HTF_NEGATE:
			case ht_var | HTF_NEGATE:

			case ht_u | HTF_NEGATE:
			case ht_ins | HTF_NEGATE:

			case ht_s | HTF_NEGATE:
			case ht_del | HTF_NEGATE:
			case ht_strike | HTF_NEGATE:

			case ht_tt | HTF_NEGATE:
			case ht_code | HTF_NEGATE:
			case ht_samp | HTF_NEGATE:
			case ht_kbd | HTF_NEGATE:

			case ht_font | HTF_NEGATE:
			case ht_small | HTF_NEGATE:
			case ht_big | HTF_NEGATE:

				layout_popstyle( ctx );
				break;


			case ht_nobr:
				ctx->mode_nobr = TRUE;
				break;

			case ht_nobr | HTF_NEGATE:
				ctx->mode_nobr = FALSE;
				break;

			//
			// Form stuff
			//

			case ht_form:
				{
					APTR o;
					char *url = getargs( "ACTION" );
					char *target = getargs( "TARGET" );
					char *method = getargs( "METHOD" );
					char *enctype = getargs( "ENCTYPE" );
					char *p;

					if( !target )
						target = ctx->basetarget;

					ctx->form_id++;
					ctx->form_eid = 0;

					// meow!
					if (!ctx->lastwasblank) // top of tables/documents etc.
					{
						o = NewObject( getlobrclass(), NULL,
							MA_Layout_Context, ctx,
							MA_Layout_Div_Align, align_newrowrow,
							MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
							TAG_DONE
						);
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
					}

					if( url )
					{
						//uri_mergeurl( ctx->baseref, url, urlbuffer );
						strcpy( urlbuffer, url );
					}
					else
					{
						strcpy( urlbuffer, ctx->baseref );
					}
					if( !method || stricmp( method, "POST" ) )
					{
						p = strrchr( urlbuffer, '?' );
						if( p )
							*p = 0;
					}

					o = JSNewObject( getloformclass(),
						MA_Layout_Context, ctx,
						MA_JS_Name, this_name,
						MA_Layout_Form_URL, urlbuffer,
						MA_Layout_Form_Target, target,
						MA_Layout_Form_Method, !method || stricmp( method, "POST" ),
						MA_Layout_Form_Enctype, enctype && !stricmp( enctype, "multipart/form-data" ),
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->currentform = o;
					ctx->lastwasblank = TRUE;
				}
				break;

			case ht_form | HTF_NEGATE:
				// Silly leading after form...
				if( ctx->currentform )
				{
					APTR o;

					if( !ctx->currenttable || ctx->current_container != ctx->currenttable->previous_container )
					{
						o = NewObject( getlobrclass(), NULL,
							MA_Layout_Context, ctx,
							MA_Layout_Div_Align, align_newrowrow,
							MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
							TAG_DONE
						);

						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
					}

					ctx->lastwasblank = TRUE;
					ctx->currentform = NULL;
				}
				break;

			case ht_button:
				{
					char *type = getargs( "TYPE" );
					char *value = getargs( "VALUE" );
					int disabled = getargs( "DISABLED" ) ? TRUE : FALSE;
					APTR o;
					int btype = formbutton_submit;
					if( type )
					{
						if( !stricmp( type, "RESET" ) )
							btype = formbutton_reset;
						else if( !stricmp( type, "BUTTON" ) )
							btype = formbutton_button;
					}
					if (ctx->current_button ) // Someone is either trying to nest buttons, or more likely forgot a </BUTTON>
					{
						unrollbutton( ctx );
					}
					o = NewObject( getloformbuttonclass(), NULL,
						MUIA_Disabled, disabled,
						MUIA_Font, ctx->currentstyle->font,
						MA_Layout_FormElement_Form, ctx->currentform,
						MA_Layout_FormElement_Name, this_name,
						MA_Layout_FormElement_Value, value,
						MA_Layout_FormButton_Type, btype,
						MA_Layout_Context, ctx,
						TAG_DONE
					);
					if( o )
					{
						struct layout_button *button;
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
						button = l_calloc( ctx, sizeof( *button ) );
						button->previous_container = ctx->current_container;
						ctx->current_container = o;
						ctx->current_button = button;
					}
				}
				break;

			case ht_button | HTF_NEGATE:
				if( ctx->current_button )
					unrollbutton( ctx );
				break;

			case ht_input:
				{
					char *type = getargs( "TYPE" );
					char *value = getargs( "VALUE" );
					int disabled = getargs( "DISABLED" ) ? TRUE : FALSE;
					APTR o = NULL;

					if( type )
						strupr( type );
					else
						type = "TEXT";

					if( !strcmp( type, "BUTTON" ) || !strcmp( type, "SUBMIT" ) || !strcmp( type, "RESET" ) )
					{
						int btype = formbutton_submit;

						if( !strcmp( type, "BUTTON" ) )
							btype = formbutton_button;
						else if( !strcmp( type, "RESET" ) )
							btype = formbutton_reset;

						o = JSNewObject( getlobuttonclass(),
							MUIA_Disabled, disabled,
							MUIA_Font, ctx->currentstyle->font,
							MA_Layout_FormElement_Form, ctx->currentform,
							MA_Layout_FormElement_Name, this_name,
							MA_Layout_FormElement_Value, value,
							MA_Layout_FormButton_Type, btype,
							MA_Layout_Context, ctx,
							MUIA_CycleChain, 1,
							TAG_DONE
						);
					}
					else if( !strcmp( type, "RADIO" ) )
					{
						int checked = (int)getargs( "CHECKED" );
						int *storedvalue = (int*)formstore_get( ctx->baseref, ctx->form_eid, ctx->form_id, NULL );

						o = JSNewObject( getloradioclass(),
							MA_Layout_Context, ctx,
							MUIA_Font, ctx->currentstyle->font,
							MUIA_Disabled, disabled,
							MA_Layout_FormElement_Form, ctx->currentform,
							MA_Layout_FormElement_Name, this_name,
							MA_Layout_FormElement_Value, value,
							MA_Layout_FormElement_ID, ctx->form_id,
							MA_Layout_FormElement_EID, ctx->form_eid++,
							MUIA_Selected, storedvalue ? *storedvalue : checked,
							MA_Layout_FormElement_DefaultValue, checked,
							MUIA_CycleChain, 1,
							TAG_DONE
						);
					}
					else if( !strcmp( type, "CHECKBOX" ) )
					{
						int checked = (int)getargs( "CHECKED" );
						int *storedvalue = (int*)formstore_get( ctx->baseref, ctx->form_eid, ctx->form_id, NULL );

						o = JSNewObject( getlocheckboxclass(),
							MA_Layout_Context, ctx,
							MUIA_Font, ctx->currentstyle->font,
							MUIA_Disabled, disabled,
							MA_Layout_FormElement_Form, ctx->currentform,
							MA_Layout_FormElement_Name, this_name,
							MA_Layout_FormElement_Value, value,
							MUIA_Selected, storedvalue ? *storedvalue : checked,
							MA_Layout_FormElement_DefaultValue, checked,
							MA_Layout_FormElement_ID, ctx->form_id,
							MA_Layout_FormElement_EID, ctx->form_eid++,
							MUIA_CycleChain, 1,
							TAG_DONE
						);
					}
					else if( !strcmp( type, "HIDDEN" ) )
					{
						o = JSNewObject( getloformhiddenclass(),
							MA_Layout_FormElement_Form, ctx->currentform,
							MA_Layout_FormElement_Name, this_name,
							MA_Layout_FormElement_Value, value,
							MA_Layout_Context, ctx,
							TAG_DONE
						);
					}
					else if( !strcmp( type, "IMAGE" ) )
					{
						isformimage = TRUE;
						goto doformimage;
					}
					else if( !strcmp( type, "FILE" ) )
					{
						int size = getnumargp( "SIZE", 20 );
						char *storedtext = formstore_get( ctx->baseref, ctx->form_eid, ctx->form_id, NULL );

						value = ""; // TOFIX -- property security

						o = NewObject( getloformfileclass(), NULL,
							MUIA_Font, ctx->currentstyle->font,
							MUIA_Disabled, disabled,
							MA_Layout_FormElement_Form, ctx->currentform,
							MA_Layout_FormElement_Name, this_name,
							MA_Layout_FormElement_Value, storedtext ? storedtext : value,
							MA_Layout_FormElement_DefaultValue, value,
							MA_Layout_FormElement_ID, ctx->form_id,
							MA_Layout_FormElement_EID, ctx->form_eid++,
							MA_Layout_FormText_Size, size,
							MA_Layout_Context, ctx,
							MUIA_CycleChain, 1,
							TAG_DONE
						);
					}
					else // Assume text
					{
						int size = getnumargp( "SIZE", 20 );
						int maxlength = getnumargp( "MAXLENGTH", 256 );
						char *storedtext = formstore_get( ctx->baseref, ctx->form_eid, ctx->form_id, NULL );

						o = JSNewObject( getloformtextclass(),
							MUIA_Font, ctx->currentstyle->font,
							MUIA_Disabled, disabled,
							MA_Layout_FormElement_Form, ctx->currentform,
							MA_Layout_FormElement_Name, this_name,
							MA_Layout_FormElement_Value, storedtext ? storedtext : value,
							MA_Layout_FormElement_DefaultValue, value,
							MA_Layout_FormText_Size, size,
							MUIA_Textinput_MaxLen, maxlength + 1,
							MUIA_Textinput_NoInput, getargs( "READONLY" ),
							MUIA_Textinput_Secret, !strcmp( type, "PASSWORD" ),
							MA_Layout_FormElement_ID, ctx->form_id,
							MA_Layout_FormElement_EID, ctx->form_eid++,
							MA_Layout_Context, ctx,
							MUIA_CycleChain, 1,
							TAG_DONE
						);
					}

					if( o )
					{
						js_doeventhandlers( ctx, o );
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
						ctx->lastwasblank = TRUE;
					}
				}
				break;


			case ht_option:
				if( ctx->currentselect )
				{
					if( do_option( ctx, (char**)&data ) )
						aborthere = TRUE;
				}
				break;

			case ht_select:
				if( ctx->currentform )
				{
					APTR o;
					int size = getnumargp( "SIZE", 1 );
					int multiple = getargs( "MULTIPLE" ) ? 1 : 0;

					o = JSNewObject( getloformcycleclass(),
						MA_Layout_Context, ctx,
						MUIA_Font, ctx->currentstyle->font,
						MUIA_Disabled, getargs( "DISABLED" ),
						MA_Layout_FormElement_Form, ctx->currentform,
						MA_Layout_FormCycle_Size, size,
						MA_Layout_FormCycle_Multiple, multiple,
						MA_Layout_FormElement_Name, this_name,
						MA_Layout_FormElement_ID, ctx->form_id,
						MA_Layout_FormElement_EID, ctx->form_eid++,
						MUIA_CycleChain, 1,
						TAG_DONE
					);
					if( o )
					{
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
						js_doeventhandlers( ctx, o );
					}
					ctx->currentselect = o;
				}
				break;

			case ht_select | HTF_NEGATE:
				if( ctx->currentselect )
				{
					DoMethod( ctx->currentselect, MM_Layout_FormCycle_Finish );
					ctx->currentselect = NULL;
				}
				break;

			case ht_textarea:
				if( ctx->currentform )
				{
					if( do_textarea( ctx, (char**)&data ) )
						aborthere = TRUE;
				}
				break;

			case ht_pre:
				{
					struct layout_style *style = layout_pushstyle( ctx );
					APTR o;

					ctx->mode_pre = TRUE;
					stccpy( style->face, "(Fixed)", sizeof( style->face ) );
					style->font = getfont( style->face, style->fontstepsize, &style->fontarray );

					// And another one...
					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, ( getv( ctx->current_container, MA_Layout_Group_IsEmpty ) ) ? align_newrow : align_newrowrow,
						MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
				}
				break;

			case ht_pre | HTF_NEGATE:
				{
					APTR o;
					layout_popstyle( ctx );
					ctx->mode_pre = FALSE;

					// And another one...
					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, ( getv( ctx->current_container, MA_Layout_Group_IsEmpty ) ) ? align_newrow : align_newrowrow,
						MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
				}
				break;

			case ht_xmp:
				ctx->mode_xmp = TRUE;
				break;

			case ht_xmp | HTF_NEGATE:
				ctx->mode_xmp = FALSE;
				break;

			// List stuff
			case ht_ul:
			case ht_ol:
			case ht_dl:
				{
					struct layout_li *li;
					int currentdepth;

					for( currentdepth = 0, li = FIRSTNODE( &ctx->listack ); NEXTNODE( li ); li = NEXTNODE( li ) )
						currentdepth++;

					li = l_malloc( ctx, sizeof( *li ) );
					li->type = ( ch == ht_ul ) ? 0 : ( ch==ht_dl?2:1);
					li->count = 1;
					li->shape = currentdepth;
					li->depth = currentdepth;

					ADDTAIL( &ctx->listack, li );
					ctx->current_li = li;
				}
				break;

			case ht_ul | HTF_NEGATE:
			case ht_ol | HTF_NEGATE:
			case ht_dl | HTF_NEGATE:
				{
					struct layout_li *li;

					li = REMTAIL( &ctx->listack );
					if( li )
					{
						APTR o;

						o = NewObject( getlobrclass(), NULL,
							MA_Layout_Context, ctx,
							MA_Layout_Div_Align, align_newrow,
							TAG_DONE
						);

						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

						ctx->lastwasblank = TRUE;

						l_free( ctx, li );
					}
					if( ISLISTEMPTY( &ctx->listack ) )
						ctx->current_li = NULL;
					else
						ctx->current_li = LASTNODE( &ctx->listack );

				}
				break;

			case ht_dt:
			case ht_li:
				{
					APTR o;

					o = NewObject( getloliclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Listitem_Size, ctx->currentstyle->font->tf_YSize,
						MA_Layout_Listitem_Color, ctx->currentstyle->color,
						MA_Layout_Listitem_Shape, ctx->current_li ? ctx->current_li->shape : 0,
						MA_Layout_Listitem_Type, ctx->current_li ? ctx->current_li->type : 0,
						MA_Layout_Listitem_Index, ctx->current_li ? ctx->current_li->count : 0,
						MA_Layout_Listitem_Depth, ctx->current_li ? ctx->current_li->depth : 0,
						MA_Layout_Listitem_InList, ctx->current_li,
						MUIA_Font, ctx->currentstyle->font,
						TAG_DONE
					);

					if( ctx->current_li )
						ctx->current_li->count++;

					ctx->lastwasblank = TRUE;

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
				}
				break;

			case ht_dd:
				{
					APTR o;
					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, align_newrowafter,
						TAG_DONE
					);
					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
					ctx->lastwasblank = TRUE;
				}
				break;
			// ht_dd|HTF_NEGATE is ignored

			case ht_isindex:
				{
					APTR o;
					char *prompt = getargs( "PROMPT" ), *p;

					strcpy( urlbuffer, ctx->baseref );
					p = strrchr( urlbuffer, '?' );
					if( p )
						*p = 0;

					o = JSNewObject( getloformclass(),
						MA_Layout_Context, ctx,
						MA_Layout_Form_URL, urlbuffer,
						MA_Layout_Form_Target, NULL,
						MA_Layout_Form_Method, TRUE,
						MA_Layout_Form_Enctype, FALSE,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->currentform = o;
					ctx->lastwasblank = TRUE;

					if( prompt )
					{
						DoMethod( ctx->current_container,
							MM_Layout_Group_AddText,
							prompt, strlen(prompt),
							ctx->currentstyle->font,
							ctx->currentstyle->fontarray,
							ctx->currentstyle->color,
							ctx->currentstyle->fontstyle,
							ctx->current_anchor
						);
					}

					o = JSNewObject( getloformtextclass(),
						MUIA_Font, ctx->currentstyle->font,
						MUIA_Disabled, FALSE,
						MA_Layout_FormElement_Form, ctx->currentform,
						MA_Layout_FormElement_Name, "\001ISINDEX\001",
						MA_Layout_FormElement_Value, NULL,
						MA_Layout_FormText_Size, 20,
						MUIA_Textinput_MaxLen, 257,
						MA_Layout_Context, ctx,
						MUIA_CycleChain, 1,
						TAG_DONE
					);
					if( o )
					{
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
					}

					o = JSNewObject( getlobuttonclass(),
						MUIA_Disabled, FALSE,
						MUIA_Font, ctx->currentstyle->font,
						MA_Layout_FormElement_Form, ctx->currentform,
						MA_Layout_FormElement_Name, NULL,
						MA_Layout_FormElement_Value, "Search index", //TOFIX!! Should be localised (actually, string should auto-submit, and this gadget be removed)
						MA_Layout_FormButton_Type, formbutton_submit,
						MA_Layout_Context, ctx,
						MUIA_CycleChain, 1,
						TAG_DONE
					);
					if( o )
					{
						DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
					}

					o = NewObject( getlobrclass(), NULL,
						MA_Layout_Context, ctx,
						MA_Layout_Div_Align, align_newrowrow,
						MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
						TAG_DONE
					);

					DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );

					ctx->lastwasblank = TRUE;
					ctx->currentform = NULL;
				}
				break;

			case ht_frameset:
				if( getflag( VFLG_IGNORE_FRAMES ) )
					break;

				if( !is_complete )
				{
					aborthere = TRUE;
				}
				else
				{
					SetAttrs( ctx->body,
						MA_Layout_MarginLeft, 0,
						MA_Layout_MarginRight, 0,
						MA_Layout_MarginTop, 0,
						MA_Layout_MarginBottom, 0,
						MA_Layout_BGColor, LO_NOPEN,
						TAG_DONE
					);

					// Reset virtual width, we only want to deal with real widths with framesets
					set( ctx->dom_document, MA_HTMLView_IsFrameset, TRUE );
					do_frameset( ctx->body, ctx, (char**)&data, TRUE, 4 );
					aborthere = TRUE;
					ctx->done = TRUE;
				}

				break;
		}
	}

	if( ctx->textbufferoffset )
	{
		// Create a text object out of the current text buffer
		DoMethod( ctx->current_container,
			MM_Layout_Group_AddText,
			ctx->textbuffer, ctx->textbufferoffset,
			ctx->currentstyle->font,
			ctx->currentstyle->fontarray,
			ctx->currentstyle->color,
			ctx->currentstyle->fontstyle,
			ctx->current_anchor
		);
		ctx->textbufferoffset = 0;
	}

	if( is_complete && is_complete != 2 && !aborthere )
	{
		while( ctx->currenttable )
			unrolltable( ctx );
		// Is this necessary?
		if( ctx->current_button )
			unrollbutton( ctx );
	}

	D( db_html, bug( "layout_do done, status=%ld, %ld ticks\n", aborthere, clock() - ts ));

	if( aborthere )
		return( olddata - begindata );
	else
		return( (char*)data - (char*)begindata );
}

int layout_do_text(
	struct layout_ctx *ctx,
	STRPTR data,
	int datasize,
	int offset,
	int is_complete
)
{
	STRPTR endofdata = data + datasize;
	int ch;
	int aborthere = FALSE;
	char *begindata = data;
	char *olddata = data;
	APTR o;

//kprintf( "*** in layout_do_text\r\n" );

	data += offset;

	if( !ctx->body )
	{
		ULONG rgb = ctx->penspec_background;

		// Create the body container object
		ctx->body = NewObject( getlogroupclass(), NULL,
			MA_Layout_MarginLeft, 10,
			MA_Layout_MarginRight, 10,
			MA_Layout_MarginTop, 15,
			MA_Layout_MarginBottom, 15,
			MA_Layout_Context, ctx,
			MA_Layout_BGColor, rgb,
			TAG_DONE
		);
		layout_addobj( ctx, ctx->body );
		ctx->current_container = ctx->body;

		ctx->lastwasblank = TRUE;

		// Create default style for body
		layout_pushstyle( ctx );
		strcpy( ctx->currentstyle->face, "(Fixed)" );
		ctx->currentstyle->fontstepsize = 2;
		ctx->currentstyle->fontstyle = 0;
		ctx->currentstyle->color = ctx->penspec_text;
		layout_styleopenfont( ctx );
		layout_make_shadow_shine( 0xffffff, ctx );
	}

#if USE_LIBUNICODE
	// Check whether we need to allocate a charset converter
	parse_setcurrentlayoutctx( ctx );

redocharsetconverter:
	D( db_html, bug( "iso charset %ld, uconv_source %ld\n", ctx->iso_charset, ctx->uconv_source ) );
//kprintf( "*** iso charset %ld, uconv_source %ld\r\n", ctx->iso_charset, ctx->uconv_source );
	if( ctx->iso_charset != ctx->uconv_source )
	{
		FreeCharConverter( ctx->uconv );
		ctx->uconv = AllocCharConverter( ctx->iso_charset, UCCS_UTF8 );
		ctx->uconv_source = ctx->iso_charset;

		if( ctx->iso_charset != UCCS_LATIN1 )
		{
			// Create default style for body
			layout_pushstyle( ctx );
			strcpy( ctx->currentstyle->face, "Unicode" );
			ctx->currentstyle->fontstepsize = 2;
			ctx->currentstyle->fontstyle = 0;
			ctx->currentstyle->color = ctx->penspec_text;
			layout_styleopenfont( ctx );
			// Hack topmost style into submission (for following table nodes)
			if( ctx->body )
			{
				struct layout_style *topstyle = FIRSTNODE( &ctx->fontstyles );
				topstyle->font = ctx->currentstyle->font;
				strcpy( topstyle->face, "Unicode" );
			}
		}
	}

#endif
	while( data < endofdata && !aborthere )
	{
		olddata = data;

		ch = *data;
		data++;

		if( !ch )
		{
			if( is_complete && ctx->textbufferoffset )
				goto dotext;
			else if( !is_complete )
				aborthere = TRUE;
			break;
		}

		if( ch == 13 )
			continue;

		if( ch != 10 )
		{
			if( ch == '\t' )
			{
				int addspaces = 8 - ( ctx->textbufferoffset ) % 8;

				while( addspaces-- )
				{
					if( ctx->textbufferoffset == ctx->textbuffersize )
					{
						int ns = ctx->textbuffersize + TEXTBUFFER_INCREMENT;
						char *ntb = l_malloc( ctx, ns );
						memcpy( ntb, ctx->textbuffer, ctx->textbuffersize );
						l_free( ctx, ctx->textbuffer );
						ctx->textbuffer = ntb;
						ctx->textbuffersize = ns;
					}
					ctx->textbuffer[ ctx->textbufferoffset++ ] = ' ';
				}
				continue;
			}

			// Space conversion
			if( ch < ' ' )
				ch = ' ';

			// Add this character to the current text buffer
			if( ctx->textbufferoffset == ctx->textbuffersize )
			{
				int ns = ctx->textbuffersize + TEXTBUFFER_INCREMENT;
				char *ntb = l_malloc( ctx, ns );
				memcpy( ntb, ctx->textbuffer, ctx->textbuffersize );
				l_free( ctx, ctx->textbuffer );
				ctx->textbuffer = ntb;
				ctx->textbuffersize = ns;
			}

			ctx->textbuffer[ ctx->textbufferoffset++ ] = ch;

			continue;
		}

dotext:
		if( ctx->textbufferoffset )
		{
#if USE_LIBUNICODE
			char *tmp;
			int newlen;

			tmp = malloc( ctx->textbufferoffset * 4 );
			newlen = layout_charsetconv( ctx, ctx->textbuffer, tmp, ctx->textbufferoffset );
			// Create a text object out of the current text buffer
			DoMethod( ctx->current_container,
				MM_Layout_Group_AddText,
				tmp, newlen,
				ctx->currentstyle->font,
				ctx->currentstyle->fontarray,
				ctx->currentstyle->color,
				ctx->currentstyle->fontstyle | FSF_PRE,
				ctx->current_anchor
			);
			ctx->textbufferoffset = 0;
			free( tmp );
#else
			// Create a text object out of the current text buffer
			DoMethod( ctx->current_container,
				MM_Layout_Group_AddText,
				ctx->textbuffer, ctx->textbufferoffset,
				ctx->currentstyle->font,
				ctx->currentstyle->fontarray,
				ctx->currentstyle->color,
				ctx->currentstyle->fontstyle | FSF_PRE,
				ctx->current_anchor
			);
			ctx->textbufferoffset = 0;
#endif
		}

		// And another one...
		o = NewObject( getlobrclass(), NULL,
			MA_Layout_Context, ctx,
			MA_Layout_Div_Height, ctx->currentstyle->font->tf_YSize,
			MA_Layout_Div_Align, align_newrow,
			TAG_DONE
		);

		DoMethod( ctx->current_container, MM_Layout_Group_AddObject, o );
	}

	if( ctx->textbufferoffset )
	{
#if USE_LIBUNICODE
			char *tmp;
			int newlen;

			tmp = malloc( ctx->textbufferoffset * 4 );
			newlen = layout_charsetconv( ctx, ctx->textbuffer, tmp, ctx->textbufferoffset );
			// Create a text object out of the current text buffer
			DoMethod( ctx->current_container,
				MM_Layout_Group_AddText,
				tmp, newlen,
				ctx->currentstyle->font,
				ctx->currentstyle->fontarray,
				ctx->currentstyle->color,
				ctx->currentstyle->fontstyle | FSF_PRE,
				ctx->current_anchor
			);
			ctx->textbufferoffset = 0;
			free( tmp );
#else
		// Create a text object out of the current text buffer
		DoMethod( ctx->current_container,
			MM_Layout_Group_AddText,
			ctx->textbuffer, ctx->textbufferoffset,
			ctx->currentstyle->font,
			ctx->currentstyle->fontarray,
			ctx->currentstyle->color,
			ctx->currentstyle->fontstyle | FSF_PRE,
			ctx->current_anchor
		);
		ctx->textbufferoffset = 0;
#endif
	}

	if( aborthere )
		return( olddata - begindata );
	else
		return( (char*)data - (char*)begindata );
}
