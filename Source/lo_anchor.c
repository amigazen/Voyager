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
** $Id: lo_anchor.c,v 1.83 2003/07/06 16:51:33 olli Exp $
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
#include "textfit.h"
#include "download.h"
#include "methodstack.h"
#include "malloc.h"
#include "js.h"
#include "mui_func.h"

static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *url;
	char *tempurl;
	char *target;
	char *title;
	char *name;
	time_t visited;
	ULONG qualifier;
	int pressed;
	int accesskeycode;
	int accesskeycode2;

	// Event handler functions
	int ix_mouseover, ix_mouseout, ix_onclick;
};

#ifdef MBX

static struct act {
	char *spec;
	int code1, code2;
} acttable[] = {
	{ "RED", 	IECODE_COLOR_RED, -1 },
	{ "BLUE",	IECODE_COLOR_BLUE, -1 },
	{ "GREEN",	IECODE_COLOR_GREEN, -1 },
	{ "YELLOW",	IECODE_COLOR_YELLOW, -1 },
	{ "0",		IECODE_RMTNUM_0, 0x4a },
	{ "1",		IECODE_RMTNUM_1, 0x41 },
	{ "2",		IECODE_RMTNUM_2, 0x42 },
	{ "3",		IECODE_RMTNUM_3, 0x43 },
	{ "4",		IECODE_RMTNUM_4, 0x44 },
	{ "5",		IECODE_RMTNUM_5, 0x45 },
	{ "6",		IECODE_RMTNUM_6, 0x46 },
	{ "7",		IECODE_RMTNUM_7, 0x47 },
	{ "8",		IECODE_RMTNUM_8, 0x48 },
	{ "9",		IECODE_RMTNUM_9, 0x49 },
	{ "SC_IN",	0x2ce, -1 },
	{ "SC_OUT",	0x2cd, -1 },
	{ 0, 0, 0 }	
};

static void setaccesskey( struct Data *data, char *keyspec )
{
	struct act *act = acttable;

	strupr( keyspec );

	while( act->spec )
	{
		if( !strcmp( keyspec, act->spec ) )
		{
			data->accesskeycode = act->code1;
			data->accesskeycode2 = act->code2;
			return;
		}
		act++;
	}
}
#else
static void setaccesskey( struct Data *data, char *keyspec )
{

}
#endif

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Context:
			data->ctx = (APTR)tag->ti_Data;
			break;

		case MA_Layout_Anchor_URL:
			l_readstrtag( tag, &data->url );
			break;

		case MA_Layout_Anchor_TempURL:
			l_readstrtag( tag, &data->tempurl );
			break;

		case MA_Layout_Anchor_Title:
			l_readstrtag( tag, &data->title );
			break;

		case MA_Layout_Anchor_Name:
			l_readstrtag( tag, &data->name );
			break;

		case MA_Layout_Anchor_Target:
			l_readstrtag( tag, &data->target );
			break;

		case MA_Layout_Anchor_Visited:
			data->visited = (time_t)tag->ti_Data;
			break;

		case (int)MUIA_Selected:
			DoMethod( _parent( obj ), MM_Layout_Group_HighliteAnchor, obj, tag->ti_Data );
			break;

		case (int)MUIA_Pressed:
			if( !tag->ti_Data && data->pressed )
			{
				int mx = 0, my = 0;

				if( _window( obj ) )
				{
#ifdef MBX
					mx = _window( obj )->ww_MouseX;
					my = _window( obj )->ww_MouseY;
#else
					mx = _window( obj )->MouseX;
					my = _window( obj )->MouseY;
#endif
				}
	
				// Link was selected

				if( DoMethod( obj, MM_Layout_Anchor_HandleMouseEvent, 0, mx, my ) )
				{
					data->pressed = FALSE;
					return( 0 );
				}

#if USE_STB_NAV
				if( data->qualifier & ( IEQUALIFIER_LSHIFT | IEQUALIFIERF_RMTSHIFT ) )
				{
					pushmethod( data->ctx->dom_win, 5,
						MM_HTMLWin_SetURL, 
						data->tempurl ? data->tempurl : data->url, 
						data->ctx->baseref, 
						"_blank",
						MF_HTMLWin_AddURL
					);
#else
				if( data->qualifier & ( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT ) )
				{
#if USE_NET
					queue_download( data->tempurl ? data->tempurl : data->url, data->ctx->baseref, TRUE, FALSE );
#endif /* USE_NET */
#endif
				}
				else
				{
					pushmethod( data->ctx->dom_win, 5,
						MM_HTMLWin_SetURL, 
						data->tempurl ? data->tempurl : data->url, 
						data->ctx->baseref, 
						data->qualifier & ( IEQUALIFIER_RALT | IEQUALIFIER_LALT ) ? "_blank" : data->target, 
						MF_HTMLWin_AddURL
					);
				}
				data->pressed = FALSE;
			}
			else if( tag->ti_Data )
			{
				data->pressed = TRUE;
			}
			break;

		case MA_Layout_Anchor_Qualifier:
			data->qualifier = tag->ti_Data;
			break;

		case MA_Layout_Anchor_AccessKey:
			setaccesskey( data, (char*)tag->ti_Data );
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
		MUIA_CycleChain, 1,
		MUIA_InputMode, MUIV_InputMode_None,
		//MUIA_Draggable, TRUE,
		MA_JS_ClassName, "Link",
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	_flags( obj ) |= MADF_KNOWSACTIVE;

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	killpushedmethods( obj );

	free( data->url );
	free( data->target );
	free( data->title );
	free( data->name );
	free( data->tempurl );

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

		case MA_Layout_Anchor_URL:
			*msg->opg_Storage = (ULONG)data->url;
			return( TRUE );

		case MA_Layout_Anchor_Target:
			*msg->opg_Storage = (ULONG)data->target;
			return( TRUE );

		case MA_Layout_Anchor_Name:
			*msg->opg_Storage = (ULONG)data->name;
			return( TRUE );

		case MA_Layout_Anchor_Title:
			*msg->opg_Storage = (ULONG)data->title;
			return( TRUE );
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
	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	return( (ULONG)&data->li );
}

