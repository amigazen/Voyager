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


#ifndef VOYAGER_HTMLCLASSES_H
#define VOYAGER_HTMLCLASSES_H
/*
 * Method definitions for HTML classes
 *
 * $Id: htmlclasses.h,v 1.153 2003/10/28 00:05:32 zapek Exp $
*/

enum {
	MAC_dummy = (int)(MTAGBASE + 42768),

	MA_HTMLWin_IsTop,		// Top level window (uses window scrollers)
	MA_HTMLWin_Name,
	MA_HTMLWin_Scrolling,
	MA_HTMLWin_DrawFrame,
	MA_HTMLWin_BGPip,
	MA_HTMLWin_OrigBaseref,
	MA_HTMLWin_MUIWindow,		// MUI window object (if we are standalone)
	MA_HTMLWin_VirtualWidth,	// Virtual width (override layout target width)

	MM_HTMLWin_ToStandalone,
	MM_HTMLWin_ToDormant,
	MM_HTMLWin_SetupToolbar,
	MM_HTMLWin_SetupIcon,
	MM_HTMLWin_SetupFastlinks,
	MM_HTMLWin_SetupPages,
	MM_HTMLWin_UpdatePageTitles,
	MM_HTMLWin_SelectPage,
	MM_HTMLWin_TriggerSelectPage,
	MM_HTMLWin_SelectFastlink,
	MM_HTMLWin_URLAcknowledge,
	MM_HTMLWin_MakeWindowTitle,
	MM_HTMLWin_SetURL,
	MM_HTMLWin_SetURLCleanup, /* private */
	MM_HTMLWin_SetTxt,
	MM_HTMLWin_OpenDocInfoWin,
	MM_HTMLWin_ShowSource,
	MM_HTMLWin_SetDone,
	MM_HTMLWin_DoToolbutton,
	MM_HTMLWin_StopXfer,
	MM_HTMLWin_Forward,
	MM_HTMLWin_Backward,
	MM_HTMLWin_EnableButtonByFunc,
	MM_HTMLWin_BuildButtonHistory,
	MM_HTMLWin_GotoButtonHistory,
	MM_HTMLWin_SetTitle,
	MM_HTMLWin_SetTempStatus,
	MM_HTMLWin_ResetStatus,
	MM_HTMLWin_CheckDone,
	MM_HTMLWin_LoadInlineGfx,
	MM_HTMLWin_Reload,
	MM_HTMLWin_SetActive,
	MM_HTMLWin_FindByName,
	MM_HTMLWin_StartTimers,
	MM_HTMLWin_TriggerTimers,
	MM_HTMLWin_CleanupTimers,
	MM_HTMLWin_Close,
	MM_HTMLWin_UpdateScrollersClock,
#if USE_LO_PIP
	MM_HTMLWin_IncPIPNum,
	MM_HTMLWin_ResetPIPNum,
	MM_HTMLWin_HasPIPs,
	MM_HTMLWin_CheckPIPs,
#endif
	MM_HTMLWin_OpenFile,
	MM_HTMLWin_SaveHTML,
	MM_HTMLWin_SaveTEXT,
	MM_HTMLWin_Print,
	MM_HTMLWin_ShowFind,
	MM_HTMLWin_ExportTearoff,
	MM_HTMLWin_SetSmoothScroll,
	MM_HTMLWin_ToggleFullScreen,
	MM_HTMLWin_SetPointer,

	MM_HTMLWin_ExecuteJSFunc,

	// Window stuff for JS window.open()
	MA_HTMLWin_Toolbar,
	MA_HTMLWin_Fastlinks,
	MA_HTMLWin_InnerWidth,
	MA_HTMLWin_InnerHeight,
	MA_HTMLWin_Height,
	MA_HTMLWin_Width,
	MA_HTMLWin_ScreenX,
	MA_HTMLWin_ScreenY,
	MA_HTMLWin_Location,
	MA_HTMLWin_StatusBar,
	MA_HTMLWin_Resizable,
	MA_HTMLWin_Scrollbars,
	MA_HTMLWin_FullScreen,
	MA_HTMLWin_Opener,
	MA_HTMLWin_IsJsWin,
	MA_HTMLWin_Imgclient,

