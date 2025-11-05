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
** This class contains an htmlview object
** It encapsulates the DOM "window" or "frame" objects
**
** $Id: htmlwin.c,v 1.491 2004/06/09 22:50:32 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#include <intuition/extensions.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#endif

/* private */
#include "classes.h"
#include "mui_func.h"
#include "prefs.h"
#include "network.h"
#include "voyager_cat.h"
#include "js.h"
#include "htmlclasses.h"
#include "gfxcompat.h"
#include "init.h"
#include "rexx.h"

#include "keyimages.h"

#include "mybrush.h"
#include "historylist.h"
#include "tearoff.h"
#include "rexx.h"
#include "methodstack.h"
#include "malloc.h"
#include "menus.h"
#include "sendmailwin.h"
#include "mime.h"
#if USE_PLUGINS
#include "plugins.h"
#endif /* USE_PLUGINS */
#include "download.h"
#include "sourceview.h"
#include "htmlwin.h"
#include <proto/vimgdecode.h>
#include "docinfowin.h"
#include "win_func.h"
#include "history.h"
#include "cmanager.h"
#include "speedbar.h"
#include "dos_func.h"
#include "template.h"

#define MAX_TOOLBUTTONS 64

APTR lastactivewin;
APTR veryfirstwin;
APTR lastactivepanel;

#define MYURL (data->doc_main?nets_fullurl(data->doc_main):(data->doc_loading?nets_fullurl(data->doc_loading):data->orig_baseref))

struct js_timer {
	struct MinNode n;
	APTR funcobj;
	int millis;
	int init_millis;
	int repeatflag;
	int id;
	int funcoffset;
	int killed;
	int numargs;
	int argtype;
	union {
		char txt[ 0 ];
		APTR args[ 0 ];
	} a;
};

struct dependentwin {
	struct MinNode n;
	APTR obj;
};

struct Data {
	struct layout_info li; // Needed when doing frames layout

	char name[ 128 ]; // Window name
	APTR v; // View object
	APTR obj; // Object backreference

	APTR owner;

	// The following objects are only used
	// if we're a standalone "window"
	APTR winobj;	  // MUI window

	APTR bay_top, bay_left, bay_right, bay_bottom;

	APTR panel_toolbar;
	APTR grp_toolbar, bar_tool, spc_tool;
	APTR iconobj;
	APTR *tb_buts;
	ULONG *but_acts;
	char **but_args;
	UBYTE *but_states;
	int numbrushes;
	int toolbar_is_horiz;
	APTR menu_forward, menu_backward;

	APTR panel_location;
	APTR str_url, pop_url, list_url, bt_add, bt_book, tx_urllabel;

	APTR panel_status;
	APTR txt_state, gauge_receive, ledobj, grp_keybm, grp_httpbm;
	char lasttext[ 128 ];

	APTR panel_fastlinks;
	APTR grp_fastlinks;
	int fastlinks_are_horiz;

	APTR clock;

	// Title related stuff
	char title[ 96 ];

	// JS stuff
	struct MinList customprops;
	struct jsop_list *jso;
	APTR js_frames, js_location;
	int ix_onunload, ix_onload, ix_onerror;
	struct MinList js_timers;
	int timer_id;
	int timer_active;
	int timer_iter;
	struct MUI_InputHandlerNode js_ihn;
	ULONG gcmagic;
	char *orig_baseref;
	int mousevisible;
	int mousetype;
	int mousebusy;

	// GUI settings (from JS, or wherever)
	int f_toolbar;
	int f_location;
	int f_directories;
	int f_status;
	int f_width;
	int f_height;
	int f_screenx;
	int f_screeny;
	int f_fullscreen;
	int f_scrollbars;
	int f_resizable;

	APTR f_opener;
	struct MinList f_dependents;

	// tells us that this HTMLWin was window.open'ed
	int is_js_win;
	// tells us that this HTMLWin is dependent on others (should notify parents of closure)
	int is_dependent;

	// holding values for switching to fullscreen and back
	int pre_fs_top, pre_fs_left, pre_fs_width, pre_fs_height;

	// Virtual layout width
	int virtual_width;

	// Document streams
	struct nstream *doc_loading;     /* stream being loaded before its type has been identified */
	struct nstream *doc_main;        /* current nstream for the window once it has been identified (text/) */

	// Scrolling modes
	int scrolling;
	int drawframe;

	// Window inner margins
	int innermargin_left, innermargin_right, innermargin_top, innermargin_bottom;

	APTR imgdec; /* image decoder object, only used in individual image view */
};

static struct MUI_CustomClass *mcc;

#if 0
char *FilePart( char *path )
{
	char *p;

	p = strrchr( path, '/' );
	if( !p )
		p = strrchr( path, '\\' );
	if( !p )
		p = strrchr( path, ':' );
	if( p )
		return( p + 1 );
	return( path );
}
#endif


MUI_HOOK( layoutwinfunc, APTR grp, struct MUI_LayoutMsg *lm )
{
	//struct Data *data = INST_DATA( OCLASS( grp ), grp );

	switch( lm->lm_Type )
	{
		case MUILM_MINMAX:
			lm->lm_MinMax.MinWidth = 1;
			lm->lm_MinMax.MinHeight = 1;
			lm->lm_MinMax.DefWidth = 512;
			lm->lm_MinMax.DefHeight = 512;
			lm->lm_MinMax.MaxWidth = MUI_MAXMAX;
			lm->lm_MinMax.MaxHeight = MUI_MAXMAX;
			return( 0 );

		case MUILM_LAYOUT:
			{
				Object *cstate = (Object *)lm->lm_Children->mlh_Head;
				Object *child;
				int yp = 0, ys = lm->lm_Layout.Height;

				while( child = NextObject( &cstate ) )
				{
					D( db_html, bug( "sending layout to child 0x%lx\n", child ) );
					//if ( getv( child, MUIA_UserData ) != 2 )
					{
						MUI_Layout( child, 0, yp, lm->lm_Layout.Width, ys, 0 ); /* XXX: ahee.. no check ? well.. MUI would just change the fontsize anyway */
					}
				}
			}
			return( TRUE );
	}

	return( MUILM_UNKNOWN );
}

static int doset( APTR obj, struct Data *data, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_Name:
		case MA_HTMLWin_Name:
			stccpy( data->name, (char*)tag->ti_Data, sizeof( data->name ) );
			break;

		case MA_HTMLWin_Scrolling:
			data->scrolling = tag->ti_Data;
			break;

		case MA_HTMLWin_DrawFrame:
			data->drawframe = tag->ti_Data;
			break;

		case MA_Layout_MarginLeft:
			data->innermargin_left = tag->ti_Data;
			break;

		case MA_Layout_MarginRight:
			data->innermargin_right = tag->ti_Data;
			break;

		case MA_Layout_MarginTop:
			data->innermargin_top = tag->ti_Data;
			break;

		case MA_Layout_MarginBottom:
			data->innermargin_bottom = tag->ti_Data;
			break;

		case MA_JS_Window_URL:
			{
				char buffer[ MAXURLSIZE ];

				uri_mergeurl( MYURL, (char*)tag->ti_Data, buffer );
				DoMethod( obj, MM_HTMLWin_SetURL, buffer, NULL, NULL, MF_HTMLWin_AddURL );
			}
			break;

		case MA_HTMLWin_Toolbar:
			data->f_toolbar = tag->ti_Data;
			break;

		case MA_HTMLWin_Fastlinks:
			data->f_directories = tag->ti_Data;
			break;

		case MA_HTMLWin_Height:
			data->f_height = (tag->ti_Data == -1) ? 0 : tag->ti_Data;
			break;

		case MA_HTMLWin_Width:
			data->f_width = (tag->ti_Data == -1) ? 0 : tag->ti_Data;
			break;

		case MA_HTMLWin_ScreenX:
			data->f_screenx = tag->ti_Data;
			break;

		case MA_HTMLWin_ScreenY:
			data->f_screeny = tag->ti_Data;
			break;

		case MA_HTMLWin_Location:
			data->f_location = tag->ti_Data;
			break;

		case MA_HTMLWin_StatusBar:
			data->f_status = tag->ti_Data;
			break;

		case MA_HTMLWin_Opener:
			data->f_opener = (APTR)tag->ti_Data;
			break;

		case MA_HTMLWin_Imgclient:
			data->imgdec = ( APTR )tag->ti_Data;
			break;

		case MA_HTMLWin_FullScreen:
			data->f_fullscreen = tag->ti_Data;
			break;

		case MA_HTMLWin_Resizable:
			data->f_resizable = tag->ti_Data;
			break;

		case MA_HTMLWin_Scrollbars:
			data->f_scrollbars = tag->ti_Data;
			break;

		case MA_HTMLWin_Dependent:
			data->is_dependent = tag->ti_Data;
			break;

		case MA_HTMLWin_IsJsWin:
			data->is_js_win = tag->ti_Data;
			break;

		case MA_HTMLWin_OrigBaseref:
			if( data->orig_baseref )
				free( data->orig_baseref );
			data->orig_baseref = strdup( (char*)tag->ti_Data );
			break;

		case MA_HTMLWin_VirtualWidth:
			if( data->virtual_width != tag->ti_Data )
			{
				data->virtual_width = tag->ti_Data;
				set( data->v, MA_HTMLWin_VirtualWidth, data->virtual_width );
				if( _parent( obj ) )
				{
					DoMethod( _parent( obj ), MUIM_Group_InitChange );
					DoMethod( _parent( obj ), MUIM_Group_ExitChange2, TRUE );
				}
			}
			break;
	}

	return( redraw );
}


DECNEW
{
	struct Data *data;
	APTR vo;

	obj = DoSuperNew( cl, obj,
	    MUIA_FillArea, FALSE,
		MUIA_CustomBackfill, TRUE,
		MUIA_Group_LayoutHook, (ULONG)&layoutwinfunc_hook,
		Child, ScrollgroupObject,
			MUIA_Scrollgroup_Contents, vo = NewObject( gethtmlviewclass(), NULL, TAG_DONE ),
			MUIA_Scrollgroup_FreeHoriz, TRUE,
			MUIA_Scrollgroup_FreeVert, TRUE,
			getflag( VFLG_SCROLLBARS ) ? MUIA_Scrollgroup_UseWinBorder : TAG_IGNORE, TRUE,
			MUIA_Scrollgroup_AutoBars, TRUE,
		End,
		MUIA_Weight, 10000,
		MUIA_Group_Spacing, 0,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->v = vo;
	data->obj = obj;
	data->gcmagic = -1UL;
	data->scrolling = TRUE;

	data->f_width = -1;
	data->f_height = -1;
	data->f_screenx = -1;
	data->f_screeny = -1;
	data->f_fullscreen = FALSE;
	data->f_scrollbars = TRUE;
	data->f_resizable = TRUE;

	NEWLIST( &data->f_dependents );

	data->innermargin_left = 10;
	data->innermargin_right = 10;
	data->innermargin_bottom = 15;
	data->innermargin_top = 15;

	data->mousevisible = TRUE;

	set( vo, MA_HTMLView_HTMLWin, obj );

	data->list_url = NewObject( gethistorylistclass(), NULL,
		InputListFrame,
		MA_Historylist_Owner, obj,
		TAG_DONE
	);

	if( !data->list_url )
	{
		CoerceMethod( cl, obj, OM_DISPOSE );
		return( 0 );
	}

	strcpy( data->title, "Empty" );

	NEWLIST( &data->customprops );
	NEWLIST( &data->js_timers );

	doset( obj, data, msg->ops_AttrList );

	_flags( obj ) |= MADF_KNOWSACTIVE;

	if( !data->orig_baseref )
		data->orig_baseref = strdup( "" );

	return( (ULONG)obj );
}

DECDISPOSE
{
	GETDATA;

	if( lastactivewin == obj )
		lastactivewin = NULL;

	if( lastactivepanel == obj )
		lastactivepanel = NULL;

	if( data->doc_loading )
		nets_close( data->doc_loading );

	if( data->doc_main )
		nets_close( data->doc_main );

	killpushedmethods( obj );
	killpushedmethods( data->gauge_receive );

	DoMethod( obj, MM_HTMLWin_CleanupTimers );

	// When not in window, or location bar is hidden, the list object is standalone
	if( !data->winobj || data->f_location )
	{
		if( data->list_url )
		{
			MUI_DisposeObject( data->list_url );
			data->list_url = NULL;
		}
	}

	if( data->jso )
	{
		jso_cleanup( data->jso );
		data->jso = NULL; /* TOFIX: needed ? */
	}

	if( data->orig_baseref )
		free( data->orig_baseref );

	return( DOSUPER );
}

//
// Javascript property/method handling
//

BEGINPTABLE
DPROP( name,        string )
DPROP( status,      string )
DPROP( self,        obj )
DPROP( window,      obj )
DPROP( top,         obj )
DPROP( document,    obj )
DPROP( alert,       funcptr )
DPROP( confirm,     funcptr )
DPROP( onload,      funcptr )
DPROP( onunload,    funcptr )
DPROP( onerror,     funcptr )
DPROP( scroll,      funcptr )
DPROP( scrollBy,    funcptr )
DPROP( scrollTo,    funcptr )
DPROP( open,	    funcptr )
DPROP( close,	    funcptr )
DPROP( setTimeout,	funcptr )
DPROP( setInterval,	funcptr )
DPROP( focus,		funcptr )
DPROP( blur,		funcptr )
DPROP( clearTimeout,	funcptr )
DPROP( clearInterval,	funcptr )
DPROP( location,    obj )
DPROP( navigator,   obj )
DPROP( parent,      obj )
DPROP( frames,      obj )
DPROP( history,     obj )
DPROP( length,		real )
DPROP( moveBy,		funcptr )
DPROP( moveTo,		funcptr )
DPROP( resizeBy,	funcptr )
DPROP( resizeTo,	funcptr )
DPROP( innerWidth, 	real )
DPROP( innerHeight, real )
DPROP( outerWidth,	real )
DPROP( outerHeight, real )
DPROP( pageXOffset, real )
DPROP( pageYOffset, real )
DPROP( screenX,		real )
DPROP( screenY,		real )
DPROP( mouseVisible,	bool )
DPROP( atob,		string )
ENDPTABLE

static void buildframesarray( struct Data *data )
{
	//if( !data->js_frames )
	//{
		data->js_frames = JSNewObject( getjs_array(), MA_JS_Object_TerseArray, TRUE, TAG_DONE );
		DoMethod( data->v, MM_Layout_Frameset_BuildFramesArray, data->js_frames );
	//}
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;
	struct customprop *cp;
	GETDATA;
	APTR o;

	if( !data->jso )
		data->jso = jso_init();

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	// Check for direct childs
	buildframesarray( data );
	if( data->js_frames )
	{
		if( DoMethodA( data->js_frames, ( Msg )msg ) )
			return( expt_obj );
	}

	cp = cp_find( &data->customprops, msg->propname );
	if( cp )
		return( expt_obj );

	for( o = FIRSTNODE( &data->jso->vars ); NEXTNODE( o ); o = NEXTNODE( o ) )
	{
		APTR obj = BASEOBJECT( o );
		int rc;

		rc = DoMethod( obj, MM_JS_NameIs, ( ULONG )msg->propname );

		if( rc )
			return( expt_obj );
	}

	return( 0 );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	GETDATA;
	struct customprop *cp;
	int width, height;	// we'll use these often enough IMO :)

	if( !data->jso )
		data->jso = jso_init();

	if( !pt )
	{
		APTR o;

		buildframesarray( data );
		if( DoMethodA( data->js_frames, ( Msg )msg ) )
		{
			return( TRUE );
		}
		cp = cp_find( &data->customprops, msg->propname );
		if( cp )
		{
			storeobjprop( msg, cp->obj );
			return( TRUE );
		}
		for( o = FIRSTNODE( &data->jso->vars ); NEXTNODE( o ); o = NEXTNODE( o ) )
		{
			APTR obj = BASEOBJECT( o );
			int rc;

			rc = DoMethod( obj, MM_JS_NameIs, ( ULONG )msg->propname );

			if( rc )
			{
				storeobjprop( msg, obj );
				return( TRUE );
			}
		}
		return( FALSE );
	}

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}

	switch( (int)pt->id )
	{
		case JSPID_name:
			storestrprop( msg, data->name );
			return( TRUE );

		case JSPID_status:
			if( data->txt_state )
				storestrprop( msg, (STRPTR)getv( data->txt_state, MUIA_Text_Contents ) );
			return( TRUE );

		case JSPID_window:
		case JSPID_self:
			storeobjprop( msg, obj );
			return( TRUE );

		case JSPID_history:
			storeobjprop( msg, data->list_url );
			return( TRUE );

		case JSPID_top:
			{
				APTR lasto = obj, o = _parent( obj );

				while( o )
				{
					if( OCLASS( o ) == cl )
						lasto = o;
					o = _parent( o );
				}
				storeobjprop( msg, lasto );
			}
			return( TRUE );

		case JSPID_document:
			storeobjprop( msg, data->v );
			return( TRUE );

		case JSPID_location:
			if( !data->js_location )
				data->js_location = JSNewObject( getjs_location(), MA_JS_Location_WindowObject, obj, TAG_DONE );
			storeobjprop( msg, data->js_location );
			return( TRUE );

		case JSPID_navigator:
			storeobjprop( msg, data->jso->go_navigator );
			return( TRUE );

		case JSPID_parent:
			{
				APTR o = _parent( obj );

				while( o )
				{
					if( OCLASS( o ) == cl )
						break;
					o = _parent( o );
				}
				storeobjprop( msg, o ? o : obj );
			}
			return( TRUE );

		case JSPID_frames:
			buildframesarray( data );
			storeobjprop( msg, data->js_frames );
			return( TRUE );

		case JSPID_length:
			buildframesarray( data );
			// Proxy method
			return( DoMethodA( data->js_frames, ( Msg )msg ) );

		case JSPID_innerWidth:
			get(data->v, MUIA_Virtgroup_Width, &width);
			if (width)
				storerealprop(msg, (double) width);
			return( TRUE );

		case JSPID_innerHeight:
			get(data->v, MUIA_Virtgroup_Height, &height);
			if (height)
				storerealprop(msg, (double) height);
			return( TRUE );

		case JSPID_outerWidth:
			get(data->winobj, MUIA_Window_Width, &width);
			if (width)
				storerealprop(msg, (double) width);
			return( TRUE );

		case JSPID_outerHeight:
			get(data->winobj, MUIA_Window_Height, &height);
			if (height)
				storerealprop(msg, (double) height);
			return( TRUE );

		case JSPID_pageXOffset:
			get(data->v, MUIA_Virtgroup_Left, &width);  // ignore the variable name ;)
			if (width)
				storerealprop(msg, (double) width);
			return( TRUE );

		case JSPID_pageYOffset:
			get(data->v, MUIA_Virtgroup_Top, &height);
			if (height)
				storerealprop(msg, (double) height);
			return( TRUE );

		case JSPID_screenX:
			get(data->winobj, MUIA_Window_LeftEdge, &width);  // ignore the variable name ;)
			if (width)
				storerealprop(msg, (double) width);
			return( TRUE );

		case JSPID_screenY:
			get(data->winobj, MUIA_Window_TopEdge, &height);
			if (height)
				storerealprop(msg, (double) height);
			return( TRUE );

 		case JSPID_mouseVisible:
 			storeintprop( msg, data->mousevisible );
 			return( TRUE );

		#if 0
		case JSPID_offscreenBuffering:
			storeintprop( msg, TRUE ); /* possibly a lie :) */
			return(TRUE);
		#endif
	}

	return( FALSE );
}

