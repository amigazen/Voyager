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


#ifndef VOYAGER_JS_H
#define VOYAGER_JS_H
/*
 * $Id: js.h,v 1.139 2004/11/16 16:01:15 zapek Exp $
 */

#ifdef __MORPHOS__
#include "classes.h"
#include <setjmp.h>
#endif /* __MORPHOS__ */

void process_nan( STRPTR s );

#ifdef __MORPHOS__
APTR STDARGS JSNewObject( APTR class, ... ) __attribute__((varargs68k));
/*
 * This is to force gcc to read as integer and avoid
 * using the FPU so that there's no alignement problems.
 */
#define TO_DOUBLE(x) ({ double d; memcpy(&d, x, sizeof(d)); d; })
#else
APTR STDARGS JSNewObject( APTR class, ... );
#define TO_DOUBLE(x) *((double*)x)
#endif /* !__MORPHOS__ */

extern double *double_nan;
extern double *double_inf;

struct yystype {
	double real;
	int integer;
	char *text;
	int re_flags_g;
	int re_flags_i;
};

struct jslineinfo {
	UWORD lineno;
	char url[ 2048 ];
};

#define MAXVARNAMES 32

typedef struct expr_stack {
	struct MinList l;
	APTR pool;
	jmp_buf *rb, myrb;

	struct MinList vl; // variable push list
	struct MinList marklist; // marker list
	struct MinList contextlist;

	ULONG func_context; // function context counter

	APTR currentcontextobj; // context object
	APTR thisobject;
	APTR window;

	char errorbuffer[ 128 ]; // error message buffer
	APTR errorobject;

	int varnameptr;
	char varname[ MAXVARNAMES + 1 ][ 256 ];
	int varname_ref[ MAXVARNAMES + 1 ]; // Expression stack holds reference

	APTR default_object;

	struct jsop_list *jso;
} expr_stack;

typedef struct evalcache {
	struct MinNode n;
	int l;
	int index;
	char txt[ 0 ];
} evalcache;

typedef struct jsop_list {
	UWORD *i;            // instruction table
	int instsize;        // size of table
	int installoc;       // allocated size

	struct expr_stack *es;

	APTR pool;

	struct MinList vars;    // automatically generated objects

	struct MinList eval_cache; // cache for eval() stuff <sigh>

	APTR go_navigator;
	APTR go_event;          // Current event object

	STRPTR baseref;

} jsop_list;

enum {
	JSOP_dummy = 1,

	JSOP_OP_SUB,
	JSOP_OP_SUBONE,
	JSOP_OP_ADD,
	JSOP_OP_ADDONE,
	JSOP_OP_NEG,
	JSOP_OP_MUL,
	JSOP_OP_DIV,
	JSOP_OP_MOD,
	JSOP_OP_BINAND,
	JSOP_OP_BINOR,
	JSOP_OP_BINEOR,
	JSOP_OP_BINNEG,
	JSOP_OP_NEGATE,
	JSOP_OP_BOOLNEG,
	JSOP_OP_BOOLAND,
	JSOP_OP_BOOLOR,
	JSOP_OP_LT,
	JSOP_OP_GT,
	JSOP_OP_LTEQ,
	JSOP_OP_GTEQ,
	JSOP_OP_EQ,
	JSOP_OP_STREQ,
	JSOP_OP_GG,
	JSOP_OP_LL,
	JSOP_OP_GGG,
	JSOP_OP_SELECT,
	JSOP_OP_SELECT_SKIP,
	JSOP_OP_SELECT_END,

	JSOP_OP_TYPEOF,

	JSOP_OP_BOOLAND_END,
	JSOP_OP_BOOLOR_END,

	JSOP_OP_POPVAL,

	JSOP_PUSH_ARRAYOP,

	JSOP_PUSH_OCONTEXT,
	JSOP_POP_OCONTEXT,

	JSOP_EVAL_LVALUE,
	JSOP_EVAL_LVALUE_CHECK,
	JSOP_EVAL_LVALUE_AND_KEEP,

	JSOP_ASSIGN,
	JSOP_ASSIGN_AND_KEEP,
	JSOP_ASSIGNPROP,
	JSOP_ASSIGNLOCAL,
	JSOP_ASSIGNARRAY,
	JSOP_SETIFUNSET,

	JSOP_EX_DEBUG,
	JSOP_EX_DEBUGESTACK,