	// window dependency
	MA_HTMLWin_Dependent,
	MM_HTMLWin_AddDependent,
	MM_HTMLWin_RemDependent,

	MM_HTMLView_LoadInlineGfx,
	MM_HTMLView_ShowNStream,
	MM_HTMLView_NewImageSizes,
	MM_HTMLView_RefreshTrigger,
	MM_HTMLView_AbortLoad,

	MA_HTMLView_TopLevelWin,
	MA_HTMLView_HTMLWin,
	MA_HTMLView_InLayout,
	MA_HTMLView_IsFrameset,

	MA_Layout_Info,
	MA_Layout_Context,
	MA_Layout_Background,
	MA_Layout_BGColor,
	MA_Layout_BGImageURL,
	MA_Layout_OwningAnchor,
	MA_Layout_VAlign,
	MA_Layout_Align,
	MA_Layout_LineAlign,
	MA_Layout_TempLineAlign,
	MA_Layout_Width,
	MA_Layout_Height,
	MA_Layout_BorderDark,
	MA_Layout_BorderLight,
	MA_Layout_TopOffset,
	MA_Layout_MarginEnd_Left,
	MA_Layout_MarginEnd_Right,

	MA_Layout_MarginTop,
	MA_Layout_MarginLeft,
	MA_Layout_MarginBottom,
	MA_Layout_MarginRight,

	MM_Layout_RefreshAfterIncrementalDump,

	MA_Layout_Cell_Rowspan,
	MA_Layout_Cell_Colspan,
	MA_Layout_Cell_Border,
	MA_Layout_Cell_Row,
	MA_Layout_Cell_DefaultLineAlign,

	MA_Layout_Table_Cellpadding,
	MA_Layout_Table_Cellspacing,
	MA_Layout_Table_Border,

	MA_Layout_Image_Border,
	MA_Layout_Image_Src,
	MA_Layout_Image_LowSrc,
	MA_Layout_Image_Alttxt,
	MM_Layout_Image_LoadImage,
	MA_Layout_Image_Width,
	MA_Layout_Image_Height,
	MA_Layout_Image_Anchor,
	MA_Layout_Image_AnchorURL,
	MA_Layout_Image_Anchor_Visited,
	MA_Layout_Image_IsMap,
	MA_Layout_Image_UseMap,

	MA_Layout_Anchor_Title,
	MA_Layout_Anchor_Name,
	MA_Layout_Anchor_URL,
	MA_Layout_Anchor_Target,
	MA_Layout_Anchor_Visited,
	MA_Layout_Anchor_ALinkColor,
	MA_Layout_Anchor_Qualifier,
	MA_Layout_Anchor_TempURL,
	MM_Layout_Anchor_FindByName,

	MA_Layout_Div_Height,
	MA_Layout_Div_Align,

	MA_Layout_Div_ClearMargin_Left,
	MA_Layout_Div_ClearMargin_Right,

	MA_Layout_HR_Penspec1,
	MA_Layout_HR_Penspec2,
	MA_Layout_HR_NoShade,
	MA_Layout_HR_Size,

	MA_Layout_Form_URL,
	MA_Layout_Form_Target,
	MA_Layout_Form_Enctype,
	MA_Layout_Form_Method,

	MA_Layout_Group_UseMUIBackground,

	MA_Layout_Group_IsEmpty,

	MM_Layout_Group_AddText,
	MM_Layout_Group_AddObject,
	MM_Layout_Group_HighliteAnchor,
	MM_Layout_Group_ActiveAnchor,
	MM_Layout_CalcMinMax,
	MM_Layout_DoLayout,

	MM_Layout_CheckLayout,
	MM_Layout_CalcUnloadedObjects,

	MM_Layout_Backfill,

	MA_Layout_FormElement_Name,
	MA_Layout_FormElement_DOMID,
	MA_Layout_FormElement_Value,
	MA_Layout_FormElement_DefaultValue,
	MA_Layout_FormElement_Form,

