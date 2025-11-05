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
** $Id: lo_formtext.c,v 1.37 2003/07/06 16:51:33 olli Exp $
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
#include "mui_func.h"
#include "malloc.h"
#include "time_func.h"
#include "form.h"
#include "methodstack.h"
#include "textinput.h"


static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	APTR formobject;
	char *name;
	char *domid;
	char *value;
	int size;
	int id, eid;
	int secret;

	struct MinList cpl;
	ULONG gcmagic;

	int ix_onblur, ix_onfocus, ix_onchange, ix_onselect;
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

		case MA_Layout_FormElement_DOMID:
			if( tag->ti_Data )
				data->domid = strdup( (char*)tag->ti_Data ); /* TOFIX */
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

		case MUIA_Textinput_Secret:
			data->secret = tag->ti_Data;
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		StringFrame,
		MUIA_Background, MUII_TextBack,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	DOM_INITCP;

	data->li.valign = valign_baseline;

	doset( data, obj, msg->ops_AttrList );

	DoMethod( obj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		data->formobject, 1, MM_Layout_Form_CheckSubmit
	);

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->name );
	free( data->value );
	free( data->domid );

	DOM_EXITCP;

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_Layout_Info, &data->li );

		case MA_JS_Name:
		STOREATTR( MA_Layout_FormElement_Name, data->name );

		case MA_JS_ID:
		STOREATTR( MA_Layout_FormElement_DOMID, data->domid );

		STOREATTR( MA_Layout_FormElement_DefaultValue, data->value );
		STOREATTR( MA_Layout_FormElement_Form, data->formobject );

		case MA_Layout_FormElement_Value:
			return( DoMethod( obj, OM_GET, MUIA_Textinput_Contents, msg->opg_Storage ) );

		STOREATTR( MA_JS_ClassName, data->secret ? "Password" : "Text" );
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

	if( msg->whichform != data->formobject )
		return( 0 );

	if( getv( obj, MUIA_Disabled ) )
		return( 0 );

	DoMethod( data->formobject, MM_Layout_Form_AttachValue,
		data->name, getstrp( obj ),
		-1,
		NULL
	);

	return( 0 );
}

DECTMETHOD( Layout_FormElement_Store )
{
	GETDATA;

	if( !data->secret )
	{
		formstore_add( 
			data->ctx->baseref,
			data->eid,
			data->id,
			getstrp( obj ),
			-1
		);
	}

	return( 0 );
}

DECSMETHOD( Layout_FormElement_Reset )
{
	GETDATA;

	if( msg->form == data->formobject )
		nnset( obj, MUIA_Textinput_Contents, data->value );

	return( 0 );
}

BEGINPTABLE
DPROP( onblur,  	funcptr )
DPROP( onfocus,		funcptr )
DPROP( onchange,	funcptr )
DPROP( onselect,	funcptr )
DPROP( name,		string )
DPROP( value,		string )
DPROP( type,		string )
DPROP( defaultValue, string )
DPROP( blur,		funcptr )
DPROP( focus,		funcptr )
DPROP( select,		funcptr )
DPROP( form,		obj )
ENDPTABLE

DOM_HASPROP

DOM_LISTPROP

DOM_GETPROP

		DOM_PROP( name )
			storestrprop( msg, data->name );
			return( TRUE );

		DOM_PROP( defaultValue )
			storestrprop( msg, data->value );
			return( TRUE );

		DOM_PROP( type )
			storestrprop( msg, data->secret ? "Password" : "Text" );
			return( TRUE );

		DOM_PROP( value )
			storestrprop( msg, (char*)getv( obj, MUIA_Textinput_Contents ) );
			return( TRUE );

		DOM_PROP( form )
			storeobjprop( msg, data->formobject );
			return( TRUE );

DOM_ENDGETPROP

DOM_SETPROP

		DOM_PROP( onblur )
			data->ix_onblur = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onfocus )
			data->ix_onfocus = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onchange )
			data->ix_onchange = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onselect )
			data->ix_onselect = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( value )
			set( obj, MUIA_Textinput_Contents, msg->dataptr );
			return( TRUE );

DOM_ENDSETPROP

DECSMETHOD( JS_CallMethod )
{
	switch( msg->pid )
	{
		case JSPID_blur:
			pushmethod( _win( obj ), 3, MUIM_Set, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Next );
			return( TRUE );

		case JSPID_focus:
			pushmethod( _win( obj ), 3, MUIM_Set, MUIA_Window_ActiveObject, obj );
			return( TRUE );

		case JSPID_select:
			SetAttrs( obj,
				MUIA_Textinput_MarkStart, 0,
				MUIA_Textinput_MarkEnd, strlen( (char*)getv( obj, MUIA_Textinput_Contents ) ),
				TAG_DONE
			);
			return( TRUE );

	}
	return( 0 );
}

DECMMETHOD( GoActive )
{
	GETDATA;

	if( data->ix_onfocus )
	{
		DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, jse_focus, data->ix_onfocus, obj,
			TAG_DONE
		);
	}

	return( DOSUPER );
}

DECMMETHOD( GoInactive )
{
	GETDATA;

	if( data->ix_onblur )
	{
		DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, jse_blur, data->ix_onblur, obj,
			TAG_DONE
		);
	}

	return( DOSUPER );
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
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_CallMethod )
DOM_JS_GC_HOOK
DEFMMETHOD( GoActive )
DEFMMETHOD( GoInactive )
ENDMTABLE

int create_loformtextclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Textinput, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loformtextClass";
#endif

	return( TRUE );
}

void delete_loformtextclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloformtextclass( void )
{
	return( lcc->mcc_Class );
}
