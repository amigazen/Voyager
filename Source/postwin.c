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


#if 0
/*
**
** $Id: postwin.c,v 1.17 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"
#include "classes.h"
#include "prefs.h"
#include "voyager_cat.h"

#define string(x,y,z) TextinputObject,StringFrame,MUIA_String_MaxLen,y,MUIA_String_Contents,x,MUIA_CycleChain,1,End

extern struct MinList winlist;
static struct MUI_CustomClass *lcc;

void geturlfilename( char *url, char *out );
int preparenewsquote( char *msgid, char *filename, char *newsgroups, char *subject, char *replyid, char *references );

void tf_load( APTR tvo, STRPTR fn )
{
	DoMethod( tvo, MUIM_Textinput_LoadFromFile, fn );
}

STRPTR tf_getcon( APTR tvo )
{
	return( getstrp( tvo ) );
}

struct window {
	struct MinNode n;
	UWORD num;
	UWORD removeme;

	APTR win;   // link back to object (for list ops)

	APTR menuitem;

	char win_title[ 128 ];
	char menutitle[ 60 ];
	char menushortcut[ 4 ];

	char url[ 256 ];

	APTR str_ng, str_subject, tf_text;

	char newsgroups[ 256 ], subject[ 256 ], replyid[ 256 ], references[ 512 ];
};


static struct window *findwinbynum( ULONG num )
{
	struct window *ch;

	for( ch = FIRSTNODE( &winlist); NEXTNODE( ch ); ch = NEXTNODE( ch ) )
		if( ch->num == num )
			return( ch );

	return( NULL );
}

static void setwintitle( struct window *w )
{
	sprintf( w->win_title, "[%ld] " APPNAME " · Post news article", w->num );
	set( w->win, MUIA_Window_Title, w->win_title );

	sprintf( w->menutitle, "[%ld] Post news", w->num );
	set( w->menuitem, MUIA_Menuitem_Title, w->menutitle );
}

DECCONST
{
	int c;
	APTR grp;
	struct window *new;
	APTR dummyobject = MUI_NewObject( MUIC_Area, NULL );
	APTR bt_send, bt_cancel;
	int isreply;

	for( c = 1; ; c++ )
	{
		if( !findwinbynum( c ) )
			break;
	}

	obj = DoSuperNew( cl, obj, 
		MUIA_Window_ScreenTitle, (ULONG)copyright,
		MUIA_Window_ID, MAKE_ID('P','N','E', c),
		MUIA_Window_RootObject, dummyobject,
		MUIA_Window_Activate, FALSE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
	{
		D( db_mail, bug( "nah!\n" ));
		return( NULL );
	}

	new = INST_DATA( cl, obj );
	//memset( new, 0, sizeof( *new ) );

	new->num = c;
	new->win = obj;

	stccpy( new->url, 
		(STRPTR)GetTagData( MA_PostWin_URL, (ULONG)"", msg->ops_AttrList ),
		256 
	);

	if( strchr( new->url, '@' ) )
		isreply = TRUE;
	else
		isreply = FALSE;

	if( isreply )
		preparenewsquote( new->url, "T:.quotetmp", new->newsgroups, new->subject, new->replyid, new->references );

	grp = VGroup, 
		Child, ColGroup( 2 ),
			Child, Label2( "Newsgroups:" ),
			Child, new->str_ng = string( isreply ? new->newsgroups : new->url, 256, 0 ),
			Child, Label2( "Subject: "),
			Child, new->str_subject = string( new->subject, 256, 0 ),
		End,

		Child, new->tf_text = TextinputscrollObject, 
			MUIA_Textinput_Multiline, TRUE,
			StringFrame,
			MUIA_CycleChain, 1, 
		End,

		Child, HGroup,
			Child, bt_send = SimpleButton( "Post article" ),
			Child, bt_cancel = SimpleButton( "Cancel" ),
		End,
	End;

	if( !grp )
	{
		MUI_Request( app, NULL, 0, copyright, GS( CANCEL ), GS( WIN_FAILED ), MUI_Error() );
		closestuff();
		exit( 20 );
	}

	set( obj, MUIA_Window_RootObject, grp );
	MUI_DisposeObject( dummyobject );

	ADDTAIL( &winlist, new );

	if( new->num < 11 )
		sprintf( new->menushortcut, "%ld", new->num < 10 ? new->num : 0 );

	if (menu)
	{
		Object *x = findmenu(MENU_WINDOWS);

		if (x)
		{
			new->menuitem = MUI_MakeObject( MUIO_Menuitem, "V", new->num < 11 ? new->menushortcut : NULL, 0, 20000 + new->num );
			if (new->menuitem)
			{
				DoMethod( x, MUIM_Family_AddTail, new->menuitem );
				DoMethod( new->menuitem, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
					obj, 3, MUIM_Set, MUIA_Window_Activate, TRUE
				);
			}
		}
	}

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Win_Kill
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Win_Kill
	);

	DoMethod( bt_send, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_PostWin_Send
	);

	set( bt_send, MUIA_CycleChain, 1 );
	set( bt_cancel, MUIA_CycleChain, 1 );

	setwintitle( new );

	if( isreply )
	{
		tf_load( new->tf_text, "T:.quotetmp" );
		DeleteFile( "T:.quotetmp" );
	}
	tf_load( new->tf_text, "PROGDIR:" APPNAME ".Signature" );
	
	return( (ULONG)obj );
}

DECDEST
{
	struct window *w = INST_DATA( cl, obj );

	//set( w->win, MUIA_Window_Open, FALSE );

	REMOVE( w );

	return( DOSUPER );
}

DECMETHOD( Win_Kill, APTR )
{
	struct window *w = INST_DATA( cl, obj );

	w->removeme = TRUE;

	if (menu && findmenu(MENU_WINDOWS))
	{
		DoMethod( findmenu( MENU_WINDOWS ), MUIM_Family_Remove, w->menuitem );
	}

	if (w->menuitem)
	{
		MUI_DisposeObject( w->menuitem );
		w->menuitem = NULL;
	}

	set( w->win, MUIA_Window_Open, FALSE );

	DoMethod( app, MM_App_CheckWinRemove );

	return( 0 );
}

extern int sendnews( char *subject, char *body, char *replyid, char *references, char *newsgroups );
DECMETHOD( PostWin_Send, APTR )
{
	struct window *w = INST_DATA( cl, obj );
	int success = FALSE;

	set( app, MUIA_Application_Sleep, TRUE );

	success = sendnews( getstrp( w->str_subject ), tf_getcon( w->tf_text ), w->replyid[ 0 ] ? w->replyid : NULL, w->references, getstrp( w->str_ng ) );

	set( app, MUIA_Application_Sleep, FALSE );

	if( success )
		DoMethod( obj, MM_Win_Kill );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Win_Kill )
DEFMETHOD( PostWin_Send )
ENDMTABLE

int create_postwinclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct window ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "PostWinClass";
#endif

	return( TRUE );
}

void delete_postwinclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

static APTR getpostwinclass( void )
{
	return( lcc->mcc_Class );
}

void createpostwin( char *url )
{
	APTR o;
	set( app, MUIA_Application_Sleep, TRUE );

	if( !( o = NewObject( getpostwinclass(), NULL, MA_PostWin_URL, url, TAG_DONE ) ) )
	{
		set( app, MUIA_Application_Sleep, FALSE );
		MUI_Request( app, NULL, 0, copyright, GS( CANCEL ), GS( WIN_FAILED ), MUI_Error() );
		closestuff();
		exit( 20 );
	}
	DoMethod( app, OM_ADDMEMBER, o );
	set( app, MUIA_Application_Sleep, FALSE );
	set( o, MUIA_Window_Open, TRUE );
}

#endif
