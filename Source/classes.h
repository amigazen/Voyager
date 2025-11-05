/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_CLASSES_H
#define VOYAGER_CLASSES_H
/*
 * $Id: classes.h,v 1.199 2003/11/21 09:34:19 zapek Exp $
 */

#include "config.h"

#ifdef __MORPHOS__
#include <exec/types.h>
#include <utility/tagitem.h>
#endif /* __MORPHOS__ */

// Custom methods
#define MTAGBASE (TAG_USER|((1307<<16)+8192)) /* 0x851b2000 */

#ifdef __GNUC__
struct BitMap;
struct MinList;
struct imgclient;
struct nstream;
struct dlnode;
#endif

// classes
APTR getamiconclass( void );
APTR getledclass( void );
APTR getmyvclass( void );
APTR getviewclass( void );
APTR getgaugeclass( void );
APTR getstbgaugeclass( void );
APTR gettxtlinkclass( void );
APTR getliclass( void );
APTR getnullclass( void );
APTR getimagelinkclass( void );
APTR geturlstringclass( void );
APTR getfastlinkclass( void );
APTR getddstringclass( void );
APTR getframeclass( void );
APTR getframeset( void );
APTR getframeborder( void );
APTR getdownloadwin( void );
APTR getappletclass( void );
APTR getprintwinclass( void );
APTR getsearchwinclass( void );
APTR getfastlinkgroupclass( void );
APTR getsmartlabelclass( void );
APTR getdocinfowinclass( void );
APTR gethistorylistclass( void );
APTR getbuttonclass( void );
APTR gettoolbarclass( void );
APTR geterrorwinclass( void );
APTR getfonttestclass( void );
#if USE_SPLASHWIN
APTR getsplashwinclass( void );
#endif
APTR getpluginwinlist( void );
APTR gethtmlviewclass( void );
APTR gethtmlwinclass( void );
APTR getcommandclass( void );
#if USE_CLOCK
APTR getclockclass( void );
#endif
#if USE_PLUGINS
APTR getpluginswin( void );
#endif
APTR getnetinfowin( void );
APTR getprefswin_mainclass( void );
APTR getprefswin_toolbarclass( void );
APTR getprefswin_languagesclass( void );
APTR getprefswin_cacheclass( void );
APTR getprefswin_certsclass( void );
APTR getprefswin_colorsclass( void );
APTR getprefswin_downloadclass( void );
APTR getprefswin_fastlinksclass( void );
APTR getprefswin_fontsclass( void );
APTR getprefswin_generalclass( void );
APTR getprefswin_hyperlinksclass( void );
APTR getprefswin_imagesclass( void );
APTR getprefswin_javascriptclass( void );
APTR getprefswin_mailnewsclass( void );
APTR getprefswin_networkclass( void );
APTR getprefswin_securityclass( void );
APTR getprefswin_spoofclass( void );
APTR getprefswin_listclass( void );
APTR getprefswin_contextmenuclass( void );

APTR getappclass( void );
APTR getjs_object( void );
APTR getjs_regexp( void );
APTR getjs_stb_root( void );
APTR getjs_stb_cdplayer( void );
struct MUI_CustomClass * getjs_object_class( void );
APTR getjs_objref( void );
APTR getjs_array( void );
struct MUI_CustomClass * getjs_array_class( void );
APTR getjs_real( void );
APTR getjs_string( void );
APTR getjs_bool( void );
APTR getjs_navigator( void );
APTR getjs_math( void );
APTR getjs_location( void );
APTR getjs_screen( void );
APTR getjs_link( void );
APTR getjs_func( void );
APTR getjs_date( void );
APTR getjs_plugin( void );
APTR getjs_mimetype( void );
APTR getjs_form( void );
APTR getjs_event( void );
APTR getcrossclass( void );
APTR getsizegroupclass( void );
APTR getscrollgroupclass( void );
#ifdef MBX
APTR getpipwindowclass( void );
#endif

enum {
	DDID_FONTS = 1,
	DDID_FASTLINKLABEL
};

enum {
	CROSSDIR_NORTH = 1,
	CROSSDIR_SOUTH = 2,
	CROSSDIR_EAST = 4,
	CROSSDIR_WEST = 8
};

#ifndef MUIA_Scrollgroup_AutoBars
#define MUIA_Scrollgroup_AutoBars 0x8042f50e
#endif

