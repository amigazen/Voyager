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
** $Id: jscript.c,v 1.121 2003/11/21 07:48:49 zapek Exp $
**
*/
#define JSCRIPT_C 1

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#endif
#include <math.h>

/* private */
#include "copyright.h"
#include "voyager_cat.h"
#include "prefs.h"
#include "classes.h"
#include "js.h"
#include "errorwin.h"
#include "htmlclasses.h"
#include "mui_func.h"
#include "time_func.h"
#include "malloc.h"
#include "dos_func.h"

/* Silly string to make listjs happy: MA_JS_ClassName, "" */

static APTR js_alloca_pool;
static jmp_buf jsjmpbuf;
static APTR errwin;
int js_lineno;
static int lastlineno;
static char *js_docurl;

BPTR js_snoop_win; // console-fh

extern void yyparse( void );
void lex_initialize( char *ptr, int len, int startline );

static ULONG dv_nan[ 2 ] = { 0xffffffff, 0xffffffff };
static ULONG dv_inf[ 2 ] = { 0x7ff00000, 0x00000000 }; 
double *double_nan = (double*)&dv_nan;
double *double_inf = (double*)&dv_inf;

void process_nan( STRPTR s )
{
#ifdef __SASC
	if( s[ 0 ] == 'N' )
		strcpy( s, "NaN" );
	else if( s[ 0 ] == 'I' )
		strcpy( s, "Infinity" );
#else
	if( s[ 0 ] && s[ 1 ] == 'i' )
		strcpy( s, "Infinity" );
#endif /* !__SASC */
}

void yyerror( char *errortext )
{
	if( getprefslong( DSI_JS_DEBUG, TRUE ) )
	{
		puterror( LT_JS, LL_ERROR, js_lineno, js_docurl, errortext );
	}
		//MUI_Request( app, errwin, NULL, GS( JS_ERROR_TITLE ), GS( CANCEL ), GS( JS_ERROR_PARSE ), js_docurl, js_lineno, errortext );
#if USE_JSERRORLOG
	if( getprefslong( DSI_JS_ERRORLOG, TRUE ) )
	{
		BPTR f = Open( getprefsstr( DSI_JS_LOGFILE, "PROGDIR:JSERROR.LOG" ), MODE_OLDFILE );
		if( !f )
			f = Open( getprefsstr( DSI_JS_LOGFILE, "PROGDIR:JSERROR.LOG" ), MODE_NEWFILE );
		else
			Seek( f, 0, OFFSET_END );
		if( f )
		{
			FPrintf( f, "PARSE ERROR\nVERSION: " LVERTAG "\nURL: %s\nLINE: %ld\nDESCRIPTION: %s\n\n",
				( int )js_docurl,
				( int )js_lineno,
				( int )errortext
			);								
			Close( f );
		}
	}
#endif /* USE_JSERRORLOG */
	longjmp( jsjmpbuf, 1 );
}

static void cleanup_js_alloca( void )
{
	if( js_alloca_pool )
	{
		DeletePool( js_alloca_pool );
		js_alloca_pool = 0;
	}
}
static void init_js_alloca( void )
{
	cleanup_js_alloca();
	js_alloca_pool = CreatePool( MEMF_ANY, 4096, 2048 );
}

void cleanup_alloca( void )
{
	D( db_init, bug( "cleaning up..\n" ) );
	cleanup_js_alloca();

	if( js_snoop_win )
	{
		Close( js_snoop_win );
	}
}

static int cntlist( struct MinList *l )
{
	struct MinNode *n;
	int cnt;

	for( cnt = 0, n = FIRSTNODE( l ); NEXTNODE( n ); n = NEXTNODE( n ), cnt++ );

	return( cnt );
}

/*
 * This is actually a SAS/C bug workaround,
 * because its libm fmod() spits out bullshit.
 */
#ifdef __SASC
static double myfmod( double q1, double q2 )
{
	double q = q1 / q2;
	return( q1 - ( floor(q)*q2 ) );
}
#define fmod myfmod
#endif /* __SASC */

// opcode lists

#define JSOL_INC 512
struct jsop_list *cjsol;

// expression stack

struct expr {
	struct MinNode n;
	int dsize, allocsize;
	int type;
	union {
		int boolval;
		double realval;
		struct {
			LONG funcptr;
			APTR fobj;
			APTR win;
		} fv;
		char txtval[ 12 ];
		APTR object;
	} d;
};


enum {
	marktype_while = 1,
	marktype_for,
	marktype_funcret,
	marktype_switch,
	marktype_do,
	marktype_forin
};

struct forin_ctx {
	char varname[ 256 ];
	struct MinList l;
	APTR pool;
	int ix;
};

struct mark {
	struct MinNode n;
	int type;
	ULONG offset;
	struct forin_ctx *fctx;
};

static void markstack_push( struct expr_stack *es, int type, UWORD *ptr )
{
	struct mark *mark = AllocPooled( es->pool, sizeof( *mark ) );
	if( !mark )
		longjmp( *es->rb, 1 );
	mark->type = type;
	mark->offset = ptr - cjsol->i;
	mark->fctx = 0;
	ADDHEAD( &es->marklist, mark );
}

static UWORD *markstack_get( struct expr_stack *es, int type )
{
	struct mark *mark = FIRSTNODE( &es->marklist );

	if( !NEXTNODE( mark ) || mark->type != type )
		longjmp( *es->rb, 10 );
	
	return( cjsol->i + mark->offset );
}

static int markstack_peek( struct expr_stack *es )
{
	struct mark *mark = FIRSTNODE( &es->marklist );

	if( NEXTNODE( mark ) )
		return( mark->type );

	return( -1 );
}

static void markstack_pop( struct expr_stack *es )
{
	struct mark *mark = REMHEAD( &es->marklist );
	if( !mark )
		longjmp( *es->rb, 10 );
	if( mark->fctx )
		DeletePool( mark->fctx->pool );
	FreePooled( es->pool, mark, sizeof( *mark ) );
}

// ** Context stack **

struct contextnode {
	struct MinNode n;
	int type;
	APTR obj;
};

static void contextstack_push( struct expr_stack *es, APTR obj, int what )
{
	struct contextnode *cn = AllocPooled( es->pool, sizeof( *cn ) );

	D( db_js, bug( "push ocontext %lx (%ld)\n", obj, what ) );

	if( !cn )
		longjmp( *es->rb, 1 );

	cn->type = what;

	switch( what )
	{
		case 0:
			cn->obj = es->default_object;
			es->default_object = obj;
			break;
		case 1:
			cn->obj = es->thisobject;
			es->thisobject = obj;
			break;
	}

	ADDHEAD( &es->contextlist, cn );
}

static void contextstack_pop( struct expr_stack *es )
{
	struct contextnode *cn = REMHEAD( &es->contextlist );

	if( !cn )
		longjmp( *es->rb, 10 );

	D( db_js, bug( "popping ocontext type %ld (%lx)\n", cn->type, cn ) );

	switch( cn->type )
	{
		case 0:
			es->default_object = cn->obj;
			break;

		case 1:
			es->thisobject = cn->obj;
			break;
	}

	FreePooled( es->pool, cn, sizeof( *cn ) );
}

// ** Expression stack **

static struct expr_stack *exprs_create( void )
{
	APTR pool = CreatePool( MEMF_CLEAR, 4096, 2048 );
	struct expr_stack *es;

	if( !pool )
		return( 0 );

	es = AllocPooled( pool, sizeof( *es ) );

	if( !es )
		return( 0 );

	es->pool = pool;
	NEWLIST( &es->l );
	es->rb = &es->myrb;

	NEWLIST( &es->vl );
	NEWLIST( &es->marklist );
	NEWLIST( &es->contextlist );

	es->varnameptr = -1;

	return( es );
}

static void exprs_del( struct expr_stack *es )
{
	if( es )
		DeletePool( es->pool );
}

static void exprs_push_funcptr( struct expr_stack *es, LONG funcptr, APTR funcobj, APTR win )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) );
	if( !ex )
		longjmp( *es->rb, 1 );
	ex->type = expt_funcptr;
	ex->d.fv.funcptr = funcptr;
	ex->d.fv.fobj = funcobj;
	ex->d.fv.win = win;
	ex->allocsize = sizeof( *ex );
	ADDHEAD( &es->l, ex );
}

static void exprs_push_undefined( struct expr_stack *es )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) );
	if( !ex )
		longjmp( *es->rb, 1 );
	ex->type = expt_undefined;
	ex->allocsize = sizeof( *ex );
	ADDHEAD( &es->l, ex );
}

static void exprs_push_bool( struct expr_stack *es, int val )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) );
	if( !ex )
		longjmp( *es->rb, 1 );
	ex->type = expt_bool;
	ex->d.boolval = val ? TRUE : FALSE;
	ex->allocsize = sizeof( *ex );
	ADDHEAD( &es->l, ex );
}

void exprs_push_argmarker( struct expr_stack *es )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) );
	if( !ex )
		longjmp( *es->rb, 1 );
	ex->type = expt_argmarker;
	ex->allocsize = sizeof( *ex );

	ADDHEAD( &es->l, ex );
}

#if VDEBUG == 1
static void exprs_dump( struct expr_stack *es )
{
	struct expr *ex;
	int cnt = 0;

	PutStr( "Expression stack dump:\n" );

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ); ex = NEXTNODE( ex ) )
	{
		Printf( "%ld: type %ld ", ( int )cnt++, ( int )ex->type );
		switch( ex->type )
		{
			case expt_string:
				WriteChars( ex->d.txtval, ex->dsize );
				break;

			case expt_real:
				{
					char buffer[ 64 ];
					sprintf( buffer, "%.16g", ex->d.realval );
					PutStr( buffer );
				}
				break;

			case expt_obj:
				Printf( "%lx\n", ( LONG )ex->d.object );
				break;

		}
		PutStr( "\n" );
	}

	PutStr( "---\n" );
}
#endif

static void exprs_swaptop( struct expr_stack *es )
{
	struct expr *ex, *ex2;

	ex = REMHEAD( &es->l );
	ex2 = REMHEAD( &es->l );

	if( !ex || !ex2 )
		longjmp( *es->rb, 2 ); // expression stack empty

	ADDHEAD( &es->l, ex );
	ADDHEAD( &es->l, ex2 );
}

static void exprs_reverse_till_mark( struct expr_stack *es, int functoo )
{
	struct expr *ex, *ex2 = NULL; // Avoid crappy compiler warning
	struct MinList templ;

	NEWLIST( &templ );

	while( ex = REMHEAD( &es->l ) )
	{
		if( ex->type == expt_argmarker )
			break;
		ADDTAIL( &templ, ex );
	}

	// Major screwup
	if( !ex )
		longjmp( *es->rb, 666 );

	// Get the actual function pointer
	if( functoo )
		ex2 = REMHEAD( &es->l );

	ADDHEAD( &es->l, ex );

	while( ex = REMHEAD( &templ ) )
		ADDHEAD( &es->l, ex );

	if( functoo )
	{
		// Attach function pointer to top of stack
		if( !ex2 )
			longjmp( *es->rb, 666 );

		ADDHEAD( &es->l, ex2 );
	}
}

static void exprs_push_real( struct expr_stack *es, double val )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) );
	if( !ex )
		longjmp( *es->rb, 1 );
	ex->type = expt_real;
	ex->d.realval = val;
	ex->allocsize = sizeof( *ex );
	ADDHEAD( &es->l, ex );
}

static void exprs_push_int( struct expr_stack *es, int intval )
{
	exprs_push_real( es, (double)intval );
}

static void exprs_push_str( struct expr_stack *es, char *txt, int txtlen )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) + ( txtlen - 11 ) );
	if( !ex )
		longjmp( *es->rb, 1 );
	ex->type = expt_string;
	memcpy( ex->d.txtval, txt, txtlen );
	ex->d.txtval[ txtlen ] = 0;
	ex->dsize = txtlen;
	ex->allocsize = sizeof( *ex ) + ( txtlen - 11 );
	ADDHEAD( &es->l, ex );
}

static void exprs_push_obj( struct expr_stack *es, APTR obj )
{
	struct expr *ex = AllocPooled( es->pool, sizeof( *ex ) );
	if( !ex )
		longjmp( *es->rb, 1 );

	if( obj )
	{
		int undef = FALSE;

		get( obj, MA_JS_IsUndefined, &undef );
		if( undef )
		{
			exprs_push_undefined( es );
			return;
		}

		if( OCLASS( obj ) == getjs_objref() )
		{
			obj = (APTR)getv( obj, MA_JS_ObjRef_Object );
		}
	}

	ex->type = expt_obj;
	ex->d.object = obj;
	ex->allocsize = sizeof( *ex );
	ADDHEAD( &es->l, ex );
}

static void freeex( struct expr_stack *es, struct expr *ex )
{
	FreePooled( es->pool, ex, ex->allocsize );
}

double exprs_pop_as_real( struct expr_stack *es )
{
	struct expr *ex = REMHEAD( &es->l );
	double v;
	if( !ex )
		longjmp( *es->rb, 2 ); // expression stack empty
	switch( ex->type )
	{
		case expt_real:
			v = ex->d.realval;
			break;

		case expt_bool:
			v = ex->d.boolval;
			break;

		case expt_string:
			{
				char *endp;

				v = strtod( ex->d.txtval, &endp );
				if( *endp )
				{
					v = *double_nan;
				}
			}
			break;

		case expt_obj:
			if( ex->d.object )
			{
				v = *double_nan;
				DoMethod( ex->d.object, MM_JS_ToReal, ( ULONG )&v );
			}
			else
			{
				v = 0;
			}
			break;

		case expt_undefined:
			v = *double_nan;
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	freeex( es, ex );
	return( v );
}

int exprs_pop_as_int( struct expr_stack *es )
{
	return( (int)exprs_pop_as_real( es ) );
}

double exprs_peek_as_real( struct expr_stack *es, int which )
{
	struct expr *ex;
	double v;

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ) && which--; ex = NEXTNODE( ex ) );
	if( !NEXTNODE( ex ) )
		longjmp( *es->rb, 2 ); // expression stack empty

	switch( ex->type )
	{
		case expt_real:
			v = ex->d.realval;
			break;

		case expt_bool:
			v = ex->d.boolval;
			break;

		case expt_string:
			v = atof( ex->d.txtval );
			break;

		case expt_undefined:
			v = *double_nan;
			break;

		case expt_obj:
			if( ex->d.object )
			{
				v = *double_nan;
				DoMethod( ex->d.object, MM_JS_ToReal, ( ULONG )&v );
			}
			else
			{
				v = 0;
			}
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	return( v );
}

