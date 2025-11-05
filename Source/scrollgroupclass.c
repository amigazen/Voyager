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
 * Scrollgroup custom class
 * ------------------------
 * - With automatic scrollbar addition/removal
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: scrollgroupclass.c,v 1.3 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#endif

/* private */
#include "classes.h"
#include "mui_func.h"


struct Data {
	APTR hbar;
	APTR vbar;
	ULONG has_hbar;
	ULONG has_vbar;
	ULONG can_hbar;
	ULONG can_vbar;
	APTR grp;
	APTR move;
	ULONG usewinborder;
};


DECNEW
{
	struct Data *data;
	ULONG grp, hbar, vbar, move = NULL;
	ULONG usewinborder = GetTagData( MUIA_Scrollgroup_UseWinBorder, FALSE, msg->ops_AttrList );
	ULONG can_hbar = FALSE;
	ULONG can_vbar = FALSE;

	if (!(grp = (APTR)GetTagData(MUIA_Scrollgroup_Contents,NULL, msg->ops_AttrList)) )
		return(NULL);

	hbar = GetTagData( MUIA_Scrollgroup_FreeHoriz, TRUE, msg->ops_AttrList );
	vbar = GetTagData( MUIA_Scrollgroup_FreeVert, TRUE, msg->ops_AttrList );

	if ( usewinborder )
	{
		if ( hbar )
		{
			hbar = ScrollbarObject, MUIA_Group_Horiz, TRUE , MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_Bottom, TAG_DONE);
		}

		if ( vbar )
		{
			vbar = ScrollbarObject, MUIA_Group_Horiz, FALSE, MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_Right , TAG_DONE);
		}
	}
	else
	{
		dprintf( "not using win border\n" );
		if ( hbar )
		{
			can_hbar = TRUE;
			
			if ( !( hbar = ScrollbarObject, MUIA_Group_Horiz, TRUE, End ) )
			{
				return( 0 );
			}
		}

		if ( vbar )
		{
			can_vbar = TRUE;

			if ( !( vbar = ScrollbarObject, MUIA_Group_Horiz, FALSE, End ) )
			{
				if ( hbar )
				{
					MUI_DisposeObject( hbar );
				}
				return ( 0 );
			}
		}

		if ( hbar && vbar )
		{
			move = RectangleObject,
				ButtonFrame,
				MUIA_Background, MUII_ButtonBack,
			End;

			if ( !move )
			{
				if ( hbar )
				{
					MUI_DisposeObject( hbar );
				}

				if ( vbar )
				{
					MUI_DisposeObject( vbar );
				}

				return ( 0 );
			}
		}
	}

	obj = ( Object * )DoSuperNew( cl, obj,
		MUIA_FillArea, FALSE,
		MUIA_Group_Columns, 2, /* XXX: hm.. why ? scrollbar ? oh cool.. it seems to work everytime ! */
		MUIA_Group_Spacing, 0,
		Child, grp,
		( usewinborder && hbar ) ? Child : TAG_IGNORE, hbar,
		( usewinborder && vbar ) ? Child : TAG_IGNORE, vbar,
		TAG_MORE, msg->ops_AttrList );

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->usewinborder = usewinborder;
	data->hbar = hbar;
	data->vbar = vbar;
	data->move = move;
	data->grp = grp;
	data->can_hbar = can_hbar;
	data->can_vbar = can_vbar;

	#if 1
	if ( !data->usewinborder )
	{
		/* XXX: hack begin (it works :) */
		#if 0
		hbar = ScrollbarObject, MUIA_Group_Horiz, TRUE , MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_Bottom, TAG_DONE);
		vbar = ScrollbarObject, MUIA_Group_Horiz, FALSE, MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_Right , TAG_DONE);
		data->move = RectangleObject,
			ButtonFrame,
			MUIA_Background, MUII_ButtonBack,
			End;

		data->hbar = hbar;
		data->vbar = vbar;
		
		DoMethod( obj, OM_ADDMEMBER, vbar );
		DoMethod( obj, OM_ADDMEMBER, hbar );
		DoMethod( obj, OM_ADDMEMBER, data->move );
		/* XXX: hack end */
		#endif

		#if 0
		if ( data->hbar && data->vbar ) // hack! XXX
		{
			dprintf( "creating stupid move stuff\n" );
			if ( data->move = RectangleObject, /* XXX: dynamic.. */
				ButtonFrame,
				MUIA_Background, MUII_ButtonBack,
				End
			)
			{
				DoMethod( obj, OM_ADDMEMBER, data->move );
			}
		}
		#endif
	}
	#endif

	if ( data->usewinborder )
	{
		if ( data->hbar ) /* XXX: dynamic.. */
		{
			set( data->hbar, MUIA_Prop_DeltaFactor, 8 );
			DoMethod( data->hbar, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime, data->grp, 3, MUIM_Set, MUIA_Virtgroup_Left, MUIV_TriggerValue );
			DoMethod( data->grp, MUIM_Notify, MUIA_Virtgroup_Left, MUIV_EveryTime, data->hbar, 3, MUIM_Set, MUIA_Prop_First, MUIV_TriggerValue );
			/* XXX: maybe enable set( data->hbar, MUIA_Prop_DoSmooth, TRUE ); */
		}
		else
		{
			set( data->grp, MUIA_Virtgroup_FreeHoriz, FALSE );
		}

		if ( data->vbar )
		{
			set( data->vbar, MUIA_Prop_DeltaFactor, 8 );
			DoMethod( data->vbar, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime, data->grp, 3, MUIM_Set, MUIA_Virtgroup_Top, MUIV_TriggerValue );
			DoMethod( data->grp, MUIM_Notify, MUIA_Virtgroup_Top, MUIV_EveryTime, data->vbar, 3, MUIM_Set, MUIA_Prop_First, MUIV_TriggerValue );
			/* XXX: maybe enable set( data->vbar, MUIA_Prop_DoSmooth, TRUE ); */
		}
		else
		{
			set( data->grp, MUIA_Virtgroup_FreeVert, FALSE );
		}
	}

	/* XXX: in fact.. all that scrollbar stuff should be dynamic */

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	/*
	 * Those objects are lying around
	 * unattached.
	 */
	#if 0 /* XXX: doesn't work yet I think */
	if ( !data->has_hbar && data->hbar )
	{
		MUI_DisposeObject( data->hbar );
	}

	if ( !data->has_vbar && data->vbar )
	{
		MUI_DisposeObject( data->vbar );
	}
	#endif

	/* XXX: add some hack to remove the move thing */

	return( DOSUPER );
}


