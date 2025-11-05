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
** $Id: lo_table.c,v 1.94 2004/12/04 12:02:25 zapek Exp $
**
*/

#include "voyager.h"
#include <math.h>

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#endif
#include <limits.h>

/* private */
#include "classes.h"
#include "htmlclasses.h"
#include "layout.h"
#include "mui_func.h"

static struct MUI_CustomClass *lcc;

//
// Standard MCC Stuff
//

struct cellinfo {
	APTR obj;
	int rowspan;
	int colspan;
	int startcol,startrow;
	char *widthspec;
	char *heightspec;

	int pctwidth,abswidth;
	int pctheight,absheight;

	struct layout_info *li;

	UBYTE processed;
	UBYTE processed_row;
	UBYTE processed_assigned;
};

struct Data {
	struct cellinfo ***cellarray;

	int *coldef,*colmin;
	double *colpct;
	int coltotdef,coltotunpctdef;
	double coltotpct;

	int maxcol, maxrow;
	int amaxcol, amaxrow;

	int maxrowspan, maxcolspan;

	APTR pool;

	int penspec_border_dark, penspec_border_light;
	int cellpadding, cellspacing;
	char *widthspec;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int redraw = FALSE;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_Table_Cellpadding:
			data->cellpadding = tag->ti_Data;
			break;

		case MA_Layout_Table_Cellspacing:
			data->cellspacing = tag->ti_Data;
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

	if( data->pool )
		DeletePool( data->pool );

	return( DOSUPER );
}

static void calcdefwidth( int maxel, double *pct, int *def, int *min, int *defout, int *minout, int contwidth );
static void spreadpercent( int totunpctdef, double totpct, int totdef, int maxel, int *def, double *pct );

DECSMETHOD( Layout_DoLayout )
{
	GETDATA;
	struct layout_info *li;
	int yp = getv( obj, MA_Layout_MarginTop ) + data->cellspacing;
	int x, y;
	int tablewidth = msg->suggested_width;
	int innertablewidth,usetabwidth;
	double usepct;
	char *widthspec;
	int rs, c;
	int xp = 0, sxp, maxxp, deftot;
	int *lineheights;
	UBYTE *rowspanflags;
	double rempct = 0.0;

	get( obj, MA_Layout_Info, &li );

	li->flags &= ~LOF_NEW;

	if( data->maxcol == 0 || data->colpct == NULL || data->colmin == NULL || data->coldef == NULL )
	{
		// Table still empty, possibly during incremental layout
		return( (ULONG)li );
	}

	rowspanflags = AllocPooled( data->pool, data->maxrowspan + 1 );
	if( rowspanflags == NULL )
		return( (ULONG)li );

	for( y = 0; y < data->maxrow; y++ )
	{
		for( x = 0; x < data->maxcol; x++ )
		{
			if( data->cellarray[ y ][ x ] )
			{
				data->cellarray[ y ][ x ]->processed = FALSE;
				rowspanflags[ data->cellarray[ y ][ x ]->rowspan ] = TRUE;
			}
		}
	}

	// Calculate final table output width
	get( obj, MA_Layout_Width, &widthspec );
	if( widthspec )
	{
		int specwidth = atoi( widthspec );
		if( strchr( widthspec, '%' ) )
		{
			specwidth = max( 1, specwidth );
			specwidth = ( specwidth * msg->outer_width ) / 100;
		}
		specwidth = max( specwidth, li->minwidth );
		specwidth = min( specwidth, tablewidth );

		tablewidth = specwidth;
	}
	else
	{
		tablewidth = min( tablewidth, li->defwidth );
	}

	innertablewidth = tablewidth - ( data->maxcol + 1 ) * data->cellspacing - getv( obj, MA_Layout_MarginLeft ) - getv( obj, MA_Layout_MarginRight );

	D( db_html, bug( "in TABLE_dolayout(%lx), container width %ld, outer width %ld, tablewidth %ld, maxcol %ld, maxrow %ld\r\n", obj, msg->suggested_width, msg->outer_width, tablewidth, data->maxcol, data->maxrow ));

/*
	// TOFIX! Redo

	get( obj, MA_Layout_Height, &heightspec );
	if( heightspec )
	{
		int specheight = atoi( heightspec );
		if( strchr( heightspec, '%' ) )
		{
			specheight = max( 0, min( 100, specheight ) );
			specheight = ( specheight * msg->suggested_height ) / 100;
			
		}
		specheight = max( specheight, li->minheight );
		specheight = min( specheight, tableheight );

		tableheight = specheight;
	}
	else
	{
		tableheight = min( tableheight, li->defheight );
	}
*/

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P3a Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], (int)(data->colpct[ x ]*10000.0) ); });

	for( x = 0; x < data->maxcol; x++ )
		if( data->coldef[ x ] < 0 )
		{
			double oldpct;
			oldpct = data->colpct[ x ];
			data->colpct[ x ] = 100.0 * ( (double)-data->coldef[ x ] / (double)innertablewidth );
			rempct += oldpct - data->colpct[ x ];
		}
	if( rempct > 0.0 )
	{
		int colsnofixedpct = 0;
		for( x = 0; x < data->maxcol; x++ )
		{
			if( data->colpct[ x ] >= 0.0 && data->coldef[ x ] >= 0 )
				colsnofixedpct++;
		}
		for( x = 0; x < data->maxcol; x++ )
		{
			if( data->colpct[ x ] >= 0.0 && data->coldef[ x ] >= 0 )
				data->colpct[ x ] += rempct / (double)colsnofixedpct;
		}
	}
	for( x = 0; x < data->maxcol; x++ )
	{
		if( data->colpct[ x ] < 0.0 )
			data->colpct[ x ] = -data->colpct[ x ];
		if( data->coldef[ x ] < 0 )
			data->coldef[ x ] = -data->coldef[ x ];
	}
	
	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P3b Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], (int)(data->colpct[ x ]*10000.0) ); });

	if( data->coltotpct > 0.0 )
		spreadpercent( data->coltotunpctdef, data->coltotpct, data->coltotdef, data->maxcol, data->coldef, data->colpct );

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P3c Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], (int)(data->colpct[ x ]*10000.0) ); });

	calcdefwidth( data->maxcol, data->colpct, data->coldef, data->colmin, &( li->defwidth ), &( li->minwidth ), innertablewidth );

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P4 Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], ((int)(data->colpct[ x ]*10000.0)) ); });

	usetabwidth = innertablewidth;
	usepct = 100.0;
