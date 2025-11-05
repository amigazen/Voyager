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
** $Id: lo_dummy.c,v 1.8 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"
#include "classes.h"
#include "htmlclasses.h"
#include "layout.h"
#include "mui_func.h"

static struct MUI_CustomClass *lcc;

struct Data {
	int dummy;
};

DECCONST
{
	obj = DoSuperNew( cl, obj,
		MUIA_CustomBackfill, TRUE,
		MUIA_FillArea, FALSE,
		TAG_MORE, msg->ops_AttrList
	);

	return( (ULONG)obj );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth = MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight = MUI_MAXMAX;

	return( 0 );
}

BEGINMTABLE
DEFCONST
DEFMMETHOD( AskMinMax )
ENDMTABLE

int create_lodummyclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "lodummyClass";
#endif

	return( TRUE );
}

void delete_lodummyclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getlodummyclass( void )
{
	return( lcc->mcc_Class );
}
