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
** $Id: lo_embed.c,v 1.25 2003/07/06 16:51:33 olli Exp $
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
#ifndef MBX
#include "bitmapclone.h"
#endif
#include "layout.h"
#include "methodstack.h"
#include "malloc.h"
#include "mui_func.h"
#include "menus.h"
#include "network.h"
#include "lo_image.h"
#if USE_PLUGINS
#include <proto/v_plugin.h>
#include "plugins.h"
#endif


static struct MUI_CustomClass *lcc;

//
// FIXME! - display plugin info?
// FIXME! - margins/border?
//

//
// Maximum number of arguments passed to plugin
//
#define MAXARGS 32

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *url;
	char *widthspec;
	char *heightspec;
	char *type;
	char *name;
	int border;

	int percentwidth, percentheight;
	int width, height;
	int marginleft, marginright, marginbottom, margintop;
	int mleft, mtop, mbottom, mright;

	// Plugin stuff
	struct nstream *nethandle;
	int arg_cnt;
	STRPTR arg_names[ MAXARGS ];
	STRPTR arg_values[ MAXARGS ];
	APTR clientobject;

	UBYTE width_given;
	UBYTE height_given;

	UBYTE pad[ 2 ];
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

		case MA_Layout_Image_Border:
			data->border = tag->ti_Data;
			break;

		case MA_Layout_Embed_Src:
			l_readstrtag( tag, &data->url );
			break;

		case MA_Layout_Embed_Type:
			l_readstrtag( tag, &data->type );
			break;

		case MA_JS_Name:
			l_readstrtag( tag, &data->name );
			break;

		case MA_Layout_VAlign:
			data->li.valign = tag->ti_Data;
			break;

		case MA_Layout_Align:
			data->li.align = tag->ti_Data;
			break;

		case MA_Layout_MarginLeft:
			data->marginleft = tag->ti_Data;
			break;

		case MA_Layout_MarginTop:
			data->margintop = tag->ti_Data;
			break;

		case MA_Layout_MarginRight:
			data->marginright = tag->ti_Data;
			break;

		case MA_Layout_MarginBottom:
			data->marginbottom = tag->ti_Data;
			break;

	}

	return( redraw );
}

DECNEW
{
	struct Data *data;
	APTR dummy;

	obj = DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		MUIA_CustomBackfill, TRUE,
		MUIA_Font, MUIV_Font_Tiny,
		Child, dummy = RectangleObject, End,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->clientobject = dummy;

	data->width = 20;
	data->height = 20;

	doset( data, obj, msg->ops_AttrList );

	/*
	 * Set absolute pixel sizes
	 */
	if( data->widthspec )
	{
		if( strchr( data->widthspec, '%' ) )
		{
			data->widthspec[ strlen( data->widthspec ) - 1 ] = '\0'; /* nuke the '%' */
			data->width_given = SGT_PERCENT;
			data->percentwidth = atoi( data->widthspec );
		}
		else
		{
			data->width_given = SGT_PIXEL;
			data->width = atoi( data->widthspec );
		}
	}
	if( data->heightspec )
	{
		if( strchr( data->heightspec, '%' ) )
		{
			data->heightspec[ strlen( data->heightspec ) - 1 ] = '\0'; /* nuke the '%' */
			data->height_given = SGT_PERCENT;
			data->percentheight = atoi( data->heightspec );
		}
		else
		{
			data->height_given = SGT_PIXEL;
			data->height = atoi( data->heightspec );
		}
	}

	/*
	 * If only width or height is specified,
	 * the other takes the same value so
	 * that it's proportional.
	 */
	if( data->percentwidth && !data->heightspec[ 0 ] )
	{
		data->percentheight = data->percentwidth;
		data->height_given = SGT_PERCENT;
	}

	if( data->percentheight && !data->widthspec[ 0 ] )
	{
		data->percentwidth = data->percentheight;
		data->width_given = SGT_PERCENT;
	}

	/*
	 * Some sensible values
	 */
	if( data->width < 1 )
		data->width = 1;
	if( data->height < 1 )
		data->height = 1;

	if( data->percentwidth < 1 )
		data->percentwidth = 1;
	if( data->percentheight < 1 )
		data->percentheight = 1;

	if( data->url )
		pushmethod( obj, 1, MM_Layout_Embed_Load );
	else
		pushmethod( obj, 1, MM_Layout_Embed_SetupPlugin );

	return( (ULONG)obj );
}

DECDISPOSE
{
	GETDATA;

	if( data->nethandle )
	{
		nets_close( data->nethandle );
	}

	free( data->widthspec );
	free( data->heightspec );
	free( data->url );
	free( data->name );
	free( data->type );

	killpushedmethods( obj );
	killpushedmethods( data->clientobject );

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
		STOREATTR( MA_JS_ClassName, "Embed" );
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

	if( data->width_given == SGT_PERCENT )
	{
		data->width = msg->suggested_width * data->percentwidth / 100;
	}

	if( data->height_given == SGT_PERCENT )
	{
		data->height = msg->suggested_height * data->percentheight / 100;
	}

	iwidth = data->width;
	iheight = data->height;

	// Borders...
	iwidth += data->marginleft + data->marginright;
	iheight += data->margintop + data->marginbottom;

	data->li.minwidth = data->li.maxwidth = data->li.defwidth = iwidth;
	data->li.minheight = data->li.maxheight = data->li.defheight = iheight;

	return( (ULONG)&data->li );
}

DECMMETHOD( Draw )
{
	//GETDATA;
	//struct RastPort *rp = _rp( obj );

	DOSUPER;

	return( 0 );
}