	JSOP_EX_IF,
	JSOP_EX_ELSE,
	JSOP_EX_ENDIF,

	JSOP_EX_WHILEBEGIN,
	JSOP_EX_WHILE,
	JSOP_EX_WHILEEND,

	JSOP_EX_BREAK,
	JSOP_EX_CONTINUE,

	JSOP_FUNC_END,
	JSOP_FUNC_RETURN,
	JSOP_FUNC_RETURN_NOVAL,
	JSOP_FUNC_CALL,
	JSOP_FUNC_ASSIGNPARM,
	JSOP_FUNC_PARMCLEANUP,
	JSOP_FUNC_BEGINPARMS,

	JSOP_EVAL,

	JSOP_FOR_PRECOND,
	JSOP_FOR_COND,
	JSOP_FOR_BODY,
	JSOP_FOR_END,

	JSOP_ISNAN,
	JSOP_ISFINITE,
	JSOP_PARSEINT,
	JSOP_PARSEFLOAT,
	JSOP_MAKESTRING,
	JSOP_MAKENUMBER,
	JSOP_ESCAPE,
	JSOP_UNESCAPE,

	JSOP_PUSH_THIS,
	JSOP_PUSH_NULL,
	JSOP_PUSH_UNDEFINED,

	JSOP_CREATEOBJECT,

	JSOP_CREATEARRAY,

	JSOP_NEW,
	JSOP_NEW_END, // pop ocontext
	JSOP_DELETE,

	JSOP_END,           // end execution

	JSOP_FUNC_BEGIN_EXPR,

	JSOP_SWITCH_START,
	JSOP_SWITCH_END,
	JSOP_CASE_START,
	JSOP_CASE,
	JSOP_CASE_END,
	JSOP_CASE_DEF,

	JSOP_EX_DOBEGIN,
	JSOP_EX_DOEND,
	JSOP_EX_DOPRECOND,

	JSOP_FORIN_BUILDPROPLIST,
	JSOP_FORIN_SET,
	JSOP_FORIN_END,

	JSOP_HASDATA = 8192, // following opcodes have data

	JSOP_LINENO, // set line number

	JSOP_PUSH_INT,
	JSOP_PUSH_REAL,
	JSOP_PUSH_STR,
	JSOP_PUSH_BOOL,

	JSOP_PUSH_OBJECT,
	JSOP_PUSH_FUNCPTR,

	JSOP_PUSH_VARNAME,
	JSOP_PUSH_REFOP,

	JSOP_FUNC_BEGIN,

	JSOP_CREATE_REGEXP,

	JSOP_max
};

#ifdef JSCRIPT_C

// This is ugly, and shouldn't be here.  The reason it *is* here is to keep it next to the list
// above, to encourage people to keep it up to date...

