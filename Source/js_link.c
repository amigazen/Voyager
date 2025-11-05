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
** $Id: js_link.c,v 1.21 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "urlparser.h"
#include "malloc.h"
#include "mui_func.h"


struct Data {
	char *url;
	char *target;
	char *name;
	int	ix_onmouseover;
	int ix_onmouseout;
};

BEGINPTABLE
DPROP( href,		string )
DPROP( hash,		string )
DPROP( hostname,	string )
DPROP( host,		string )
DPROP( pathname,	string )
DPROP( port,		string )
DPROP( search,		string )
DPROP( reload,		funcptr )
DPROP( onmouseover,	funcptr )
DPROP( onmouseout,	funcptr )
ENDPTABLE

static char *parsebuffer;

DECSMETHOD( JS_GetProperty )
{
	GETDATA;
	struct parsedurl purl;
	char *url = data->url;

	if( !parsebuffer )
		parsebuffer = malloc( 2048 );
	strcpy( parsebuffer, url );
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
			storestrprop( msg, purl.path );
			return( TRUE );

		case JSPID_port:
			storerealprop( msg, (double)purl.port );
			return( TRUE );

		case JSPID_search:
			storestrprop( msg, purl.args );
			return( TRUE );
	}

	return( DOSUPER );
}

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Link",
		TAG_MORE, msg->ops_AttrList
	);
	if( obj )
	{
		struct Data *data = INST_DATA( cl, obj );
		data->url = (STRPTR)GetTagData( MA_JS_Link_URL, 0, msg->ops_AttrList );
		data->target = (STRPTR)GetTagData( MA_JS_Link_Target, 0, msg->ops_AttrList );
		data->name = (STRPTR)GetTagData( MA_JS_Link_Name, 0, msg->ops_AttrList );
	}
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetTypeData )
{
	GETDATA;

	*msg->typeptr = expt_string;
	*msg->dataptr = data->url;
	*msg->datasize = strlen( data->url );

	return( TRUE );
}

DECSMETHOD( JS_SetData )
{
	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;

	if( msg->tosize && *msg->tosize )
		memcpy( msg->tobuffer, data->url, *msg->tosize );
	else
		strcpy( msg->tobuffer, data->url );

	if( msg->tosize )
		*msg->tosize = strlen( data->url );

	return( TRUE );
}

DECSMETHOD( JS_CallMethod )
{
	switch( msg->pid )
	{
		case JSPID_reload:
			return( TRUE );
	}

	return( DOSUPER );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
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
			return( TRUE );

		case JSPID_onmouseover:
			data->ix_onmouseover = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onmouseout:
			data->ix_onmouseout = *((int*)msg->dataptr);
			return( TRUE );
	}

	return( DOSUPER );
}

DECSMETHOD( JS_HandleEvent_MouseOver )
{
	GETDATA;
	if( data->ix_onmouseover )
	{
		struct jsop_list *jso = (APTR)getv( msg->win, MA_JS_Window_JSO );
		STRPTR baseref = (STRPTR)getv( obj, MA_JS_Object_Baseref );
		//Printf( "\n*** Executing MOUSEOVER handler for %lx\n", obj );
		js_interpret( msg->win, msg->container, obj, baseref, jso, data->ix_onmouseover, NULL, 0, baseref );
	}
	return( TRUE );
}

DECSMETHOD( JS_HandleEvent_MouseOut )
{
	GETDATA;
	if( data->ix_onmouseout )
	{
		struct jsop_list *jso = (APTR)getv( msg->win, MA_JS_Window_JSO );
		STRPTR baseref = (STRPTR)getv( obj, MA_JS_Object_Baseref );
		//Printf( "\n*** Executing MOUSEOUT handler for %lx\n", obj );
		js_interpret( msg->win, msg->container, obj, baseref, jso, data->ix_onmouseout, NULL, 0, baseref );
	}
	return( TRUE );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetTypeData )
DEFSMETHOD( JS_SetData )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_HandleEvent_MouseOver )
DEFSMETHOD( JS_HandleEvent_MouseOut )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_link( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_LinkClass";
#endif

	return( TRUE );
}

void delete_js_link( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_link( void )
{
	return( mcc->mcc_Class );
}
