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
 * Pip window class
 * -----------------
 * - ..
 *
 * © 1999-2003 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: pipwindow.c,v 1.7 2003/07/06 16:51:34 olli Exp $
 *
*/

// FIMXE: mbx only

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/layers.h>
#endif

/* private */
#include "voyager_cat.h"
#include "copyright.h"
#include "classes.h"
#include "mui_func.h"
#if USE_STB_NAV
#include "mcp_lib_calls.h"
#include <modules/mbxgui/classes.h>
#endif

#ifdef MBX

//#define PIP_MOUSEMOVEABLE

struct Data {
	int position;
	int size;

	Object *pip;
};

int saveValueToEnv (char *str, UDWORD v)
{
	EnvNode_p r,e;
	int rc = 0;
	
	r = EnterTree(NULL, "", ENVMODE_MODIFY);
	if (r) {
		if (CreateEnvTree(r, "www/options")) {
			e = EnterTree(NULL, "www/options", ENVMODE_MODIFY);
			if (e) {
				WriteEnvUDword(e, str, v, 0L);
				rc = 1;
				LeaveTree (e);
			}
		}
		LeaveTree(r);
	}
	return rc;
}

static int doset( struct Data *data, APTR obj, struct TagItem *tags )
{
	struct TagItem *tag;
	int reposition = 0;
	
	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
	case MA_Pipwindow_Position:
		if (data->position!=tag->ti_Data) {
			// FIXME: move to a global save function, called on box shutdown
			saveValueToEnv ("pipPos", tag->ti_Data);
			data->position=tag->ti_Data;
			reposition = 1;
		}
		break;
	case MA_Pipwindow_Size:
		if (data->size!=tag->ti_Data) {
			// FIXME: move to a global save function, called on box shutdown
			saveValueToEnv ("pipSize", tag->ti_Data);			
			data->size=tag->ti_Data;
			reposition = 1;
		}
		break;
	}

	return reposition;
	
}

DECTMETHOD ( Pipwindow_CalculatePosition )
{
 	int x=0,y=0,w=0,h=0;
	Screen_p s;
	GETDATA;

	s = GetActiveScreen ();

	if (s) {
		switch (data->size) {
		case 0: w=s->sc_Width/4; break;
		case 1: w=s->sc_Width/3; break;
		default: w=s->sc_Width/2; break;
		}
		h = (3*w)/4;
		w+= s->sc_WBorLeft + s->sc_WBorRight;
		h+= 2*s->sc_WBorBottom;

		switch (data->position%3) {
		case 0: x=0; break;
		case 1: x=(s->sc_Width-w)/2; break;
		case 2: x=s->sc_Width-w; break;
		}

		switch ((data->position%9)/3) {
		case 0: y=0; break;
		case 1: y=(s->sc_Height-h)/2; break;
		case 2: y=s->sc_Height-h; break;			
		}

		SetAttrs(obj,
				 MUIA_Window_LeftEdge, x,
				 MUIA_Window_TopEdge , y,
				 MUIA_Window_Width   , w,
				 MUIA_Window_Height  , h,
				 TAG_DONE);
		
		return 1;
	}
	return 0;
}

DECCONST
{
	struct Data *data;
	Object *pip;
	
	obj = DoSuperNew( cl, obj,
		    MUIA_Window_ActivationPriority, 0,
 			MUIA_Window_Activate, FALSE,
 			MUIA_Window_NoMenus , TRUE,
 			MUIA_Window_DepthGadget, FALSE,
 			MUIA_Window_CloseGadget, FALSE,
 			MUIA_Window_DragBar    , FALSE,
 			MUIA_Window_SizeGadget , FALSE,
#ifndef PIP_MOUSEMOVEABLE
		    MUIA_Window_NeverActive, TRUE,		  
 			MUIA_Window_MouseEnabled, FALSE,
#endif					  
 			WindowContents, VGroup,
 				MUIA_InnerLeft  , 0,
 				MUIA_InnerRight , 0,
 				MUIA_InnerTop   , 0,
 				MUIA_InnerBottom, 0,
					  Child, pip = PipObject,
#ifdef PIP_MOUSEMOVEABLE												 
												 MUIA_Pip_Movable,TRUE,
#endif
End,
 				End,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->pip = pip;
	data->size=1;
	data->position=8;
	doset( data, obj, msg->ops_AttrList );
	DoMethod (obj, MM_Pipwindow_CalculatePosition);
	return( (ULONG)obj );
}

DECSET
{
	GETDATA;

	if( doset( data, obj, msg->ops_AttrList ) )
		DoMethod (obj, MM_Pipwindow_CalculatePosition);

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		STOREATTR(MA_Pipwindow_Position, data->position);
		STOREATTR(MA_Pipwindow_Size, data->size);		
	}

	return( DOSUPER );	
}

BEGINMTABLE
DEFCONST
DEFGET
DEFSET
DEFTMETHOD( Pipwindow_CalculatePosition )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_pipwindowclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );
	
	return( TRUE );
}

void delete_pipwindowclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getpipwindowclass( void )
{
	return( mcc->mcc_Class );
}

#endif
