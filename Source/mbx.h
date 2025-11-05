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


#ifndef VOYAGER_MBX_H
#define VOYAGER_MBX_H
/*
 * $Id: mbx.h,v 1.108 2003/04/25 19:13:55 zapek Exp $
 */

#define CPUMATH64	1
#define _DCC
#ifdef __DCC__
#endif

#ifdef __GNUC__

#ifndef ASM
#define ASM
#endif

#ifndef __far
#define __far
#endif

#ifndef SAVEDS
#define SAVEDS
#endif

#ifndef STDARGS
#define STDARGS
#endif

#ifndef __chip
#define __chip
#endif

#ifndef __reg
#define __reg(x,y) y /* MBX does not support __asm__ */
#endif

#endif __GNUC__

/*
 * DIAB (mcf only)
 */
#ifdef __DCC__

#ifndef ASM
#define ASM
#endif

#ifndef __far
#define __far
#endif

#ifndef SAVEDS
#define SAVEDS
#endif

#ifndef STDARGS
#define STDARGS
#endif

#ifndef __chip
#define __chip
#endif

#ifndef __reg
#define __reg(x,y) y
#endif

#define __inline INLINE
#define __FUNC__ "??"

#endif /* __DCC__ */

#include <system/system.h>
#include <system/sysdata.h>
#include <modules/taglists.h>
#include <modules/graphics/graphics.h>
#include <modules/inspiration/screens.h>
#include <modules/inspiration/inspiration.h>
#include <modules/locale/locale.h>
#define NO_ABSNETBASE_BASES 1
#include <modules/net/externs.h>

#include <system_lib_calls.h>
#include <locale_lib_calls.h>
#include <graphics_lib_calls.h>
#include <net_lib_calls.h>
#include <regions_lib_calls.h>
#include <inspiration_lib_calls.h>
#include <unicode_lib_calls.h>
#include <modules/unicode/unicode.h>

#define AMIGA_STCCPY 1
#include <modules/mui/features.h>
#include <modules/mui/mui.h>
#include <modules/mui/compat.h>
#include <system/types.h>

#define __REGD0(arg) arg
#define __REGD1(arg) arg
#define __REGD2(arg) arg
#define __REGD3(arg) arg
#define __REGD4(arg) arg
#define __REGD5(arg) arg
#define __REGD6(arg) arg
#define __REGD7(arg) arg
#define __REGA0(arg) arg
#define __REGA1(arg) arg
#define __REGA2(arg) arg
#define __REGA3(arg) arg
#define __REGA4(arg) arg
#define __REGA5(arg) arg
#define __REGA6(arg) arg
#define __REGA7(arg) arg
#define SAVEDS
#define ASM

#include "dos_compat.h"

#define   h_addr  h_addr_list[0]   /* address, for backward compatibility */

#define Node ListNode
#define MinNode TinyNode
#define MinList ListHead
#define List ListHead
#define mlh_Head lh_Head
#define ln_Pri ln_Priority

#define SignalSemaphore Semaphore

#define MsgPort MsgQueue
#define mp_SigBit mqu_SignalBit
#define mp_SigTask mqu_SignalProcess
#define mp_MsgList mqu_MsgList
#define mn_ReplyPort msg_ReplyQueue
#define mn_Node msg_Node

#define tc_Node pr_Node
#define pr_MsgPort pr_TaskMsgQueue

#define timerequest TimeRequest
#define tr_node tr_DrvIOReq
#define tr_time tr_Time

#define io_Message ior_Msg
#define io_Device ior_Driver
#define io_Command ior_Command

#define cp_x rp_PenPos.X
#define cp_y rp_PenPos.Y

#define UserPort ww_UserPort

#define MaxY Max.Y
#define MinY Min.Y
#define MaxX Max.X
#define MinX Min.X

#define Class im_Class
#define Code im_Code
#define MouseX im_MouseX
#define MouseY im_MouseY
#define Qualifier im_Qualifier

