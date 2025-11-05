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
** $Id: lo_formcycle.c,v 1.50 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/graphics.h>
#endif

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
#include "form.h"
#include "methodstack.h"

static struct MUI_CustomClass *lcc;

struct option {
	struct MinNode n;
	char *name;
	char *value;
	int selected;
	APTR obj;
};

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	APTR formobject;
	char *name;
	int size;
	int multiple;

	APTR obj;
	int type;

	APTR lvo, to, pb;

	struct MinList options;
	char **entries;
	char **values;
	int numentries;

	int id, eid;

	int ix_onblur, ix_onchange, ix_onfocus;

	struct MinList cpl;
	ULONG gcmagic;
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

		case MA_Layout_FormElement_Form:
			data->formobject = (APTR)tag->ti_Data;
			break;

		case MA_Layout_FormCycle_Size:
			data->size = tag->ti_Data;
			break;

		case MA_Layout_FormCycle_Multiple:
			data->multiple = tag->ti_Data;
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
	APTR dummy;

	obj = DoSuperNew( cl, obj,
		Child, dummy = RectangleObject, End,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	data->obj = dummy;

	data->li.valign = valign_baseline;

	NEWLIST( &data->options );

	DOM_INITCP;

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;
	struct option *opt;

	DOM_EXITCP;

	while( opt = REMHEAD( &data->options ) )
	{
		free( opt->name );
		free( opt->value );
		free( opt );
	}

	free( data->name );
	free( data->entries );
	free( data->values );

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
		STOREATTR( MA_Layout_FormElement_Value, NULL );
		STOREATTR( MA_Layout_FormElement_Form, data->formobject );
		STOREATTR( MA_JS_ClassName, "Select" );
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

MUI_HOOK( powfunc, APTR popo, APTR win )
{
	set( win, MUIA_Window_DefaultObject, popo );
	return( 0 );
}

DECTMETHOD( Layout_FormCycle_Finish )
{
	GETDATA;
	int type;
	APTR o = 0;
	int count = 0;
	char **entries, **values;
	struct option *opt;
	int c;
	int selectnum = -1;
	int longestentrynum = -1;
	int longestentrysize = 0;
	UBYTE *storeddata;
	int storeddatasize;
#ifdef MBX
	struct RastPort *rp;

	if( rp = AllocRastPort( TAG_DONE ) )
	{
		SetFont( rp, _fontspec( obj ) );
	}
	else
	{
		//TOFIX!!
	}
#else
	struct RastPort rp;
	InitRastPort( &rp );
	SetFont( &rp, _fontspec( obj ) );
#endif /* !MBX */


	for( opt = FIRSTNODE( &data->options ); NEXTNODE( opt ); opt = NEXTNODE( opt ) )
	{
		int thissize;

#ifdef MBX
		thissize = TextLength( rp, opt->name, strlen( opt->name ) );
#else
		thissize = TextLength( &rp, opt->name, strlen( opt->name ) );
#endif /* !MBX */

		if( thissize > longestentrysize )
		{
			longestentrysize = thissize;
			longestentrynum = count;
		}

		if( opt->selected )
			selectnum = count;
		count++;
	}

	data->numentries = count;

	entries = malloc( count * 4 + 4 );
	values = malloc( count * 4 + 4 );
	c = 0;
	for( opt = FIRSTNODE( &data->options ); NEXTNODE( opt ); opt = NEXTNODE( opt ) )
	{
		entries[ c ] = opt->name;
		values[ c++ ] = opt->value ? opt->value : opt->name;
	}
	entries[ c ] = 0;
	values[ c ] = 0;

	// Here we build the actual object
	if( data->size > 1 || data->multiple )
		type = 2;
	else /* if( count > 8 ) */
		type = 1;
/*
	else
		type = 0;
*/
	data->type = type;
	data->entries = entries;
	data->values = values;

	switch( type )
	{
		case 0: // Cycle
			if( !count )
			{
				static char *dummyentry[] = { "", NULL };
				entries = dummyentry;
			}
			o = CycleObject,
				MUIA_CycleChain, 1,
				MUIA_Cycle_Entries, entries,
				MUIA_Cycle_Active, selectnum > 0 ? selectnum : 0,
				End;
			DoMethod( o, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
				obj, 1, MM_Layout_FormCycle_TriggerChange
			);
			break;

		case 1: // Popup
			o = PopobjectObject,
				MUIA_Popstring_Button, data->pb = PopButton( MUII_PopUp ),
				MUIA_Popstring_String, data->to = TextObject, TextFrame, 
					MUIA_FixWidth, longestentrysize + 8,
					MUIA_Text_SetMin, FALSE,
					MUIA_Text_SetMax, FALSE,
					MUIA_Background, MUII_TextBack, 
				End,
				MUIA_Popobject_WindowHook, &powfunc_hook,
				MUIA_Popobject_Object, data->lvo = ListviewObject, MUIA_CycleChain, 1,
					MUIA_Listview_List, ListObject, InputListFrame,
						MUIA_List_SourceArray, entries,
					End,
				End,
			End;

			if( o )
			{
				set( data->pb, MUIA_CycleChain, 1 );
				DoMethod( data->lvo, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
					obj, 1, MM_Layout_FormCycle_SelectEntry
				);
				set( data->lvo, MUIA_List_Active, selectnum > 0 ? selectnum : 0 );
				DoMethod( data->lvo, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
				   o, 2, MUIM_Popstring_Close, TRUE
				);
				DoMethod( data->lvo, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
					obj, 1, MM_Layout_FormCycle_TriggerChange
				);
			}

			break;

		case 2: // List
			o = ListviewObject, MUIA_CycleChain, 1,
					MUIA_Listview_MultiSelect, data->multiple ? MUIV_Listview_MultiSelect_Shifted : MUIV_Listview_MultiSelect_None,
					MUIA_Listview_List, ListObject, InputListFrame,
						MUIA_FixWidth, longestentrysize + 32,
						MUIA_List_SourceArray, entries,
					End,
				End;

			if( o )
			{
				c = 0;
				for( opt = FIRSTNODE( &data->options ); NEXTNODE( opt ); opt = NEXTNODE( opt ) )
				{
					if( opt->selected )
					{
						if( data->multiple )
							DoMethod( o, MUIM_List_Select, c, MUIV_List_Select_On, NULL );
						else
							set( o, MUIA_List_Active, c );
					}
					c++;
				}
				DoMethod( o, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
					obj, 1, MM_Layout_FormCycle_TriggerChange
				);
			}

			break;

	}

	// Now, set stored contents
	storeddata = formstore_get( data->ctx->baseref, data->eid, data->id, &storeddatasize );
	if( storeddata )
	{
		int c;

		for( c = 0; c < storeddatasize; c++ )
		{
			if( storeddata[ c ] )
			{
				if( type == 0 )
					set( o, MUIA_Cycle_Active, c );
				else if( type == 1 )
					DoMethod( o, MM_Layout_FormCycle_SelectEntry, c );
			}
			if( type == 2 )
			{
				if( data->multiple )
					DoMethod( o, MUIM_List_Select, c, storeddata[ c ] ? MUIV_List_Select_On : MUIV_List_Select_Off, NULL );
				else if( storeddata[ c ] )
					nnset( o, MUIA_List_Active, c );
			}
		}
	}

	if( o )
	{
		DoMethod( obj, MUIM_Group_InitChange );
		DoMethod( obj, OM_REMMEMBER, data->obj );
		DoMethod( obj, OM_ADDMEMBER, o );
		DoMethod( obj, MUIM_Group_ExitChange );

		MUI_DisposeObject( data->obj );
		data->obj = o;
	}

#ifdef MBX
	FreeRastPort( rp );
#endif /* MBX */
	
	return( 0 );
}

DECSMETHOD( Layout_FormCycle_AddOption )
{
	GETDATA;
	struct option *opt;

	opt = malloc( sizeof( *opt ) );
	if( opt )
	{
		memset( opt, 0, sizeof( *opt ) );

		if( msg->name )
		{
			opt->name = strdup( msg->name ); /* TOFIX */
		}
		else
		{
			opt->name = strdup( "" ); /* TOFIX */
		}
		if( msg->value )
			opt->value = strdup( msg->value ); /* TOFIX */
		else
			opt->value = NULL;
		opt->selected = msg->selected;

		ADDTAIL( &data->options, opt );
	}

	return( 0 );
}

DECTMETHOD( Layout_FormCycle_SelectEntry )
{
	GETDATA;
	int act;

	get( data->lvo, MUIA_List_Active, &act );
	if( act >= 0 )
		set( data->to, MUIA_Text_Contents, data->entries[ act ] );

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

	switch( data->type )
	{
		case 0:
			DoMethod( data->formobject, MM_Layout_Form_AttachValue,
				data->name, data->values[ getv( data->obj, MUIA_Cycle_Active ) ],
				-1,
				NULL
			);
			break;

		case 1:
			DoMethod( data->formobject, MM_Layout_Form_AttachValue,
				data->name, data->values[ getv( data->lvo, MUIA_List_Active ) ],
				-1,
				NULL
			);
			break;

		case 2:
			{
				int v = MUIV_List_NextSelected_Start;

				for(;;)
				{
					DoMethod( data->obj, MUIM_List_NextSelected, &v );
					if( v == MUIV_List_NextSelected_End )
						break;
					DoMethod( data->formobject, MM_Layout_Form_AttachValue,
						data->name, data->values[ v ],
						-1,
						NULL
					);
				}
			}
			break;
	}

	return( 0 );
}

DECTMETHOD( Layout_FormElement_Store )
{
	GETDATA;
	UBYTE *stmem;
	int ix;

	stmem = malloc( data->numentries );
	if( !stmem )
		return( 0 );

	memset( stmem, 0, data->numentries );

	switch( data->type )
	{
		case 0:
			ix = getv( data->obj, MUIA_Cycle_Active );
			if( ix >= 0 )
				stmem[ ix ] = TRUE;
			break;

		case 1:
			ix = getv( data->lvo, MUIA_List_Active );
			if( ix >= 0 )
				stmem[ ix ] = TRUE;
			break;

		case 2:
			{
				int v = MUIV_List_NextSelected_Start;

				for(;;)
				{
					DoMethod( data->obj, MUIM_List_NextSelected, &v );
					if( v == MUIV_List_NextSelected_End )
						break;
					stmem[ v ] = TRUE;
				}
			}
			break;
	}

	formstore_add( data->ctx->baseref, data->eid, data->id, stmem, data->numentries );
	free( stmem );
	return( 0 );
}

DECSMETHOD( Layout_FormElement_Reset )
{
	GETDATA;
	struct option *opt;
	int ix = 0;

	if( msg->form != data->formobject )
		return( 0 );

	for( opt = FIRSTNODE( &data->options ); NEXTNODE( opt ); opt = NEXTNODE( opt ) )
	{
		switch( data->type )
		{
			case 0:
				if( opt->selected )
					set( data->obj, MUIA_Cycle_Active, ix );
				break;

			case 1:
				if( opt->selected )
					DoMethod( obj, MM_Layout_FormCycle_SelectEntry, ix );
				break;

			case 2:
				DoMethod( data->obj, MUIM_List_Select, ix, opt->selected ? MUIV_List_Select_On : MUIV_List_Select_Off, NULL );
				break;
		}
		ix++;
	}

	return( 0 );
}

BEGINPTABLE
DPROP( onblur,  		funcptr )
DPROP( onfocus,			funcptr )
DPROP( onchange,		funcptr )
DPROP( name,			string )
DPROP( type,			string )
DPROP( blur,			funcptr )
DPROP( focus,			funcptr )
DPROP( options,			obj )
DPROP( form,			obj )
DPROP( selectedIndex,	real )
DPROP( length,			real )
ENDPTABLE

DECSMETHOD( JS_HasProperty ) 
{ 
	GETDATA; 
	ULONG rc;

	rc = dom_hasproperty( msg, ptable, &data->cpl );
	if( !rc )
	{
		// Try to find whether the property name is a number
		if( isnum( msg->propname ) )
		{
			int num = atoi( msg->propname );
			if( num >= 0 && num < data->numentries )
				return( expt_obj );
		}
	}
	return( rc );
}

DOM_LISTPROP

DECSMETHOD( JS_GetProperty ) 
{ 
	GETDATA; 

	ULONG rc = dom_getproperty( msg, ptable, &data->cpl ); 
	if( !rc )
	{
		if( isnum( msg->propname ) )
		{
			int num = atoi( msg->propname );
			if( num >= 0 && num < data->numentries )
			{
				struct option *o = FIRSTNODE( &data->options );
				int n = num;
				while( n-- > 0 )
					o = NEXTNODE( o );
				if( !o->obj )
				{
					o->obj = JSNewObject( getloform_optionclass(),
						MA_Layout_FormOption_Text, o->name,
						MA_Layout_FormOption_Value, o->value,
						MA_Layout_FormOption_Selected, o->selected,
						MA_Layout_FormOption_SelectObject, obj,
						MA_Layout_FormOption_Index, num,
						TAG_DONE
					);
				}
				if( o->obj )
				{
					storeobjprop( msg, o->obj );
					return( TRUE );
				}
			}
		}
	}
	else if( rc <= 1 ) 
		return( rc ); 

	switch( rc ) 
	{
		DOM_PROP( name )
			storestrprop( msg, data->name );
			return( TRUE );

		DOM_PROP( type )
			storestrprop( msg, data->multiple ? "select-multiple" : "select-one" );
			return( TRUE );

		DOM_PROP( form )
			storeobjprop( msg, data->formobject );
			return( TRUE );

		DOM_PROP( length )
			storeintprop( msg, data->numentries );
			return( TRUE );

		DOM_PROP( options )
			storeobjprop( msg, obj );
			return( TRUE );

		DOM_PROP( selectedIndex )
			switch( data->type )
			{
				case 0:
					storeintprop( msg, getv( data->obj, MUIA_Cycle_Active ) );
					return( TRUE );

				case 1:
					storeintprop( msg, getv( data->lvo, MUIA_List_Active ) );
					return( TRUE );

				case 2:
					storeintprop( msg, getv( data->obj, MUIA_List_Active ) );
					return( TRUE );
			}
			return( FALSE );
	}

	return( 0 );
}

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

		DOM_PROP( selectedIndex )
			{
				int ix = (int)TO_DOUBLE( msg->dataptr );
				switch( data->type )
				{
					case 0:
						set( data->obj, MUIA_Cycle_Active, ix );
						break;

					case 1:
						set( data->lvo, MUIA_List_Active, ix );
						break;

					case 2:
						set( data->obj, MUIA_List_Active, ix );
						break;
				}
				return( TRUE );
			}
			break;

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

	}
	return( 0 );
}