DECSMETHOD( JS_ToString )
{
	js_tostring( "Window", msg );
	return( TRUE );
}

DECSMETHOD( JS_SetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );
	GETDATA;

	if( !pt )
	{
		cp_set( &data->customprops, msg->propname, *(APTR*)msg->dataptr );
		return( TRUE );
	}

	switch( (int)pt->id )
	{
		case JSPID_status:
			{
				APTR lasto = obj, o = _parent( obj );

				while( o )
				{
					if( OCLASS( o ) == cl )
						lasto = o;
					o = _parent( o );
				}
				DoMethod( lasto, MM_HTMLWin_SetTxt, ( ULONG )msg->dataptr );
			}
			return( TRUE );

		case JSPID_location:
			{
				char buffer[ MAXURLSIZE ];
				int size = sizeof( buffer );

				DoMethod( *(APTR*)msg->dataptr, MM_JS_ToString, ( ULONG )buffer, ( ULONG )&size );

				set( obj, MA_JS_Window_URL, buffer );
			}
			return( TRUE );

		case JSPID_name:
			stccpy( data->name, msg->dataptr, sizeof( data->name ) );
			return( TRUE );

		case JSPID_onload:
			data->ix_onload = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onunload:
			data->ix_onunload = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_onerror:
			data->ix_onerror = *((int*)msg->dataptr);
			return( TRUE );

		case JSPID_mouseVisible:
			data->mousevisible =  *((int*)msg->dataptr);
			// Update pointer
			DoMethod( obj, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );
			return( TRUE );

		#if 0
		case JSPID_offscreenBuffering:
			return(TRUE);
		#endif
	}

	return( FALSE );
}

static APTR openjswin( APTR parentobj, STRPTR url, STRPTR name, STRPTR features, STRPTR orig_baseref )
{
	APTR o;
	char fnb[ 256 ], fab[ 256 ];
	int fnc;
	int inarg;
	int boolval, intval;
	int f_toolbar = FALSE,
		f_directories = FALSE,
		f_height = -1,
		f_width = -1,
		f_location = FALSE,
		f_screenx = -1,
		f_screeny = -1,
		f_fullscreen = FALSE,
		is_dependent = FALSE,
		f_resizable = TRUE,
		f_scrollbars = TRUE,
		f_status = FALSE;

	//dprintf( "openjswin %s\n", name );

	if( *name && ( o = win_find_by_name( name, FALSE ) ) )
	{
		DoMethod( o, MM_HTMLWin_SetURL, url, NULL, name, MF_HTMLWin_Reload | MF_HTMLWin_AddURL );
		return( o );
	}

	// Parse the damned features string
	while( *features )
	{
		features = stpblk( features );
		fnc = 0;
		inarg = 0;
		memset( fnb, 0, sizeof( fnb ) );
		memset( fab, 0, sizeof( fnb ) );
		while( *features )
		{
			if( isspace( *features ) )
			{
				features = stpblk( features );
				if( *features != '=' )
					break;
			}

			if( *features == ',' )
			{
				features++;
				break;
			}

			if( *features == '=' )
			{
				inarg = 1;
				fnc = 0;
				features = stpblk( features + 1 );
				continue;
			}

			if( inarg )
				fab[ fnc++ ] = *features++;
			else
				fnb[ fnc++ ] = *features++;
		}

		boolval = TRUE;
		intval = atoi( fab );
		if( fab[ 0 ] )
		{
			int ch = tolower( fab[ 0 ] );
			if( ch == 'f' || ch == 'n' || ch == '0' )
				boolval = FALSE;
		}

		strlwr( fnb );

		if( !strcmp( fnb, "toolbar" ) )
			f_toolbar = boolval;
		else if( !strcmp( fnb, "directories" ) )
			f_directories = boolval;
		else if( !strcmp( fnb, "location" ) )
			f_location = boolval;
		else if( !strcmp( fnb, "status" ) )
			f_status = boolval;
		else if( !strcmp( fnb, "width" ) )
			f_width = intval;
		else if( !strcmp( fnb, "outerwidth" ) )
			f_width = intval;
		else if( !strcmp( fnb, "innerwidth" ) )
			f_width = intval;
		else if( !strcmp( fnb, "height" ) )
			f_height = intval;
		else if( !strcmp( fnb, "outerheight" ) )
			f_height = intval;
		else if( !strcmp( fnb, "innerheight" ) )
			f_height = intval;
		else if( !strcmp( fnb, "screenx" ) )
			f_screenx = intval;
		else if( !strcmp( fnb, "left" ))
			f_screenx = intval;
		else if( !strcmp( fnb, "screeny" ) )
			f_screeny = intval;
		else if( !strcmp( fnb, "top" ) )
			f_screeny = intval;
		else if( !strcmp( fnb, "fullscreen" ) )
			f_fullscreen = boolval;
		else if( !strcmp( fnb, "resizable" ) )
			f_resizable = boolval;
		else if( !strcmp( fnb, "scrollbars" ) )
			f_scrollbars = boolval;
		else if( !strcmp( fnb, "dependent" ) )
			is_dependent = boolval;

		D( db_gui, bug( "got features(%s) = '%s', bv = %ld, iv = %ld\n", fnb, fab, boolval, intval ));
	}

	// to match IE (sucks) if no features at all are given, fire up a plain
	// ordinary window with full toolbars. Hack. No toolbars yet. To be done.
	if ( f_width > -1 || f_height > -1 || f_screenx > -1 || f_screeny > -1 )
	{
		f_scrollbars = FALSE;
	}

	o = win_create( name, url, NULL, NULL, TRUE, FALSE, FALSE );
	if( !o )
		return( NULL );

	SetAttrs( o,
		MA_HTMLWin_Opener, parentobj,
		MA_HTMLWin_Toolbar, !f_toolbar,
		MA_HTMLWin_Fastlinks, !f_directories,
		MA_HTMLWin_Height, ((f_height > 99) || (f_height == -1)) ? f_height : 100,
		MA_HTMLWin_Width,  ((f_width > 99) || (f_width == -1)) ? f_width : 100,
		MA_HTMLWin_Location, !f_location,
		MA_HTMLWin_ScreenX, f_screenx,
		MA_HTMLWin_ScreenY, f_screeny,
		MA_HTMLWin_StatusBar, !f_status,
		MA_HTMLWin_IsJsWin, TRUE,
		MA_HTMLWin_OrigBaseref, orig_baseref,
		MA_HTMLWin_Dependent, is_dependent,
		MA_HTMLWin_FullScreen, f_fullscreen,
		MA_HTMLWin_Scrollbars, f_scrollbars,
		MA_HTMLWin_Resizable, f_resizable,
		MUIA_Scrollgroup_UseWinBorder, TRUE,
		TAG_DONE
	);

	DoMethod( o, MM_HTMLWin_ToStandalone );

	if( is_dependent )
	{
		DoMethod( parentobj, MM_HTMLWin_AddDependent, o );
	}

	return( o );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;

	switch( (int)msg->pid )
	{
		case JSPID_focus:
			if( data->winobj )
				set( data->winobj, MUIA_Window_Activate, TRUE );
			if( muiRenderInfo( obj ) && _win( obj ) )
				set( _win( obj ), MUIA_Window_ActiveObject, obj );
			return( TRUE );

		case JSPID_blur:
			/* send window to back? */
			return( TRUE );

		case JSPID_setTimeout:
		case JSPID_setInterval:
			{
				char buffer[ 512 ];
				struct js_timer *jt;
				double timeout;
				int argtype;
				int funcix = 0;
				int numargs = 0;
				static double rval;

				if( msg->argcnt < 2 )
					return( 0 );

				msg->argcnt -= 2;

				argtype = exprs_get_type( msg->es, 0, NULL );
				if( argtype == expt_obj )
				{
					// We need to figure out whether this is actually an object
					// reference to a function (bah, bah, bah )
					APTR obj = exprs_peek_as_object( msg->es, 0 );
					if( obj )
					{
						int dummy;
						if( get( obj, MA_JS_Func_Index, &dummy ) )
							argtype = expt_funcptr;
					}
				}

				if( argtype == expt_funcptr )
				{
					APTR dummy;

					funcix = exprs_pop_as_funcptr( msg->es, &dummy, &dummy, &dummy );
					numargs = msg->argcnt;
					jt = malloc( sizeof( *jt ) + numargs * sizeof( APTR ) );
					memset( jt, '\0', sizeof( *jt ) + numargs * sizeof( APTR ) );
				}
				else
				{
					exprs_pop_as_string( msg->es, buffer, sizeof( buffer ) );
					jt = malloc( sizeof( *jt ) + strlen( buffer ) + 1 );
					memset( jt, '\0', sizeof( *jt ) + strlen( buffer ) + 1 );
				}
				timeout = exprs_pop_as_real( msg->es );
				if( jt )
				{
					jt->argtype = argtype;
					jt->numargs = numargs;
					if( argtype == expt_funcptr )
					{
						int c;

						for( c = 0; c < numargs; c++ )
						{
							exprs_pop_as_object( msg->es, &jt->a.args[ c ] );
							msg->argcnt--;
						}
						jt->funcoffset = funcix;
					}
					else
					{
						strcpy( jt->a.txt, buffer );
					}

					jt->millis = timeout;
					jt->init_millis = jt->millis;
					jt->id = ++data->timer_id;
					jt->repeatflag = ( msg->pid == JSPID_setInterval );
					ADDHEAD( &data->js_timers, jt );

					rval = jt->id;
					*msg->typeptr = expt_real;
					*msg->dataptr = &rval;

					DoMethod( obj, MM_HTMLWin_StartTimers );
				}
			}
			return( TRUE );

		case JSPID_clearTimeout:
		case JSPID_clearInterval:
			{
				int kill_id;
				struct js_timer *jt;

				if( msg->argcnt-- < 1 )
					return( FALSE );

				kill_id = (int)exprs_pop_as_real( msg->es );

				for( jt = FIRSTNODE( &data->js_timers ); NEXTNODE( jt ); jt = NEXTNODE( jt ) )
				{
					// Mark timer for removal by TriggerTimers
					if( jt->id == kill_id )
						jt->killed = TRUE;
				}

			}
			return( TRUE );

		case JSPID_alert:
			{
				char buffer[ 2048 ];

				if( msg->argcnt-- < 1 )
					return( 0 );

				exprs_pop_as_string( msg->es, buffer, sizeof( buffer ) - 1 );
				buffer[ sizeof( buffer ) - 1 ] = 0;
				MUI_Request( app, _win( obj ), 0, "Javascript Alert", "OK", "%s", (unsigned long)buffer );
			}
			return( TRUE );

		case JSPID_confirm:
			{
				char buffer[ 2048 ];

				if( msg->argcnt-- < 1 )
					return( 0 );

				exprs_pop_as_string( msg->es, buffer, sizeof( buffer ) - 1 );
				buffer[ sizeof( buffer ) - 1 ] = 0;
				return( (ULONG)MUI_Request( app, _win( obj ), 0, "Javascript Question", "OK|Cancel", "%s", (unsigned long)buffer ) );
			}

		case JSPID_scroll:
		case JSPID_scrollTo:
			{
				int x, y;

				if( msg->argcnt-- < 1 )
					return( 0 );
				if( msg->argcnt-- < 1 )
					return( 0 );

				x = exprs_pop_as_int( msg->es );
				y = exprs_pop_as_int( msg->es );

				pushmethod( data->v, 3, MUIM_Set, MUIA_Virtgroup_Top, y );
				pushmethod( data->v, 3, MUIM_Set, MUIA_Virtgroup_Left, x );

				return( TRUE );
			}

		case JSPID_scrollBy:
			{
				int x, y;

				if( msg->argcnt-- < 1 )
					return( 0 );
				if( msg->argcnt-- < 1 )
					return( 0 );

				x = getv( data->v, MUIA_Virtgroup_Left ) + exprs_pop_as_int( msg->es );
				y = getv( data->v, MUIA_Virtgroup_Top ) + exprs_pop_as_int( msg->es );

				pushmethod( data->v, 3, MUIM_Set, MUIA_Virtgroup_Top, y );
				pushmethod( data->v, 3, MUIM_Set, MUIA_Virtgroup_Left, x );

				return( TRUE );
			}

		case JSPID_close:
			{
				if( data->winobj )
				{
					if( data->f_opener )
					{
						pushmethod( obj, 1, MM_HTMLWin_Close );
					}
				}
				return( TRUE );
			}
			break;

		case JSPID_open:
			{
				char url[ 1024 ];
				char buffer[ MAXURLSIZE ];
				char name[ 256 ];
				char features[ 512 ];
				APTR wobj;

				if( msg->argcnt-- < 1 )
					return( 0 );

				exprs_pop_as_string( msg->es, url, sizeof( url ) );
				if( url[ 0 ] )
					uri_mergeurl( MYURL, url, buffer );
				else
					strcpy( buffer, "" );

				if( msg->argcnt-- > 0 )
				{
					exprs_pop_as_string( msg->es, name, sizeof( name ) );
				}
				else
					name[ 0 ] = 0;

				if( msg->argcnt-- > 0 )
				{
					exprs_pop_as_string( msg->es, features, sizeof( features ) );
				}
				else
					features[ 0 ] = 0;

				wobj = openjswin( obj, buffer, name, features, MYURL );

				if( wobj )
				{
					*msg->typeptr = expt_obj;
					*msg->dataptr = wobj;
					*msg->datasize = sizeof( wobj );
				}

				return( TRUE );
			}
			break;

		case JSPID_moveBy:
		case JSPID_moveTo:
			// don't bother if we're in fullscreen or aren't the top level htmlwin
			if (!data->f_fullscreen && data->winobj)
			{
				int	x=0, y=0, dx=0, dy=0;

				if (msg->argcnt-- > 0 ) dx = exprs_pop_as_int( msg->es );
					else return( 0 );
				if (msg->argcnt-- > 0 ) dy = exprs_pop_as_int( msg->es );
					else return( 0 );

				if (msg->pid == JSPID_moveTo)
				{
					get(_win(obj), MUIA_Window_LeftEdge, &x);
					get(_win(obj), MUIA_Window_TopEdge, &y);

					dx = dx + x;
					dy = dy + y;
				}

				MoveWindow(_window(obj), dx, dy);
			}
			return( TRUE );
		break;

		case JSPID_resizeBy:
		case JSPID_resizeTo:
			// don't bother if we're in fullscreen or aren't the top level htmlwin
			if (!data->f_fullscreen && data->winobj)
	        {
				int	w=0, h=0, dw=0, dh=0;

				if (msg->argcnt-- > 0 ) dw = exprs_pop_as_int( msg->es );
					else return( 0 );
				if (msg->argcnt-- > 0 ) dh = exprs_pop_as_int( msg->es );
					else return( 0 );

				get(_win(obj), MUIA_Window_Width, &w);
				get(_win(obj), MUIA_Window_Height, &h);

				if (msg->pid == JSPID_resizeTo)
				{
					if (dw < 100) dw = 100;
					if (dh < 100) dh = 100;
					dw = dw - w;
					dh = dh - h;
				}
				else // msg->pid == JSPID_resizeBy
				{
					if ( (w + dw) < 100 ) dw = dw + (100 - w + dw);
					if ( (h + dh) < 100 ) dh = dh + (100 - h + dh);
				}

				SizeWindow(_window(obj), dw, dh);
			}
			return( TRUE );
		break;

		case JSPID_find:
		/*
			TOFIX

			haha! not implemented. This needs to check the caller's
			originating URL or "domain" attribute somehow to be
			secure to Javascript 1.1/1.2 specs

			Should move document to text, highlight etc.

			no string: display find dialog
		*/
		break;

		case JSPID_atob:

			/* convert from base64 to string */

		break;

		case JSPID_btoa:

			/* convert from string to base64 (confusing isn't it?)  */

		break;

		case JSPID_stop:
		/*	DoMethod( obj, MM_HTMLWin_StopXfer ); */
		break;

	}

	// no super class..
	return( 0 );
}