#define tv_secs tv_sec
#define tv_micro tv_usec

#define inet_addr Inet_Addr

//#define sprintf SPrintF
//#define kprintf KPrintF //TOFIX!! temporary, there's a kprintf() in libcaos.a

// undef CopyMem before redefining it
#undef CopyMem
#define CopyMem(src,dst,siz) memcpy(dst,src,siz)
#define ToLower(chr) ConvToLower(NULL,chr)
#define ToUpper(chr) ConvToUpper(NULL,chr)

#define MEMF_CHIP	0

extern UnicodeData_p UnicodeBase;

#define NT_REPLYMSG LNT_MSGREPLIED

typedef unsigned short USHORT;

//Below here is compatibility layer stuff for mbx - hopefully not too much
//and should hopefully get merged somewhere else sometime later

int ErrNo(void);
APTR v_NextObject(APTR cstate);
#define NextObject v_NextObject

#undef CreatePool
#define CreatePool(req,pud,thrsh) SYSCALL->sys_CreatePool(SysBase,req,0,pud,thrsh)
#undef InitSemaphore
#define InitSemaphore(sema) SYSCALL->sys_InitSemaphore(SysBase,sema,NULL,0)
#undef AttemptSemaphore
#define AttemptSemaphore(arg0) SYSCALL->sys_AttemptSemaphore(SysBase,arg0,FALSE)
#undef ObtainSemaphore
#define ObtainSemaphore(arg0) SYSCALL->sys_ObtainSemaphore(SysBase,arg0,FALSE)
#undef ObtainSemaphoreShared
#define ObtainSemaphoreShared(arg0) SYSCALL->sys_ObtainSemaphore(SysBase,arg0,TRUE)
#define CreateMsgPort() SYSCALL->sys_CreateMsgQueue(SysBase,"voyager-port",0,TRUE)
#define DeleteMsgPort(arg0) SYSCALL->sys_DeleteMsgQueue(SysBase,arg0,FALSE)
#define CreatePort(arg0,arg1) SYSCALL->sys_CreateMsgQueue(SysBase,arg0,arg1,TRUE)
#define DeletePort(arg0) SYSCALL->sys_DeleteMsgQueue(SysBase,arg0,TRUE)
#define WaitPort WaitMsgQueue
#define FindTask FindProcess
#define GetBitMapAttr GetBitMapAttrs

#define NewList InitListHead
#undef RemHead
#define RemHead RemNodeHead
#undef RemTail
#define RemTail RemNodeTail
#define AddHead AddNodeHead
#define AddTail AddNodeTail
#define Enqueue AddNodeSorted
#undef Remove
#define Remove(arg0) RemNode(arg0)
#define RectFill(arg0,arg1,arg2,arg3,arg4) FillRectangular(arg0,arg1,arg2,(1+(arg3))-(arg1),(1+(arg4))-(arg2),0)

#undef InitRastPort
#define InitRastPort(arg0)  GFXCALL->gfx_InitRastPort(GraphicsBase,arg0,TAG_DONE)

#undef DrawEllipse
#define DrawEllipse(arg0,arg1,arg2,arg3,arg4) GFXCALL->gfx_DrawEllipse(GraphicsBase,arg0,arg1,arg2,arg3,arg4,0)

#define getclone( arg0, arg1 ) arg0
#define markclonemodified( arg0 )
#define DoHookClipRects( arg0, arg1, arg2 )

//Pretend like we've included these header files - amongst other stuff, MUI
//tries to include them if we don't.

#define EXEC_LISTS_H		1
#define EXEC_TYPES_H		1
#define INTUITION_CLASSES_H	1
#define INTUITION_SCREENS_H	1
#define CLIB_INTUITION_PROTOS_H	1
#define UTILITY_TAGITEM_H	1
#define LIBRARIES_COMMODITIES_H	1
#define MBX_TYPES		1

#define NO_PRAGMA		1	//used by vat.h and imgstub.h

//Now that we've got all the compatibility craporama sorted out, do the
//global includes/defines we want.

