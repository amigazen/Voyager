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
 * AuthBrowser window class
 * ------------------------
 * - Displays a list with authentications
 *
 * © 1999 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: authbrowser.c,v 1.22 2003/07/06 16:51:32 olli Exp $
 *
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "authbrowser.h"
#include "classes.h"
#include "voyager_cat.h"
#include "authipc.h"
#include "mui_func.h"
#include "nlist.h"


APTR win_authbrowser;

extern struct MinList authlist;
extern struct SignalSemaphore authlistsem;

/* instance data */
struct Data
{
	APTR lv_auth;
	int  changed;
};

/* hooks */
MUI_HOOK( auth_disp, STRPTR *array, struct authnode *auth )
{
	if (auth)
	{
		*array++ = auth->server;
		*array++ = auth->realm;
		*array++ = auth->authuser;
		*array   = auth->save ? "\033cYes" : "\033cNo";
	}
	else
	{
		*array++ = GS(COOKIEBROWSER_LIST_SERVER);
		*array++ = GS(AUTHBROWSER_LIST_REALM);
		*array++ = GS(AUTHBROWSER_LIST_USER);
		*array   = GS(AUTHBROWSER_LIST_REMEMBER);
	}
	return(0);
}


DECNEW
{
	struct Data *data;
	APTR lv_auth, bt_removeall, bt_remove;

	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_ID, MAKE_ID('A','T','B','R'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_NoMenus, TRUE,
		MUIA_Window_Title, GS(AUTHBROWSER_WINTITLE),
		MUIA_Window_RootObject, VGroup,

			Child, lv_auth = MUI_NewObject(listviewclass, MUIA_CycleChain, 1,
				MUIA_Listview_List, MUI_NewObject(listclass, InputListFrame,
					MUIA_List_Title, TRUE,
					MUIA_List_Format, "BAR,BAR,BAR,BAR",
					MUIA_List_DisplayHook, &auth_disp_hook,
				End,
			End,

			Child, HGroup,
				Child, bt_removeall = makebutton(MSG_COOKIEBROWSER_BT_REMOVEALL),
				Child, bt_remove = makebutton(MSG_COOKIEBROWSER_BT_REMOVE),
			End,
		End,
	End;

	if (!obj)
		return(0);

	data = INST_DATA(cl, obj);

	data->lv_auth = lv_auth;

	/* remove button */
	DoMethod(lv_auth, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, bt_remove, 3, MUIM_Set, MUIA_Disabled, FALSE);
	DoMethod(lv_auth, MUIM_Notify, MUIA_List_Active, MUIV_List_Active_Off, bt_remove, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod(bt_remove, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MM_AuthBrowser_ListRemove, FALSE);
	set(bt_remove, MUIA_Disabled, TRUE);

	/* double click on lister */
	DoMethod(lv_auth, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, obj, 1, MM_AuthBrowser_TogglePassword);

	/* remove all button */
	DoMethod(bt_removeall, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Self, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod(bt_removeall, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MM_AuthBrowser_ListRemove, TRUE);

	/* window */
	DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 3, MUIM_Set, MUIA_Window_Open, FALSE);

	ObtainSemaphoreShared(&authlistsem);
	if (ISLISTEMPTY(&authlist))
	{
		set(bt_removeall, MUIA_Disabled, TRUE);  
	}
	else
	{
		struct authnode *anp, *anpn;

		for (anp = FIRSTNODE(&authlist); anpn = NEXTNODE(anp); anp = anpn)
		{
			DoMethod(lv_auth, MUIM_List_InsertSingle, anp, MUIV_List_Insert_Bottom);
		}
	}
		
	ReleaseSemaphore(&authlistsem);

	return((ULONG)obj);
}


/* deletes one/all entrie(s) in a list and the authnode */
DECSMETHOD(AuthBrowser_ListRemove)
{
	struct authnode *auth, *anp, *anpn;
	GETDATA; 

	ObtainSemaphore(&authlistsem);
	
	if (msg->all)
	{
		for (anp = FIRSTNODE(&authlist); anpn = NEXTNODE(anp); anp = anpn)
		{
			REMOVE( anp );
		}
		DoMethod(data->lv_auth, MUIM_List_Clear);
	}
	else
	{
		DoMethod(data->lv_auth, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &auth);

		for (anp = FIRSTNODE(&authlist); anpn = NEXTNODE(anp); anp = anpn)
		{
			if (!strcmp(auth->realm, anp->realm) && !strcmp(auth->server, anp->server) && !strcmp(auth->authuser, anp->authuser))
			{
				REMOVE( anp );

				DoMethod(data->lv_auth, MUIM_List_Remove, MUIV_List_Remove_Active);
			}
		}
	}
	data->changed = TRUE;
	ReleaseSemaphore(&authlistsem);
	
	return(TRUE);
}


/* adds one entry to the list */
DECSMETHOD(AuthBrowser_Add)
{
	GETDATA;

	DoMethod(data->lv_auth, MUIM_List_InsertSingle, msg->auth, MUIV_List_Insert_Bottom);

	return(TRUE);
}


/* toggle password saving state */
DECMETHOD( AuthBrowser_TogglePassword, ULONG )
{
	struct authnode *auth, *anp, *anpn;
	GETDATA;

	ObtainSemaphore(&authlistsem);

	DoMethod(data->lv_auth, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &auth);

	for (anp = FIRSTNODE(&authlist); anpn = NEXTNODE(anp); anp = anpn)
	{
		if (!strcmp(auth->realm, anp->realm) && !strcmp(auth->server, anp->server) && !strcmp(auth->authuser, anp->authuser))
		{
			anp->save = auth->save = !anp->save;
			DoMethod(data->lv_auth, MUIM_List_Redraw, MUIV_List_Redraw_Active);
		}
	}
	data->changed = TRUE;
	
	ReleaseSemaphore(&authlistsem);

	return(TRUE);
}

// TOFIX!! it seems this is not needed anymore
DECGET
{
	GETDATA;

	if (msg->opg_AttrID == MA_AuthBrowser_Changed)
	{
		*msg->opg_Storage = data->changed;
		return(TRUE);
	}
	return(DOSUPER);
}


BEGINMTABLE
DEFNEW
DEFGET
DEFMETHOD( AuthBrowser_TogglePassword )
DEFSMETHOD( AuthBrowser_ListRemove )
DEFSMETHOD( AuthBrowser_Add )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_authbrowserwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "AuthBrowserWinClass";
#endif

	return( TRUE );
}

void delete_authbrowserwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getauthbrowserwin( void )
{
	return( mcc->mcc_Class );
}

void openauthbrowserwin(void)
{
	if (!win_authbrowser)
	{
		if (win_authbrowser = NewObject(getauthbrowserwin(), NULL, TAG_DONE))
		{
			DoMethod(app, OM_ADDMEMBER, win_authbrowser);
		}
		else
			displaybeep();
	}
	
	if (win_authbrowser)
		set(win_authbrowser, MUIA_Window_Open, TRUE);
}

#endif /* USE_NET */
