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
** $Id: parse.c,v 1.84 2003/10/16 09:53:07 neko Exp $
**
*/

#include "voyager.h"
#include "malloc.h"
#include "parse.h"
#include "layout.h"

// Windows to ISO-8859-1-Table
#if !USE_LIBUNICODE
#include "charmap.h"
#endif

void convertentities( char *from, char *to );

#if USE_LIBUNICODE
static struct layout_ctx *current_ctx;

void parse_setcurrentlayoutctx( struct layout_ctx *ctx )
{
	current_ctx = ctx;
}

#endif

/*static int casecmp( char *from, char *what, int len )
{
	while( len-- )
	{
		if( toupper( *from ) != *what++ )
			return( TRUE );
		from++;
	}
	return( FALSE );
}*/

#define NUMTOKENARGS 16
#define TOKENARGBUFF 1536

struct tokenarg {
	char name[ 32 ];
	int talen;
	int isconverted;
	char *data;
#if USE_LIBUNICODE
	char *data_utf8;
#endif
} tokenargs[ NUMTOKENARGS ];
char *tokenargs_name;
char *tokenargs_id;

static int tokenargcnt;

void init_tokenbuff( void )
{
	int c;

	D( db_init, bug( "initializing..\n" ) );

	for( c = 0; c < NUMTOKENARGS; c++ )
	{
		tokenargs[ c ].data = malloc( TOKENARGBUFF );
		memset( tokenargs[ c ].data, '\0', TOKENARGBUFF ); /* TOFIX: maybe not needed */
#if USE_LIBUNICODE
		tokenargs[ c].data_utf8 = malloc( TOKENARGBUFF * 4 );
		memset( tokenargs[ c ].data_utf8, '\0', TOKENARGBUFF * 4 ); /* TOFIX: maybe not needed */
#endif
	}
}

static char *splittokenargs( char *from )
{
	struct tokenarg *ta = tokenargs;
	int tl;
	struct tokenarg *ta_name = NULL;
	struct tokenarg *ta_id = NULL;

	tokenargcnt = 0;

	from = stpblk( from );

	while( tokenargcnt < NUMTOKENARGS )
	{
		if( !*from || *from == '>' )
			break;

		// Begin of token argument
		for( tl = 0; ; tl++ )
		{
			if( !*from || *from == '>' || isspace( *from ) || *from == '=' )
				break;
			if( tl < sizeof( ta->name ) )
				ta->name[ tl ] = toupper( *from );
			from++;
		}

		if( tl < sizeof( ta->name ) )
			ta->name[ tl ] = 0;
		ta->talen = tl;

		//Printf( "storing val %s\n", ta->name );

		// Skip blanks
		from = stpblk( from );

		if( *from != '=' )
		{
			ta->data[ 0 ] = 0;
			ta->isconverted = FALSE;
			tokenargcnt++;
			ta++;
			continue;
		}

		from = stpblk( from + 1 );
		// Copy over data
		if( *from == '"' || *from == '\'' )
		{
			int closing_char = *from++;

			// quoted, copy till quote
			for( tl = 0; *from && *from != closing_char; from++ )
			{
				if( tl < TOKENARGBUFF - 2 )
					ta->data[ tl++ ] = *from;
			}
			// skip closing quote
			if( *from )
				from++;
			ta->data[ tl ] = 0;
		}
		else
		{
			// unquoted, copy till space
			for( tl = 0; *from && *from != '>' && !isspace( *from ); from++ )
			{
				if( tl < TOKENARGBUFF - 2 )
					ta->data[ tl++ ] = *from;
			}
			ta->data[ tl ] = 0;
		}

		if( ta->talen == 2 && !strcmp( ta->name, "ID" ) )
			ta_id = ta;
		else if( ta->talen == 4 && !strcmp( ta->name, "NAME" ) )
			ta_name = ta;

		ta->isconverted = FALSE;
		tokenargcnt++;
		ta++;
		from = stpblk( from );

	}

	tokenargs_id = NULL;
	if( ta_name )
	{
		tokenargs_name = tokenargs_id = ta_name->data;
	}
	else
	{
		tokenargs_name = NULL;
	}
	if( ta_id )
	{
		tokenargs_id = ta_id->data;
	}

	if( tokenargcnt == NUMTOKENARGS )
	{
		while( *from && *from != '>' )
			from++;
	}

	return( *from ? from + 1 : NULL );
}