/*
 * Always add entries at the end of the list otherwise
 * it's not in sync with the imgdecoders.
 * Clean it up between releases though.
 *
 * Well, in fact, move MM_Img* at the top of the list and problem solved
 *
 */
enum {
	MA_dummy = (int)( MTAGBASE + 32768), /* 0x851ba000 */

	/* warning! for the next 3 the order counts */
	MM_NStream_GotInfo = (int)0x851ba045, // we now have document information (UNS_WAITING)
	MM_NStream_GotData, // data comming (UNS_READING)
	MM_NStream_Done,    // finished (UNS_DONE or UNS_FAILED)

	MM_URH_Inform,

	MM_Prefslist_Selectchange, // not used anymore... remove one day ( this desyncs the imagedecoders so be carefull, and move them on TOP too ! )

	MM_Led_Update,

	MM_Gauge_Reset,
	MM_Gauge_Clear,
	MM_Gauge_Set,
	MM_Gauge_SetText,
	MM_Gauge_Tick,         /* 10 */

	MM_ImgDecode_Done,
	MM_ImgDecode_HasInfo,
	MM_ImgDecode_GotScanline,

	MA_Frameset_NoResize,
	MA_Frameset_URL,
	MA_Frameset_ParentURL,
	MA_Frameset_Scrolling,
	MA_Frameset_Name,
	MA_Frameset_MarginHeight,
	MA_Frameset_MarginWidth,   /* 20 */
	MA_Frameset_IsFrameSet,
	MA_Frameset_Reload,
	MA_Frameset_ImageLoadMode,
	MA_Frameset_HTMLObject,

	MM_Frameset_FindName,
	MM_Frameset_SetURL,
	MM_Frameset_Back,
	MM_Frameset_Forward,
	MM_Frameset_Home,
	MM_Frameset_Print,          /* 30 */
	MM_Frameset_Reload,
	MM_Frameset_CalcLoading,
	MM_Frameset_CheckResize,

	MA_Frameborder_Orient, // 0 = vert, 1 = horiz
	MA_Frameborder_3D,
	MA_Frameborder_Min,
	MA_Frameborder_Max,
	MA_Frameborder_URL,
	MA_Frameborder_Iter,
	MA_Frameborder_IRow,        /* 40 */
	MA_Frameborder_ICol,

	MA_ImgObject_ImageURL,
	MA_ImgObject_ClickX,
	MA_ImgObject_ClickY,

	MA_Formelement_Type,    // type of formelement

	MA_Clicklink_MarkStart,
	MA_Clicklink_MarkEnd,
	MA_Clicklink_IsClickLink,
	MM_Clicklink_ResetMarkStartEnd,
	MA_Clicklink_LineNum,       /* 50 */
	MA_Clicklink_Title,

	MM_URLString_Change,

	MA_Ruler_NoShade,

	MA_Printwin_HTMLView,
	MA_Printwin_URL,

	MM_Downloadwin_Cleanup,
	MM_Downloadwin_SetButtons,
	MM_Downloadwin_Refresh,
	MM_Downloadwin_Abort,
	MM_Downloadwin_UpdateEntry,    /* 60 */
	MA_Downloadwin_Path,

	MA_Searchwin_HTMLView,

	MA_Smartlabel_Text,
	MA_Smartlabel_MoreText,

	MM_DocinfoWin_Close,        //
	MM_DocinfoWin_SetContent,   // struct MP_DocinfoWin_SetContent
	MM_DocinfoWin_SetCipher,    // struct MP_DocinfoWin_SetCipher
	MA_DocinfoWin_URL,          // char *url
	MA_DocinfoWin_SSL,          // BOOL
	MA_DocinfoWin_mailto,       // BOOL         /* 70 */

	MM_JS_SetGCMagic,	    // Set the garbage collector magic value
	MM_JS_GetGCMagic,	    // Return the current magic value

	MA_JS_FuncContext,          // function context of object
	MA_JS_Name,                 // object name
	MA_JS_ID, 	                // object id (DOM only)
	MM_JS_NameIs,               // Name is?

	MM_JS_GetTypeData,          // typeptr, dataptr, datasizeptr
	MM_JS_SetData,              // dataptr, datasizeptr
	MM_JS_GetProperty,          // name, typeptr, dataptr, datasizeptr
	MM_JS_SetProperty,          // name, dataptr, datasize, type        /* 80 */
	MM_JS_HasProperty,          // name
	MM_JS_ListProperties,		