retry:
	deftot = 0;
	for( x = 0; x < data->maxcol; x++ )
	{
		if( usepct>0.0 && data->colpct[ x ] >= 0.0 )
		{
			data->coldef[ x ] = (int)( ( data->colpct[ x ] * (double) usetabwidth ) / usepct );

			if( data->coldef[ x ] < data->colmin[ x ] )
			{
				usepct -= data->colpct[ x ];
				data->colpct[ x ] = -1.0;
				usetabwidth -= data->colmin[ x ];
				goto retry;
			}
		}
		else
			data->coldef[ x ] = data->colmin[ x ];
		deftot += data->coldef[ x ];
	}
	/* Rounding errors can mean we end up using more than we're supposed to */
	if( deftot > innertablewidth )
	{
		usetabwidth--;
		goto retry;
	}

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P5 Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], ((int)(data->colpct[ x ]*10000.0)) ); });

	sxp = getv( obj, MA_Layout_MarginLeft ) + data->cellspacing;
	maxxp = 0;
	for( y = 0; y < data->maxrow; y++ )
	{
		xp = sxp;
		for( x = 0; x < data->maxcol; x++ )
		{
			struct cellinfo *ci;

			ci = data->cellarray[ y ][ x ];
			if( ci && x==ci->startcol )
			{
				int width = 0;
				int cs;

				// Add up column widths for colspanning cells
				for( cs = 0; cs < ci->colspan; cs++ )
				{
					width += data->coldef[ x + cs ];
					if( cs )
					{
						width += data->cellspacing;
					}
				}
				if( !ci->processed )
				{
					DoMethod( ci->obj, MM_Layout_DoLayout, width, ci->absheight < 0 ? msg->suggested_height : ci->absheight, width );
					ci->li->xp = xp;
					ci->processed = TRUE;
				}
				xp += width + data->cellspacing;
			}
		}
		maxxp = max( xp, maxxp );
	}

	// Allocate temporary line buffer
	lineheights = AllocPooled( data->pool, data->maxrow * sizeof( int ) );
	if( !lineheights )
		return( (ULONG)li );

	// Calculate cell heights
	for( rs = 1; rs <= data->maxrowspan; rs++ )
	{
		if( !rowspanflags[ rs ] )
			continue;
		for( y = 0; y < data->maxrow; y++ )
		{
			for( x = 0; x < data->maxcol; x++ )
			{
				struct cellinfo *ci;

				ci = data->cellarray[ y ][ x ]; 

				if( ci && !ci->processed_row && ci->rowspan == rs )
				{
					int haveheight = 0;
					int needheight = 0;

					for( c = 0; c < rs; c++ )
					{
						haveheight += lineheights[ y + c ];
						if( c > 0 )
							haveheight += data->cellspacing;
					}

					needheight = ci->li->ys;

					if( ci->heightspec )
					{
						if( !strchr( ci->heightspec, '%' ) )
						{
							int hs = atoi( ci->heightspec );
							needheight = max( hs, needheight );
						}
					}

					if( haveheight < needheight )
					{
						int rest = ( needheight - haveheight ) % rs;

						// Distribute space across rows
						for( c = 0; c < rs; c++ )
						{
							lineheights[ y + c ] += ( needheight - haveheight ) / rs + rest;
							rest = 0;
						}
					}

					ci->processed_row = TRUE;
				}
			}
		}
	}

	for( y = 0; y < data->maxrow; y++ )
	{
		for( x = 0; x < data->maxcol; x++ )
		{
			struct cellinfo *ci;

			ci = data->cellarray[ y ][ x ]; 

			if( ci && !ci->processed_assigned )
			{
				int thisline;
				int rest;

				ci->li->yp = yp;

				ci->processed_assigned = TRUE;

				// Align cell...
				thisline = 0;
				for( c = 0; c < ci->rowspan; c++ )
				{
					thisline += lineheights[ y + c ];
					if( c )
					{
						thisline += data->cellspacing;
					}
				}

				rest = thisline - ci->li->ys;
				if( rest )
				{
					if( ci->li->valign == valign_top )
						rest = 0;
					else if( ci->li->valign == valign_middle )
						rest /= 2;
				}
				set( data->cellarray[ y ][ x ]->obj, MA_Layout_TopOffset, rest );

				ci->li->ys = thisline;

			}
		}
		yp += lineheights[ y ] + data->cellspacing;
	}

	//set( data->cellarray[ y ][ x ]->obj, MA_Layout_TopOffset, yp );

	li->xs = maxxp + getv( obj, MA_Layout_MarginRight );;
	li->ys = yp + getv( obj, MA_Layout_MarginBottom );

	DeletePool( data->pool );
	data->pool = NULL;

	D( db_html, bug( "finished TABLE_dolayout(%lx) -> %ld,%ld\r\n", obj, li->xs, li->ys ));

	if( li->xs > msg->suggested_width )
		reporterror( "%ld > %ld!\n", li->xs, msg->suggested_width );

	return( (ULONG)li );
}