static char *gettokenarg_cv( char *what, int convert )
{
	int c, wl = strlen( what );

	for( c = 0; c < tokenargcnt; c++ )
	{
		if( tokenargs[ c ].talen == wl && !strcmp( tokenargs[ c ].name, what ) )
		{
			if( convert && !tokenargs[ c ].isconverted )
			{
				char tempbuff[ TOKENARGBUFF ], *p = tempbuff, *p2 = tokenargs[ c ].data;

				convertentities( tokenargs[ c ].data, tempbuff );
				while( *p )
				{
					if( isspace( *p ) )
						*p2++ = ' ';
					else
						*p2++ = *p;
					p++;
				}
				*p2 = 0;
#if USE_LIBUNICODE
				{
					layout_charsetconv( current_ctx, tokenargs[ c ].data, tokenargs[ c ].data_utf8, strlen( tokenargs[ c ].data ) );
				}
#endif
				tokenargs[ c ].isconverted = TRUE;
			}
#if USE_LIBUNICODE
			return( tokenargs[ c ].isconverted ? tokenargs[ c ].data_utf8 : tokenargs[ c ].data );
#else
			return( tokenargs[ c ].data );
#endif
		}
	}
	return( 0 );
}

int gettokenarg_cnt( int cnt, char **name, char **val )
{
	if( cnt >= tokenargcnt )
		return( FALSE );
	*name = tokenargs[ cnt ].name;
	*val = tokenargs[ cnt ].data;
	return( TRUE );
}

struct token {
	char *name;
	UWORD len;
	UWORD code;
};

#include "html.h"

#define DEFTOKEN(s,c) {s,sizeof(s)-1,c},

/*
	NOTA BENE: This table MUST be sorted by length!
*/

static __far struct token tokens[] = {
DEFTOKEN( "b", ht_b )
DEFTOKEN( "s", ht_s )
DEFTOKEN( "i", ht_i )
DEFTOKEN( "a", ht_a )
DEFTOKEN( "p", ht_p )
DEFTOKEN( "u", ht_u )
DEFTOKEN( "h1", ht_h1 )
DEFTOKEN( "h2", ht_h2 )
DEFTOKEN( "h3", ht_h3 )
DEFTOKEN( "h4", ht_h4 )
DEFTOKEN( "h5", ht_h5 )
DEFTOKEN( "h6", ht_h6 )
DEFTOKEN( "br", ht_br )
DEFTOKEN( "hr", ht_hr )
DEFTOKEN( "li", ht_li )
DEFTOKEN( "ul", ht_ul )
DEFTOKEN( "ol", ht_ol )
DEFTOKEN( "dd", ht_dd )
DEFTOKEN( "dl", ht_dl )
DEFTOKEN( "dt", ht_dt )
DEFTOKEN( "tr", ht_tr )
DEFTOKEN( "td", ht_td )
DEFTOKEN( "th", ht_th )
DEFTOKEN( "em", ht_em )
DEFTOKEN( "tt", ht_tt )
DEFTOKEN( "pre", ht_pre )
DEFTOKEN( "dir", ht_dir )
DEFTOKEN( "img", ht_img )
DEFTOKEN( "del", ht_del )
DEFTOKEN( "ins", ht_ins )
DEFTOKEN( "map", ht_map )
DEFTOKEN( "div", ht_div )
DEFTOKEN( "big", ht_big )
DEFTOKEN( "xmp", ht_xmp )
DEFTOKEN( "kbd", ht_kbd )
DEFTOKEN( "dfn", ht_dfn )
DEFTOKEN( "var", ht_var )
DEFTOKEN( "span", ht_span )
DEFTOKEN( "code", ht_code )
DEFTOKEN( "samp", ht_samp )
DEFTOKEN( "cite", ht_cite )
DEFTOKEN( "menu", ht_menu )
DEFTOKEN( "form", ht_form )
DEFTOKEN( "nobr", ht_nobr )
DEFTOKEN( "body", ht_body )
DEFTOKEN( "font", ht_font )
DEFTOKEN( "meta", ht_meta )
DEFTOKEN( "area", ht_area )
DEFTOKEN( "base", ht_base )
DEFTOKEN( "param", ht_param )
DEFTOKEN( "embed", ht_embed )
DEFTOKEN( "style", ht_style )
DEFTOKEN( "title", ht_title )
DEFTOKEN( "image", ht_img )
DEFTOKEN( "table", ht_table )
DEFTOKEN( "input", ht_input )
DEFTOKEN( "small", ht_small )
DEFTOKEN( "frame", ht_frame )
//DEFTOKEN( "thead", ht_thead )
//DEFTOKEN( "tbody", ht_tbody )
DEFTOKEN( "script", ht_script )
DEFTOKEN( "button", ht_button )
DEFTOKEN( "strike", ht_strike )
DEFTOKEN( "center", ht_center )
DEFTOKEN( "centre", ht_center )
DEFTOKEN( "select", ht_select )
DEFTOKEN( "option", ht_option )
DEFTOKEN( "strong", ht_strong )
DEFTOKEN( "applet", ht_applet )
DEFTOKEN( "comment", ht_comment )
DEFTOKEN( "address", ht_address )
DEFTOKEN( "isindex", ht_isindex )
DEFTOKEN( "listing", ht_listing )
DEFTOKEN( "caption", ht_caption )
DEFTOKEN( "noembed", ht_noembed )
DEFTOKEN( "textarea", ht_textarea )
DEFTOKEN( "colgroup", ht_colgroup )
DEFTOKEN( "frameset", ht_frameset )
DEFTOKEN( "noframes", ht_noframes )
DEFTOKEN( "noscript", ht_noscript )
DEFTOKEN( "basefont", ht_basefont )
DEFTOKEN( "blockquote", ht_blockquote )
{0,32767,0}
};