	MA_Layout_FormButton_Type,

	MA_Layout_FormElement_ID,
	MA_Layout_FormElement_EID,

	MM_Layout_FormElement_Reset,
	MM_Layout_FormElement_Store,

	MM_Layout_FormElement_ReportValue,
	MM_Layout_Form_AttachValue,

	MA_Layout_Map_Name,
	MA_Layout_Map_DefaultURL,
	MA_Layout_Map_DefaultTarget,
	MM_Layout_Map_FindByName,
	MM_Layout_Map_FindArea,

	MA_Layout_Area_URL,
	MA_Layout_Area_Name,
	MA_Layout_Area_Target,
	MM_Layout_Area_SetCoords,
	MM_Layout_Area_DrawFrame,

	MA_Layout_FormText_Size,
	MA_Layout_FormText_Maxlength,

	MM_Layout_Form_Submit,
	MM_Layout_Form_Reset,

	MM_Layout_Image_StartAnim,
	MM_Layout_Image_StopAnim,
	MM_Layout_Image_DoAnim,

	MM_Layout_FormCycle_AddOption,
	MM_Layout_FormCycle_Finish,
	MA_Layout_FormCycle_Size,
	MA_Layout_FormCycle_Multiple,
	MM_Layout_FormCycle_SelectEntry,
	MM_Layout_FormCycle_TriggerChange,

	MA_Layout_FormTextarea_Rows,
	MA_Layout_FormTextarea_Cols,

	MA_Layout_Listitem_Size,
	MA_Layout_Listitem_Index,
	MA_Layout_Listitem_Shape,
	MA_Layout_Listitem_Color,
	MA_Layout_Listitem_Type,
	MA_Layout_Listitem_InList,
	MA_Layout_Listitem_Depth,

	MA_Layout_Frame_Horizspec,
	MA_Layout_Frame_Vertspec,
	MA_Layout_Frame_Border,
	MA_Layout_Frame_Frameborder,

	MM_Layout_FormRadio_SetActive,

	MM_Layout_Form_CheckSubmit,

	MM_HTMLView_GetPushedData,

	MA_Layout_AddMargin_Left,
	MA_Layout_AddMargin_Right,

	MM_HTMLWin_AddBM,

	MM_Layout_Image_NeedsRelayout,

	MA_Layout_Anchor_AccessKey,
	MM_Layout_Anchor_HandleAccessKey,

	MM_Layout_Frameset_BuildFramesArray,

	MM_HTMLWin_ExecuteEvent,
	MA_HTMLWin_CJSOL,

	MM_Layout_Anchor_HandleMouseEvent,
	MA_Layout_BGImageURLName,

	MM_HTMLWin_DoActiveFrame,
	MM_HTMLRexx_LoadBG,
	MM_HTMLRexx_ReloadURL,
	MM_HTMLRexx_SetURLFromObject,
	MM_HTMLRexx_OpenSourceView,
	MM_HTMLRexx_OpenDocInfo,
	MM_HTMLRexx_SaveURL,
	MM_HTMLRexx_SetClipFromObject,

	MA_Layout_Embed_Src,
	MA_Layout_Embed_Type,
	MM_Layout_Embed_AddArg,
	MM_Layout_Embed_Load,
	MM_Layout_Embed_SetupPlugin,
	MM_Layout_Embed_FindPlugin,

	MA_Layout_FormOption_Text,
	MA_Layout_FormOption_Value,
	MA_Layout_FormOption_Index,
	MA_Layout_FormOption_SelectObject,
	MA_Layout_FormOption_Selected,
	MA_Layout_FormOption_DefaultSelected
};

struct MP_Layout_Embed_AddArg {
	ULONG MethodID;
	char *name;
	char *value;
};

struct MP_HTMLWin_ExecuteEvent {
	ULONG MethodID;
	int type;
	int funcix;
	APTR this;
	ULONG tags;
};

struct MP_HTMLWin_SetPointer {
	ULONG MethodID;
	ULONG type;
};