static int realloccellarray( struct Data *data, int nmaxrow, int nmaxcol )
{
	struct cellinfo ***newa;
	int r;
	int omaxcol = data->amaxcol;
	int omaxrow = data->amaxrow;

	nmaxrow++;
	nmaxcol++;

	data->maxrow = max( data->maxrow, nmaxrow );
	data->maxcol = max( data->maxcol, nmaxcol );

	if( nmaxcol <= omaxcol && nmaxrow <= omaxrow )
		return 0; // Old matrix large enough	

	if( nmaxcol > omaxcol )
		data->amaxcol += max( 8, nmaxcol - omaxcol + 1 );
	if( nmaxrow > omaxrow )
		data->amaxrow += max( 8, nmaxrow - omaxrow + 1 );

	newa = AllocPooled( data->pool, data->amaxrow * sizeof( struct cellinfo** ) );
	if( !newa )
		return -1;
	if( data->amaxcol != omaxcol )
	{
		for( r = 0; r < data->amaxrow; r++ )
		{
			newa[ r ] = AllocPooled( data->pool, data->amaxcol * sizeof( struct cellinfo* ) );
			if( !newa[ r ] )
				return -1;
			if( r < omaxrow )
			{
				memcpy( newa[ r ], data->cellarray[ r ], omaxcol * sizeof( struct cellinfo* ) );
				FreePooled( data->pool, data->cellarray[ r ], omaxcol * sizeof( struct cellinfo* ) );
			}
		}
	}
	else
	{
		for( r = 0; r < omaxrow; r++ )
			newa[ r ] = data->cellarray[ r ];
		for( r = omaxrow; r < data->amaxrow; r++ )
		{
			newa[ r ] = AllocPooled( data->pool, data->amaxcol * sizeof( struct cellinfo* ) );
			if( !newa[ r ] )
				return( -1 );
		}
	}
	if( data->cellarray )
		FreePooled( data->pool, data->cellarray, omaxrow * sizeof( struct cellinfo** ) );

	data->cellarray = newa;

	return 0;
}

