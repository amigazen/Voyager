#ifndef  CLIB_MUIGFX_PROTOS_H
#define  CLIB_MUIGFX_PROTOS_H

struct ViewPort;
struct TextAttr;
struct Screen;
struct BitMapHeader;
struct Frame;
struct TagItem;
struct MUI_Image;
struct CGXHook;
struct MUI_MinMax;
struct Rectangle;
struct MUI_RenderInfo;
struct MUI_ScreenInfo;
struct MUI_PenSpec;
struct TextFont;
struct ColorMap;
struct BitMap;
struct RastPort;


#ifdef AMIGAOS

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#endif


/* BitMap functions */

struct BitMap *MUIG_AllocBitMap(LONG width,LONG height,LONG depth,LONG flags,struct BitMap *frnd);
VOID           MUIG_FreeBitMap(struct BitMap *bm);
ULONG          MUIG_GetBitMapAttr(struct BitMap *bm,ULONG attr);
struct BitMap *MUIG_CloneBitMap(struct BitMap *src,struct BitMap *frnd);
struct BitMap *MUIG_RemapBitMap(struct BitMap *src,int destdepth,UBYTE *pentable);
struct BitMap *MUIG_MaskBitMap(struct BitMap *src,LONG width);
struct BitMap *MUIG_GetMBR(struct MUI_RenderInfo *mri,struct BitMapHeader *bmhd,UBYTE *body,struct BitMap **mask);

/* Drawing Functions */

VOID           MUIG_BltBitMapTiled(struct BitMap *Src,LONG SrcOffsetX,LONG SrcOffsetY,LONG SrcSizeX,LONG SrcSizeY,struct BitMap *Dst,struct Rectangle *DstBounds);
VOID           MUIG_DrawDisablePattern(struct MUI_RenderInfo *mri,LONG l,LONG t,LONG w,LONG h);

/* Pen Functions */

VOID           MUIG_DummyWasObtainPen(struct ColorMap *,LONG index,ULONG r,ULONG g,ULONG b,ULONG flags);
LONG           MUIG_ObtainBestPenA(struct ColorMap *cm,ULONG r,ULONG g,ULONG b,struct TagItem *taglist);
VOID           MUIG_ReleasePen(struct ColorMap *cm,LONG pen);
LONG           MUIG_ObtainMUIPen(struct MUI_ScreenInfo *si,struct MUI_PenSpec *spec);
VOID           MUIG_ReleaseMUIPen(struct MUI_ScreenInfo *si,LONG muipen);

/* Image Functions */

VOID              MUIG_DrawImage(struct MUI_RenderInfo *mri,struct MUI_Image *img,LONG l,LONG t,LONG w,LONG h,ULONG flags,ULONG flags2);
struct MUI_Image *MUIG_GetImage(struct MUI_RenderInfo *mri,char *spec,ULONG flags);
VOID              MUIG_ReleaseImage(struct MUI_Image *img);
ULONG             MUIG_GetImageAttr(struct MUI_Image *img,ULONG attr,struct MUI_RenderInfo *mri);

/* Misc Functions */

struct BitMap *MUIG_AllocBitMapFromBody(struct BitMapHeader *bmhd,UBYTE *body);
struct MUI_ScreenInfo *MUIG_GetScreenInfo(struct Screen *screen);
VOID MUIG_FreeScreenInfo(struct MUI_ScreenInfo *si);
struct TextFont *MUIG_OpenFont(STRPTR spec,struct TextAttr *def);
struct MUI_RenderInfo *MUIG_GetRenderInfo(struct Screen *screen,Object *obj);
VOID MUIG_FreeRenderInfo(struct MUI_RenderInfo *mri);
VOID MUIG_DrawFrame(struct MUI_RenderInfo *mri,struct Frame *Frame,LONG l,LONG t,LONG w,LONG h,LONG state);
struct Frame *MUIG_GetFrame(LONG nr,BOOL thin);
struct MUI_RenderInfo *MUIG_AllocMRIBuffer(Object *obj,LONG width,LONG height,ULONG flags);
VOID MUIG_FreeMRIBuffer(struct MUI_RenderInfo *mri);
LONG MUIG_TextDim(struct MUI_RenderInfo *mri,struct TextFont *miscfont,char *source,LONG len,char *preparse);
VOID MUIG_TextPrint(struct MUI_RenderInfo *mri,LONG l,LONG t,LONG w,LONG h,char *source,LONG len,char *preparse,LONG underline);
LONG MUIG_TextOffset(struct MUI_RenderInfo *mri,LONG x,LONG y,LONG w,LONG h,char *source,LONG len,char *preparse);
BOOL MUIG_StringToRGB(char *str,ULONG *rgb);

VOID MUIG_SetRGB32(struct ViewPort *vp,ULONG pen,ULONG *rgb);
VOID MUIG_GetRGB32(struct ColorMap *cm,ULONG pen,ULONG *rgb);

BOOL MUIG_ScrollDamage(struct RastPort *rp,WORD dx,WORD dy,WORD left,WORD top,WORD right,WORD bottom);
LONG MUIG_ScrollRaster(struct RastPort *rp,WORD dx,WORD dy,WORD left,WORD top,WORD right,WORD bottom);

void MUIG_DoColorImposeHook(struct CGXHook *hook,struct RastPort *rp,LONG minx,LONG miny,LONG maxx,LONG maxy,ULONG color);
void MUIG_DoBitMapImposeHook(struct CGXHook *hook,struct BitMap *src1,struct BitMap *src2,struct RastPort *rp,ULONG src1x,ULONG src1y,ULONG src2x,ULONG src2y,LONG dstx,LONG dsty,ULONG width,ULONG height);
void MUIG_DoBalanceHook(struct CGXHook *hook,struct RastPort *rp,LONG minx,LONG miny,LONG maxx,LONG maxy,LONG radd,LONG gadd,LONG badd);

VOID MUIG_GetImageSize(struct MUI_Image *img,struct MUI_MinMax *mm);

#endif /* CLIB_MUIGFX_PROTOS_H */