char __far *dbjsops[]={
	"JSOP_debug_dummy", //0

	"JSOP_dummy", //1

	"JSOP_OP_SUB", //2
	"JSOP_OP_SUBONE", //3
	"JSOP_OP_ADD", //4
	"JSOP_OP_ADDONE", //5
	"JSOP_OP_NEG", //6
	"JSOP_OP_MUL", //7
	"JSOP_OP_DIV",
	"JSOP_OP_MOD",
	"JSOP_OP_BINAND", //10

	"JSOP_OP_BINEOR",
	"JSOP_OP_BINNEG",
	"JSOP_OP_NEGATE",
	"JSOP_OP_BOOLNEG",
	"JSOP_OP_BOOLAND",
	"JSOP_OP_BOOLOR",
	"JSOP_OP_LT",
	"JSOP_OP_GT",
	"JSOP_OP_LTEQ", //20
	"JSOP_OP_GTEQ",
	"JSOP_OP_EQ",
	"JSOP_OP_STREQ",
	"JSOP_OP_GG",
	"JSOP_OP_LL",
	"JSOP_OP_GGG",
	"JSOP_OP_SELECT",
	"JSOP_OP_SELECT_SKIP",
	"JSOP_OP_SELECT_END",

	"JSOP_OP_TYPEOF", //30

	"JSOP_OP_BOOLAND_END",
	"JSOP_OP_BOOLOR_END",

	"JSOP_OP_POPVAL",

	"JSOP_PUSH_ARRAYOP",

	"JSOP_PUSH_OCONTEXT",
	"JSOP_POP_OCONTEXT",

	"JSOP_EVAL_LVALUE",
	"JSOP_EVAL_LVALUE_CHECK",
	"JSOP_EVAL_LVALUE_AND_KEEP",

	"JSOP_ASSIGN", //40
	"JSOP_ASSIGN_AND_KEEP",
	"JSOP_ASSIGNPROP",
	"JSOP_ASSIGNLOCAL",
	"JSOP_ASSIGNARRAY",
	"JSOP_SETIFUNSET",

	"JSOP_EX_DEBUG",
	"JSOP_EX_DEBUGESTACK",

	"JSOP_EX_IF",
	"JSOP_EX_ELSE",
	"JSOP_EX_ENDIF",

	"JSOP_EX_WHILEBEGIN", //50
	"JSOP_EX_WHILE",
	"JSOP_EX_WHILEEND",

	"JSOP_EX_BREAK",
	"JSOP_EX_CONTINUE",

	"JSOP_FUNC_END",
	"JSOP_FUNC_RETURN",
	"JSOP_FUNC_RETURN_NOVAL",
	"JSOP_FUNC_CALL",
	"JSOP_FUNC_ASSIGNPARM",
	"JSOP_FUNC_PARMCLEANUP",
	"JSOP_FUNC_BEGINPARMS",

	"JSOP_EVAL",

	"JSOP_FOR_PRECOND",
	"JSOP_FOR_COND",
	"JSOP_FOR_BODY",
	"JSOP_FOR_END",

	"JSOP_ISNAN",
	"JSOP_PARSEINT",
	"JSOP_PARSEFLOAT",
	"JSOP_MAKESTRING",
	"JSOP_MAKENUMBER",
	"JSOP_ESCAPE",
	"JSOP_UNESCAPE",

	"JSOP_PUSH_THIS",
	"JSOP_PUSH_NULL",

	"JSOP_CREATEOBJECT",

	"JSOP_CREATEARRAY",

	"JSOP_NEW",
	"JSOP_NEW_END", // pop ocontext
	"JSOP_DELETE",

	"JSOP_END",         // end execution

	"JSOP_FUNC_BEGIN_EXPR"
};

char * __far dbjsdops[]={
	"JSOP_HASDATA", // following opcodes have data

	"JSOP_LINENO", // set line number

	"JSOP_PUSH_INT",
	"JSOP_PUSH_REAL",
	"JSOP_PUSH_STR",
	"JSOP_PUSH_BOOL",
	"JSOP_PUSH_OBJECT",
	"JSOP_PUSH_FUNCPTR",

	"JSOP_PUSH_VARNAME",
	"JSOP_PUSH_REFOP",

	"JSOP_FUNC_BEGIN",

	"JSOP_CREATE_REGEXP",

	"JSOP_max"
};
#endif

#define SOP(x) jso_storeop( JSOP_##x, 0, 0 )
#define SOPD(x,d,l) jso_storeop( JSOP_##x, d, l )
#define SOPS(x,s) jso_storeop( JSOP_##x, s, strlen(s) )


typedef enum expr_type {
	expt_dummy,
	expt_real,
	expt_string,
	expt_bool,
	expt_obj,
	expt_funcptr,
	expt_argmarker,
	expt_undefined,

	expt_readonly = 128 // Flag value, to be ored
} expr_type;

// Property table
// used by findproperty()
typedef struct propt {
	char *name;         // NULL == end of table
	expr_type type;     // type of property (expt_)
	int id;             // ID (class-internal)
} propt;

// Property-related helper functions
struct propt *findprop( struct propt *table, char *propname );
int findpropid( struct propt *table, char *propname );
void storestrprop( struct MP_JS_GetProperty *msg, char *string );
void storestrlprop( struct MP_JS_GetProperty *msg, char *string, int strl );
void storefuncprop( struct MP_JS_GetProperty *msg, ULONG id );
void storerealprop( struct MP_JS_GetProperty *msg, double val );
void storeobjprop( struct MP_JS_GetProperty *msg, APTR obj );
void storeintprop( struct MP_JS_GetProperty *msg, int val );

// Deal with property lists
void js_lp_addcplist( struct MP_JS_ListProperties *msg, struct MinList *cpl );
void js_lp_addptable( struct MP_JS_ListProperties *msg, struct propt *pt );

// DOM sideband implementations

ULONG dom_hasproperty( struct MP_JS_HasProperty *msg, struct propt *ptable, struct MinList *cpl );
ULONG dom_getproperty( struct MP_JS_GetProperty *msg, struct propt *ptable, struct MinList *cpl );

