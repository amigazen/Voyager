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
** $Id: amicon.c,v 1.35 2003/07/06 16:51:32 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/gadtools.h>
#include <datatypes/pictureclass.h>

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#endif

/* private */
#include "classes.h"
#include "htmlclasses.h"
#include "prefs.h"
#include "mui_func.h"


struct Data {
	int frame;
	int rotating;
	int active;
	struct MUI_InputHandlerNode ihn;
	APTR ctm;
};

static APTR dto;
static int framexsize = 42, frameysize = 36;
static int frameticks = 100;
static struct BitMap *bm;
static int framenum;
static int bmusecount;

#if USE_MENUS
DECNEW
{
	struct Data *data;
	static struct NewMenu nmcontext[] = {
		{ NM_TITLE, "Anim", 0, 0, 0, 0 },
		{ NM_ITEM, "Hide", 0, 0, 0, (APTR)1 },
		NM_END
	};

	obj = (APTR)DOSUPER;
	if( obj )
	{
		data = INST_DATA( cl, obj );
		data->ctm = ( APTR )MUI_MakeObject( MUIO_MenustripNM, ( ULONG )nmcontext, 0 );
		set( obj, MUIA_ContextMenu, data->ctm );
	}
	return( (ULONG)obj );
}
#endif /* USE_MENUS */

#if USE_MENUS
DECDISPOSE
{
	GETDATA;
	if( data->ctm )
		MUI_DisposeObject( data->ctm );
	return( DOSUPER );
}
#endif /* USE_MENUS */

static void setupbitmap( char *filename, struct Screen *scr )
{
#if USE_DOS
	struct BitMapHeader *bmh;
	struct DiskObject *diskobj;

	// First, create an DTO Object
	dto = NewDTObject( filename,
		DTA_GroupID, GID_PICTURE,
		DTA_SourceType, DTST_FILE,
		PDTA_FreeSourceBitMap, TRUE,
		OBP_Precision, PRECISION_IMAGE,
		PDTA_DestMode, PMODE_V43,
		PDTA_UseFriendBitMap, TRUE,
		PDTA_Screen, scr,
		TAG_DONE
	);

	if( !dto )
		return;

	DoMethod( dto, DTM_PROCLAYOUT, NULL, TRUE );

	GetDTAttrs( dto, 
		PDTA_DestBitMap, &bm, 
		PDTA_BitMapHeader, &bmh, 
		TAG_DONE 
	);

	if( !bm || !bmh )
	{
		DisposeDTObject( dto );
		dto = NULL;
		return;
	}

	frameysize = bmh->bmh_Height;
	framexsize = bmh->bmh_Width / 16;

	diskobj = GetDiskObjectNew( filename );
	if( diskobj )
	{
		char *tt;
		tt = FindToolType( diskobj->do_ToolTypes, "FRAMEWIDTH" );
		if( !tt )
			tt = FindToolType( diskobj->do_ToolTypes, "WIDTH" );
		if( tt )
			framexsize = atoi( tt );

		tt = FindToolType( diskobj->do_ToolTypes, "FRAMETICKS" );
		if( tt )
			frameticks = atoi( tt );

		FreeDiskObject( diskobj );
	}

	framenum = bmh->bmh_Width / framexsize;

	if( framenum < 2 )
		framenum = 1;
#else
//TOFIX!! Get the bitmap from elsewhere
	framenum = 1;
#endif /* USE_DOS */

}

static void cleanupbitmap( void )
{
	if( dto )
	{
#if USE_DOS
		DisposeDTObject( dto );
#endif
		dto = NULL;
	}
}

DECSET
{
	struct TagItem *tag, *tagp = msg->ops_AttrList;
	GETDATA;
	
	while( tag = NextTagItem( &tagp ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Amicon_Active:

			if( tag->ti_Data )
			{
				data->rotating = TRUE;
				data->active = TRUE;
			}
			else
			{
				if( data->active )
				{
					data->active = FALSE;
				}
			}
			break;
	}
	return( DOSUPER );
}

DECGET
{
	if( msg->opg_AttrID == MA_DropObject_URL )
	{
		*msg->opg_Storage = (ULONG)"About:";
		return( TRUE );
	}
	else if( msg->opg_AttrID == MA_DropObject_Name )
	{
		*msg->opg_Storage = (ULONG)"About V³";
		return( TRUE );
	}
	return( DOSUPER );
}

DECMMETHOD( Draw )
{
	GETDATA;

	DOSUPER;

	if(msg->flags & ( MADF_DRAWUPDATE | MADF_DRAWOBJECT ) )
	{
		if( bm )
		{
			BltBitMapRastPort( 
				bm, data->frame * framexsize, 0,
				_rp( obj ), _mleft( obj ), _mtop( obj ),
				framexsize, frameysize,
				0xc0
			);
		}
	}

	return( 0 );
}


DECMMETHOD( Setup )
{
	GETDATA;

	if( !DOSUPER )
		return( FALSE );

	if( !bmusecount++ )
		setupbitmap( "PROGDIR:TransferAnim", _screen( obj ) );

	data->ihn.ihn_Object = obj;
	data->ihn.ihn_Flags = MUIIHNF_TIMER;
	data->ihn.ihn_Millis = frameticks;
	data->ihn.ihn_Method = MM_TriggerMe;

	DoMethod( _app( obj ), MUIM_Application_AddInputHandler, ( ULONG )&data->ihn );

	return( TRUE );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	DoMethod( _app( obj ), MUIM_Application_RemInputHandler, ( ULONG )&data->ihn );

	if( !--bmusecount )
		cleanupbitmap();

	return( DOSUPER );
}

DECMMETHOD( AskMinMax )
{
	struct MUI_MinMax *mmx;

	DOSUPER;

	mmx = msg->MinMaxInfo;

	mmx->MinWidth += framexsize;
	mmx->MinHeight += frameysize;

	mmx->DefWidth += framexsize;
	mmx->DefHeight += frameysize;

	mmx->MaxWidth += framexsize;
	mmx->MaxHeight += frameysize;

	return( 0 );
}

DECMETHOD( TriggerMe, APTR )
{
	GETDATA;

	if( framenum < 2 )
		return( 0 );

	if( data->rotating == FALSE )
	{
		return( 0 );
	}

	if( ( !data->active ) )
	{
		data->rotating = FALSE;
		data->frame = 0;
		MUI_Redraw( obj, MADF_DRAWUPDATE );
		return( 0 );
	}


	data->frame = ( data->frame + 1 ) % ( framenum - 1 );
	MUI_Redraw( obj, MADF_DRAWUPDATE );

	return( 0 );
}

#if USE_MENUS
DECMMETHOD( ContextMenuChoice )
{
	ULONG id;

	get( msg->item, MUIA_UserData, &id );
	if( id == 1 )
	{
		setflag( VFLG_HIDE_ICON, TRUE );
		DoMethod( app, MUIM_Application_PushMethod, app, 2, MM_DoAllWins, MM_HTMLWin_SetupIcon );
	}
	return( 0 );
}
#endif

BEGINMTABLE
#if USE_MENUS
DEFNEW
#endif
DEFSET
DEFGET
#if USE_MENUS
DEFDISPOSE
#endif
DEFMMETHOD( Draw )
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( AskMinMax )
#if USE_MENUS
DEFMMETHOD( ContextMenuChoice )
#endif
DEFMETHOD( TriggerMe )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_amiconclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "AmiconClass";
#endif

	return( TRUE );
}

void delete_amiconclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getamiconclass( void )
{
	return( mcc->mcc_Class );
}