struct MP_Layout_Anchor_HandleAccessKey {
	ULONG MethodID;
	int iecode;
};

struct MP_HTMLWin_SetURL {
	ULONG MethodID;
	STRPTR url;
	STRPTR referer;
	STRPTR target;
	ULONG flags;
};

struct MP_HTMLWin_SetURLCleanup {
	ULONG MethodID;
	STRPTR url;
	STRPTR referer;
	STRPTR target;
	ULONG flags;
};

/* Flags values */
#define MF_HTMLWin_Reload      ( 1L << 1 ) /* forces a reload */
#define MF_HTMLWin_AddURL      ( 1L << 2 ) /* adds the URL to the historylist */
#define MF_HTMLWin_FreeURL     ( 1L << 3 ) /* frees the memory when do */
#define MF_HTMLWin_FreeReferer ( 1L << 4 ) /* ditto */
#define MF_HTMLWin_FreeTarget  ( 1L << 5 ) /* ditto */

struct MP_HTMLWin_BuildButtonHistory {
	ULONG MethodID;
	ULONG butnum;
};

struct MP_Layout_Group_AddText {
	ULONG MethodID;
	STRPTR text;
	ULONG textlen;
	struct TextFont *font;
	UBYTE *fontarray;
	ULONG penspec;
	ULONG style;
	APTR *owning_anchor;
};

struct MP_Layout_Group_AddObject {
	ULONG MethodID;
	APTR obj;
	APTR *owning_anchor;
};

struct MP_Layout_Group_HighliteAnchor {
	ULONG MethodID;
	APTR anchor;
	ULONG state;
};

struct MP_Layout_Group_ActiveAnchor {
	ULONG MethodID;
	APTR anchor;
	ULONG state;
};

struct MP_Layout_DoLayout {
	ULONG MethodID;
	int suggested_width;
	int suggested_height;
	int outer_width;
};

struct MP_Layout_CalcMinMax {
	ULONG MethodID;
	int suggested_width;
	int suggested_height;
	int window_width;
};

struct MP_Layout_Area_SetCoords {
	ULONG MethodID;
	int type;
	char *coords;
};

struct MP_Layout_Map_FindByName {
	ULONG MethodID;
	char *name;
	APTR *objptr;
};

struct MP_Layout_Map_FindArea {
	ULONG MethodID;
	int x, y;
};

struct MP_Layout_Area_DrawFrame {
	ULONG MethodID;
	int x, y;
};

struct MP_Layout_Backfill {
	ULONG MethodID;
	ULONG left, top, right, bottom, xoffset, yoffset;
};

struct MP_Layout_FormCycle_AddOption {
	ULONG MethodID;
	char *name;
	char *value;
	int selected;
};

struct MP_Layout_FormRadio_SetActive {
	ULONG MethodID;
	APTR form;
	STRPTR name;
	APTR obj;
};

struct MP_Layout_Form_Submit {
	ULONG MethodID;
	APTR trigger;
};

struct MP_Layout_Form_Reset {
	ULONG MethodID;
	APTR trigger;
};

struct MP_Layout_FormElement_Reset {
	ULONG MethodID;
	APTR form;
};

struct MP_Layout_FormElement_ReportValue {
	ULONG MethodID;
	APTR whichform;
};

struct MP_Layout_Form_AttachValue {
	ULONG MethodID;
	STRPTR name;
	STRPTR value;
	int valuesize;
	STRPTR filename;
};

struct MP_HTMLWin_FindByName {
	ULONG MethodID;
	STRPTR name;
	APTR *obj;
};

struct MP_Layout_CalcUnloadedObjects {
	ULONG MethodID;
	int *cnt_img;
	int *cnt_frames;
	int *cnt_other;
};

struct MP_Layout_Image_NeedsRelayout {
	ULONG MethodID;
	int *cnt;
};

struct MP_Layout_Frameset_BuildFramesArray {
	ULONG MethodID;
	APTR farray;
};

struct MP_Layout_Anchor_HandleMouseEvent {
	ULONG MethodID;
	int type;
	int mousex, mousey;
};