#define DOM_INITCP NEWLIST( &data->cpl );data->gcmagic=(ULONG)-1
#define DOM_EXITCP cp_flush( &data->cpl )

#define DOM_HASPROP DECSMETHOD( JS_HasProperty ) { GETDATA; return( dom_hasproperty( msg, ptable, &data->cpl ) ); }

#define DOM_LISTPROP DECSMETHOD( JS_ListProperties ) { GETDATA; js_lp_addptable( msg, ptable); js_lp_addcplist( msg, &data->cpl ); return( TRUE ); }

#define DS_LISTPROP DECSMETHOD( JS_ListProperties ) { js_lp_addptable( msg, ptable); return( DOSUPER ); }

#define DOM_GETPROP DECSMETHOD( JS_GetProperty ) { GETDATA; ULONG rc = dom_getproperty( msg, ptable, &data->cpl ); if( rc <= 1 ) return( rc ); switch( rc ) {
#define DOM_PROP(x) case JSPID_##x:
#define DOM_ENDGETPROP } return( 0 ); }

#define DOM_SETPROP DECSMETHOD( JS_SetProperty ) { GETDATA; struct propt *pt = findprop( ptable, msg->propname ); if( !pt ) { 	if( msg->type != expt_obj ) return( FALSE ); cp_set( &data->cpl, msg->propname, *(APTR*)msg->dataptr ); return( TRUE ); } switch( pt->id ) {
#define DOM_ENDSETPROP } return( 0 ); }

#define JS_GC_HOOK \
 case MM_JS_SetGCMagic: { GETDATA; if( data->gcmagic != ((ULONG*)msg)[1] ) { struct customprop *cp; data->gcmagic = ((ULONG*)msg)[1]; for( cp = FIRSTNODE( &data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) ) if( cp->obj ) DoMethodA( cp->obj, (Msg)msg ); } return(TRUE); } \
 case MM_JS_GetGCMagic: { GETDATA; return( data->gcmagic ); }

#define DOM_JS_GC_HOOK \
 case MM_JS_SetGCMagic: { GETDATA; if( data->gcmagic != ((ULONG*)msg)[1] ) { struct customprop *cp; data->gcmagic = ((ULONG*)msg)[1]; for( cp = FIRSTNODE( &data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) ) if( cp->obj ) DoMethodA( cp->obj, (Msg)msg ); } if( data->formobject ) DoMethodA( data->formobject, (Msg)msg ); return(TRUE); } \
 case MM_JS_GetGCMagic: { GETDATA; return( data->gcmagic ); }

// expression stack manipulation functions
double exprs_pop_as_real( struct expr_stack *es );
LONG exprs_pop_as_funcptr( struct expr_stack *es, APTR *funcobjptr, APTR *fpobj, APTR *winobj );
int exprs_pop_as_bool( struct expr_stack *es );
void exprs_pop_as_string( struct expr_stack *es, char *to, int maxsize );
void exprs_pop_as_object( struct expr_stack *es, APTR *object );
int exprs_get_type( struct expr_stack *es, int which, int *strl );
char *exprs_peek_as_string( struct expr_stack *es, int which );
double exprs_peek_as_real( struct expr_stack *es, int which );
int exprs_pop_as_int( struct expr_stack *es );
void exprs_pop( struct expr_stack *es );
APTR exprs_peek_as_object( struct expr_stack *es, int which );

#define BEGINPTABLE static __far propt ptable[] = {
#define ENDPTABLE {NULL,0,0}};
#define DPROP(name,type) { #name, expt_##type, JSPID_##name },

// Global JS property IDs
enum jspid {
	JSPID_base = 1,

	JSPID_valueOf,
	JSPID_indexOf,
	JSPID_lastIndexOf,
	JSPID_charAt,
	JSPID_charCodeAt,
	JSPID_length,
	JSPID_substring,
	JSPID_fromCharCode,
	JSPID_substr,
	JSPID_toLowerCase,
	JSPID_toUpperCase,
	JSPID_split,

	JSPID_appName,     // 14
	JSPID_appCodeName,
	JSPID_appVersion,
	JSPID_language,
	JSPID_mimeTypes,
	JSPID_platform,
	JSPID_plugins,
	JSPID_userAgent,
	JSPID_javaEnabled,