#include <modules/mui/mui.h>
#include <modules/mui/boopsi_spx.h>
#include <mui_lib_calls.h>
#include "mcp_lib_calls.h"
#include "modules/net/establish.h"
#include "env_lib_calls.h"
#include <mbxgui_lib_calls.h>

#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

extern SYSBASE;
extern LOCALEBASE;
extern TAGLISTSBASE;
extern GRAPHICSBASE;
extern MUIBASE;
extern NETBASE;
extern REGIONSBASE;
extern INSPIRATIONBASE;
extern ENVBASE;
extern MBXGUIBASE;

extern BitMap_s mbxbackbmp;
extern BitMap_s mbxfindbmp;
extern BitMap_s mbxfrwdbmp;
extern BitMap_s mbxhomebmp;
extern BitMap_s mbxloadbmp;
extern BitMap_s mbxprntbmp;
extern BitMap_s mbxreldbmp;
extern BitMap_s mbxstopbmp;

#define MUIMasterBase ((Module_p)MUIBase)
#define lib_Version mod_Version
#define lib_Revision mod_Revision

#undef MUI_NewObject
#undef NewObject
#define MUI_NewObject wixMUI_NewObject
#define NewObject wixNewObject
OPTR wixMUI_NewObject( STRPTR classname, TagItemID tag1, ...);
OPTR wixNewObject( STRPTR classname, APTR dummy, TagItemID tag1, ...);

#include <modules/mui/textinput_mcc.h>
// On CaOS, Textinput classes are embedded in MUI and thus named .mui instead of .mcc
#undef MUIC_Textinput
#undef MUIC_Textinputscroll
#define MUIC_Textinput "Textinput.mui"
#define MUIC_Textinputscroll "Textinputscroll.mui"

#if USE_SPEEDBAR
#include "speedbar/SpeedBar_mcc.h"
#include "speedbar/SpeedButton/SpeedButton_mcc.h"
#endif /* USE_SPEEDBAR */
#include "imgstub.h"

/* Below were originally taken from include/macros/vapor.h */
#ifdef VAPOR_H_BROKEN
#define DEFNEW case OM_NEW:return(handleOM_NEW(cl, obj, (APTR)msg));
#define DEFCONST DEFNEW // obsolete
#define DEFDISPOSE case OM_DISPOSE:return(handleOM_DISPOSE(cl, obj, (APTR)msg));
#define DEFDISP DEFDISPOSE // obsolete
#define DEFSET case OM_SET:return(handleOM_SET(cl, obj, (APTR)msg));
#define DEFGET case OM_GET:return(handleOM_GET(cl, obj, (APTR)msg));
#define DEFMMETHOD(methodid) case MUIM_##methodid:return(handleMUIM_##methodid(cl,obj,(APTR)msg));
#define DEFMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(APTR)msg));
#define DEFSMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(APTR)msg));
#define DEFTMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(APTR)msg));
#else
#define DECNEW case OM_NEW:return(handleOM_NEW(cl, obj, (APTR)msg));
#define DECCONST DECNEW // obsolete
#define DECDISPOSE case OM_DISPOSE:return(handleOM_DISPOSE(cl, obj, (APTR)msg));
#define DECDISP DECDISPOSE // obsolete
#define DECSET case OM_SET:return(handleOM_SET(cl, obj, (APTR)msg));
#define DECGET case OM_GET:return(handleOM_GET(cl, obj, (APTR)msg));
#define DECMMETHOD(methodid) case MUIM_##methodid:return(handleMUIM_##methodid(cl,obj,(APTR)msg));
#define DECMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(APTR)msg));
#define DECSMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(APTR)msg));
#define DECTMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(APTR)msg));
#endif /* !VAPOR_H_BROKEN */

#define BEGINMTABLE static ULONG dispatch( __reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, Msg msg)){switch(msg->MethodID){
#define BEGINMTABLE2(name) static ULONG name##_Dispatcher( __reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, Msg msg)){switch(msg->MethodID){

