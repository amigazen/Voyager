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


#include "config.h"

#if USE_LO_PIP

/*
**
** $Id: lo_pip.c,v 1.32 2003/07/06 16:51:33 olli Exp $
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
#include "methodstack.h"
#include "malloc.h"
#include "mui_func.h"

#if USE_STB_NAV
#include "mcp_lib_calls.h"
#include "modules/net/establish.h"
#include <modules/mbxgui/classes.h>
extern MCPBASE;
#endif

static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *widthspec;
	char *heightspec;
	int width, height;
	int height_given, width_given;
	int marginleft, marginright, marginbottom, margintop;

	struct MinList cpl;
	ULONG gcmagic;

	char *name;

	char *lastchname;
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

		case MA_Layout_Image_Width:
			l_readstrtag( tag, &data->widthspec );
			break;

		case MA_Layout_Image_Height:
			l_readstrtag( tag, &data->heightspec );
			break;

		case MA_Layout_FormElement_Name:
			if( tag->ti_Data )
			{
				if( data->name )
					free( data->name );
				data->name = strdup( (char*)tag->ti_Data ); /* TOFIX */
			}
			break;

		case MUIA_Pip_ChannelName:
			free( data->lastchname );
			if( tag->ti_Data )
				data->lastchname = strdup( (char*)tag->ti_Data ); /* TOFIX */
			else
				data->lastchname = NULL;
			break;

	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		MUIA_CustomBackfill, TRUE,
		MUIA_Font, MUIV_Font_Tiny,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->width = 20;
	data->height = 20;
	
	DOM_INITCP;

	doset( data, obj, msg->ops_AttrList );

	if( data->widthspec )
	{
		if( !strchr( data->widthspec, '%' ) )
		{
			data->width = atoi( data->widthspec );
			data->width_given = TRUE;
		}
	}
	if( data->heightspec )
	{
		if( !strchr( data->heightspec, '%' ) )
		{
			data->height = atoi( data->heightspec );
			data->height_given = TRUE;
		}
	}

	if( data->width < 1 )
		data->width = 1;
	if( data->height < 1 )
		data->height = 1;

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->widthspec );
	free( data->heightspec );

	free( data->lastchname );

	killpushedmethods( obj );

	DOM_EXITCP;

	return( DOSUPER );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( DOSUPER );
}

DECGET
{
	GETDATA;
	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_Layout_Info, &data->li );
		STOREATTR( MA_Layout_FormElement_Name, data->name );
		STOREATTR( MA_JS_Name, data->name );
	}

	return( DOSUPER );
}


DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	int iwidth, iheight;

	iwidth = data->width;
	iheight = data->height;

	// Borders...
	iwidth += data->marginleft + data->marginright;
	iheight += data->margintop + data->marginbottom;

	data->li.xs = iwidth;
	data->li.ys = iheight;

	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	int iwidth, iheight;

	iwidth = data->width;
	iheight = data->height;

	// Borders...
	iwidth += data->marginleft + data->marginright;
	iheight += data->margintop + data->marginbottom;

	data->li.minwidth = data->li.maxwidth = data->li.defwidth = iwidth;
	data->li.minheight = data->li.maxheight = data->li.defheight = iheight;

	return( (ULONG)&data->li );
}

BEGINPTABLE
DPROP( name,        	string )
DPROP( channelName,     string )
DPROP( channelId,	real )
ENDPTABLE

DOM_HASPROP

static int v;

DOM_GETPROP

		case JSPID_name:
			storestrprop( msg, data->name );
			return( TRUE );

		case JSPID_channelName:
			{
				storestrprop( msg, data->lastchname ? data->lastchname : "" );
				return( TRUE );
			}
			
		case JSPID_channelId:
			{
				storerealprop( msg, getv( obj, MUIA_Pip_ChannelId ) );
				return( TRUE );			
			}
			
DOM_ENDGETPROP


DOM_SETPROP

		case JSPID_name:
			if( data->name )
				free( data->name );
			if( msg->dataptr )
			{
				data->name = strdup( (char*)msg->dataptr ); /* TOFIX */
			}
			else
			{
				data->name = strdup( "" ); /* TOFIX */
			}
            return( TRUE );

		case JSPID_channelName:
			{
				SetAttrs( obj, 
					MUIA_Pip_ChannelName, (char*)msg->dataptr,
					MUIA_Pip_AutoTune, TRUE,
					TAG_DONE 
				);
			}
			return( TRUE );
			
		case JSPID_channelId:
			{
				SetAttrs( obj, 
					MUIA_Pip_ChannelId, (int)*((double*)msg->dataptr),
					MUIA_Pip_AutoTune, TRUE,
					TAG_DONE 
				);
			}
			return( TRUE );

DOM_ENDSETPROP

DOM_LISTPROP


DECTMETHOD( Layout_RefreshAfterIncrementalDump )
{
	MUI_Redraw( obj, MADF_DRAWOBJECT );
	return( 0 );
}

DECMMETHOD ( Pip_Tune )
{
#if USE_STB_NAV	
	ULONG autotune=0;
	get (obj, MUIA_Pip_AutoTune, &autotune);
	if (autotune) {
		ULONG channelid=0;
		get (obj, MUIA_Pip_ChannelId, &channelid);
		if (channelid>0) {
			mcp_tv_SetTVChannel (channelid);
		}
	}
	mcp_tv_EnableTV (); // enable after set to display channel osd with new channel.			
	
	return 0;
#else
	return  ( DOSUPER );
#endif
}
			
BEGINMTABLE
DEFNEW
DEFSET
DEFGET
DEFDISPOSE
DEFMMETHOD( Pip_Tune )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_ListProperties )
JS_GC_HOOK
DEFTMETHOD( Layout_RefreshAfterIncrementalDump )
ENDMTABLE


int create_lopipclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL,
	NULL, MBXGUI_GetCustomClass(MUIC_Pip), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lopipClass";
#endif

	return( TRUE );
}

void delete_lopipclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlopipclass( void )
{
	return( lcc->mcc_Class );
}

#endif