void exprs_pop( struct expr_stack *es )
{
	struct expr *ex = REMHEAD( &es->l );
	if( !ex )
		longjmp( *es->rb, 2 ); // expression stack empty
	freeex( es, ex );
}

LONG exprs_pop_as_funcptr( struct expr_stack *es, APTR *funcobjptr, APTR *fobjptr, APTR *winptr )
{
	struct expr *ex = REMHEAD( &es->l );
	LONG v;

	if( !ex )
		longjmp( *es->rb, 2 ); // expression stack empty

	*fobjptr = NULL;

	switch( ex->type )
	{
		case expt_funcptr:
			v = ex->d.fv.funcptr;
			*funcobjptr = ex->d.fv.fobj;
			*winptr = ex->d.fv.win;
			break;

		case expt_obj:
			if( !ex->d.object )
			{
				v = 0;
				*fobjptr = NULL;
				break;
			}
			if( get( ex->d.object, MA_JS_Func_Index, &v ) )
			{
				get( ex->d.object, MA_JS_Func_Object, funcobjptr );
				get( ex->d.object, MA_JS_Func_Window, winptr );
				*fobjptr = ex->d.object;
				break;
			}

		default:
			longjmp( *es->rb, 18 ); // unknown type
	}
	freeex( es, ex );
	return( v );
}