static void squashtable( struct Data *data )
{
	struct cellinfo ***newa = NULL;
	int *canloserow = NULL,*canlosecol = NULL;
	int newmaxcol,newmaxrow;
	int omaxcol,omaxrow;
	int ccol,crow;
	int x,y;

	if( data->maxrow == 1 && data->maxcol == 1 )
		return;

	omaxcol = newmaxcol = data->maxcol;
	omaxrow = newmaxrow = data->maxrow;
	canloserow = AllocPooled( data->pool, data->maxrow * sizeof( int ) );
	if( !canloserow )
		goto cleanup;
	canlosecol = AllocPooled( data->pool, data->maxcol * sizeof( int ) );
	if( !canlosecol )
		goto cleanup;
	for( y = 0; y != data->maxrow; y++ )
	{
		for( x = 0; x != data->maxcol; x++ )
			if( data->cellarray[ y ][ x ] && data->cellarray[ y ][ x ]->startrow == y )
				break;
		if( x == data->maxcol )
		{
			canloserow[ y ] = 1;
			for( x = 0; x != data->maxcol; x++ )
				if( data->cellarray[ y ][ x ] && data->cellarray[ y ][ x ]->startcol == x )
					data->cellarray[ y ][ x ]->rowspan--;
			newmaxrow--;
		}
	}
	for( x = 0; x != data->maxcol; x++ )
	{
		for( y = 0; y != data->maxrow; y++ )
			if( data->cellarray[ y ][ x ] && data->cellarray[ y ][ x ]->startcol == x )
				break;
		if( y == data->maxrow )
		{
			canlosecol[ x ] = 1;
			for( y = 0; y != data->maxrow; y++ )
				if( data->cellarray[ y ][ x ] && data->cellarray[ y ][ x ]->startrow == y )
					data->cellarray[ y ][ x ]->colspan--;
			newmaxcol--;
		}
	}

	if( ( newmaxrow == omaxrow && newmaxcol == omaxcol ) || ( newmaxrow == 0 ) || ( newmaxcol == 0 ) )
		goto cleanup;

	newa = AllocPooled( data->pool, newmaxrow * sizeof( struct cellinfo** ) );
	if( !newa )
		goto cleanup;
	for( y = 0; y < newmaxrow; y++ )
	{
		newa[ y ] = AllocPooled( data->pool, newmaxcol * sizeof( struct cellinfo* ) );
		if( !newa[ y ] )
			goto cleanup;
	}

	crow = 0;
	data->maxcolspan = 0;
	data->maxrowspan = 0;
	for( y = 0; y != omaxrow; y++ )
	{
		if( !canloserow[ y ] )
		{
			ccol = 0;
			for( x = 0; x != omaxcol; x++ )
			{
				if( canlosecol[ x ] )
					continue;
				if( !data->cellarray[ y ][ x ])
				{
					ccol++;
					continue;
				}
				if( data->cellarray[ y ][ x ]->startrow == y && data->cellarray[ y ][ x ]->startcol == x )
				{
					data->cellarray[ y ][ x ]->startrow = crow;
					data->cellarray[ y ][ x ]->startcol = ccol;
				}
				newa[ crow ][ ccol++ ] = data->cellarray[ y ][ x ];
				if( data->cellarray[ y ][ x ]->colspan > data->maxcolspan )
					data->maxcolspan = data->cellarray[ y ][ x ]->colspan;
				if( data->cellarray[ y ][ x ]->rowspan > data->maxrowspan )
					data->maxrowspan = data->cellarray[ y ][ x ]->rowspan;
			}
			crow++;
		}
		FreePooled( data->pool, data->cellarray[ y ], data->amaxcol * sizeof( struct cellinfo* ) );
	}
	FreePooled( data->pool, data->cellarray, data->amaxrow * sizeof( struct cellinfo** ) );
	data->cellarray = newa;
	newa = NULL; /* Prevent it getting cleaned up below */
	data->amaxcol = data->maxrow = newmaxrow;
	data->amaxrow = data->maxcol = newmaxcol;

cleanup:
	if( canloserow )
		FreePooled( data->pool, canloserow, omaxrow * sizeof( int ) );
	if( canlosecol )
		FreePooled( data->pool, canlosecol, omaxcol * sizeof( int ) );
	if( newa )
	{
		for( y = 0; y != newmaxcol; y++ )
			if( newa[ y ] )
				FreePooled( data->pool, newa[ y ], newmaxcol * sizeof( struct cellinfo* ) );
		FreePooled( data->pool, newa, newmaxrow * sizeof( struct cellinfo ** ) );
	}
}

