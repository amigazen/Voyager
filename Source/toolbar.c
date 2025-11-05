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
 * Toolbar class
 * -------------
 * - Virtgroup subclass with visual drag & drop capabilities
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: toolbar.c,v 1.14 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"
#include "classes.h"
#include "mui_func.h"

/*
 * Instance data
 */
struct Data {
	int dummy;
};


DECNEW
{
	struct Data *data;

	if( !( obj = (Object *)DoSuperNew( cl, obj,
										TAG_MORE, msg->ops_AttrList ) ) )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	return( ( ULONG )obj );
}


BEGINMTABLE
DEFNEW
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_toolbarclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Virtgroup, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "ToolbarClass";
#endif

	return( TRUE );
}

void delete_toolbarclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR gettoolbarclass( void )
{
	return( mcc->mcc_Class );
}
