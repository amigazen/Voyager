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
** $Id: js_string.c,v 1.36 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "malloc.h"
#include "mui_func.h"


struct Data {
	char *value;
	int dsize;
	char *tstring;
};


BEGINPTABLE
DPROP( indexOf, 	funcptr )
DPROP( lastIndexOf,	funcptr )
DPROP( charAt,	 	funcptr )
DPROP( charCodeAt, 	funcptr )
DPROP( substring,	funcptr )
DPROP( substr,		funcptr )
DPROP( length,		real )
DPROP( valueOf,		funcptr )
DPROP( toLowerCase, funcptr )
DPROP( toUpperCase, funcptr )
DPROP( fromCharCode, funcptr )
DPROP( concat,      funcptr )
DPROP( split,       funcptr )
DPROP( match,		funcptr )
DPROP( replace,		funcptr )
DPROP( search,		funcptr )
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
		case JSPID_length:
			storerealprop( msg, (double)data->dsize );
			return( TRUE );
	}

	return( DOSUPER );
}

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "String",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	if( data->dsize )
		free( data->value );

	if ( data->tstring )
		free( data->tstring );

	return( DOSUPER );
}

DECSMETHOD( JS_GetTypeData )
{
	GETDATA;

	*msg->typeptr = expt_string;
	*msg->dataptr = data->value;
	*msg->datasize = data->dsize;

	return( TRUE );
}