static void spreadpercent( int totunpctdef, double totpct, int totdef, int maxel, int *def, double *pct )
{
	int x;
	double ratio;
	
	D( db_html, bug( "spreadpercent totunpctdef=%ld totpct=%ld totdef=%ld maxel=%ld\r\n", totunpctdef, (int)(totpct*10000.0), totdef, maxel ) );
	if( totunpctdef )
	{
		for( x = 0; x != maxel; x++ )
		{
			D( db_html, bug( "pct[ x ] = %ld   def[ x ] = %ld\r\n", (int)(pct[ x ]*10000.0), def[ x ] ) );
			if( pct[ x ] > -99999.0 )
				continue;
			ratio = (double)abs( def[ x ] );
			ratio /= (double)totunpctdef;
			ratio *= totpct;
			pct[ x ] = ratio;
		}
	}
	else if( totdef )
	{
		for( x = 0; x != maxel; x++ )
		{
			ratio = (double)def[ x ];
			ratio /= (double)totdef;
			ratio *= totpct;
			if( pct[ x ] < 0.0 )
				pct[ x ] -= ratio;
			else
				pct[ x ] += ratio;
		}
	}
}

static void calcdefwidth( int maxel, double *pct, int *def, int *min, int *defout, int *minout, int contwidth )
{
	int x,el=0;
	double minratio,v;

	minratio = 999999999.99;
	*minout = 0;

	for( x = 0; x < maxel; x++ )
	{
		if( def[ x ] )
			v = fabs( pct[ x ] ) / (double)abs( def[ x ] );
		else
			minratio = 0.0, v = 1.0;

		if( v < minratio )
		{
			minratio = v;
			el = x;
		}
		*minout += min[ x ];
	}
	if( minratio > 0.0 && minratio < 999999998 )
	{
		D( db_html, bug( "calcdefwidth using column %ld for width calc pct=%ld def=%ld\n", el, ((int)(pct[el]*10000.0)), def[el] ));
		minratio = 100.0;
		minratio /= fabs( pct[ el ] );
		minratio *= (double)abs( def[ el ] );
		minratio += 0.9999999;
		*defout = (int)minratio;
	}
	else
		*defout = max( contwidth, *minout );

	/* this should(?) never happen.  I think.  */
	if( *defout < *minout )
		*defout = *minout;
}

