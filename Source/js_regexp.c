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
** $Id: js_regexp.c,v 1.23 2003/07/06 16:51:33 olli Exp $
*/

#include <limits.h>

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "malloc.h"
#include "mui_func.h"
#include "rgx_regex.h"

#define MAXRES 10

struct Data {
	char *source;
	int nocase, global, multiline;

	char *input;

	int re_compiled;
	regex_t re;
	regmatch_t res[ MAXRES ];

	struct MinList reslist; // Return list of arrays..
};

// Evil, evil hack
APTR last_regexp_object;

static void compile( struct Data *data, struct expr_stack *es )
{
	if( !data->re_compiled && data->source )
	{
		int flags = REG_EXTENDED;
		int rc;
		char *tmp, *p, *p2;

		tmp = malloc( strlen( data->source ) * 12 );
		p = data->source;
		p2 = tmp;
		while( *p )
		{
#define sete(c,s) case c:strcpy(p2,s);p2+=strlen(s);p+=2;break
			if( *p == '\\' ) switch( p[ 1 ] )
			{
				sete( 'w', "[A-Za-z0-9_]" );
				sete( 'W', "[^A-Za-z0-9_]" );
				sete( 'd', "[0-9]" );
				sete( 'D', "[^0-9]" );
				sete( 's', "[[:space:]]" );
				sete( 'S', "[^[:space:]]" );
				sete( 'b', "[[:space:]]" );
				sete( 'B', "[^[:space:]]" );
	
				sete( 'f', "\f" );
				sete( 'n', "\n" );
				sete( 'r', "\r" );

				case 'c':
					*p2++ = tolower( p[ 2 ] ) - 'a';
					p += 3;
					break;

				default:
					*p2++ = *p++;
					break;
			}
			else
				*p2++ = *p++;
		}
		*p2 = 0;

		if( data->nocase )
			flags |= REG_ICASE;
		if( data->multiline )
			flags |= REG_NEWLINE;

		rc = regcomp( &data->re, tmp, flags );

		free( tmp );

		if( rc < 0 )
		{
			regerror( rc, &data->re, es->errorbuffer, sizeof( es->errorbuffer ) );
			longjmp( *es->rb, 20 );
		}

		data->re_compiled = TRUE;
	}
}

