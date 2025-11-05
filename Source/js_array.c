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
** $Id: js_array.c,v 1.46 2004/01/06 19:50:07 neko Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "classes.h"
#include "js.h"
#include "malloc.h"
#include "mui_func.h"

struct Data {
	struct MinList *cpl;
	char *freeme;
	int in_tostring;
};

BEGINPTABLE
DPROP( length,	real )
DPROP( concat,  funcptr )
DPROP( join,	funcptr )
DPROP( pop,		funcptr )
DPROP( push,	funcptr )
DPROP( reverse,	funcptr )
DPROP( shift,	funcptr )
DPROP( slice,	funcptr )
DPROP( splice,	funcptr )
DPROP( sort,	funcptr )
DPROP( unshift,	funcptr )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Array",
		TAG_MORE, msg->ops_AttrList
	);
	if( obj )
	{
		struct Data *data = INST_DATA( cl, obj );
		get( obj, MA_JS_Object_CPL, &data->cpl );
	}
	return( (ULONG)obj );
}

DECDISPOSE
{
	GETDATA;
	if( data->freeme )
		free( data->freeme );
	return( DOSUPER );
}

static char *tostring( struct Data *data, char *sep, int *sizeptr, char **freeptr )
{
	struct customprop *cp;
	char *tmp;
	int tmp_offset = 0, tmp_rest;
	int seplen = strlen( sep );
	int totalsize = 0;

	if( data->in_tostring )
	{
		*sizeptr = 0;
		*freeptr = malloc( 1 );
		**freeptr = 0;
		return( *freeptr );
	}
	data->in_tostring = TRUE;

	for( cp = FIRSTNODE( data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
	{
		char buffer[ 128 ];
		int bfsize = sizeof( buffer );
		if( cp->obj )
		{
			if( !DoMethod( cp->obj, MM_JS_ToString, buffer, &bfsize, 0 ) )
			{
				bfsize = 9;
			}
		}
		else
			bfsize = 0;

		totalsize += bfsize + seplen;
	}

	*freeptr = tmp = malloc( totalsize + 1 );
	memset( tmp, '\0', totalsize + 1 ); /* TOFIX: maybe not necessary */
	tmp_rest = totalsize + 1;

	for( cp = FIRSTNODE( data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
	{
		int bfsize;

		strcpy( &tmp[ tmp_offset ], sep );
		tmp_offset += seplen;
		tmp_rest -= seplen;
	
		if( cp->obj )
		{
			bfsize = tmp_rest;
			if( !DoMethod( cp->obj, MM_JS_ToString, &tmp[ tmp_offset ], &bfsize, 0 ) )
			{
				strcpy( &tmp[ tmp_offset ], "(unnamed)" );
				bfsize = 9;
			}
		}
		else
			bfsize = 0;

		tmp_offset += bfsize;
		tmp_rest -= bfsize;
	}

	if( totalsize > 0 )
		*sizeptr = totalsize - seplen;
	else
		*sizeptr = 0;

	data->in_tostring = FALSE;

	return( tmp + seplen );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;
	char *tmp, *freeptr;
	int totalsize;

	tmp = tostring( data, ",", &totalsize, &freeptr );

	if( msg->tosize && *msg->tosize )
	{
		stccpy( msg->tobuffer, tmp, *msg->tosize );
	}
	else
		strcpy( msg->tobuffer, tmp );

	if( msg->tosize )
		*msg->tosize = totalsize ;

	free( freeptr );

	return( TRUE );
}

int isnum( const char *txt )
{
	while( *txt )
		if( !isdigit( *txt++ ) )
			return( FALSE );
	return( TRUE );	
}


static int getmaxindex( struct Data *data )
{
	struct customprop *cp;
	int maxix = -1;

	for( cp = FIRSTNODE( data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
	{
		if( isnum( cp->name ) )
			maxix = max( maxix, atoi( cp->name ) );
		else
			maxix++;
	}
	return( maxix );
}

DECMETHOD( JS_Array_AddMember, APTR )
{
	GETDATA;
	char name[ 32 ];

	// find topmost index

	sprintf( name, "%d", getmaxindex( data ) + 1 );
	cp_set( data->cpl, name, msg[ 1 ] );

	return( TRUE );
}

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
		case JSPID_length:
			storerealprop( msg, getmaxindex( data ) + 1.0 );
			return( TRUE );
	}

	return( DOSUPER );
}

static void relabel( struct MinList *cpl, int offset )
{
	struct customprop *cp;

	for( cp = FIRSTNODE( cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
	{
		if( isnum( cp->name ) )
		{
			sprintf( cp->name, "%d", atoi( cp->name ) + offset );
		}
	}
}

static int jso_cmpfunc( const void *v1, const void *v2 )
{
	struct customprop **o1, **o2;
	o1 = (struct customprop **)v1;
	o2 = (struct customprop **)v2;
	if( *o1 && (*o1)->obj && *o2 && (*o2)->obj )
	{
		char bf1[ 256 ], bf2[ 256 ];
		int ts;

		ts = sizeof( bf1 );
		DoMethod( (*o1)->obj, MM_JS_ToString, bf1, &ts, 0 );
		ts = sizeof( bf2 );
		DoMethod( (*o2)->obj, MM_JS_ToString, bf2, &ts, 0 );

		return( strcmp( bf1, bf2 ) );
	}
	if( !*o1 && !*o2 )
		return( 0 );
	else if( *o1 )
		return( 1 );
	else
		return( -1 );
}

DECSMETHOD( JS_CallMethod )
{
	static double rval;
	APTR dval = obj;
	GETDATA;

	switch( msg->pid )
	{
		case JSPID_concat:
			{
				APTR o;
				struct customprop *cp;
				struct MinList *l2;

				if( msg->argcnt-- < 1 )
					return( 0 );

				exprs_pop_as_object( msg->es, &o );

				l2 = 0;
				get( o, MA_JS_Object_CPL, &l2 );

				dval = JSNewObject( getjs_array(),
					TAG_DONE
				);

				for( cp = FIRSTNODE( data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
				{
					DoMethod( dval, MM_JS_Array_AddMember, cp->obj );					
				}
				if( l2 )
				{
					for( cp = FIRSTNODE( l2 ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
					{
						DoMethod( dval, MM_JS_Array_AddMember, cp->obj );					
					}
				}
			}
			break;

		case JSPID_push:
			{
				int c;
				APTR lastpush = 0;

				for( c = 0; c < msg->argcnt; c++ )
				{
					exprs_pop_as_object( msg->es, &lastpush );
					DoMethod( obj, MM_JS_Array_AddMember, lastpush );
				}
				rval = getmaxindex( data ) + 1;
				*msg->typeptr = expt_real;
				*msg->dataptr = &rval;
				msg->argcnt = 0;
				return( TRUE );
			}
			break;

		case JSPID_unshift:
			{
				int c;
				APTR o;

				for( c = 0; c < msg->argcnt; c++ )
				{
					exprs_pop_as_object( msg->es, &o );
					relabel( data->cpl, 1 );
					cp_set( data->cpl, "0", o );
					ADDHEAD( data->cpl, REMTAIL( data->cpl ) );
				}
				rval = getmaxindex( data ) + 1;
				*msg->typeptr = expt_real;
				*msg->dataptr = &rval;
				msg->argcnt = 0;
				return( TRUE );
			}
			break;

		case JSPID_join:
			{
				char sep[ 128 ], *ret;

				if( msg->argcnt-- )
					exprs_pop_as_string( msg->es, sep, sizeof( sep ) );
				else
					strcpy( sep, "," );

				if( data->freeme )
				{
					free( data->freeme );
					data->freeme = NULL;
				}
				ret = tostring( data, sep, msg->datasize, &data->freeme );

				*msg->typeptr = expt_string;
				*msg->dataptr = ret;
				return( TRUE );
			}

		case JSPID_pop:
		case JSPID_shift:
			{
				struct customprop *cp;

				if( msg->pid == JSPID_pop )
					cp = REMTAIL( data->cpl );
				else
				{
					cp = REMHEAD( data->cpl );
					relabel( data->cpl, -1 );
				}
				if( !cp )
				{
					// return "undefined"
					return( TRUE );
				}

				dval = cp->obj;
				free( cp );
			}
			break;

		case JSPID_reverse:
		case JSPID_sort:
			{
				struct customprop **tab, *cp;
				int tabsize = getmaxindex( data ) + 1;
				int lastindex = 0;
				int c;
				
				tab = malloc( 4 * tabsize );
				memset( tab, 0, 4 * tabsize );
				while( cp = REMHEAD( data->cpl ) )
				{
					if( isnum( cp->name ) )
						lastindex = atoi( cp->name );

					tab[ lastindex++ ] = cp;
				}

				if( msg->pid == JSPID_reverse )
				{
					for( c = 0; c < tabsize / 2; c++ )
					{
						cp = tab[ c ];
						tab[ c ] = tab[ tabsize - 1 - c ];
						tab[ tabsize - 1 - c ] = cp;
					}
				}
				else
				{
					qsort( tab, tabsize, 4, jso_cmpfunc );
				}

				for( c = 0; c < tabsize; c++ )
				{
					if( tab[ c ] )
					{
						char buff[ 64 ];

						sprintf( buff, "%u", c );

						cp_set( data->cpl, buff, tab[ c ]->obj );
						free( tab[ c ] );
					}
				}

				free( tab );
			}
			return( TRUE );

		default:
			return( DOSUPER );
	}

	*msg->typeptr = expt_obj;
	*msg->dataptr = dval;
	*msg->datasize = sizeof( dval );

	return( TRUE );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_Array_AddMember )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_array( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );
#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_ArrayClass";
#endif

	return( TRUE );
}

void delete_js_array( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_array( void )
{
	return( mcc->mcc_Class );
}

struct MUI_CustomClass * getjs_array_class( void )
{
	return( mcc );
}