	MM_JS_CallMethod,           // ID, typeptr, dataptr, datasize, argcnt, argarray

	MM_JS_ToString,             // name, strptr, lenptr
	MM_JS_ToReal,               // name, realptr
	MM_JS_ToBool,               // name, boolptr

	MA_JS_ClassName,            // Class Name

	// Helper attributes, class-specific
	MA_JS_Location_WindowObject,    // Window Object backlink
	MA_JS_Window_URL,               // used by Location to get current window URL

	MA_JS_Link_URL,                /* 90 */
	MA_JS_Link_Target,
	MA_JS_Link_Name,

	MA_JS_Date_Seconds,			// For initializing
	MA_JS_Date_Micros,
	MA_JS_Datestring,			

	MA_JS_ObjRef_Object,

	MA_JS_IsUndefined,

	MM_JS_Array_AddMember,
	MA_JS_Object_TerseArray,
	MA_JS_Object_CPL, // custom prop list       /* 100 */

	MM_JS_FreeCallMethod,

	MM_JS_DeleteProperty,       // delete a property

	MA_JS_Link_Object,          // for href/img link objects, link back to the A object

	// Event stuff
	MM_JS_HandleEvent_MouseOver,
	MM_JS_HandleEvent_MouseOut,

	MA_JS_Window_JSO, // JSO object of window

	MA_JS_Func_Index,
	MA_JS_Func_Object,
	MA_JS_Func_Window,

	MA_JS_Object_Baseref,       // URL base reference (for dealing with partial URLs, bah)   /* 110 */

	MM_JS_ObjRef_CleanupRef,

	MM_JS_Func_SetParms,

	/* Window Management */
	MM_Win_Kill,

	/* CookieBrowser */
	MM_CookieBrowser_ListRemove,
	MM_CookieBrowser_Add,
	MM_CookieBrowser_Delete,
	MM_CookieBrowser_StartSorting,
	MA_CookieBrowser_Column,
	MA_CookieBrowser_Reverse,

	/* Netinfo */
	MM_Netinfo_Close,                   /* 120 */
	MM_Netinfo_SetProgress,
	MM_Netinfo_SetURL,
	MM_Netinfo_SetMax,
	MM_Netinfo_SetClear,
	MM_Netinfo_SetParked,
	MM_Netinfo_Stop,

	/* Auth */
	MM_Auth_Close,
	MA_Auth_Server,
	MA_Auth_Realm,
	MA_Auth_Failed,                      /* 130 */
	MA_Auth_FTP,
	MA_Auth_AUM,

	/* AuthBrowser */
	MM_AuthBrowser_ListRemove,
	MM_AuthBrowser_Add,
	MM_AuthBrowser_TogglePassword,
	MA_AuthBrowser_Changed,

	/* Historylist */
	MM_Historylist_InsertURL,
	MM_Historylist_InsertFrame,
	MM_Historylist_AddURL,
	MM_Historylist_AddGlobal,             /* 140 */
	MM_Historylist_AddBF,
	MM_Historylist_Back,
	MM_Historylist_Next,
	MM_Historylist_GotoEntry,
	MM_Historylist_RemoveURL,
	MM_Historylist_Exists,
	MM_Historylist_InsertURLExtern,
	MM_Historylist_End,
	MM_Historylist_Undo,
	MM_Historylist_RestoreBFList,         /* 150 */
	MM_Historylist_StoreXY,
	MM_Historylist_GetXY,
	MA_Historylist_HasBack,
	MA_Historylist_HasNext,
	MA_Historylist_CurrentEntry,	/* active entry (real one, as the user can change MUIA_List_Active manually) */
	MA_Historylist_BFList,
	MA_Historylist_BFReadLock,
	MA_Historylist_Entries,
	MA_Historylist_Standalone,

	MM_App_DemoTimeout,                    /* 160 */
	MM_App_GetErrorWinClass, // needed because puterror() can be called from *ANY* task
	MM_App_CacheFlush,
	MM_App_CheckWinRemove,
	MM_App_SetWinClose,
	MM_App_UpdateSpoofMenu,
	MM_GetActiveWindow,
	MM_App_OpenBMWin,
	MM_App_OpenDownloadWin,
#ifdef VDEBUG
	MM_App_SetImgDebug,
#endif

#if USE_LO_PIP
	MM_App_ReenablePip,
#endif