APTR exprs_peek_as_object( struct expr_stack *es, int which )
{
	struct expr *ex;
	APTR newobj;

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ) && which--; ex = NEXTNODE( ex ) );
	if( !NEXTNODE( ex ) )
	{
		longjmp( *es->rb, 2 ); // expression stack empty
	}

	switch( ex->type )
	{
		case expt_real:
			newobj = JSNewObject( getjs_real(),
				MA_JS_FuncContext, es->func_context,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			DoMethod( newobj, MM_JS_SetData, ( ULONG )&ex->d.realval );
			D( db_js, bug( "creating temporary real object\n" ) );
			break;

		case expt_bool:
			newobj = JSNewObject( getjs_bool(),
				MA_JS_FuncContext, es->func_context,
				TAG_DONE
			);
			if( !newobj )
			{
				longjmp( *es->rb, 1 );
			}
			DoMethod( newobj, MM_JS_SetData, ( ULONG )&ex->d.boolval );
			D( db_js, bug( "creating temporary bool object\n" ) );
			break;

		case expt_string:
			newobj = JSNewObject( getjs_string(),
				MA_JS_FuncContext, es->func_context,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			DoMethod( newobj, MM_JS_SetData, ( ULONG )ex->d.txtval, ex->dsize );
			D( db_js, bug( "creating temporary string object %lx\n", newobj ) );
			break;

		case expt_funcptr:
			newobj = JSNewObject( getjs_func(),
				MA_JS_FuncContext, es->func_context,
				MA_JS_Func_Index, ex->d.fv.funcptr,
				MA_JS_Func_Object, ex->d.fv.fobj,
				MA_JS_Func_Window, ex->d.fv.win,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			D( db_js, bug( "creating temporary function object %lx\n", newobj ) );
			break;

		case expt_obj:
			newobj = ex->d.object;
			break;

		case expt_undefined:
			newobj = NULL;
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	return( newobj );
}

static LONG exprs_peek_as_funcptr( struct expr_stack *es, int which, APTR *funcobjptr, APTR *winptr )
{
	struct expr *ex;
	LONG v;

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ) && which--; ex = NEXTNODE( ex ) );
	if( !NEXTNODE( ex ) )
		longjmp( *es->rb, 2 ); // expression stack empty

	switch( ex->type )
	{
		case expt_funcptr:
			v = ex->d.fv.funcptr;
			*funcobjptr = ex->d.fv.fobj;
			*winptr = ex->d.fv.win;
			break;

		case expt_obj:
			if( !ex->d.object )
			{
				v = 0;
				*funcobjptr = NULL;
				break;
			}
			if( get( ex->d.object, MA_JS_Func_Index, &v ) )
			{
				get( ex->d.object, MA_JS_Func_Object, funcobjptr );
				get( ex->d.object, MA_JS_Func_Window, winptr );
				break;
			}
		// Fallthrough to unknown type

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	return( v );
}

int exprs_pop_as_bool( struct expr_stack *es )
{
	struct expr *ex = REMHEAD( &es->l );
	int v;
	if( !ex )
		longjmp( *es->rb, 2 ); // expression stack empty
	switch( ex->type )
	{
		case expt_real:
			{
				char buffer[ 64 ];

				sprintf( buffer, "%.16g", ex->d.realval );
				if( buffer[ 0 ] == 'N' )
					v = FALSE;
				else if( buffer[ 0 ] == 'I' )
					v = TRUE;
				else
					v = ex->d.realval ? TRUE : FALSE;
			}
			break;

		case expt_bool:
			v = ex->d.boolval ? TRUE : FALSE;
			break;

		case expt_string:
			v = ex->d.txtval[ 0 ] ? TRUE : FALSE;
			break;

		case expt_obj:
			if( ex->d.object )
			{
				v = TRUE;
				DoMethod( ex->d.object, MM_JS_ToBool, ( ULONG )&v );
			}
			else
			{
				v = FALSE;
			}
			break;

		case expt_undefined:
			v = FALSE;
			break;

		case expt_funcptr:
			v = TRUE;
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	freeex( es, ex );
	return( v );
}

static int exprs_peek_as_bool( struct expr_stack *es, int which )
{
	struct expr *ex;
	int v;

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ) && which--; ex = NEXTNODE( ex ) );
	if( !NEXTNODE( ex ) )
		longjmp( *es->rb, 2 ); // expression stack empty

	switch( ex->type )
	{
		case expt_real:
			{
				char buffer[ 64 ];

				sprintf( buffer, "%.16g", ex->d.realval );
				if( buffer[ 0 ] == 'N' )
					v = FALSE;
				else if( buffer[ 0 ] == 'I' )
					v = TRUE;
				else
					v = ex->d.realval ? TRUE : FALSE;
			}
			break;

		case expt_bool:
			v = ex->d.boolval ? TRUE : FALSE;
			break;

		case expt_string:
			v = ex->d.txtval[ 0 ] ? TRUE : FALSE;
			break;

		case expt_obj:
			v = TRUE;
			D( db_js, bug( "calling tobool %lx\n", ex->d.object ) );
			if( ex->d.object )
			{
				v = TRUE;
				DoMethod( ex->d.object, MM_JS_ToBool, ( ULONG )&v );
			}
			else
			{
				v = FALSE;
			}
			break;

		case expt_undefined:
			v = FALSE;
			break;

		case expt_funcptr:
			v = TRUE;
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	return( v );
}

void exprs_pop_as_string( struct expr_stack *es, char *to, int maxsize )
{
	struct expr *ex = REMHEAD( &es->l );
	if( !ex )
		longjmp( *es->rb, 2 ); // expression stack empty
	D( db_js, bug( "popasstring type %ld (do=%lx)\n", ex->type, es->default_object ) );
	
	switch( ex->type )
	{
		case expt_real:
			{
				char buffer[ 128 ];
				sprintf( buffer, "%.16g", ex->d.realval );
				process_nan( buffer );
				strcpy( to, buffer );
			}
			break;

		case expt_bool:
			strcpy( to, ex->d.boolval ? "true" : "false" );
			break;

		case expt_undefined:
			strcpy( to, "undefined" );
			break;

		case expt_string:
			if( maxsize )
			{
				stccpy( to, ex->d.txtval, maxsize );
			}
			else
			{
				memcpy( to, ex->d.txtval, ex->dsize );
			}
			break;

		case expt_funcptr:
			{
				char buffer[ 128 ];

				strcpy( buffer, "[Function]" );

				if( maxsize )
				{
					stccpy( to, buffer, maxsize );
				}
				else
				{
					strcpy( to, buffer );
				}
			}
			break;

		case expt_obj:
			if( !ex->d.object )
			{
				strcpy( to, "null" );
				break;
			}

			if( !DoMethod( ex->d.object, MM_JS_ToString, ( ULONG )to, ( ULONG )&maxsize, 0 ) )
			{
				char buffer[ 128 ];

				sprintf( buffer, "[object %s]", (char*)getv( ex->d.object, MA_JS_ClassName ) );

				if( maxsize )
				{
					stccpy( to, buffer, maxsize );
				}
				else
				{
					strcpy( to, buffer );
				}

				D( db_js, bug( "no ToString method -> %s\n", buffer ) );
			}
			else
			{
#ifdef DEBUG
				char buffer[ 256 ];
				stccpy( buffer, to, sizeof( buffer ) );
				D( db_js, bug( "got %ld, %s\n", maxsize, buffer ) );
#endif
			}
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	freeex( es, ex );
}

void exprs_pop_as_object( struct expr_stack *es, APTR *object )
{
	APTR newobj;

	struct expr *ex = REMHEAD( &es->l );
	if( !ex )
		longjmp( *es->rb, 2 ); // expression stack empty
	switch( ex->type )
	{
		case expt_real:
			newobj = JSNewObject( getjs_real(),
				MA_JS_FuncContext, es->func_context,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			DoMethod( newobj, MM_JS_SetData, ( ULONG )&ex->d.realval );
			D( db_js, bug( "creating temporary real object\n" ) );
			*object = newobj;
			break;

		case expt_bool:
			newobj = JSNewObject( getjs_bool(),
				MA_JS_FuncContext, es->func_context,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			DoMethod( newobj, MM_JS_SetData, ( ULONG )&ex->d.boolval );
			D( db_js, bug( "creating temporary bool object %lx\n", newobj ) );
			*object = newobj;
			break;

		case expt_string:
			newobj = JSNewObject( getjs_string(),
				MA_JS_FuncContext, es->func_context,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			DoMethod( newobj, MM_JS_SetData, ( ULONG )ex->d.txtval, ex->dsize );
			D( db_js, bug( "creating temporary string object %lx\n", newobj ) );
			*object = newobj;
			break;

		case expt_funcptr:
			newobj = JSNewObject( getjs_func(),
				MA_JS_FuncContext, es->func_context,
				MA_JS_Func_Index, ex->d.fv.funcptr,
				MA_JS_Func_Object, ex->d.fv.fobj,
				MA_JS_Func_Window, ex->d.fv.win,
				TAG_DONE
			);
			if( !newobj )
				longjmp( *es->rb, 1 );
			D( db_js, bug( "creating temporary function object %lx\n", newobj ) );
			*object = newobj;
			break;

		case expt_obj:
			*object = ex->d.object;
			break;

		case expt_undefined:
			*object = NULL;
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	freeex( es, ex );
}

char *exprs_peek_as_string( struct expr_stack *es, int which )
{
	struct expr *ex;
	static char peekbuffer[ 4 * 256 ];
	static int peekcnt;
	char *to = &peekbuffer[ ( peekcnt++ ) % 4 * 256 ];

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ) && which--; ex = NEXTNODE( ex ) );

	if( !NEXTNODE( ex ) )
		longjmp( *es->rb, 2 ); // expression stack empty
	switch( ex->type )
	{
		case expt_real:
			sprintf( to, "%.16g", ex->d.realval );
			process_nan( to );
			break;

		case expt_bool:
			strcpy( to, ex->d.boolval ? "true" : "false" );
			break;

		case expt_undefined:
			strcpy( to, "undefined" );
			break;

		case expt_funcptr:
			strcpy( to, "[Function]" );
			break;

		case expt_string:
			to = ex->d.txtval;
			break;

		case expt_obj:
			if( !ex->d.object )
			{
				strcpy( to, "null" );
				break;
			}
			else if( OCLASS( ex->d.object ) == getjs_string() )
			{
				int dummy;
				DoMethod( ex->d.object, MM_JS_GetTypeData, ( ULONG )&dummy, ( ULONG )&to, ( ULONG )&dummy );
			}
			else
			{
				int size = 256;
				DoMethod( ex->d.object, MM_JS_ToString, ( ULONG )to, ( ULONG )&size, 0 );
			}
			break;

		default:
			longjmp( *es->rb, 3 ); // unknown type
	}
	return( to );
}

int exprs_get_type( struct expr_stack *es, int which, int *strl )
{
	struct expr *ex;

	for( ex = FIRSTNODE( &es->l ); NEXTNODE( ex ) && which--; ex = NEXTNODE( ex ) );

	if( !NEXTNODE( ex ) )
	{
		longjmp( *es->rb, 2 ); // stack empty
	}

	if( strl )
	{
		char buffer[ 256 ];

		switch( ex->type )
		{
			case expt_real:
				sprintf( buffer, "%.16g", ex->d.realval );
				process_nan( buffer );
				*strl = strlen( buffer );
				break;

			case expt_bool:
				*strl = ex->d.boolval ? 4 : 5; // "true" vs. "false"
				break;

			case expt_string:
				*strl = ex->dsize;
				break;

			case expt_funcptr:
				*strl = strlen( "[Function]" );
				break;

			case expt_obj:
				// Calculate string size
				if( !ex->d.object )
				{
					*strl = 4; // "null"
					break;
				}

				*strl = sizeof( buffer );
				if( !DoMethod( ex->d.object, MM_JS_ToString, ( ULONG )buffer, ( ULONG )strl, 0 ) )
				{
					sprintf( buffer, "[object %s]", (char*)getv( ex->d.object, MA_JS_ClassName ) );
					*strl = strlen( buffer );
				}
				break;

			case expt_undefined:
				*strl = 9; // "undefined"
				break;

			default:
				longjmp( *es->rb, 3 ); // unknown type
		}
	}

	return( ex->type ); 
}

// Check whether something is "scalar", aka a valid real
static int exprs_check_scalar( struct expr_stack *es, int howmany )
{
	struct expr *ex;

	for( ex = FIRSTNODE( &es->l ); howmany--; ex = NEXTNODE( ex ) )
	{
		if( !NEXTNODE( ex ) )
			longjmp( *es->rb, 2 );

		D( db_js, bug( "check_scalar %ld\n", ex->type ) );

		if( ex->type != expt_real && ex->type != expt_bool )
		{
			// if it's a string, try to check whether it can
			// be converted to "int"/"real" seamlessly
			if( ex->type == expt_string )
			{
				char *endptr;
				strtod( ex->d.txtval, &endptr );
				if( ! *endptr )
					continue;
			}
			else if( ex->type == expt_obj )
			{
				if( ex->d.object )
				{
					char buffer[ 256 ];
					int maxsize = sizeof( buffer );

					if( OCLASS( ex->d.object ) == getjs_real() )
						continue;
					if( OCLASS( ex->d.object ) == getjs_bool() )
						continue;

					if( DoMethod( ex->d.object, MM_JS_ToString, ( ULONG )buffer, ( ULONG )&maxsize, 0 ) )
					{
						char *endptr;

						D( db_js, bug( "check scalar, stringtest '%s'\n", buffer ) );

						strtod( buffer, &endptr );
						if( ! *endptr )
							continue;
					}
				}
			}
			D( db_js, bug( "failing check scalar\n" ) );
			return( FALSE );
		}
	}
	return( TRUE );
}

static int exprs_check_null_or_undefined( struct expr_stack *es, int howmany )
{
	struct expr *ex;
	int null1 = 1, null2 = 2;

	for( ex = FIRSTNODE( &es->l ); howmany--; ex = NEXTNODE( ex ) )
	{
		if( !NEXTNODE( ex ) )
			longjmp( *es->rb, 2 );
		if( ex->type != expt_undefined && ( ex->type != expt_obj || ex->d.object ) )
		{
			if( howmany == 1 )
				null2 = FALSE;
			else if( !howmany )
				null1 = FALSE;
		}
	}

	return( null1 | null2 );
}

static int exprs_check_objects( struct expr_stack *es, int howmany )
{
	struct expr *ex;

	for( ex = FIRSTNODE( &es->l ); howmany--; ex = NEXTNODE( ex ) )
	{
		if( !NEXTNODE( ex ) )
			longjmp( *es->rb, 2 );
		if( ex->type != expt_obj )
			return( FALSE );
		if( OCLASS( ex->d.object ) == getjs_string() )
			return( FALSE );
		else if( OCLASS( ex->d.object ) == getjs_real() )
			return( FALSE );
		else if( OCLASS( ex->d.object ) == getjs_bool() )
			return( FALSE );
	}

	return( TRUE );
}

#define DNMAXNAMEBUFF 128

// Dispose off all variables in current function
// context (aka, auto variables)
static void dom_erase( struct expr_stack *es )
{
	struct _Object *o, *next;
	int cleanupctx = es->func_context ? es->func_context : 1;

	D( db_js, bug( "cleaning up func context %ld\n", es->func_context ) );

	for( o = FIRSTNODE( &cjsol->vars ); next = NEXTNODE( o ); o = next )
	{
		APTR obj = BASEOBJECT( o );
		int func;

		if( get( obj, MA_JS_FuncContext, &func ) )
		{
			if( func >= cleanupctx )
			{
				D( db_js, bug( "removing variable %lx (%s, %s) from ctx %ld\n", obj, OCLASS( obj )->cl_ID, (char*)getv( obj, MA_JS_Name ), func ) );
				REMOVE( o );
			}
		}
	}
}

static void dom_delete( struct expr_stack *es )
{
	APTR refobj = es->default_object;

	if( es->varname_ref[ es->varnameptr ] )
	{
		exprs_pop_as_object( es, &refobj );
	}

	D( db_js, bug( "DELETEing property %s on %lx\n", es->varname[ es->varnameptr ], refobj ) );

	if( refobj )
		DoMethod( refobj, MM_JS_DeleteProperty, ( ULONG )es->varname[ es->varnameptr ] );
}

static void dom_set( struct expr_stack *es, int localonly )
{
	struct _Object *o;
	APTR refobj = es->currentcontextobj;
	int found_context, found;
	int rc;
	int datasize;
	APTR newobj;
	int proptype;
	char *varname = es->varname[ es->varnameptr ];

	D( db_js, bug( "dom_set: varname %s, ref %ld (%ld), mode %ld\n", es->varname[ es->varnameptr ], es->varname_ref[ es->varnameptr ], es->varnameptr, localonly ) );

	// Sadly, JS always has multiple scopes
	// a) local variables
	// b) local object scope
	// c) global scope

	if( localonly == 2 )
	{
		refobj = es->default_object;
		proptype = expt_obj;
		goto hasprop;
	}

	if( es->varname_ref[ es->varnameptr ] )
	{
		// Expression already evaluated to the object in doubt
		exprs_swaptop( es );
		exprs_pop_as_object( es, &refobj );

		if( !refobj )
			longjmp( *es->rb, 11 );

		if( !( proptype = DoMethod( refobj, MM_JS_HasProperty, ( ULONG )varname ) ) )
			proptype = expt_obj;

hasprop:

		D( db_js, bug( "assigning to reference object %lx as type %ld\n", refobj, proptype ) );

		switch( proptype )
		{
			case expt_real:
				{
					double v = exprs_peek_as_real( es, 0 );
					rc = DoMethod( refobj, MM_JS_SetProperty, ( ULONG )varname, ( ULONG )&v, sizeof( v ), proptype );
				}
				break;

			case expt_bool:
				{
					int v = exprs_peek_as_bool( es, 0 );
					rc = DoMethod( refobj, MM_JS_SetProperty, ( ULONG )varname, ( ULONG )&v, sizeof( v ), proptype );
				}
				break;

			case expt_obj:
			case expt_undefined:
				{
					APTR v = exprs_peek_as_object( es, 0 );
					rc = DoMethod( refobj, MM_JS_SetProperty, ( ULONG )varname, ( ULONG )&v, sizeof( v ), expt_obj );
				}
				break;

			case expt_string:
				{
					STRPTR v = exprs_peek_as_string( es, 0 );
					rc = DoMethod( refobj, MM_JS_SetProperty, ( ULONG )varname, ( ULONG )v, strlen( v ), expt_string );
				}
				break;

			case expt_funcptr:
				{
					APTR dummy;
					LONG v = exprs_peek_as_funcptr( es, 0, &dummy, &dummy );
					rc = DoMethod( refobj, MM_JS_SetProperty, ( ULONG )varname, ( ULONG )&v, sizeof( v ), expt_funcptr );
				}
				break;

			default:
				longjmp( *es->rb, 13 ); // unknown type
		}

		if( !rc )
		{
			es->errorobject = refobj;
			longjmp( *es->rb, 15 ); // property can't be modified
		}

		return;
	}

	// check for existing global vars

	D( db_js, bug( "iterating cjsol->vars\n" ) );
	
	found = FALSE;
	for( o = FIRSTNODE( &cjsol->vars ); NEXTNODE( o ); o = NEXTNODE( o ) )
	{
		APTR obj = BASEOBJECT( o );

		rc = DoMethod( obj, MM_JS_NameIs, ( ULONG )varname );

		if( rc == 2 )
		{
			// Global object, cannot be modified
			continue;
		}
		else if( rc == 1 )
		{
			get( obj, MA_JS_FuncContext, &found_context );
			if( localonly || found_context )
			{
				if( es->func_context != found_context )
					continue; // different context
			}

			D( db_js, bug( "replacing existing global var %s\n", varname ) );

			// Object found, remove it so it can be replaced
			REMOVE( o );

			found = TRUE;
			break;
		}
	}

	D( db_js, bug( "found %ld, refobj %lx, localonly %ld\n", found, refobj, localonly ) );

	if( !found && es->default_object && !localonly )
	{
		D( db_js, bug( "es->default_object = %lx\n", es->default_object ) );

		if( proptype = DoMethod( es->default_object, MM_JS_HasProperty, ( ULONG )varname ) )
		{
			refobj = es->default_object;
			goto hasprop;
		}
	}

	if( !found && es->thisobject && !localonly )
	{
		D( db_js, bug( "es->thisobject = %lx\n", es->thisobject ) );

		if( proptype = DoMethod( es->thisobject, MM_JS_HasProperty, ( ULONG )varname ) )
		{
			refobj = es->thisobject;
			goto hasprop;
		}
	}

	if( !found && refobj && !localonly )
	{
		// Still not found
		if( proptype = DoMethod( refobj, MM_JS_HasProperty, ( ULONG )varname ) )
		{
			// Ok, a property does exist
			D( db_js, bug( "property of type %ld exists!\n", proptype ) );
			goto hasprop;
		}
	}

	// Right, create a DOM object

	if( !found )
		found_context = ( localonly ? es->func_context : 0 );

	rc = exprs_get_type( es, 0, &datasize );

	D( db_js, bug( "creating new object type %ld, datasize %ld\n", rc, datasize ) );

	switch( rc )
	{
		case expt_real:
			{
				double v = exprs_peek_as_real( es, 0 );
				newobj = JSNewObject( getjs_real(),
					MA_JS_Name, varname,
					MA_JS_FuncContext, found_context,
					TAG_DONE
				);
				D( db_js, bug( "creating new REAL object %lx\n", newobj ) );
				if( !newobj )
					longjmp( *es->rb, 1 );
				DoMethod( newobj, MM_JS_SetData, ( ULONG )&v );
				ADDHEAD( &cjsol->vars, _OBJECT( newobj ) );
			}
			break;

		case expt_funcptr:
			{
				APTR o, w;
				LONG v = exprs_peek_as_funcptr( es, 0, &o, &w );
				newobj = JSNewObject( getjs_func(),
					MA_JS_Name, varname,
					MA_JS_FuncContext, found_context,
					MA_JS_Func_Index, v,
					MA_JS_Func_Object, o,
					MA_JS_Func_Window, w,
					TAG_DONE
				);
				D( db_js, bug( "creating new FUNC object %lx\n", newobj ) );
				if( !newobj )
					longjmp( *es->rb, 1 );
				ADDHEAD( &cjsol->vars, _OBJECT( newobj ) );
			}
			break;

		case expt_obj:
			{
				APTR v = exprs_peek_as_object( es, 0 );
				D( db_js, bug( "creating OBJREF to %lx\n", v ) );
				newobj = JSNewObject( getjs_objref(),
					MA_JS_Name, varname,
					MA_JS_FuncContext, found_context,
					MA_JS_ObjRef_Object, v,
					TAG_DONE
				);
				D( db_js, bug( "creating new OBJREF object %lx\n", newobj ) );
				if( !newobj )
					longjmp( *es->rb, 1 );
				ADDHEAD( &cjsol->vars, _OBJECT( newobj ) );
			}
			break;

		case expt_bool:
			{
				int v = exprs_peek_as_bool( es, 0 );
				newobj = JSNewObject( getjs_bool(),
					MA_JS_Name, varname,
					MA_JS_FuncContext, found_context,
					TAG_DONE
				);
				D( db_js, bug( "creating new BOOL object %lx\n", newobj ) );
				if( !newobj )
					longjmp( *es->rb, 1 );
				DoMethod( newobj, MM_JS_SetData, ( ULONG )&v );
				ADDHEAD( &cjsol->vars, _OBJECT( newobj ) );
			}
			break;

		case expt_string:
			{
				char *v = exprs_peek_as_string( es, 0 );
				newobj = JSNewObject( getjs_string(),
					MA_JS_Name, varname,
					MA_JS_FuncContext, found_context,
					TAG_DONE
				);
				D( db_js, bug( "creating new STRING object %lx\n", newobj ) );
				if( !newobj )
					longjmp( *es->rb, 1 );
				DoMethod( newobj, MM_JS_SetData, ( ULONG )v, strlen( v ) );
				ADDHEAD( &cjsol->vars, _OBJECT( newobj ) );
			}
			break;

		case expt_undefined:
			{
				D( db_js, bug( "creating UNDEFINED object\n" ) );
				newobj = JSNewObject( getjs_object(),
					MA_JS_Name, varname,
					MA_JS_FuncContext, found_context,
					MA_JS_IsUndefined, TRUE,
					TAG_DONE
				);
				D( db_js, bug( "creating new UNDEFINED object %lx\n", newobj ) );
				if( !newobj )
					longjmp( *es->rb, 1 );
				ADDHEAD( &cjsol->vars, _OBJECT( newobj ) );
			}
			break;

		default:
			D( db_js, bug( "unknown type!\n" ) );
			longjmp( *es->rb, 3 ); // unknown type
	}
}

static void dom_eval( struct expr_stack *es, int failonmiss )
{
	struct _Object *o;
	APTR refobj = es->currentcontextobj;
	int found_context;
	int rc;
	APTR fobj = NULL;
	int max_context = -1;

	D( db_js, bug( "dom_eval: varname %s, ref %ld (%ld)\n", es->varname[ es->varnameptr ], es->varname_ref[ es->varnameptr ], es->varnameptr ) );
	//exprs_dump( es );

	if( es->varname_ref[ es->varnameptr ] )
	{
		int type, datasize;
		APTR dataptr;

		// Expression already evaluated to the object in doubt
		exprs_pop_as_object( es, &refobj );

hasprop:
		D( db_js, bug( "Evaluating property %s on %lx (%s)\n", es->varname[ es->varnameptr ], refobj, OCLASS( refobj )->cl_ID ) );

		if( !refobj || !DoMethod( refobj, MM_JS_GetProperty, ( ULONG )es->varname[ es->varnameptr ], ( ULONG )&type, ( ULONG )&dataptr, ( ULONG )&datasize ) )
		{
			//js_snoop( "Property %s of object of class %s is undefined", es->varname[ es->varnameptr ], getv( refobj, MA_JS_ClassName ) );
			if( !strcmp( es->varname[ es->varnameptr ], "toString" ) )
			{
				D( db_js, bug( "property is special toString method\n" ) );
				exprs_push_funcptr( es, -1, refobj, es->window );
			}
			else
			{
				D( db_js, bug( "property is undefined\n" ) );
				exprs_push_undefined( es );
			}
			return;
		}

		D( db_js, bug( "property is type %ld, pushing %lx...\n", type, dataptr ) );

		switch( type )
		{
			case expt_real:
				exprs_push_real( es, TO_DOUBLE( dataptr ) );
				break;

			case expt_bool:
				exprs_push_bool( es, (int)dataptr );
				break;

			case expt_string:
				exprs_push_str( es, dataptr, datasize );
				break;

			case expt_obj:
				D( db_js, bug( "pushing object %lx (%s)...\n", dataptr, dataptr ? ( STRPTR )OCLASS( dataptr )->cl_ID : ( STRPTR )"null" ) );
				exprs_push_obj( es, dataptr );
				break;

			case expt_funcptr:
				exprs_push_funcptr( es, (LONG)dataptr, refobj, es->window );
				break;

			default:
				longjmp( *es->rb, 13 );
		}

		return;
	}

	// check for existing global vars

	if( !strcmp( es->varname[ es->varnameptr ], "this" ) )
	{
		D( db_js, bug( "pushing this\n" ) );
		exprs_push_obj( es, es->thisobject );
		return;
	}

	// Check for a property of the current reference object
	if( es->default_object )
	{
		if( DoMethod( es->default_object, MM_JS_HasProperty, ( ULONG )es->varname[ es->varnameptr ] ) )
		{
			D( db_js, bug( "find in default object\n" ) );
			refobj = es->default_object;
			goto hasprop;
		}
	}

	// Check for an actual variable
	// Find the one with a matching context number
	for( o = FIRSTNODE( &cjsol->vars ); NEXTNODE( o ); o = NEXTNODE( o ) )
	{
		APTR obj = BASEOBJECT( o );

		rc = DoMethod( obj, MM_JS_NameIs, ( ULONG )es->varname[ es->varnameptr ] );
		if( rc )
		{
			found_context = 0;
			get( obj, MA_JS_FuncContext, &found_context );
			if( !found_context || found_context == es->func_context )
			{
				if( found_context > max_context )
				{
					max_context = found_context;
					fobj = obj;
				}
			}
		}
	}

	if( fobj )
	{
		D( db_js, bug( "found actual variable context %ld, pushing\n", found_context ) );
		exprs_push_obj( es, fobj );
		return;
	}

	// Check for a property of the current this object
	if( es->thisobject )
	{
		if( DoMethod( es->thisobject, MM_JS_HasProperty, ( ULONG )es->varname[ es->varnameptr ] ) )
		{
			D( db_js, bug( "find in this object\n" ) );
			refobj = es->thisobject;
			goto hasprop;
		}
	}

	// Check for a property of the current reference object
	if( DoMethod( refobj, MM_JS_HasProperty, ( ULONG )es->varname[ es->varnameptr ] ) )
	{
		D( db_js, bug( "find in refobj\n" ) );
		goto hasprop;
	}

	if( DoMethod( es->window, MM_JS_HasProperty, ( ULONG )es->varname[ es->varnameptr ] ) )
	{
		D( db_js, bug( "find in window\n" ) );
		refobj = es->window;
		goto hasprop;
	}

	// Check for the global "RegExp" object
	// (evil hack, evil)
	if( !strcmp( es->varname[ es->varnameptr ], "RegExp" ) )
	{
		extern APTR last_regexp_object;
		exprs_push_obj( es, last_regexp_object );
		return;
	}

	// Not found, can't eval
	if( !failonmiss )
		exprs_push_undefined( es );
	else
		longjmp( *es->rb, 11 );
}

struct jsop_list *jso_init( void )
{
	APTR pool;
	APTR newobj;

	pool = CreatePool( MEMF_ANY, 4096, 2048 );
	if( !pool )
		return( 0 );

	cjsol = AllocPooled( pool, sizeof( *cjsol ) );
	cjsol->pool = pool;
	cjsol->i = AllocPooled( pool, JSOL_INC * 2 );
	cjsol->instsize = 1;
	cjsol->installoc = JSOL_INC;

	cjsol->es = NULL;

	NEWLIST( &cjsol->vars );
	NEWLIST( &cjsol->eval_cache );

	// Create default objects

	// Navigator
	newobj = JSNewObject( getjs_navigator(),
		MA_JS_Name, "navigator",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );
	cjsol->go_navigator = newobj;

	// Event
	newobj = JSNewObject( getjs_event(),
		MA_JS_Name, "Event",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );
	cjsol->go_event = newobj;

	newobj = JSNewObject( getjs_math(),
		MA_JS_Name, "Math",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );

	newobj = JSNewObject( getjs_string(),
		MA_JS_Name, "String",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );

#ifdef MBX
	newobj = JSNewObject( getjs_stb_root(),
		MA_JS_Name, "STB",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );
#endif

	newobj = JSNewObject( getjs_screen(),
		MA_JS_Name, "screen",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );

	newobj = JSNewObject( getjs_date(),
		MA_JS_Name, "Date",
		TAG_DONE
	);
	ADDTAIL( &cjsol->vars, _OBJECT( newobj ) );

	return( cjsol );
}

void jso_cleanup( struct jsop_list *jsol )
{
	if( jsol )
	{
		if( jsol->es )
			exprs_del( jsol->es );
		DeletePool( jsol->pool );
	}
}

void jso_gc( struct jsop_list *jsol, ULONG magic )
{
	APTR o;
	struct expr *exp;

	for( o = FIRSTNODE( &jsol->vars ); NEXTNODE( o ); o = NEXTNODE( o ) )
	{
		APTR obj = BASEOBJECT( o );
		DoMethod( obj, MM_JS_SetGCMagic, magic );
	}

	if( jsol->es )
	{
		for( exp = FIRSTNODE( &jsol->es->l ); NEXTNODE( exp ); exp = NEXTNODE( exp ) )
		{
			if( exp->type == expt_obj )
			{
				if( exp->d.object )
					DoMethod( exp->d.object, MM_JS_SetGCMagic, magic );
			}
		}
	}
}

void jso_storeop( int opcode, APTR data, int datasize )
{
	int rdsize = ( datasize + 1 ) / 2;

#if (VDEBUG >= 1)
	DL( DEBUG_CHATTY, db_js, bug( "storeop: %s, data: %s, ds %ld, line %ld, size %ld/%ld\n", opcode>=8192?dbjsdops[opcode-8192]:dbjsops[opcode], data, datasize, js_lineno, cjsol->instsize, cjsol->installoc ) );
#endif

	if( lastlineno != js_lineno )
	{
		struct jslineinfo jli;

		jli.lineno = js_lineno;
		strcpy( jli.url, js_docurl );

		lastlineno = js_lineno;

		jso_storeop( JSOP_LINENO, &jli, 2 + strlen( js_docurl ) + 1 );
	}

	while( 6 + rdsize + cjsol->instsize >= cjsol->installoc )
	{
		UWORD *nb;
		// increment
		nb = AllocPooled( cjsol->pool, ( cjsol->installoc + JSOL_INC ) * 2 );
		if( !nb )
			yyerror( "out of bytecode memory" );
		memcpy( nb, cjsol->i, cjsol->instsize * 2 );
		FreePooled( cjsol->pool, cjsol->i, cjsol->installoc * 2 );
		cjsol->i = nb;
		cjsol->installoc += JSOL_INC;
	}
	cjsol->i[ cjsol->instsize++ ] = opcode;
	if( opcode & JSOP_HASDATA )
	{
		cjsol->i[ cjsol->instsize++ ] = datasize;
		if( datasize )
			memcpy( &cjsol->i[ cjsol->instsize ], data, datasize );
		cjsol->instsize += rdsize;
	}

}

#define SKIPOP if( *execindex++ & JSOP_HASDATA ) { execindex += ( execindex[ 0 ] + 1 ) / 2 + 1; }
#define SKIPTOP if( *tempindex++ & JSOP_HASDATA ) { tempindex += ( tempindex[ 0 ] + 1 ) / 2 + 1; }

/* WARNING! refwin == htmlview, refobj == htmlwin. No comment */
static void js_bytecode( 
	APTR refwin, 
	APTR refobj, 
	APTR thisobject, 
	STRPTR baseref, 
	struct jsop_list *jsol, 
	UWORD *execindex, 
	UWORD *maxexec, 
	struct expr_stack *es,
	int *execlinecnt,
	STRPTR docurl,
	char **execurl
)
{
	time_t starttime = timed(), now;
	int instcounter = 0;
	int opcode;
	UWORD *tempindex;
	UWORD *lastseendefault = 0;
	char buffer[ 512 ];
	int last_new_was_constructor = FALSE;

	// Scan bytecode for function definitions
	for( tempindex = execindex; tempindex < maxexec; )
	{
		switch( *tempindex )
		{
			case JSOP_FUNC_BEGIN:
				if( es->varnameptr == MAXVARNAMES )
					longjmp( es->myrb, 16 );
				stccpy( es->varname[ ++es->varnameptr ], (char*)&tempindex[ 2 ], tempindex[ 1 ] + 1 );
				// Store current reference varname
				tempindex += 2 + ( ( tempindex[ 1 ] + 1 ) / 2 );
				es->varname_ref[ es->varnameptr ] = FALSE;
				exprs_push_funcptr( es, tempindex - jsol->i, refobj, es->window );

				dom_set( es, TRUE );
				es->varnameptr--;
				exprs_pop( es );

				//while( tempindex < maxexec && *tempindex != JSOP_FUNC_END )
				//{
				//	SKIPTOP;
				//}
				break;

			case JSOP_END:
				tempindex= maxexec;
				break;
		}
		SKIPTOP;
	}

	while( execindex < maxexec )
	{
		if( ++instcounter > 64 )
		{
			now = timed();
			if( now - starttime > 30 )
			{
				//if( MUI_Request( app, refwin, 0, GS( JS_ERROR_TITLE ), GS( JS_TOO_LONG_BUT ), GS( JS_TOO_LONG ), 0 ) )
				//	break;
				starttime = timed();
			}
			instcounter = 0;
		}

//#define JS_DEBUG TOFIX!!
#ifdef JS_DEBUG
		D( db_js, bug( "opcode %ld", *execindex ) );
		if( *execindex & JSOP_HASDATA )
		{
			int c;
			D( db_js, bug( " datasize %ld:", execindex[ 1 ] ) );
			for( c = 0; c < ( execindex[ 1 ] + 1 ) / 2; c++ )
				D( db_js, bug( " %04lx", execindex[ c + 2 ] ) );
		}
		PutStr( "\n" );
		//Delay( 50 );
#endif

		switch( opcode = *execindex++ )
		{
			case JSOP_END:
				execindex = maxexec;
				break;

			case JSOP_EVAL_LVALUE:
				dom_eval( es, FALSE );
				es->varnameptr--;
				break;

			case JSOP_EVAL_LVALUE_CHECK:
				dom_eval( es, TRUE );
				if( exprs_get_type( es, 0, NULL ) == expt_undefined )
				{
					longjmp( es->myrb, 17 );
				}
				es->varnameptr--;
				break;

			case JSOP_EVAL_LVALUE_AND_KEEP:
				dom_eval( es, TRUE );
				break;

			case JSOP_FOR_PRECOND:
				markstack_push( es, marktype_for, execindex );
				break;

			case JSOP_FOR_COND:
				{
					int v = exprs_pop_as_bool( es );

					D( db_js, bug( "for cond = %ld\n", v ) );

					if( !v ) 
					{
						int nestcnt = 1;
						markstack_pop( es );
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_FOR_COND )
								nestcnt++;
							else if( *execindex == JSOP_FOR_END )
								nestcnt--;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
					}
					else
					{
						int nestcnt = 1;
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_FOR_COND )
								nestcnt++;
							else if( *execindex == JSOP_FOR_BODY )
								nestcnt--;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
					}
				}
				break;

			case JSOP_FOR_BODY:
				// Back to beginning of loop
				// This OP is only reached when
				// the increment operation was been hit
				execindex = markstack_get( es, marktype_for );
				D( db_js, bug( "in for body, back to @%ld\n", execindex ) );
				break;

			case JSOP_FOR_END:
				{
					int nestcnt = 1;

					execindex = markstack_get( es, marktype_for );

					D( db_js, bug( "for end\n" ) );

					while( nestcnt && execindex < maxexec )
					{
						if( *execindex == JSOP_FOR_PRECOND )
							nestcnt++;
						else if( *execindex == JSOP_FOR_COND )
							nestcnt--;
						SKIPOP;
					}
					// not found, throw up
					if( nestcnt )
						longjmp( es->myrb, 6 );
					
				}
				break;

			case JSOP_EX_CONTINUE:
			case JSOP_EX_BREAK:
				{
					int type = markstack_peek( es );
					int nestcnt = 1;
					int isbreak = ( opcode == JSOP_EX_BREAK );

					if( type == marktype_while )
					{
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_EX_WHILEBEGIN )
								nestcnt++;
							else if( *execindex == JSOP_EX_WHILEEND )
								nestcnt--;
							if( nestcnt )
								SKIPOP;
						}
					}
					else if( type == marktype_do )
					{
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_EX_DOBEGIN )
								nestcnt++;
							else if( *execindex == JSOP_EX_DOPRECOND )
								nestcnt--;
							if( nestcnt )
								SKIPOP;
						}
						if( !nestcnt && isbreak )
						{
							while( execindex < maxexec && *execindex != JSOP_EX_DOEND )
							{
								SKIPOP;
							}
						}
					}
					else if( type == marktype_for )
					{
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_FOR_PRECOND )
								nestcnt++;
							else if( *execindex == JSOP_FOR_END )
								nestcnt--;
							if( nestcnt )
								SKIPOP;
						}
					}
					else if( type == marktype_forin )
					{
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_FORIN_SET )
								nestcnt++;
							else if( *execindex == JSOP_FORIN_END )
								nestcnt--;
							if( nestcnt )
								SKIPOP;
						}
					}
					else if( type == marktype_switch )
					{
						isbreak = FALSE;
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_SWITCH_START )
								nestcnt++;
							else if( *execindex == JSOP_SWITCH_END )
								nestcnt--;
							if( nestcnt )
								SKIPOP;
						}
					}

					if( nestcnt )
						longjmp( es->myrb, 6 );

					if( isbreak )
					{
						markstack_pop( es );
						SKIPOP;
					}
				}
				break;

			case JSOP_EX_WHILEBEGIN:
				markstack_push( es, marktype_while, execindex );
				break;

			case JSOP_EX_WHILE:
				{
					int v = exprs_pop_as_bool( es );

					if( !v ) 
					{
						int nestcnt = 1;
						markstack_pop( es );
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_EX_WHILE )
								nestcnt++;
							else if( *execindex == JSOP_EX_WHILEEND )
								nestcnt--;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
					}
				}
				break;

			case JSOP_EX_WHILEEND:
				execindex = markstack_get( es, marktype_while );
				break;

			case JSOP_EX_DOBEGIN:
				markstack_push( es, marktype_do, execindex);
				break;

			case JSOP_EX_DOEND:
				{
					int v = exprs_pop_as_bool( es );

					if( v ) 
					{
						execindex = markstack_get( es, marktype_do );
					}
					else
					{
						markstack_pop( es );
					}
				}
				break;

			case JSOP_EX_DOPRECOND:
				// This is just a marker for break/continue
				break;

			case JSOP_EX_IF:
				{
					int v = exprs_pop_as_bool( es );

					D( db_js, bug( "if value = %ld\n", v ) );

					if( !v )
					{
						int nestcnt = 1;
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_EX_IF )
								nestcnt++;
							else if( *execindex == JSOP_EX_ENDIF )
								nestcnt--;
							else if( *execindex == JSOP_EX_ELSE && nestcnt == 1 )
								nestcnt = 0;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
						// seems we found the end of block
					}
				}
				break;

			case JSOP_EX_ELSE:
				// if we come here, we executed a if( true ) block
				// find the according ENDIF
				{
					int nestcnt = 1;
					while( nestcnt && execindex < maxexec )
					{
						if( *execindex == JSOP_EX_IF )
							nestcnt++;
						else if( *execindex == JSOP_EX_ENDIF )
							nestcnt--;
						SKIPOP;
					}
					// not found, throw up
					if( nestcnt )
						longjmp( es->myrb, 6 );
				}
				break;


			case JSOP_EX_ENDIF:
				// those are merely markers
				break;

			case JSOP_ISNAN:
				{
					double v;
					v = exprs_pop_as_real( es );
					exprs_push_bool( es, v == *double_nan );
				}
				break;

			case JSOP_ISFINITE:
				{
					double v;
					char dummy[ 64 ];
					int rc = TRUE;
					v = exprs_pop_as_real( es );
					sprintf( dummy, "%.16g", v );
					if( dummy[ 0 ] == 'I' )
						rc = FALSE;
					else if( dummy[ 0 ] == '-' && dummy[ 1 ] == 'I' )
						rc = FALSE;
					else if( dummy[ 0 ] == 'N' )
						rc = FALSE;
					exprs_push_bool( es, rc );
				}
				break;

			case JSOP_PARSEFLOAT:
				{
					char buffer[ 128 ], *endp;
					double v;
					exprs_pop_as_string( es, buffer, sizeof( buffer ) );
					v = strtod( buffer, &endp );
					exprs_push_real( es, endp > buffer ? v : *double_nan );
				}
				break;

			case JSOP_PARSEINT:
				{
					char buffer[ 128 ], *endp;
					int radix;
					double v;
					radix = exprs_pop_as_real( es );
					exprs_pop_as_string( es, buffer, sizeof( buffer ) );
					if( radix )
						v = strtol( buffer, &endp, radix );
					else
						v = floor( strtod( buffer, &endp ) );
					exprs_push_real( es, endp > buffer ? v : *double_nan );
				}
				break;

			case JSOP_MAKESTRING:
				{
					int strl;
					char *temp;
					exprs_get_type( es, 0, &strl );
					temp = malloc( strl + 1 );
					memset( temp, '\0', strl + 1 ); /* TOFIX: probably not needed */
					exprs_pop_as_string( es, temp, 0 );
					exprs_push_str( es, temp, strl );
					free( temp );
				}
				break;

			case JSOP_MAKENUMBER:
				{
					double v = exprs_pop_as_real( es );
					exprs_push_real( es, v );
				}
				break;

			case JSOP_ESCAPE:
				{
					extern void encodedata( char*,char*);
					int strl;
					char *temp, *temp2;
					exprs_get_type( es, 0, &strl );
					temp = malloc( strl + 1 );
					temp2 = malloc( strl * 3 + 1 );
					if( !temp || !temp2 )
						longjmp( es->myrb, 1 );
					memset( temp, '\0', strl + 1 ); /* TOFIX: 2 lines probably not needed */
					memset( temp2, '\0', strl * 3 + 1 );
					exprs_pop_as_string( es, temp, strl + 1 );
					encodedata( temp, temp2 );
					exprs_push_str( es, temp2, strlen( temp2 ) );
					free( temp );
					free( temp2 );
				}
				break;

			case JSOP_UNESCAPE:
				{
					int strl;
					char *temp, *p;
					exprs_get_type( es, 0, &strl );
					temp = malloc( strl * 3 + 1 );
					if( !temp )
						longjmp( es->myrb, 1 );
					memset( temp, '\0', strl * 3 + 1 ); /* TOFIX: probably not needed */
					exprs_pop_as_string( es, temp, strl + 1 );

					while( p = strchr( temp, '%' ) )
					{
						char bf[ 4 ];
						long v;

						bf[ 0 ] = p[ 1 ];
						bf[ 1 ] = p[ 2 ];
						bf[ 2 ] = 0;

						stch_l( bf, &v );
						*p = v;
						strcpy( p + 1, p + 3 );
					}

					exprs_push_str( es, temp, strlen( temp ) );
					free( temp );
				}
				break;

			case JSOP_OP_POPVAL:
				// just pop value, it wasn't used
				D( db_js, bug( "popping unused value\n" ) );
				exprs_pop( es );
				break;

			case JSOP_OP_NEGATE:
				{
					double v = exprs_pop_as_real( es );
					exprs_push_real( es, -v );
				}
				break;

			case JSOP_OP_GG:
				{
					int v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );

					exprs_push_int( es, v2 >> v1 );
				}
				break;

			case JSOP_OP_GGG:
				{
					unsigned int v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );

					exprs_push_int( es, v2 >> v1 );
				}
				break;

			case JSOP_OP_LL:
				{
					int v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );

					exprs_push_int( es, v2 << v1 );
				}
				break;

			case JSOP_OP_STREQ:
				{
					int t1 = exprs_get_type( es, 0, NULL );
					int t2 = exprs_get_type( es, 1, NULL );

					if( t1 != t2 && t1 != expt_undefined && t2 != expt_undefined )
					{
						exprs_pop( es );
						exprs_pop( es );
						exprs_push_bool( es, FALSE );
						break;
					}

					opcode = JSOP_OP_EQ;
				}

			case JSOP_OP_EQ:
			case JSOP_OP_GT:
			case JSOP_OP_LT:
			case JSOP_OP_GTEQ:
			case JSOP_OP_LTEQ:
				{
					int rv = 0; // Avoid crappy compiler warning

					D( db_js, bug( "beginning comparision\n" ) );

					if( exprs_check_scalar( es, 2 ) )
					{
						double v1, v2;

						v1 = exprs_pop_as_real( es );
						v2 = exprs_pop_as_real( es );

						D( db_js, bug( "scalar comparision: %ld, %ld\n", (int)v1, (int)v2 ) );

						switch( opcode )
						{
							case JSOP_OP_GT:
								rv = ( v2 > v1 );
								break;
							case JSOP_OP_LT:
								rv = ( v2 < v1 );
								break;
							case JSOP_OP_EQ:
								rv = ( v2 == v1 );
								break;
							case JSOP_OP_GTEQ:
								rv = ( v2 >= v1 );
								break;
							case JSOP_OP_LTEQ:
								rv = ( v2 <= v1 );
								break;
						}
					}
					else if( rv = exprs_check_null_or_undefined( es, 2 ) )
					{
						exprs_pop( es );
						exprs_pop( es );
						if( opcode == JSOP_OP_EQ )
							rv = ( rv == 3 );
						else
							rv = FALSE;
					}
					else if( exprs_check_objects( es, 2 ) )
					{
						// Both are object references
						APTR obj1, obj2;
						exprs_pop_as_object( es, &obj1 );
						exprs_pop_as_object( es, &obj2 );

						D( db_js, bug( "object reference comparision: %lx, %lx\n", (int)obj1, (int)obj2 ) );
						
						if( opcode == JSOP_OP_EQ )
						{
							rv = ( obj1 == obj2 );
						}
						else
							longjmp( *es->rb, 19 );
					}
					else
					{
						char *s1, *s2;

						s1 = exprs_peek_as_string( es, 0 );
						s2 = exprs_peek_as_string( es, 1 );

						D( db_js, bug( "string comparision: %s, %s\n", s1, s2 ) );

						rv = strcmp( s2, s1 );

						switch( opcode )
						{
							case JSOP_OP_GT:
								rv = ( rv > 0 );
								break;
							case JSOP_OP_LT:
								rv = ( rv < 0 );
								break;
							case JSOP_OP_EQ:
								rv = ( rv == 0 );
								break;
							case JSOP_OP_GTEQ:
								rv = ( rv >= 0 );
								break;
							case JSOP_OP_LTEQ:
								rv = ( rv <= 0 );
								break;
						}
						exprs_pop( es );
						exprs_pop( es );
					}
					exprs_push_bool( es, rv );
				}
				break;

			case JSOP_OP_MUL:
				{
					double v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );

					exprs_push_real( es, v1 * v2 );
				}
				break;

			case JSOP_OP_BOOLNEG:
				{
					int v = exprs_pop_as_bool( es );
					exprs_push_bool( es, !v );
				}
				break;

			case JSOP_OP_ADDONE:
				{
					double v = exprs_pop_as_real( es );
					exprs_push_real( es, v + 1.0 );
				}
				break;

			case JSOP_OP_SUBONE:
				{
					double v = exprs_pop_as_real( es );
					exprs_push_real( es, v - 1.0 );
				}
				break;

			case JSOP_OP_ADD:
				{
					int sl1, sl2;
					int t1, t2;

					t1 = exprs_get_type( es, 0, &sl1 );
					t2 = exprs_get_type( es, 1, &sl2 );

					D( db_js, bug( "t1 %ld sl1 %ld, t2 %ld sl2 %ld\n", t1, sl1, t2, sl2 ) );

					if( t1 == expt_obj )
					{
						APTR o = exprs_peek_as_object( es, 0 );
						if( o && OCLASS( o ) != getjs_bool() && OCLASS( o ) != getjs_real() && OCLASS( o ) != getjs_date() )
							t1 = expt_string;
					}
					if( t2 == expt_obj )
					{
						APTR o = exprs_peek_as_object( es, 1 );
						if( o && OCLASS( o ) != getjs_bool() && OCLASS( o ) != getjs_real() && OCLASS( o ) != getjs_date() )
							t2 = expt_string;
					}

					if( t1 != expt_string && t2 != expt_string )
					{
						double v1, v2;

						v1 = exprs_pop_as_real( es );
						v2 = exprs_pop_as_real( es );
						exprs_push_real( es, v1 + v2 );
					}
					else
					{
						char *tmp, bf;
						// string concatanation
						tmp = malloc( sl1 + sl2 + 1 );
						if( !tmp )
							longjmp( *es->rb, 1 );
						memset( tmp, '\0', sl1 + sl2 + 1 ); /* TOFIX: probably not needed */
						exprs_pop_as_string( es, tmp + sl2, 0 );
						bf = tmp[ sl2 ];
						exprs_pop_as_string( es, tmp, 0 );
						tmp[ sl2 ] = bf;
						exprs_push_str( es, tmp, sl1 + sl2 );
						free( tmp );
					}
				}
				break;

			case JSOP_OP_SUB:
				{
					double v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );
					exprs_push_real( es, v2 - v1 );
				}
				break;

			case JSOP_OP_DIV:
				{
					double v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );
					if( !v1 )
						exprs_push_real( es, *double_inf );
					else    
						exprs_push_real( es, v2 / v1 );
				}
				break;

			case JSOP_OP_MOD:
				{
					double v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );

					if( !v1 )
						exprs_push_real( es, *double_inf );
					else    
						exprs_push_real( es, fmod( v2, v1 ) );
				}
				break;

			case JSOP_OP_BINAND:
				{
					int v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );
					exprs_push_int( es, v2 & v1 );
				}
				break;

			case JSOP_OP_BINOR:
				{
					int v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );
					exprs_push_int( es, v2 | v1 );
				}
				break;

			case JSOP_OP_BOOLAND:
				{
					int v1;

					v1 = exprs_pop_as_bool( es );
					if( !v1 )
					{
						int nestcnt = 1;
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_OP_BOOLAND )
								nestcnt++;
							else if( *execindex == JSOP_OP_BOOLAND_END )
								nestcnt--;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
						exprs_push_bool( es, FALSE );
					}
				}
				break;

			case JSOP_OP_BOOLOR:
				{
					int v1;

					v1 = exprs_pop_as_bool( es );
					if( v1 )
					{
						int nestcnt = 1;
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_OP_BOOLOR )
								nestcnt++;
							else if( *execindex == JSOP_OP_BOOLOR_END )
								nestcnt--;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
						exprs_push_bool( es, TRUE );
					}
				}
				break;

			case JSOP_OP_BOOLAND_END:
			case JSOP_OP_BOOLOR_END:
				break;

			case JSOP_OP_SELECT:
				{
					int v1;

					v1 = exprs_pop_as_bool( es );
					if( !v1 )
					{
						int nestcnt = 1;
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_OP_SELECT )
								nestcnt++;
							else if( *execindex == JSOP_OP_SELECT_SKIP )
								nestcnt--;
							SKIPOP;
						}
						// not found, throw up
						if( nestcnt )
							longjmp( es->myrb, 6 );
					}
				}
				break;

			case JSOP_OP_SELECT_SKIP:
				{
					int nestcnt = 1;
					while( nestcnt && execindex < maxexec )
					{
						if( *execindex == JSOP_OP_SELECT )
							nestcnt++;
						else if( *execindex == JSOP_OP_SELECT_END )
							nestcnt--;
						SKIPOP;
					}
					// skip until select end
				}
				break;

			case JSOP_OP_SELECT_END:
				break;

			case JSOP_OP_BINEOR:
				{
					int v1, v2;

					v1 = exprs_pop_as_real( es );
					v2 = exprs_pop_as_real( es );
					exprs_push_int( es, v2 ^ v1 );
				}
				break;

			case JSOP_OP_BINNEG:
				{
					int v = exprs_pop_as_real( es );
					exprs_push_int( es, ~v );
				}
				break;

			case JSOP_PUSH_INT:
				{
					execindex++; // skip data size (2)
					exprs_push_int( es, *( (int*)execindex ) );
					execindex += 2; // skip int val
				}
				break;

			case JSOP_PUSH_BOOL:
				{
					execindex++; // skip data size (2)
					exprs_push_bool( es, *( (int*)execindex ) );
					execindex += 2; // skip int val
				}
				break;

			case JSOP_PUSH_OBJECT:
				{
					execindex++; // skip data size (2)
					exprs_push_obj( es, *( (APTR*)execindex ) );
					execindex += 2; // skip APTR val
				}
				break;

			case JSOP_PUSH_FUNCPTR:
				{
					execindex++; // skip data size (2)
					exprs_push_funcptr( es, *( (int*)execindex ), refobj, es->window );
					execindex += 2; // skip int val
				}
				break;

			case JSOP_PUSH_STR:
				{
					exprs_push_str( es, (char*)&execindex[ 1 ], *execindex );
					execindex += 1 + ( ( *execindex + 1 ) / 2 );
				}
				break;

			case JSOP_PUSH_REAL:
				{
					execindex++;
					exprs_push_real( es, TO_DOUBLE( execindex ) );
					execindex += sizeof( double ) / 2;
				}
				break;

			case JSOP_EX_DEBUG:
				{
					exprs_pop_as_string( es, buffer, sizeof( buffer ) );
#ifndef MBX
					Printf( "JSDEBUG: %s\n", ( int )buffer );
#else
					kprintf( "JSDEBUG: %s\n", ( int )buffer );
#endif
				}
				break;

			case JSOP_LINENO:
				{
					// set linenumber
					*execlinecnt = execindex[ 1 ];
					*execurl = (char*)&execindex[ 2 ];
					execindex += 1 + ( ( *execindex + 1 ) / 2 );
				}
				break;

			// ...
			case JSOP_PUSH_VARNAME:
			case JSOP_PUSH_REFOP:
				{
					if( es->varnameptr == MAXVARNAMES )
						longjmp( es->myrb, 16 );
					stccpy( es->varname[ ++es->varnameptr ], (char*)&execindex[ 1 ], *execindex + 1 );
					// Store current reference varname
					execindex += 1 + ( ( *execindex + 1 ) / 2 );
					es->varname_ref[ es->varnameptr ] = ( opcode == JSOP_PUSH_REFOP );
				}
				break;

			case JSOP_PUSH_ARRAYOP:
				{
					if( es->varnameptr == MAXVARNAMES )
						longjmp( es->myrb, 16 );
					exprs_pop_as_string( es, es->varname[ ++es->varnameptr ], 256 );
					es->varname_ref[ es->varnameptr ] = TRUE;
					D( db_js, bug( "push-array %s, %ld, %ld\n", es->varname[ es->varnameptr ], es->varname_ref[ es->varnameptr ], es->varnameptr ) );
				}
				break;

			case JSOP_PUSH_THIS:
				exprs_push_obj( es, es->thisobject );
				break;

			case JSOP_PUSH_NULL:
				exprs_push_obj( es, NULL );
				break;

			case JSOP_PUSH_UNDEFINED:
				exprs_push_undefined( es );
				break;

			case JSOP_ASSIGN:
				dom_set( es, FALSE );
				es->varnameptr--;
				//exprs_dump( es );
				break;

			case JSOP_ASSIGN_AND_KEEP:
				dom_set( es, FALSE );
				break;

			case JSOP_ASSIGNPROP:
				dom_set( es, 2 );
				exprs_pop( es );
				es->varnameptr--;
				break;

			case JSOP_ASSIGNARRAY:
				{
					APTR obj;
					exprs_pop_as_object( es, &obj );
					DoMethod( es->default_object, MM_JS_Array_AddMember, ( ULONG )obj );
				}
				break;

			case JSOP_ASSIGNLOCAL:
				dom_set( es, TRUE );
				es->varnameptr--;
				break;

			case JSOP_SETIFUNSET:
				{
					int rc = 0;
					APTR o;
					for( o = FIRSTNODE( &cjsol->vars ); NEXTNODE( o ); o = NEXTNODE( o ) )
					{
						APTR obj = BASEOBJECT( o );

						rc = DoMethod( obj, MM_JS_NameIs, ( ULONG )es->varname[ es->varnameptr ] );
						if( rc )
						{
							int found_context;
							get( obj, MA_JS_FuncContext, &found_context );
							if( found_context == es->func_context )
								break;
							rc = 0;
						}
					}

					if( !rc )
					{
						exprs_push_undefined( es );
						dom_set( es, TRUE );
					}
					es->varnameptr--;
				}
				break;

			case JSOP_FUNC_BEGIN:
				//Printf( "skipping\n" );
				{
					int nestcnt = 1;

					execindex += ( execindex[ 0 ] + 1 ) / 2 + 1;
					while( nestcnt && execindex < maxexec )
					{
						if( *execindex == JSOP_FUNC_END )
							nestcnt--;
						else if( *execindex == JSOP_FUNC_BEGIN )
							nestcnt++;
						SKIPOP;
					}
				}
				break;

			case JSOP_FUNC_BEGIN_EXPR:
				exprs_push_funcptr( es, execindex - jsol->i, refobj, es->window );

				while( execindex < maxexec && *execindex != JSOP_FUNC_END )
				{
					SKIPOP;
				}
				SKIPOP;
				break;

			case JSOP_FUNC_CALL:
				exprs_reverse_till_mark( es, TRUE );
