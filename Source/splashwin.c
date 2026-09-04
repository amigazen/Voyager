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
 * Splash window class
 * -------------------
 * - Displays a window with some pictures to make the user happy while
 *   he waits for V to load
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: splashwin.c,v 1.19 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"
#include "classes.h"
#include "voyager_cat.h"
#include "copyright.h"
#include "mui_func.h"

#include "naglogo.h"

#if GREX_RELEASE
#include "grex.h"
#endif /* GREX_RELEASE */

#if USE_SPLASHWIN

APTR splashwin;
int use_splashwin = 1;

#define MAX_GAUGE_STAGE 7 /* number of maximum gauge stages */
/*
 * 1 - cookies
 * 2 - auth
 * 3 - image decoders
 * 4 - history
 * 5 - cache
 * 6 - plugins
 * 7 - initializing
 */


/* instance data */
struct Data
{
	APTR gauge;
	ULONG gauge_value;
};


DECCONST
{
	struct Data *data;
	APTR gauge;

	obj = ( Object * )DoSuperNew( cl, obj,
		MUIA_Window_NoMenus, TRUE,
		MUIA_Window_CloseGadget, FALSE,
		MUIA_Window_DepthGadget, FALSE,
		MUIA_Window_DragBar, FALSE,
		MUIA_Window_SizeGadget, FALSE,
		MUIA_Window_Activate, FALSE,
		MUIA_Window_RootObject, HGroup,

			Child, VGroup,
				Child, VSpace( 0 ),
					Child, BodychunkObject,
						GroupFrame,
						MUIA_Weight, 0,
						InnerSpacing( 0, 0 ),
						MUIA_Bodychunk_Body, reglogo_body,
						MUIA_Bodychunk_Compression, REGLOGO_COMPRESSION,
						MUIA_Bodychunk_Masking, REGLOGO_MASKING,
						MUIA_Bodychunk_Depth, REGLOGO_DEPTH,
						MUIA_Bitmap_Height, REGLOGO_HEIGHT, MUIA_Bitmap_Width, REGLOGO_WIDTH,
						MUIA_FixWidth, REGLOGO_WIDTH, MUIA_FixHeight, REGLOGO_HEIGHT,
						MUIA_Bitmap_SourceColors, reglogo_colors,
						MUIA_Bitmap_Precision, PRECISION_EXACT,
					End,
				Child, VSpace( 0 ),
			End,
			
			Child, MUI_MakeObject( MUIO_VBar, 0 ),

			Child, VGroup,
				
				Child, VSpace( 0 ),
				
				Child, TextObject,
					MUIA_Text_Contents, "\33c\33b" APPNAME " " VERSIONSTRING,
				End,
			
				Child, TextObject,
					MUIA_Text_Contents, "\33c" REVDATE,
					MUIA_Font, MUIV_Font_Tiny,
				End,

#if GREX_RELEASE
				Child, TextObject,
					MUIA_Text_Contents, "\33c\33bSpecial GRex CD edition",
				End,

#endif /* GREX_RELEASE */

				Child, TextObject,
					MUIA_Text_Contents, "\33c© VaporWare 1995-2002",
				End,

				Child, VSpace( 0 ),

				Child, gauge = GaugeObject,
					GaugeFrame,
					MUIA_Gauge_Horiz, TRUE,
					MUIA_Gauge_Max, MAX_GAUGE_STAGE,
					MUIA_Gauge_InfoText, GS( SPLASHWIN_INIT ),
				End,
			
				Child, VSpace( 0 ),
			
			End,

#if GREX_RELEASE

			Child, MUI_MakeObject( MUIO_VBar, 0 ),

			Child, VGroup,
				Child, VSpace( 0 ),
					Child, BodychunkObject,
						GroupFrame,
						MUIA_Weight, 0,
						InnerSpacing( 0, 0 ),
						MUIA_Bodychunk_Body, grex5_body,
						MUIA_Bodychunk_Compression, GREX5_COMPRESSION,
						MUIA_Bodychunk_Masking, GREX5_MASKING,
						MUIA_Bodychunk_Depth, GREX5_DEPTH,
						MUIA_Bitmap_Height, GREX5_HEIGHT, MUIA_Bitmap_Width, GREX5_WIDTH,
						MUIA_FixWidth, GREX5_WIDTH, MUIA_FixHeight, GREX5_HEIGHT,
						MUIA_Bitmap_SourceColors, grex5_colors,
						MUIA_Bitmap_Precision, PRECISION_EXACT,
					End,
				Child, VSpace( 0 ),
			End,

#endif /* GREX_RELEASE */

		End,
	End;

	if ( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	data->gauge = gauge;

	return( ( ULONG )obj );
}


/*
 * Sets a new text in the GaugeObject and increments it
 * one step.
 */
DECSMETHOD( SplashWin_Update )
{
	GETDATA;

	data->gauge_value++;

	set( data->gauge, MUIA_Gauge_InfoText, msg->text );
	set( data->gauge, MUIA_Gauge_Current, data->gauge_value );

	tickapp();

	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFSMETHOD( SplashWin_Update )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_splashwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "SplashWinClass";
#endif

	return( TRUE );
}

void delete_splashwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getsplashwinclass( void )
{
	return( mcc->mcc_Class );
}
#endif /* USE_SPLASHWIN */