	MA_Formelement_DefaultValue, // For RESET to defaults

	MA_dummy2,

	MM_App_DisplayError,	   // display an error; as HTML or MUI_Request()
	MM_App_ApplySpoof,

	MM_Cert_Close,
	MA_Cert_CertMessage,

	MM_Cookie_Close,
	MA_Cookie_CookieMessage,

	MM_Print_Close,
	MM_Print_CallPrefs,
	MM_Print_Start,

	MM_Search_Close,
	MM_Search_Start,

	MM_Sourceview_Close,
	MM_Sourceview_Update,
	MA_Sourceview_URL,

	MA_Amicon_Active, // that class should get a new name

	MM_DoAllWins,
	MM_DoActiveWin,
	MM_DoLastActiveWin,
	MM_DoActivePanel,
	MM_DoLastActivePanel,
	MM_DoNumWin,
	MM_DoRexxWin,

	MM_MyV_SetDoc,
	MM_MyV_IncrementDoc,
	MM_MyV_ReviewDoc,
	MM_MyV_GotoHash,
	MM_MyV_ResetForm,
	MM_MyV_EnableForm,
	MM_MyV_CalcUnloadedImages,
	MM_MyV_Print,
	MM_MyV_SetFormRadio,

	MA_MyV_IterDepth,
	MA_MyV_Title,

	MA_DropObject_URL,
	MA_DropObject_Name,

	MA_DDID,

	MA_Form_ID,

	MM_Formelement_Reset,
	MA_Formelement_Name,
	MA_Formelement_Names,
	MA_Formelement_Data,
	MA_Formelement_IsSend,
	MA_Formelement_IsReset,

	MA_TextView_Contents,

	MA_URLString_Text,

	MA_TFrame_Size,

	MM_Custom, //?!?

	/* DisplayNode */

	MM_DNode_Select,

	MA_DNode_X,
	MA_DNode_XS,
	MA_DNode_Y,
	MA_DNode_YS,
	MA_DNode_AnchorID,
	MA_DNode_IsImage,

	/* DisplayNode <shrug>, find out what it is */
	MA_DNodeT_Txt,
	MA_DNodeT_TxtLen,
	MA_DNodeT_URL,
	MA_DNodeT_URLTarget,
	MA_DNodeT_Pen1,
	MA_DNodeT_Pen2,
	MA_DNodeT_Style,
	MA_DNodeT_Image,
	MA_DNode_PenShadow,
	MA_DNode_PenShine,
	MA_DNode_IsLink,
	MA_DNodeLI_Pen,
	MA_DNodeLI_Level,
	MA_DNodeI_IsMap,
	MA_DNodeI_VSpace,
	MA_DNodeI_Align,
	MA_DNodeI_UseMap,
	
	MM_DNodeI_SetCSMap,

	MA_DNodeI_IsFormButton,
	MA_DNodeI_AltText,
	MA_DNodeI_PenAlt,
	MA_DNodeI_ImgClient,
	MM_DNodeI_CalcNotReady,
	MA_DNodeT_State,

	MA_Fastlink_Number,
	MM_Fastlink_Update,

	MM_PostWin_Send,
	MA_PostWin_To,
	MA_PostWin_Subject,
	MA_PostWin_Contents,
	MA_PostWin_URL,

	MM_TriggerMe,

	MM_Downloadwin_Retry,
	MM_Downloadwin_CheckEntry,
	MM_Downloadwin_Enqueue,

	MM_MyV_BuildFramesArray,

	MM_ErrorWin_Close,
	MM_ErrorWin_PutError,

	MM_PrunecacheWin_Busy,
	MM_PrunecacheWin_Process,
	MM_PrunecacheWin_SetCurrentSize,
	MM_PrunecacheWin_SetTxt,
	MM_PrunecacheWin_SetRealTotalSize,

	// PluginWin */
	MM_PluginWin_Close,

	// JS Plugin array
	MA_JS_Plugin_Plugin,

	// Fonttest
	MM_Fonttest_SetFont,

	MM_HTMLView_RefreshMarking,
#if USE_SPLASHWIN
	MM_SplashWin_Update,
#endif

	MA_Formelement_Filesize,
	MA_Formelement_Filename,

