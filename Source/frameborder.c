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
** $Id: frameborder.c,v 1.19 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/graphics.h>
#endif

/* private */
#include "classes.h"
#include "frameset.h"
#include "mui_func.h"

struct Data {
	ULONG y;
	UWORD x;
	UWORD xs, ys;
	int orient, threed;
	int drag, olddrag;
	int minp, maxp;
	int irow, icol;
	char *url;
	int iter;
	struct MUI_EventHandlerNode ehnode;
};

DECNEW
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		InnerSpacing( 0, 0 ),
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	data->x = GetTagData( MA_DNode_X, 0, msg->ops_AttrList );
	data->xs = GetTagData( MA_DNode_XS, 0, msg->ops_AttrList );
	data->y = GetTagData( MA_DNode_Y, 0, msg->ops_AttrList );
	data->ys = GetTagData( MA_DNode_YS, 0, msg->ops_AttrList );
	data->orient = GetTagData( MA_Frameborder_Orient, 0, msg->ops_AttrList );
	data->threed = GetTagData( MA_Frameborder_3D, 0, msg->ops_AttrList );
	data->minp = GetTagData( MA_Frameborder_Min, 0, msg->ops_AttrList );
	data->maxp = GetTagData( MA_Frameborder_Max, 0, msg->ops_AttrList );
	data->irow = GetTagData( MA_Frameborder_IRow, 0, msg->ops_AttrList );
	data->icol = GetTagData( MA_Frameborder_ICol, 0, msg->ops_AttrList );
	data->iter = GetTagData( MA_Frameborder_Iter, 0, msg->ops_AttrList );
	data->url = (char*)GetTagData( MA_Frameborder_URL, 0, msg->ops_AttrList );

	return( (ULONG) obj );
}

DECMMETHOD( Draw )
{
	GETDATA;
	struct RastPort *rp;

	DOSUPER;

	rp = _rp( obj );

	if( msg->flags & MADF_DRAWUPDATE )
	{
#ifndef MBX
		SetABPenDrMd( rp, _pens( obj )[ MPEN_SHADOW ], 0, JAM1 | COMPLEMENT );
#endif //TOFIX!!

		if( data->orient )
		{
			if( data->olddrag >= 0 )
			{
				RectFill( rp, data->olddrag, _top( obj ), data->olddrag + _width( obj ) - 1, _bottom( obj ) );
			}
			if( data->drag >= 0 )
			{
				RectFill( rp, data->drag, _top( obj ), data->drag + _width( obj ) - 1, _bottom( obj ) );
			}
		}
		else
		{
			if( data->olddrag >= 0 )
			{
				RectFill( rp, _left( obj ), data->olddrag, _right( obj ), data->olddrag + _height( obj ) - 1 );
			}
			if( data->drag >= 0 )
			{
				RectFill( rp, _left( obj ), data->drag, _right( obj ), data->drag + _height( obj ) - 1 );
			}
		}
		data->olddrag = data->drag;

		SetDrMd( rp, JAM1 );
	}

	if( msg->flags & MADF_DRAWOBJECT )
	{
		SetAPen( rp, _pens( obj )[ MPEN_BACKGROUND ] );
		RectFill( rp, _left( obj ), _top( obj ), _right( obj ), _bottom( obj ) );
		if( !data->orient )
		{
			if( data->threed )
			{
				SetAPen( rp, _pens( obj )[ MPEN_SHINE ] );
				RectFill( rp, _left( obj ), _top( obj ) + 1 , _right( obj ), _top( obj ) + 1 );
				SetAPen( rp, _pens( obj )[ MPEN_SHADOW ] );
				RectFill( rp, _left( obj ), _bottom( obj ) - 1, _right( obj ), _bottom( obj ) - 1 );
			}
		}
		else
		{
			SetAPen( rp, _pens( obj )[ MPEN_BACKGROUND ] );
			RectFill( rp, _left( obj ), _top( obj ), _right( obj ), _bottom( obj ) );
			if( data->threed )
			{
				SetAPen( rp, _pens( obj )[ MPEN_SHINE ] );
				RectFill( rp, _left( obj ) + 1, _top( obj ), _left( obj ) + 1, _bottom( obj ) );
				SetAPen( rp, _pens( obj )[ MPEN_SHADOW ] );
				RectFill( rp, _right( obj ) - 1, _top( obj ), _right( obj ) - 1, _bottom( obj ) );
			}
		}
	}

	return( 0 );
}

