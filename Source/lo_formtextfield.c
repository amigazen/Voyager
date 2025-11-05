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
** $Id: lo_formtextfield.c,v 1.22 2003/07/06 16:51:33 olli Exp $
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
#include "methodstack.h"
#include "textinput.h"


static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	APTR formobject;
	char *name;
	char *value;
	int rows, cols;
	int id, eid;
	struct MinList cpl;
	ULONG gcmagic;

	int ix_onblur, ix_onfocus, ix_onchange, ix_onselect;
	int ix_onkeydown, ix_onkeypress, ix_onkeyup;
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

		case MA_Layout_FormTextarea_Rows:
			data->rows = tag->ti_Data;
			break;

		case MA_Layout_FormTextarea_Cols:
			data->cols = tag->ti_Data;
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
		StringFrame,
		MUIA_Background, MUII_TextBack,
		MUIA_Font, MUIV_Font_Fixed,
		MUIA_Textinput_Multiline, TRUE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->li.valign = valign_baseline;

	DOM_INITCP;

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->name );
	free( data->value );

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
		STOREATTR( MA_Layout_FormElement_Value, data->value );
		STOREATTR( MA_Layout_FormElement_Form, data->formobject );
		STOREATTR( MA_JS_ClassName, "Textarea" );
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
	int addheight;

	DOSUPER;
	
	addwidth = ( data->cols - 3 ) * _font( obj )->tf_XSize;
	addheight = data->rows * _font( obj )->tf_YSize;

	addwidth = max( 0, addwidth );

	msg->MinMaxInfo->MinWidth += addwidth;
	msg->MinMaxInfo->DefWidth += addwidth;

	msg->MinMaxInfo->MinHeight += addheight;
	msg->MinMaxInfo->DefHeight += addheight;

	return( 0 );
}

DECTMETHOD( Layout_RefreshAfterIncrementalDump )
{
	MUI_Redraw( obj, MADF_DRAWOBJECT );
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

BEGINPTABLE
DPROP( onblur,  	funcptr )
DPROP( onfocus,		funcptr )
DPROP( onchange,	funcptr )
DPROP( onselect,	funcptr )
DPROP( onkeyup,		funcptr )
DPROP( onkeydown,	funcptr )
DPROP( onkeypress,	funcptr )
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
			storestrprop( msg, "textarea" );
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

		DOM_PROP( onkeydown )
			data->ix_onkeydown = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onkeyup )
			data->ix_onkeyup = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onkeypress )
			data->ix_onkeydown = *((int*)msg->dataptr);
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
DEFTMETHOD( Layout_RefreshAfterIncrementalDump );
DEFSMETHOD( Layout_FormElement_ReportValue )
DEFTMETHOD( Layout_FormElement_Store )
DEFTMETHOD( Layout_FormElement_Reset )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
DOM_JS_GC_HOOK
DEFMMETHOD( GoActive )
DEFMMETHOD( GoInactive )
ENDMTABLE

int create_loformtextfieldclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Textinputscroll, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loformtextfieldClass";
#endif

	return( TRUE );
}

void delete_loformtextfieldclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloformtextfieldclass( void )
{
	return( lcc->mcc_Class );
}