	MA_JS_Form_Form, 				// Pointer back to struct form*
	MA_JS_Form_MyVClass,
	MM_JS_Form_BuildElements, 
	MM_JS_Form_HandleSubmit,
	MM_JS_Form_HandleReset,

	MM_PluginsWin_Close,
	MM_PluginsWin_SetVer,
	MM_PluginsWin_ClearInfo,
	MM_PluginsWin_ChangeGroup,
	MA_PluginsWin_Listview,
	MA_PluginsWin_Name,
	MA_PluginsWin_Copyright,
	MA_PluginsWin_InfoString,
	MA_PluginsWin_Info,

	MA_PluginsWinlist_Counter,

	MA_Historylist_Owner,

	MA_Layout_Formelement_Name,
	MA_Layout_Formelement_Data,

	MM_App_NewWindow,

	MM_Command_Select,
	MM_Command_ChangeMode,
	MM_Command_FillArgsList,
	MM_Command_Template,
	MM_Command_Remove_Backups,
	MA_Command_String,
	MA_Command_String_NoReset,
	MA_Command_Mode,

	MM_Clock_Tick,
	MM_Clock_ChangeDisplay,

	MM_Prefswin_Toolbar_BuildButtons,
	MM_Prefswin_Toolbar_CreateButton,
	MM_Prefswin_Toolbar_CreateBar,
	MM_Prefswin_Toolbar_AddButton,
	MM_Prefswin_Toolbar_MoveButton,
	MM_Prefswin_Toolbar_AddSpacer,
	MM_Prefswin_Toolbar_RemoveButton,
	MM_Prefswin_Toolbar_FixFields,
	MM_Prefswin_Toolbar_DisplayFields,

	MM_Prefswin_Languages_GetLocale,

	MM_Prefswin_Certs_Display,
	MM_Prefswin_Certs_ChangeState,

	MM_Prefswin_Fastlinks_SetStrings,
	MM_Prefswin_Fastlinks_Copy,

	MM_Prefswin_Fonts_SetFace,
	MM_Prefswin_Fonts_Test,
	MM_Prefswin_Fonts_Add,
	MM_Prefswin_Fonts_Del,
	MM_Prefswin_Fonts_Name,
	MM_Prefswin_Fonts_Update,
	MM_Prefswin_Fonts_Range,

	MM_Prefswin_Mailnews_SetMode,

	MM_Prefswin_Main_SelectChange,
	MM_Prefswin_Main_Close,
	MA_Prefswin_Main_Page,

	MM_Prefswin_ContextMenu_Add,
	MM_Prefswin_ContextMenu_AddSub,
	MM_Prefswin_ContextMenu_Remove,
	MM_Prefswin_ContextMenu_SetType,
	MM_Prefswin_ContextMenu_SaveTree,
	MM_Prefswin_ContextMenu_DisplayFields,
	MM_Prefswin_ContextMenu_FixFields,
	MM_Prefswin_ContextMenu_Renumber,
	MM_Prefswin_ContextMenu_ClearFields,
	MM_Prefswin_ContextMenu_Rename,
	MM_Prefswin_ContextMenu_LoadTree,

	MM_App_OpenPluginsWindow,
	MM_App_OpenErrorWindow,
	MM_App_OpenPrefsWindow,
	MM_App_OpenNetinfoWindow,

	MM_Prefswin_ContextMenu_ChangeEditGroup,

	MA_JS_Event_Type,
 	MA_JS_Event_Data,
 	MA_JS_Event_Height,
 	MA_JS_Event_LayerX,
	MA_JS_Event_LayerY,
	MA_JS_Event_Modifiers,
	MA_JS_Event_PageX,
	MA_JS_Event_PageY,
	MA_JS_Event_ScreenX,
	MA_JS_Event_ScreenY,
	MA_JS_Event_Target,
	MA_JS_Event_Which,
	MA_JS_Event_Width,

	MM_JS_FindByName,
	MM_JS_FindByID,

	MA_JS_RegExp_Source,
	MA_JS_RegExp_Global,
	MA_JS_RegExp_NoCase,
	MM_JS_RegExp_Match,
	MM_JS_RegExp_Search,
	MM_JS_RegExp_Replace,

#ifdef MBX
	MM_JS_CDPlayer_DiskChange,
#endif
#if USE_STB_NAV
	MM_STB_FullScreenTV,
	MM_STB_ToggleOnline,
	MM_STB_GotoURL,
#endif

	MM_Cross_SetDir,

