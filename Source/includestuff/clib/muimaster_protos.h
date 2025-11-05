#ifndef  CLIB_MUIMASTER_PROTOS_H
#define  CLIB_MUIMASTER_PROTOS_H

struct MUI_FrameSpec;
struct Screen;
struct BresenhamInfo;
struct InputEvent;
struct RDArgs;
struct Region;
struct ConfigItem;
struct Library;
struct MUI_RenderInfo;
struct MyIFFHandle;
struct MUI_ImageSpec;
struct MUI_PenSpec;
struct TagItem;
struct MUI_CustomClass;
struct MUI_Image;
struct MagicList;
struct LongRect;
struct IntuiMessage;
struct RastPort;
struct IClass;


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef POS
#ifndef LIBRARIES_COMMODITIES_H
#include <libraries/commodities.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************/
/* functions to be used in applications */
/****************************************/

Object *MUI_NewObjectA         (STRPTR classname,struct TagItem *tags);
#if !defined(USE_INLINE_STDARG)
Object *MUI_NewObject          (STRPTR classname,Tag tag1,...);
#endif
Object *MUI_MakeObjectA        (LONG type,ULONG *params);
#if !defined(USE_INLINE_STDARG)
Object *MUI_MakeObject         (LONG type,...);
#endif
VOID    MUI_DisposeObject      (Object *obj);
LONG    MUI_RequestA           (APTR app,APTR win,ULONG flags,STRPTR title,STRPTR gadgets,STRPTR format,APTR params);
#if !defined(USE_INLINE_STDARG)
LONG    MUI_Request            (APTR app,APTR win,ULONG flags,STRPTR title,STRPTR gadgets,STRPTR format,...);
#endif
LONG    MUI_Error              (VOID);
APTR    MUI_AllocAslRequest    (unsigned long reqType, struct TagItem *tagList);
#if !defined(USE_INLINE_STDARG)
APTR    MUI_AllocAslRequestTags(unsigned long reqType, Tag Tag1, ...);
#endif
VOID    MUI_FreeAslRequest     (APTR requester );
BOOL    MUI_AslRequest         (APTR requester, struct TagItem *tagList);
#if !defined(USE_INLINE_STDARG)
BOOL    MUI_AslRequestTags     (APTR requester, Tag Tag1, ...);
#endif

/******************************************/
/* functions to be used in custom classes */
/******************************************/

LONG                    MUI_SetError         (LONG num);
struct IClass *         MUI_GetClass         (STRPTR classname);
VOID                    MUI_FreeClass        (struct IClass *classptr);
VOID                    MUI_RequestIDCMP     (Object *obj,ULONG flags);
VOID                    MUI_RejectIDCMP      (Object *obj,ULONG flags);
VOID                    MUI_Redraw           (Object *obj,ULONG flags);
APTR                    MUI_AddClipping      (struct MUI_RenderInfo *mri,WORD left,WORD top,WORD width,WORD height);
VOID                    MUI_RemoveClipping   (struct MUI_RenderInfo *mri,APTR handle);
APTR                    MUI_AddClipRegion    (struct MUI_RenderInfo *mri,struct Region *r);
VOID                    MUI_RemoveClipRegion (struct MUI_RenderInfo *mri,APTR handle);
BOOL                    MUI_BeginRefresh     (struct MUI_RenderInfo *mri,ULONG flags);
VOID                    MUI_EndRefresh       (struct MUI_RenderInfo *mri,ULONG flags);
struct MUI_CustomClass *MUI_CreateCustomClass(struct Library *base,STRPTR supername,struct MUI_CustomClass *supermcc,int datasize,APTR dispatcher);
BOOL                    MUI_DeleteCustomClass(struct MUI_CustomClass *mcc);
LONG                    MUI_ObtainPen        (struct MUI_RenderInfo *mri,struct MUI_PenSpec *spec,ULONG flags);
VOID                    MUI_ReleasePen       (struct MUI_RenderInfo *mri,LONG pen);

/*************************************************************/
/* layout function, use only in custom layout callback hook! */
/*************************************************************/

BOOL MUI_Layout(Object *obj,LONG left,LONG top,LONG width,LONG height,ULONG flags);
BOOL MUI_LayoutObj(Object *obj,LONG left,LONG top,LONG width,LONG height,ULONG flags);

ULONG MUI_Show(Object *obj);
ULONG MUI_Hide(Object *obj);
VOID MUI_Offset(Object *obj,LONG addx,LONG addy);