dofunccall:
				{
					LONG ix;
					APTR fobj, fpobj, winobj;
					int r_type, r_size;
					APTR r_data;
					D( db_js, bug( "beginning funccall\n" ) );
//#if DEBUG == 1 //TOFIX !!
//					  exprs_dump( es );
//#endif
					ix = exprs_pop_as_funcptr( es, &fobj, &fpobj, &winobj );
					if( ix < 0 )
					{
						int argcnt = 0;
						struct MP_JS_CallMethod msg;

						while( exprs_get_type( es, argcnt, NULL ) != expt_argmarker )
						{
							argcnt++;
						}

						D( db_js, bug( "invoking method ID %ld on object %lx, %ld args\n", ix, fobj, argcnt ) );
						if( ix == -1 )
						{
							// Special toString hack
							int radix = 0;
							char buffer[ 1024 ];
							int bfsize = sizeof( buffer );

							if( argcnt > 0 )
							{
								radix = exprs_pop_as_real( es );
								argcnt--;
							}

							if( fobj )
							{
								if( !DoMethod( fobj, MM_JS_ToString, buffer, &bfsize, radix ) )
								{
									sprintf( buffer, "[object %s]", (char*)getv( fobj, MA_JS_ClassName ) );
								}
							}
							else
							{
								strcpy( buffer, "null" );
								bfsize = 4;
							}

							while( argcnt-- >= 0 )
								exprs_pop( es );

							buffer[ bfsize ] = 0;
							exprs_push_str( es, buffer, bfsize );

							break;
						}

						// Default return value
						r_type = expt_undefined;

						msg.MethodID = MM_JS_CallMethod;
						msg.pid = -ix;
						msg.typeptr = &r_type;
						msg.dataptr = &r_data;
						msg.datasize = &r_size;
						msg.argcnt = argcnt;
						msg.es = es;

						if( !DoMethodA( fobj, ( Msg )&msg ) )
							longjmp( es->myrb, 14 );

						D( db_js, bug( "function call done; remaining args %ld, rtype %ld\n", argcnt, r_type ) );

						// pop the arg marker as well
						while( msg.argcnt-- > 0 )
							exprs_pop( es );
						exprs_pop( es );

						// push the resulting value on the expression stack
						switch( r_type )
						{
							case expt_real:
								exprs_push_real( es, TO_DOUBLE( r_data ) );
								break;

							case expt_bool:
								exprs_push_bool( es, (int)r_data );
								break;

							case expt_string:
								exprs_push_str( es, r_data, r_size );
								break;

							case expt_obj:
								exprs_push_obj( es, r_data );
								break;

							case expt_undefined:
								exprs_push_undefined( es );
								break;

							// Nonsense, really
							//case expt_funcptr:
							//	exprs_push_funcptr( es, (LONG)r_data, refobj );
							//	break;

							default:
								longjmp( *es->rb, 13 );
						}

					}
					else
					{
						int argcnt = 0;
						if( fpobj )
						{
							APTR argarray = JSNewObject( getjs_array(),
								MA_JS_FuncContext, es->func_context,
								TAG_DONE 
							);

							while( exprs_get_type( es, argcnt, NULL ) != expt_argmarker )
							{
								APTR o = exprs_peek_as_object( es, argcnt );
								DoMethod( argarray, MM_JS_Array_AddMember, ( ULONG )o );
								argcnt++;
							}
							DoMethod( fpobj, MM_JS_Func_SetParms, ( ULONG )argarray );
						}

						if( winobj != es->window )
						{
							APTR retobj;

							D( db_js, bug( "calling function in different window (%lx %s vs. %lx %s ) with %ld args\n", winobj, OCLASS( winobj )->cl_ID, es->window, OCLASS( es->window )->cl_ID, argcnt ) );
							// Pop arguments (they're passed via the func object argument array)
							while( argcnt-- )
								exprs_pop( es );
							// and the marker, too
							exprs_pop( es );

							retobj = (APTR)DoMethod( winobj, MM_HTMLWin_ExecuteJSFunc, fpobj );
							D( db_js, bug( "call returned %ld\n", retobj ) );
							exprs_push_obj( es, retobj );
						}
						else
						{
							markstack_push( es, marktype_funcret, execindex );
							es->func_context++;
							D( db_js, bug( "calling function @%ld, new func_context %ld, ms %ld, from %ld\n", ix, es->func_context, cntlist( &es->marklist ), execindex - cjsol->i ) );
							execindex = cjsol->i + ix;
						}
					}
				}
				break;

			case JSOP_FUNC_RETURN_NOVAL:
				if( markstack_peek( es ) != marktype_funcret )
				{
					// When not calling a function, probably trying to leave event handler
					execindex = maxexec;
					break;
				}
			case JSOP_FUNC_END:
				{
					execindex = markstack_get( es, marktype_funcret );
					exprs_push_undefined( es );
					markstack_pop( es );
					dom_erase( es );
					es->func_context--;
					D( db_js, bug( "dropping out to ix %ld, ms = %ld, new func_context %ld\n", execindex - cjsol->i, cntlist( &es->marklist ), es->func_context ) );
//#if DEBUG == 1  //TOFIX!!
//					  exprs_dump( es );
//#endif
				}
				break;

			case JSOP_FUNC_RETURN:
				{
					if( markstack_peek( es ) != marktype_funcret )
					{
						D( db_js, bug( "return, finalizing\n" ) );
						// When not calling a function, probably trying to leave event handler
						execindex = maxexec;
						break;
					}

					execindex = markstack_get( es, marktype_funcret );
					// return expression is on the stack
					markstack_pop( es );
					dom_erase( es );
					es->func_context--;
					D( db_js, bug( "returning to ix %ld, ms %ld\n", execindex- cjsol->i, cntlist( &es->marklist ) ) );
				}
				break;

			case JSOP_FUNC_BEGINPARMS:
				exprs_push_argmarker( es );
				break;

			case JSOP_FUNC_ASSIGNPARM:
				if( exprs_get_type( es, 0, NULL ) == expt_argmarker )
				{
					// no more args
					exprs_push_undefined( es );
				}
				dom_set( es, TRUE );
				es->varnameptr--;
				exprs_pop( es );
				break;

			case JSOP_FUNC_PARMCLEANUP:
				while( exprs_get_type( es, 0, NULL ) != expt_argmarker )
					exprs_pop( es );
				// pop the arg marker as well
				exprs_pop( es );
				break;

			case JSOP_OP_TYPEOF:
				{
					char *s = "undefined";

					switch( exprs_get_type( es, 0, NULL ) )
					{
						case expt_real:
							s = "number";
							break;

						case expt_string:
							s = "string";
							break;

						case expt_bool:
							s = "bool";
							break;

						case expt_obj:
							s = "object";
							break;

						case expt_funcptr:
							s = "function";
							break;
					}
					exprs_pop( es );
					exprs_push_str( es, s, strlen( s ) );
				}
				break;

			case JSOP_EVAL:
				{
					char *cmd = exprs_peek_as_string( es, 0 );
					int offs = execindex - jsol->i;
					int moffs = maxexec - jsol->i;
					int nix = -1;
					struct expr *ex;
					struct evalcache *evc;
					int l = strlen( cmd );

					for( evc = FIRSTNODE( &jsol->eval_cache ); NEXTNODE( evc ); evc = NEXTNODE( evc ) )
					{
						if( evc->l == l )
						{
							if( !strcmp( evc->txt, cmd ) )
							{
								nix = evc->index;
								D( db_js, bug( "using cached eval() code for %s\n", cmd ) );
								break;
							}
						}
					}

					if( nix < 0 )
					{
						nix = js_compile( refwin, jsol, cmd, l, *execlinecnt, docurl );
						if( nix > 0 )
						{
							evc = AllocPooled( jsol->pool, sizeof( *evc ) + l + 1 );
							if( evc )
							{
								strcpy( evc->txt, cmd );
								evc->index = nix;
								evc->l = l;								
								ADDTAIL( &jsol->eval_cache, evc );
							}
						}
					}

					// Pop execute string
					exprs_pop( es );

					ex = FIRSTNODE( &es->l );

					if( nix > 0 )
						js_bytecode( refwin, refobj, thisobject, baseref, jsol, jsol->i + nix, jsol->i + jsol->instsize, es, execlinecnt, docurl, execurl );

					// if nothing was returned (or error), push undefined
					if( FIRSTNODE( es ) == ex )
						exprs_push_undefined( es );

					execindex = jsol->i + offs;
					maxexec = jsol->i + moffs;
				}
				break;

			case JSOP_PUSH_OCONTEXT:
				{
					APTR obj;
					
					exprs_pop_as_object( es, &obj );
					contextstack_push( es, obj, 0 );
				}
				break;

			case JSOP_POP_OCONTEXT:
				{
					contextstack_pop( es );
				}
				break;

			case JSOP_CREATEOBJECT:
			case JSOP_CREATEARRAY:
				{
					APTR obj = JSNewObject( opcode == JSOP_CREATEOBJECT ? getjs_object() : getjs_array(),
						TAG_DONE
					);
					if( !obj )
						longjmp( es->myrb, 1 );

					exprs_push_obj( es, obj );
					contextstack_push( es, obj, 0 );
				}
				break;

			case JSOP_CREATE_REGEXP:
				{
					char *args;
					APTR obj;

					args = malloc( *execindex + 1 );
					stccpy( args, (char*)&execindex[ 1 ], *execindex + 1 );

					obj = JSNewObject( getjs_regexp(),
						MA_JS_RegExp_Source, args + 2,
						MA_JS_RegExp_NoCase, args[ 0 ] == 'i',
						MA_JS_RegExp_Global, args[ 1 ] == 'g',
						TAG_DONE
					);

					free( args );

					D( db_js, bug( "creating literal regexp object %s, obj %lx\n", args, obj ) );
					if( !obj )
						longjmp( es->myrb, 1 );

					execindex += 1 + ( ( *execindex + 1 ) / 2 );

					exprs_push_obj( es, obj );
				}
				break;


			case JSOP_DELETE:
				{
					dom_delete( es );
					es->varnameptr--;
					exprs_push_undefined( es );
				}
				break;

			case JSOP_NEW:
				{
					int argcnt = 0;
					char *classname = es->varname[ es->varnameptr-- ];
					APTR newobj;

					while( exprs_get_type( es, argcnt, NULL ) != expt_argmarker )
					{
						argcnt++;
					}

					D( db_js, bug( "NEW Object %s with %ld arguments\n", classname, argcnt ) );

					if( !strcmp( classname, "Object" ) )
					{
						APTR newobj = JSNewObject( getjs_object(), TAG_DONE );
						if( !newobj )
							longjmp( es->myrb, 1 );
						// Drop any excess constructor arguments
						while( argcnt-- >= 0 )
							exprs_pop( es );
						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "Array" ) )
					{
						APTR newobj = JSNewObject( getjs_array(), TAG_DONE );
						if( !newobj )
							longjmp( es->myrb, 1 );

						exprs_reverse_till_mark( es, FALSE );

						while( argcnt-- )
						{
							APTR o;
							exprs_pop_as_object( es, &o );
							DoMethod( newobj, MM_JS_Array_AddMember, ( ULONG )o );
						}
						exprs_pop( es ); // Argmarker

						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "Boolean" ) )
					{
						int v = FALSE;
						if( argcnt )
						{
							v = exprs_pop_as_bool( es );
							argcnt--;
						}
						// Drop any excess constructor arguments
						while( argcnt-- >= 0 )
							exprs_pop( es );
						exprs_push_bool( es, v );
						exprs_pop_as_object( es, &newobj );
						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "Image" ) )
					{
						int width = 0, height = 0;
						APTR newobj;

						if( argcnt )
						{
							width = exprs_pop_as_real( es );
							argcnt--;
							if( argcnt )
							{
								height = exprs_pop_as_real( es );
								argcnt--;
							}
						}
						// Drop any excess constructor arguments
						while( argcnt-- >= 0 )
							exprs_pop( es );

						newobj = JSNewObject( getloimageclass(),
							//MA_DNode_XS, width,
							//MA_DNode_YS, height,
							MA_JS_Object_Baseref, baseref,
							TAG_DONE
						);
						if( !newobj )
							longjmp( es->myrb, 1 );

						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "RegExp" ) )
					{
						char *args_src = "";
						char *args_flags = "";
						APTR newobj;

						if( argcnt > 0 )
						{
							args_src = exprs_peek_as_string( es, 0 );
							if( argcnt > 1 )
								args_flags = exprs_peek_as_string( es, 1 );
						}
						// Drop any excess constructor arguments
						while( argcnt-- >= 0 )
							exprs_pop( es );

						newobj = JSNewObject( getjs_regexp(),
							MA_JS_RegExp_Source, args_src,
							MA_JS_RegExp_NoCase, strchr( args_flags, 'i' ),
							MA_JS_RegExp_Global, strchr( args_flags, 'g' ),
							TAG_DONE
						);
						if( !newobj )
							longjmp( es->myrb, 1 );

						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "Number" ) )
					{
						double v = 0;
						if( argcnt )
						{
							v = exprs_pop_as_real( es );
							argcnt--;
						}
						// Drop any excess constructor arguments
						while( argcnt-- >= 0 )
							exprs_pop( es );
						exprs_push_real( es, v );
						exprs_pop_as_object( es, &newobj );
						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "String" ) )
					{
						int strl;
						char *temp;

						if( argcnt )
						{
							exprs_get_type( es, 0, &strl );
							temp = malloc( strl + 1 );
							memset( temp, '\0', strl + 1 ); /* TOFIX: probably not needed.. NULL check missing */
							exprs_pop_as_string( es, temp, 0 );
							// Drop any excess constructor arguments
							while( --argcnt >= 0 )
								exprs_pop( es );
							exprs_push_str( es, temp, strl );
							free( temp );
						}
						else
							exprs_push_str( es, "", 0 );
						exprs_pop_as_object( es, &newobj );
						contextstack_push( es, newobj, 1 );
					}
					else if( !strcmp( classname, "Date" ) )
					{
						APTR newobj;

						newobj = JSNewObject( getjs_date(),
							TAG_DONE
						);
						if( !newobj )
							longjmp( es->myrb, 1 );

						if( argcnt )
						{
							if( argcnt == 1 )
							{
								if( exprs_check_scalar( es, 1 ) )
								{
									// Construct from Milliseconds
									double millis = exprs_pop_as_real( es );
									SetAttrs( newobj,
										MA_JS_Date_Seconds, (int)(millis / 1000.0),
										MA_JS_Date_Micros, (int)millis % 1000,
										TAG_DONE
									);
								}
								else
								{
									// Construct from string
									char buffer[ 128 ];

									exprs_pop_as_string( es, buffer, sizeof( buffer ) );
									set( newobj, MA_JS_Datestring, buffer );
								}
								argcnt--;
							}
							else
							{
								double rval;
								struct tm tm;

								if( argcnt < 3 )
									longjmp( *es->rb, 14 );

								memset( &tm, 0, sizeof( tm ) );

								if( argcnt > 3 )
								{
									if( argcnt < 6 )
										longjmp( *es->rb, 14 );

									if( argcnt > 6 )
									{
										rval = exprs_pop_as_real( es );
										set( newobj, MA_JS_Date_Micros, (int)rval * 1000 );
										argcnt--;
									}

									rval = exprs_pop_as_real( es );
									tm.tm_sec = rval;

									rval = exprs_pop_as_real( es );
									tm.tm_min = rval;

									rval = exprs_pop_as_real( es );
									tm.tm_hour = rval;

									argcnt -= 3;
								}

								rval = exprs_pop_as_real( es );
								tm.tm_mday = rval;

								rval = exprs_pop_as_real( es );
								tm.tm_mon = rval;

								rval = exprs_pop_as_real( es );
								tm.tm_year = ( rval <= 99 ) ? rval : rval - 1900;

								argcnt -= 3;

								set( newobj, MA_JS_Date_Seconds, (int)mktime( &tm ) );

							}
						}

						// Drop any excess constructor arguments
						while( argcnt-- >= 0 )
							exprs_pop( es );

						contextstack_push( es, newobj, 1 );
					}
					else
					{
						D( db_js, bug( "unknown class %s, finding function\n", classname ) );

						newobj = JSNewObject( getjs_object(),
							TAG_DONE
						);
						if( !newobj )
							longjmp( es->myrb, 1 );
						contextstack_push( es, newobj, 1 );

						exprs_reverse_till_mark( es, FALSE );

						es->varnameptr++;
						dom_eval( es, TRUE );
						es->varnameptr--;

						last_new_was_constructor = TRUE;

						goto dofunccall;
					}

					// clean up any excess constructor arguments
				}
				break;

			case JSOP_NEW_END:
				if( last_new_was_constructor )
					exprs_pop( es ); // Drop the function return argument
				last_new_was_constructor = FALSE;
				exprs_push_obj( es, es->thisobject );
				contextstack_pop( es );
				break;

			case JSOP_SWITCH_START:
				{
					markstack_push( es, marktype_switch, execindex );
				}
				break;

			case JSOP_CASE_START:
				{
					int stsz;
					
					switch( exprs_get_type( es, 0, &stsz ) )
					{
						case expt_real:
							{
								double v;
								v=exprs_peek_as_real( es, 0 );
								exprs_push_real( es, v );
							}
							break;
						case expt_bool:
							{
								int v;
								v=exprs_peek_as_bool( es, 0 );
								exprs_push_bool( es, v );
							}
							break;
						case expt_string:
							{
								char *v;
								v=exprs_peek_as_string( es, 0 );
								exprs_push_str( es, v, stsz );
							}
							break;
						case expt_funcptr:
							{
								LONG v;
								APTR obj, w;
								v=exprs_peek_as_funcptr( es, 0, &obj, &w );
								exprs_push_funcptr( es, v, obj, w );
							}
							break;
						case expt_obj:
							{
								APTR v;
								v=exprs_peek_as_object( es, 0 );
								exprs_push_obj( es, v );
							}
							break;
						case expt_undefined:
							exprs_push_undefined( es );
							break;
					}
				}
				break;

			case JSOP_CASE_DEF:
			case JSOP_CASE:
				{
					int v;

					if( opcode == JSOP_CASE )
					{
						v=exprs_pop_as_bool( es );
					}
					else
					{
						v = FALSE;
						lastseendefault = execindex;
					}

					D( db_js, bug( "v=%d\r\n", v ) );
					if( !v )
					{
						int nestcnt = 0;
						while( ( nestcnt || ( *execindex!=JSOP_CASE_START && *execindex != JSOP_CASE_DEF && *execindex!=JSOP_SWITCH_END ) ) && execindex < maxexec )
						{
							if( *execindex == JSOP_SWITCH_START )
								nestcnt++;
							else if( *execindex == JSOP_SWITCH_END )
								nestcnt--;
							SKIPOP;
						}
						// if not found, throw up
						if( execindex>=maxexec )
							longjmp( es->myrb, 6 );
					}
					else
					{
						lastseendefault = 0;
						// Begin entering case block
					}
				}
				break;

			case JSOP_CASE_END:
				D( db_js, bug( "Skipping to next case\n" ) );
				while( execindex < maxexec )
				{
					if( *execindex == JSOP_CASE )
					{
						// Fallthrough to this case
						SKIPOP;
						break;
					}
					else if( *execindex == JSOP_SWITCH_END )
					{
						execindex--;
						break;
					}
					SKIPOP;
				}
				if( execindex >= maxexec )
				{
					// Something is screwed
					longjmp( es->myrb, 6 );
				}
				break;
			
			case JSOP_SWITCH_END:
				D( db_js, bug( "End of switch, lastseendefault=%ld\n", lastseendefault ) );
				if( lastseendefault )
				{
					// Continue at default rule
					execindex = lastseendefault;
					lastseendefault = 0;
					break;
				}
				exprs_pop( es );
				markstack_pop( es );
				break;


			//
			// Property lists handling for( x in y ) ...
			// 

			case JSOP_FORIN_BUILDPROPLIST:
				{
					struct forin_ctx *fctx;
					APTR pool;
					APTR obj;
					struct mark *topmark;

					exprs_pop_as_object( es, &obj );

					pool = CreatePool( MEMF_ANY, 4096, 2048 );
					fctx = AllocPooled( pool, sizeof( *fctx ) );
					fctx->ix = -1;
					NEWLIST( &fctx->l );
					fctx->pool = pool;

					if( obj )
						DoMethod( obj, MM_JS_ListProperties, &fctx->l, pool );

					stccpy( fctx->varname, es->varname[ es->varnameptr-- ], sizeof( fctx->varname ) );

					markstack_push( es, marktype_forin, execindex );
					topmark = FIRSTNODE( &es->marklist );
					topmark->fctx = fctx;

					D( db_js, bug( "Initializing property list, iter var '%s'\n", fctx->varname ) );
				}
				break;

			case JSOP_FORIN_SET:
				{
					struct mark *topmark;
					struct forin_ctx *fctx;
					struct proplistentry *ple;
					int c;

					if( markstack_peek( es ) != marktype_forin )
						longjmp( *es->rb, 10 );

					topmark = FIRSTNODE( &es->marklist );
					fctx = topmark->fctx;

					// Find next entry
					fctx->ix++;
					for( c = 0, ple = FIRSTNODE( &fctx->l ); ( c < fctx->ix ) && NEXTNODE( ple ); ple = NEXTNODE( ple ), c++ );

					D( db_js, bug( "Returning next property, index %ld, name '%s'\n", fctx->ix, ple->name ) );

					if( NEXTNODE( ple ) )
					{
						exprs_push_str( es, ple->name, strlen( ple->name ) );
						strcpy( es->varname[ ++es->varnameptr ], fctx->varname );
						es->varname_ref[ es->varnameptr ] = FALSE;
						dom_set( es, 1 );
						es->varnameptr--;
					}
					else
					{
						int nestcnt = 1;
						// Skip to end of for/in loop
						while( nestcnt && execindex < maxexec )
						{
							if( *execindex == JSOP_FORIN_SET )
								nestcnt++;
							else if( *execindex == JSOP_FORIN_END )
								nestcnt--;
							if( nestcnt )
								SKIPOP;
						}
						if( nestcnt )
							longjmp( es->myrb, 6 );
						markstack_pop( es );
						SKIPOP;
					}
				}
				break;

			case JSOP_FORIN_END:
				{
					execindex = markstack_get( es, marktype_forin );
				}
				break;

			default:
				longjmp( es->myrb, 4 ); // Fatal
		}
	}
}