DECTMETHOD( Layout_FormCycle_TriggerChange )
{
	GETDATA;

	if( data->ix_onchange )
	{
		pushmethod( data->ctx->dom_win, 5, MM_HTMLWin_ExecuteEvent, jse_change, data->ix_onchange, obj,
			TAG_DONE
		);
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

DECSMETHOD( JS_SetGCMagic )
{
	GETDATA;

	if( data->gcmagic != msg->magic )
	{
		struct customprop *cp;
		struct option *o;

		data->gcmagic = msg->magic;

		for( cp = FIRSTNODE( &data->cpl ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
		{
			if( cp->obj )
				DoMethodA( cp->obj, (Msg)msg );
		}
		for( o = FIRSTNODE( &data->options ); NEXTNODE( o ); o = NEXTNODE( o ) )
		{
			if( o->obj )
				DoMethodA( o->obj, (Msg)msg );
		}
	}
	return( TRUE );
}

DECTMETHOD( JS_GetGCMagic )
{
	GETDATA;
	return( data->gcmagic );
}

BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFTMETHOD( Layout_FormCycle_Finish )
DEFTMETHOD( Layout_FormCycle_SelectEntry )
DEFTMETHOD( Layout_RefreshAfterIncrementalDump );
DEFSMETHOD( Layout_FormCycle_AddOption )
DEFSMETHOD( Layout_FormElement_ReportValue )
DEFTMETHOD( Layout_FormElement_Store )
DEFTMETHOD( Layout_FormElement_Reset )
DEFTMETHOD( Layout_FormCycle_TriggerChange )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_SetGCMagic )
DEFSMETHOD( JS_GetGCMagic )
DEFMMETHOD( GoActive )
DEFMMETHOD( GoInactive )
ENDMTABLE

int create_loformcycleclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loformcycleClass";
#endif

	return( TRUE );
}

void delete_loformcycleclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloformcycleclass( void )
{
	return( lcc->mcc_Class );
}