struct MP_HTMLWin_DoActiveFrame {
	ULONG MethodID;
	APTR msg;
};

struct MP_HTMLRexx_SetURLFromObject {
	ULONG MethodID;
	int newwin;
	int reload;
};

struct MP_HTMLRexx_OpenDocInfo {
	ULONG MethodID;
	STRPTR url;
};

struct MP_HTMLRexx_SaveURL {
	ULONG MethodID;
	STRPTR url;
	STRPTR name;
	int ask;
};

struct MP_Layout_Anchor_FindByName {
	ULONG MethodID;
	STRPTR name;
	int *ypos;
};

struct MP_HTMLView_ShowNStream {
	ULONG MethodID;
	APTR ns;
};

struct MP_HTMLWin_SetSmoothScroll {
	ULONG MethodID;
	int smooth;
};

struct MP_HTMLWin_SelectPage {
	ULONG MethodID;
	APTR newobj;
};

struct MP_HTMLWin_TriggerSelectPage {
	ULONG MethodID;
	APTR newobj;
};

struct MP_HTMLWin_ExecuteJSFunc {
	ULONG MethodID;
	APTR funcobj;
};

struct MP_HTMLWin_AddDependent {
	ULONG MethodID;
	APTR obj;
};

struct MP_HTMLWin_RemDependent {
	ULONG MethodID;
	APTR obj;
};

struct MP_HTMLWin_HasPIPs {
	ULONG MethodID;
	int *flag;
};

APTR win_create( STRPTR name, STRPTR url, STRPTR referer, APTR owner, int embedded, int reload, int fullscreen );

/* This structure is shared by all layout objects */
struct layout_info {
	int xp, yp, xs, ys;
	int minwidth, maxwidth, defwidth;
	int minheight, maxheight, defheight;
	int space_top, space_left, space_right, space_bottom;
	UBYTE align; // inline, left margin, right margin, new row
	UBYTE valign; // alignment in line
	UBYTE flags;

	UBYTE pad;
};

#define LOF_NEW 1
#define LOF_BACKFILLING 2 // Backfills it's area

enum align {
	align_inline,
	align_left,
	align_right,
	align_newrow,
	align_newrowrow,
	align_newrowafter
};

enum valign {
	valign_baseline,
	valign_absmiddle,
	valign_absbottom,
	valign_bottom,
	valign_middle,
	valign_texttop,
	valign_top,
	valign_special
};

enum linealign {
	linealign_left,
	linealign_right,
	linealign_center,
	linealign_justified
};

enum formbuttontype {
	formbutton_button,
	formbutton_reset,
	formbutton_submit
};

enum imapareatypes {
	IMAPAREA_RECT,
	IMAPAREA_CIRCLE,
	IMAPAREA_POLY
};

enum htmlwin_drawframe {
	MV_HTMLWin_DrawFrame_Top = 1,
	MV_HTMLWin_DrawFrame_Left = 2,
	MV_HTMLWin_DrawFrame_Bottom = 4,
	MV_HTMLWin_DrawFrame_Right = 8
};

APTR getlotableclass( void );
APTR getloembedclass( void );
APTR getlogroupclass( void );
APTR getlodivclass( void );
APTR getlobrclass( void );
APTR getlogroupmcc( void );
APTR getloimageclass( void );
APTR getloanchorclass( void );
APTR getlodivclass( void );
APTR getlohrclass( void );
APTR getlodummyclass( void );
APTR getloformclass( void );
APTR getlobuttonclass( void );
APTR getloradioclass( void );
APTR getloformbuttonclass( void );
APTR getloformtextclass( void );
APTR getloformfileclass( void );
APTR getloformtextfieldclass( void );
APTR getloformhiddenclass( void );
APTR getloformcycleclass( void );
APTR getlocheckboxclass( void );
APTR getlomapclass( void );
APTR getloareaclass( void );
APTR getloliclass( void );
APTR getloframesetclass( void );
APTR getlopipclass( void );
APTR getlomarginclass( void );
APTR getloform_optionclass( void );

#endif /* VOYAGER_HTMLCLASSES_H */

