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
 * Smart requester window
 * ----------------------
 * - more or less a MUI_Request() clone.. the design part has been taken from it
 * - non blocking
 * - stackable
 * - sends back one method to the calling object with the number of the pressed button
 * - improves sexual life (not really, I spent the whole night on this instead of going out)
 *
 * $Id: smartreq.c,v 1.6 2003/07/06 16:51:34 olli Exp $
 */

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "classes.h"
#include "mui_func.h"
#include "smartreq.h"


/*
 * Settable constants
 */
#define REQ_BUFFERSIZE 4096 /* same as MUI_Request() but without buffer overflows. Beware.. we use the stack too */

/* TOFIX: MUIA_Window_IsSubWindow, TRUE. and delete it in OM_DISPOSE of the parent object */

struct reqnode {
	struct MinNode n;
	LONG userdata;
	STRPTR buffer;
	STRPTR gadgets;
};

struct Data {
	APTR *objptr;    /* calling party expects that pointer to be non NULL to dispose us */
	APTR parentobj; /* where to send back the replies */
	APTR txt_req;
	APTR grp_buttons;
	ULONG methodid;
	struct MinList reqlist;

};


DECNEW
{
	struct Data *data;
	APTR txt_req, grp_buttons;

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_IsSubWindow, TRUE,
		MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
		MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
		MUIA_Window_Width, MUIV_Window_Width_MinMax( 0 ),
		MUIA_Window_Height, MUIV_Window_Height_MinMax( 0 ),
		MUIA_Window_CloseGadget, FALSE,
		MUIA_Window_Activate, TRUE, /* TOFIX: hm.. beware of that.. */
		MUIA_Window_Remember, FALSE, /* TOFIX: wtf is that for ? ask stuntzi :) */
		MUIA_Window_NoMenus, TRUE, /* TOFIX: yes or no ? */
		WindowContents, VGroup,
			InnerSpacing( 4, 4 ),
			GroupSpacing( 8 ),
			MUIA_Background, MUII_RequesterBack,
			Child, txt_req = TextObject,
				TextFrame,
				InnerSpacing( 8, 8 ),
				MUIA_Background, MUII_TextBack,
				MUIA_Text_SetMax, TRUE,
			End,
			Child, grp_buttons = HGroup,
				GroupSpacing( 0 ),
			End,
		End,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	if( !( data->parentobj = ( APTR )GetTagData( MA_SmartReq_Parent, 0, msg->ops_AttrList ) ) || !( data->objptr = ( APTR *)GetTagData( MA_SmartReq_Ptr, 0, msg->ops_AttrList ) ) )
	{
		CoerceMethod( cl, obj, OM_DISPOSE );
		return( 0 );
	}

	data->methodid = GetTagData( MA_SmartReq_MethodID, 0, msg->ops_AttrList );
	data->txt_req = txt_req;
	data->grp_buttons = grp_buttons;

	NEWLIST( &data->reqlist );

	D( db_dlwin, bug( "smartreq object 0x%lx created\n", obj ) );

	return( ( ULONG )obj );
}


DECSMETHOD( SmartReq_Enqueue )
{
	GETDATA;

	ADDTAIL( &data->reqlist, msg->rn );

	/*
	 * Fire off the first request.
	 */
	if( !getv( obj, MUIA_Window_Open ) )
	{
		DoMethod( obj, MM_SmartReq_Change );
		set( obj, MUIA_Window_Open, TRUE );
		if( !getv( obj, MUIA_Window_Open ) )
		{
			displaybeep();
			return( 0 ); /* argh, there's little we can do, wait the next request :/ */
		}
	}

	return( 0 );
}


/*
 * Changes the requester.
 */