#if USE_SPEEDBAR
DECMETHOD( HTMLWin_SetupToolbar, ULONG )
{
	int num = 0;
	int c;
	struct MUIS_SpeedBar_Button *bspecs;
	struct Data *data = INST_DATA( cl, obj );
	struct MyBrush *brushes[ 128 ];

	if( !data->panel_toolbar )
	{
		return( 0 );
	}

	DoMethod( data->grp_toolbar, MUIM_Group_InitChange );
	DoMethod( data->grp_toolbar, OM_REMMEMBER, data->spc_tool );
	DoMethod( data->grp_toolbar, OM_REMMEMBER, data->bar_tool );
	MUI_DisposeObject( data->bar_tool );

	num = getprefslong( DSI_BUTTON_NUM, 0 );

	if( !num )
	{
		for( num = 0; ; num++ )
		{
			if( !getprefs( DSI_BUTTONS_LABELS + num ) )
				break;
		}
	}

	data->numbrushes = num;
	bspecs = malloc( sizeof( *bspecs ) * ( num + 1 ) );
	memset(bspecs, 0, sizeof( *bspecs ) * ( num + 1 ));

	data->tb_buts = malloc( 4 * num );
	data->but_acts = malloc( 4 * num );
	data->but_args = malloc( 4 * num );
	data->but_states = malloc( num );

	/* setup buttons */
	for( c = 0; c < num; c++ )
	{
		int act = getprefslong( DSI_BUTTONS_ACTION + c, BFUNC_SEP );

		data->but_acts[ c ] = act;
		data->but_args[ c ] = getprefsstr( DSI_BUTTONS_ARGS + c, "" );
		data->but_states[ c ] = 1;

		if( act == BFUNC_SEP )
		{
			bspecs[ c ].Img = MUIV_SpeedBar_Spacer;
		}
		else
		{
			/*
			 * This could be done in a better way and will probably change
			 * in the future anyway.
			 */
			if( match_command( data->but_acts[ c ], "GoForward", data->but_args[ c ] ) || match_command( data->but_acts[ c ], "GoBackward", data->but_args[ c ] ) )
			{
				bspecs[ c ].Class = getbuttonclass();
			}

			brushes[ c ] = b_load( getprefsstr( DSI_BUTTONS_IMAGES + c, "" ) );
			bspecs[ c ].Img = c;
			/* toggle the startup state of the buttons */
			bspecs[ c ].Text = getprefsstr( DSI_BUTTONS_LABELS + c, "" );
			if( match_command( data->but_acts[ c ], "GoBackward", data->but_args[ c ] ) ||
				match_command( data->but_acts[ c ], "GoForward", data->but_args[ c ] ) ||
				match_command( data->but_acts[ c ], "Stop", data->but_args[ c ] ) ||
				match_command( data->but_acts[ c ], "LoadImages", data->but_args[ c ] ) ||
				match_command( data->but_acts[ c ], "Print", data->but_args[ c ] )
			)
			{
				bspecs[ c ].Flags = MUIV_SpeedBar_ButtonFlag_Disabled;
			}
		}
	}

	bspecs[ c ].Img = MUIV_SpeedBar_End;
	brushes[ c ] = CreateEmptyBrush( 16, 1 );

	data->bar_tool = SpeedBarObject,
		MUIA_SpeedBar_Images, brushes,
		MUIA_SpeedBar_SameWidth, TRUE,
		MUIA_SpeedBar_Buttons, bspecs,
		MUIA_SpeedBar_SpacerIndex, c,
		MUIA_SpeedBar_RaisingFrame, getprefslong( DSI_BUTTON_STYLE_RAISED, TRUE ),
		MUIA_SpeedBar_Borderless, getprefslong( DSI_BUTTON_STYLE_BORDERLESS, TRUE ),
		MUIA_SpeedBar_Sunny, getprefslong( DSI_BUTTON_STYLE_SUNNY, TRUE ),
		MUIA_SpeedBar_SmallImages, getprefslong( DSI_BUTTON_STYLE_SMALL, FALSE ),
		MUIA_SpeedBar_ViewMode, getflag( VFLG_ACTIONBUT_MODES ),
		MUIA_Group_Horiz, data->toolbar_is_horiz,
	End;

	if( data->bar_tool )
	{
		for( c = 0; c < num; c++ )
		{
			data->tb_buts[ c ] = bspecs[ c ].Object;
			DoMethod( data->tb_buts[ c ], MUIM_Notify, MUIA_Pressed, FALSE,
				obj, 2, MM_HTMLWin_DoToolbutton, c
			);

			/*
			 * We use ContextMenuBuild in toolbar.c
			 */
			if( match_command( data->but_acts[ c ], "GoForward", data->but_args[ c ] ) )
			{
				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_UserData, bfunc_forward ); //TOFIX!! this sux, the name should change
				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_ContextMenu, 1 );

				DoMethod( data->list_url, MUIM_Notify, MA_Historylist_HasNext, MUIV_EveryTime, data->bar_tool, 5, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue );
				if( getv( data->list_url, MA_Historylist_HasNext ) )
				{
					DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_Disabled, FALSE );
				}
			}
			if(  match_command( data->but_acts[ c ], "GoBackward", data->but_args[ c ] ) )
			{

				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_UserData, bfunc_back ); //TOFIX!! this sux as much as above
				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_ContextMenu, 1 );

				DoMethod( data->list_url, MUIM_Notify, MA_Historylist_HasBack, MUIV_EveryTime, data->bar_tool, 5, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue );
				if( getv( data->list_url, MA_Historylist_HasBack ) )
				{
					DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_Disabled, FALSE );
				}
			}

		}

		if( data->iconobj )
			DoMethod( data->grp_toolbar, OM_REMMEMBER, data->iconobj );

		DoMethod( data->grp_toolbar, OM_ADDMEMBER, data->bar_tool );
		DoMethod( data->grp_toolbar, OM_ADDMEMBER, data->spc_tool );

		if( data->iconobj )
			DoMethod( data->grp_toolbar, OM_ADDMEMBER, data->iconobj );
	}
	DoMethod( data->grp_toolbar, MUIM_Group_ExitChange );

	free( bspecs );

	return( 0 );
}
#else

DECMETHOD( HTMLWin_SetupToolbar, ULONG )
{
	// Don't create a toolbar...
	return( 0 );
}

#endif

#if USE_NET
static APTR fastlinkbutton( int num, int setmin )
{
	return( NewObject( getfastlinkclass(), NULL, MUIA_Text_SetMin, setmin, MA_Fastlink_Number, num, InnerSpacing( 1, 1 ), TAG_DONE ) );
}
#endif /* USE_NET */

#if USE_NET
DECMETHOD( HTMLWin_SetupFastlinks, ULONG )
{
	GETDATA;
	int c;
	int num = getprefslong( DSI_FASTLINKS_NUM, 8 );
	APTR o, ostate;
	struct List *l;
	int ishoriz = data->fastlinks_are_horiz;
	int rows = ( num + 7 ) / 8;
	APTR containergroup = data->grp_fastlinks;
	int fastlinks_setmin = !getflag( VFLG_FASTLINKS_STRIPTEXT );

	if( !containergroup )
		return( 0 );

	DoMethod( data->grp_fastlinks, MUIM_Group_InitChange );

	get( data->grp_fastlinks, MUIA_Group_ChildList, &l );

	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		DoMethod( data->grp_fastlinks, OM_REMMEMBER, o );
		MUI_DisposeObject( o );
	}

	if( ishoriz && rows > 1 )
	{
		containergroup = ColGroup( 8 ), MUIA_Group_SameWidth, TRUE, End;
	}

	for( c = 0; c < num; c++ )
	{
		if( ( o = fastlinkbutton( c, fastlinks_setmin ) ) )
		{
			DoMethod( containergroup, OM_ADDMEMBER, o );
			DoMethod( o, MUIM_Notify, MUIA_Pressed, FALSE,
				obj, 2, MM_HTMLWin_SelectFastlink, c
			);
		}
		else
		{
			/*
			 * We can fail there. The user will have missing fastlinks..
			 * better than a crash.
			 */
			displaybeep();
			break;
		}
	}

	if( rows > 1 && ishoriz )
	{
		// add rects
		for( ; c % 8; c++ )
		{
			if( ( o = RectangleObject, MUIA_Weight, 1, End ) )
			{
				DoMethod( containergroup, OM_ADDMEMBER, o );
			}
		}
	}

	if( data->grp_fastlinks != containergroup )
		DoMethod( data->grp_fastlinks, OM_ADDMEMBER, containergroup );

	DoMethod( data->grp_fastlinks, MUIM_Group_ExitChange );

	return( 0 );

}
#endif /* USE_NET */

DECMETHOD( HTMLWin_SetupIcon, ULONG )
{
	int needit = !getflag( VFLG_HIDE_ICON );
	GETDATA;

	if( !data->panel_toolbar )
		return( 0 );

	if( data->iconobj && !needit )
	{
		DoMethod( data->grp_toolbar, MUIM_Group_InitChange );
		DoMethod( data->grp_toolbar, OM_REMMEMBER, data->iconobj );
		MUI_DisposeObject( data->iconobj );
		data->iconobj = NULL;
		DoMethod( data->grp_toolbar, MUIM_Group_ExitChange );
	}
	else if( !data->iconobj && needit )
	{
		data->iconobj = NewObject(
				getamiconclass(), NULL,
				ImageButtonFrame,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Weight, 0,
				InnerSpacing( 1, 1 ),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_ShortHelp, GS( SH_WIN_BT_VOYAGER ),
				MUIA_Draggable, TRUE,
				TAG_DONE
		);

		if( data->iconobj )
		{
			DoMethod( data->grp_toolbar, MUIM_Group_InitChange );
			DoMethod( data->grp_toolbar, OM_ADDMEMBER, data->iconobj );
			DoMethod( data->grp_toolbar, MUIM_Group_ExitChange );
			// V Home
			DoMethod( data->iconobj, MUIM_Notify, MUIA_Pressed, FALSE,
				obj, 2, MM_HTMLWin_SelectFastlink, -1
			);
		}
	}
	return( 0 );
}

void dorawkey(Object *app,struct IntuiMessage *imsg)
{
}

#if USE_KEYFILES
extern int demotimedout;
#else
int demotimedout = 0;
#endif