struct entity {
	char *name;
	UBYTE len;
	UBYTE ch;
};

#define DEFENT(s,c) {s,sizeof(s)-1,c},
static __far struct entity entities[] = {

DEFENT("excl",'!')
DEFENT("quot",'"')
//DEFENT("num",'#')
DEFENT("dollar",'$')
DEFENT("percnt",'%')
DEFENT("amp",'&')
DEFENT("apos",'\'')
DEFENT("lpar",'(')
DEFENT("rpar",')')
DEFENT("ast",'*')
DEFENT("plus",'+')
//DEFENT("comma",',')
DEFENT("hyphen",'-')
DEFENT("minus",'-')
DEFENT("dash",'-')
DEFENT("period",'.')
DEFENT("sol",'/')
DEFENT("colon",':')
DEFENT("semi",';')
DEFENT("lt",'<')
DEFENT("equals",'=')
DEFENT("gt",'>')
DEFENT("quest",'?')
DEFENT("commat",'@')
DEFENT("lsqb",'[')
DEFENT("bsol",'\\')
DEFENT("rsqb",']')
DEFENT("circ",'^')
DEFENT("caret",'^')
DEFENT("lowbar",'_')
DEFENT("grave",'`')
DEFENT("lcub",'{')
DEFENT("verbar",'|')
DEFENT("rcub",'}')
DEFENT("tilde",'~')
DEFENT("sim",'~')
#if USE_LIBUNICODE
DEFENT("nbsp",127)
#else
DEFENT("nbsp",160)
#endif
DEFENT("iexcl",'¡')
DEFENT("cent",'¢')
DEFENT("pound",'£')
DEFENT("curren",'¤')
DEFENT("yen",'¥')
DEFENT("brvbar",'¦')
DEFENT("brkbar",'¦')
DEFENT("sect",'§')
DEFENT("uml",'¨')
DEFENT("die",'¨')
DEFENT("copy",'©')
DEFENT("COPY",'©') /* workaround for stupid sites */
DEFENT("ordf",'ª')
DEFENT("laquo",'«')
DEFENT("not",'¬')
DEFENT("shy",'­')
DEFENT("reg",'®')
DEFENT("macr",'¯')
DEFENT("hibar",'¯')
DEFENT("deg",'°')
DEFENT("plusmn",'±')
DEFENT("sup2",'²')
DEFENT("sup3",'³')
DEFENT("acute",'´')
DEFENT("micro",'µ')
DEFENT("para",'¶')
DEFENT("middot",'·')
DEFENT("cedil",'¸')
DEFENT("sup1",'¹')
DEFENT("ordm",'º')
DEFENT("raquo",'»')
DEFENT("frac14",'¼')
DEFENT("frac12",'½')
DEFENT("half",'½')
DEFENT("frac34",'¾')
DEFENT("iquest",'¿')
DEFENT("Agrave",'À')
DEFENT("Aacute",'Á')
DEFENT("Acirc",'Â')
DEFENT("Atilde",'Ã')
DEFENT("Auml",'Ä')
DEFENT("Aring",'Å')
DEFENT("angst",'Å')
DEFENT("AElig",'Æ')
DEFENT("Ccedil",'Ç')
DEFENT("Egrave",'È')
DEFENT("Eacute",'É')
DEFENT("Ecirc",'Ê')
DEFENT("Euml",'Ë')
DEFENT("Igrave",'Ì')
DEFENT("Iacute",'Í')
DEFENT("Icirc",'Î')
DEFENT("Iuml",'Ï')
DEFENT("ETH",'Ð')
DEFENT("Ntilde",'Ñ')
DEFENT("Ograve",'Ò')
DEFENT("Oacute",'Ó')
DEFENT("Ocirc",'Ô')
DEFENT("Otilde",'Õ')
DEFENT("Ouml",'Ö')
DEFENT("times",'×')
DEFENT("Oslash",'Ø')
DEFENT("Ugrave",'Ù')
DEFENT("Uacute",'Ú')
DEFENT("Ucirc",'Û')
DEFENT("Uuml",'Ü')
DEFENT("Yacute",'Ý')
DEFENT("THORN",'Þ')
DEFENT("szlig",'ß')
DEFENT("agrave",'à')
DEFENT("aacute",'á')
DEFENT("acirc",'â')
DEFENT("atilde",'ã')
DEFENT("auml",'ä')
DEFENT("aring",'å')
DEFENT("aelig",'æ')
DEFENT("ccedil",'ç')
DEFENT("egrave",'è')
DEFENT("eacute",'é')
DEFENT("ecirc",'ê')
DEFENT("euml",'ë')
DEFENT("igrave",'ì')
DEFENT("iacute",'í')
DEFENT("icirc",'î')
DEFENT("iuml",'ï')
DEFENT("eth",'ð')
DEFENT("ntilde",'ñ')
DEFENT("ograve",'ò')
DEFENT("oacute",'ó')
DEFENT("ocirc",'ô')
DEFENT("otilde",'õ')
DEFENT("ouml",'ö')
DEFENT("divide",'÷')
DEFENT("oslash",'ø')
DEFENT("ugrave",'ù')
DEFENT("uacute",'ú')
DEFENT("ucirc",'û')
DEFENT("uuml",'ü')
DEFENT("yacute",'ý')
DEFENT("thorn",'þ')
DEFENT("yuml",'ÿ')
DEFENT("euro",'e')
DEFENT("lsquo",'`')
DEFENT("rsquo",'\'')

DEFENT("OElig",'Ø')
DEFENT("oelig",'ø' )
DEFENT("Scaron",'S')
DEFENT("scaron",'s' )
DEFENT("ensp",' ' )
DEFENT("emsp",' ' )
DEFENT("thinsp", ' ' )
DEFENT("zwnj", ' ' )
DEFENT("zwj", ' ' )
DEFENT("lrm", '.' )
DEFENT("rlm", '.' )
DEFENT("ndash", '-' )
DEFENT("mdash", '-' )
DEFENT("sbquo", '´' )
DEFENT("ldquo", '"' )
DEFENT("rdquo", '"' )
DEFENT("bdquo", '"' )
DEFENT("dagger", '/' )
DEFENT("Dagger", '/' )
DEFENT("permil", '%' )
DEFENT("bullet", '·' )
DEFENT("bull", '·' )	/* this is against the specs, a for-AmigaOS hack ;) */
DEFENT("beta", 'ß' )
DEFENT("trade",'™' )	/* might not be in all fonts (Windows 1252 codepage) */
{0,0,0}
};

