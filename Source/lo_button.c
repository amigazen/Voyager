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
** $Id: lo_button.c,v 1.32 2003/07/06 16:51:33 olli Exp $
**
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
#include "malloc.h"
#include "mui_func.h"
#include "methodstack.h"
#include "js.h"

static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	APTR formobject;
	char *name;
	char *value;
	int type;
	int meactive;

	struct MinList cpl;
	ULONG gcmagic;

	int ix_onclick;
	int ix_mouseover;
	int ix_mouseout;
	int ix_onfocus;
	int ix_onblur;
};

/* MA_JS_ClassName, "Button" */
static char *classtypes[] = { "Button", "Reset", "Submit" };

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
				data->value = strdup( (char*)tag->ti_Data ); /* TOFIX */
				set( obj, MUIA_Text_Contents, tag->ti_Data );
			}
			break;

		case MA_Layout_FormElement_Form:
			data->formobject = (APTR)tag->ti_Data;
			break;

		case MA_Layout_FormButton_Type:
			data->type = tag->ti_Data;
			break;

		case (int)MUIA_Pressed:
			if( !tag->ti_Data && data->ix_onclick )
			{
				if( DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, jse_click, data->ix_onclick, obj, TAG_DONE ) )
					break;
			}

			// Button pressed
			if( !tag->ti_Data && data->formobject )
			{
				if( data->type == formbutton_submit )
				{
					data->meactive = TRUE;
					DoMethod( data->formobject, MM_Layout_Form_Submit, obj );
					data->meactive = FALSE;
				}
				else if( data->type == formbutton_reset )
				{
					DoMethod( data->formobject, MM_Layout_Form_Reset );
				}
			}
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;
	int type = GetTagData( MA_Layout_FormButton_Type, 0, msg->ops_AttrList );
	char *deflabel = classtypes[ type ];

	obj = DoSuperNew( cl, obj,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_Text_Contents, deflabel,
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
		STOREATTR( MA_JS_ClassName, classtypes[ data->type ] );
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
	data->li.defwidth = amm.MinMaxInfo->DefWidth;
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

DECSMETHOD( Layout_FormElement_ReportValue )
{
	GETDATA;

	// Only report my value when I was clicked
	if( msg->whichform != data->formobject || !data->meactive )
		return( 0 );

	if( getv( obj, MUIA_Disabled ) )
		return( 0 );

	DoMethod( data->formobject, MM_Layout_Form_AttachValue,
		data->name, data->value,
		-1,
		NULL
	);

	return( 0 );
}

BEGINPTABLE
DPROP( onclick,  	funcptr )
DPROP( onfocus,  	funcptr )
DPROP( onblur,  	funcptr )
DPROP( onmouseover,	funcptr )
DPROP( onmouseout,	funcptr )
DPROP( name,		string )
DPROP( value,		string )
DPROP( type,		string )
DPROP( form,		obj )
DPROP( blur,		funcptr )
DPROP( focus,		funcptr )
ENDPTABLE

DOM_HASPROP

DOM_LISTPROP

DOM_GETPROP

		DOM_PROP( name )
			storestrprop( msg, data->name );
			return( TRUE );

		DOM_PROP( value )
			storestrprop( msg, data->value );
			return( TRUE );

		DOM_PROP( type )
			storestrprop( msg, classtypes[ data->type ] );
			return( TRUE );

		DOM_PROP( form )
			storeobjprop( msg, data->formobject );
			return( TRUE );

DOM_ENDGETPROP

DOM_SETPROP

		DOM_PROP( onclick )
			data->ix_onclick = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onfocus )
			data->ix_onfocus = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( onblur )
			data->ix_onblur = *((int*)msg->dataptr);
			return( TRUE );

		DOM_PROP( value )
			set( obj, MA_Layout_FormElement_Value, (char*)msg->dataptr );
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

		case JSPID_click:
			set( obj, MUIA_Pressed, TRUE );
			set( obj, MUIA_Pressed, FALSE );
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
DEFSMETHOD( Layout_FormElement_ReportValue )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
DOM_JS_GC_HOOK
DEFMMETHOD( GoActive )
DEFMMETHOD( GoInactive )
ENDMTABLE

int create_lobuttonclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Text, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lobuttonClass";
#endif

	return( TRUE );
}

void delete_lobuttonclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlobuttonclass( void )
{
	return( lcc->mcc_Class );
}