BOOL MUI_LayoutObj(Object *obj,LONG left,LONG top,LONG width,LONG height,ULONG flags);
BOOL MUIP_TestDebug(ULONG flag);
struct MUI_AppInfo *MUIP_GetAppInfo(STRPTR basename);
#if !defined(USE_INLINE_STDARG)
APTR MUIP_listCreate(LONG tag, ...);
#endif
APTR MUIP_listCreateA(struct TagItem *tags);
#if !defined(USE_INLINE_STDARG)
LONG MUIP_listSetup(struct MagicList *ml, LONG tag, ...);
#endif
LONG MUIP_listSetupA(struct MagicList *ml, struct TagItem *tags);
VOID MUIP_listDelete(struct MagicList *ml);
LONG MUIP_listInsert(struct MagicList *ml,APTR ptr,LONG pos);
APTR MUIP_listRemove(struct MagicList *ml,LONG nr);
VOID MUIP_listClear(struct MagicList *ml);
LONG MUIP_listEntries(struct MagicList *ml);
APTR MUIP_listGetEntry(struct MagicList *ml,LONG nr);
APTR MUIP_listTraverse(struct MagicList *ml,LONG *id);
LONG MUIP_listFind(struct MagicList *ml,APTR entry);
VOID MUIP_listSort(struct MagicList *ml);
BOOL MUIP_listSelect(struct MagicList *ml,LONG nr,LONG type);
APTR MUIP_AllocVec(ULONG size);
VOID MUIP_FreeVec(APTR mem);
APTR MUIP_AllocMem(ULONG size);
APTR MUIP_AllocVecPooled(APTR pool,ULONG size,int dummy);
APTR MUIP_FreeVecPooled(APTR pool,APTR mem,int dummy);
VOID MUIP_FreeMem(APTR mem,ULONG size);
APTR MUIP_InternalBoopsiClass(LONG nr);
#ifndef POS
BOOL MUIP_MatchIX(struct InputEvent *ie,IX *ix);
#endif
ULONG MUIP_ExportNumber(Object *obj,APTR msg,ULONG attr);
ULONG MUIP_ExportString(Object *obj,APTR msg,ULONG attr);
ULONG MUIP_ImportNumber(Object *obj,APTR msg,ULONG attr);
ULONG MUIP_ImportString(Object *obj,APTR msg,ULONG attr);
struct RDArgs *MUIP_ReadArgsString(STRPTR  source,STRPTR  templ,ULONG *array);
VOID MUIP_FreeArgs(struct RDArgs *rda);
APTR MUIP_ObtainMaster(VOID);
VOID MUIP_ReleaseMaster(VOID);
BOOL MUIP_Bresenham(LONG space,LONG anz,struct BresenhamInfo *bi,LONG structsize);
APTR MUIP_GetPropClass(VOID);
VOID MUIP_FlushImages(struct Screen *screen);
APTR MUIP_GetNextApplication(ULONG *id);
LONG MUIP_IMsgToChar(struct IntuiMessage *imsg,ULONG dccode,ULONG dcquali);
VOID MUIP_CheckRefreshRedraw(Object *obj,ULONG flags,ULONG lock);
BOOL MUIP_BoxInRegion(struct Region *reg,LONG l,LONG t,LONG w,LONG h);
BOOL MUIP_RegionInBox(struct Region *reg,LONG l,LONG t,LONG w,LONG h);
VOID MUIP_BlitzObject(Object *obj,ULONG flags);
LONG MUIP_Recalc(Object *obj);
LONG MUIP_ProfileInit(VOID);
VOID MUIP_ProfileExit(VOID);
VOID MUIP_ProfileOff(VOID);
VOID MUIP_ProfileOn(VOID);
ULONG MUIP_Show(Object *obj);
ULONG MUIP_Hide(Object *obj);
VOID MUIP_Offset(Object *obj,LONG addx,LONG addy);
VOID MUIP_GetVirtualRect(Object *obj,struct LongRect *r);
BOOL MUIP_IsInVirtualRect(Object *obj,struct LongRect *lr);
struct MyIFFHandle *MUIP_OpenPrefsWrite(STRPTR name);
VOID MUIP_ClosePrefsWrite(struct MyIFFHandle *iff);
BOOL MUIP_ReadPrefs(STRPTR name,Object *configdata,Object *windowdata);
struct ConfigItem *MUIP_ReadConfigItem(LONG id);
BOOL MUIP_ConfigItemAvailable(struct ConfigItem *ci);
Object *MUIP_GetApplistObject(VOID);
Object *MUIP_GetCclistObject(VOID);
Object *MUIP_GetMasterConfigObject(VOID);
LONG MUIP_RegStatus(VOID);
struct RegUser *MUIP_RegUser(VOID);
Object *MUIP_RegObject(BOOL local);
BOOL MUIP_SpawnRegProgram(Object *obj);
VOID MUIP_SetTestMode(BOOL onoff);
struct MUI_Image       *MUIP_ObtainImage     (struct MUI_RenderInfo *mri,struct MUI_ImageSpec *spec,ULONG flags);
VOID                    MUIP_ReleaseImage    (struct MUI_RenderInfo *mri,struct MUI_Image *img);
struct ConfigItem *MUIP_GetConfigItemStructure(VOID);
ULONG MUIP_ShowClipped(Object *obj,struct LongRect *lr);
struct Library *MUIP_Get020Base(void);

struct MUI_RenderInfo *MUIP_GetRenderInfo(struct Screen *screen,Object *obj,ULONG flags);
VOID MUIP_FreeRenderInfo(struct MUI_RenderInfo *mri);
LONG MUIP_GetFrameAttr(struct MUI_RenderInfo *mri,struct MUI_FrameSpec *spec,ULONG attr);
VOID MUIP_DrawFrame(struct MUI_RenderInfo *mri,struct MUI_FrameSpec *spec,LONG left,LONG top,LONG width,LONG height,ULONG flags);
LONG MUIP_ScrollRaster(struct RastPort *rp,LONG dx,LONG dy,LONG left,LONG top,LONG right,LONG bottom);
BOOL MUIP_ParseVersionString(STRPTR ver,STRPTR buf,int bufsize,STRPTR *rcname,STRPTR *rcversion,STRPTR *rcdate,STRPTR *rccopyright);
struct MUI_Image *MUIP_AllocImage(struct MUI_RenderInfo *mri,struct MUI_ImageSpec *spec,struct TagItem *tags);
VOID MUIP_FreeImage(struct MUI_Image *img);
VOID MUIP_DrawImage(struct MUI_RenderInfo *mri,struct MUI_Image *img,LONG left,LONG top,LONG width,LONG height,struct TagItem *tags);
ULONG MUIP_GetImageAttr(struct MUI_Image *img,ULONG attr,struct MUI_RenderInfo *mri);
LONG MUIP_TestIntuiMsg(Object *obj,struct IntuiMessage *imsg);
#ifdef __cplusplus
}
#endif
#endif /* CLIB_MUIMASTER_PROTOS_H */
