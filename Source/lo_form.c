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
** $Id: lo_form.c,v 1.57 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include <proto/vimgdecode.h>
#include "prefs.h"
#include "voyager_cat.h"
#include "js.h"
#include "urlparser.h"
#include "htmlclasses.h"
#include "layout.h"
#include "fontcache.h"
#include "textfit.h"
#include "network.h"
#include "malloc.h"
#include "mui_func.h"
#include "form.h"
#include "parse.h"
#include "methodstack.h"
#include "mime.h"

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *url;
	char *target;
	UWORD method;
	UWORD enctype;

	char *databuffer;
	int databuffersize;

	int did_build_list;
};

BEGINPTABLE
DPROP( name, 		string )
DPROP( action,		string )
DPROP( encoding,	string )
DPROP( method,		string )
DPROP( target,		string )
DPROP( elements,	obj )
DPROP( submit,		funcptr )
DPROP( reset,		funcptr )
ENDPTABLE

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			break;

		case MA_Layout_Form_URL:
			l_readstrtag( tag, &data->url );
			break;

		case MA_Layout_Form_Target:
			l_readstrtag( tag, &data->target );
			break;

		case MA_Layout_Form_Method:
			data->method = tag->ti_Data;
			break;

		case MA_Layout_Form_Enctype:
			data->enctype = tag->ti_Data;
			break;
	}

	return( redraw );
}

DECSET
{
	GETDATA;
	doset( data, obj, msg->ops_AttrList );
	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_Layout_Info:
			*msg->opg_Storage = (ULONG)&data->li;
			return( TRUE );

		case MA_Layout_Form_URL:
			*msg->opg_Storage = (ULONG)data->url;
			return( TRUE );

		case MA_Layout_Form_Target:
			*msg->opg_Storage = (ULONG)data->target;
			return( TRUE );
	}

	return( DOSUPER );
}

DECCONST
{
	struct Data *data;
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Form",
		MA_JS_Object_TerseArray, TRUE,
		MA_Layout_Align, align_newrow,
		TAG_MORE, msg->ops_AttrList
	);
	data = INST_DATA( cl, obj );
	doset( data, obj, msg->ops_AttrList );
	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;
	free( data->url );
	free( data->target );
	return( DOSUPER );
}

static void recbuildformarray( Object *obj, struct MinList *cpl, APTR form )
{
	APTR o,ostate;
	struct List *l;

	if( !get( obj, MUIA_Group_ChildList, &l ) )
		return;
	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		APTR cmpform = 0;
		get( o, MA_Layout_FormElement_Form, &cmpform );

		if( cmpform == form )
		{
			char *name;
			char tmpname[ 32 ];

			name = 0;
			get( o, MA_JS_Name, &name );
			if( !name || !*name )
			{
				sprintf( tmpname, "unnamed%x", (int)o );
				name = tmpname;
			}

			// Radio buttons need special handling
			// since they are coascaeled into an array
			// of their group name
			if( OCLASS( o ) == getloradioclass() )
			{
				struct customprop *cp;
				APTR arrayobject;

				// First find whether there already is an
				// array...
				cp = cp_find( cpl, name );
				if( cp )
				{
					arrayobject = cp->obj;
				}
				else
				{
					arrayobject = JSNewObject( getjs_array(),
						MA_JS_Object_TerseArray, TRUE,
						MA_JS_Name, name,
						TAG_DONE
					);
					cp_set( cpl, name, arrayobject );
				}
				if( arrayobject )
					DoMethod( arrayobject, MM_JS_Array_AddMember, o );
			}
			else
			{
				cp_set( cpl, name, o );
			}
		}
		else
			recbuildformarray( o, cpl, form ); 
	}
}

static void buildformarray( APTR rootobj, APTR form )
{
	struct MinList *cpl;

	get( form, MA_JS_Object_CPL, &cpl );
	recbuildformarray( rootobj, cpl, form );
}