static void setattrs( struct IClass *cl, APTR obj, struct TagItem *tagp )
{
	struct TagItem *tag;
	GETDATA;
	int free_re = FALSE;

	while( tag = NextTagItem( &tagp ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_RegExp_Source:
			if( data->source )
			{
				free( data->source );
				data->source = NULL;
			}
			if( tag->ti_Data )
			{
				data->source = strdup( (STRPTR)tag->ti_Data ); /* TOFIX */
			}
			free_re = TRUE;
			break;

		case MA_JS_RegExp_Global:
			data->global = (int)tag->ti_Data;
			free_re = TRUE;
			break;

		case MA_JS_RegExp_NoCase:
			data->nocase = (int)tag->ti_Data;
			free_re = TRUE;
			break;
	}

	if( free_re && data->re_compiled )
	{
		regfree( &data->re );
		data->re_compiled = FALSE;
	}
}

BEGINPTABLE
DPROP( global,			bool )
DPROP( ignoreCase,		bool )
DPROP( input,			string )
DPROP( lastIndex,		real )
DPROP( lastMatch,		string )
DPROP( lastParen,		string )
DPROP( leftContext,		string )
DPROP( multiline,		bool )
DPROP( rightContext,	string )
DPROP( source,			string )
DPROP( compile,			funcptr )
DPROP( exec,			funcptr )
DPROP( test,			funcptr )

// Now, The Bizarrly Named Ones
{ "$_", expt_string, JSPID_input },
{ "$&", expt_string, JSPID_lastMatch },
{ "$*", expt_bool,   JSPID_multiline },
{ "$+", expt_string, JSPID_lastParen },
{ "$`", expt_string, JSPID_leftContext },
{ "$'", expt_string, JSPID_rightContext },
{ "$0", expt_string, JSPID_re0 },
{ "$1", expt_string, JSPID_re1 },
{ "$2", expt_string, JSPID_re2 },
{ "$3", expt_string, JSPID_re3 },
{ "$4", expt_string, JSPID_re4 },
{ "$5", expt_string, JSPID_re5 },
{ "$6", expt_string, JSPID_re6 },
{ "$7", expt_string, JSPID_re7 },
{ "$8", expt_string, JSPID_re8 },
{ "$9", expt_string, JSPID_re9 },

ENDPTABLE

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	GETDATA;

	if( !pt )
		return( DOSUPER );

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}

	switch( pt->id )
	{
		case JSPID_source:
			storestrprop( msg, data->source );
			return( TRUE );

		case JSPID_input:
			storestrprop( msg, data->input );
			return( TRUE );

		case JSPID_global:
			storeintprop( msg, data->global );
			return( TRUE );

		case JSPID_ignoreCase:
			storeintprop( msg, data->nocase );
			return( TRUE );

		case JSPID_multiline:
			storeintprop( msg, data->multiline );
			return( TRUE );

		case JSPID_leftContext:
			{
				if( (int)data->res[ 0 ].rm_so >= 0 )
				{
					storestrlprop( msg, data->input, data->res[ 0 ].rm_so );
				}
				else
				{
					storestrprop( msg, NULL );
				}
			}
			return( TRUE );

		case JSPID_rightContext:
			{
				if( (int)data->res[ 0 ].rm_so >= 0 )
				{
					storestrprop( msg, &data->input[ data->res[ 0 ].rm_eo ] );
				}
				else
				{
					storestrprop( msg, NULL );
				}
			}
			return( TRUE );

		case JSPID_lastMatch:
		case JSPID_lastParen:
		case JSPID_re0:
		case JSPID_re1:
		case JSPID_re2:
		case JSPID_re3:
		case JSPID_re4:
		case JSPID_re5:
		case JSPID_re6:
		case JSPID_re7:
		case JSPID_re8:
		case JSPID_re9:
			{
				int ix = pt->id - JSPID_re0;
				regmatch_t *r;

				if( pt->id == JSPID_lastMatch )
					ix = 0;
				else if( pt->id == JSPID_lastParen )
				{
					for( ix = 1; ix < MAXRES - 1; ix++ )
					{
						r = &data->res[ ix + 1 ];
						if( (int)r->rm_so < 0 )
							break;
					}
				}

				r = &data->res[ ix ];
				if( (int)r->rm_so >= 0 )
				{
					storestrlprop( msg, &data->input[ r->rm_so ], r->rm_eo - r->rm_so );
				}
				else
				{
					storestrprop( msg, NULL );
				}
			}
			return( TRUE );
	}

	return( DOSUPER );
}

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "RegExp",
		TAG_MORE, msg->ops_AttrList
	);

	data = INST_DATA( cl, obj );

	setattrs( cl, obj, msg->ops_AttrList );

	last_regexp_object = obj;

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	if( data->re_compiled )
		regfree( &data->re );

	if( data->source )
		free( data->source );

	if( data->input )
		free( data->input );

	if( last_regexp_object == obj )
		last_regexp_object = NULL;

	return( DOSUPER );
}