static UBYTE getentity( char *ent, int *lenp )
{
	struct entity *e = entities;
	int len = 0;
	int entlen = 0;
	int ch;
	char *p;

	if( *ent == '#' )
	{
		len++;
		ent++;

		// digit entity
		if( isdigit( *ent ) )
		{
			char bf[ 16 ];

			while( isdigit( *ent ) && entlen < 16 )
				bf[ entlen++ ] = *ent++;

			bf[ entlen ] = 0;
			len += entlen;

			if( *ent == ';' )
				len++;
			*lenp = len;

			ch = atoi( bf );

			if( ch && ch < 256 )
#if USE_LIBUNICODE
				return( ch );
#else
				return( charmap[ ch ] );
#endif
			else switch( ch )
			{
				case 0: // dummy... for the warning
				default:
					return( ' ' );
			}
		}
		// hex digit entity
		else if( *ent == 'x' || *ent == 'X' )
		{
			char bf[ 16 ];
			int ch;

			ent++;
			entlen++;

			while( isxdigit( *ent ) && entlen < 16 )
				bf[ entlen++ ] = *ent++;

			bf[ entlen ] = 0;
			len += entlen;

			if( *ent == ';' )
				len++;
			*lenp = len;

			stch_l( &bf[ 1 ], (long*)&ch );

			if( ch && ch < 256 )
#if USE_LIBUNICODE
				return( ch );
#else
				return( charmap[ ch ] );
#endif
			else switch( ch )
			{
				case 0: // dummy... for the warning
				default:
					return( ' ' );
			}
		}
	}

	// named entity
	p = ent;
	while( isalnum( *p ) )
	{
		p++;
		entlen++;
	}

	len += entlen;

	if( *p == ';' )
		len++;
	*lenp = len;

	for( ; e->name; e++ )
	{
		if( e->len == entlen && !memcmp( e->name, ent, entlen ) )
			return( e->ch );
	}

/*
	PutStr( "Unknown entity: " );
	WriteChars( ent, entlen );
	PutStr( "\n" );
*/

	return( 0 );
}

