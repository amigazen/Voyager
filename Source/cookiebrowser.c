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
 * CookieBrowser window class
 * --------------------------
 * - Displays cookies with their infos in a list
 *
 * © 1999 by David Gerber <zapek@vapor.com>
 * All rights reserved
 *
 * $Id: cookiebrowser.c,v 1.27 2003/07/06 16:51:33 olli Exp $
 *
 * PS: Locutus sucks
 *
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "classes.h"
#include "voyager_cat.h"
#include "cookies.h"
#include "time_func.h"
#include "prefs.h"
#include "mui_func.h"
#include "nlist.h"

APTR win_cookiebrowser;

extern struct MinList cookielist;
extern struct SignalSemaphore cookiesem;

/* instance data */
struct Data
{
	APTR lv_cookies;
	int column;      /* number of columns */
	int reverse;   /* direction of sorting */
};

static struct MUI_CustomClass *mcc;

#if USE_NLIST

MUI_HOOK( cookie_comp2, struct cookie *cookie, struct NList_CompareMessage *ncmsg )
{
	struct Data *data = (VOID *)(((UBYTE *)h->h_Data) + mcc->mcc_Class->cl_InstOffset);
	struct cookie *cookie1 = ncmsg->entry1;
	struct cookie *cookie2 = ncmsg->entry2;
	LONG col1 = ncmsg->sort_type & MUIV_NList_TitleMark_ColMask;
	LONG result = 0;

	if (ncmsg->sort_type == MUIV_NList_SortType_None)
		return(0);

	if (data->column == -1 && data->reverse == TRUE)
	{
		ncmsg->sort_type = ncmsg->sort_type - MUIV_NList_TitleMark_TypeMask;
	}

	if (col1 == 0) /* column: server */
	{
		if (data->reverse) /* order */
			result = stricmp(cookie2->server, cookie1->server);
		else
			result = stricmp(cookie1->server, cookie2->server);
	}
	else if (col1 == 1) /* column: path */
	{
		if (data->reverse)
			result = stricmp(cookie2->path, cookie1->path);
		else
			result = stricmp(cookie1->path, cookie2->path);
	}
	else if (col1 == 2) /* column: age */
	{
		if (data->reverse)
			result = cookie2->age - cookie1->age;
		else
			result = cookie1->age - cookie2->age;
	}
	else if (col1 == 3) /* column: expires */
	{
		if (data->reverse)
			result = cookie2->expires - cookie1->expires;
		else
			result = cookie1->expires - cookie2->expires;
	}
	else if (col1 == 4) /* column: name */
	{
		if (data->reverse)
			result = stricmp(cookie2->name, cookie1->name);
		else
			result = stricmp(cookie1->name, cookie2->name);
	}
	else if (col1 == 5) /* column: data */
	{
		if (data->reverse)
			result = stricmp(cookie2->data, cookie1->data);
		else
			result = stricmp(cookie1->data, cookie2->data);
	}
 
	return(result);
}
#endif /* USE_NLIST */

/* hooks */
MUI_HOOK( cookie_disp, STRPTR *array, struct cookie *cookie )
{
	if (cookie)
	{
		static char expiresbuffer[22];
		static char agebuffer[22];
		
		*array++ = cookie->server;
		*array++ = cookie->path;
		strcpy(agebuffer, date2string(cookie->age));
		*array++ = agebuffer;
		strcpy(expiresbuffer, date2string(cookie->expires));
		*array++ = expiresbuffer;
		*array++ = cookie->name;
		*array   = cookie->data;
	}
	else
	{
		*array++ = GS(COOKIEBROWSER_LIST_SERVER);
		*array++ = GS(COOKIEBROWSER_LIST_PATH);
		*array++ = GS(COOKIEBROWSER_LIST_SETON);
		*array++ = GS(COOKIEBROWSER_LIST_EXPIRES);
		*array++ = GS(COOKIEBROWSER_LIST_NAME);
		*array   = GS(COOKIEBROWSER_LIST_DATA);
	}
	return(0);
}