DECSMETHOD( Scrollgroup_CheckScrollers )
{
	GETDATA;

	dprintf( "checkscroller: vscroll: %lu, hscroll: %lu, usewinborder: %lu, can_vbar: %lu, can_hbar: %lu\n", msg->vscroll, msg->hscroll, data->usewinborder, data->can_vbar, data->can_hbar );

	if ( !data->usewinborder )
	{
		DoMethod( obj, MUIM_Group_InitChange );

		if ( msg->hscroll && msg->vscroll ) /* XXX: not sure that works reliably.. if not we have to add empty objects.. sigh */
		{
			set( obj, MUIA_Group_Columns, 2 ); /* XXX: confirmed.. seems it doesn't work at all */
		}
		else
		{
			set( obj, MUIA_Group_Columns, 1 );
		}
		
		/* vbar */
		if ( msg->vscroll )
		{
			if ( !data->has_vbar && data->can_vbar )
			{
				dprintf( "adding vbar\n" );
				DoMethod( obj, OM_ADDMEMBER, data->vbar );

				data->has_vbar = TRUE;
			}
		}
		else
		{
			if ( data->has_vbar )
			{
				dprintf( "removing vbar\n" );
				DoMethod( obj, OM_REMMEMBER, data->vbar );

				data->has_vbar = FALSE;
			}
		}

		/* hbar */
		if ( msg->hscroll )
		{
			if ( !data->has_hbar && data->can_hbar )
			{
				dprintf( "adding hbar\n" );
				DoMethod( obj, OM_ADDMEMBER, data->hbar );

				data->has_hbar = TRUE;
			}
		}
		else
		{
			if ( data->has_hbar )
			{
				dprintf( "removing hbar\n" );
				DoMethod( obj, OM_REMMEMBER, data->hbar );

				data->has_hbar = FALSE;
			}
		}

		if ( msg->hscroll && msg->vscroll )
		{
			if ( data->has_hbar && data->has_vbar )
			{
				dprintf( "adding cube\n" );
				DoMethod( obj, OM_ADDMEMBER, data->move );
			}
		}
		else
		{
			if ( data->has_hbar && data->has_vbar )
			{
				dprintf( "removing cube\n" );
				DoMethod( obj, OM_REMMEMBER, data->move );
			}
		}

		DoMethod( obj, MUIM_Group_ExitChange );
	}
	return ( 0 );
}


DECMMETHOD( AdjustLayout )
{
	dprintf( "got adjustlayout!\n" );
	return ( DOSUPER );
}


DECMMETHOD( Scrollgroup_Inform )
{
	GETDATA;

	if (data->hbar)
	{
		SetAttrs(data->hbar,
			MUIA_NoNotify    , TRUE,
			MUIA_Prop_First  , msg->xpos,
			MUIA_Prop_Entries, msg->width,
			MUIA_Prop_Visible, msg->viswidth,
			TAG_DONE);
	}

	if (data->vbar)
	{
		SetAttrs(data->vbar,
			MUIA_NoNotify    , TRUE,
			MUIA_Prop_First  , msg->ypos,
			MUIA_Prop_Entries, msg->height,
			MUIA_Prop_Visible, msg->visheight,
			TAG_DONE);
	}
	return ( TRUE );
}


DECMMETHOD( HandleEvent )
{
	/* XXX */
	return ( 0 );
}


DECGET
{
	GETDATA;

	switch (((struct opGet *)msg)->opg_AttrID)
	{
		case MUIA_Scrollgroup_HorizBar: *(((struct opGet *)msg)->opg_Storage) = (ULONG)data->hbar; return(TRUE);
		case MUIA_Scrollgroup_VertBar : *(((struct opGet *)msg)->opg_Storage) = (ULONG)data->vbar; return(TRUE);
		case MUIA_Scrollgroup_Contents: *(((struct opGet *)msg)->opg_Storage) = (ULONG)data->grp; return(TRUE);
	}

	return ( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFGET
DEFDISPOSE
DEFSMETHOD( Scrollgroup_CheckScrollers )
DEFMMETHOD( AdjustLayout )
DEFMMETHOD( Scrollgroup_Inform )
DEFMMETHOD( HandleEvent )
ENDMTABLE


static struct MUI_CustomClass *mcc;

int create_scrollgroupclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "ScrollgroupClass";
#endif

	return( TRUE );
}

void delete_scrollgroupclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getscrollgroupclass( void )
{
	return( mcc->mcc_Class );
}