DECMMETHOD( Setup )
{
	ULONG rc;
	
	rc = DOSUPER;

	return( rc );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

DECMMETHOD( Show )
{
	GETDATA;
	ULONG rc;

	rc = DOSUPER;

	data->mleft = _left( obj ) + data->marginleft;
	data->mright = _right( obj ) - data->marginright;
	data->mtop = _top( obj ) + data->margintop;
	data->mbottom = _bottom( obj ) - data->marginbottom;

	return( rc );
}

DECSMETHOD( Layout_CalcUnloadedObjects )
{
	GETDATA;
	if( data->nethandle && !nets_state( data->nethandle ) )
		*msg->cnt_other = *msg->cnt_other + 1;
	return( 0 );
}

DECTMETHOD( Layout_Embed_Load )
{
	GETDATA;

	data->nethandle = nets_open( data->url, NULL, obj, NULL, NULL, 0, 0 );

	return( 0 );
}

DECMETHOD( NStream_GotInfo, APTR )
{
	GETDATA;

	if( !data->type && nets_mimetype( data->nethandle )[ 0 ] )
		data->type = strdup( nets_mimetype( data->nethandle ) ); /* TOFIX */

	return( DoMethod( obj, MM_Layout_Embed_SetupPlugin ) );
}

DECMETHOD( NStream_Done, APTR )
{
	GETDATA;

	if( data->nethandle && nets_state( data->nethandle ) > 0 )
	{
		if( data->clientobject )
			DoMethodA( data->clientobject, (Msg)msg );
	}

	// Send a CHECKDONE to the htmlwin...
	if( data->ctx )
		pushmethod( data->ctx->dom_win, 1, MM_HTMLWin_CheckDone );

	return( 0 );
}

DECTMETHOD( Layout_Embed_SetupPlugin )
{
#if USE_PLUGINS
	GETDATA;
	APTR class = 0;
	char *p, *ext = NULL;

	/*
	 * Find the extension of the file
	 */
	if ( data->url )
	{
		ext = strrchr( data->url, '.' );
		p = strrchr( data->url, '/' );

		if( p > ext )
		{
			/*
			 * .com, .net, etc...
			 */
			ext = NULL;
		}
	
	}

	if ( data->type )
	{
		class = plugin_mimetype( data->type, ext );
	}

	if( !class )
	{
		/*
		 * Display the "plugin not found" informations
		 */
		if( data->nethandle )
		{
			nets_close( data->nethandle );
			data->nethandle = NULL;
		}

		DoMethod( obj, MUIM_Group_InitChange, 0 );
		if( data->clientobject )
		{
			DoMethod( obj, OM_REMMEMBER, data->clientobject );
			MUI_DisposeObject( data->clientobject );
		}

		data->clientobject = TextObject, MUIA_Font, MUIV_Font_Tiny, ButtonFrame, MUIA_Background, MUII_ButtonBack, MUIA_Text_SetMin, FALSE, MUIA_Text_SetMax, FALSE, MUIA_InputMode, MUIV_InputMode_RelVerify, MUIA_Text_SetVMax, FALSE, End;

		DoMethod( data->clientobject, MUIM_SetAsString, MUIA_Text_Contents,
			GS( PLUGIN_EMBED_NOPLUGIN ),
			data->type
		);
		DoMethod( data->clientobject, MUIM_Notify, MUIA_Pressed, FALSE,
			obj, 1, MM_Layout_Embed_FindPlugin
		);
		DoMethod( obj, OM_ADDMEMBER, data->clientobject );
		DoMethod( obj, MUIM_Group_ExitChange, 0 );
	}
	else
	{
		/*
		 * Display the plugin now
		 */
		APTR newobj = NewObject( class, NULL,
			VPLUG_EmbedInfo_URL, data->url,
			VPLUG_EmbedInfo_ParentURL, data->ctx->baseref, 
			VPLUG_EmbedInfo_Baseref, data->ctx->baseref,
			VPLUG_EmbedInfo_ArgNames, data->arg_names,
			VPLUG_EmbedInfo_ArgValues, data->arg_values,
			VPLUG_EmbedInfo_ArgCnt, data->arg_cnt,
			VPLUG_EmbedInfo_NetStream, data->nethandle,
			TAG_DONE
		);
		if( newobj )
		{
			DoMethod( obj, MUIM_Group_InitChange, 0 );
			if( data->clientobject )
			{
				DoMethod( obj, OM_REMMEMBER, data->clientobject );
				MUI_DisposeObject( data->clientobject );
			}
			DoMethod( obj, OM_ADDMEMBER, newobj );
			data->clientobject = newobj;
			DoMethod( obj, MUIM_Group_ExitChange, 0 );
		}
	}

#endif
	return( 0 );
}

DECTMETHOD( Layout_Embed_FindPlugin )
{
#if USE_PLUGINS
	GETDATA;
	char url[ 1024 ];

	sprintf( url, "http://v3.vapor.com/index.php?plugin=%s", data->type );

	win_create( "", url, NULL, NULL, FALSE, FALSE, FALSE );

#endif
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFSET
DEFGET
DEFDISPOSE
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_CalcUnloadedObjects )
DEFTMETHOD( Layout_Embed_Load )
DEFTMETHOD( Layout_Embed_SetupPlugin )
DEFTMETHOD( Layout_Embed_FindPlugin )
DEFSMETHOD( NStream_GotInfo )
DEFSMETHOD( NStream_Done )
DEFMMETHOD( Setup )
DEFMMETHOD( Draw )
DEFMMETHOD( Show )
DEFMMETHOD( AskMinMax )
ENDMTABLE

int create_lo_embed( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loembedClass";
#endif

	return( TRUE );
}

void delete_lo_embed( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloembedclass( void )
{
	return( lcc->mcc_Class );
}