DECMMETHOD( AskMinMax )
{
	DOSUPER;

	msg->MinMaxInfo->MaxWidth += MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight += MUI_MAXMAX;

	return( 0 );
}

DECMMETHOD( Setup )
{
	GETDATA;
	data->ehnode.ehn_Object = obj;
	data->ehnode.ehn_Class = cl;
	data->ehnode.ehn_Events = IDCMP_MOUSEBUTTONS;
	data->ehnode.ehn_Flags = MUI_EHF_GUIMODE;
	DoMethod( _win( obj ), MUIM_Window_AddEventHandler, ( ULONG )&data->ehnode );
	return( DOSUPER );
}

DECMMETHOD( Cleanup )
{
	GETDATA;

	DoMethod( _win( obj ), MUIM_Window_RemEventHandler, ( ULONG )&data->ehnode );
	return( DOSUPER );
}

#define _isinobject(x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_bottom(obj)))
#define _between(a,x,b) ((x)>=(a) && (x)<=(b))

static void myinputloop( struct Window *w, APTR obj, struct Data *data )
{
	struct IntuiMessage *msg;
	int Done = FALSE;

	while( !Done )
	{
		while( !( msg = (struct IntuiMessage*)GetMsg( w->UserPort ) ) )
			WaitPort( w->UserPort );

		if( msg->Class == IDCMP_MOUSEBUTTONS )
		{
			if( msg->Code == SELECTUP )
			{
				Done = TRUE;
			}
		}
		else if( msg->Class == IDCMP_MOUSEMOVE )
		{
			if( data->orient )
				data->drag = msg->MouseX;
			else
				data->drag = msg->MouseY;

			data->drag = min( data->drag, data->maxp );
			data->drag = max( data->drag, data->minp );

			MUI_Redraw( obj, MADF_DRAWUPDATE );
		}
		ReplyMsg( ( struct Message * )msg );
	}

	// TOFIX!
	//fset_setaweight( data->url, data->iter, data->irow, data->icol, data->orient, data->orient ? data->drag - _left( obj ) : data->drag - _top( obj ) );

	data->drag = -1;
	MUI_Redraw( obj, MADF_DRAWUPDATE | MADF_DRAWOBJECT );

	DoMethod( app, MUIM_Application_PushMethod, ( ULONG )_parent( obj ), 3, MM_MyV_SetDoc, -1, -1, FALSE );
	//DoMethod( _parent( obj ), MUIM_Group_ExitChange );
}

DECMMETHOD( HandleEvent )
{
	if( msg->imsg )
	{
		GETDATA;
		if( msg->imsg->Class == IDCMP_MOUSEBUTTONS )
		{
			if( msg->imsg->Code == SELECTDOWN )
			{
				if( _isinobject( msg->imsg->MouseX, msg->imsg->MouseY ) )
				{   
					data->olddrag = -1;

					if( data->orient )
						data->drag = _left( obj );
					else
						data->drag = _top( obj );

					MUI_Redraw( obj, MADF_DRAWUPDATE );

#ifndef MBX
					if ( MUIMasterBase->lib_Version < 20 )
						DoMethod( _win( obj ), MUIM_Window_RemEventHandler, &data->ehnode );
#endif /* MBX */
					data->ehnode.ehn_Events |= IDCMP_MOUSEMOVE;

#ifndef MBX
					if ( MUIMasterBase->lib_Version < 20 )
						DoMethod( _win( obj ), MUIM_Window_AddEventHandler, &data->ehnode );
#endif /* MBX */

					myinputloop( (APTR)getv( _win( obj ), MUIA_Window_Window ), obj, data );

#ifndef MBX
					if ( MUIMasterBase->lib_Version < 20 ) 
						DoMethod( _win( obj ), MUIM_Window_RemEventHandler, &data->ehnode );
#endif /* MBX */
					
					data->ehnode.ehn_Events &= ~IDCMP_MOUSEMOVE;

#ifndef MBX
					if ( MUIMasterBase->lib_Version < 20 ) 
						DoMethod( _win( obj ), MUIM_Window_AddEventHandler, &data->ehnode );
#endif /* MBX */
				}
			}
		}
	}
	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMMETHOD( Draw )
DEFMMETHOD( HandleEvent )
DEFMMETHOD( Setup )
DEFMMETHOD( Cleanup )
DEFMMETHOD( AskMinMax )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_frameborderclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Area, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "FrameBorderClass";
#endif

	return( TRUE );
}

void delete_frameborderclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getframeborder( void )
{
	return( mcc->mcc_Class );
}