DECNEW
{
	struct Data *data;
	APTR lv_cookies, bt_removeall, bt_remove;

	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_ID, MAKE_ID('C','K','B','R'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_NoMenus, TRUE,
		MUIA_Window_Title, GS(COOKIEBROWSER_WINTITLE),
		MUIA_Window_RootObject, VGroup,

			Child, lv_cookies = MUI_NewObject(listviewclass, MUIA_CycleChain, 1,
				MUIA_Listview_List, MUI_NewObject(listclass, InputListFrame,
					MUIA_List_Title, TRUE,
					MUIA_List_Format, "BAR,BAR,BAR,BAR,BAR,BAR",
					MUIA_List_DisplayHook, &cookie_disp_hook,
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

#if USE_NLIST
	if (nlist)
	{
		set(lv_cookies, MUIA_NList_MinColSortable, 0);
		set(lv_cookies, MUIA_NList_CompareHook2, &cookie_comp2_hook);
		cookie_comp2_hook.h_Data = obj;
	}
#endif

	data = INST_DATA(cl, obj);

	data->lv_cookies = lv_cookies;

#if USE_NLIST
	/* sorting */
	if (nlist)
		DoMethod(lv_cookies, MUIM_Notify, MUIA_NList_TitleClick, MUIV_EveryTime, obj, 2, MM_CookieBrowser_StartSorting, MUIV_TriggerValue, data->reverse);
#endif

	/* remove button */
	DoMethod(lv_cookies, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, bt_remove, 3, MUIM_Set, MUIA_Disabled, FALSE);
	DoMethod(lv_cookies, MUIM_Notify, MUIA_List_Active, MUIV_List_Active_Off, bt_remove, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod(bt_remove, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MM_CookieBrowser_ListRemove, FALSE);
	set(bt_remove, MUIA_Disabled, TRUE);

	/* remove all button */
	DoMethod(bt_removeall, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Self, 3, MUIM_Set, MUIA_Disabled, TRUE);
	DoMethod(bt_removeall, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MM_CookieBrowser_ListRemove, TRUE);

	/* window */
	DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 3, MUIM_Set, MUIA_Window_Open, FALSE);

	/* fill in the list */
	ObtainSemaphoreShared(&cookiesem);
	if (ISLISTEMPTY(&cookielist))
	{
		set(bt_removeall, MUIA_Disabled, TRUE);  
	}
	else
	{
		struct cookie *cmp, *cmpn;

		for (cmp = FIRSTNODE(&cookielist); cmpn = NEXTNODE(cmp); cmp = cmpn)
		{
			DoMethod(lv_cookies, MUIM_List_InsertSingle, cmp, MUIV_List_Insert_Bottom);
		}
	}
		
	ReleaseSemaphore(&cookiesem);

	data->column = -1;

#if USE_NLIST
	if (nlist)
		DoMethod(obj, MM_CookieBrowser_StartSorting, getprefslong(DSI_MISC_COOKIEBROWSER_SORT_COLUMN, 0), getprefslong(DSI_MISC_COOKIEBROWSER_SORT_REVERSE, FALSE));
#endif

	return((ULONG)obj);
}


/* deletes one/all entrie(s) in a list and the cookies */
DECSMETHOD(CookieBrowser_ListRemove)
{
	GETDATA;

	struct cookie *cookie, *cmp, *cmpn;

	ObtainSemaphore(&cookiesem);
	
	if (msg->all)
	{
		while( cmp = REMHEAD( &cookielist ) )
		{
			free( cmp->name );
			free( cmp->data );
			free( cmp->server );
			free( cmp->path );
			free( cmp );
		}
		DoMethod(data->lv_cookies, MUIM_List_Clear);
	}
	else
	{
		DoMethod(data->lv_cookies, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &cookie);

		for (cmp = FIRSTNODE(&cookielist); cmpn = NEXTNODE(cmp); cmp = cmpn)
		{
			if(!strcmp(cookie->name, cmp->name) && !strcmp(cookie->server, cmp->server))
			{
				REMOVE( cmp );
				free( cmp->name );
				free( cmp->data );
				free( cmp->server );
				free( cmp->path );
				free( cmp );

				DoMethod(data->lv_cookies, MUIM_List_Remove, MUIV_List_Remove_Active);
			}
		}
	}
	ReleaseSemaphore(&cookiesem);
	
	return(TRUE);
}


/* adds one entry to the list */
DECSMETHOD(CookieBrowser_Add)
{
	GETDATA;

	DoMethod(data->lv_cookies, MUIM_List_InsertSingle, msg->cookie, MUIV_List_Insert_Bottom);

	return(TRUE);
}


/* deletes one entry in the list */
DECSMETHOD(CookieBrowser_Delete)
{
	int i, max;
	struct cookie *cookie;

	GETDATA;

	for (max = getv(data->lv_cookies, MUIA_List_Entries), i = 0 ; i < max ; i++)
	{
		DoMethod(data->lv_cookies, MUIM_List_GetEntry, i, &cookie);
		
		if(!strcmp(msg->cookie->name, cookie->name) && !strcmp(msg->cookie->server, cookie->server))
		{
			DoMethod(data->lv_cookies, MUIM_List_Remove, i);
			break;
		}
	}
 
	return(TRUE);
}

#if USE_NLIST

/* puts the arrows on the titles and sort */
DECSMETHOD(CookieBrowser_StartSorting)
{
	GETDATA;

	if (data->column == msg->column)
	{
		data->reverse = !data->reverse;
	}
	else
	{
		if (data->column == -1)
			data->reverse = msg->reverse;
		else
			data->reverse = FALSE;
	}

	DoMethod(data->lv_cookies, MUIM_NList_Sort2, msg->column, MUIV_NList_SortTypeAdd_2Values);

	data->column = msg->column;

	set(data->lv_cookies, MUIA_NList_TitleMark, data->column | (data->reverse ? MUIV_NList_TitleMark_Up : MUIV_NList_TitleMark_Down));
	
	return(0);
}
#endif /* USE_NLIST */

/* OM_GET */
DECGET
{
	GETDATA;

	if (msg->opg_AttrID == MA_CookieBrowser_Column)
	{
		*msg->opg_Storage = data->column;
		return(TRUE);
	}
	else if (msg->opg_AttrID == MA_CookieBrowser_Reverse)
	{
		*msg->opg_Storage = data->reverse;
		return(TRUE);
	}
	return(DOSUPER);
}


BEGINMTABLE
DEFNEW
DEFGET
#if USE_NLIST
DEFSMETHOD( CookieBrowser_StartSorting )
#endif
DEFSMETHOD( CookieBrowser_ListRemove )
DEFSMETHOD( CookieBrowser_Add )
DEFSMETHOD( CookieBrowser_Delete )
ENDMTABLE


int create_cookiebrowserwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "CookieBrowserWinClass";
#endif

	return( TRUE );
}

void delete_cookiebrowserwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getcookiebrowserwin( void )
{
	return( mcc->mcc_Class );
}


void opencookiebrowserwin(void)
{
	if (!win_cookiebrowser)
	{
		if (win_cookiebrowser = NewObject(getcookiebrowserwin(), NULL, TAG_DONE))
		{
			DoMethod(app, OM_ADDMEMBER, win_cookiebrowser);
			set(win_cookiebrowser, MUIA_Window_Open, TRUE); 
		}
		else
			displaybeep();
	}
	else
		set(win_cookiebrowser, MUIA_Window_Open, TRUE);
}

#endif /* USE_NET */