DECSMETHOD( JS_SetData )
{
	GETDATA;
	if( data->dsize )
		free( data->value );
	data->value = malloc( msg->datasize + 1 );
	memcpy( data->value, msg->dataptr, msg->datasize );
	data->value[ msg->datasize ] = 0;
	data->dsize = msg->datasize;
	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;

	if( msg->tosize && *msg->tosize )
		memcpy( msg->tobuffer, data->value, *msg->tosize );
	else
		memcpy( msg->tobuffer, data->value, data->dsize );

	if( msg->tosize )
		*msg->tosize = data->dsize;

	return( TRUE );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;
	static double rval;
	char *arg2 = NULL;
	ULONG rc;

	switch( msg->pid )
	{
		// RegExp support functions
		case JSPID_match:
		case JSPID_replace:
		case JSPID_search:
			{
				APTR robj;

				if( msg->argcnt-- < 1 )
					return( FALSE );

				*msg->dataptr = NULL;
				exprs_pop_as_object( msg->es, &robj );
				if( robj )
				{
					ULONG mid = 0;

					// Check whether the object is a regexp, or a string
					if( OCLASS( robj ) != getjs_regexp() )
					{
						char *tmp;
						int tosize = 1;
						char dummy[ 32 ];

						// Object seems to be a simple string
						// Create an regexp object
						DoMethod( robj, MM_JS_ToString, dummy, &tosize );
						tmp = malloc( tosize + 1 );
						if( tmp )
						{
							tosize++;
							DoMethod( robj, MM_JS_ToString, tmp, &tosize );
							tmp[ tosize ] = 0;
							robj = JSNewObject( getjs_regexp(), MA_JS_RegExp_Source, tmp, TAG_DONE );
							free( tmp );
						}
					}
					switch( msg->pid )
					{
						case JSPID_match:
							mid = MM_JS_RegExp_Match;
							break;

						case JSPID_replace:
							{
								// Peek the replace string
								if( msg->argcnt-- < 1 )
									return( FALSE );
								arg2 = exprs_peek_as_string( msg->es, 0 );
								mid = MM_JS_RegExp_Replace;
							}
							break;

						case JSPID_search:
							mid = MM_JS_RegExp_Search;
							break;
					}

					rc = DoMethod( robj, mid, data->value, msg, arg2 );
					if( arg2 )
						exprs_pop( msg->es );
					return( rc );

				}
			}
			return( FALSE );

		case JSPID_indexOf:
			{
				char *search, *p;
				int num = 0, popnum = FALSE;

				if( msg->argcnt-- < 1 )
					return( FALSE );

				search = exprs_peek_as_string( msg->es, 0 );

				if( msg->argcnt-- > 0 )
				{
					num = exprs_peek_as_real( msg->es, 1 );
					num = max( 0, num );
					num = min( data->dsize, num );
					popnum = TRUE;
				}

				p = strstr( &data->value[ num ], search );
				if( p )
					rval = p - data->value;
				else
					rval = -1;

				exprs_pop( msg->es );
				if( popnum )
					exprs_pop( msg->es );

				*msg->typeptr = expt_real;
				*msg->dataptr = &rval;

				return( TRUE );
			}
			break;

		case JSPID_lastIndexOf:
			{
				char *search, *p = 0, *p2;
				int num = 0, popnum = FALSE;

				if( msg->argcnt-- < 1 )
					return( FALSE );

				search = exprs_peek_as_string( msg->es, 0 );

				if( msg->argcnt-- > 0 )
				{
					num = exprs_peek_as_real( msg->es, 1 );
					num = max( 0, num );
					num = min( data->dsize, num );
					popnum = TRUE;
				}

				for(;;)
				{
					p2 = strstr( p ? p + 1 : &data->value[ num ], search );
					if( !p2 )
						break;
					p = p2;
				}

				if( p )
					rval = p - data->value;
				else
					rval = -1;

				exprs_pop( msg->es );
				if( popnum )
					exprs_pop( msg->es );

				*msg->typeptr = expt_real;
				*msg->dataptr = &rval;

				return( TRUE );
			}
			break;

		case JSPID_charCodeAt:
			{
				int index;

				if( msg->argcnt-- )
					index = exprs_pop_as_real( msg->es );
				else
					index = 0;

				if( index < 0 || index >= data->dsize )
					rval = -1;
				else
					rval = data->value[ index ];

				*msg->typeptr = expt_real;
				*msg->dataptr = &rval;

				return( TRUE );
			}
			break;

		case JSPID_charAt:
			{
				int index;

				// Exactly one argument

				if( msg->argcnt-- )
					index = exprs_pop_as_real( msg->es );
				else
					index = 0;

				if( index < 0 || index > data->dsize )
					index = data->dsize;

				*msg->typeptr = expt_string;
				*msg->dataptr = &data->value[ index ];
				*msg->datasize = 1;

				return( TRUE );
			}
			break;

		case JSPID_valueOf:
			{
				*msg->typeptr = expt_string;
				*msg->dataptr = data->value;
				*msg->datasize = data->dsize;

				return( TRUE );
			}
			break;

		case JSPID_substring:
		case JSPID_substr:
			{
				int ix1, ix2;

				if( msg->argcnt-- )
					ix1 = exprs_pop_as_real( msg->es );
				else
					ix1 = 0;

				if( msg->argcnt-- > 0 )
					ix2 = exprs_pop_as_real( msg->es );
				else
					ix2 = data->dsize;

				ix1 = max( 0, ix1 );
				ix1 = min( ix1, data->dsize - 1 );
				ix2 = max( 0, ix2 );

				if( msg->pid == JSPID_substring )
				{
					if( ix1 > ix2 )
					{
						int t = ix2;
						ix2 = ix1;
						ix1 = t;
					}
					ix2 -= ix1;
				}

				if( ix1 + ix2 > data->dsize )
					ix2 = data->dsize - ix1;

				*msg->typeptr = expt_string;
				*msg->dataptr = &data->value[ ix1 ];
				*msg->datasize = ix2;

				return( TRUE );
			}
			break;

		case JSPID_fromCharCode:
			{
				static char rv[ 4 ];

				if( msg->argcnt-- < 0 )
					return( FALSE );

				rv[ 0 ] = exprs_pop_as_real( msg->es );

				*msg->typeptr = expt_string;
				*msg->dataptr = rv;
				*msg->datasize = 1;

				return( TRUE );
			}
			break;

		case JSPID_toLowerCase:
			{
				if (data->tstring)    // if it's already there, we don't want it anymore
					free(data->tstring);

				data->tstring = malloc((data->dsize)+1);
				if (data->tstring)
				{
					char *s;
					
					memcpy(data->tstring, data->value, data->dsize+1);

					for (s = data->tstring; *s; s++)
						*s = tolower(*s);

					*msg->typeptr = expt_string;
					*msg->dataptr = data->tstring;
					*msg->datasize = data->dsize;

					return(TRUE);
				}
				return(FALSE);
			}
			break;

		case JSPID_toUpperCase:
			{
				if (data->tstring)    // if it's already there, we don't want it anymore
					free(data->tstring);

				data->tstring = malloc((data->dsize)+1);
				if (data->tstring)
				{
					char *s;
					
					memcpy(data->tstring, data->value, data->dsize+1);

					for (s = data->tstring; *s; s++)
						*s = toupper(*s);

					*msg->typeptr = expt_string;
					*msg->dataptr = data->tstring;
					*msg->datasize = data->dsize;

					return(TRUE);
				}
				return(FALSE);
			}
			break;

		case JSPID_concat:
			{
				int strsize, n;
				char *temp, *s, *d;

				if (msg->argcnt < 1)  // we need at least 1 arg! :)
					return(FALSE);

				if (data->tstring)    // if it's already there, we don't want it anymore
					free(data->tstring);

				strsize = data->dsize;

				// find out how big we need the string to be
				for(n=0;n<(msg->argcnt);n++)
				{
					temp = exprs_peek_as_string(msg->es, n);
					strsize += strlen(temp);
				}

				data->tstring = malloc(strsize+1);
				
				if (data->tstring)
				{
					memcpy(data->tstring, data->value, data->dsize);
					d = data->tstring + data->dsize;

					for(n=0;n<(msg->argcnt);n++)
					{
						temp = exprs_peek_as_string(msg->es, n);
						s = temp;

						while(*s != '\0')
							*d++ = *s++;
					}
					*d = '\0';
 
					for (;(msg->argcnt)>0;msg->argcnt--) exprs_pop(msg->es);

					*msg->typeptr = expt_string;
					*msg->dataptr = data->tstring;
					*msg->datasize = strsize;
					return(TRUE);
				}
				return(FALSE);
			}
			break;

		case JSPID_split:
			{
				APTR o;
				char *tmpcpy, *p;
				char *sep = "";
				int maxnum = -1;
				char sepbuff[ 128 ];
				int cnt = 0;
				char cntbuffer[ 32 ];
				struct MinList *cpl;

				if( msg->argcnt-- > 0 )
				{
					exprs_pop_as_string( msg->es, sepbuff, sizeof( sepbuff ) );
					sep = sepbuff;
				}
				if( msg->argcnt-- > 0 )
				{
					maxnum = exprs_pop_as_real( msg->es );
				}

				if( *sep == ' ' )
					sep = " \xa0\r\n\t";
				
				o = JSNewObject( getjs_array(), NULL,
						MA_JS_Object_TerseArray, TRUE,
						TAG_DONE
				);

				get( o, MA_JS_Object_CPL, &cpl );

				tmpcpy = malloc( data->dsize + 1 );
				memcpy( tmpcpy, data->value, data->dsize );
				tmpcpy[ data->dsize ] = 0;

				for( p = strtok( tmpcpy, sep ); p ; p = strtok( NULL, sep ) )
				{
					if( maxnum > 0 && cnt >= maxnum )
						break;
					sprintf( cntbuffer, "%d", cnt++ );
					cp_setstr( cpl, cntbuffer, p, -1 );
				}

				free( tmpcpy );

				*msg->typeptr = expt_obj;
				*msg->dataptr = o;
				*msg->datasize = sizeof( o );
				return(TRUE);
			}
			break;
	}

	return( DOSUPER );
}

DECSMETHOD( JS_ToBool )
{
	GETDATA;

	*msg->boolptr = data->dsize ? TRUE : FALSE;

	return( TRUE );
}

DECSMETHOD( JS_ToReal )
{
	GETDATA;
	char *endp;
	double v;

	v = strtod( data->value, &endp );
	if( *endp )
	{
		v = *double_nan;
	}

	*msg->realptr = v;

	return( TRUE );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( JS_GetTypeData )
DEFSMETHOD( JS_SetData )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_ToBool )
DEFSMETHOD( JS_ToReal )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_string( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_StringClass";
#endif

	return( TRUE );
}

void delete_js_string( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_string( void )
{
	return( mcc->mcc_Class );
}