/*
	Create a function call
*/
int js_createfunccall( struct jsop_list *jso, int funcindex, int numargs, APTR *args )
{
	int ix;

	cjsol = jso;
	ix = cjsol->instsize;

	SOPD( PUSH_FUNCPTR, &funcindex, sizeof( funcindex ) );
	SOP( FUNC_BEGINPARMS );

	while( numargs-- > 0 )
	{
		SOPD( PUSH_OBJECT, &(*args), sizeof( APTR ) );
		args++;
	}
	SOP( FUNC_CALL );
	SOP( END );

	return( ix );
}


/*
	Interpret some bytecode
*/
APTR js_interpret_obj( APTR refwin, APTR refobj, APTR thisobject, STRPTR baseref, struct jsop_list *jsol, int startix, STRPTR docurl )
{
	UWORD *execindex = jsol->i + startix, *maxexec = jsol->i + jsol->instsize;
	int rc;
	struct expr_stack *es, *oldes;
	char buffer[ 512 ];
	volatile int execlinecnt = 0;
	char *execurl = "(no file)";
	APTR returnobj;	

	cjsol = jsol;

	oldes = jsol->es;
	es = jsol->es = exprs_create();
	
	if( !es )
	{
		rc = 1;
		goto giveup;
	}

	es->currentcontextobj = refobj;
	es->thisobject = thisobject;
	es->window = refwin;

	if( rc = setjmp( es->myrb ) )
	{
giveup:
		switch( rc )
		{
			case 1:
				strcpy( buffer, "out of memory" );
				break;

			case 2:
				strcpy( buffer, "expression stack empty" );
				break;

			case 3:
				strcpy( buffer, "invalid type conversion" );
				break;

			case 4:
				strcpy( buffer, "unknown opcode" );
				break;

			// this isn't used anymore
			/*case 5:
				strcpy( buffer, "division by zero" );
				break;*/

			case 6:
				strcpy( buffer, "if/else/endif/while/for mismatch" );
				break;

			case 7:
				strcpy( buffer, "object/array reference mismatch" );
				break;

			case 8:
				strcpy( buffer, "lvalue expected" );
				break;

			case 9:
				strcpy( buffer, "variable name/reference list too long" );
				break;

			case 10:
				strcpy( buffer, "unexpected internal error: marker stack screwed" );
				break;

			case 11:
				sprintf( buffer, "undefined object/variable %s", es->varname[ es->varnameptr ] );
				break;

			case 12:
				sprintf( buffer, "object of class \"%s\" has no member \"%s\"", (char*)getv( es->errorobject, MA_JS_ClassName ), es->varname[ es->varnameptr ] );
				break;

			case 13:
				strcpy( buffer, "invalid property type conversion" );
				break;

			case 14:
				strcpy( buffer, "wrong number of arguments to function" );
				break;

			case 15:
				sprintf( buffer, "property \"%s\" of class \"%s\" can't be modified", es->varname[ es->varnameptr ], (char*)getv( es->errorobject, MA_JS_ClassName ) );
				break;

			case 16:
				strcpy( buffer, "too many object references in expression" );
				break;

			case 17:
				sprintf( buffer, "property \"%s\" is undefined", es->varname[ es->varnameptr ] );
				break;

			case 18:
				strcpy( buffer, "function expected" );
				break;

			case 19:
				strcpy( buffer, "illegal comparision with objects" );
				break;

			case 20:
				buffer[ 100 ] = 0;
				strins( buffer, "Invalid RegExp: " );
				break;

			default:
				sprintf( buffer, "unknown error code %d", rc );
				break;
		}

		if( getprefslong( DSI_JS_DEBUG, TRUE ) )
		{
			//MUI_Request( app, refwin, 0, GS( JS_ERROR_TITLE ), GS( CANCEL ), GS( JS_ERROR_EXEC ), docurl, execlinecnt, buffer );
			puterror( LT_JS, LL_ERROR, execlinecnt, execurl, buffer );
		}

		D( db_js, bug( "ERROR: %s, %ld -> %s\n", docurl, execlinecnt, buffer ) );

#if USE_JSERRORLOG
		if( getprefslong( DSI_JS_ERRORLOG, TRUE ) )
		{
			BPTR f = Open( getprefsstr( DSI_JS_LOGFILE, "PROGDIR:JSERROR.LOG" ), MODE_OLDFILE );
			if( !f )
				f = Open( getprefsstr( DSI_JS_LOGFILE, "PROGDIR:JSERROR.LOG" ), MODE_NEWFILE );
			else
				Seek( f, 0, OFFSET_END );
			if( f )
			{
				char *p = buffer;

				while( *p )
				{
					if( *p == '\033' )
						strcpy( p, p + 2 );
					else if( isspace( *p ) )
						*p++ = ' ';
					else
						p++;
				}

				FPrintf( f, "EXECUTION ERROR %ld\nVERSION: " LVERTAG "\nURL: %s\nLINE: %ld\nDESCRIPTION: %s\n\n",
					( int )rc,
					( int )execurl,
					( int )execlinecnt,
					( int )buffer
				);								
				Close( f );
			}
		}
#endif /* USE_JSERRORLOG */

		if( es )
		{
			dom_erase( es );
			exprs_del( es );
		}

		jsol->es = oldes;

		return( NULL );
	}

	exprs_push_undefined( es );

	js_bytecode( refwin, refobj, thisobject, baseref, jsol, execindex, maxexec, es, (int*)&execlinecnt, docurl, &execurl );

	// check whether something is on the expression stack
	exprs_pop_as_object( es, &returnobj );

	while( !ISLISTEMPTY( &es->l ) )
		exprs_pop( es );

	dom_erase( es );

	exprs_del( es );

	jsol->es = oldes;

	D( db_js, bug( "leaving js_interpret, result = %lx\n", returnobj ) );

	return( returnobj );
}

