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
** $Id: js_screen.c,v 1.17 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/graphics.h>
#if USE_CGX
#include <proto/cybergraphics.h>
#endif
#endif

/* private */
#include "classes.h"
#if USE_CGX
#ifndef __SASC
#include <cybergraphx/cybergraphics.h>
#endif /* !__SASC */
#endif
#include "js.h"
#include "mui_func.h"

struct Data {
	int dummy;
};

BEGINPTABLE
DPROP( height, 		real )
DPROP( width,		real )
DPROP( availHeight, real )
DPROP( availWidth,  real )
DPROP( pixelDepth,  real )
DPROP( colorDepth,  real )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Screen",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetProperty )
{
	extern struct Screen *destscreen;
	struct Screen *scr = destscreen;

#ifndef MBX
	// HACK!
	if( !scr )
		scr = (APTR)OpenWorkBench();
#else
	if( !scr )
		scr = InspirationBase->inspid_ActiveScreen;
#endif

	
	switch( findpropid( ptable, msg->propname ) )
	{
		case JSPID_height:
		case JSPID_availHeight:
			storerealprop( msg, (double)scr->sc_Height );
			return( TRUE );
		case JSPID_width:
		case JSPID_availWidth:
			storerealprop( msg, (double)scr->sc_Width );
			return( TRUE );

		case JSPID_pixelDepth:
		case JSPID_colorDepth:
#if USE_CGX
			if( CyberGfxBase )
			{
				if( GetCyberMapAttr( scr->RastPort.BitMap, CYBRMATTR_ISCYBERGFX ) )
				{
					storerealprop( msg, (double)GetCyberMapAttr( scr->RastPort.BitMap, CYBRMATTR_DEPTH ) );
					return( TRUE );
				}
			}
#endif
#ifdef MBX
			storerealprop( msg, (double)GetBitMapAttr( scr->sc_RastPort->rp_BitMap, BMA_DEPTH ) );
#else
			storerealprop( msg, (double)GetBitMapAttr( scr->sc_RastPort.rp_BitMap, BMA_DEPTH ) );
#endif
			return( TRUE );
	}

	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_screen( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_ScreenClass";
#endif

	return( TRUE );
}

void delete_js_screen( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_screen( void )
{
	return( mcc->mcc_Class );
}