DECTMETHOD( SmartReq_Change )
{
	GETDATA;
	struct reqnode *rn;
	ULONG gadcnt, i;
	int act = FALSE, ie = FALSE;
	char *p, *next, *ul;
	APTR txtobj;
	char iebuf[ 10 ][ 3 ];
	struct List *l;
	APTR ostate, o;

	if( ISLISTEMPTY( &data->reqlist ) )
	{
		/*
		 * Ok, no more queued stuff. Close the window and kill it to
		 * save memory.
		 */
		DoMethod( app, MUIM_Application_PushMethod, obj, 1, MM_SmartReq_Close );
		return( 0 );
	}

	rn = LASTNODE( &data->reqlist );

	D( db_dlwin, bug( "processing node 0x%lx\n", rn ) );

	/*
	 * Set the main text
	 */
	set( data->txt_req, MUIA_Text_Contents, rn->buffer );

	DoMethod( data->grp_buttons, MUIM_Group_InitChange );

	/*
	 * Remove the childs, if any.
	 */
	if( ( l = ( struct List * )getv( data->grp_buttons, MUIA_Group_ChildList ) ) )
	{
		if( !ISLISTEMPTY( l ) )
		{
			ostate = FIRSTNODE( l );
			while( o = NextObject( &ostate ) )
			{
				DoMethod( data->grp_buttons, OM_REMMEMBER, o );
				MUI_DisposeObject( o );
			}
		}
	}

	/*
	 * Build the buttons
	 */
	for( gadcnt = 1, p = rn->gadgets; *p; p++ )
	{
		if( *p == '|' )
		{
			gadcnt++;
		}
	}

	for( p = rn->gadgets, i = 0; i < gadcnt; i++, p = next )
	{
		/* handle special chars */
		if( ( next = strchr( p, '|' ) ) )
		{
			*next++ = '\0';
		}
		
		if( *p == '*' )
		{
			act = TRUE;
			p++;
		}

		if( ( ul = strchr( p, '_' ) ) )
		{
			ie = FALSE;
		}

		txtobj = TextObject,
			ButtonFrame,
			MUIA_CycleChain, 1,
			MUIA_Text_Contents, p,
			MUIA_Text_PreParse, "\33c",
			MUIA_InputMode, MUIV_InputMode_RelVerify,
			MUIA_Background, MUII_ButtonBack,
			ul ? MUIA_Text_HiIndex : TAG_IGNORE, '_',
			ul ? MUIA_ControlChar : TAG_IGNORE, ul ? ToLower(*(ul+1)) : 0,
		End;

		if( txtobj )
		{
			if( gadcnt == 1 )
			{
				DoMethod( data->grp_buttons, OM_ADDMEMBER, HSpace(0) );
				DoMethod( data->grp_buttons, OM_ADDMEMBER, HSpace(0) );
				DoMethod( data->grp_buttons, OM_ADDMEMBER, txtobj );
				DoMethod( data->grp_buttons, OM_ADDMEMBER, HSpace(0) );
				DoMethod( data->grp_buttons, OM_ADDMEMBER, HSpace(0) );
				//set( obj, MUIA_Window_DefaultObject, txtobj );
			}
			else if( i < gadcnt - 1 )
			{
				DoMethod( data->grp_buttons, OM_ADDMEMBER, txtobj );
				DoMethod( data->grp_buttons, OM_ADDMEMBER, HSpace(4) );
				DoMethod( data->grp_buttons, OM_ADDMEMBER, HSpace(0) );
			}
			else
			{
				DoMethod( data->grp_buttons, OM_ADDMEMBER, txtobj );
			}

			if( i <= 8 )
			{
				iebuf[i][0] = 'f';
				iebuf[i][1] = '0'+i+1;
				iebuf[i][2] = 0;
				
				DoMethod( obj, MUIM_Notify, MUIA_Window_InputEvent, iebuf[ i ], app, 6, MUIM_Application_PushMethod, obj, 3, MM_SmartReq_Pressed, ( i == gadcnt - 1 ) ?  0 : ( i + 1 ), rn->userdata );
			}
			DoMethod( txtobj, MUIM_Notify, MUIA_Pressed, FALSE, app, 6, MUIM_Application_PushMethod, obj, 3, MM_SmartReq_Pressed, ( i == gadcnt - 1 ) ?  0 : ( i + 1 ), rn->userdata );

			if( act )
			{
			//	  set( obj, MUIA_Window_ActiveObject, txtobj );
				act = FALSE;
			}
		}
		if( next )
		{
			*(next - 1) = '|';
		}
	}

	DoMethod( data->grp_buttons, MUIM_Group_ExitChange );

	REMOVE( rn );
	free( rn->gadgets );
	free( rn->buffer );
	free( rn );

	return( 0 );
}