	MA_SizeGroup_SizeX,
	MA_SizeGroup_SizeY,

#ifdef MBX	
	MA_Pipwindow_Position,
	MA_Pipwindow_Size,
	MM_Pipwindow_CalculatePosition,

	MA_App_Pipmode,
	MA_App_Pipwindow,
	MA_App_PipEnabled,
	MA_App_ScreenWidth,
#endif	

	MM_Scrollgroup_CheckScrollers,

	MA_dummyend /* do not add any tag/method below ! */
};

#ifndef DEPEND
#include "fastclasses.h"
#endif /* !DEPEND */

/* special values */
#define MV_Historylist_NoEntry ~0
#define MV_Historylist_Popdown -1

#define MV_App_CacheFlush_Mem 1
#define MV_App_CacheFlush_Disk 2

#define MV_Prefswin_Certs_Disable 0
#define MV_Prefswin_Certs_Enable 1
#define MV_Prefswin_Certs_Switch 2

/* App */
struct MP_DoAllWins {
	ULONG MethodID;
	ULONG args;
};

struct MP_DoActiveWin {
	ULONG MethodID;
	ULONG args;
};

struct MP_DoActivePanel {
	ULONG MethodID;
	ULONG args;
};

struct MP_DoLastActivePanel {
	ULONG MethodID;
	ULONG args;
};

struct MP_DoNumWin {
	ULONG MethodID;
	ULONG num;
	ULONG args;
};

struct MP_DoLastActiveWin {
	ULONG MethodID;
	ULONG args;
};

struct MP_App_DisplayError {
	ULONG MethodID;
	STRPTR url;
	STRPTR errorstring;
};

/* Window */
struct MP_Win_AddURL {
	ULONG MethodID;
	STRPTR url;
};

struct MP_Win_AddURLToHistory {
	ULONG MethodID;
	struct nstream *ns;
};

struct MP_Win_GetButtonNum {
	ULONG MethodID;
	ULONG num;
};

struct MP_Win_BuildButtonHistory {
	ULONG MethodID;
	ULONG butnum;
};

/* CookieBrowser */
struct MP_CookieBrowser_ListRemove {
	ULONG MethodID;
	int all;
};

struct MP_CookieBrowser_Add {
	ULONG MethodID;
	struct cookie *cookie;
};

struct MP_CookieBrowser_Delete {
	ULONG MethodID;
	struct cookie *cookie;
};

struct MP_CookieBrowser_StartSorting {
	ULONG MethodID;
	LONG column;
	int reverse;
};


/* Netinfo*/
struct MP_Netinfo_SetURL {
	ULONG MethodID;
	struct unode *un;
};

struct MP_Netinfo_Stop {
	ULONG MethodID;
	ULONG button_number;
};


/* AuthBrowser */
struct MP_AuthBrowser_ListRemove {
	ULONG MethodID;
	int all;
};

struct MP_AuthBrowser_Add {
	ULONG MethodID;
	struct authnode *auth;
};

struct MP_AuthBrowser_Delete {
	ULONG MethodID;
	struct authnode *auth;
};


/* Win */
struct MP_Win_OpenResumeWin {
	ULONG MethodID;
	STRPTR url;
	STRPTR referer;
	APTR informobj;
};

struct MP_Win_SetURL {
	ULONG MethodID;
	STRPTR url;
	int addurl;
	STRPTR referer;
	STRPTR target;
};


/* Historylist */

struct MP_Historylist_AddURL {
	ULONG MethodID;
	APTR url;
};

struct MP_Historylist_AddGlobal {
	ULONG MethodID;
	APTR url;
};

struct MP_Historylist_AddBF {
	ULONG MethodID;
	APTR url;
};

struct MP_Historylist_Exists {
	ULONG MethodID;
	STRPTR url;
	LONG   startentry;
};

struct MP_Historylist_InsertURLExtern {
	ULONG MethodID;
	struct history_mainpage *mp;
};

struct MP_Historylist_GotoEntry {
	ULONG MethodID;
	LONG pos;
};

struct MP_Historylist_Undo {
	ULONG MethodID;
	int abort;
};

struct MP_Historylist_StoreXY {
	ULONG MethodID;
	STRPTR url;
	ULONG x;
	ULONG y;
};

struct MP_Historylist_GetXY {
	ULONG MethodID;
	STRPTR url;
	ULONG *x;
	ULONG *y;
};