DECSMETHOD( JS_GetProperty )
{
	GETDATA;

	if( !data->did_build_list )
	{
		buildformarray( data->ctx->dom_document, obj );
		//data->did_build_list = TRUE;
	}

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_name:
			storestrprop( msg, (STRPTR)getv( obj, MA_JS_Name ) );
			return( TRUE );
		case JSPID_elements:
			storeobjprop( msg, obj );
			return( TRUE );
		case JSPID_target:
			storestrprop( msg, data->target );
			return( TRUE );
		case JSPID_action:
			storestrprop( msg, data->url );
			return( TRUE );
		case JSPID_method:
			storestrprop( msg, data->method ? "POST" : "GET" );
			return( TRUE );
		case JSPID_encoding:
			storestrprop( msg, data->enctype ? "multipart/form-data" : "application/x-www-urlencoded" );
			return( TRUE );
		case JSPID_submit:
			storefuncprop( msg, -JSPID_submit );
			return( TRUE );
		case JSPID_reset:
			storefuncprop( msg, -JSPID_reset );
			return( TRUE );
	}

	return( DOSUPER );
}

DECSMETHOD( JS_SetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		DOM_PROP( target )
			free( data->target );
			if( msg->dataptr )
			{
				data->target = strdup( msg->dataptr ); /* TOFIX */
			}
			else
			{
				data->target = strdup( "" ); /* TOFIX */
			}
			return( TRUE );

		DOM_PROP( action )
			free( data->url );
			if( msg->dataptr )
			{
				data->url = strdup( msg->dataptr ); /* TOFIX */
			}
			else
			{
				data->url = strdup( "" ); /* TOFIX */
			}
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

DECSMETHOD( JS_CallMethod )
{
	switch( msg->pid )
	{
		case JSPID_submit:
			*msg->typeptr = expt_bool;
			*msg->dataptr = (void*)TRUE;
			*msg->datasize = 4;
			pushmethod( obj, 2, MM_Layout_Form_Submit, NULL );
			return( TRUE );

		case JSPID_reset:
			*msg->typeptr = expt_bool;
			*msg->dataptr = (void*)TRUE;
			*msg->datasize = 4;
			pushmethod( obj, 1, MM_Layout_Form_Reset );
			return( TRUE );
	}

	return( FALSE );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	return( (ULONG)&data->li );
}

DECTMETHOD( Layout_Form_Reset )
{
	GETDATA;

	DoMethod( data->ctx->body, MM_Layout_FormElement_Reset, obj );

	return( 0 );
}

DECSMETHOD( Layout_Form_Submit )
{
	GETDATA;
	char *tempurl;
	char *url;

	// Check whether URL is JS code. If so, don't bother to assemble parameter list
	if( !strnicmp( data->url, "javascript:", 11 ) )
	{
		DoMethod( data->ctx->dom_win, MM_HTMLWin_SetURL, data->url, data->ctx->baseref, data->target, MF_HTMLWin_AddURL | MF_HTMLWin_Reload );
		return( 0 );
	}

	// if this fails, then form submit won't happen properly. Doubt that'll ever happen.
	url = malloc( strlen(data->url) + strlen(data->ctx->baseref) + 16 ); // be safe ;)
	if (url)
	{
		// remerge partial form action URLs (like <form action="blah.html">) so that we don't
		// go to weird places like http://view.pl
		uri_mergeurl(data->ctx->baseref, data->url, url);

		data->databuffersize = 0;

		DoMethod( data->ctx->body, MM_Layout_FormElement_ReportValue, obj );

		if( data->method && !data->enctype )
		{
			// url + ? + args + null
			if ((tempurl = malloc( data->databuffersize + strlen( url ) + 2 )))
			{
				strcpy( tempurl, url );
				strcat( tempurl, "?" );
				if( data->databuffer )
					strcat( tempurl, data->databuffer );
			}
		}
		else
		{
			if ((tempurl = malloc( 40 + strlen( url ) )))
			{
				if( data->enctype )
				{
					char *temp;
					#define ENDTAG "--" MULTIPART_SEP "--\r\n"
					if ((temp = malloc( data->databuffersize + strlen( ENDTAG ) + 2 )))
					{
						memcpy( temp, data->databuffer, data->databuffersize );
						strcpy( &temp[ data->databuffersize ], ENDTAG );
						sprintf( tempurl, "%s?{%u}¿", url, formp_storedata( temp, data->databuffersize + strlen( ENDTAG ), data->enctype ) );
						free( temp );
					}
				}
				else
				{
					sprintf( tempurl, "%s?{%u}¿", url, formp_storedata( data->databuffer, data->databuffersize, data->enctype ) );
				}
			}
		}

		free( data->databuffer );
		data->databuffer = NULL;
		data->databuffersize = 0;

		if (tempurl)
		{
			DoMethod( data->ctx->dom_win, MM_HTMLWin_SetURL, tempurl, data->ctx->baseref, data->target, MF_HTMLWin_AddURL | MF_HTMLWin_Reload );
			free( tempurl );
		}

		free(url);
	}
	
	return( 0 );
}

DECSMETHOD( Layout_Form_AttachValue )
{
	GETDATA;
	int thislen = msg->valuesize;
	int newsize;
	char *newdata, *newptr;

	if( !msg->name || !msg->value )
		return( 0 );

	if( thislen < 0 )
		thislen = strlen( msg->value );

	if( !data->enctype && msg->filename )
		return( 0 );

	if( data->enctype )
		thislen += 128;

	thislen += strlen( msg->name );
	thislen += 3;
	thislen *= 3;

	newsize = data->databuffersize + thislen;
	newdata = malloc( newsize );

	memcpy( newdata, data->databuffer, data->databuffersize );
	newdata[ data->databuffersize ] = 0;
	newptr = newdata + data->databuffersize;

	if( data->enctype )
	{
		if( msg->filename )
		{
			/*
			 * Try to find the mimetype.
			 */
			char mimebuf[ 128 ];
			STRPTR mimeptr;

			if ( mime_findbyextension( msg->filename, NULL, NULL, NULL, mimebuf ) )
			{
				mimeptr = mimebuf;
			}
			else
			{
				mimeptr = "application/octet-stream";
			}

			sprintf( newptr, "--" MULTIPART_SEP "\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\n\r\n",
				msg->name,
				msg->filename,
				mimeptr
			);

			data->databuffersize += strlen( newptr );
			newptr += strlen( newptr );
			memcpy( newptr, msg->value, msg->valuesize );
			newptr += msg->valuesize;
			*newptr++ = '\r';
			*newptr++ = '\n';
			data->databuffersize += 2 + msg->valuesize;
		}
		else
		{
			sprintf( newptr, "--" MULTIPART_SEP "\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
				msg->name,
				msg->value
			);
			data->databuffersize += strlen( newptr );
		}
	}
	else
	{
		if( data->databuffersize )
			strcat( newptr, "&" );
		if( strcmp( msg->name, "\001ISINDEX\001" ) )
		{
			encodedata( msg->name, strchr( newptr, 0 ) );
			strcat( newptr, "=" );
		}
		encodedata( msg->value, strchr( newptr, 0 ) );
		data->databuffersize += strlen( newptr );
	}

	free( data->databuffer );
	data->databuffer = newdata;

	return( 0 );
}

DECTMETHOD( Layout_Form_CheckSubmit )
{
	pushmethod( obj, 2, MM_Layout_Form_Submit, NULL );
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSET
DEFGET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_Form_Submit )
DEFSMETHOD( Layout_Form_Reset )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( Layout_Form_AttachValue )
DEFTMETHOD( Layout_Form_CheckSubmit )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_lo_form( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_array_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "loformClass";
#endif

	return( TRUE );
}

void delete_lo_form( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getloformclass( void )
{
	return( mcc->mcc_Class );
}