// Move window into it's own GUI window
DECMETHOD( HTMLWin_ToStandalone, APTR )
{
	GETDATA;
	APTR grp_contents;
	#if USE_TEAROFF
	int f_flags = 0;
	#endif
	ULONG winnum;
	APTR bt_popurl;

	// Are we already in a window?
	if( data->winobj )
	{
		return( FALSE );
	}

	if( !( winnum = get_window_number() ) )
	{
		return( FALSE );
	}

	if (data->f_fullscreen)
	{
		// get screen width and height, respecting titlebar etc., and fuck any values
		// that a user might ever pass..
		//extern struct Screen *destscreen;
		struct Screen *curscreen = muiRenderInfo( obj )->mri_Screen; //destscreen; XXX: yeah well.. to have this working it needs to be set HERE, not on setup.. major hack anyway.. nees to be rethought out and redone */

		if (!curscreen)
		{
			// whoops. No Screen!
			data->f_fullscreen = FALSE;
		}
		else
		{
			// replace specified stuff with screen values
			data->f_width = curscreen->Width;
			data->f_height = curscreen->Height - curscreen->BarHeight - 1;
			data->f_screenx = 0;
			data->f_screeny = curscreen->BarHeight + 1;

			// IE disables all these in JS fullscreen mode..
			if (data->is_js_win)
			{
				data->f_location = TRUE;
				data->f_directories = TRUE;
				data->f_toolbar = TRUE;
				data->f_status = TRUE;
			}
		}
	}

	data->winobj = WindowObject,
		MUIA_Window_ScreenTitle, (ULONG)copyright,
		// ( data->f_innerheight < 0 && data->f_innerwidth < 0 && data->f_screenx < 0 && data->f_screeny < 0 && data->f_height < 0 && data->f_width < 0 ) ? MUIA_Window_ID : TAG_IGNORE, MAKE_ID( 'V', 'H', 'T', winnum ),
		data->is_js_win ? TAG_IGNORE : MUIA_Window_ID, MAKE_ID( 'V', 'H', 'T', winnum ),
		data->f_fullscreen ? TAG_IGNORE : MUIA_Window_UseRightBorderScroller, getflag( VFLG_SCROLLBARS ),
		data->f_fullscreen ? TAG_IGNORE : MUIA_Window_UseBottomBorderScroller, getflag( VFLG_SCROLLBARS ),
		MUIA_Window_AppWindow, TRUE,
		data->f_screenx >= 0 ? MUIA_Window_LeftEdge : TAG_IGNORE, data->f_screenx,
		data->f_screeny >= 0 ? MUIA_Window_TopEdge : TAG_IGNORE, data->f_screeny,
/*
		data->f_width >= 0 ? MUIA_Window_Width : TAG_IGNORE, data->f_width + 15,
		data->f_height >= 0 ? MUIA_Window_Height : TAG_IGNORE, data->f_height + 20,
*/
		data->f_fullscreen ? MUIA_Window_Borderless : TAG_IGNORE, TRUE,
		data->f_fullscreen ? MUIA_Window_DepthGadget: TAG_IGNORE, FALSE,
		data->f_fullscreen ? MUIA_Window_CloseGadget: TAG_IGNORE, FALSE,
		data->f_fullscreen ? MUIA_Window_DragBar : TAG_IGNORE, FALSE,
		( data->f_fullscreen || !data->f_resizable ) ? MUIA_Window_SizeGadget : TAG_IGNORE , FALSE,

/*		  MUIA_Window_RootObject, grp_contents = VGroup,
			InnerSpacing( 0, 0 ),
			MUIA_Group_Spacing, 0,
*/
		MUIA_Window_RootObject,
				grp_contents = NewObject( getsizegroupclass(), NULL,
								MA_SizeGroup_SizeX, data->f_width > 0 ? data->f_width : 0,
								MA_SizeGroup_SizeY, data->f_height > 0 ? data->f_height : 0,
								InnerSpacing( 0, 0 ),
								MUIA_Group_Spacing, 0,
								TAG_DONE ),

			End,

		TAG_DONE;

	if( !data->winobj )
		return( FALSE );

#if USE_TEAROFF
	if( gp_tearoff )
	{
		// We need the bays..
		if( !data->f_location || !data->f_status || !data->f_toolbar || !data->f_directories )
		{
			data->bay_top = TearOffBayObject, MUIA_VertWeight, 0,
				MUIA_ObjectID, MAKE_ID( f_flags, 'B','1', winnum ),
			End;

			if( data->bay_top )
			{
				data->bay_left = TearOffBayObject, MUIA_VertWeight, 0,
					MUIA_ObjectID, MAKE_ID( f_flags, 'B','2', winnum ),
					MUIA_TearOffBay_Horiz, 0,
				End;
				data->bay_right = TearOffBayObject, MUIA_VertWeight, 0,
					MUIA_ObjectID, MAKE_ID( f_flags, 'B','3', winnum ),
					MUIA_TearOffBay_Horiz, 0,
				End;
				data->bay_bottom = TearOffBayObject, MUIA_VertWeight, 0,
					MUIA_ObjectID, MAKE_ID( f_flags, 'B','4', winnum ),
				End;

				if( data->bay_left && data->bay_right && data->bay_bottom )
				{
					SetAttrs( data->bay_top,
						MUIA_TearOffBay_LinkedBay, data->bay_left,
						MUIA_TearOffBay_LinkedBay, data->bay_right,
						MUIA_TearOffBay_LinkedBay, data->bay_bottom,
						NULL
					);
				}
				DoMethod( grp_contents, OM_ADDMEMBER, data->bay_top );
			}
		}
	}
#endif

	// Build toolbar panel?
	if( !data->f_toolbar )
	{
		APTR grp;

#if USE_SPEEDBAR // TOFIX!!
		grp = data->grp_toolbar = HGroup,
			Child, data->bar_tool = SpeedBarObject, End,
			Child, data->spc_tool = RectangleObject, MUIA_Weight, 1, End,
		End;
#else
		grp = data->grp_toolbar = HGroup,
			Child, data->spc_tool = RectangleObject, MUIA_Weight, 1, End,
		End;
#endif /* !USE_SPEEDBAR */

#if USE_TEAROFF
		if( gp_tearoff )
		{
			data->panel_toolbar = TearOffPanelObject,
				MUIA_Weight, 0, InnerSpacing( 0, 0 ),
				MUIA_ObjectID, MAKE_ID( f_flags, 'P', '1', winnum ),
				MUIA_TearOffPanel_Contents, grp,
				MUIA_TearOffPanel_Label, "Navigation",
				MUIA_TearOffPanel_CanFlipShape, TRUE,
			End;

			if( data->panel_toolbar )
			{
	            DoMethod( data->bay_top, OM_ADDMEMBER, data->panel_toolbar );

				DoMethod( data->panel_toolbar, MUIM_Notify, MUIA_TearOffPanel_Horiz, MUIV_EveryTime,
					app, 3, MUIM_WriteLong, MUIV_TriggerValue, &data->toolbar_is_horiz
				);
				DoMethod( data->panel_toolbar, MUIM_Notify, MUIA_TearOffPanel_Horiz, MUIV_EveryTime,
					data->grp_toolbar, 3, MUIM_Set, MUIA_Group_Horiz, MUIV_TriggerValue
				);
				DoMethod( data->panel_toolbar, MUIM_Notify, MUIA_TearOffPanel_Horiz, MUIV_EveryTime,
					obj, 1, MM_HTMLWin_SetupToolbar
				);
			}
			else
			{
				displaybeep();
			}
		}
		else
#endif /* USE_TEAROFF */
		{
			data->panel_toolbar = grp;
			if( data->panel_toolbar )
			{
				DoMethod( grp_contents, OM_ADDMEMBER, data->panel_toolbar );
			}
			else
			{
				displaybeep();
			}
		}

	}

	//TOFIX: check the allocations following as well

	// Build location panel?
	if( !data->f_location )
	{
		APTR grp;

		grp = HGroup, InnerSpacing( 0, 0 ), MUIA_Group_SameHeight, TRUE, MUIA_Weight, 0,

			Child, data->tx_urllabel = NewObject( getsmartlabelclass(), NULL,
				MUIA_Font, MUIV_Font_Tiny,
				MA_Smartlabel_Text, GS( WIN_URL ),
				MA_Smartlabel_MoreText, GS( WIN_URL ),
			End,

			Child, HGroup, MUIA_Group_SameHeight, TRUE, MUIA_Group_Spacing, 0,

				Child, data->pop_url = PopobjectObject,
					MUIA_Popstring_String, data->str_url = NewObject( geturlstringclass(), NULL,
						MUIA_CycleChain, 1,
						MUIA_ShortHelp, GS( SH_WIN_BT_URL ),
						MUIA_Disabled, demotimedout,
					End,
					MUIA_Popstring_Button, bt_popurl = PopButton( MUII_PopUp ),
					MUIA_Popobject_Object, ListviewObject,
						MUIA_Listview_List, data->list_url,
					End,
				End,

#if USE_CMANAGER
				Child, data->bt_add = MUI_MakeObject( MUIO_Button, (unsigned long)"Add" ),
				Child, data->bt_book = MUI_MakeObject( MUIO_Button, (unsigned long)"BM" ),
#endif /* USE_CMANAGER */

			End,

		End;

#if USE_CMANAGER
		SetAttrs( data->bt_add,
			MUIA_CycleChain, 1,
			MUIA_Weight, 0,
			MUIA_Text_SetVMax, FALSE,
			TAG_DONE
		);
		SetAttrs( data->bt_book,
			MUIA_CycleChain, 1,
			MUIA_Weight, 0,
			MUIA_Text_SetVMax, FALSE,
			TAG_DONE
		);
#endif /* USE_CMANAGER */

		if( data->str_url )
		{
			set( data->str_url, MA_URLString_Text, data->tx_urllabel );

			DoMethod( data->str_url, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
				obj, 1, MM_HTMLWin_URLAcknowledge
			);
		}

#if USE_CMANAGER
		if( data->bt_book )
		{
			DoMethod( data->bt_book, MUIM_Notify, MUIA_Pressed, FALSE,
				app, 1, MM_App_OpenBMWin
			);
		}

		if( data->bt_add )
		{
			DoMethod( data->bt_add, MUIM_Notify, MUIA_Pressed, FALSE,
				obj, 1, MM_HTMLWin_AddBM
			);
		}
#endif /* USE_CMANAGER */

#if USE_TEAROFF
		if( gp_tearoff )
		{
			data->panel_location = TearOffPanelObject,
				MUIA_Weight, 0, InnerSpacing( 0, 0 ),
				MUIA_ObjectID, MAKE_ID( f_flags, 'P', '2', winnum ),
				MUIA_TearOffPanel_Contents, grp,
				MUIA_TearOffPanel_Label, "Location",
				MUIA_TearOffPanel_CanFlipShape, FALSE,
			End;
			DoMethod( data->bay_top, OM_ADDMEMBER, data->panel_location );
		}
		else
#endif /* USE_TEAROFF */
		{
			data->panel_location = grp;
			DoMethod( grp_contents, OM_ADDMEMBER, data->panel_location );
		}

	}

#if USE_NET
	if( !data->f_directories )
	{
		APTR grp = data->grp_fastlinks = NewObject( getfastlinkgroupclass(), NULL, MUIA_Group_Horiz, TRUE, Child, HSpace( 0 ), MUIA_Disabled, demotimedout, End;

#if USE_TEAROFF
		if( gp_tearoff )
		{
			data->panel_fastlinks = TearOffPanelObject,
				MUIA_Weight, 0, InnerSpacing( 0, 0 ),
				MUIA_ObjectID, MAKE_ID( f_flags, 'P', '3', winnum ),
				MUIA_TearOffPanel_Contents, grp,
				MUIA_TearOffPanel_Label, "Fastlinks",
				MUIA_TearOffPanel_CanFlipShape, TRUE,
			End;
			DoMethod( data->bay_top, OM_ADDMEMBER, data->panel_fastlinks );

			DoMethod( data->panel_fastlinks, MUIM_Notify, MUIA_TearOffPanel_Horiz, MUIV_EveryTime,
				app, 3, MUIM_WriteLong, MUIV_TriggerValue, &data->fastlinks_are_horiz
			);
			DoMethod( data->panel_fastlinks, MUIM_Notify, MUIA_TearOffPanel_Horiz, MUIV_EveryTime,
				grp, 3, MUIM_Set, MUIA_Group_Horiz, MUIV_TriggerValue
			);
			DoMethod( data->panel_fastlinks, MUIM_Notify, MUIA_TearOffPanel_Horiz, MUIV_EveryTime,
				obj, 1, MM_HTMLWin_SetupFastlinks
			);
		}
		else
#endif /* USE_TEAROFF */
		{
			data->panel_fastlinks = grp;
			DoMethod( grp_contents, OM_ADDMEMBER, data->panel_fastlinks );
		}

	}
#endif /* USE_NET */

	if( data->bay_left )
	{
		APTR g = HGroup, MUIA_Group_Spacing, 0, InnerSpacing( 0, 0 ),
			Child, data->bay_left,
			Child, obj,
			Child, data->bay_right,
		End;

		DoMethod( grp_contents, OM_ADDMEMBER, g );

		DoMethod( grp_contents, OM_ADDMEMBER, data->bay_bottom );
	}
	else
	{
		// place ourselfs on the window
		APTR g = HGroup, MUIA_Group_Spacing, 0, InnerSpacing( 0, 0 ),
			Child, obj,
		End;
		if (g)
			DoMethod( grp_contents, OM_ADDMEMBER, g );
		else
			D( db_html, bug( "htmlview HGroup was not created\n" ) );
	}

	if( !data->f_status )
	{
		APTR grp, grp_info;

#if USE_CLOCK
		if( getflag( VFLG_USE_CLOCK ) )
		{
			if( !( data->clock = NewObject( getclockclass(), NULL, TAG_DONE ) ) )
			{
				return( 0 );
			}
		}
		else
		{
			if( !( data->clock = MUI_NewObject( MUIC_Area, 0 ) ) )
			{
				return( 0 );
			}
		}
#endif /* USE_CLOCK */

		grp = HGroup, MUIA_Font, MUIV_Font_Tiny, MUIA_InnerLeft, 4, MUIA_InnerRight, 4, MUIA_InnerTop, 2, MUIA_InnerBottom, 2,
			Child, grp_info = HGroup, ButtonFrame, MUIA_Group_Spacing, 1, MUIA_Background, "2:96969696,96969696,96969696", MUIA_InputMode, MUIV_InputMode_RelVerify,
				Child, data->grp_keybm = PageGroup,
					Child, BitmapObject,
						MUIA_Bitmap_Bitmap, &keyImage2BitMap,
						MUIA_Bitmap_SourceColors, keyimage1CMap32,
						MUIA_Bitmap_UseFriend, TRUE,
						MUIA_Bitmap_Height, 6,
						MUIA_FixHeight, 6,
						MUIA_Bitmap_Width, 13,
						MUIA_FixWidth, 13,
					End,
					Child, BitmapObject,
						MUIA_Bitmap_Bitmap, &keyImage1BitMap,
						MUIA_Bitmap_SourceColors, keyimage1CMap32,
						MUIA_Bitmap_UseFriend, TRUE,
						MUIA_Bitmap_Height, 6,
						MUIA_FixHeight, 6,
						MUIA_Bitmap_Width, 13,
						MUIA_FixWidth, 13,
					End,
				End,

				Child, data->grp_httpbm = PageGroup,
					Child, ImageObject,
						MUIA_FixWidth, 8,
						MUIA_FixHeight, 6,
						MUIA_Image_FreeVert, TRUE,
						MUIA_Image_FreeHoriz, TRUE,
						MUIA_Image_Spec, "6:25",
					End,
					Child, ImageObject,
						MUIA_FixWidth, 8,
						MUIA_FixHeight, 6,
						MUIA_Image_FreeVert, TRUE,
						MUIA_Image_FreeHoriz, TRUE,
						MUIA_Image_Spec, "6:23",
					End,
					Child, BitmapObject,
						MUIA_Bitmap_Bitmap, &http11ImageBitMap,
						MUIA_Bitmap_SourceColors, http10imgCMap32,
						MUIA_Bitmap_UseFriend, TRUE,
						MUIA_Bitmap_Height, 6,
						MUIA_FixHeight, 6,
						MUIA_Bitmap_Width, 8,
						MUIA_FixWidth, 8,
					End,
					Child, BitmapObject,
						MUIA_Bitmap_Bitmap, &http10ImageBitMap,
						MUIA_Bitmap_SourceColors, http10imgCMap32,
						MUIA_Bitmap_UseFriend, TRUE,
						MUIA_Bitmap_Height, 6,
						MUIA_FixHeight, 6,
						MUIA_Bitmap_Width, 8,
						MUIA_FixWidth, 8,
					End,
					Child, BitmapObject,
						MUIA_Bitmap_Bitmap, &ftpImageBitMap,
						MUIA_Bitmap_SourceColors, http10imgCMap32,
						MUIA_Bitmap_UseFriend, TRUE,
						MUIA_Bitmap_Height, 6,
						MUIA_FixHeight, 6,
						MUIA_Bitmap_Width, 8,
						MUIA_FixWidth, 8,
					End,
				End,
			End,

			Child, HGroup,
				Child, data->txt_state = TextObject, NoFrame, MUIA_Font, MUIV_Font_Tiny, MUIA_Text_Contents, "Ready.", MUIA_Text_SetMin, FALSE, End,
				Child, data->gauge_receive = NewObject( getgaugeclass(), NULL, TAG_DONE ),
			End,
			Child, data->ledobj = NewObject( getledclass(), NULL, NoFrame, MUIA_InputMode, MUIV_InputMode_RelVerify, MUIA_Weight, 0, TAG_DONE ),
#if USE_CLOCK
			Child, data->clock,
#endif /* USE_CLOCK */
		End;

		/* open docinfowin */
		DoMethod( grp_info, MUIM_Notify, MUIA_Pressed, FALSE,
			obj, 1, MM_HTMLWin_OpenDocInfoWin
		);

		/* opens Network status when clicking on the leds */
#if USE_NET
		DoMethod( data->ledobj, MUIM_Notify, MUIA_Pressed, FALSE, app, 1, MM_App_OpenNetinfoWindow );
#endif /* USE_NET */

#if USE_TEAROFF
		if( gp_tearoff )
		{
			data->panel_status = TearOffPanelObject,
				MUIA_Weight, 0, InnerSpacing( 0, 0 ),
				MUIA_ObjectID, MAKE_ID( f_flags, 'P', '4', winnum ),
				MUIA_TearOffPanel_Contents, grp,
				MUIA_TearOffPanel_Label, "Status",
				MUIA_TearOffPanel_CanFlipShape, FALSE,
			End;

			if( data->panel_status )
			{
				DoMethod( data->bay_bottom, OM_ADDMEMBER, data->panel_status );
			}
			else
			{
				displaybeep(); /* TOFIX: we could fallback to not using TearOff */
			}
		}
		else
#endif /* USE_TEAROFF */
		{
			data->panel_status = grp;
			DoMethod( grp_contents, OM_ADDMEMBER, data->panel_status );
		}

	}

	DoMethod( app, OM_ADDMEMBER, data->winobj );

#if USE_TEAROFF
	if( gp_tearoff )
		DoMethod( (APTR)getv( data->winobj, MUIA_Window_RootObject ), MUIM_Import, tearoff_dataspace );
#endif

	// Setup notifications
	DoMethod( data->winobj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_HTMLWin_Close
	);
	DoMethod( data->winobj, MUIM_Notify, MUIA_Window_Activate, TRUE,
		obj, 1, MM_HTMLWin_SetActive
	);

	data->fastlinks_are_horiz = TRUE;
	data->toolbar_is_horiz = TRUE;

	add_window_menu_extended( obj, data->winobj, data->title, winnum, !data->f_fullscreen );
	// Setup controls
	DoMethod( obj, MM_HTMLWin_SetupToolbar );
	DoMethod( obj, MM_HTMLWin_SetupIcon );
	DoMethod( obj, MM_HTMLWin_SetupFastlinks );

	set( data->winobj, MUIA_Window_Open, TRUE );

	/*
	 * I don't know why the above notification doesn't work.
	 * MUI bug ?
	 */
	DoMethod( obj, MM_HTMLWin_SetActive );

	/*
	 * Tell historylist that we are a frameset/full HTML area
	 * and that we can set URLs globally in the history
	 */
	set( data->list_url, MA_Historylist_Standalone, TRUE );

	/* if the window is fullscreen, disable that damned window size menu */
	if (data->f_fullscreen) SetAttrs( findmenu(MENU_SETWINSIZE), MUIA_Menuitem_Enabled, FALSE );

	return( (ULONG)data->winobj );
}


DECMETHOD( HTMLWin_SetActive, ULONG )
{
	GETDATA;
	APTR o = obj;

	DoMethod( obj, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );

	lastactivepanel = data->obj;

	// Find topmost object which is of our class
	while( o )
	{
		if( OCLASS( o ) == cl )
			lastactivewin = o;
		o = _parent( o );
	}

	return( 0 );
}


DECMETHOD( HTMLWin_SelectFastlink, LONG )
{
	GETDATA;

	if( data->str_url )
	{
		char *url;

		if( msg[ 1 ] >= 0 )
			url = getprefs( DSI_FASTLINKS_URLS + msg[ 1 ] );
		else
			url = "about:";

		set( data->str_url, MUIA_String_Contents, url );
		DoMethod( obj, MM_HTMLWin_URLAcknowledge );
	}

	return( 0 );
}

#if USE_CMANAGER
static int cm_alias_searchlist( struct CMGroup *grp, char *alias, char *to )
{
	struct CMWWW *entries;
	struct CMGroup *groups;

	for( entries = FIRSTNODE( &grp->Entries ); NEXTNODE( entries ); entries = NEXTNODE( entries ) )
	{
		if( !stricmp( entries->Alias, alias ) )
		{
			stccpy( to, entries->WWW, MAXURLSIZE );
			return( TRUE );
		}
	}

	for( groups = FIRSTNODE( &grp->SubGroups ); NEXTNODE( groups ); groups = NEXTNODE( groups ) )
	{
		if( cm_alias_searchlist( groups, alias, to ) )
			return( TRUE );
	}

	return( FALSE );
}

static int cm_findalias( char *alias, char *to )
{
	struct CMData   *cmdata;
	int found = FALSE;

	if( bm_create() )
	{
		DoMethod( cm_obj, MUIM_CManager_GrabLists );

		cmdata = (struct CMData *)getv( cm_obj, MUIA_CManager_CMData );

		found = cm_alias_searchlist( cmdata->WWWs, alias, to );
	}

	return( found );
}
#endif /* USE_CMANAGER */

// URL string updated and pressed enter (or activated otherwise)
DECMETHOD( HTMLWin_URLAcknowledge, ULONG )
{
	GETDATA;
	char buffer[ MAXURLSIZE ];
	char *url = getstrp( data->str_url );

	/*
	 * strip leading and ending spaces
	 */
	if( strchr( url, ' ' ) )
	{
		char *p;

		strcpy( buffer, stpblk( url ) );

		p = buffer + strlen( buffer ) - 1;

		while( *p == ' ' )
		{
			*p-- = '\0';
		}
		nnset( data->str_url, MUIA_String_Contents, buffer );
		url = getstrp( data->str_url );
	}

	if( !strstr( url, ":" ) )
	{
		stccpy( buffer, url, sizeof( buffer ) - 32 );
		if( !buffer[ 0 ] )
			return( 0 );

		if( strpbrk( buffer, "./" ) )
		{
			if( !strncmp( buffer, "ftp.", 4 ) )
				strins( buffer, "ftp://" );
			else
				strins( buffer, "http://" );
		}
		else
		{
			/* this is an alias? */
#if USE_CMANAGER
			if ( !cm_findalias( url, buffer ) )
#endif
				/*
				 * The following is taken out temporarily. There
				 * must be something smarter as this is a real
				 * PITA for local networks.
				 */
				//sprintf( buffer, "http://www.%s.com/", url );
			{
				/* XXX: experimental for now.. */
				STRPTR p = url;
				STRPTR q = buffer + 31;
				ULONG len = MAXURLSIZE - 32; /* the size below + NULL term*/
				strcpy( buffer, "http://www.google.com/search?q=" );

				while ( *p && len)
				{
					if ( *p == ' ')
					{
						*q = '+';
					}
					else
					{
						*q = *p;
					}
					p++;
					q++;
					len--;
				}
				*q = '\0';
			}
		}
		nnset( data->str_url, MUIA_String_Contents, buffer );
		url = getstrp( data->str_url );
	}

	DoMethod( obj, MM_HTMLWin_SetURL, url, NULL, NULL, MF_HTMLWin_AddURL );

	return( 0 );

}

DECMETHOD( HTMLWin_MakeWindowTitle, APTR )
{
	GETDATA;
	if ( !(data->f_fullscreen) ) set_window_title( data->obj, data->title );
	return( 0 );
}

/* start a telnet executable */
static void starttelnet( char *url )
{
	char buff1[ 256 ], buff2[ 256 ], *p;

	while( *url == '/' )
		url++;

	stccpy( buff1, url, 256 );
	p = strchr( buff1, ':' );
	if( p )
	{
		*p++ = 0;
	}
	else
	{
		p = "23";
	}

	expandtemplate( getprefs( DSI_NET_TELNET ), buff2, 256,
		's', buff1,
		'h', buff1,
		'p', p,
		NULL
	);

#if USE_DOS
	SystemTags( buff2,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);
#endif /* USE_DOS */
}

#define MAILTMP "T:VMailTmp"

/* start a mail application executable */
#if USE_DOS && USE_NET
static void startmailto_app( char *to, char *subject )
{
	char buff2[ 256 ];

	expandtemplate( getprefs( DSI_NET_MAIL_APP ), buff2, 256,
		't', to,
		's', subject,
		'c', MAILTMP,
		NULL
	);

	SystemTags( buff2,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);
}
#endif /* USE_DOS && USE_NET */

/* starts MD2 */
#if USE_DOS && USE_NET
static void startmailto_md2( char *to, char *subject )
{
	char buff2[ 512 ];
	struct MsgPort *mp;

	Forbid();
	mp = FindPort( "MICRODOT.1" );
	if( mp )
	{
		sprintf( buff2, "NewMsgWindow To \"%s\" Subject \"%s\" Contents " MAILTMP "",
			to, subject
		);
		VAT_SendRXMsg( buff2, "MICRODOT.1", ".MDRX" );
		Permit();
		return;
	}
	Permit();

	if( GetVar( "VAPOR/MD2_LASTUSEDDIR", buff2, sizeof( buff2 ), 0 ) <= 0 )
	{
		MUI_Request( app, NULL, 0, GS( ERROR ), GS( OK ), GS( MAILTO_NOMD2 ) );
		return;
	}

	AddPart( buff2, "MicroDot", sizeof( buff2 ) );

	sprintf( strchr( buff2, 0 ), " To \"%s\" Subject \"%s\" Contents " MAILTMP "",
		to, subject
	);

	SystemTags( buff2,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);
}
#endif /* USE_DOS && USE_NET */

#if USE_NET
static void deescape( char *p )
{
	while( *p )
	{
		if( *p == '+' )
		{
			*p++ = ' ';
		}
		else if( *p == '%' )
		{
			char dat[ 4 ];
			long v;

			dat[ 0 ] = p[ 1 ];
			dat[ 1 ] = p[ 2 ];
			dat[ 2 ] = 0;

			stch_l( dat, &v );

			strcpy( p, p + 2 );
			*p++ = v;
		}
		else
			p++;
	}
}
#endif /* USE_NET */

#if USE_NET
static void startmailto( char *url, char *referer, char *contents )
{
#if USE_DOS
	int mode = getprefslong( DSI_NET_USE_MAILAPP, 0 );
#endif
	char tmp[ MAXURLSIZE ], *subject = 0, *body = 0, *p;

	stccpy( tmp, url, sizeof( tmp ) );

	strtok( tmp, "?" );
	while( p = strtok( NULL, "&" ) )
	{
		if( !strnicmp( p, "Subject=", 8 ) )
		{
			subject = p + 8;
			deescape( subject );
		}
		else if( !strnicmp( p, "Body=", 5 ) )
		{
			body = p + 5;
			deescape( body );
		}
	}

	if( !contents )
		contents = body;

	//TOFIX!!! broken atm
	//if( !subject )
	//	  sprintf( subject = ts, "URL %.108s", url );

#if USE_DOS
	if( mode )
	{
		BPTR f;

		f = Open( MAILTMP, MODE_NEWFILE );
		if( f )
		{
			if( contents )
				Write( f, contents, strlen( contents ) );
			Close( f );
		}
	}

	if( mode == 1 )
		startmailto_md2( tmp, subject );
	else if( mode == 2 )
		startmailto_app( tmp, subject );
	else
#endif /* USE_DOS */
		createmailwin( tmp, subject, contents ? contents : "" );
}
#endif /* USE_NET */

#if USE_DOS && USE_NET
static void startnews( char *what )
{
	char groupname[ 128 ];
	char msgid[ 256 ];
	char buff2[ 256 ], buff[ 256 ];
	int mode = getprefslong( DSI_NET_USE_NEWSAPP, 0 );

	if( strchr( what, '@' ) )
	{
		sprintf( msgid, "\"%s\"", what );
		strcpy( groupname, "\"\"" );
	}
	else
	{
		sprintf( groupname, "\"%s\"", what );
		strcpy( msgid, "\"\"" );
	}

	if( !mode )
	{
		if( GetVar( "VAPOR/MD2_LASTUSEDDIR", buff2, sizeof( buff2 ), 0 ) <= 0 )
		{
			MUI_Request( app, NULL, 0, GS( ERROR ), GS( OK ), GS( MAILTO_NOMD2 ) );
			return;
		}
		AddPart( buff2, "MicroDot", sizeof( buff2 ) );
		strcat( buff2, " MessageID %m GroupName %g" );
	}
	else
		strcpy( buff2, getprefs( DSI_NET_NEWS_APP ) );

	expandtemplate( buff2, buff, 256,
		'g', groupname,
		'm', msgid,
		NULL
	);

	SystemTags( buff2,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);
}
#endif /* USE_DOS && USE_NET */

/*
 * Cleans up the possible temporary strings from
 * MM_HTMLWin_SetURL. Only called from it.
 */
DECSMETHOD( HTMLWin_SetURLCleanup )
{
	if( msg->flags & MF_HTMLWin_FreeURL )
	{
		free( msg->url );
	}

	if( msg->flags & MF_HTMLWin_FreeReferer )
	{
		free( msg->referer );
	}

	if( msg->flags & MF_HTMLWin_FreeTarget )
	{
		free( msg->target );
	}

	return( 0 );
}

/*
 * Sets up loading of an URL into this window.
 */
DECSMETHOD( HTMLWin_SetURL )
{
	GETDATA;
	char *url = stpblk( msg->url );

	if( !*url )
		return( 0 );

	// check for special URLs
	if( !strnicmp( url, "telnet:", 7 ) )
	{
		starttelnet( url + 7 );
		if( data->str_url )
			nnset( data->str_url, MUIA_String_Contents, data->doc_main ? nets_url( data->doc_main ) : "" );
		DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
		return( 0 );
	}
	else if( !strnicmp( url, "javascript:", 11 ) )
	{
		char resultbuffer[ 1024 ];
		char *sp = stpblk( url + 11 );

		// Some hackery for dummy scripts
		if( !*sp || !strnicmp( sp, "VOID", 4 ) )
			return( 0 );

		if( !data->jso )
			data->jso = jso_init();

		cjsol = data->jso;
		js_run(
			obj,
			obj,
			MYURL,
			sp,
			strlen( sp ),
			0,
			resultbuffer,
			sizeof( resultbuffer ),
			MYURL
		);

		if( data->str_url )
			nnset( data->str_url, MUIA_String_Contents, data->doc_main ? nets_url( data->doc_main ) : "" );
		DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
		return( 0 );
	}
#if USE_NET
	else if( !strnicmp( url, "mailto:", 7 ) )
	{
		startmailto( url + 7, msg->referer, NULL );
		if( data->str_url )
			nnset( data->str_url, MUIA_String_Contents, data->doc_main ? nets_url( data->doc_main ) : "" );
		DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
		return( 0 );
	}
#if USE_DOS
	else if( !strnicmp( url, "news:", 7 ) )
	{
		startnews( url + 7 );
		if( data->str_url )
			nnset( data->str_url, MUIA_String_Contents, data->doc_main ? nets_url( data->doc_main ) : "" );
		DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
		return( 0 );
	}
#endif /* USE_DOS */
#endif /* USE_NET */

	if( !url_hasscheme( msg->url ) && msg->url[ 0 ] )
	{
		char buffer[ MAXURLSIZE ];
		sprintf( buffer, "http://%s", msg->url );
		if( msg->flags & MF_HTMLWin_FreeURL )
			free( msg->url );
		msg->url = strdup( buffer );
		msg->flags |= MF_HTMLWin_FreeURL;
	}

	// We need to check the target..

	if( msg->target && strcmp( msg->target, data->name ) && strcmp( msg->target, "_self" ) )
	{
		APTR topwin = obj, o = obj;

		// Find the top window
		while( o && _parent( o ) )
		{
			if( OCLASS( o ) == OCLASS( obj ) )
				topwin = o;
			o = _parent( o );
		}

		if( !strcmp( msg->target, "_top" ) )
		{
			msg->target = NULL;
			DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
			return( DoMethodA( topwin, (Msg)msg ) );
		}
		else if( !strcmp( msg->target, "_parent" ) )
		{
			o = _parent( obj );

			// Find the parent window
			while( o )
			{
				if( OCLASS( o ) == OCLASS( obj ) )
					break;
				o = _parent( o );
			}

			if( !o )
				goto doit;

			msg->target = NULL;
			DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
			return( DoMethodA( o, (Msg)msg ) );
		}

		o = NULL;
		DoMethod( topwin, MM_HTMLWin_FindByName, msg->target, &o );
		if( !o )
			o = win_find_by_name( msg->target, FALSE );
		if( o )
		{
			DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
			return( DoMethodA( o, (Msg)msg ) );
		}
		else
		{
			// Create new window..
			win_create(
				strcmp( msg->target, "_blank" ) ? (STRPTR)msg->target : (STRPTR)"unnamed",
				msg->url,
				msg->referer,
				obj,
				FALSE,
				( msg->flags & MF_HTMLWin_Reload ) ? TRUE : FALSE,
				FALSE
			);

			DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
			return( 0 );
		}
	}

doit:
	// Previously loading document?
	if( data->doc_loading )
	{
		nets_close( data->doc_loading );
		data->doc_loading = NULL;
	}

	// Check for an anchor
	if( msg->url )
	{
		if( strchr( msg->url, '#' ) || strchr( MYURL, '#' ) )
		{
			char buffer1[ MAXURLSIZE ], *p1;
			char buffer2[ MAXURLSIZE ], *p2;

			strcpy( buffer1, msg->url );
			strcpy( buffer2, MYURL );

			p1 = strrchr( buffer1, '#' );
			if( p1 )
				*p1++ = 0;
			p2 = strrchr( buffer2, '#' );
			if( p2 )
				*p2++ = 0;

			if( !strcmp( buffer1, buffer2 ) )
			{
				// Ok, it *IS* the same URL, safe the anchors
				int ypos = 0;

				if( p1 )
					DoMethod( obj, MM_Layout_Anchor_FindByName, p1, &ypos );

				set_smooth_scroll( data->v, FALSE );
				set( data->v, MUIA_Virtgroup_Top, ypos );
				set_smooth_scroll( data->v, TRUE );

				if( data->str_url )
					nnset( data->str_url, MUIA_String_Contents, msg->url );

				if( msg->flags & MF_HTMLWin_AddURL )
				{
					DoMethod( data->list_url, MM_Historylist_AddURL, msg->url );
				}

				DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
				return( 0 );
			}
		}
	}

	/*
	 * Abort everything that the page was loading (images, etc..)
	 */
	if( data->doc_main )
	{
		nets_abort( data->doc_main );
		DoMethod( data->v, MM_HTMLView_AbortLoad );
	}

	if( msg->url && msg->url[ 0 ] )
	{
		data->doc_loading = nets_open(
			msg->url,
			msg->referer,
			obj,
			data->gauge_receive,
			obj,
			0,
			( ( msg->flags & MF_HTMLWin_Reload ) ? NOF_RELOAD : 0 ) | NOF_ADDURL
		);

		if( data->str_url )
		{
			STRPTR dispurl = NULL;
			if( strchr( msg->url, 0xBF ) )
			{
				STRPTR lastbrace;
				dispurl = strdup( msg->url ); /* TOFIX */
				lastbrace = strrchr( dispurl, '?' );
				if( lastbrace )
					*lastbrace='\0';
			}
			nnset( data->str_url, MUIA_String_Contents, dispurl ? dispurl : msg->url );
		}

		DoMethod( obj, MM_HTMLWin_SetDone, FALSE );
		if( msg->flags & MF_HTMLWin_AddURL )
		{
			DoMethod( data->list_url, MM_Historylist_AddURL, msg->url );
		}
	}

	DoMethod( obj, MM_HTMLWin_SetURLCleanup, msg->url, msg->referer, msg->target, msg->flags );
	return( 0 );
}


//
// Handle network info replies
//
DECMETHOD( NStream_GotInfo, APTR )
{
	GETDATA;

	if( data->doc_loading )
	{
		int imageloadmode = 0;
		int viewmode = 0;

		if( data->doc_loading->un && !data->doc_loading->un->dcn && data->doc_loading->un->retr_mode < 2 && !data->doc_loading->un->fromcache )
		{
			/*
			 * DNS query failed. We handle the case that only
			 * HTTP/FTP without cache can do that :)
			 */
			DoMethod( data->list_url, MM_Historylist_Undo, FALSE );
		}
		else
		{
			/*
			 * Socket error.
			 */
			if( nets_state( data->doc_loading ) == -1 )
			{
				DoMethod( data->list_url, MM_Historylist_Undo, FALSE );
			}
		}

		if( nets_redirecturl( data->doc_loading ) )
		{
			char buffer[ MAXURLSIZE ];
			struct nstream *nn;

			uri_mergeurl( nets_url( data->doc_loading ), nets_redirecturl( data->doc_loading ), buffer );

			if( data->str_url )
				nnset( data->str_url, MUIA_String_Contents, buffer );

			nn = nets_open( buffer, nets_referer( data->doc_loading ), obj, data->gauge_receive, obj, 0, 0 );
			nets_close( data->doc_loading );
			data->doc_loading = nn;

			return( 0 );
		}

		if( nets_state( data->doc_loading ) && nets_errorstring( data->doc_loading )[ 0 ] )
		{
			/*
			 * Various errors
			 */
			DoMethod( obj, MM_HTMLWin_SetDone, TRUE );

			nets_close( data->doc_loading );
			data->doc_loading = NULL;
			DoMethod( obj, MM_HTMLWin_SetTxt, "Ready." );

			return( 0 );
		}

		if( data->grp_keybm )
		{
			set( data->grp_keybm, MUIA_Group_ActivePage, nets_issecure( data->doc_loading ) );
			set( data->grp_httpbm, MUIA_Group_ActivePage, nets_sourceid( data->doc_loading ) );
		}

		if( getmenucheck( MENU_SET_LOADIMAGES_IMAPS ) )
			imageloadmode = 2;
		else if( getmenucheck( MENU_SET_LOADIMAGES_ALL ) )
			imageloadmode = 1;

		// as soon as we have some info from the URL, we can start and display it
		mime_findbytype( nets_mimetype( data->doc_loading ), NULL, NULL, &viewmode );

		addurlhistory( nets_url( data->doc_loading ) );

		/*
		 * Check if a file is for downloading
		 */
#if USE_PLUGINS
		if( !plugin_mimetype( nets_mimetype( data->doc_loading ), nets_mimeextension( data->doc_loading ) ) )
#endif
		{
#if USE_NET
			if( strnicmp( nets_mimetype( data->doc_loading ), "text/", 5 ) && ( strnicmp( nets_mimetype( data->doc_loading ), "image/", 6 ) || !( viewmode & MF_VIEW_INLINE ) ) )
			{
				DoMethod( data->list_url, MM_Historylist_Undo, TRUE );
				queue_download( nets_url( data->doc_loading ), NULL, FALSE, FALSE );
				nets_close( data->doc_loading );

				data->doc_loading = NULL;

				DoMethod( obj, MM_HTMLWin_SetDone, TRUE );
				if( data->doc_main && data->str_url )
					nnset( data->str_url, MUIA_String_Contents, nets_url( data->doc_main ) );
				killpushedmethods( data->gauge_receive );
				killpushedmethods( data->txt_state );
				DoMethod( obj, MM_HTMLWin_SetTxt, GS( INFOLINE_DONE ) );
				if( data->gauge_receive )
					DoMethod( data->gauge_receive, MM_Gauge_Clear );
				return( 0 );
			}
#endif /* USE_NET */
		}
		nets_settomem( data->doc_loading );

		DoMethod( obj, MUIM_Group_InitChange );

		if( data->doc_main )
		{
			DoMethod( data->v, MM_HTMLView_ShowNStream, NULL );
			nets_close( data->doc_main );
		}

		data->doc_main = data->doc_loading;
		data->doc_loading = NULL;

		if( data->jso )
			jso_cleanup( data->jso );

		data->jso = jso_init();

		data->ix_onload = 0;
		data->ix_onunload = 0;
		data->ix_onerror = 0;

		DoMethod( obj, MM_HTMLWin_CleanupTimers );

		cp_flush( &data->customprops );
		data->js_frames = NULL;

		if( !data->name[ 0 ] )
			strcpy( data->name, "_top" );

		stccpy( data->title, FilePart( nets_url( data->doc_main ) ), sizeof( data->title ) );

		DoMethod( obj, MM_HTMLWin_MakeWindowTitle );

		// Reset pointer
		data->mousevisible = TRUE;
		DoMethod( obj, MM_HTMLWin_SetPointer, POINTERTYPE_NORMAL );

		// Attach nstream to view object now
		DoMethod( data->v, MM_HTMLView_ShowNStream, data->doc_main );

		if( !gp_cacheimg )
			imgdec_flushimages();

		DoMethod( obj, MUIM_Group_ExitChange );
	}

	return( 0 );
}

DECMETHOD( NStream_Done, APTR )
{
	GETDATA;
	return( DoMethodA( data->v, (Msg)msg ) );
}

DECMETHOD( NStream_GotData, APTR )
{
	GETDATA;
	return( DoMethodA( data->v, (Msg)msg ) );
}


//
// Set status text (from networking etc.)
//

DECMETHOD( HTMLWin_SetTxt, STRPTR )
{
	GETDATA;

	if( data->txt_state )
	{
		if( strcmp( (char*)getv( data->txt_state, MUIA_Text_Contents ), msg[ 1 ] ) )
		{
			set( data->txt_state, MUIA_Text_Contents, filter_escapecodes( msg[ 1 ] ) );
		}
	}

	stccpy( data->lasttext, msg[ 1 ], sizeof( data->lasttext ) );

	return( 0 );
}

/* open the Document Information Window */
DECMETHOD( HTMLWin_OpenDocInfoWin, APTR )
{
	GETDATA;

	if( data->doc_main )
	{
		createdocinfowin( data->doc_main->url );
	}
	else
		displaybeep();

	return( 0 );
}

/* open the Document Information Window */
DECMETHOD( HTMLWin_ShowSource, APTR )
{
	GETDATA;

	if( data->doc_main && !strnicmp( nets_mimetype( data->doc_main ), "text/", 5 ) )
	{
		opensourceview( data->doc_main );
	}
	else
		displaybeep();

	return( 0 );
}


// Start/Stop Transfer Icon anim
DECMETHOD( HTMLWin_SetDone, ULONG )
{
	GETDATA;

	if( !msg[ 1 ] )
	{
		DoMethod( obj, MM_HTMLWin_EnableButtonByFunc, "Stop", TRUE );
		DoMethod( obj, MM_HTMLWin_EnableButtonByFunc, "LoadImages", FALSE );
		DoMethod( obj, MM_HTMLWin_EnableButtonByFunc, "Print", FALSE );

		if( data->iconobj )
			set( data->iconobj, MA_Amicon_Active, TRUE );
	}
	else
	{
		ULONG imgcount = 0;
		ULONG framecount = 0;
		ULONG othercount = 0;

		// TOFIX! add new method in htmlview for this!
		DoMethod( data->v, MM_Layout_CalcUnloadedObjects, &imgcount, &framecount, &othercount );

		DoMethod( obj, MM_HTMLWin_EnableButtonByFunc, "Stop", FALSE );
		DoMethod( obj, MM_HTMLWin_EnableButtonByFunc, "LoadImages", imgcount ? TRUE : FALSE );
		DoMethod( obj, MM_HTMLWin_EnableButtonByFunc, "Print", TRUE );

		if( data->iconobj )
			set( data->iconobj, MA_Amicon_Active, FALSE );
		if( data->gauge_receive )
			DoMethod( data->gauge_receive, MM_Gauge_Clear );
	}

	return( 0 );
}

DECTMETHOD( HTMLWin_CheckDone )
{
	GETDATA;
	char buff[ 256 ];
	int imgcount = 0;
	int framecount = 0;
	int othercount = 0;

	if( !data->winobj )
	{
		// Forward upwards to "real" window..
		APTR lasto = NULL, o = _parent( obj );

		while( o )
		{
			if( OCLASS( o ) == cl )
				lasto = o;
			o = _parent( o );
		}
		if( lasto )
			return( DoMethodA( lasto, (Msg)msg ) );
	}

	DoMethod( obj, MM_Layout_CalcUnloadedObjects, &imgcount, &framecount, &othercount );

	if( imgcount > 0 || framecount > 0 || othercount > 0 )
	{
		int hadone = FALSE;

		strcpy( buff, GS( INFOLINE_PREFIX ) );
		if( imgcount > 0 )
		{
			sprintf( strchr( buff, 0 ), imgcount == 1 ? GS( INFOLINE_IMAGES_SINGLE) : GS( INFOLINE_IMAGES ), imgcount );
			hadone = TRUE;
		}
		if( framecount > 0 )
		{
			if( hadone )
				strcat( buff, ", " );
			sprintf( strchr( buff, 0 ), framecount == 1 ? GS( INFOLINE_FRAMES_SINGLE ) : GS( INFOLINE_FRAMES ), framecount );
			hadone = TRUE;
		}
		if( othercount > 0 )
		{
			if( hadone )
				strcat( buff, ", " );
			sprintf( strchr( buff, 0 ), othercount == 1 ? GS( INFOLINE_OBJECTS_SINGLE ) : GS( INFOLINE_OBJECTS ), othercount );
		}

		DoMethod( obj, MM_HTMLWin_SetTxt, buff );
	}
	else
	{
		DoMethod( obj, MM_HTMLWin_SetDone, TRUE );
		DoMethod( obj, MM_HTMLWin_SetTxt, GS( INFOLINE_DONE ) );
	}

	return( 0 );
}


DECMETHOD( HTMLWin_DoToolbutton, ULONG )
{
#if USE_SPEEDBAR //TOFIX !!
	GETDATA;
	int v;
	int num = msg[ 1 ];


	DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, num, OM_GET, MUIA_Disabled, &v );
	if( v )
		return( 0 );

	execute_command( data->but_acts[ num ], getprefsstr( DSI_BUTTONS_ARGS + msg[ 1 ], "" ), VREXX_WINDOW, MYURL, MYURL, MYURL );

#if 0 //TOFIX!! This will go away once all is implemented
	switch( data->but_acts[ num ] )
	{
		case bfunc_back:
			DoMethod( obj, MM_HTMLWin_Backward );
			break;

		case bfunc_forward:
			DoMethod( obj, MM_HTMLWin_Forward );
			break;

		case bfunc_find:
			DoMethod( obj, MM_HTMLWin_SetURL, "search:", NULL, NULL, MF_HTMLWin_AddURL );
			break;

		case bfunc_home:
			DoMethod( obj, MM_HTMLWin_SetURL, getprefsstr( DSI_HOMEPAGE, "about:empty" ), NULL, NULL, MF_HTMLWin_AddURL );
			break;

		case bfunc_loadimages:
			DoMethod( data->v, MM_HTMLView_LoadInlineGfx );
			break;

		case bfunc_print:
			//CallHook( &h, NULL, VF_PRINT );
			break;

		case bfunc_reload:
			if( data->doc_main )
			{
				char *url

				if( ( url = strdup( nets_url( data->doc_main ) ) ) )
				{
					DoMethod( data->v, MM_HTMLView_ShowNStream, NULL );

					nets_close( data->doc_main );
					data->doc_main = NULL;

					DoMethod( obj, MM_HTMLWin_SetURL, url, NULL, NULL, MF_HTMLWin_Reload );

					free( url );
				}
				else
				{
					displaybeep();
				}
			}
			break;

		case bfunc_stop:
			DoMethod( obj, MM_HTMLWin_StopXfer );
			break;

		case bfunc_rexx:
			VAT_SendRXMsg( getprefsstr( DSI_BUTTONS_ARGS + msg[ 1 ], "" ), VREXXPORT, VREXXEXT );
			break;

		case bfunc_js:
			{
				//APTR htmlviewobj = data->vobj;
				//char buffer[ 512 ];

				//DoMethod( htmlviewobj, MM_HTMLView_ExecJS, data->win, getprefsstr( DSI_BUTTONS_ARGS + msg[ 1 ], "" ), buffer, sizeof( buffer ) );
			}
			break;
	}
#endif /* 0 */

#endif /* USE_SPEEDBAR */

	return( 0 );
}


DECMETHOD( HTMLWin_Reload, ULONG ) //TOFIX!! should that move to HTMLView ?
{
	GETDATA;

	if( data->doc_main )
	{
		char *url;

		if( (  url = strdup( nets_url( data->doc_main ) ) ) )
		{
			DoMethod( obj, MM_HTMLWin_CleanupTimers );
			DoMethod( data->v, MM_HTMLView_ShowNStream, NULL );

			nets_close( data->doc_main );
			data->doc_main = NULL;

			DoMethod( obj, MM_HTMLWin_SetURL, url, NULL, NULL, MF_HTMLWin_Reload );

			free( url );
		}
		else
		{
			displaybeep();
		}
	}
	return( 0 );
}


DECMETHOD( HTMLWin_StopXfer, ULONG )
{
	GETDATA;

	if( data->doc_loading )
	{
		//nets_close( data->doc_loading );
		DoMethod( data->list_url, MM_Historylist_Undo, TRUE );
		data->doc_loading = NULL;
	}

	if( data->doc_main )
	{
		if( data->str_url )
			nnset( data->str_url, MUIA_String_Contents, nets_url( data->doc_main ) );
		nets_abort( data->doc_main );
		DoMethod( data->v, MM_HTMLView_AbortLoad );
	}

	DoMethod( obj, MM_HTMLWin_SetDone, TRUE );

	pushmethod( obj, 2, MM_HTMLWin_SetTxt, "User aborted." );
	if( data->gauge_receive )
		DoMethod( data->gauge_receive, MM_Gauge_Clear );

	killpushedmethods( obj );
	killpushedmethods( data->gauge_receive );

	return( DOSUPER );
}

DECMETHOD( HTMLWin_Forward, ULONG )
{
	GETDATA;

	// TOFIX! needs handling of embedded frames
	return( DoMethod( data->list_url, MM_Historylist_Next ) );
}

DECMETHOD( HTMLWin_Backward, ULONG )
{
	GETDATA;
	// TOFIX! needs handling of embedded frames
	return( DoMethod( data->list_url, MM_Historylist_Back ) );
}

DECSMETHOD( HTMLWin_EnableButtonByFunc )
{
#if USE_SPEEDBAR // TOFIX!!
	GETDATA;
	int newstate = !msg->state; /* FALSE/TRUE */
	int c;


	for( c = 0; c < data->numbrushes; c++ )
	{
		if( match_command( data->but_acts[ c ], data->but_args[ c ], msg->function ) )
		{
			if( data->but_states[ c ] != newstate )
				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Set, MUIA_Disabled, newstate );
			data->but_states[ c ] = newstate;
		}
	}

#endif /* USE_SPEEDBAR */

	return( 0 );
}

DECSMETHOD( HTMLWin_BuildButtonHistory )
{
#if USE_SPEEDBAR // TOFIX!!
	GETDATA;
	struct MinList *l, *bflist;
	APTR o, ostate;
	APTR m_forward, m_backward;
	int c, num, act;
	struct history_mainpage	*mp;
	int hasback = FALSE;
	int hasnext = FALSE;


	if( data->menu_forward )
	{
		MUI_DisposeObject( data->menu_forward );
	}
	data->menu_forward = MenustripObject, End;

	if( data->menu_backward )
	{
		MUI_DisposeObject( data->menu_backward );
	}
	data->menu_backward = MenustripObject, End;

	get( data->menu_forward, MUIA_Family_List, &l );
	ostate = FIRSTNODE( l );
	while( o = NextObject( &ostate ) )
	{
		DoMethod( data->menu_forward, MUIM_Family_Remove, o );
		MUI_DisposeObject( o );
	}

	get( data->menu_backward, MUIA_Family_List, &l );
	ostate = FIRSTNODE( l );
	while( o = NextObject( &ostate ) )
	{
		DoMethod( data->menu_backward, MUIM_Family_Remove, o );
		MUI_DisposeObject( o );
	}

	// Build menus
	m_forward = MenuObject, MUIA_Menu_Title, "Forward", End;
	m_backward = MenuObject, MUIA_Menu_Title, "Backward", End;

	DoMethod( data->menu_forward, MUIM_Family_AddTail, m_forward );
	DoMethod( data->menu_backward, MUIM_Family_AddTail, m_backward );

	set( data->list_url, MA_Historylist_BFReadLock, TRUE );

	/* WRONG !!! */
	num = getv( data->list_url, MA_Historylist_Entries );
	act = getv( data->list_url, MA_Historylist_CurrentEntry );
	bflist = ( struct MinList * )getv( data->list_url, MA_Historylist_BFList );

	mp = FIRSTNODE( bflist );

	for( c = 0; c < num; mp = NEXTNODE( mp ), c++ )
	{
		if( c == act )
			continue;

		if( !mp )
			break;

		o = MenuitemObject, MUIA_Menuitem_Title, mp->url, MUIA_UserData, c, End;

		if( c < act )
		{
			DoMethod( m_backward, MUIM_Family_AddHead, o );
			hasback = TRUE;
		}
		else
		{
			DoMethod( m_forward, MUIM_Family_AddTail, o );
			hasnext = TRUE;
		}
	}

	for( c = 0; c < data->numbrushes; c++)
	{
		if( match_command( data->but_acts[ c ], data->but_args[ c ], "GoForward" ) )
		{
			struct MinList *l = ( struct MinList * )getv( m_forward, MUIA_Family_List );
			if( !ISLISTEMPTY( l ) )
			{
				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Notify, MUIA_ContextMenuTrigger, MUIV_EveryTime,
					obj, 2, MM_HTMLWin_GotoButtonHistory, MUIV_TriggerValue
				);
			}
		}
		else if( match_command( data->but_acts[ c ], data->but_args[ c ], "GoBackward" ) )
		{
			struct MinList *l = ( struct MinList * )getv( m_backward, MUIA_Family_List );
			if( !ISLISTEMPTY( l ) )
			{
				DoMethod( data->bar_tool, MUIM_SpeedBar_DoOnButton, c, MUIM_Notify, MUIA_ContextMenuTrigger, MUIV_EveryTime,
					obj, 2, MM_HTMLWin_GotoButtonHistory, MUIV_TriggerValue
				);
			}
		}
	}

	set( data->list_url, MA_Historylist_BFReadLock, FALSE );

	switch( msg->butnum )
	{
		case bfunc_forward:
			return( hasnext ? ( ULONG )data->menu_forward : 0 );
		case bfunc_back:
			return( hasback ? ( ULONG )data->menu_backward : 0 );
		default:
			return( 0 );
	}
#else
	return( 0 );//TOFIX!!
#endif /* !USE_SPEEDBAR */

}