	JSPID_name,        // 23
	JSPID_value,
	JSPID_defaultValue,
	JSPID_self,
	JSPID_window,
	JSPID_form,

	// Math object stuff

	JSPID_E,           // 29
	JSPID_LN10,
	JSPID_LN2,
	JSPID_LOG10E,
	JSPID_LOG2E,
	JSPID_PI,
	JSPID_SQRT1_2,
	JSPID_SQRT2,

	JSPID_abs,         // 37
	JSPID_acos,
	JSPID_asin,
	JSPID_atan,
	JSPID_atan2,
	JSPID_ceil,
	JSPID_cos,
	JSPID_exp,
	JSPID_floor,
	JSPID_log,
	JSPID_max,
	JSPID_min,
	JSPID_pow,
	JSPID_random,
	JSPID_round,
	JSPID_sin,
	JSPID_sqrt,
	JSPID_tan,

	// Window object

	JSPID_alert,       // 55
	JSPID_top,
	JSPID_document,
	JSPID_open,
	JSPID_location,
	JSPID_status,
	JSPID_navigator,
	JSPID_parent,
	JSPID_moveBy,
	JSPID_moveTo,
	JSPID_resizeBy,
	JSPID_resizeTo,
	JSPID_innerWidth,
	JSPID_innerHeight,
	JSPID_outerWidth,
	JSPID_outerHeight,
	JSPID_pageXOffset,
	JSPID_pageYOffset,
	JSPID_screenX,
	JSPID_screenY,
	JSPID_mouseVisible,

	// Document object

	JSPID_URL,         // 76
	JSPID_domain,
	JSPID_lastModified,
	JSPID_write,
	JSPID_writeln,
	JSPID_cookie,
	JSPID_images,
	JSPID_title,
	JSPID_links,
	JSPID_anchors,
	JSPID_forms,
	JSPID_referrer,
	JSPID_cookies,
	JSPID_embeds,
	JSPID_close,       // 90
#if USE_LO_PIP
	JSPID_pips,
#endif

	// Location object

	JSPID_href,
	JSPID_hash,
	JSPID_hostname,
	JSPID_host,
	JSPID_pathname,
	JSPID_port,
	JSPID_search,
	JSPID_reload,
	JSPID_protocol,

	// Screen object
	JSPID_height,
	JSPID_width,
	JSPID_availHeight,
	JSPID_availWidth,
	JSPID_pixelDepth,
	JSPID_colorDepth,

	// Array object
	JSPID_concat,
	JSPID_join,
	JSPID_pop,
	JSPID_push,
	JSPID_reverse,
	JSPID_shift,
	JSPID_slice,
	JSPID_splice,
	JSPID_sort,
	JSPID_unshift,

	// Event handler properties
	JSPID_onabort,
	JSPID_onblur,
	JSPID_onchange,
	JSPID_onclick,
	JSPID_ondblclick,
	JSPID_ondragdrop,
	JSPID_onerror,
	JSPID_onfocus,
	JSPID_onkeydown,
	JSPID_onkeyup,
	JSPID_onkeypress,
	JSPID_onload,
	JSPID_onmousedown,
	JSPID_onmousemove,
	JSPID_onmouseout,
	JSPID_onmouseover,
	JSPID_onmouseup,
	JSPID_onmove,
	JSPID_onreset,
	JSPID_onresize,
	JSPID_onselect,
	JSPID_onsubmit,
	JSPID_onunload,

	JSPID_blur,
	JSPID_focus,
	JSPID_click,
	JSPID_select,

	JSPID_checked,
	JSPID_defaultChecked,

	// Function
	JSPID_arguments,
	JSPID_arity,
	JSPID_caller,

	// Image
	JSPID_border,
	JSPID_complete,
	JSPID_hspace,
	JSPID_src,
	JSPID_lowsrc,
	JSPID_vspace,