void js_interpret( APTR refwin, APTR refobj, APTR thisobject, STRPTR baseref, struct jsop_list *jsol, int startix, char *resultbuffer, int rbsize, STRPTR docurl )
{
	APTR returnobj;

	returnobj = js_interpret_obj( refwin, refobj, thisobject, baseref, jsol, startix, docurl );
	if( resultbuffer )
	{
		if( returnobj )
		{
			if( !DoMethod( returnobj, MM_JS_ToString, ( ULONG )resultbuffer, ( ULONG )&rbsize, 0 ) )
			{
				char buffer[ 128 ];

				sprintf( buffer, "[object %s]", (char*)getv( returnobj, MA_JS_ClassName ) );

				if( rbsize )
				{
					stccpy( resultbuffer, buffer, rbsize );
				}
				else
				{
					strcpy( resultbuffer, buffer );
				}
			}
		}
		else
		{
			strcpy( resultbuffer, "undefined" );
		}
	}
}

// ...

void *js_alloca( int size )
{
	return( AllocPooled( js_alloca_pool, size ) );
}

int js_compile( APTR refwin, struct jsop_list *jol, UBYTE *script, int scriptlen, int lineno, STRPTR docurl )
{
	int startix;

	lastlineno = -1;

#if USE_JS
	if( !gp_javascript )
#endif
		return( -1 );

	cjsol = jol;

	startix = cjsol->instsize;

//#define DODEBUG
#ifdef DODEBUG //TOFIX!!
	D( db_js, bug( "JS: run(%lx,%lx, %ld,%ld) ->\n=======\n", refwin, 0, scriptlen, lineno ) );
	WriteChars( script, scriptlen );
	PutStr( "\n===========\n" );
#endif

	js_docurl = docurl;

	init_js_alloca();
	errwin = refwin;
	if( setjmp( jsjmpbuf ) )
	{
		cleanup_js_alloca();
		return( -1 );
	}
	lex_initialize( script, scriptlen, lineno );
	yyparse();
	jso_storeop( JSOP_END, NULL, 0 );
	cleanup_js_alloca();

	return( startix );
}