DECMETHOD( HTMLWin_GotoButtonHistory, APTR )
{
	GETDATA;
	APTR menuitem = msg[ 1 ];

	pushmethod( data->list_url, 2, MM_Historylist_GotoEntry, muiUserData( menuitem ) );

	return( 0 );
}

DECMETHOD( HTMLWin_SetTitle, STRPTR )
{
	GETDATA;

	stccpy( data->title, msg[ 1 ], sizeof( data->title ) );
	return( DoMethod( obj, MM_HTMLWin_MakeWindowTitle ) );
}

DECSMETHOD( ImgDecode_HasInfo )
{
	GETDATA;
	char imginfo[ 64 ];

	/*
	 * Unecessary but better safe than sorry. If no one complained
	 * there about some months, just remove the code [05.06.2001]
	 */
	imgdec_getinfostring( data->imgdec, imginfo ); /* TOFIX: now what happens if we have real panels ? */

	DoMethod( obj, MM_HTMLWin_SetTitle, ( ULONG )imginfo );

	return( 0 );
}

DECSMETHOD( ImgDecode_GotScanline )
{
	return( 0 );
}

DECTMETHOD( ImgDecode_Done )
{
	return( 0 );
}


DECMETHOD( HTMLWin_SetTempStatus, STRPTR )
{
	GETDATA;

	// We're a frame, forward upwards
	if( !data->winobj )
	{
		// Find next
		do
		{
			obj = _parent( obj );
		} while( obj && OCLASS( obj ) != cl );
		if( obj )
			return( DoMethodA( obj, (Msg)msg ) );
	}

	if( data->txt_state )
	{
		set( data->txt_state, MUIA_Text_Contents, filter_escapecodes( msg[ 1 ] ) );
	}

	return( 0 );
}