/* Methods */

/*
 * MUI method (ie. MUIM_List_InsertSingle)
 */
#ifdef VAPOR_H_BROKEN
#define DECMMETHOD(methodid) static ULONG handleMUIM_##methodid(struct IClass *cl,Object*obj,struct MUIP_##methodid *msg)
#else
#define DEFMMETHOD(methodid) static ULONG handleMUIM_##methodid(struct IClass *cl,Object*obj,struct MUIP_##methodid *msg)
#endif /* !VAPOR_H_BROKEN */

/*
 * Custom method with ONE argument only (no msg[n] please)
 */
#ifdef VAPOR_H_BROKEN
#define DECMETHOD(methodid,type) static ULONG handleMM_##methodid(struct IClass *cl, Object *obj, type *msg)
#else
#define DEFMETHOD(methodid,type) static ULONG handleMM_##methodid(struct IClass *cl, Object *obj, type *msg)
#endif /* !VAPOR_H_BROKEN */

/*
 * Custom method with NO real arguments (Msg still passed for DSM etc.)
 */
#ifdef VAPOR_H_BROKEN
#define DECTMETHOD(methodid) static ULONG handleMM_##methodid(struct IClass *cl, Object *obj, Msg msg)
#else
#define DEFTMETHOD(methodid) static ULONG handleMM_##methodid(struct IClass *cl, Object *obj, Msg msg)
#endif /* !VAPOR_H_BROKEN */

/*
 * Custom structured method
 */
#ifdef VAPOR_H_BROKEN
#define DECSMETHOD(name) static ULONG handleMM_##name(struct IClass *cl,Object*obj,struct MP_##name *msg)
#else
#define DEFSMETHOD(name) static ULONG handleMM_##name(struct IClass *cl,Object*obj,struct MP_##name *msg)
#endif /* !VAPOR_H_BROKEN */

/*
 * OM_NEW method (construct)
 */
#ifdef VAPOR_H_BROKEN
#define DECNEW static ULONG handleOM_NEW(struct IClass *cl,Object*obj,struct opSet *msg)
#define DECCONST DECNEW
#else
#define DEFNEW static ULONG handleOM_NEW(struct IClass *cl,Object*obj,struct opSet *msg)
#define DEFCONST DEFNEW
#endif /* !VAPOR_H_BROKEN */

/*
 * OM_SET method
 */
#ifdef VAPOR_H_BROKEN
#define DECSET static ULONG handleOM_SET(struct IClass *cl,Object*obj,struct opSet *msg)
#else
#define DEFSET static ULONG handleOM_SET(struct IClass *cl,Object*obj,struct opSet *msg)
#endif /* !VAPOR_H_BROKEN */

/*
 * OM_GET method
 */
#ifdef VAPOR_H_BROKEN
#define DECGET static ULONG handleOM_GET(struct IClass *cl,Object*obj,struct opGet *msg)
#else
#define DEFGET static ULONG handleOM_GET(struct IClass *cl,Object*obj,struct opGet *msg)
#endif /* !VAPOR_H_BROKEN */

/*
 * OM_DISPOSE method (destruct)
 */
#ifdef VAPOR_H_BROKEN
#define DECDISPOSE static ULONG handleOM_DISPOSE( struct IClass *cl,Object*obj,struct opSet *msg)
#define DECDEST DECDISPOSE
#define DECDISP DECDISPOSE
#else
#define DEFDISPOSE static ULONG handleOM_DISPOSE( struct IClass *cl,Object*obj,struct opSet *msg)
#define DEFDEST DEFDISPOSE
#define DEFDISP DEFDISPOSE
#endif /* !VAPOR_H_BROKEN */


#define GETDATA struct Data *data = INST_DATA(cl, obj)

#define FINDMENU(id) (APTR)DoMethod(menu,MUIM_FindUData,id)
#define findmenu FINDMENU // obsolete
#define DOSUPER DoSuperMethodA(cl,obj,(Msg)msg)