DECMMETHOD( GoActive )
{
	DoMethod( _parent( obj ), MM_Layout_Group_ActiveAnchor, obj, TRUE );
	return( 0 );
}

DECMMETHOD( GoInactive )
{
	DoMethod( _parent( obj ), MM_Layout_Group_ActiveAnchor, obj, FALSE );
	return( 0 );
}

#ifdef SHOWHIDE

DECMMETHOD( Show )
{
	GETDATA;
	kprintf( "SHOW(%08lx,%s) %ld\n", obj, data->url, _flags( obj ) & MADF_CYCLECHAIN );
	return( TRUE );
}

DECMMETHOD( Hide )
{
	GETDATA;
	kprintf( "HIDE(%08lx,%s)\n", obj, data->url );
	return( DOSUPER );
}
#endif

DECMMETHOD( Draw )
{
	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

DECSMETHOD( Layout_Anchor_HandleAccessKey )
{
	GETDATA; 
	if( msg->iecode ) 
	{
		int code = msg->iecode & ~IECODE_UP_PREFIX; 
		int isup = msg->iecode & IECODE_UP_PREFIX;

		if( code == data->accesskeycode || code == data->accesskeycode2 )
		{
			set( obj, MUIA_Pressed, !isup );
		}	
	}
	return( 0 );
}

BEGINPTABLE
DPROP( onclick,  	funcptr )
DPROP( onmouseover,	funcptr )
DPROP( onmouseout,	funcptr )
DPROP( href,		string )
ENDPTABLE

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DECSMETHOD( JS_GetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_href:
			storestrprop( msg, data->url );
			return( TRUE );
	}
	return( DOSUPER );
}

DECSMETHOD( JS_SetProperty )
{
	GETDATA;

	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_onclick:
			data->ix_onclick = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onmouseover:
			data->ix_mouseover = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onmouseout:
			data->ix_mouseout = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_href:
			{
				free( data->url );
				if( msg->dataptr )
				{
					data->url = strdup( (char*)msg->dataptr ); /* TOFIX */
				}
				else
				{
					data->url = strdup( "" ); /* TOFIX */
				}
			}
			return( TRUE );
	}

	return( DOSUPER );
}

DS_LISTPROP

DECSMETHOD( Layout_Anchor_HandleMouseEvent )
{
	GETDATA;
	int type, ix;

	switch( msg->type )
	{
		case 0: // click
			ix = data->ix_onclick;
			type = jse_click;
			break;

		case 1: // mouseover
			ix = data->ix_mouseover;
			type = jse_mouseover;
			break;

		case 2: // mouseout
			ix = data->ix_mouseout;
			type = jse_mouseout;
			break;

		default:
			return( 0 );
	}

	return DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, type, ix, obj,
		TAG_DONE
	);
}

DECSMETHOD( Layout_Anchor_FindByName )
{
	GETDATA;

	if( data->name )
	{
		char *myname = data->name;

		while( *myname == '#' )
			myname++;

		if( !stricmp( myname, msg->name ) )
		{
			*msg->ypos = data->li.yp;
			return( TRUE );
		}
	}

	return( FALSE );
}


BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( GoActive )
DEFMMETHOD( GoInactive )
DEFMMETHOD( AskMinMax )
#ifdef SHOWHIDE
DEFMMETHOD( Show )
DEFMMETHOD( Hide )
#endif
DEFMMETHOD( Draw )
DEFSMETHOD( Layout_Anchor_HandleAccessKey )
DEFSMETHOD( Layout_Anchor_FindByName )
DEFSMETHOD( Layout_Anchor_HandleMouseEvent )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

int create_loanchorclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loanchorClass";
#endif

	return( TRUE );
}

void delete_loanchorclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloanchorclass( void )
{
	return( lcc->mcc_Class );
}
