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
** $Id: lo_frameset.c,v 1.21 2004/12/30 23:26:22 henes Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#endif

/* private */
#include "classes.h"
#include "htmlclasses.h"
#include "layout.h"
#include "mui_func.h"
#include "malloc.h"
#include "js.h"

static struct MUI_CustomClass *lcc;

//
// Standard MCC Stuff
//

struct finfo {
	APTR o;
	struct layout_info *li;
};

struct Data {
	char *widthspec;
	char *heightspec;
	int frameborder, border;
	struct finfo *fmatrix;
	int *minwidths, *minheights;
	int *specwidths, *specheights;
	int *typewidths, *typeheights;
	int rcnt, ccnt;
	APTR pool;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Frame_Horizspec:
			l_readstrtag( tag, &data->widthspec );
			break;

		case MA_Layout_Frame_Vertspec:
			l_readstrtag( tag, &data->heightspec );
			break;

		case MA_Layout_Frame_Border:
			data->border = tag->ti_Data;
			break;

		case MA_Layout_Frame_Frameborder:
			data->frameborder = tag->ti_Data;
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MA_Layout_Align, align_newrow,
		TAG_MORE, msg->ops_AttrList
	);

	data = INST_DATA( cl, obj );

	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		MUI_Redraw( obj, MADF_DRAWOBJECT );

	return( DOSUPER );
}

DECDEST
{
	GETDATA;

	free( data->widthspec );
	free( data->heightspec );

	if( data->pool )
		DeletePool( data->pool );

	return( DOSUPER );
}

static int countspec( char *spec )
{
	int n = 1;
	while( *spec )
	{
		if( *spec++ == ',' )
			n++;
	}
	return( n );
}

//
// Process a frame size specification
//
static void dolayout(
	int *mins,
	int *specs,
	int *types,
	int cnt,
	int space,
	int border
)
{
	int c;
	int numperc = 0;
	int numall = 0;
	int numfixed = 0;
	int totalmin = 0, totalpercsize = 0;
	int togo;
	int availspace;

	availspace = space - border * ( cnt - 1 );

	D( db_html, bug( "space %ld availspace %ld border %ld cnt %ld\n", space, availspace, border, cnt ) );

	for( c = 0; c < cnt; c++ )
	{
		D( db_html, bug( "item[%ld] spec %ld type %ld min %ld\n", c, specs[ c ], types[ c ], mins[ c ] ) );
		if( types[ c ] == 1 )
		{
			int want;

			numperc++;

			// Calculate wanted size of this frame
			want = ( availspace * specs[ c ] ) / 100;
			specs[ c ] = max( want, mins[ c ] );
			D( db_html, bug( "-> perc; want %ld\n", specs[ c ] ) );
		}
		else if( types[ c ] == 2 )
		{
			numall++;
			specs[ c ] = mins[ c ];
		}
		else
		{
			numfixed++;
			specs[ c ] = mins[ c ];
		}

		totalmin += mins[ c ];
		if( c )
			totalmin += border;
	}

	togo = space - totalmin;

	D( db_html, bug( "totalpercsize %ld togo %ld\n", totalpercsize, togo ));

	// Fix stupid webmasters
	while( numfixed && togo < 0 )
	{
		for( c = 0; c < cnt; c++ )
		{
			if( types[ c ] == 0 )
			{
				if( specs[ c ] > 1 )
				{
					specs[ c ]--;
					togo++;
				}
				else
				{
					types[ c ] = -1;
					numfixed--;
				}
			}
		}
	}

	// Blow up perc frames
	while( numperc && togo > 0 )
	{
		for( c = 0; c < cnt; c++ )
		{
			if( types[ c ] == 1 )
			{
				if( mins[ c ] < specs[ c ] )
				{
					mins[ c ]++;
					togo--;
				}
				else
				{
					numperc--;
					types[ c ] = -1;
				}
			}
		}
	}
	if( numall && togo > 0 )
	{
		for( c = 0; c < cnt; c++ )
		{
			if( types[ c ] == 2 )
			{
				mins[ c ] += togo / numall;
			}
		}
	}

#ifdef DEBUG
	for( c = 0; c < cnt; c++ )
	{
		D( db_html, bug( "res[%ld] = %ld\n", c, mins[c] ) );
	}
#endif

}


DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	struct layout_info *li;
	int xp = 0, yp = 0, x, y;
	int border = data->border;

	get( obj, MA_Layout_Info, &li );

	// Here we go to the painful process of laying out the individual frames ;)
	dolayout( data->minwidths, data->specwidths, data->typewidths, data->ccnt, msg->suggested_width, data->border );
	dolayout( data->minheights, data->specheights, data->typeheights, data->rcnt, msg->suggested_height, data->border );

	for( y = 0; y < data->rcnt; y++ )
	{
		xp = 0;
		for( x = 0; x < data->ccnt; x++ )
		{
			struct finfo *fi = &data->fmatrix[ x + y * data->ccnt ];
			if( fi->o )
			{
				DoMethod( fi->o, MM_Layout_DoLayout, data->minwidths[ x ], data->minheights[ y ] );
				fi->li->xp = xp;
				fi->li->yp = yp;
			}
			xp += data->minwidths[ x ] + border;
		}
		yp += data->minheights[ y ] + border;
	}

	li->xs = msg->suggested_width;
	li->ys = msg->suggested_height;

	if( data->pool )
	{
		DeletePool( data->pool );
		data->pool = NULL;
	}

	return( (ULONG)li );
}