DECSET
{
	setattrs( cl, obj, msg->ops_AttrList );
	return( DOSUPER );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;

	last_regexp_object = obj;

	switch( msg->pid )
	{
		case JSPID_test:
		case JSPID_exec:
			{
				char *str;
				int pop = FALSE;
				int rc;

				if( msg->argcnt-- > 0 )
				{
					str = exprs_peek_as_string( msg->es, 0 );
					if( data->input )
						free( data->input );
					if( str )
					{
						data->input = strdup( str ); /* TOFIX */
					}
					else
					{
						data->input = strdup( "" ); /* TOFIX */
					}
					pop = TRUE;
				}

				str = data->input ? data->input : "";

				compile( data, msg->es );

				rc = regexec(
					(const regex_t*)&data->re,
					(const char*)str,
					MAXRES, 
					data->res,
					0 
				);

				if( msg->pid == JSPID_exec )
				{
					*msg->typeptr = expt_obj;
					if( rc == 0 )
					{
						int c;
						struct MinList *cpl;
						APTR obj = JSNewObject( getjs_array(),
							MA_JS_Object_TerseArray, TRUE,
							TAG_DONE
						);
						
						get( obj, MA_JS_Object_CPL, &cpl );
						
						cp_setstr( cpl, "input", str, -1 );
						cp_setreal( cpl, "index", (double)((int)data->res[ 0 ].rm_so) );

						for( c = 0; c < MAXRES; c++ )
						{
							if( (int)(data->res[ c ].rm_so) >= 0 )
							{
								char name[ 32 ];
								sprintf( name, "%d", c );
								cp_setstr( cpl, name, &str[ data->res[ c ].rm_so ], data->res[ c ].rm_eo );
							}
						}

						*msg->dataptr = obj;
					}
					else
					{
						*msg->dataptr = NULL;
					}
				}
				else
				{
					*msg->typeptr = expt_bool;
					*msg->dataptr = (APTR)( rc == 0 );
				}

				if( pop )
					exprs_pop( msg->es );

				return( TRUE );
			}
			break;

		case JSPID_compile:
			{
				char *str;

				if( msg->argcnt-- < 1 )
					return( FALSE );

				str = exprs_peek_as_string( msg->es, 0 );
				set( obj, MA_JS_RegExp_Source, str );
				exprs_pop( msg->es );

				if( msg->argcnt-- > 0 )
				{
					str = exprs_peek_as_string( msg->es, 0 );
					SetAttrs( obj,
						MA_JS_RegExp_NoCase, strchr( str, 'i' ),
						MA_JS_RegExp_Global, strchr( str, 'g' ),
						TAG_DONE 
					);				
					exprs_pop( msg->es );
				}

				compile( data, msg->es );

				return( TRUE );
			}
			break;

	}

	return( DOSUPER );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	last_regexp_object = obj;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DS_LISTPROP

DECSMETHOD( JS_RegExp_Search )
{
	GETDATA;
	int rc;
	static double rval = -1;

	compile( data, msg->msg->es );

	rc = regexec(
		(const regex_t*)&data->re,
		(const char*)msg->sourcestr,
		MAXRES, 
		data->res,
		0 
	);

	*msg->msg->typeptr = expt_real;
	*msg->msg->dataptr = &rval;
	if( rc == 0 )
	{
		rval = data->res[ 0 ].rm_so;
	}
	return( TRUE );
}

DECSMETHOD( JS_RegExp_Match )
{
	GETDATA;
	int rc;

	compile( data, msg->msg->es );

	rc = regexec(
		(const regex_t*)&data->re,
		(const char*)msg->sourcestr,
		MAXRES, 
		data->res,
		0 
	);

	*msg->msg->typeptr = expt_obj;
	if( rc == 0 )
	{
		int c;
		struct MinList *cpl;
		APTR obj = JSNewObject( getjs_array(),
			MA_JS_Object_TerseArray, TRUE,
			TAG_DONE
		);
		
		get( obj, MA_JS_Object_CPL, &cpl );
		
		for( c = 0; c < MAXRES; c++ )
		{
			if( (int)(data->res[ c ].rm_so) >= 0 )
			{
				char name[ 32 ];
				sprintf( name, "%d", c );
				cp_setstr( cpl, name, &msg->sourcestr[ data->res[ c ].rm_so ], data->res[ c ].rm_eo );
			}
		}

		*msg->msg->dataptr = obj;
	}

	return( TRUE );
}

DECSMETHOD( JS_RegExp_Replace )
{
	GETDATA;
	int rc;
	char *oldinput = data->input;
	APTR nobj;

	data->input = msg->sourcestr;

	compile( data, msg->msg->es );

	rc = regexec(
		(const regex_t*)&data->re,
		(const char*)msg->sourcestr,
		MAXRES, 
		data->res,
		0 
	);

	*msg->msg->typeptr = expt_obj;
	if( rc == 0 )
	{
		char *newstr, *rep = msg->repstr;
		int d, e;
		int newlen = strlen( msg->sourcestr ) + strlen( rep ) * 2;

		// Allocate new buffer
		newstr = malloc( newlen + 2 );

{
		//int x;
		//for( x = 0; x < 5; x++ )
		//	  kprintf( "%ld: %ld/%ld\n", x, data->res[ x ].rm_so, data->res[ x ].rm_eo );
}

		for( d = 0, e = 0; msg->sourcestr[ d ]; d++ )
		{
			if( d >= (int)(data->res[ 0 ].rm_so) )
			{
//kprintf("offset %ld\n", d );
				// found a match
				// replace with replacement string
				while( *rep )
				{
					char *msgid = NULL;
					char tre[ 4 ];

//kprintf( "doing replace with %s\n", rep );

					if( *rep == '$' && rep[ 1 ] >= '1' && rep[ 1 ] <= '9' )
					{
						stccpy( tre, rep, 3 );
						rep += 2;
						msgid = tre;
					}
#define RTEST(s)  else if( !strnicmp( rep, s, strlen( s ) ) ) { msgid=s; rep += strlen(s); }
					RTEST( "lastMatch" )
					RTEST( "lastParen" )
					RTEST( "leftContext" )
					RTEST( "rightContext" )

					if( msgid )
					{
						struct MP_JS_GetProperty msg;
						int size = 0;
						char *res;

						msg.MethodID = MM_JS_GetProperty;
						msg.propname = msgid;
						msg.dataptr = (void**)&res;
						msg.datasize = &size;
						DoMethodA( obj, (Msg)&msg );

//kprintf("-> %s (len %ld)\n", res, size );

						if( size )
						{
							while( size-- )
							{
								if( e == newlen )
								{
									newlen *= 2;
									newstr = realloc( newstr, newlen + 2 );
								} 
								newstr[ e++ ] = *res++;
							}
						}
					}
					else
					{
						if( e == newlen )
						{
							newlen *= 2;
							newstr = realloc( newstr, newlen + 2 );
						} 
						newstr[ e++ ] = *rep++;
					}
				}
				d = data->res[ 0 ].rm_eo - 1;
				data->res[ 0 ].rm_so = INT_MAX;
			}
			else
			{
				if( e == newlen )
				{
					newlen *= 2;
					newstr = realloc( newstr, newlen + 1 );
				} 
				newstr[ e++ ] = msg->sourcestr[ d ];
			}
		}

		newstr[ e ] = 0;
		// Create replaced object
		nobj = JSNewObject( getjs_string(), TAG_DONE );
		DoMethod( nobj, MM_JS_SetData, newstr, strlen( newstr ) );
		*msg->msg->typeptr = expt_obj;
		*msg->msg->dataptr = nobj;

		free( newstr );
	}

	data->input = oldinput;

	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;
	char *src = data->source;

	if( !src )
		src = "";

	if( msg->tosize && *msg->tosize )
		memcpy( msg->tobuffer, src, *msg->tosize );
	else
		strcpy( msg->tobuffer, src );

	if( msg->tosize )
		*msg->tosize = strlen( src );

	return( TRUE );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSET
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_RegExp_Match )
DEFSMETHOD( JS_RegExp_Search )
DEFSMETHOD( JS_RegExp_Replace )
DEFSMETHOD( JS_ToString )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_regexp( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "js_regexpClass";
#endif

	return( TRUE );
}

void delete_js_regexp( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_regexp( void )
{
	return( mcc->mcc_Class );
}
