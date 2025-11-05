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
 * Toolbar Button class
 * --------------------
 * - Mostly used because I need a MUI_ContextMenuBuild.
 *
 * © 1999-2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: button.c,v 1.11 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* private */
#include "classes.h"
#include "htmlwin.h"
#include "htmlclasses.h"
#include "prefs.h"
#include "mui_func.h"
#include "speedbar.h"


/* instance data */
struct Data
{
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


DECMMETHOD( ContextMenuBuild )
{
	switch( muiUserData( obj ) )
	{
		case bfunc_forward:
			return( DoMethod( app, MM_DoActiveWin, MM_HTMLWin_BuildButtonHistory, bfunc_forward ) );
		case bfunc_back:
			return( DoMethod( app, MM_DoActiveWin, MM_HTMLWin_BuildButtonHistory, bfunc_back ) );
	}
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMMETHOD( ContextMenuBuild )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_buttonclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

#if USE_SPEEDBAR
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_SpeedButton, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
#else
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Text, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
#endif /* !USE_SPEEDBAR */
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "ButtonClass";
#endif

	return( TRUE );
}

void delete_buttonclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getbuttonclass( void )
{
	return( mcc->mcc_Class );
}