/* Javascript */
struct MP_JS_GetProperty {
	ULONG MethodID;

	STRPTR propname;
	int *typeptr;
	APTR *dataptr;
	int *datasize;
};

struct MP_JS_SetProperty {
	ULONG MethodID;

	STRPTR propname;
	APTR dataptr;
	int datasize;
	int type;
};

struct MP_JS_GetTypeData {
	ULONG MethodID;

	int *typeptr;
	APTR *dataptr;
	int *datasize;
};

struct MP_JS_SetData {
	ULONG MethodID;

	APTR dataptr;
	int datasize;
};

struct MP_JS_ToString {
	ULONG MethodID;

	STRPTR tobuffer;
	int *tosize;
	int radix;
};

struct MP_JS_CallMethod {
	ULONG MethodID;

	ULONG pid;
	int *typeptr;
	APTR *dataptr;
	int *datasize;
	int argcnt;
	APTR es; // expression stack
};

struct MP_JS_FreeCallMethod {
	ULONG MethodID;
	ULONG pid;
	APTR data;
	int datasize;
};

struct MP_JS_HasProperty {
	ULONG MethodID;

	char *propname;
};

struct MP_JS_ToBool {
	ULONG MethodID;

	int *boolptr;   
};

struct MP_JS_ToReal {
	ULONG MethodID;

	double *realptr;
};


/* DocinfoWin */
struct MP_DocinfoWin_SetContent {
	ULONG MethodID;

	int final_update;
};


struct MP_DocinfoWin_SetCipher {
	ULONG MethodID;

	char *cipher;
};

struct MP_HTMLView_ExecJS {
	ULONG MethodID;

	APTR refobj;
	STRPTR data;
	STRPTR resultbuffer;
	int resultbuffersize;
};

struct MP_JS_HandleEvent_MouseOver {
	ULONG MethodID;
	APTR win, container;
	ULONG MouseX, MouseY;
};

struct MP_JS_HandleEvent_MouseOut {
	ULONG MethodID;
	APTR win, container;
	ULONG MouseX, MouseY;
};

struct MP_Downloadwin_CheckEntry {
	ULONG MethodID;
	LONG entry;
};

/* ErrorWin */
struct MP_ErrorWin_PutError {
	ULONG MethodID;
	struct errorlog *el;
};

/* PrunecacheWin */
struct MP_PrunecacheWin_Busy {
	ULONG MethodID;
	int busy;
};

struct MP_PrunecacheWin_SetCurrentSize {
	ULONG MethodID;
	ULONG size;
	int all;
};

struct MP_PrunecacheWin_SetTxt {
	ULONG MethodID;
	STRPTR txt;
};

struct MP_PrunecacheWin_SetRealTotalSize {
	ULONG MethodID;
	ULONG size;
};

/* PluginWin */
struct MP_PluginWin_Close {
	ULONG MethodID;
	ULONG mode;
};


/* HTMLView */
struct MP_HTMLView_DoPrint {
	ULONG MethodID;
	APTR gaugeobj;
	int *abortflag;
	ULONG mode;
};

struct MP_HTMLView_RefreshMarking {
	ULONG MethodID;
	int copytoclip;
};

#if USE_SPLASHWIN
struct MP_SplashWin_Update {
	ULONG MethodID;
	STRPTR text;
};
#endif

struct MP_PluginsWin_Close {
	ULONG MethodID;
	int use;
};

struct MP_PluginsWin_SetVer {
	ULONG MethodID;
	ULONG ver;
	ULONG rev;
	ULONG api;
};

#if USE_PLUGINS
struct plugin;
#endif

struct MP_PluginsWin_ChangeGroup {
	ULONG MethodID;
#if USE_PLUGINS
	struct plugin *plugin;
#endif
};

struct MP_Print_CallPrefs {
	ULONG MethodID;
	STRPTR path;
};

struct MP_Gauge_Set {
	ULONG MethodID;
	LONG max;
	LONG current;
	STRPTR txt;
};

struct MP_Gauge_SetText {
	ULONG MethodID;
	STRPTR txt;
};

struct MP_Downloadwin_Retry {
	ULONG MethodID;
	LONG entry;
};

struct MP_Command_ChangeMode {
	ULONG MethodID;
	int mode;
};

struct MP_Command_SetString {
	ULONG MethodID;
	STRPTR txt;
};