void convertentities( char *from, char *to )
{
	while( *from )
	{
		if( *from == '&' && !isspace( from[ 1 ] ) )
		{
			int entlen;
			int ch = getentity( from + 1, &entlen );
			if( ch )
			{
				*to++ = ch;
				from += entlen + 1;
			}
			else
				*to++ = *from++;
		}
		else
			*to++ = *from++;
	}
	*to = 0;
}

ULONG gettoken( char **text, int *lineno )
{
	char *p = *text;
	char bf[ 256 ];
	int c;
	int tokenlen;
	ULONG negate = FALSE;
	struct token *toklist;

redo:

	if( *p != '<' )
	{
		ULONG ch = *p;

		if( !ch )
			return( 0 );

		if( ch == '&' && !isspace( p[ 1 ] ) )   // Entity
		{
			int entlen;
			ch = getentity( p + 1, &entlen );
			if( !ch )
			{
				if( !p[ entlen + 1 ] )
					return( 0 );
				ch = *p;
				goto skipit;
			}
			*text = *text + ( entlen + 1 );
#if USE_LIBUNICODE
			return( ch );
#else
			return( charmap[ ch ] );
#endif
		}

skipit:
		if( ch == '\n' )
			*lineno = *lineno + 1;

#if !USE_LIBUNICODE
		// Just in case someone is using CED for
		// web page design :)
		if( ch == 160 )
			ch = ' ';
#endif

		*text = *text + 1;
#if USE_LIBUNICODE
		return( ch );
#else
		return( charmap[ ch ] );
#endif
	}

	p++;

	// skip comments
	if( p[0]=='!' && p[1]=='-' && p[2]=='-' )
	{
		int commentdepth = 0;
		int oldlineno = *lineno;

		p += 3;
		while( *p )
		{
			if( *p == '\n' )
				*lineno = *lineno + 1;
			if( !strncmp( p, "-->", 3 ) && !commentdepth )
				break;
			/*else if( !strncmp( p, "--", 2 ) )
			{
				commentdepth = !commentdepth;
				p++;
			}*/
			p++;
		}

		if( *p )
		{
			p += 3;
		}
		else
		{
			*lineno = oldlineno;
			return( 0 );

			// fallback, comment not terminated properly
			/*p = oldp;
			while( *p && *p != '>' )
				p++;
			if( *p )
				p++;
			else
				return( 0 );

			*text = p;
			return( ht_dummy );*/
		}

		*text = p;
		goto redo;
	}

	if( *p == '/' )
	{
		negate = HTF_NEGATE;
		p++;
	}

	for( c = 0; c < 255; c++ )
	{
		if( !p[ c ] || p[ c ] == '>' || isspace( p[ c ] ) )
			break;
		bf[ c ] = p[ c ];
	}
	bf[ c ] = 0;

	if( !p[ c ] )
		return( 0 );

	if( !bf[ 0 ] || bf[ 0 ] == '<' || bf[ 0 ] == '&' || isspace( bf[ 0 ] ) )
	{
		// This is apparently an incomplete tag
		*text = *text + 1;
		return( '<' );
	}

	tokenlen = c;


	p += c;

	// c = token-len

	p = splittokenargs( p );
	// Incomplete token
	if( !p )
	{
		*text = strchr( *text, 0 );
		return( 0 );
	}

	*text = p;

	strlwr( bf );

	toklist = tokens;

	while( toklist->len <= tokenlen )
	{
		if( toklist->len == tokenlen && !strcmp( toklist->name, bf ) )
		{
			return( (ULONG)toklist->code | negate );
		}
		toklist++;
	}

	return( ht_dummy );
}

