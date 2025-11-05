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
** $Id: lo_map.c,v 1.13 2003/07/06 16:51:33 olli Exp $
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
#include "malloc.h"
#include "mui_func.h"


static struct MUI_CustomClass *lcc;

struct Data {
	struct layout_info li;
	struct layout_ctx *ctx;
	char *name;
	char *url;
	char *target;
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

		case MA_Layout_Map_Name:
			l_readstrtag( tag, &data->name );
			break;

		case MA_Layout_Map_DefaultURL:
			l_readstrtag( tag, &data->url );
			break;

		case MA_Layout_Map_DefaultTarget:
			l_readstrtag( tag, &data->target );
			break;
	}

	return( redraw );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Map",
		MA_JS_Object_TerseArray, TRUE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	doset( data, obj, msg->ops_AttrList );

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->name );
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

		case MA_Layout_Map_Name:
			*msg->opg_Storage = (ULONG)data->name;
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

DECMMETHOD( Draw )
{
	return( 0 );
}

DECSMETHOD( Layout_Map_FindByName )
{
	GETDATA;
	char *n = msg->name;

	if( *n == '#' )
		n++;

	if( !stricmp( data->name, n ) )
		*msg->objptr = obj;

	return( TRUE );
}

DECSMETHOD( Layout_Map_FindArea )
{
	struct MinList *l;
	struct customprop *cp;

	get( obj, MA_JS_Object_CPL, &l );

	for( cp = FIRSTNODE( l ); NEXTNODE( cp ); cp = NEXTNODE( cp ) )
	{
		if( DoMethodA( cp->obj, (Msg)msg ) )
			return( (ULONG)cp->obj );
	}
	return( (ULONG)NULL );
}


BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
DEFSMETHOD( Layout_CalcMinMax )
DEFSMETHOD( Layout_DoLayout )
DEFSMETHOD( Layout_Map_FindByName )
DEFSMETHOD( Layout_Map_FindArea )
DEFMMETHOD( Draw )
ENDMTABLE

int create_lomapclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_array_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lomapClass";
#endif

	return( TRUE );
}

void delete_lomapclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlomapclass( void )
{
	return( lcc->mcc_Class );
}