DECMETHOD( HTMLWin_ResetStatus, STRPTR )
{
	GETDATA;

	// We're a frame, forward upwards
	if( !data->winobj )
	{
		// Find next
		do
		{
			obj = _parent( obj );
		} while( obj && OCLASS( obj ) != cl );
		if( obj )
			return( DoMethodA( obj, (Msg)msg ) );
	}

	if( data->txt_state )
	{
		set( data->txt_state, MUIA_Text_Contents, data->lasttext );
	}

	return( 0 );
}

/*
 * Forwarder
 */
DECMETHOD( HTMLWin_LoadInlineGfx, ULONG )
{
	GETDATA;

	DoMethod( data->v, MM_HTMLView_LoadInlineGfx );

	return( 0 );
}

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	data->li.xs = msg->suggested_width;
	data->li.ys = msg->suggested_height;
	return( (ULONG)&data->li );
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;

	data->li.minwidth = 1;
	data->li.minheight = 1;
	data->li.defwidth = msg->suggested_width;
	data->li.defheight = msg->suggested_height;

	return( (ULONG)&data->li );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		STOREATTR( MA_Layout_Info, &data->li );
		STOREATTR( MA_Layout_MarginLeft, data->innermargin_left );
		STOREATTR( MA_Layout_MarginRight, data->innermargin_right );
		STOREATTR( MA_Layout_MarginTop, data->innermargin_top );
		STOREATTR( MA_Layout_MarginBottom, data->innermargin_bottom );
		STOREATTR( MA_HTMLWin_Name, data->name )
		STOREATTR( MA_JS_Name, data->name )
		STOREATTR( MA_HTMLWin_MUIWindow, data->winobj )
		STOREATTR( MA_HTMLWin_CJSOL, data->jso )

		case MA_JS_ClassName:
			*msg->opg_Storage = (ULONG)( data->winobj ? "window" : "frame" );
			return( TRUE );

		case MA_JS_Window_URL:
			*msg->opg_Storage = (ULONG)( MYURL );
			return( TRUE );

		case MA_HTMLWin_FullScreen:
			*msg->opg_Storage = (ULONG) data->f_fullscreen;
			return( TRUE );
	}

	return( DOSUPER );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