char *getargs( char *token )
{
	return( gettokenarg_cv( token, TRUE ) );
}

char *getargs_ne( char *token )
{
	char *a = gettokenarg_cv( token, TRUE );
	if( a && !*a )
		a = 0;
	return( a );
}

char *getargs_def( char *token, char *def )
{
	char *p = gettokenarg_cv( token, TRUE );
	return( p ? p : def );
}

char *getargsncv( char *token )
{
	return( gettokenarg_cv( token, FALSE ) );
}


void pushtokenargs( char *args )
{
	splittokenargs( args );
}

static int mystrtol( char *str )
{
	return( strtol( str, NULL, 0 ) );
}

LONG getnumarg( char *token, LONG defval )
{
	char *p = getargs( token );
	if( p )
		return( mystrtol( p ) );
	else
		return( defval );
}

// Get a positive only value
LONG getnumargp( char *token, LONG defval )
{
	char *p = getargs( token );
	if( p )
		return( max( 0, mystrtol( p ) ) );
	else
		return( defval );
}

LONG getnumargmm( char *token, LONG defval, int minval, int maxval )
{
	char *p = getargs( token );
	if( p )
	{
		int v = mystrtol( p );
		v = max( minval, v );
		v = min( maxval, v );
		return( v );
	}
	else
		return( defval );
}

int getboolarg( char *token, int defval )
{
	char *p = getargs( token );
	if( p )
	{
		if( !strcmp( p, "0" ) )
			return( FALSE );
		if( !strcmp( p, "1" ) )
			return( TRUE );
		if( !stricmp( p, "yes" ) )
			return( TRUE );
		if( !stricmp( p, "no" ) )
			return( FALSE );
		if( !*p )
			return( TRUE );
	}
	return( defval );
}

LONG STDARGS getstatearg( char *token, LONG default_not_there, LONG default_unknown, ... )
{
	va_list va;
	char *p;
	char **a;

	va_start( va, default_unknown );

	p = getargs( token );
	a = va_arg( va, char ** );

	va_end( va );

	if( !p )
		return( default_not_there );

	strupr( p );

	// now iterate the list
	while( *a )
	{
		if( !strcmp( *a, p ) )
			return( (LONG)a[ 1 ] );
		a += 2;
	}

	return( default_unknown );
}

void encodedata( char *from, char *to )
{
	while( *from )
	{
		if( *from == ' ' )
		{
			from++;
			*to++ = '+';
		}
		else if( strchr( "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.-_@", *from ) )
			*to++ = *from++;
		else if( *from == '\n' )
		{
			from++;
			strcpy( to, "%0D%0A" );
			to += 6;
		}
		else
		{
			sprintf( to, "%%%02lX", (long unsigned int)*from++ );
			to += 3;
		}
	}
	*to = 0;
}
