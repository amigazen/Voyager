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
** $Id: lo_formfile.c,v 1.21 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "classes.h"
#include <proto/vimgdecode.h>
#include "prefs.h"
#include "voyager_cat.h"
#include "js.h"
#include "urlparser.h"
#include "htmlclasses.h"
#include "layout.h"
#include "fontcache.h"
#include "malloc.h"
#include "mui_func.h"
#include "form.h"
#include "textinput.h"
#include "dos_func.h"


static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	APTR formobject;
	char *name;
	char *value;
	int size;
	int id, eid;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			break;

		case MA_Layout_FormElement_Name:
			if( tag->ti_Data )
				data->name = strdup( (char*)tag->ti_Data ); /* TOFIX */
			break;

		case MA_Layout_FormElement_Value:
			if( tag->ti_Data )
			{
				set( obj, MUIA_Textinput_Contents, tag->ti_Data );
			}
			break;

		case MA_Layout_FormElement_DefaultValue:
			if( tag->ti_Data )
			{
				data->value = strdup( (char*)tag->ti_Data ); /* TOFIX */
			}
			break;

		case MA_Layout_FormElement_Form:
			data->formobject = (APTR)tag->ti_Data;
			break;

		case MA_Layout_FormText_Size:
			data->size = tag->ti_Data;
			break;

		case MA_Layout_FormElement_ID:
			data->id = tag->ti_Data;
			break;

		case MA_Layout_FormElement_EID:
			data->eid = tag->ti_Data;
			break;

	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_Popstring_Button, PopButton( MUII_PopFile ),
		MUIA_Popstring_String, TextinputObject, StringFrame, End,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->li.valign = valign_baseline;

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->name );
	free( data->value );

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_Layout_Info, &data->li );
		STOREATTR( MA_Layout_FormElement_Name, data->name );
		STOREATTR( MA_Layout_FormElement_Value, data->value );
		STOREATTR( MA_Layout_FormElement_Form, data->formobject );
	}

	return( DOSUPER );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( DOSUPER );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	struct MUIP_AskMinMax amm;

	amm.MethodID = MUIM_AskMinMax;
	DoMethodA( obj, (Msg)&amm );

	data->li.minwidth = amm.MinMaxInfo->MinWidth;
	data->li.maxwidth = amm.MinMaxInfo->MaxWidth;
	data->li.defwidth = max( amm.MinMaxInfo->DefWidth, amm.MinMaxInfo->MinWidth ); // Textinput is broken, erm
	data->li.minheight = amm.MinMaxInfo->MinHeight;

	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	data->li.ys = data->li.minheight;
	data->li.xs = data->li.minwidth;
	return( (ULONG)&data->li );
}

DECMMETHOD( AskMinMax )
{
	GETDATA;
	int addwidth;

	DOSUPER;

	addwidth = data->size * _font( obj )->tf_XSize * 2 / 3;

	msg->MinMaxInfo->MinWidth += addwidth;
	msg->MinMaxInfo->DefWidth += addwidth;

	return( 0 );
}

DECSMETHOD( Layout_FormElement_ReportValue )
{
	GETDATA;
#ifdef MBX
	DOSHandle_p f;
#else
	BPTR f;
#endif
	STRPTR buffer;
	int size;
	char *name;

	if( msg->whichform != data->formobject )
		return( 0 );

	if( getv( obj, MUIA_Disabled ) )
		return( 0 );

	name = getstrp( obj );

	// Read temp file (hm, silly)
	f = Open( name, MODE_OLDFILE );
	if( !f )
		return( 0 );

	Seek( f, 0, OFFSET_END );
	size = Seek( f, 0, OFFSET_BEGINNING );

	if( !size )
	{
		Close( f );
		return( 0 );
	}

	buffer = malloc( size );

	Read( f, buffer, size );
	Close( f );

	DoMethod( data->formobject, MM_Layout_Form_AttachValue,
		data->name, buffer, size,
		FilePart( name )
	);

	free( buffer );

	return( 0 );
}

DECTMETHOD( Layout_FormElement_Store )
{
	GETDATA;

	formstore_add(
		data->ctx->baseref,
		data->eid,
		data->id,
		getstrp( obj ),
		-1
	);

	return( 0 );
}

DECSMETHOD( Layout_FormElement_Reset )
{
	GETDATA;

	if( msg->form == data->formobject )
		nnset( obj, MUIA_Textinput_Contents, data->value );

	return( 0 );
}

BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( AskMinMax )
DEFSMETHOD( Layout_FormElement_ReportValue )
DEFTMETHOD( Layout_FormElement_Store )
DEFTMETHOD( Layout_FormElement_Reset )
ENDMTABLE

int create_loformfileclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Popasl, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loformfileClass";
#endif

	return( TRUE );
}

void delete_loformfileclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloformfileclass( void )
{
	return( lcc->mcc_Class );
}