DECSMETHOD( HTMLWin_FindByName )
{
	GETDATA;

	if( !strcmp( msg->name, data->name ) )
	{
		*msg->obj = obj;
		return( TRUE );
	}
	else
		return( DOSUPER );
}

DECMMETHOD( Draw )
{
	GETDATA;

	if( data->drawframe )
	{
		SetAPen( _rp( obj ), _pens( obj )[ MPEN_SHADOW ] );
		if( data->drawframe & MV_HTMLWin_DrawFrame_Top )
		{
			HLine( _rp( obj ), _left( obj ) - 1, _right( obj ) + 1, _top( obj ) - 1 );
		}
		if( data->drawframe & MV_HTMLWin_DrawFrame_Left )
		{
			VLine( _rp( obj ), _left( obj ) - 1, _top( obj ) - 1, _bottom( obj ) + 1 );
		}
		SetAPen( _rp( obj ), _pens( obj )[ MPEN_SHINE ] );
		if( data->drawframe & MV_HTMLWin_DrawFrame_Bottom )
		{
			HLine( _rp( obj ), _left( obj ) - 1, _right( obj ) + 1, _bottom( obj ) + 1 );
		}
		if( data->drawframe & MV_HTMLWin_DrawFrame_Right )
		{
			VLine( _rp( obj ), _right( obj ) + 1, _top( obj ) - 1, _bottom( obj ) + 1 );
		}
	}

	return( DOSUPER );
}


DECSET
{
	GETDATA;

	doset( obj, data, msg->ops_AttrList );

	return( DOSUPER );
}

DECTMETHOD( HTMLWin_AddBM )
{
#if USE_CMANAGER
	GETDATA;
	addtobookmarks(
		getstrp( data->str_url ), data->title
	);
#endif /* USE_CMANAGER */
	return( 0 );
}

DECSMETHOD( Layout_CalcUnloadedObjects )
{
	GETDATA;

	if( data->doc_loading || ( data->doc_main && !nets_state( data->doc_main ) ) )
		*msg->cnt_frames = *msg->cnt_frames + 1;

	return( DOSUPER );
}

DECSMETHOD( HTMLWin_ExecuteEvent )
{
	GETDATA;
	int rc = 0;

	if( msg->funcix )
	{
		char resultbuffer[ 512 ];

		if( !data->jso )
			data->jso = jso_init();

		D( db_js, bug( "executing event handler for %x, offset %d\n", obj, msg->funcix ) );
		resultbuffer[ 0 ] = 0;
		js_interpret( obj, data->v, msg->this, MYURL,
			data->jso, msg->funcix,
			resultbuffer, sizeof( resultbuffer ),
			MYURL
		);
		D( db_js, bug( "event handler returned %s\n", resultbuffer ) );
		rc = atoi( resultbuffer );
		if( !rc )
		{
			if( !stricmp( resultbuffer, "false" ) )
				rc = 1;
		}

		if( ( data->timer_iter++ % 10 ) == 0 )
		{
			js_gc();
		}
	}

	return( (ULONG)rc );
}

/*
 * Executes the following method array on the
 * frame set by ContextMenuChoice
 */
DECSMETHOD( HTMLWin_DoActiveFrame )
{
	ULONG retval;
	retval = DoMethodA( rexx_obj ? rexx_obj : obj, msg->msg );

	rexx_obj = NULL;

	return( retval );
}

DECSMETHOD( JS_FindByName )
{
	GETDATA;

	if( !strcmp( msg->name, data->name ) )
		return( (ULONG)obj );
	else
		return( 0 );
}

DECTMETHOD( HTMLWin_StartTimers )
{
	GETDATA;
	if( !data->timer_active )
	{
		data->js_ihn.ihn_Object = obj;
		data->js_ihn.ihn_Flags = MUIIHNF_TIMER;
		data->js_ihn.ihn_Millis = 100;
		data->js_ihn.ihn_Method = MM_HTMLWin_TriggerTimers;
		DoMethod( app, MUIM_Application_AddInputHandler, ( ULONG )&data->js_ihn );
		data->timer_active = TRUE;
	}
	return( 0 );
}

DECTMETHOD( HTMLWin_TriggerTimers )
{
	GETDATA;
	struct js_timer *jt, *jtn;

	if( data->timer_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->js_ihn );
		data->timer_active = FALSE;
	}

	for( jt = FIRSTNODE( &data->js_timers ); jtn = NEXTNODE( jt ); jt = jtn )
	{
		// This timer has been killed by clearTimeout/clearIntervall
		if( jt->killed )
		{
			REMOVE( jt );
			free( jt );
			continue;
		}

		jt->millis -= 100;
		if( jt->millis < 0 )
		{
			int old_instsize = data->jso->instsize;
			int myfuncoffset = 0;

			// Timer triggered
			if( jt->funcoffset <= 0 )
			{
				myfuncoffset = js_compile( obj, data->jso, jt->a.txt, strlen( jt->a.txt ), 0, MYURL );
			}
			else
			{
				myfuncoffset = js_createfunccall( data->jso, jt->funcoffset, jt->numargs, jt->a.args );
			}
			if( myfuncoffset > 0 )
			{
				char buffer[ 512 ];

				js_interpret( data->v, obj, obj, MYURL, data->jso, myfuncoffset, buffer, sizeof( buffer ), MYURL );
			}
			else
			{
				jt->killed = TRUE;
			}
			if( !jt->repeatflag )
			{
				jt->killed = TRUE;
			}
			else
			{
				jt->millis = jt->init_millis;
			}
			data->jso->instsize = old_instsize;
		}
	}

	if( !ISLISTEMPTY( &data->js_timers ) )
		DoMethod( obj, MM_HTMLWin_StartTimers );

	if( ( data->timer_iter++ % 10 ) == 0 )
	{
		js_gc();
	}

	return( 0 );
}

DECTMETHOD( HTMLWin_CleanupTimers )
{
	GETDATA;
	struct js_timer *jt;

	if( data->timer_active )
	{
		DoMethod( app, MUIM_Application_RemInputHandler, ( ULONG )&data->js_ihn );
		data->timer_active = FALSE;
	}
	while( jt = REMHEAD( &data->js_timers ) )
	{
		free( jt );
	}
	return( 0 );
}

DECTMETHOD( HTMLWin_OpenFile )
{
	GETDATA;

	struct FileRequester *fr;
	struct Screen *scr;
	static char tmp[ 300 ];
	static char lastopendir[ 256 ];
	STRPTR fp;

	get( data->winobj, MUIA_Window_Screen, &scr );

	strcpy( tmp, lastopendir );

	fp = FilePart( tmp );

	if( fp )
	{
		*fp = '\0';
	}

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_TitleText, GS( WIN_OPENLOCALFILE ),
		ASLFR_Screen, scr,
		ASLFR_InitialDrawer, tmp,
		ASLFR_InitialFile, FilePart( lastopendir ),
		ASLFR_InitialPattern, "#?",
		ASLFR_DoPatterns, TRUE,
		ASLFR_RejectIcons, TRUE,
		TAG_DONE
	);

	if( !fr )
		return( 0 );

	set( app, MUIA_Application_Sleep, TRUE );

	if( MUI_AslRequestTags( fr, TAG_DONE ) )
	{
		strcpy( lastopendir, fr->fr_Drawer );
		AddPart( lastopendir, fr->fr_File, 256 );

		sprintf( tmp, "file:///%s", lastopendir );

		pushmethod( obj, 5, MM_HTMLWin_SetURL, tmp, NULL, NULL, MF_HTMLWin_Reload | MF_HTMLWin_AddURL );
	}
	MUI_FreeAslRequest( fr );

	set( app, MUIA_Application_Sleep, FALSE );

	return( 0 );
}

#if USE_TEAROFF
/* export the TearOffObject's dataspace */
DECTMETHOD( HTMLWin_ExportTearoff )
{
	GETDATA;

	if( gp_tearoff && data->winobj )
		DoMethod( (APTR)getv( data->winobj, MUIA_Window_RootObject ), MUIM_Export, tearoff_dataspace );

	return( 0 );
}
#endif

DECSMETHOD( JS_ListProperties )
{
	GETDATA;

	js_lp_addcplist( msg, &data->customprops );
	js_lp_addptable( msg, ptable );

	return( 0 );
}