void js_run( APTR refwin, APTR obj, STRPTR baseref, UBYTE *script, int scriptlen, int lineno, char *resultbuffer, int rbsize, STRPTR docurl )
{
	int startix;

	resultbuffer[ 0 ] = 0;

#if USE_JS
	if( !gp_javascript )
#endif
		return;

	// run...
	startix = js_compile( refwin, cjsol, script, scriptlen, lineno, docurl );
	if( startix > 0 )
	{
		js_interpret( refwin, obj, obj, baseref, cjsol, startix, resultbuffer, rbsize, docurl );
		D( db_js, bug( "js_run done; returning: %s\n", resultbuffer ) );
	}
}

// ******************************************************************************

// Helper functions

struct propt *findprop( struct propt *table, char *propname )
{
	while( table->name )
	{
		if( !strcmp( table->name, propname ) )
			return( table );
		table++;
	}
	return( NULL );
}

int findpropid( struct propt *table, char *propname )
{
	while( table->name )
	{
		if( !strcmp( table->name, propname ) )
			return( table->id );
		table++;
	}
	return( 0 );
}

void storestrprop( struct MP_JS_GetProperty *msg, char *string )
{
	if( !string )
		string = "";

	*msg->typeptr = expt_string;
	*msg->dataptr = string;
	*msg->datasize = strlen( string );
}

