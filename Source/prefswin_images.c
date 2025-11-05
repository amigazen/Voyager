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
 * Images
 * ------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_images.c,v 1.15 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

/* private */
#include "prefswin.h"
#include "mui_func.h"

struct Data {
	APTR chk_dbuffer;
	APTR chk_stop_animgif;
	APTR cyc_jpeg_quant;
	APTR cyc_jpeg_dither;
	APTR cyc_gif_dither;
	APTR cyc_png_dither;
	APTR cyc_jpeg_dct;
	APTR chk_alttext;
	APTR chk_placeholder;
};


DECNEW
{
	struct Data *data;
	APTR cyc_jpeg_quant, cyc_jpeg_dither, cyc_jpeg_dct, chk_dbuffer, chk_stop_animgif, cyc_gif_dither, cyc_png_dither, chk_alttext, chk_placeholder;
	static STRPTR quantopts[ 3 ], ditheropts[ 4 ], dctopts[ 4 ], ditheropts2[ 3 ];

	quantopts[ 0 ] = GS( PREFSWIN_IMG_JPEG_QUANT_SIMPLE );
	quantopts[ 1 ] = GS( PREFSWIN_IMG_JPEG_QUANT_2PASS );

	ditheropts[ 0 ] = GS( PREFSWIN_IMG_JPEG_DITHER_NONE );
	ditheropts[ 1 ] = GS( PREFSWIN_IMG_JPEG_DITHER_ORDERED );
	ditheropts[ 2 ] = GS( PREFSWIN_IMG_JPEG_DITHER_FS );

	ditheropts2[ 0 ] = GS( PREFSWIN_IMG_JPEG_DITHER_NONE );
	ditheropts2[ 1 ] = GS( PREFSWIN_IMG_JPEG_DITHER_FS );

	dctopts[ 0 ] = GS( PREFSWIN_IMG_JPEG_DCT_SLOW );
	dctopts[ 1 ] = GS( PREFSWIN_IMG_JPEG_DCT_FAST );
	dctopts[ 2 ] = GS( PREFSWIN_IMG_JPEG_DCT_FLOAT );

	obj = DoSuperNew( cl, obj,
		Child, VGroup, GroupFrameT( GS( PREFSWIN_IMG_L_GENERAL ) ),

			Child, ColGroup( 4 ),

				Child, HSpace( 0 ),
				Child, VGroup,
					Child, Label( GS( PREFSWIN_IMG_OPT_DOUBLEBUFFER ) ),
					Child, TextObject,
						MUIA_Text_Contents, GS( PREFSWIN_IMG_OPT_DOUBLEBUFFERWARN ),
						MUIA_Font, MUIV_Font_Tiny,
					End,
				End,
				Child, VGroup,
					Child, VSpace( 0 ),
					Child, chk_dbuffer = pcheck( DSI_IMG_DOUBLEBUFFER, GS(PREFSWIN_IMG_OPT_DOUBLEBUFFER) ),
					Child, VSpace( 0 ),
				End,
				Child, HSpace( 0 ),

				Child, HSpace( 0 ),    
				Child, Label( GS( PREFSWIN_IMG_OPT_ALT_TEXT ) ),
				Child, VGroup,
					Child, VSpace( 0 ),
					Child, chk_alttext = pchecki( DSI_FLAGS + VFLG_IMG_SHOW_ALT_TEXT , GS(PREFSWIN_IMG_OPT_ALT_TEXT) ),
					Child, VSpace( 0 ),
				End,
				Child, HSpace( 0 ),

				Child, HSpace( 0 ),    
				Child, Label( GS( PREFSWIN_IMG_OPT_PLACEHOLDER ) ),
				Child, VGroup,
					Child, VSpace( 0 ),
					Child, chk_placeholder = pchecki( DSI_FLAGS + VFLG_IMG_SHOW_PH_BORDER, GS(PREFSWIN_IMG_OPT_PLACEHOLDER) ),
					Child, VSpace( 0 ),
				End,
				Child, HSpace( 0 ),

				Child, HSpace( 0 ),    
				Child, Label( GS( PREFSWIN_IMG_STOP_ANIMGIF ) ),
				Child, VGroup,
					Child, VSpace( 0 ),
					Child, chk_stop_animgif = pcheck( DSI_IMG_STOP_ANIMGIF, GS(PREFSWIN_IMG_STOP_ANIMGIF) ),
					Child, VSpace( 0 ),
				End,
				Child, HSpace( 0 ),
			End,

			Child, VGroup,
				Child, CLabel2( GS( PREFSWIN_IMG_JPEG_L_DITHER ) ),
				Child, HGroup,
					Child, Label2( ( STRPTR )"JPEG:" ),
					Child, cyc_jpeg_dither = pcycle( ditheropts, DSI_IMG_JPEG_DITHER, "JPEG:" ),
					Child, Label2( ( STRPTR )"GIF:" ),
					Child, cyc_gif_dither = pcycle( ditheropts2, DSI_IMG_GIF_DITHER, "GIF:" ),
					Child, Label2( ( STRPTR )"PNG:" ),
					Child, cyc_png_dither = pcycle( ditheropts2, DSI_IMG_PNG_DITHER, "PNG:" ),
				End,

				Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_IMG_L_NOTRUECOLOR ), MUIA_Font, MUIV_Font_Tiny, End,

			End,
		End,

		Child, ColGroup( 2 ), GroupFrameT( GS( PREFSWIN_IMG_L_JPEG ) ),

			Child, Label2( GS( PREFSWIN_IMG_JPEG_L_QUANT ) ),
			Child, cyc_jpeg_quant = pcycle( quantopts, DSI_IMG_JPEG_QUANT, GS( PREFSWIN_IMG_JPEG_L_QUANT ) ),

			Child, HSpace( 0 ),
			Child, TextObject, MUIA_Text_Contents, GS( PREFSWIN_IMG_L_NOTRUECOLOR ), MUIA_Font, MUIV_Font_Tiny, End,

			Child, Label2( GS( PREFSWIN_IMG_JPEG_L_DCT ) ),
			Child, cyc_jpeg_dct = pcycle( dctopts, DSI_IMG_JPEG_DCT,  GS( PREFSWIN_IMG_JPEG_L_DCT ) ),

		End,

	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->chk_dbuffer = chk_dbuffer;
	data->chk_stop_animgif = chk_stop_animgif;
	data->cyc_jpeg_quant = cyc_jpeg_quant;
	data->cyc_jpeg_dither = cyc_jpeg_dither;
	data->cyc_gif_dither = cyc_gif_dither;
	data->cyc_png_dither = cyc_png_dither;
	data->cyc_jpeg_dct = cyc_jpeg_dct;
	data->chk_alttext = chk_alttext;
	data->chk_placeholder = chk_placeholder;

	/* Help */
	set( chk_dbuffer, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_DBUFFER ) );
	set( cyc_jpeg_dither, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_JPEG_DITHER ) );
	set( cyc_gif_dither, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_JPEG_DITHER ) );
	set( cyc_png_dither, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_JPEG_DITHER ) );
	set( cyc_jpeg_quant, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_JPEG_QUANT ) );
	set( cyc_jpeg_dct, MUIA_ShortHelp, GS( SH_PREFSWIN_CYC_JPEG_DCT ) );
	set( chk_stop_animgif, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_STOP_ANIMGIF ) );
	set( chk_alttext, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_SHOW_ALT_TEXT ) );
	set( chk_placeholder, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_SHOW_PH_BORDER ) );

	setupd( cyc_jpeg_dither, MUIA_Cycle_Active );
	setupd( cyc_gif_dither, MUIA_Cycle_Active );
	setupd( cyc_png_dither, MUIA_Cycle_Active );
	setupd( cyc_jpeg_quant, MUIA_Cycle_Active );
	setupd( cyc_jpeg_dct, MUIA_Cycle_Active );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	storeattr( data->chk_dbuffer, MUIA_Selected, DSI_IMG_DOUBLEBUFFER );
	storeattr( data->chk_stop_animgif, MUIA_Selected, DSI_IMG_STOP_ANIMGIF );
	gp_image_stop_animgif = getv( data->chk_stop_animgif, MUIA_Selected );
	storeattr( data->cyc_jpeg_quant, MUIA_Cycle_Active, DSI_IMG_JPEG_QUANT );
	storeattr( data->cyc_jpeg_dither, MUIA_Cycle_Active, DSI_IMG_JPEG_DITHER );
	storeattr( data->cyc_gif_dither, MUIA_Cycle_Active, DSI_IMG_GIF_DITHER );
	storeattr( data->cyc_png_dither, MUIA_Cycle_Active, DSI_IMG_PNG_DITHER );
	storeattr( data->cyc_jpeg_dct, MUIA_Cycle_Active, DSI_IMG_JPEG_DCT );
	storeattr( data->chk_alttext, MUIA_Selected, DSI_FLAGS + VFLG_IMG_SHOW_ALT_TEXT );
	gp_image_show_alt_text = getv( data->chk_alttext, MUIA_Selected );
	storeattr( data->chk_placeholder, MUIA_Selected, DSI_FLAGS + VFLG_IMG_SHOW_PH_BORDER );
	gp_image_show_ph_border = getv( data->chk_placeholder, MUIA_Selected );

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_imagesclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_ImagesClass";
#endif

	return( TRUE );
}

void delete_prefswin_imagesclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_imagesclass( void )
{
	return( mcc->mcc_Class );
}