DECTMETHOD( JS_GetGCMagic )
{
	GETDATA;
	return( data->gcmagic );
}

DECSMETHOD( JS_SetGCMagic )
{
	GETDATA;

	if( msg->magic != data->gcmagic )
	{
		struct customprop *cp;
		struct js_timer *jt;

		data->gcmagic = msg->magic;
		if( data->jso )
			jso_gc( data->jso, msg->magic );
		if( data->js_frames )
			DoMethodA( data->js_frames, (Msg)msg );
		if( data->js_location )
			DoMethodA( data->js_location, (Msg)msg );
		for( cp = FIRSTNODE( &data->customprops ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
		{
			if( cp->obj )
				DoMethodA( cp->obj, (Msg)msg );
		}
		for( jt = FIRSTNODE( &data->js_timers ); NEXTNODE( jt ); jt = NEXTNODE( jt ) )
		{
			if( !jt->killed )
			{
				if( jt->argtype == expt_funcptr )
				{
					int c;
					for( c = 0; c < jt->numargs; c++ )
						if( jt->a.args[ c ] )
							DoMethodA( jt->a.args[ c ], (Msg)msg );
				}
			}
		}
		return( DOSUPER );
	}
	return( TRUE );
}

//
// Put this HTMLWin into dormant mode
// This happens when the surrounding window is "gone",
// and the htmlwin is now solely under control of the GC
//
DECTMETHOD( HTMLWin_ToDormant )
{
	if( _parent( obj ) )
	{
		GETDATA;

		//DoMethod( _parent( obj ), MUIM_Group_InitChange );
		DoMethod( _parent( obj ), OM_REMMEMBER, obj );
		//DoMethod( _parent( obj ), MUIM_Group_ExitChange );

		// Those objects don't exist anymore when we're dormant
		data->txt_state = NULL;
		data->str_url = NULL;
		data->bar_tool = NULL;
		data->grp_keybm = NULL;
		data->iconobj = NULL;
		data->gauge_receive = NULL;

		if( data->doc_main )
			nets_setiobj( data->doc_main, NULL, NULL );
		if( data->doc_loading )
			nets_setiobj( data->doc_loading, NULL, NULL );

		killpushedmethods( data->gauge_receive );
		killpushedmethods( data->txt_state );

	}
	return( 0 );
}


/*
 * Forwarder
 */
DECSMETHOD( Historylist_GetXY )
{
	GETDATA;

	return( DoMethod( data->list_url, MM_Historylist_GetXY, msg->url, msg->x, msg->y ) );
}


/*
 * Forwarder
 */
DECSMETHOD( Historylist_StoreXY )
{
	GETDATA;

	return( DoMethod( data->list_url, MM_Historylist_StoreXY, msg->url, msg->x, msg->y ) );
}

/*
 * Update selected pages
 */
DECSMETHOD( HTMLWin_SelectPage )
{
	GETDATA;

	//kprintf ("VOYAGER: Entering HTMLWin_SelectPage\n");
	if( lastactivewin )
	{
		if( data->winobj )
		{
			APTR muiwin;

			//if( muiRenderInfo( lastactivewin ) && _win( lastactivewin ) )
			get( lastactivewin, MA_HTMLWin_MUIWindow, &muiwin );
			if( muiwin )
			{
				//muiwin = _win( lastactivewin );

				lastactivewin = obj;
				{
					struct Window *w;
					struct Region *r, *oldr;

					get( muiwin, MUIA_Window_Window, &w );

					r = NewRegion();
					oldr = InstallClipRegion( w->WLayer, r );
					//kprintf ("VOYAGER: Calling OlliHack\n");

					if( !DoMethod( muiwin, MUIM_Window_OlliHack, data->winobj ) )
						set( data->winobj, MUIA_Window_Open, TRUE );

					InstallClipRegion( w->WLayer, oldr );
					DisposeRegion( r );
					MUI_Redraw( (APTR)getv( data->winobj, MUIA_Window_RootObject ), MADF_DRAWOBJECT );

				}

				DoMethod( obj, MM_HTMLWin_SetActive );

//  				// TOFIX! This has no job of being here, really
//  				set( app, MUIA_Application_Iconified, FALSE );
			}
			else
			{
//kprintf( "WE SHOULD NOT BE HERE, REALLY (law=%lx)\n", lastactivewin );
				set( data->winobj, MUIA_Window_Open, TRUE );
			}
		}
	}

	//kprintf ("VOYAGER: Leaving HTMLWin_SelectPage\n");
	return( 0 );
}


/*
 * Forwarder (sort of)
 */
DECSMETHOD( HTMLWin_SetSmoothScroll )
{
	GETDATA;

	set( data->v, MUIA_Virtgroup_Smooth, msg->smooth );

	return( 0 );
}


/*
 * "Close" window
 * Depending on whether we're in single window mode, this is a different operation
 */

DECTMETHOD( HTMLWin_Close )
{
	struct dependentwin *mn, *mn2;
	GETDATA;

	// close all dependent windows
	for ( 	mn = FIRSTNODE( &data->f_dependents ); mn2 = NEXTNODE( mn ); mn = mn2 )
	{
		SetAttrs( mn->obj, MA_HTMLWin_Dependent, FALSE );
		pushmethod( mn->obj, 1, MM_HTMLWin_Close );

		REMOVE(mn);

		free(mn);
	}

	// if it's dependent, tell the parent window we're closing so that it doesn't
	// try and close us (again) later
	if ( data->is_dependent )
		DoMethod( data->f_opener, MM_HTMLWin_RemDependent, data->obj );

	DoMethod( app, MM_App_SetWinClose, obj );
	DoMethod( app, MUIM_Application_PushMethod, app, 1, MM_App_CheckWinRemove );

	return( 0 );
}

DECTMETHOD( HTMLWin_UpdateScrollersClock )
{
	GETDATA;

	if( data->winobj && (!data->f_fullscreen))
	{
		SetAttrs( data->winobj,
			MUIA_Window_UseRightBorderScroller, getflag( VFLG_SCROLLBARS ),
			MUIA_Window_UseBottomBorderScroller, getflag( VFLG_SCROLLBARS ),
			TAG_DONE
		);
	}

	if( data->clock )
	{
		APTR grp = _parent( data->clock );

		DoMethod( grp, MUIM_Group_InitChange );
		DoMethod( grp, OM_REMMEMBER, data->clock );
		MUI_DisposeObject( data->clock );

		if( getflag( VFLG_USE_CLOCK ) )
		{
			data->clock = NewObject( getclockclass(), NULL, TAG_DONE );
		}
		else
		{
			data->clock = MUI_NewObject( MUIC_Area, 0 );
		}

		DoMethod( grp, OM_ADDMEMBER, data->clock );
		DoMethod( grp, MUIM_Group_ExitChange );
	}

	return( 0 );
}


DECSMETHOD( HTMLWin_ExecuteJSFunc )
{
	GETDATA;
	APTR returnobj;
	APTR fobj;
	LONG funcix;

	get( msg->funcobj, MA_JS_Func_Index, &funcix );
	get( msg->funcobj, MA_JS_Func_Object, &fobj );

	D( db_js, bug( "funcix %ld, obj %lx (%s)\n", funcix, fobj, MYURL ) );

	funcix = js_createfunccall( data->jso, funcix, 0, NULL );

	returnobj = js_interpret_obj( obj, data->v, fobj, MYURL,
		data->jso, funcix,
		MYURL
	);

	D( db_js, bug( "call finished, returnobj = %lx\n", returnobj ) );

	return( (ULONG)returnobj );
}

DECTMETHOD( HTMLWin_ToggleFullScreen )
{
	GETDATA;

	extern struct Screen *destscreen;
	struct Screen *curscreen = destscreen;

	switch(data->f_fullscreen)
	{
		case FALSE:
		{
			if (curscreen)
			{
					// preserve window position
					get( data->winobj, MUIA_Window_Width, &data->pre_fs_width );
					get( data->winobj, MUIA_Window_Height, &data->pre_fs_height );
					get( data->winobj, MUIA_Window_LeftEdge, &data->pre_fs_left );
					get( data->winobj, MUIA_Window_TopEdge, &data->pre_fs_top );

					// close us up
					set( data->winobj, MUIA_Window_Open, FALSE );

					SetAttrs( data->winobj,
							MUIA_Window_UseRightBorderScroller, FALSE,
							MUIA_Window_UseBottomBorderScroller, FALSE,
							MUIA_Window_Borderless, TRUE,
							MUIA_Window_DepthGadget, FALSE,
							MUIA_Window_CloseGadget, FALSE,
							MUIA_Window_DragBar, FALSE,
							MUIA_Window_SizeGadget, FALSE,
							MUIA_Window_Title, NULL,
							TAG_DONE );

					set( data->winobj, MUIA_Window_Open, TRUE );

					ChangeWindowBox( _window(obj), 0, curscreen->BarHeight + 1, curscreen->Width, curscreen->Height - curscreen->BarHeight - 1 );

					SetAttrs( findmenu(MENU_SETWINSIZE), MUIA_Menuitem_Enabled, FALSE);

					data->f_fullscreen = TRUE;
			}
		}
		break;

		case TRUE:
		{
			// close us up
			set( data->winobj, MUIA_Window_Open, FALSE );

			SetAttrs( data->winobj,
					MUIA_Window_UseRightBorderScroller, getflag( VFLG_SCROLLBARS ),
					MUIA_Window_UseBottomBorderScroller, getflag( VFLG_SCROLLBARS ),
					MUIA_Window_Borderless, FALSE,
					MUIA_Window_DepthGadget, TRUE,
					MUIA_Window_CloseGadget, TRUE,
					MUIA_Window_DragBar, TRUE,
					MUIA_Window_SizeGadget, TRUE,
					TAG_DONE );

			set( data->winobj, MUIA_Window_Open, TRUE );

			// JS windows don't have any previous numbers to reset..
			if ( !data->is_js_win ) ChangeWindowBox( _window(obj), data->pre_fs_left, data->pre_fs_top, data->pre_fs_width, data->pre_fs_height);

			SetAttrs( findmenu(MENU_SETWINSIZE), MUIA_Menuitem_Enabled, TRUE);

			data->f_fullscreen = FALSE;

			// bring back the title
			DoMethod( obj, MM_HTMLWin_MakeWindowTitle );
		}
		break;
	}

	return( 0 );
}

DECSMETHOD( HTMLWin_AddDependent )
{
	struct dependentwin *dw;

	GETDATA;

	dw = malloc( sizeof(struct dependentwin) );
	if (dw)
	{
		dw->obj = msg->obj;
		ADDTAIL( &data->f_dependents, dw );
	}
	// window isn't in dependent list if it failed. No big fuss..
	return( 0 );
}

DECSMETHOD( HTMLWin_RemDependent )
{
	struct dependentwin *mn, *mn2;

	GETDATA;

	for (mn = FIRSTNODE( &data->f_dependents ); mn2 = NEXTNODE(mn); mn = mn2)
	{
		if ( mn->obj == msg->obj )
		{
			REMOVE(mn);
			free(mn);
		}
	}
	return( 0 );
}


DECSMETHOD( HTMLWin_SetPointer )
{
	GETDATA;

	// If we are not a top window, forward upwards
	if( !data->winobj )
	{
		APTR lasto = _parent( obj ), w = NULL;

		while( lasto )
		{
			if( OCLASS( lasto ) == OCLASS( obj ) )
				w = lasto;
			lasto = _parent( lasto );
		}
		if( w )
			DoMethodA( w, (Msg)msg );
		return( 0 );
	}
	else
	{
		if ( muiRenderInfo( obj ) && _win( obj ) && _window( obj ) )
			SetWindowPointer( _window( obj ), WA_PointerType, msg->type, TAG_DONE );
	}

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( HTMLWin_ToStandalone )
DEFSMETHOD( HTMLWin_ToDormant )
DEFSMETHOD( HTMLWin_SetupToolbar )
DEFSMETHOD( HTMLWin_SetupIcon )
#if USE_NET
DEFSMETHOD( HTMLWin_SetupFastlinks )
#endif /* USE_NET */
DEFSMETHOD( HTMLWin_SelectFastlink )
DEFSMETHOD( HTMLWin_URLAcknowledge )
DEFSMETHOD( HTMLWin_MakeWindowTitle )
DEFSMETHOD( HTMLWin_SetURL )
DEFSMETHOD( HTMLWin_SetURLCleanup )
DEFSMETHOD( HTMLWin_SetTxt )
DEFSMETHOD( HTMLWin_OpenDocInfoWin )
DEFSMETHOD( HTMLWin_ShowSource )
DEFSMETHOD( HTMLWin_SetDone )
DEFSMETHOD( HTMLWin_StopXfer )
DEFSMETHOD( HTMLWin_Forward )
DEFSMETHOD( HTMLWin_Backward )
DEFSMETHOD( HTMLWin_EnableButtonByFunc )
DEFSMETHOD( HTMLWin_DoToolbutton )
DEFSMETHOD( HTMLWin_BuildButtonHistory )
DEFSMETHOD( HTMLWin_GotoButtonHistory )
DEFSMETHOD( HTMLWin_SetTitle )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( HTMLWin_FindByName )
DEFMETHOD( HTMLWin_LoadInlineGfx )
DEFMETHOD( HTMLWin_Reload )
DEFMETHOD( HTMLWin_SetActive )
DEFSMETHOD( NStream_GotInfo )
DEFSMETHOD( NStream_GotData )
DEFSMETHOD( NStream_Done )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_ListProperties )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_FindByName )
DEFMMETHOD( Draw )
DEFMMETHOD( AskMinMax )
DEFSMETHOD( HTMLWin_SetTempStatus )
DEFSMETHOD( HTMLWin_ResetStatus )
DEFSMETHOD( HTMLWin_CheckDone )
DEFTMETHOD( HTMLWin_AddBM )
DEFSMETHOD( Layout_CalcUnloadedObjects )
DEFSMETHOD( HTMLWin_ExecuteEvent )
DEFSMETHOD( HTMLWin_DoActiveFrame )
DEFSMETHOD( Historylist_GetXY )
DEFSMETHOD( Historylist_StoreXY )
DEFTMETHOD( HTMLWin_StartTimers )
DEFTMETHOD( HTMLWin_TriggerTimers )
DEFTMETHOD( HTMLWin_CleanupTimers )
DEFTMETHOD( HTMLWin_OpenFile )
DEFTMETHOD( JS_SetGCMagic )
DEFTMETHOD( JS_GetGCMagic )
DEFSMETHOD( HTMLWin_SetSmoothScroll )
DEFSMETHOD( HTMLWin_SelectPage )
DEFTMETHOD( HTMLWin_ToggleFullScreen )
DEFSMETHOD( HTMLWin_AddDependent )
DEFSMETHOD( HTMLWin_RemDependent )
DEFSMETHOD( ImgDecode_HasInfo )
DEFTMETHOD( ImgDecode_Done )
DEFSMETHOD( ImgDecode_GotScanline )
#if USE_TEAROFF
DEFTMETHOD( HTMLWin_ExportTearoff )
#endif
DEFTMETHOD( HTMLWin_Close )
DEFTMETHOD( HTMLWin_UpdateScrollersClock )
DEFSMETHOD( HTMLWin_ExecuteJSFunc )
DEFSMETHOD( HTMLWin_SetPointer )
ENDMTABLE

int create_htmlwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "HtmlWinClass";
#endif

	return( TRUE );
}

void delete_htmlwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR gethtmlwinclass( void )
{
	return( mcc->mcc_Class );
}


// Access functions

APTR win_create( STRPTR name, STRPTR url, STRPTR referer, APTR owner, int embedded, int reload, int fullscreen )
{
	APTR o;

	o = JSNewObject( gethtmlwinclass(),
		MA_HTMLWin_Name, name,
		MA_HTMLWin_FullScreen, fullscreen,
		MUIA_Scrollgroup_UseWinBorder, !embedded,
		TAG_DONE
	);

	if( o )
	{
		if( !embedded )
		{
			DoMethod( o, MM_HTMLWin_ToStandalone );
		}

		DoMethod( o, MM_HTMLWin_SetURL, url, referer, NULL, ( reload ? MF_HTMLWin_Reload : 0 ) | MF_HTMLWin_AddURL );

		if( !veryfirstwin )
			veryfirstwin = o;
	}
	else
	{
		displaybeep();
	}
	return( o );
}