	// Date
	JSPID_getDate,
	JSPID_getDay,   // Returns the day of the week for the specified date.
	JSPID_getHours, // Returns the hour in the specified date.
	JSPID_getMinutes,   // Returns the minutes in the specified date.
	JSPID_getMonth, // Returns the month in the specified date.
	JSPID_getSeconds,   // Returns the seconds in the specified date.
	JSPID_getTime,  // Returns the numeric value corresponding to the time for the specified date.
	JSPID_getTimezoneOffset,    // Returns the time-zone offset in minutes for the current locale.
	JSPID_getMilliseconds,
	JSPID_getYear,  // Returns the year in the specified date.
	JSPID_getFullYear,  // Returns the year in the specified date.
	JSPID_setDate,  // Sets the day of the month for a specified date.
	JSPID_setHours, // Sets the hours for a specified date.
	JSPID_setMinutes,   // Sets the minutes for a specified date.
	JSPID_setMonth, // Sets the month for a specified date.
	JSPID_setSeconds,   // Sets the seconds for a specified date.
	JSPID_setTime,  // Sets the value of a Date object.
	JSPID_setYear,  // Sets the year for a specified date.
	JSPID_setFullYear,  // Sets the year for a specified date.
	JSPID_setMilliseconds,

	JSPID_DATE_UTC_MARK_FIRST,

	JSPID_getUTCDate,  // Returns the day of the month for the specified date.
	JSPID_getUTCDay,   // Returns the day of the week for the specified date.
	JSPID_getUTCHours, // Returns the hour in the specified date.
	JSPID_getUTCMinutes,   // Returns the minutes in the specified date.
	JSPID_getUTCMonth, // Returns the month in the specified date.
	JSPID_getUTCSeconds,   // Returns the seconds in the specified date.
	JSPID_getUTCTime,  // Returns the numeric value corresponding to the time for the specified date.
	JSPID_getUTCTimezoneOffset,    // Returns the time-zone offset in minutes for the current locale.
	JSPID_getUTCMilliseconds,
	JSPID_getUTCYear,  // Returns the year in the specified date.
	JSPID_getUTCFullYear,  // Returns the year in the specified date.
	JSPID_setUTCDate,  // Sets the day of the month for a specified date.
	JSPID_setUTCHours, // Sets the hours for a specified date.
	JSPID_setUTCMinutes,   // Sets the minutes for a specified date.
	JSPID_setUTCMonth, // Sets the month for a specified date.
	JSPID_setUTCSeconds,   // Sets the seconds for a specified date.
	JSPID_setUTCTime,  // Sets the value of a Date object.
	JSPID_setUTCYear,  // Sets the year for a specified date.
	JSPID_setUTCFullYear,  // Sets the year for a specified date.
	JSPID_setUTCMilliseconds,

	JSPID_DATE_UTC_MARK_LAST,

	JSPID_parse,    // Returns the number of milliseconds in a date string since January 1, 1970, 00:00:00, local time.

	JSPID_toGMTString,  // Converts a date to a string, using the Internet GMT conventions.
	JSPID_toUTCString,
	JSPID_toLocaleString,   // Converts a date to a string, using the current locale's conventions.
	JSPID_UTC,  // Returns the number of milliseconds in a Date object since January 1, 1970, 00:00:00, Universal Coordinated Time (GMT).

	// Plugin/MimeType
	JSPID_enabledPlugin,
	JSPID_description,
	JSPID_suffixes,
	JSPID_type,
	JSPID_filename,

	// Window
	JSPID_confirm,

	JSPID_frames,

	JSPID_scroll,
	JSPID_scrollTo,
	JSPID_scrollBy,

	JSPID_action,
	JSPID_elements,
	JSPID_target,
	JSPID_method,
	JSPID_encoding,

	JSPID_history,

	JSPID_back,
	JSPID_forward,
	JSPID_go,
	JSPID_current,
	JSPID_next,
	JSPID_previous,

	JSPID_replace,
	JSPID_match,

	JSPID_route,

	JSPID_data,
	JSPID_modifiers,
	JSPID_which,
	JSPID_pagex,
	JSPID_pagey,
	JSPID_layerx,
	JSPID_layery,
	JSPID_screenx,
	JSPID_screeny,
	JSPID_x,
	JSPID_y,

#if USE_LO_PIP
	JSPID_channelName,
	JSPID_channelId,
	JSPID_bgPip,
#endif

	JSPID_playmp3,
	JSPID_stopmp3,

	JSPID_submit,
	JSPID_reset,

	// Timers
	JSPID_setTimeout,
	JSPID_setInterval,
	JSPID_clearTimeout,
	JSPID_clearInterval,

	// RegExp object
	JSPID_global,
	JSPID_ignoreCase,
	JSPID_input,
	JSPID_lastIndex,
	JSPID_lastMatch,
	JSPID_lastParen,
	JSPID_leftContext,
	JSPID_multiline,
	JSPID_rightContext,
	JSPID_source,
	JSPID_compile,
	JSPID_exec,
	JSPID_test,
	JSPID_re0,
	JSPID_re1,
	JSPID_re2,
	JSPID_re3,
	JSPID_re4,
	JSPID_re5,
	JSPID_re6,
	JSPID_re7,
	JSPID_re8,
	JSPID_re9,