DECSMETHOD( Layout_CalcMinMax )
{
	GETDATA;
	APTR o, ostate;
	struct MinList *l;
	int col = 1, lastrow = 1;
	int marginadd;
	int objcount = 0;
	int x, y;
	struct layout_info *li = (APTR)getv( obj, MA_Layout_Info );
	char *widthspec;
	struct cellinfo *ci;

	if( data->pool )
		DeletePool( data->pool );
	data->pool = CreatePool( MEMF_CLEAR, 1024, 512 );
	if( !data->pool )
		return( (ULONG)li );

	D( db_html, bug( "in TABLE_calcminmax(%lx), sw=%ld, sh=%ld\n", obj, msg->suggested_width, msg->suggested_height ));

	data->cellarray = NULL;
	data->maxrow = 0;
	data->maxcol = 0;
	data->amaxcol = 0;
	data->amaxrow = 0;

	// Copy to cellarray

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->mlh_Head;
	// Skip dummy
	NextObject( &ostate );
	col = 0;
	lastrow = 0;
	while( o = NextObject( &ostate ) )
	{
		int row;
		int x, y;

		objcount++;

		get( o, MA_Layout_Cell_Row, &row );
		row--;

#ifdef VDEBUG
		if( row < 0 )
		{
			MUI_Request( app, NULL, 0, "Error", "Hrmpf", "row %ld!", row );
			row = 0;
		}
#endif

		if( row > lastrow )
		{
			col = 0;
			lastrow = row;
		}

		if( realloccellarray( data, row, col ) )
		{
			DeletePool( data->pool );
			data->pool = NULL;
			return( (ULONG)li );
		}
		while( data->cellarray[ row ][ col ] )
		{
			// Cell already used by spanning cell, shift right
			col++;
			if( realloccellarray( data, row, col ) )
			{
				DeletePool( data->pool );
				data->pool = NULL;
				return( (ULONG)li );
			}
		}

		ci = AllocPooled( data->pool, sizeof( struct cellinfo ) );
		if( !ci )
		{
			DeletePool( data->pool );
			data->pool = NULL;
			return( (ULONG)li );
		}

		get( o, MA_Layout_Cell_Rowspan, &ci->rowspan );
		get( o, MA_Layout_Cell_Colspan, &ci->colspan );
		get( o, MA_Layout_Width, &ci->widthspec );
		get( o, MA_Layout_Height, &ci->heightspec );

		data->maxrowspan = max( data->maxrowspan, ci->rowspan );
		data->maxcolspan = max( data->maxcolspan, ci->colspan );

		ci->obj = o;
		ci->li = (APTR)DoMethod( ci->obj, MM_Layout_CalcMinMax, msg->suggested_width, msg->suggested_height, 0 );

		if( ci->widthspec )
		{
			//dprintf( "ci->widthspec at %p, obj class: <%s>\n", ci->widthspec, OCLASS( o )->cl_ID );
			if( strchr( ci->widthspec, '%' ) )
			{
		 		ci->pctwidth = atoi( ci->widthspec );
		 		ci->abswidth = -1;
		 	}
	 		else
	 		{
	 			ci->pctwidth = -99999;
	 			ci->abswidth = atoi( ci->widthspec );
	 		}
	 	}
	 	else
	 	{
	 		ci->pctwidth = -99999;
	 		ci->abswidth = -1;
	 	}
	 	if( ci->heightspec )
	 	{
	 		if( strchr( ci->heightspec, '%' ) )
	 		{
	 			ci->pctheight = atoi( ci->heightspec );
	 			ci->absheight = -1;
	 		}
	 		else
	 		{
	 			ci->pctheight = -1;
	 			ci->absheight = atoi( ci->heightspec );
	 		}
	 	}
	 	else
	 	{
	 		ci->pctheight = -1;
	 		ci->absheight = -1;
	 	}

		ci->startcol = col;
		ci->startrow = row;

		if( realloccellarray( data, row + ci->rowspan - 1, col + ci->colspan - 1 ) )
		{
			DeletePool( data->pool );
			data->pool = NULL;
			return( (ULONG)li );
		}

		for( x = 0; x < ci->colspan; x++ )
		{
			for( y = 0; y < ci->rowspan; y++ )
			{
				data->cellarray[ row + y ][ col + x ] = ci;
			}
		}

		col += ci->colspan;

	}

	D( db_html, bug( "cell matrix size %ld x %ld, %ld objs\n", data->maxcol, data->maxrow, objcount ));

	if( !data->maxcol )
	{
		// Table still empty, possibly during incremental layout
		return( (ULONG)li );
	}

	squashtable( data );

	data->colpct = AllocPooled( data->pool, data->maxcol * sizeof( double ) );
	data->colmin = AllocPooled( data->pool, data->maxcol * sizeof( int ) );
	data->coldef = AllocPooled( data->pool, data->maxcol * sizeof( int ) );

	if( data->colpct == NULL || data->colmin == NULL || data->coldef == NULL )
	{
		DeletePool( data->pool );
		data->pool = NULL;
		return( (ULONG)li );
	}

	data->coltotpct = 100.0;
	data->coltotdef = 0;
	data->coltotunpctdef = 0;

	/* First, go through and find the biggest percentage, minimum and default widths for each column of the table */
	for( x = 0; x < data->maxcol; x++ )
	{
		data->colpct[ x ] = -99999.0;
		data->colmin[ x ] = -1;
		data->coldef[ x ] = 0;
		for( y = 0; y < data->maxrow; y++ )
		{
			ci = data->cellarray[ y ][ x ];
			if( !ci || ci->colspan>1 )
				continue;

			if( (double)ci->pctwidth > data->colpct[ x ] )
				data->colpct[ x ] = (double)ci->pctwidth;

			if( ci->li->minwidth > data->colmin[ x ] )
				data->colmin[ x ] = ci->li->minwidth;

			if( ci->abswidth > 0 && -ci->abswidth < data->coldef[ x ] )
				data->coldef[ x ] = -ci->abswidth;
			else if( data->coldef[ x ] >= 0 && ci->li->defwidth > data->coldef[ x ] )
				data->coldef[ x ] = ci->li->defwidth; 
			D( db_html, bug( "After row %ld coldef[ %ld ]=%ld (ci->li->minwidth=%ld ci->li->defwidth=%ld)\n", y, x, data->coldef[ x ], ci->li->minwidth, ci->li->defwidth ));
		}
	}

/*	for( x = 0; x < data->maxcol; x++ )
	{
		if( data->coldef[ x ] < 0 )
		{
			data->coldef[ x ] = -data->coldef[ x ];
		}
	}
*/
	for( y = 0; y < data->maxrow; y++ )
		for( x = 0; x < data->maxcol; x++ )
		{
			ci = data->cellarray[ y ][ x ];
			if( ci && ci->colspan > 1 && ci->startcol == x && ci->li->minwidth )
			{
				int cs,mintot=0,deftot=0,deforabs;

				deforabs = ci->abswidth > 0 ? ci->abswidth : ci->li->defwidth;
				for( cs = 0; cs != ci->colspan; cs++ )
				{
					mintot += data->colmin[ x + cs ];
					deftot += abs( data->coldef[ x + cs ] );
				}
				if( !mintot )
					for( cs = 0; cs != ci->colspan; cs++ )
						data->colmin[ x + cs ] = ci->li->minwidth / ci->colspan;
				else if( ci->li->minwidth > mintot )
				{
					int nv,csmin;
					csmin = ci->li->minwidth;
retry:
					for( cs = 0; cs != ci->colspan; cs++ )
					{
						if( data->colmin[ x + cs ] < 0 )
							continue;
						if(deftot)
							nv = ( csmin * abs( data->coldef[ x + cs ] ) ) / deftot;
						else
							nv = csmin / ci->colspan;
						if( nv < data->colmin[ x + cs ] )
						{
							csmin -= data->colmin[ x + cs ];
							mintot -= data->colmin[ x + cs ];
							data->colmin[ x + cs ] = -data->colmin[ x + cs ];
							goto retry;
						}
						data->colmin[ x + cs ] = nv;
					}
					mintot = 0;
					for( cs = 0; cs != ci->colspan; cs++ )
					{
						data->colmin[ x + cs ] = abs( data->colmin[ x + cs ] );
						mintot += data->colmin[ x + cs ];
					}
					if( mintot < ci->li->minwidth )
						data->colmin[ x + ci->colspan -1 ] += ci->li->minwidth - mintot;
				}
				if( !deforabs )
					continue;
				if( !deftot )
					for( cs = 0; cs != ci->colspan; cs++ )
						data->coldef[ x + cs ] = deforabs / ci->colspan;
				else if( deforabs > deftot )
					for( cs = 0; cs != ci->colspan; cs++ )
						data->coldef[ x + cs ] = ( deforabs * data->coldef[ x + cs ] ) / deftot;
			}
		}
	
	for( x = 0; x < data->maxcol; x++ )
	{
		if( data->colmin[ x ] > abs( data->coldef[ x ] ) )
			data->coldef[ x ] = data->colmin[ x ];
		
		if( fabs( data->colpct[ x ] ) > 0.0 && data->colpct[ x ] != -99999.0 )
		{
			data->colpct[ x ] = -data->colpct[ x ];
			data->coltotpct += data->colpct[ x ];
		}
		else if( data->colpct[ x ] == -99999.0 && abs( data->coldef[ x ] ) > 0 )
			data->coltotunpctdef += abs( data->coldef[ x ] );

		if( abs( data->coldef[ x ] ) > 0 )
			data->coltotdef += abs( data->coldef[ x ] );
	}

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P1a Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], (int)( data->colpct[ x ] * 10000.0 ) ); });

	if( data->coltotpct > 0.0 )
		spreadpercent( data->coltotunpctdef, data->coltotpct, data->coltotdef, data->maxcol, data->coldef, data->colpct );