#define INITASTORE struct TagItem *tag, *tagstate = msg->ops_AttrList
#define BEGINASTORE while( tag = NextTagItem( &tagstate ) ) switch( (int)tag->ti_Tag ) {
#define ENDASTORE }
#define ASTORE(t,x) case (int)t: data->x = tag->ti_Data;break;

#define DECDISP DECDISPOSE

#define ENDMTABLE }return(DOSUPER);}
#define DISPATCHERREF dispatch
#define DISPATCHERREF2(name) name##_dispatch

#define FIRSTNODE(l) ((APTR)((struct List*)l)->lh_Head)
#define LASTNODE(l) ((APTR)((struct List*)l)->lh_TailPred)

#define NEXTNODE(n) ((APTR)((struct Node*)n)->ln_Succ)
#define PREVNODE(n) ((APTR)((struct Node*)n)->ln_Pred)

#define FINDNAME(l,n) ((APTR)FindNode((struct List*)l,n))

#define REMHEAD(l) ((APTR)RemHead((struct List*)l))
#define REMTAIL(l) ((APTR)RemTail((struct List*)l))
#define REMOVE(n) Remove((struct Node*)n)

#define ADDHEAD(l,n) AddHead((struct List*)l,(struct Node*)n)
#define ADDTAIL(l,n) AddTail((struct List*)l,(struct Node*)n)
#define ENQUEUE(l,n) Enqueue((struct List*)l,(struct Node*)n)

#define STOREATTR(i,x) case (int)i:*msg->opg_Storage=(ULONG)(x);return(TRUE);

#define NEWLIST(l) NewList((struct List*)l)
#define ISLISTEMPTY(l) IsListEmpty(((struct List*)l))
#define ITERATELIST(node,list) for(node=FIRSTNODE(list);NEXTNODE(node);node=NEXTNODE(node))

#define DEFHOOK(n) static struct Hook n##_hook={{0,0},(HOOKFUNC)n##_func}
#define MUI_HOOK(n, y, z) \
	static LONG ASM SAVEDS n##_func(__reg(a0, struct Hook *h), __reg(a2, y), __reg(a1, z)); \
	static struct Hook n##_hook = { { 0, 0 }, (HOOKFUNC)n##_func }; \
	static LONG ASM SAVEDS n##_func(__reg(a0, struct Hook *h), __reg(a2, y), __reg(a1, z))

// standard globals

extern UWORD fmtfunc[];
extern char version[];
extern char lversion[];
extern char copyright[];

#define max(x,y) ((x>y)?x:y)
#define min(x,y) ((x>y)?y:x)

#define PRECISION_EXACT 0

//
// String table
//
#define CATCOMP_NUMBERS
extern const char * const __stringtable[];
#define GS(x) (char*)__stringtable[MSG_##x]
#define GSI(x) (char*)__stringtable[x]

// Define the v_ types

#define v_Task Process

#define clock() 0

typedef UBYTE *AMIGAPLANEPTR;

struct AmigaBitMap
{
	UWORD   BytesPerRow;
	UWORD   Rows;
	UBYTE   Flags;
	UBYTE   Depth;
	UWORD   pad;
	AMIGAPLANEPTR Planes[8];
};

extern unsigned char mbximage[];
extern unsigned int mbximagelen;

/* font wrappers from mbx.c */
extern int mbxtextfit( STRPTR text, UWORD textlen, UWORD pixelsize, TextFont_p withfont );
extern int mbxtextlen( STRPTR text, UWORD textlen, TextFont_p withfont );
extern void mbxText( struct RastPort *rp, char *txt, int len );

/* custom 64-bit math stuff */
ASM	LONG q_div(QWORD *dividend, LONG divisor);
ASM LONG q_mod(QWORD *dividend, LONG divisor);
ASM void q_add(QWORD *a, QWORD *b, QWORD *res);
ASM void q_sub(QWORD *a, QWORD *b, QWORD *res);

#endif /* VOYAGER_MBX_H */