struct MP_Prefswin_Toolbar_CreateButton {
	ULONG MethodID;
	int cnt;
};

struct MP_Prefswin_Toolbar_CreateBar {
	ULONG MethodID;
	int index;
};

struct MP_Prefswin_Toolbar_MoveButton {
	ULONG MethodID;
	int direction;
};

struct MP_HTMLWin_EnableButtonByFunc {
	ULONG MethodID;
	STRPTR function;
	int state;
};

struct MP_Clock_ChangeDisplay {
	ULONG MethodID;
	ULONG mode;
};

struct MP_App_CacheFlush {
	ULONG MethodID;
	int type;
};

struct MP_Prefswin_Certs_ChangeState {
	ULONG MethodID;
	int mode;
};

struct MP_Prefswin_Fonts_Test {
	ULONG MethodID;
	APTR obj;
};

struct MP_Prefswin_Main_Close {
	ULONG MethodID;
	int use;
};

struct MP_App_SetWinClose {
	ULONG MethodID;
	APTR obj;
};

struct MP_DoRexxWin {
	ULONG MethodID;
	ULONG win; /* WARNING: this argument is very important as the real method is stuffed into it. See app.c/DoRexxWin */
	ULONG args;
};

struct MP_Prefswin_ContextMenu_SetType {
	ULONG MethodID;
	ULONG type;
};

struct MP_Prefswin_ContextMenu_SaveTree {
	ULONG MethodID;
	ULONG type;
};

struct MP_Prefswin_ContextMenu_Renumber {
	ULONG MethodID;
	ULONG tn;
	ULONG depth;
};

struct MP_Prefswin_ContextMenu_Rename {
	ULONG MethodID;
	STRPTR txt;
};

struct MP_Prefswin_ContextMenu_LoadTree {
	ULONG MethodID;
	ULONG type;
};

struct MP_URLString_Change {
	ULONG MethodID;
	STRPTR newcontents;
};

struct MP_Prefswin_ContextMenu_ChangeEditGroup {
	ULONG MethodID;
	LONG tn;
};

struct MP_JS_FindByName {
	ULONG MethodID;
	char *name;
};

struct MP_JS_FindByID {
	ULONG MethodID;
	char *name;
};

struct MP_JS_ListProperties {
	ULONG MethodID;
	struct MinList *l;
	APTR pool;
};

struct MP_JS_SetGCMagic {
	ULONG MethodID;
	ULONG magic;
};

struct MP_JS_RegExp_Match {
	ULONG MethodID;
	char *sourcestr;
	struct MP_JS_CallMethod *msg;
};

struct MP_JS_RegExp_Replace {
	ULONG MethodID;
	char *sourcestr;
	struct MP_JS_CallMethod *msg;
	char *repstr;
};

struct MP_JS_RegExp_Search {
	ULONG MethodID;
	char *sourcestr;
	struct MP_JS_CallMethod *msg;
};

struct MP_Cross_SetDir {
	ULONG MethodID;
	int newdir;
};

struct MP_ImgDecode_HasInfo {
	ULONG MethodID;
	struct BitMap *bm;
	int img_x;
	int img_y;
	struct BitMap *maskbm;
	struct MinList *imagelist;
};

struct MP_ImgDecode_GotScanline {
	ULONG MethodID;
	int min_touched_y;
	int max_touched_y;
};

struct MP_NStream_GotInfo {
	ULONG MethodID;
	struct nstream *ns;
	ULONG ts;
	int docptr;
};

struct MP_NStream_GotData {
	ULONG MethodID;
	struct nstream *ns;
	ULONG ts;
	int docptr;
};

struct MP_NStream_Done {
	ULONG MethodID;
	struct nstream *ns;
	ULONG ts;
	int docptr;
};

struct MP_Downloadwin_Enqueue {
	ULONG MethodID;
	STRPTR url;
	STRPTR referer;
	ULONG flags;
};

struct MP_Downloadwin_SetButtons {
	ULONG MethodID;
	LONG entry;
};

struct MP_Downloadwin_Abort {
	ULONG MethodID;
	LONG entry;
};

struct MP_Downloadwin_UpdateEntry {
	ULONG MethodID;
	struct dlnode *dl;
};

struct MP_Scrollgroup_CheckScrollers {
	ULONG MethodID;
	ULONG hscroll;
	ULONG vscroll;
};

#endif /* VOYAGER_CLASSES_H */