//	else
	{
		int pctcutoff = 0;
		data->coltotpct = 0.0;
		for( x = 0; x < data->maxcol; x++ )
		{
			if( data->colpct[ x ] != -99999.0 && data->coltotpct + fabs( data->colpct[ x ] ) >= 100.0 )
			{
				data->colpct[ x ] = 100.0 - data->coltotpct;
				pctcutoff = 1;
			}
			else if( data->colpct[ x ] == -99999.0 )
				data->colpct[ x ] = 0.0;
			data->coltotpct += fabs( data->colpct[ x ] );
		}
		if( pctcutoff )
		{
			for( x = 0; x < data->maxcol; x++ )
				if( data->coldef[ x ] < 0 )
				{
					data->coldef[ x ] = -data->coldef[ x ];
				}
		}
			
		data->coltotpct = 0.0;
	}

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P1b Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], (int)( data->colpct[ x ] * 10000.0 ) ); });
	
	calcdefwidth( data->maxcol, data->colpct, data->coldef, data->colmin, &( li->defwidth ), &( li->minwidth ), msg->suggested_width );

	D( db_html, int x; for( x = 0; x != data->maxcol; x++ ) { bug( "P2 Col %ld min %ld def %ld pct %ld\r\n", x, data->colmin[ x ], data->coldef[ x ], (int)( data->colpct[ x ] * 10000.0 ) ); });

	// Add cell spacing to the table minimum width
	marginadd = getv( obj, MA_Layout_MarginLeft ) + getv( obj, MA_Layout_MarginRight );

	marginadd += ( data->maxcol + 1 ) * data->cellspacing;

	li->minwidth += marginadd;
	li->defwidth += marginadd;
	
	// Check whether we have a width specification for the whole table
	get( obj, MA_Layout_Width, &widthspec );
	if( widthspec )
	{
		int specwidth = atoi( widthspec );
		if( strchr( widthspec, '%' ) )
		{
			if( msg->window_width > 0 )
			{
				specwidth = max( 1, specwidth );
				specwidth = ( specwidth * msg->window_width ) / 100;
				specwidth = max( specwidth, li->minwidth );

				li->minwidth = specwidth;
				li->defwidth = specwidth;
			}
		}
		else
		{
			specwidth = max( specwidth, li->minwidth );

			li->minwidth = specwidth;
			li->defwidth = specwidth;
		}
	}

	// TOFIX! Check whether this can go completely
	li->minheight = li->defheight = li->maxheight = 0;

	D( db_html, bug( "finished TABLE_calcminmax(%lx) -> %ld,%ld %ld,%ld \n", obj, li->minwidth, li->minheight, li->defwidth, li->defheight ));

	return( (ULONG)li );
}

BEGINMTABLE
DEFNEW
DEFSET
DEFDISPOSE
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_CalcMinMax )
ENDMTABLE

int create_lotableclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getlogroupmcc(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lotableClass";
#endif

	return( TRUE );
}

void delete_lotableclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlotableclass( void )
{
	return( lcc->mcc_Class );
}