DECSMETHOD( SmartReq_Pressed )
{
	GETDATA;

	/*
	 * Inform the parent object and rebuild
	 * the next requester.
	 */
	DoMethod( data->parentobj, data->methodid ? data->methodid : MM_SmartReq_Pressed, msg->butnum, msg->userdata );
	DoMethod( obj, MM_SmartReq_Change );

	return( 0 );
}


/*
 * Closes the window. Always call from PushMethod.
 */
DECTMETHOD( SmartReq_Close )
{
	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( app, OM_REMMEMBER, obj );
	MUI_DisposeObject( obj );

	return( 0 );
}


DECDISPOSE
{
	GETDATA;

	*data->objptr = NULL; /* invalidate ourself to the calling task */

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFSMETHOD( SmartReq_Enqueue )
DEFSMETHOD( SmartReq_Pressed )
DEFTMETHOD( SmartReq_Change )
DEFTMETHOD( SmartReq_Close )
ENDMTABLE


static struct MUI_CustomClass *mcc;

int create_smartreqclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "SmartReqClass";
#endif

	return( TRUE );
}

void delete_smartreqclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getsmartreqclass( void )
{
	return( mcc->mcc_Class );
}


/*
 * We don't use a method so that we have
 * the convenience of varargs.
 * If the smartreq object is not there, it's automatically created. It can also be automatically
 * removed anytime. That's why the calling party must check the 'obj' pointer to see if it's NULL
 * before disposing the object itself within its OM_DISPOSE method (see download.c)
 */
int STDARGS smartreq_request( APTR *obj, STRPTR title, APTR winobj, ULONG methodid, LONG userdata, STRPTR gadgets, STRPTR format, ... )
{
	struct reqnode *rn;
	va_list va;
	
	if( !title ) /* TOFIX: hm, put that in the structure too ? */
	{
		title = "Voyager request";
	}

	va_start( va, format );

	D( db_dlwin, bug( "adding request %s\n", title ) );

	if( !*obj )
	{
		*obj = NewObject( getsmartreqclass(), NULL,
			MA_SmartReq_Ptr, obj,
			MA_SmartReq_Parent, winobj,
			MA_SmartReq_MethodID, methodid,
			MUIA_Window_Title, title,
			MUIA_Window_RefWindow, winobj,
			TAG_DONE
		);

		if( !*obj )
		{
			va_end( va );
			return( FALSE );
		}
		DoMethod( app, OM_ADDMEMBER, *obj );
	}

	if( ( rn = malloc( sizeof( struct reqnode ) ) ) )
	{
		char buf[ REQ_BUFFERSIZE ];
		int len;
		rn->userdata = userdata;

		/*
		 * Process the input to build a buffer.
		 * TOFIX: optimization, only copy when there's more than 1 request.
		 */
		buf[ 0 ] = '\0'; /* I don't know if vsnprintf() NULL terminates if the format is not here.. better safe than sorry */
#ifdef __SASC
		vsprintf( buf, format, va );
#else
		vsnprintf( buf, REQ_BUFFERSIZE, format, va ); /* TOFIX: needs SAS/C support.. grr :( */
#endif
		len = strlen( buf );

		if( len )
		{
			if( ( rn->buffer = malloc( len + 1 ) ) )
			{
				strcpy( rn->buffer, buf );

				if( ( rn->gadgets = malloc( strlen( gadgets ) + 1 ) ) )
				{
					strcpy( rn->gadgets, gadgets );
					DoMethod( *obj, MM_SmartReq_Enqueue, rn );
					va_end( va );
					return( TRUE );
				}
				free( rn->buffer );
			}
		}
		free( rn );
	}
	va_end( va );

	return( FALSE );
}

