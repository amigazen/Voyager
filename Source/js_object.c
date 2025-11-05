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
** $Id: js_object.c,v 1.52 2003/07/06 16:51:33 olli Exp $
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

/* Hacky string to make "listjs.c" happy: MA_JS_ClassName, "Object" */

struct Data {
	char *name;
	char *id;
	char *classname;
	char *baseref;
	int funccontext;
	UBYTE terse;
	UBYTE undef;
	UBYTE pad[ 2 ];
	struct MinList cpl;
	ULONG gcmagic;
};

// Function for wrapping NewObject and
APTR STDARGS JSNewObject( APTR class, ... )
{
	va_list va;
	APTR o;

	va_start( va, class );
#ifdef __MORPHOS__
	o = NewObjectA( class, NULL, (struct TagItem *)va->overflow_arg_area );
#else
	o = NewObjectA( class, NULL, (struct TagItem *)va );
#endif /* !__MORPHOS__ */
	js_gc_add( o );
	va_end( va );
	return( o );
}

// Functions for handling custom properties (sigh, JS *SUCKS*)

struct customprop *cp_find( struct MinList *cpl, STRPTR name )
{
	struct customprop *cp;

	for( cp = FIRSTNODE( cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
		if( !strcmp( name, cp->name ) )
			return( cp );

	return( NULL );
}

void cp_del( struct MinList *cpl, STRPTR name )
{
	struct customprop *cp;

	cp = cp_find( cpl, name );
	if( cp )
	{
		REMOVE( cp );
		free( cp );
	}
}

void cp_set( struct MinList *cpl, STRPTR name, APTR obj )
{
	struct customprop *cp;

	cp_del( cpl, name );
	
	cp = malloc( sizeof( *cp ) + 7 + strlen( name ) );
	if( cp )
	{
		strcpy( cp->name, name );
		cp->obj = obj;
		ADDTAIL( cpl, cp );
	}
}

/* Add a Property as a String Object */
void cp_setstr( struct MinList *cpl, STRPTR name, STRPTR str, int strsize )
{
	APTR strobj;

	if( strsize < 0 )
		strsize = strlen( str );
	
	strobj = JSNewObject( getjs_string(),
		MA_JS_Name, name,
		TAG_DONE
	);
	if( strobj )
	{
		DoMethod( strobj, MM_JS_SetData, str, strsize );
		cp_set( cpl, name, strobj );
	}
}

/* Add a Property as a Number Object */
void cp_setreal( struct MinList *cpl, STRPTR name, double val )
{
	APTR realobj;

	realobj = JSNewObject( getjs_real(),
		MA_JS_Name, name,
		TAG_DONE
	);
	if( realobj )
	{
		DoMethod( realobj, MM_JS_SetData, &val /*, sizeof( val ) */ );
		cp_set( cpl, name, realobj );
	}
}


// Generell implementation of "ToString" method
// (for objects which don't inherit js_object, grrr)

void js_tostring( char *classname, struct MP_JS_ToString *msg )
{
	char to[ 128 ];

	sprintf( to, "[object %s]", classname ? classname : "[UNKNOWN]" );

	if( msg->tosize && *msg->tosize )
	{
		stccpy( msg->tobuffer, to, *msg->tosize );
	}
	else
		strcpy( msg->tobuffer, to );

	if( msg->tosize )
		*msg->tosize = strlen( to );
}

void js_lp_addcplist( struct MP_JS_ListProperties *msg, struct MinList *cpl )
{
	struct proplistentry *pe;
	struct customprop *cp;

	for( cp = FIRSTNODE( cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
	{
		pe = AllocPooled( msg->pool, sizeof( *pe ) + strlen( cp->name ) + 1 );
		strcpy( pe->name, cp->name );
		ADDTAIL( msg->l, pe );
	}
}

void js_lp_addptable( struct MP_JS_ListProperties *msg, struct propt *pt )
{
	struct proplistentry *pe;

	while( pt->name )
	{
		// Hack!
		if( pt->type == expt_funcptr )
		{
			if( pt->name[ 0 ] != 'o' || pt->name[ 1 ] != 'n' )
			{
				pt++;
				continue;
			}
		}

		pe = AllocPooled( msg->pool, sizeof( *pe ) + strlen( pt->name ) + 1 );
		strcpy( pe->name, pt->name );
		ADDTAIL( msg->l, pe );
		pt++;
	}
}

void cp_flush( struct MinList *cpl )
{
	struct customprop *cp;

	while( cp = REMHEAD( cpl ) )
	{
		free( cp );
	}
}

void cp_cleanup( struct MinList *cpl )
{
	struct customprop *cp;

	while( cp = REMHEAD( cpl ) )
		free( cp );
}

BEGINPTABLE
DPROP( valueOf, 	funcptr )
ENDPTABLE

static void setattrs( struct IClass *cl, APTR obj, struct TagItem *tagp )
{
	struct TagItem *tag;
	GETDATA;

	while( tag = NextTagItem( &tagp ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_Name:
			if( data->name )
			{
				free( data->name );
				data->name = NULL;
			}
			if( tag->ti_Data )
				data->name = strdup( (STRPTR)tag->ti_Data ); /* TOFIX */
			break;

		case MA_JS_ID:
			if( data->id )
			{
				free( data->id );
				data->id = NULL;
			}
			if( tag->ti_Data )
				data->id = strdup( (STRPTR)tag->ti_Data ); /* TOFIX */
			break;

		case MA_JS_ClassName:
			data->classname = (STRPTR)tag->ti_Data;
			break;

		case MA_JS_FuncContext:
			data->funccontext = tag->ti_Data;
			break;

		case MA_JS_Object_TerseArray:
			data->terse = tag->ti_Data ? TRUE : FALSE;
			break;

		case MA_JS_IsUndefined:
			data->undef = tag->ti_Data ? TRUE : FALSE;
			break;

		case MA_JS_Object_Baseref:
			data->baseref = (APTR)tag->ti_Data;
			break;
	}
}

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
		TAG_MORE, msg->ops_AttrList
	);

	data = INST_DATA( cl, obj );
	data->classname = "Object";
	data->gcmagic = (ULONG)-1;

	NEWLIST( &data->cpl );

	_flags( obj ) |= MADF_KNOWSACTIVE;

	setattrs( cl, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;
	cp_cleanup( &data->cpl );
	if( data->name )
		free( data->name );
	if( data->id )
		free( data->id );
	return( DOSUPER );
}

DECSET
{
	setattrs( cl, obj, msg->ops_AttrList );
	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_JS_Name:
			*msg->opg_Storage = (ULONG)data->name;
			return( TRUE );

		case MA_JS_ID:
			*msg->opg_Storage = (ULONG)data->id;
			return( TRUE );

		case MA_JS_ClassName:
			*msg->opg_Storage = (ULONG)data->classname;
			return( TRUE );

		case MA_JS_FuncContext:
			*msg->opg_Storage = (ULONG)data->funccontext;
			return( TRUE );

		case MA_JS_Object_CPL:
			*msg->opg_Storage = (ULONG)&data->cpl;
			return( TRUE );			

		case MA_JS_Object_Baseref:
			*msg->opg_Storage = (ULONG)data->baseref;
			return( TRUE );			

		case MA_JS_IsUndefined:
			*msg->opg_Storage = (ULONG)data->undef;
			return( TRUE );			
	}

	return( DOSUPER );
}

DECMETHOD( JS_NameIs, STRPTR )
{
	GETDATA;

	if( data->name )
	{
		return( (ULONG)!strcmp( data->name, msg[ 1 ] ) );
	}

	return( FALSE );
}

DECSMETHOD( JS_FindByName )
{
	GETDATA;

	if( data->name )
	{
		if( !strcmp( msg->name, data->name ) )
			return( (ULONG)obj );
	}

	return( (ULONG)NULL );
}

DECSMETHOD( JS_FindByID )
{
	GETDATA;

	if( data->id )
	{
		if( !strcmp( msg->name, data->id ) )
			return( (ULONG)obj );
	}

	return( (ULONG)NULL );
}

DECMETHOD( JS_ToReal, double* )
{
	char buffer[ 128 ];
	int bf = 128;
	char *endp;
	double v;

	DoMethod( obj, MM_JS_ToString, buffer, &bf );
	v = strtod( buffer, &endp );
	if( *endp )
	{
		return( FALSE );
	}
	*msg[ 1 ] = v;
	return( TRUE );
}

DECSMETHOD( JS_GetProperty )
{
	struct customprop *cp;
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_valueOf:
			storefuncprop( msg, -JSPID_valueOf );
			return( TRUE );
	}

	cp = cp_find( &data->cpl, msg->propname );
	if( cp )
	{
		storeobjprop( msg, cp->obj );
		return( TRUE );
	}

	if( data->terse )
	{
		if( isnum( msg->propname ) )
		{
			int v = atoi( msg->propname );
			for( cp = FIRSTNODE( &data->cpl ); v && NEXTNODE( cp ); cp = NEXTNODE( cp ) )
				v--;

			if( NEXTNODE( cp ) )
			{
				storeobjprop( msg, cp->obj );
				return( TRUE );
			}
		}
	}

	// No Superclass
	return( 0 );
}

DECSMETHOD( JS_SetProperty )
{
	GETDATA;

	// This is probably a screwup
	if( msg->type != expt_obj )
		return( FALSE );

	cp_set( &data->cpl, msg->propname, *(APTR*)msg->dataptr );

	return( TRUE );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;
	customprop *cp;
	GETDATA;
	
	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	cp = cp_find( &data->cpl, msg->propname );
	if( cp )
		return( expt_obj );

	if( data->terse )
	{
		if( isnum( msg->propname ) )
		{
			int v = atoi( msg->propname );

			for( cp = FIRSTNODE( &data->cpl ); v && NEXTNODE( cp ); cp = NEXTNODE( cp ) )
				v--;

			if( !v )
				return( TRUE );
		}
	}

	return( 0 );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;

	js_tostring( data->classname, msg );
	return( TRUE );
}

DECSMETHOD( JS_CallMethod )
{
	switch( msg->pid )
	{
		case JSPID_valueOf:
			{
				int radix;
				static char buffer[ 256 ];
				int bfsize = sizeof( buffer );

				// 0 or 1 arguments

				if( msg->argcnt-- )
					radix = exprs_pop_as_real( msg->es );
				else
					radix = 0;

				DoMethod( obj, MM_JS_ToString, buffer, &bfsize, radix );

				*msg->typeptr = expt_string;
				*msg->dataptr = buffer;
				*msg->datasize = bfsize;

				return( TRUE );
			}
			break;
	}

	// no super class
	return( 0 );
}

DECMETHOD( JS_DeleteProperty, STRPTR )
{
	GETDATA;
	cp_del( &data->cpl, msg[ 1 ] );
	return( TRUE );
}

#if 0
DECMETHOD( JS_Cleanup, APTR )
{
	GETDATA;
	cp_flush( &data->cpl );
	return( 0 );
}
#endif

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

DECSMETHOD( JS_ListProperties )
{
	GETDATA;

	js_lp_addcplist( msg, &data->cpl );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSET
DEFGET
DEFMETHOD( JS_NameIs )
DEFSMETHOD( JS_FindByName )
DEFSMETHOD( JS_FindByID )
DEFMETHOD( JS_ToReal )
DEFMETHOD( JS_ToString )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_DeleteProperty )
DEFSMETHOD( JS_ListProperties )
DEFMMETHOD( AskMinMax )
JS_GC_HOOK
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_object( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_ObjectClass";
#endif

	return( TRUE );
}

void delete_js_object( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_object( void )
{
	return( mcc->mcc_Class );
}

struct MUI_CustomClass * getjs_object_class( void )
{
	return( mcc );
}