static void parsespec(
	char *spec,
	int cnt,
	int *mins,
	int *specs,
	int *types
)
{
	while( cnt )
	{
		int n = atoi( spec );
		int type = 0;

		spec = stpblk( spec );
		while( isdigit( *spec ) )
			spec++;
		spec = stpblk( spec );
		if( *spec == '%' )
			type = 1;
		else if( *spec == '*' )
			type = 2;

		if( !n )
			n = 1;

		if( !type )
		{
			*mins = n;
			*specs = n;
		}
		else if( type == 1 )
		{
			*specs = n;
			*types = 1;
		}
		else
		{
			*types = 2;
		}

		// Skip to next entry
		while( *spec && *spec != ',' )
			spec++;
		if( *spec )
			spec++;

		types++;
		mins++;
		specs++;

		cnt--;
	}
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	struct layout_info *li = (APTR)getv( obj, MA_Layout_Info );
	struct MinList *l;
	APTR o, ostate;
	int cnt, c;
	int thiscols, thisrows;

	data->rcnt = thisrows = countspec( data->heightspec );
	data->ccnt = thiscols = countspec( data->widthspec );

	if( data->pool )
		DeletePool( data->pool );
	data->pool = CreatePool( MEMF_CLEAR, 2048, 1024 );
	if( !data->pool )
		return( (ULONG)li );

	data->fmatrix = AllocPooled( data->pool, thisrows * thiscols * sizeof( struct finfo ) );
	data->minwidths = AllocPooled( data->pool, thiscols * sizeof( int ) );
	data->minheights = AllocPooled( data->pool, thisrows * sizeof( int ) );
	data->specwidths = AllocPooled( data->pool, thiscols * sizeof( int ) );
	data->specheights = AllocPooled( data->pool, thisrows * sizeof( int ) );
	data->typewidths = AllocPooled( data->pool, thiscols * sizeof( int ) );
	data->typeheights = AllocPooled( data->pool, thisrows * sizeof( int ) );

	if( data->fmatrix == NULL
	 || data->minwidths == NULL
	 || data->minheights == NULL
	 || data->specwidths == NULL
	 || data->specheights == NULL
	 || data->typewidths == NULL
	 || data->typeheights == NULL )
	{
		DeletePool( data->pool );
		data->pool = NULL;
		data->fmatrix = NULL;
		data->minwidths = NULL;
		data->minheights = NULL;
		data->specwidths = NULL;
		data->specheights = NULL;
		data->typewidths = NULL;
		data->typeheights = NULL;
		return( (ULONG)li);
	}

	parsespec( data->widthspec, data->ccnt, data->minwidths, data->specwidths, data->typewidths );
	parsespec( data->heightspec, data->rcnt, data->minheights, data->specheights, data->typeheights );

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->mlh_Head;
	cnt = 0;
	NextObject( &ostate ); // Dummy
	while( o = NextObject( &ostate ) )
	{
		struct layout_info *tli;

		tli = (APTR)DoMethod( o, MM_Layout_CalcMinMax, msg->suggested_width, msg->suggested_height );
		data->fmatrix[ cnt ].o = o;
		data->fmatrix[ cnt ].li = tli;

		data->minwidths[ cnt % thiscols ] = max( data->minwidths[ cnt % thiscols ], tli->minwidth );
		data->minheights[ cnt / thiscols ] = max( data->minheights[ cnt / thiscols ], tli->minheight );

		cnt++;

	}

	li->minwidth = 0;
	for( c = 0; c < thiscols; c++ )
	{
		li->minwidth += data->minwidths[ c ];
		if( c )
			li->minwidth += data->border;
	}
	li->minheight = 0;
	for( c = 0; c < thisrows; c++ )
	{
		li->minheight += data->minheights[ c ];
		if( c )
			li->minheight += data->border;
	}

	li->defwidth = msg->suggested_width;
	li->maxwidth = msg->suggested_width;
	li->defheight = msg->suggested_height;
	li->maxheight = msg->suggested_height;

	return( (ULONG)li );
}

DECSMETHOD( Layout_Frameset_BuildFramesArray )
{
	struct MinList *l;
	APTR o, ostate;
	struct MinList *cpl;

	get( msg->farray, MA_JS_Object_CPL, &cpl );

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->mlh_Head;
	NextObject( &ostate ); // Dummy
	while( o = NextObject( &ostate ) )
	{
		char *name;
		char tmpname[ 32 ];

		if( OCLASS( o ) != gethtmlwinclass() )
		{
			if( OCLASS( o ) == cl )
				DoMethodA( o, (Msg)msg );
			continue;
		}

		get( o, MA_JS_Name, &name );
		if( !name || !*name )
		{
			sprintf( tmpname, "unnamed%lx", (ULONG)o );
			name = tmpname;
		}
		cp_set( cpl, name, o );
	}
	return( 0 );
}


DECSMETHOD( JS_FindByName )
{
	APTR ostate, o, or;
	struct List *l;

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->lh_Head;
	while( o = NextObject( &ostate ) )
	{
		if( ( or = (APTR)DoMethodA( o, ( Msg )msg ) ) )
		{
			return( (ULONG)or );
		}
	}
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFSET
DEFDISPOSE
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_Frameset_BuildFramesArray )
DEFSMETHOD( JS_FindByName )
ENDMTABLE

int create_loframesetclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getlogroupmcc(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loframesetClass";
#endif

	return( TRUE );
}

void delete_loframesetclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloframesetclass( void )
{
	return( lcc->mcc_Class );
}
