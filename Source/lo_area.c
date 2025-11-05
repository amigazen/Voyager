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
** $Id: lo_area.c,v 1.40 2004/02/17 11:02:26 zapek Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <graphics/gfxmacros.h>
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
#include "textfit.h"
#include "download.h"
#include "methodstack.h"
#include "malloc.h"
#include <limits.h>
#include "mui_func.h"


static struct MUI_CustomClass *lcc;

struct imappoint {
	int x;
	int y;
};

struct imapline {
	struct imappoint p1, p2;
};

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
	struct imappoint *points;
	int numpoints;
	int type;
	int mousein;

	int accesskeycode;
	int accesskeycode2;

	struct MUI_EventHandlerNode ehnode;

	int ix_onclick, ix_mouseover, ix_mouseout;
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
			if( data->ix_onclick )
			{
				if( DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, jse_click, data->ix_onclick, obj, TAG_DONE ) )
					break;
			}
			if( !tag->ti_Data )
			{
				// Link was selected
				if( data->qualifier & ( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT ) )
				{
#if USE_STB_NAV
					pushmethod( data->ctx->dom_win, 5, MM_HTMLWin_SetURL, data->tempurl ? data->tempurl : data->url, data->ctx->baseref, "_blank", MF_HTMLWin_AddURL );
#else
#if USE_NET
					queue_download( data->tempurl ? data->tempurl : data->url, data->ctx->baseref, TRUE, FALSE );
#endif /* USE_NET */
#endif
				}
				else
				{
					pushmethod( data->ctx->dom_win, 5, MM_HTMLWin_SetURL, data->tempurl ? data->tempurl : data->url, data->ctx->baseref, data->qualifier & ( IEQUALIFIER_RALT | IEQUALIFIER_LALT ) ? "_blank" : data->target, MF_HTMLWin_AddURL );
				}
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

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
		MUIA_CycleChain, 1,
		MA_JS_ClassName, "Area",
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
	free( data->points );

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
	//GETDATA;
	// TOFIX!
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

DECSMETHOD( Layout_Area_SetCoords )
{
	GETDATA;
	int pnum = 1;
	char *p;
	struct imappoint *ptr;
	int cnt = 0;

	// count number of coords
	p = msg->coords;
	while( *p )
	{
		if( isspace( *p ) )
		{
			p = stpblk( p );
			pnum++;
		}
		else if( *p++ == ',' )
			pnum++;
	}

	pnum = max( ( ( pnum + 1 ) / 2 ), 2 );

	data->type = msg->type;

	data->points = malloc( sizeof( struct imappoint ) * pnum );
	if( !data->points )
		return( 0 );

	memset( data->points, '\0', sizeof( struct imappoint ) * pnum ); /* TOFIX: maybe not needed */

	data->numpoints = pnum;

	ptr = data->points;
	Forbid();
	p = strtok( msg->coords, ", " );
	while( p )
	{
		if( cnt )
		{
			ptr->y = atoi( p );
			cnt = 0;
			ptr++;
		}
		else
		{
			ptr->x = atoi( p );
			cnt = 1;
		}

		p = strtok( NULL, ", " );
	}
	Permit();

	// close polygon?
	if( data->type == IMAPAREA_POLY )
	{
		int smallx = INT_MAX, smally = INT_MAX;
		int fullrun;

		if( !memcmp( ptr - 1, data->points, sizeof( struct imappoint ) ) )
		{
			data->numpoints--;
		}

		fullrun = data->numpoints;

		// now shift for smallest x/y pair
		for(;;)
		{
			struct imappoint temp;

			if( fullrun <= 0 && data->points[ 0 ].x == smallx && data->points[ 0 ].y == smally )
				break;
			if( data->points[ 0 ].y < smally || ( data->points[ 0 ].y == smally && data->points[ 0 ].x < smallx ) )
			{
				smallx = data->points[ 0 ].x;
				smally = data->points[ 0 ].y;
			}
			temp = data->points[ 0 ];
			memmove( &data->points[ 0 ], &data->points[ 1 ], 8 * ( data->numpoints - 1 ) );
			data->points[ data->numpoints - 1 ] = temp;
			fullrun--;
		}
	}

	return( TRUE );
}

static int ccw( struct imappoint p0, struct imappoint p1, struct imappoint p2 )
{
	int dx1, dx2, dy1, dy2;

	dx1 = p1.x - p0.x;
	dy1 = p1.y - p0.y;
	dx2 = p2.x - p0.x;
	dy2 = p2.y - p0.y;

	if( dx1 * dy2 > dy1 * dx2 )
		return( 1 );

	if( dx1 * dy2 < dy1 * dx2 )
		return( -1 );

	if( ( dx1 * dx2 < 0 ) || ( dy1 * dy2 < 0 ) )
		return( -1 );

	if( ( dx1 * dx1 + dy1 * dy1 ) < ( dx2 * dx2 + dy2 * dy2 ) )
		return( +1 );

	return( 0 );
}

static int intersect( struct imapline l1, struct imapline l2 )
{
	return( ( ccw( l1.p1, l1.p2, l2.p1 )
			* ccw( l1.p1, l1.p2, l2.p2 ) ) <= 0 )
	   && ( ( ccw( l2.p1, l2.p2, l1.p1 )
			* ccw( l2.p1, l2.p2, l1.p2 ) ) <= 0 );
}

static int polyinside( struct imappoint *t, struct imappoint *poly, int N )
{
	struct imappoint p[ 128 ];
	int i, count = 0, j = 0;
	struct imapline lt, lp;

	N = min( N, 126 );

	memcpy( &p[ 1 ], poly, N * sizeof( struct imappoint ) );

	p[ 0 ] = p[ N ];
	p[ N + 1 ] = p[ 1 ];

	lt.p1 = *t;
	lt.p2 = *t;
	lt.p2.x = 2048;

	for( i = 1; i <= N; i++ )
	{   
		lp.p1 = p[ i ];
		lp.p2 = p[ i ];
		// falls der punkt auf unserer testgraden liegt, ignorieren
		if( !intersect( lp, lt ) )
		{
			lp.p2 = p[ j ];
			j = i;

			if( intersect( lp, lt ) )
			{
				count++;
			}
		}
	}

	return( count & 1 );
}

DECSMETHOD( Layout_Map_FindArea )
{
	GETDATA;
	int x = msg->x;
	int y = msg->y;

	switch( data->type )
	{
		case IMAPAREA_RECT: // simple one
			if( data->points[ 0 ].x <= x &&
				data->points[ 0 ].y <= y &&
				data->points[ 1 ].x >= x &&
				data->points[ 1 ].y >= y 
				)
				return( (ULONG)obj );
			break;

		case IMAPAREA_POLY:
			{
				struct imappoint tp;

				tp.x = x; 
				tp.y = y;

				if( polyinside( &tp, data->points, data->numpoints ) )
					return( (ULONG)obj );
			}
			break;

		case IMAPAREA_CIRCLE:
			{
				struct imappoint tp, ca[ 4 ];
				int r = data->points[ 1 ].x;
				int cx = data->points[ 0 ].x;
				int cy = data->points[ 0 ].y;

				tp.x = x; 
				tp.y = y;

				ca[ 0 ].x = cx;
				ca[ 0 ].y = cy - r;

				ca[ 1 ].x = cx + r;
				ca[ 1 ].y = cy;

				ca[ 2 ].x = cx;
				ca[ 2 ].y = cy + r;

				ca[ 3 ].x = cx - r;
				ca[ 3 ].y = cy;

				if( polyinside( &tp, ca, 4 ) )
					return( (ULONG)obj );
			}
			break;
	}

	return( (ULONG)NULL );
}

DECSMETHOD( Layout_Area_DrawFrame )
{
	GETDATA;
	int c;
	static UWORD areaptr[ ] = { 0xcccc, 0xcccc };
	struct RastPort *rp = _rp( obj );

	SetAPen( rp, 255 );
	SetDrMd( rp, COMPLEMENT );
	SetAfPt( rp, areaptr, 1 );

	switch( data->type )
	{
		case IMAPAREA_RECT:
			Move( rp, data->points[ 0 ].x + msg->x, data->points[ 0 ].y + msg->y );
			Draw( rp, data->points[ 1 ].x + msg->x - 1, data->points[ 0 ].y + msg->y );
			Draw( rp, data->points[ 1 ].x + msg->x - 1, data->points[ 1 ].y + msg->y - 1 );
			Draw( rp, data->points[ 0 ].x + msg->x, data->points[ 1 ].y + msg->y - 1 );
			Draw( rp, data->points[ 0 ].x + msg->x, data->points[ 0 ].y + msg->y );
			break;

		case IMAPAREA_CIRCLE:
#ifdef MBX
			DrawCircle( rp, msg->x + data->points[ 0 ].x, msg->y + data->points[ 0 ].y, data->points[ 1 ].x, 0 );
#else
			DrawCircle( rp, msg->x + data->points[ 0 ].x, msg->y + data->points[ 0 ].y, data->points[ 1 ].x );
#endif /* !MBX */
			break;

		case IMAPAREA_POLY:
			Move( rp, data->points[ 0 ].x + msg->x, data->points[ 0 ].y + msg->y );
			for( c = 1; c < data->numpoints; c++ )
				Draw( rp, data->points[ c ].x + msg->x, data->points[ c ].y + msg->y );
			break;
	}
	SetDrMd( rp, JAM1 );
	SetAfPt( rp, NULL, 0 );

	return( 0 );
}

BEGINPTABLE
DPROP( onclick,  	funcptr )
DPROP( onmouseover,	funcptr )
DPROP( onmouseout,	funcptr )
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
	/*switch( findpropid( ptable, msg->propname ) )
	{
	}*/
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
	}

	return( DOSUPER );
}

