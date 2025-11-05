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
** $Id: js_location.c,v 1.27 2004/01/06 19:50:07 neko Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "urlparser.h"
#include "mui_func.h"
#include "malloc.h"
#include "methodstack.h"
#include "htmlclasses.h"

struct Data {
	APTR windowobject;
	char *tempstr;
};

BEGINPTABLE
DPROP( href,		string )
DPROP( hash,		string )
DPROP( hostname,	string )
DPROP( host,		string )
DPROP( pathname,	string )
DPROP( port,		string )
DPROP( search,		string )
DPROP( protocol,	string )
DPROP( reload,		funcptr )
DPROP( replace,		funcptr )
DPROP( x,			real )
DPROP( y,			real )
ENDPTABLE

static char *parsebuffer;

static void seturl( struct Data *data, char *newurl, int add )
{
	char *url = (STRPTR)getv( data->windowobject, MA_JS_Window_URL );

	if( !data->tempstr )
		data->tempstr = malloc( MAXURLSIZE );
	uri_mergeurl( url, newurl, data->tempstr );
	pushmethod(
		data->windowobject,
		5,
		MM_HTMLWin_SetURL,
		data->tempstr,
		NULL,
		NULL,
		add ? MF_HTMLWin_AddURL : 0
	);
}

DECSMETHOD( JS_GetProperty )
{
	GETDATA;
	struct parsedurl purl;
	char *url = (STRPTR)getv( data->windowobject, MA_JS_Window_URL );

	if( !parsebuffer )
		parsebuffer = malloc( MAXURLSIZE );
	stccpy( parsebuffer, url, MAXURLSIZE );
	uri_split( parsebuffer, &purl );

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_href:
			storestrprop( msg, url );
			return( TRUE );

		case JSPID_hash:
			storestrprop( msg, purl.fragment );
			return( TRUE );

		case JSPID_hostname:
		case JSPID_host:
			storestrprop( msg, purl.host );
			return( TRUE );

		case JSPID_pathname:
			sprintf( parsebuffer, "/%s", purl.path );
			storestrprop( msg, parsebuffer );
			return( TRUE );

		case JSPID_protocol:
			{
				char buffer[ 16 ]; /* should be enough for everyone (tm) */

				snprintf( buffer, sizeof( buffer ), "%s:", purl.scheme );

				storestrprop( msg, buffer );
			}
			return( TRUE );

		case JSPID_port:
			storerealprop( msg, (double)purl.port );
			return( TRUE );

		case JSPID_search:
			{
				char buffer[ MAXURLSIZE ];

				if( purl.args )
				{
					buffer[ 0 ] = '?';
					strcpy( &buffer[ 1 ], purl.args );
					storestrprop( msg, buffer );
				}
				else
					storestrprop( msg, "" );
			}
			return( TRUE );

		case JSPID_reload:
			storefuncprop( msg, -JSPID_reload );
			return( TRUE );

		case JSPID_replace:
			storefuncprop( msg, -JSPID_replace );
			return( TRUE );
	}

	return( DOSUPER );
}

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Location",
		TAG_MORE, msg->ops_AttrList
	);
	if( obj )
	{
		struct Data *data = INST_DATA( cl, obj );
		data->windowobject = (APTR)GetTagData( MA_JS_Location_WindowObject, 0, msg->ops_AttrList );
	}
	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;
	if( data->tempstr )
		free( data->tempstr );
	return( DOSUPER );
}

DECSMETHOD( JS_GetTypeData )
{
	GETDATA;
	char *url = (STRPTR)getv( data->windowobject, MA_JS_Window_URL );

	*msg->typeptr = expt_string;
	*msg->dataptr = url;
	*msg->datasize = strlen( url );

	return( TRUE );
}

DECSMETHOD( JS_SetData )
{
	GETDATA;
	seturl( data, msg->dataptr, TRUE );
	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;
	char *url = (STRPTR)getv( data->windowobject, MA_JS_Window_URL );

	if( msg->tosize && *msg->tosize )
		memcpy( msg->tobuffer, url, *msg->tosize );
	else
		strcpy( msg->tobuffer, url );

	if( msg->tosize )
		*msg->tosize = strlen( url );

	return( TRUE );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;

	switch( msg->pid )
	{
		case JSPID_reload:
			pushmethod( data->windowobject, 1, MM_HTMLWin_Reload );
			return( TRUE );

		case JSPID_replace:
			if( msg->argcnt-- > 0 )
			{
				char buffer[ MAXURLSIZE ];

				exprs_pop_as_string( msg->es, buffer, sizeof( buffer ) );
				seturl( data, buffer, FALSE );
				return( TRUE );
			}
			return( FALSE );
	}

	return( DOSUPER );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( 0 );
}

DECSMETHOD( JS_SetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	GETDATA;

	if( !pt )
		return( FALSE );

	switch( pt->id )
	{
		case JSPID_href:
			seturl( data, msg->dataptr, TRUE );
			return( TRUE );
	}

	return( FALSE );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( JS_GetTypeData )
DEFSMETHOD( JS_SetData )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_location( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_LocationClass";
#endif

	return( TRUE );
}

void delete_js_location( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_location( void )
{
	return( mcc->mcc_Class );
}