	// STB object
	JSPID_CDPlayer,

	// STB.CDPlayer object
	JSPID_track,
	JSPID_trackMin,
	JSPID_trackSec,
	JSPID_trackFrame,
	JSPID_diskMin,
	JSPID_diskSec,
	JSPID_diskFrame,
	JSPID_numtracks,
	JSPID_ffwd,
	JSPID_frev,
	JSPID_play,
	JSPID_eject,
	JSPID_load,
	JSPID_pause,
	JSPID_stop,	// also window.stop()
	JSPID_tracks,
	JSPID_refresh,

	JSPID_ondiskchange,
	JSPID_ontrackchange,

	// Select object
	JSPID_options,
	JSPID_selectedIndex,

	// Select.option object
	JSPID_text,
	JSPID_index,
	JSPID_selected,
	JSPID_defaultSelected,

	// DOM stuff
	JSPID_getElementById,
	JSPID_getElementsByName,
	JSPID_all,

	// more window properties
	JSPID_find,
	JSPID_atob,
	JSPID_btoa,

	/* those should be above but someone added numbers so I don't know if it's supposed to be ordered or not */
	JSPID_cookieEnabled,
	JSPID_cpuClass,
	JSPID_onLine,
	JSPID_vendor,

	JSPID_end
};

extern struct jsop_list *cjsol;

//
// Helper functions dealing with custom properties
//

typedef struct customprop {
	struct MinNode n;
	APTR obj;
	char name[ 0 ];
} customprop;

struct customprop *cp_find( struct MinList *cpl, STRPTR name );
void cp_del( struct MinList *cpl, STRPTR name );
void cp_set( struct MinList *cpl, STRPTR name, APTR obj );
void cp_flush( struct MinList *cpl );
void cp_setreal( struct MinList *cpl, STRPTR name, double val );
void cp_setstr( struct MinList *cpl, STRPTR name, STRPTR str, int strsize );
int isnum( const char * );

struct proplistentry {
	struct MinNode n;
	char name[ 0 ];
};

// Generell implementation of "ToString" method
// (for objects which don't inherit js_object, grrr)
void js_tostring( char *classname, struct MP_JS_ToString *msg );

// Event types
enum {
	jse_begin = 0,

	jse_abort,
	jse_blur,
	jse_change,
	jse_click,
	jse_dblclick,
	jse_dragdrop,
	jse_error,
	jse_focus,
	jse_keydown,
	jse_keypress,
	jse_keyup,
	jse_load,
	jse_mousedown,
	jse_mousemove,
	jse_mouseout,
	jse_mouseover,
	jse_mouseup,
	jse_move,
	jse_reset,
	jse_resize,
	jse_select,
	jse_submit,
	jse_unload,
	jse_diskchange,

	jse_max
};

// JS control/helper functions

void js_run( APTR refwin, APTR obj, STRPTR baseref, UBYTE *script, int scriptlen, int lineno, char *resultbuffer, int rbsize, STRPTR docurl );

struct jsop_list *jso_init( void );
void jso_cleanup( struct jsop_list *jsol );
void jso_gc( struct jsop_list *jsol, ULONG magic );

void jso_storeop( int opcode, APTR data, int datasize );

int js_compile( APTR refwin, struct jsop_list *jol, UBYTE *script, int scriptlen, int lineno, STRPTR docurl );
int js_createfunccall( struct jsop_list *jso, int funcindex, int numargs, APTR *args );
void js_interpret( APTR refwin, APTR refobj, APTR thisobj, STRPTR baseref, struct jsop_list *jsol, int startix, char *resultbuffer, int rbsize, STRPTR docurl );
APTR js_interpret_obj( APTR refwin, APTR refobj, APTR thisobject, STRPTR baseref, struct jsop_list *jsol, int startix, STRPTR docurl );
void exprs_push_argmarker( struct expr_stack *es );

void STDARGS js_snoop( STRPTR, ... );
void js_opensnoop( void );

void js_gc( void );
void js_gc_add( APTR obj );
void js_gc_cleanup( void );


#endif /* VOYAGER_JS_H */