void storestrlprop( struct MP_JS_GetProperty *msg, char *string, int strl )
{
	if( !string )
		string = "";

	*msg->typeptr = expt_string;
	*msg->dataptr = string;
	*msg->datasize = strl;
}

void storefuncprop( struct MP_JS_GetProperty *msg, ULONG id )
{
	*msg->typeptr = expt_funcptr;
	*msg->dataptr = (APTR)id;
	*msg->datasize = 4;
}

void storeobjprop( struct MP_JS_GetProperty *msg, APTR obj )
{
	*msg->typeptr = expt_obj;
	*msg->dataptr = (APTR)obj;
	*msg->datasize = 4;
}

void storerealprop( struct MP_JS_GetProperty *msg, double val )
{
	static double v;

	v = val;
	*msg->typeptr = expt_real;
	*msg->dataptr = &v;
}

void storeintprop( struct MP_JS_GetProperty *msg, int val )
{
	static double v;

	v = val;
	*msg->typeptr = expt_real;
	*msg->dataptr = &v;
}

// The following functions provide standard implementations
// of the JS methods for "DOM" objects which are not derived
// from js_object

ULONG dom_hasproperty( struct MP_JS_HasProperty *msg, struct propt *ptable, struct MinList *cpl )
{
	struct propt *pt;
	customprop *cp;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	cp = cp_find( cpl, msg->propname );
	if( cp )
		return( expt_obj );

	return( 0 );
}

ULONG dom_getproperty( struct MP_JS_GetProperty *msg, struct propt *ptable, struct MinList *cpl )
{
	struct propt *pt;
	customprop *cp;

	if( pt = findprop( ptable, msg->propname ) )
	{
		if( pt->type == expt_funcptr )
		{
			storefuncprop( msg, -pt->id );
			return( TRUE );
		}
		return( (ULONG)pt->id );
	}

	cp = cp_find( cpl, msg->propname );
	if( cp )
	{
		storeobjprop( msg, cp->obj );
		return( TRUE );
	}
	return( 0 );
}

#ifndef MBX
void STDARGS js_snoop( STRPTR txt, ... )
{
}

void js_opensnoop( void )
{
	if( !js_snoop_win )
	{
		js_snoop_win = Open( "CON:///300/V³ Javascript Snooper/CLOSE", MODE_NEWFILE );
	}
}
#endif
