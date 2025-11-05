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
**	Javascript Garbage Collector
**	----------------------------
**
**	(C) 2001-2002 Oliver Wagner <owagner@vapor.com>, All Rights Reserved
**
**	$Id: js_gc.c,v 1.15 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"
#include "copyright.h"
#include "voyager_cat.h"
#include "prefs.h"
#include "classes.h"
#include "js.h"
#include "mui_func.h"
#include "win_func.h"

#define OBJLISTINC 1024
static APTR *objlist;
static int objlistsize;
static ULONG gcmagic;

/*
 *	Register an object to the garbage collector
 */
void js_gc_add( APTR obj )
{
	int c;
	APTR *newobjlist;

#ifdef VDEBUG
	if( !DoMethod( obj, MM_JS_GetGCMagic ) )
	{
		ALERT( ( "object %lx (%s) did not return a gcmagic upon add!\n", obj, OCLASS( obj )->cl_ID ) );
	}
#endif

	// Try to find a free slot
	for( c = 0; c < objlistsize; c++ )
	{
		if( !objlist[ c ] )
		{
			// Found!
			objlist[ c ] = obj;
			return;
		}
	}

	// Ok, we need to grow the list
	newobjlist = malloc( ( objlistsize + OBJLISTINC ) * sizeof( APTR ) );
	if( !newobjlist )
	{
		reporterror( "Out of mem for GC (%ld)", objlistsize );
		return;
	}
	memcpy( newobjlist, objlist, objlistsize * sizeof( APTR ) );
	memset( &newobjlist[ objlistsize ], 0, OBJLISTINC * sizeof( APTR ) );
	free( objlist );
	objlist = newobjlist;

	objlist[ objlistsize ] = obj;
	objlistsize += OBJLISTINC;

	D( db_js, bug( "added object 0x%lx\n", obj ) );
}

/*
 *	Run the garbage collector. A new gcmagic is broadcasted to all
 *  the objects, then we remove all the objects without this
 *  magic.
 */
void js_gc( void )
{
	int c;
	APTR htmlwinclass = gethtmlwinclass();
int activeobj=0;

	gcmagic++;
	D( db_js, bug( "running GC with magic %lx\n", gcmagic ) );
	doallwins( MM_JS_SetGCMagic, gcmagic );

	for( c = 0; c < objlistsize; c++ )
	{
		if( objlist[ c ] && OCLASS( objlist[ c ] ) == htmlwinclass )
		{
			ULONG thismagic;

			thismagic = DoMethod( objlist[ c ], MM_JS_GetGCMagic );
			D( db_js, bug( "checking object %lx (%s), name %s, magic = %lx, _parent = %lx\n", objlist[ c ], OCLASS( objlist[ c ] )->cl_ID, getv( objlist[ c ], MA_JS_Name ) ? (char*)getv( objlist[ c ], MA_JS_Name ) : "(anonymous)", thismagic, _parent( objlist[ c ] ) ) );
#ifdef VDEBUG
			if( !thismagic )
				reporterror( "Object %lx has no GCMagic", objlist[ c ] );
			else 
#endif
			if( thismagic != gcmagic )
			{
				if( _parent( objlist [ c ] ) )
				{
					// Object still has a parent, we cannot possibly
					// kill it. Send it a gc message, so it's members
					// remain
					DoMethod( objlist[ c ], MM_JS_SetGCMagic, gcmagic );
					D( db_js, bug( "not finalized due to _parent = %lx\n", _parent( objlist[ c ] ) ) );
activeobj++;
				}
				else
				{
					D( db_js, bug( "-> FINALIZING\n" ) );
					MUI_DisposeObject( objlist[ c ] );
					objlist[ c ] = NULL;
				}
			}
else
activeobj++;
		}
	}

	for( c = 0; c < objlistsize; c++ )
	{
		if( objlist[ c ] && OCLASS( objlist[ c ] ) != htmlwinclass )
		{
			ULONG thismagic;

			thismagic = DoMethod( objlist[ c ], MM_JS_GetGCMagic );
			D( db_js, bug( "checking object %lx (%s), name %s, magic = %lx\n", objlist[ c ], OCLASS( objlist[ c ] )->cl_ID, getv( objlist[ c ], MA_JS_Name ) ? (char*)getv( objlist[ c ], MA_JS_Name ) : "(anonymous)", thismagic ) );
#ifdef VDEBUG

			if( !thismagic )
				reporterror( "Object %lx has no GCMagic", objlist[ c ] );
			else 
#endif
				if( thismagic != gcmagic )
			{
				D( db_js, bug( "-> FINALIZING\n" ) );
				MUI_DisposeObject( objlist[ c ] );
				objlist[ c ] = NULL;
			}
else
activeobj++;
		}
	}

//kprintf( "GC STATS: active %ld, objlistsize %ld\n", activeobj, objlistsize );
}

/*
 *	Cleanup any remaining objects
 */
void js_gc_cleanup( void )
{
	int c;
	int done = FALSE;

	while( !done )
	{
		done = TRUE;
		for( c = 0; c < objlistsize; c++ )
		{
			if( objlist[ c ] )
			{
				done = FALSE;
				js_gc();
				break;
			}
		}
	}
}