DECMMETHOD( Setup )
{
	GETDATA;
	ULONG rc;

	rc = DOSUPER;

	data->ehnode.ehn_Object = obj;
	data->ehnode.ehn_Class = cl;
	data->ehnode.ehn_Events = IDCMP_INTUITICKS;
	data->ehnode.ehn_Priority = 1;
	data->ehnode.ehn_Flags = MUI_EHF_GUIMODE;
	DoMethod( _win( obj ), MUIM_Window_AddEventHandler, ( ULONG )&data->ehnode );

	return( rc );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	DoMethod( _win( obj ), MUIM_Window_RemEventHandler, ( ULONG )&data->ehnode );

	return( DOSUPER );
}

#define _isinobject(x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_top(obj)+data->li.ys-1))
#define _between(a,x,b) ((x)>=(a) && (x)<=(b))

DECMMETHOD( HandleEvent )
{
	GETDATA;

	if( msg->imsg )
	{
		if( msg->imsg->Class == IDCMP_INTUITICKS )
		{
			if( _isinobject( msg->imsg->MouseX, msg->imsg->MouseY ) )
			{
				if( !data->mousein )
				{
					data->mousein = TRUE;
					if( data->ix_mouseover )
					{
						if( DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, jse_mouseover, data->ix_mouseover, obj, TAG_DONE ) )
							return( MUI_EventHandlerRC_Eat );
					}
				}
			}
			else
			{
				if( data->mousein )
				{
					data->mousein = FALSE;
					if( data->ix_mouseout )
					{
						if( DoMethod( data->ctx->dom_win, MM_HTMLWin_ExecuteEvent, jse_mouseout, data->ix_mouseout, obj, TAG_DONE ) )
							return( MUI_EventHandlerRC_Eat );
					}
				}
			}
		}
	}

	return( 0 );
}

DS_LISTPROP

BEGINMTABLE
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( HandleEvent )
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFMMETHOD( GoActive )
DEFMMETHOD( AskMinMax )
DEFSMETHOD( Layout_Map_FindArea )
DEFSMETHOD( Layout_Area_SetCoords )
DEFSMETHOD( Layout_Area_DrawFrame )
DEFSMETHOD( Layout_Anchor_HandleAccessKey )
#ifdef SHOWHIDE
DEFMMETHOD( Show )
DEFMMETHOD( Hide )
#endif
DEFMMETHOD( Draw )
DEFSMETHOD( JS_SetProperty )
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

int create_loareaclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loareaClass";
#endif

	return( TRUE );
}

void delete_loareaclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloareaclass( void )
{
	return( lcc->mcc_Class );
}
