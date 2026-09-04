#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/timer.h>
#include <proto/rexxsyslib.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/iffparse.h>
#include <proto/layers.h>
#include <exec/execbase.h>

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <proto/muimaster.h>
//#include <libraries/mui.h>
#include "mui.h"
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <devices/keyboard.h>
#include <dos/dostags.h>
#include <datatypes/soundclass.h>
#include <datatypes/pictureclass.h>

#include <mui/textinput_mcc.h>
#include <mui/nlist_mcc.h>
#include <mui/nlistview_mcc.h>

#include <cl/lists.h>

#define USE_BUILTIN_MATH
#include <time.h>
#include <string.h>
#include <constructor.h>
#include <ctype.h>
#include <stdlib.h>
#include <setjmp.h>

#define MAKE_ID(a,b,c,d)	\
	((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define findmenu(id) (APTR)DoMethod(menu,MUIM_FindUData,id)

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);

#define BEGINMTABLE static ULONG __asm __saveds dispatch( register __a0 struct IClass *cl, register __a2  Object *obj, register __a1 Msg msg ){switch(msg->MethodID){
#define DEFMETHOD(mid) case mid:return(handle##mid(cl,obj,(APTR)msg));
#define ENDMTABLE }return(DoSuperMethodA(cl,obj,msg));}
#define DECMETHOD(name,type) static ULONG handle##name( struct IClass *cl,Object*obj,type *msg)
#define DOSUPER DoSuperMethodA(cl,obj,(Msg)msg)

// MUI method
#define DECMMETHOD(name) static ULONG handleMUIM_##name( struct IClass *cl,Object*obj,struct MUIP_##name *msg)
#define DECCONST static ULONG handleOM_NEW( struct IClass *cl,Object*obj,struct opSet *msg )
#define DECSET static ULONG handleOM_SET( struct IClass *cl,Object*obj,struct opSet *msg )
#define DECGET static ULONG handleOM_GET( struct IClass *cl,Object*obj,struct opGet *msg )
#define DECDEST static ULONG handleOM_DISPOSE( struct IClass *cl,Object*obj,struct opSet *msg )

#define GETDATA struct Data *data = INST_DATA( cl, obj )

#define DECSUBCLASS(super,name,pri) static struct MUI_CustomClass *classp##name;\
	CONSTRUCTOR_P(init##name,pri){classp##name=MUI_CreateCustomClass(NULL,super,NULL,sizeof(struct Data),dispatch);return(classp##name?0:-1);}\
	DESTRUCTOR_P(init##name,pri){if(classp##name)MUI_DeleteCustomClass(classp##name);}\
	APTR get##name(void){return(classp##name->mcc_Class);}

#define DECSUBCLASSPTR(super,name,pri) static struct MUI_CustomClass *classp##name;\
	CONSTRUCTOR_P(init##name,pri){classp##name=MUI_CreateCustomClass(NULL,NULL,super,sizeof(struct Data),dispatch);return(classp##name?0:-1);}\
	DESTRUCTOR_P(init##name,pri){if(classp##name)MUI_DeleteCustomClass(classp##name);}\
	APTR get##name(void){return(classp##name->mcc_Class);}

#define INITASTORE struct TagItem *tag, *tagstate = msg->ops_AttrList
#define BEGINASTORE while( tag = NextTagItem( &tagstate ) ) switch( tag->ti_Tag ) {
#define ENDASTORE }
#define ASTORE(t,x) case t: data->x = tag->ti_Data;break;
#define ASTOREP(t,x) case t: data->x = (APTR)tag->ti_Data;break;

#define DEFHOOK(n) static struct Hook n##_hook={0,0,(HOOKFUNC)n##_func}

#define __callback __asm __saveds
#define _reg(x) register __##x

// standard globals

extern UWORD fmtfunc[];
extern char version[];
extern char copyright[];

//
// String table
//

#define CATCOMP_NUMBERS
extern char *__stringtable[];
#define GS(x) __stringtable[MSG_##x]
#define GSI(x) __stringtable[x]

#if 0
#define CreatePool(p1,p2,p3) VAT_CreatePool(p1,p2,p3)
#define DeletePool(p1) VAT_DeletePool(p1)
#define FreePooled(p1,p2,p3) VAT_FreePooled(p1,p2,p3)
#define AllocPooled(p1,p2) VAT_AllocPooled(p1,p2)
#endif
