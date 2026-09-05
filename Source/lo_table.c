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
/* Toggle VAPOR_H_BROKEN for lo_ files - opposite of global config */
#ifdef VAPOR_H_BROKEN
#undef VAPOR_H_BROKEN
#else
#define VAPOR_H_BROKEN
#endif
#include <math.h>
#include "dos_func.h"

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

/* Upper bound for the spanning-cell column walk in Layout_CalcMinMax */
#define MAX_TABLE_COLS 512

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
	int colmintot;
	int colsnofixedpct;

	get( obj, MA_Layout_Info, &li );

	li->flags &= ~LOF_NEW;

	if( data->pool == NULL || data->maxcol == 0 || data->colpct == NULL
		|| data->colmin == NULL || data->coldef == NULL )
	{
		// Table still empty, possibly during incremental layout.
		// data->pool is NULL when CalcMinMax hit an allocation failure and
		// threw the pool away; AllocPooled( NULL, ... ) below would crash.
		VoyLog(( "[TABLE] DoLayout skip obj=%lx pool=%lx maxcol=%ld\n",
			(ULONG)obj, (ULONG)data->pool, (long)data->maxcol ));
		VoyFlush();
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

	// Calculate final table output width. Never shrink below minwidth:
	// WIDTH=800 or WIDTH=100% on a narrower window used to clip the
	// table, then the column loop below retried forever.
	/* AWeb Layouttable: WIDTH is a hint on the containing block
	 * (suggested_width). Shrink to the container when content fits;
	 * never go below minwidth (that caused the old infinite retry). */
	get( obj, MA_Layout_Width, &widthspec );
	if( widthspec )
	{
		int specwidth;
		int pctbase;
		int container;

		specwidth = atoi( widthspec );
		container = msg->suggested_width;
		if( container < 1 )
			container = msg->outer_width;
		if( strchr( widthspec, '%' ) )
		{
			specwidth = max( 1, specwidth );
			pctbase = msg->outer_width;
			if( pctbase < 1 )
				pctbase = container;
			if( pctbase < 1 )
				pctbase = 1;
			specwidth = ( specwidth * pctbase ) / 100;
		}
		tablewidth = specwidth;
		if( container > 0 && tablewidth > container && li->minwidth <= container )
			tablewidth = container;
	}
	else
	{
		tablewidth = min( tablewidth, li->defwidth );
	}
	if( tablewidth < li->minwidth )
		tablewidth = li->minwidth;

	innertablewidth = tablewidth - ( data->maxcol + 1 ) * data->cellspacing - getv( obj, MA_Layout_MarginLeft ) - getv( obj, MA_Layout_MarginRight );
	if( innertablewidth < 1 )
		innertablewidth = 1;

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
		colsnofixedpct = 0;
		for( x = 0; x < data->maxcol; x++ )
		{
			if( data->colpct[ x ] >= 0.0 && data->coldef[ x ] >= 0 )
				colsnofixedpct++;
		}
		if( colsnofixedpct )
		{
			for( x = 0; x < data->maxcol; x++ )
			{
				if( data->colpct[ x ] >= 0.0 && data->coldef[ x ] >= 0 )
					data->colpct[ x ] += rempct / (double)colsnofixedpct;
			}
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

	colmintot = 0;
	for( x = 0; x < data->maxcol; x++ )
	{
		if( data->colmin[ x ] < 0 )
			data->colmin[ x ] = 0;
		colmintot += data->colmin[ x ];
	}

	usetabwidth = innertablewidth;
	usepct = 100.0;
retry:
	if( usetabwidth < 0 )
		usetabwidth = 0;
	deftot = 0;
	for( x = 0; x < data->maxcol; x++ )
	{
		if( usepct > 0.0 && data->colpct[ x ] >= 0.0 && usetabwidth > 0 )
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
	/* Rounding can overshoot by a pixel. If column minima already exceed
	 * the container (nowrap text, WIDTH=800 nav bars, forum date cells)
	 * shrinking usetabwidth never catches up and layout never finishes. */
	if( deftot > innertablewidth && colmintot < innertablewidth && usetabwidth > 0 )
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
					int cellwidth;
					int cellheight;

					cellwidth = width;
					if( ci->li && ci->li->minwidth > cellwidth )
						cellwidth = ci->li->minwidth;
					cellheight = ( ci->absheight < 0 ) ? msg->suggested_height : ci->absheight;
					DoMethod( ci->obj, MM_Layout_DoLayout, cellwidth, cellheight, cellwidth );
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
	/*
	 * Everything below was allocated from that pool. Clear the pointers as
	 * well, otherwise a second Layout_DoLayout without an intervening
	 * CalcMinMax sees non-NULL values, decides the widths are valid and reads
	 * freed memory.
	 */
	data->cellarray = NULL;
	data->colpct = NULL;
	data->colmin = NULL;
	data->coldef = NULL;

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
			memset( newa[ r ], 0, data->amaxcol * sizeof( struct cellinfo* ) );
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
			memset( newa[ r ], 0, data->amaxcol * sizeof( struct cellinfo* ) );
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
	/*
	 * Only the losable entries are assigned below, but every entry is read
	 * again when the matrix is rebuilt. AllocPooled does not clear reused
	 * puddle memory, so rows/columns were kept or dropped at random.
	 */
	memset( canloserow, 0, data->maxrow * sizeof( int ) );
	memset( canlosecol, 0, data->maxcol * sizeof( int ) );
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
	/*
	 * amaxrow/amaxcol describe the allocation: newa is newmaxrow rows of
	 * newmaxcol entries. They were assigned the other way round, so a later
	 * realloccellarray() sized and memcpy'd the matrix wrongly.
	 */
	data->amaxrow = data->maxrow = newmaxrow;
	data->amaxcol = data->maxcol = newmaxcol;

cleanup:
	if( canloserow )
		FreePooled( data->pool, canloserow, omaxrow * sizeof( int ) );
	if( canlosecol )
		FreePooled( data->pool, canlosecol, omaxcol * sizeof( int ) );
	if( newa )
	{
		/* newa has newmaxrow rows, not newmaxcol */
		for( y = 0; y != newmaxrow; y++ )
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

	VoyLog(( "[TABLE] CalcMinMax enter obj=%lx sw=%ld sh=%ld\n",
		(ULONG)obj, (long)msg->suggested_width, (long)msg->suggested_height ));
	VoyFlush();

	if( data->pool )
		DeletePool( data->pool );
	data->pool = CreatePool( MEMF_CLEAR, 1024, 512 );
	if( !data->pool )
		return( (ULONG)li );

	D( db_html, bug( "in TABLE_calcminmax(%lx), sw=%ld, sh=%ld\n", obj, msg->suggested_width, msg->suggested_height ));

	data->cellarray = NULL;
	/*
	 * These three live in the pool that was just deleted. Layout_DoLayout
	 * treats a non-NULL value as "widths are valid", so leaving them dangling
	 * made it read freed memory whenever CalcMinMax returned early - which it
	 * does for every still-empty table during incremental layout.
	 */
	data->colpct = NULL;
	data->colmin = NULL;
	data->coldef = NULL;
	data->maxrow = 0;
	data->maxcol = 0;
	data->amaxcol = 0;
	data->amaxrow = 0;

	// Copy to cellarray

	get( obj, MUIA_Group_ChildList, &l );
	ostate = l->mlh_Head;
	col = 0;
	lastrow = 0;
	while( o = NextObject( &ostate ) )
	{
		int row;
		int x, y;

		/* Dummy (and any other MUI helper child) has no cell row.
		 * Skipping a fixed first child used to drop the real first
		 * cell when Dummy was not in ChildList; a failed get() also
		 * left row uninitialized. */
		/* Dummy has no row. Do not require get() to return TRUE:
		 * on some MUI builds GetAttr returns 0 after storing the
		 * value, which skipped every cell and left only 1px borders. */
		row = 0;
		get( o, MA_Layout_Cell_Row, &row );
		if( row < 1 )
			continue;

		objcount++;
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
			/*
			 * A bogus COLSPAN/ROWSPAN can fill the row and make this walk
			 * grow the cell array without end, so cap it at a width no real
			 * table reaches rather than allocating until the machine dies.
			 */
			if( col > MAX_TABLE_COLS )
			{
				VoyLog(( "[TABLE] CalcMinMax runaway column scan obj=%lx row=%ld col=%ld\n",
					(ULONG)obj, (long)row, (long)col ));
				VoyFlush();
				DeletePool( data->pool );
				data->pool = NULL;
				return( (ULONG)li );
			}
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
		if( !ci->li )
		{
			FreePooled( data->pool, ci, sizeof( struct cellinfo ) );
			continue;
		}
		/* AWeb Measuretable: width >= 1 so a cell cannot collapse the table. */
		if( ci->li->minwidth < 1 )
			ci->li->minwidth = 1;
		if( ci->li->defwidth < ci->li->minwidth )
			ci->li->defwidth = ci->li->minwidth;

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

	VoyLog(( "[TABLE] CalcMinMax cells=%ld matrix=%ldx%ld\n",
		(long)objcount, (long)data->maxcol, (long)data->maxrow ));
	VoyFlush();

	if( !data->maxcol )
	{
		// Table still empty, possibly during incremental layout
		return( (ULONG)li );
	}

	squashtable( data );

	VoyLog(( "[TABLE] squashed matrix=%ldx%ld maxcolspan=%ld maxrowspan=%ld\n",
		(long)data->maxcol, (long)data->maxrow, (long)data->maxcolspan, (long)data->maxrowspan ));
	VoyFlush();

	VoyLog(( "[TABLE] alloc pool=%lx maxcol=%ld\n", (ULONG)data->pool, (long)data->maxcol ));
	VoyFlush();

	data->colpct = AllocPooled( data->pool, data->maxcol * sizeof( double ) );
	VoyLog(( "[TABLE] alloc colpct=%lx\n", (ULONG)data->colpct ));
	VoyFlush();
	data->colmin = AllocPooled( data->pool, data->maxcol * sizeof( int ) );
	VoyLog(( "[TABLE] alloc colmin=%lx\n", (ULONG)data->colmin ));
	VoyFlush();
	data->coldef = AllocPooled( data->pool, data->maxcol * sizeof( int ) );
	VoyLog(( "[TABLE] alloc coldef=%lx\n", (ULONG)data->coldef ));
	VoyFlush();

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
			if( !ci || !ci->li || ci->colspan > 1 )
				continue;
			if( ci->startcol != x || ci->startrow != y )
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
	VoyLog(( "[TABLE] colscan done\n" ));
	VoyFlush();

	for( y = 0; y < data->maxrow; y++ )
		for( x = 0; x < data->maxcol; x++ )
		{
			ci = data->cellarray[ y ][ x ];
			if( ci && ci->li && ci->colspan > 1 && ci->startcol == x && ci->li->minwidth )
			{
				int cs,mintot=0,deftot=0,deforabs;
				int retries = 0;

				/*
				 * colmin/coldef only have maxcol entries. A colspan reaching
				 * past the last column (possible after squashtable trims the
				 * matrix) would read and write off the end of both arrays.
				 */
				if( x + ci->colspan > data->maxcol )
				{
					VoyLog(( "[TABLE] colspan clamp x=%ld colspan=%ld maxcol=%ld\n",
						(long)x, (long)ci->colspan, (long)data->maxcol ));
					VoyFlush();
					ci->colspan = data->maxcol - x;
					if( ci->colspan <= 1 )
						continue;
				}

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
							/*
							 * A column is taken out of the redistribution by
							 * negating its minimum, which the test above then
							 * skips. That only works for a minimum above zero:
							 * an empty cell leaves colmin at 0, and once csmin
							 * has gone negative nv is negative too, so this
							 * branch would negate 0, subtract 0 and retry for
							 * ever. Keep 0 as the floor in that case.
							 */
							if( data->colmin[ x + cs ] > 0 && retries++ < ci->colspan )
							{
								csmin -= data->colmin[ x + cs ];
								mintot -= data->colmin[ x + cs ];
								data->colmin[ x + cs ] = -data->colmin[ x + cs ];
								goto retry;
							}

							nv = data->colmin[ x + cs ] > 0 ? data->colmin[ x + cs ] : 0;
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
	
	VoyLog(( "[TABLE] colspan done\n" ));
	VoyFlush();

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
	
	VoyLog(( "[TABLE] pct done coltotpct=%ld\n", (long)data->coltotpct ));
	VoyFlush();

	calcdefwidth( data->maxcol, data->colpct, data->coldef, data->colmin, &( li->defwidth ), &( li->minwidth ), msg->suggested_width );

	VoyLog(( "[TABLE] calcdefwidth done minw=%ld defw=%ld\n", (long)li->minwidth, (long)li->defwidth ));
	VoyFlush();

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
			int pctbase;

			/* Nested WIDTH=100% is the containing cell, not the window.
			 * Using window_width made every inner 100% table as wide as
			 * the view, then padding (CELLPADDING=30/40) crushed it. */
			pctbase = msg->suggested_width;
			if( pctbase < 1 )
				pctbase = msg->window_width;
			if( pctbase > 0 )
			{
				specwidth = max( 1, specwidth );
				specwidth = ( specwidth * pctbase ) / 100;
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

	VoyLog(( "[TABLE] CalcMinMax exit obj=%lx minw=%ld defw=%ld\n",
		(ULONG)obj, (long)li->minwidth, (long)li->defwidth ));
	VoyFlush();

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
