#ifndef VAPOR_MACROS_H
#define VAPOR_MACROS_H

/*
 * VaporWare Macros
 * ----------------
 *
 * � 1999-2000 by VaporWare CVS team <ibcvs@vapor.com>
 * � 2006 by Ambient Open Source Team
 * All rights reserved
 *
 *
 * $Id: vapor.h,v 1.1 2012/05/04 21:14:11 bigfoot Exp $
 *
*/

#ifdef __SASC
#include <clib/alib_protos.h>
#endif /* __SASC */

#define MAINTASK

/* MUI Macros */
#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif
#define FINDMENU(id) (APTR)DoMethod(menu,MUIM_FindUData,id)
#define findmenu FINDMENU // obsolete
#define DOSUPER DoSuperMethodA(cl,obj,(Msg)msg)

/* Dispatcher */

/*
 * Use DISPATCHERREF to give the dispatcher to the MUI_CreateCustomClass() call, eg:
 * mcc = MUI_CreateCustomClass(NULL, MUIC_Area, NULL, sizeof(struct Data), DISPATCHERREF);
 */
#ifdef __AROS__
#define DISPATCHERREF dispatch
#define DISPATCHERREF2(name) name##_dispatch
#else
#ifdef __MORPHOS__
#define DISPATCHERREF &GATE_dispatch
#define DISPATCHERREF2(name) &GATE_##name##_dispatch
#else
#define DISPATCHERREF dispatch
#define DISPATCHERREF2(name) name##_dispatch
#endif /* !__MORPHOS__ */
#endif

/*
 * Use BEGINMTABLE to start the description of a dispatcher (BEGINMTABLE2 if you need a name)
 */
#ifdef __GNUC__
#ifdef __AROS__
#define BEGINMTABLE static BOOPSI_DISPATCHER(IPTR, dispatch, cl, obj, msg){switch(msg->MethodID){
#else
#ifdef __MORPHOS__
#define BEGINMTABLE static ULONG dispatch(void); \
    static struct EmulLibEntry GATE_dispatch = \
    { \
        TRAP_LIB, 0, (void (*)(void)) dispatch \
    }; \
    static ULONG dispatch(void) \
    { \
        struct IClass *cl = (struct IClass *)REG_A0; \
        Msg msg = (Msg)REG_A1; \
        Object *obj = (Object *)REG_A2; \
        MAINTASK; \
        switch (msg->MethodID) \
        {

#else
#define BEGINMTABLE static ULONG dispatch( __reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, Msg msg)){switch(msg->MethodID){
#endif /* !__MORPHOS__ */
#endif
#else
#define BEGINMTABLE static ULONG __asm __saveds dispatch( register __a0 struct IClass *cl, register __a2  Object *obj, register __a1 Msg msg ){switch(msg->MethodID){
#endif /* __GNUC__ */

/*
 * Some common methods
 */
#define DECNEW case OM_NEW:return(handleOM_NEW(cl, obj, (struct opSet *)msg));
#ifdef __SASC
/* DECCONST should expand to function definition (like DEFNEW), not case statement */
/* We define it directly here to avoid forward reference issues */
#define DECCONST static ULONG ASM SAVEDS handleOM_NEW(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct opSet *msg))
#else
#define DECCONST DECNEW // obsolete
#endif
#define DECDISPOSE case OM_DISPOSE:return(handleOM_DISPOSE(cl, obj, (struct opSet *)msg));
#define DECDISP DECDISPOSE // obsolete
#define DECSET case OM_SET:return(handleOM_SET(cl, obj, (struct opSet *)msg));
#define DECGET case OM_GET:return(handleOM_GET(cl, obj, (struct opGet *)msg));
#define DECADDMEMBER case OM_ADDMEMBER:return(handleOM_ADDMEMBER(cl, obj, (APTR)msg));
#define DECREMMEMBER case OM_REMMEMBER:return(handleOM_REMMEMBER(cl, obj, (APTR)msg));
#define DECMMETHOD(methodid) case MUIM_##methodid:return(handleMUIM_##methodid(cl,obj,(struct MUIP_##methodid *)msg));
#ifdef __SASC
/* For SAS/C, DECMETHOD and DECSMETHOD can be used as function definitions */
/* Define them directly to avoid forward reference issues */
#define DECMETHOD(methodid,type) static ULONG ASM SAVEDS handleMM_##methodid(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, type *msg))
#define DECSMETHOD(methodid) static ULONG ASM SAVEDS handleMM_##methodid(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct MP_##methodid *msg))
#else
#define DECMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(type) msg));
#define DECSMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(struct MP_##methodid *)msg));
#endif
#define DECTMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg) msg));

/*
 * Use ENDMTABLE to end the description of a dispatcher
 */
#ifndef __AROS__
#define ENDMTABLE }return(DOSUPER);}
#else
#define ENDMTABLE }return(DOSUPER);}BOOPSI_DISPATCHER_END
#endif

/*
 * Dispatcher helper macros - for use inside BEGINMTABLE...ENDMTABLE
 * These handle the case where DEFMETHOD/DEFSMETHOD are called with one argument
 */
#ifdef __SASC
/* When DEFMETHOD is called with one arg in dispatcher, it should be a case statement */
/* Since C89 can't detect missing args, we provide dispatcher-specific macros */
/* These expand to case statements for use in BEGINMTABLE...ENDMTABLE */
#define DEFMETHOD_DISPATCH(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg)msg));
#define DEFSMETHOD_DISPATCH(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg)msg));
/* For compatibility with existing code using DEFMETHOD(methodid) in dispatchers, */
/* we'll try to make it work by redefining DEFMETHOD before BEGINMTABLE */
/* But that's complex. Better: update dispatcher entries to use DEFMETHOD_DISPATCH */
/* Or use a trick: redefine DEFMETHOD to handle both cases */
/* Actually, let's try: make DEFMETHOD expand to case when second arg is a special token */
/* But simplest: Make DEFMETHOD(methodid) work by using empty second arg trick */
/* If code uses DEFMETHOD(methodid, ), we can detect empty and use case */
/* But code uses DEFMETHOD(methodid) with no second arg at all */
/* Final solution: Right before BEGINMTABLE sections, redefine DEFMETHOD */
/* But that requires code changes. Let's provide the macro for now. */
#endif

/* Methods */

#ifdef __INLINED_METHODS
#define METHOD_INLINE inline
#else
#define METHOD_INLINE
#endif

/*
 * MUI method (ie. MUIM_List_InsertSingle)
 */
#ifdef __SASC
#define DEFMMETHOD(methodid) static ULONG ASM SAVEDS handleMUIM_##methodid(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct MUIP_##methodid *msg))
/* DEFMETHOD - when used with one arg in dispatcher, it's a case statement; with two args it's a function definition */
/* We need to handle both cases. For dispatcher usage with one arg, we'll use overload-style detection */
#define _DEFMETHOD_FUNC(methodid,type) static ULONG ASM SAVEDS handleMM_##methodid(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, type *msg))
#define _DEFMETHOD_CASE(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg)msg));
/* For dispatcher: DEFMETHOD(methodid) with one arg should be case statement */
/* For function def: DEFMETHOD(methodid, type) with two args should be function definition */
/* Since C89 doesn't support variadic macros well, we'll use a helper that detects empty second arg */
/* When second arg is provided and non-empty, use function definition; otherwise use case statement */
/* For SAS/C: DEFMETHOD with one arg (dispatcher) = case statement, with two args = function definition */
/* Since C89 can't detect missing args, we'll make DEFMETHOD work for dispatcher use (1 arg) */
/* For dispatcher: DEFMETHOD(methodid) should expand to case statement */
#define DEFMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg)msg));
/* For function definitions, use DECMETHOD(methodid, type) instead */
#define DEFTMETHOD(methodid) static ULONG ASM SAVEDS handleMM_##methodid(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, Msg msg))
#define _DEFSMETHOD_FUNC(name) static ULONG ASM SAVEDS handleMM_##name(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct MP_##name *msg))
#define _DEFSMETHOD_CASE(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg)msg));
/* For SAS/C: DEFSMETHOD with one arg (dispatcher) = case statement */
/* For function definitions, use DECSMETHOD(methodid) instead */
#define DEFSMETHOD(methodid) case MM_##methodid:return(handleMM_##methodid(cl,obj,(Msg)msg));
#define DEFSMETHOD_DISPATCH(methodid) _DEFSMETHOD_CASE(methodid)
/* For SAS/C: DEFNEW in dispatcher must be case statement, not function definition */
/* Function definitions use DECCONST instead */
#define DEFNEW case OM_NEW:return(handleOM_NEW(cl, obj, (struct opSet *)msg));
#define DEFCONST DEFNEW
#define DEFSET static ULONG ASM SAVEDS handleOM_SET(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct opSet *msg))
#define DEFGET static ULONG ASM SAVEDS handleOM_GET(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct opGet *msg))
#define DEFADDMEMBER static ULONG ASM SAVEDS handleOM_ADDMEMBER(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct opMember *msg))
#define DEFREMMEMBER static ULONG ASM SAVEDS handleOM_REMMEMBER(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct opMember *msg))
#define DEFDISPOSE static ULONG ASM SAVEDS handleOM_DISPOSE(__reg(a0, struct IClass *cl), __reg(a2, Object *obj), __reg(a1, struct opSet *msg))
#define DEFDEST DEFDISPOSE
#define DEFDISP DEFDISPOSE
#else
#define DEFMMETHOD(methodid) METHOD_INLINE static IPTR __attribute__ ((noinline)) handleMUIM_##methodid(struct IClass *cl,Object*obj,struct MUIP_##methodid *msg)
#define DEFMETHOD(methodid,type) METHOD_INLINE static IPTR __attribute__ ((noinline)) handleMM_##methodid(struct IClass *cl, Object *obj, type *msg)
#define DEFTMETHOD(methodid) METHOD_INLINE static IPTR __attribute__ ((noinline)) handleMM_##methodid(struct IClass *cl, Object *obj, Msg msg)
#define DEFSMETHOD(name) METHOD_INLINE static IPTR __attribute__ ((noinline)) handleMM_##name(struct IClass *cl,Object*obj,struct MP_##name *msg)
#define DEFNEW METHOD_INLINE static IPTR __attribute__ ((noinline)) handleOM_NEW(struct IClass *cl,Object*obj,struct opSet *msg)
#define DEFCONST DEFNEW
#define DEFSET METHOD_INLINE static IPTR __attribute__ ((noinline)) handleOM_SET(struct IClass *cl,Object*obj,struct opSet *msg)
#define DEFGET METHOD_INLINE static IPTR handleOM_GET(struct IClass *cl,Object*obj,struct opGet *msg)
#define DEFADDMEMBER METHOD_INLINE static IPTR __attribute__ ((noinline)) handleOM_ADDMEMBER(struct IClass *cl,Object*obj,struct opMember *msg)
#define DEFREMMEMBER METHOD_INLINE static IPTR __attribute__ ((noinline)) handleOM_REMMEMBER(struct IClass *cl,Object*obj,struct opMember *msg)
#define DEFDISPOSE METHOD_INLINE static IPTR __attribute__ ((noinline)) handleOM_DISPOSE( struct IClass *cl,Object*obj,struct opSet *msg)
#define DEFDEST DEFDISPOSE
#define DEFDISP DEFDISPOSE
#endif


/* Classes */

/*
 * Get the instance data
 */
#define GETDATA struct Data *data = (struct Data *) INST_DATA(cl, obj)
/* same but named one */
#define GETDATANAME(name) struct name##_Data *data = (struct name##_Data *) INST_DATA(cl,obj)

/*
 * Creates a subclass (constructor type)
 */
#define DECSUBCLASS(super,name,pri) struct MUI_CustomClass *classp##name;\
    CONSTRUCTOR_P(init##name,pri){\
        classp##name=MUI_CreateCustomClass(NULL,super,NULL,sizeof(struct Data),(APTR)DISPATCHERREF);\
        if(classp##name&&MUIMasterBase->lib_Version>=20)classp##name->mcc_Class->cl_ID=#name;\
        return(classp##name?0:-1);\
    }\
    DESTRUCTOR_P(init##name,pri){if(classp##name)MUI_DeleteCustomClass(classp##name);}\
    APTR get##name(void){return(classp##name->mcc_Class);}

/*
 * Creates a subclass of one of your own subclass (constructor type)
 */
#define DECSUBCLASSPTR(super,name,pri) struct MUI_CustomClass *classp##name;\
    CONSTRUCTOR_P(init##name,pri){\
        extern struct MUI_CustomClass *classp##super;\
        classp##name=MUI_CreateCustomClass(NULL,NULL,classp##super,sizeof(struct Data),(APTR)DISPATCHERREF);\
        if(classp##name&&MUIMasterBase->lib_Version>=20)classp##name->mcc_Class->cl_ID=#name;\
        return(classp##name?0:-1);\
    }\
    DESTRUCTOR_P(init##name,pri){if(classp##name)MUI_DeleteCustomClass(classp##name);}\
    IClass *get##name(void){return(classp##name->mcc_Class);}

/*
 * Creates a subclass (no constructor)
 */
#define DECSUBCLASS_NC(super,name) static struct MUI_CustomClass *mcc##name; \
    ULONG create_##name(void) \
    { \
        if (!(mcc##name = (struct MUI_CustomClass *)MUI_CreateCustomClass(NULL, super, NULL, sizeof(struct Data), (APTR)DISPATCHERREF))) \
            return (FALSE); \
            if (MUIMasterBase->lib_Version >= 20) \
                mcc##name->mcc_Class->cl_ID = (CONST_STRPTR) #name; \
        return (TRUE); \
    } \
    void delete_##name(void) \
    { \
        if (mcc##name) \
            MUI_DeleteCustomClass(mcc##name); \
    } \
    IClass *get##name(void) \
    { \
        return (mcc##name->mcc_Class); \
    } \
    APTR get##name##root(void) \
    { \
        return (mcc##name); \
    }

/*
 * Creates a subclass of one of your own subclass (no constructor)
 */
#define DECSUBCLASSPTR_NC(super,name) static struct MUI_CustomClass *mcc##name; \
    ULONG create_##name(void) \
    { \
        if (!(mcc##name = (struct MUI_CustomClass *)MUI_CreateCustomClass(NULL, NULL, (struct MUI_CustomClass *) get##super##root(), sizeof(struct Data), (APTR)DISPATCHERREF))) \
            return (FALSE); \
            if (MUIMasterBase->lib_Version >= 20) \
                mcc##name->mcc_Class->cl_ID = (CONST_STRPTR) #name; \
        return (TRUE); \
    } \
    void delete_##name(void) \
    { \
        if (mcc##name) \
            MUI_DeleteCustomClass(mcc##name); \
    } \
    IClass *get##name(void) \
    { \
        return (mcc##name->mcc_Class); \
    } \
    APTR get##name##root(void) \
    { \
        return (mcc##name); \
    }


/* get()/set() */

#define INITASTORE struct TagItem *tag, *tagstate = msg->ops_AttrList
#define BEGINASTORE while( tag = NextTagItem( &tagstate ) ) switch( tag->ti_Tag ) {
#define ENDASTORE }
#define ASTORE(t,x) case t: data->x = tag->ti_Data;break;
#define ASTOREP(t,x) case t: data->x = (APTR)tag->ti_Data;break;
#define STOREP(x) *msg->opg_Storage=(IPTR)(x)
#define STOREATTR(i,x) case i:*msg->opg_Storage=(IPTR)(x);return(TRUE);

/* Hooks */

#ifdef __AROS__

#define MUI_HOOK(n, y, z) \
    static IPTR n##_func(struct Hook * n, y, z); \
    static struct Hook n##_hook = {{0, 0}, (APTR)n##_func, NULL, NULL}; \
    static IPTR n##_func(struct Hook * n, y, z)

#else
#ifdef __MORPHOS__
#define __callback

/*
 * Hook, called with the following:
 * n - name of the hook (_hook appened at the end)
 * y - a2
 * z - a1
 *
 * return type is LONG
 */
#define MUI_HOOK(n, y, z) \
    static LONG n##_GATE(void); \
    static LONG n##_GATE2(struct Hook *h, y, z); \
    struct EmulLibEntry n = { \
    TRAP_LIB, 0, (void (*)(void))n##_GATE }; \
    static LONG n##_GATE(void) { \
    return (n##_GATE2((struct Hook *)REG_A0, (void *)REG_A2, (void *)REG_A1)); } \
    static struct Hook n##_hook = { { 0, 0}, (ULONG (*)(void))&n, (ULONG (*)(void))&n##_GATE2 , NULL }; \
    static LONG n##_GATE2(struct Hook *h, y, z)
#else
#define DEFHOOK(n) static struct Hook n##_hook={0,0,(HOOKFUNC)n##_func}

#define MUI_HOOK(n, y, z) \
    static LONG ASM SAVEDS n##_func(__reg(a0, struct Hook *h), __reg(a2, y), __reg(a1, z)); \
    static struct Hook n##_hook = { 0, 0, (HOOKFUNC)n##_func }; \
    static LONG ASM SAVEDS n##_func(__reg(a0, struct Hook *h), __reg(a2, y), __reg(a1, z))

#define __callback __asm __saveds
#endif /* !_MORPHOS__ */
#endif /* !__AROS__ */
#define _reg(x) register __##x


/* catmaker */
#define CATCOMP_NUMBERS
extern const char * const __stringtable[];
#ifdef __SASC
#define GSI(x) __stringtable[x]
#else
#define GSI(x) (char *)__stringtable[x]
#endif

/* Long word alignement (mainly used to get
 * FIB or DISK_INFO as auto variables)
 */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((IPTR)(a_##name+3) & ~3)


/* Exec list support macros */
#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

/* following 2 macros DON'T return NULL when the list is empty ! */
#ifndef FIRSTNODE
#define FIRSTNODE(l) ((APTR)((struct List*)l)->lh_Head)
#endif

#ifndef LASTNODE
#define LASTNODE(l) ((APTR)((struct List*)l)->lh_TailPred)
#endif

#ifndef NEXTNODE
#if !defined(__cplusplus)   
#define NEXTNODE(n) ((APTR)((struct Node*)n)->ln_Succ)
#else
#define NEXTNODE(n) ((APTR)((struct ::Node*)n)->ln_Succ)
#endif
#endif

#ifndef PREVNODE
#if !defined(__cplusplus)
#define PREVNODE(n) ((APTR)((struct Node*)n)->ln_Pred)
#else
#define PREVNODE(n) ((APTR)((struct ::Node*)n)->ln_Pred)
#endif
#endif

#ifndef FINDNAME
#define FINDNAME(l,n) ((APTR)FindName((struct List*)l,n))
#endif

#ifndef REMHEAD
#define REMHEAD(l) ((APTR)RemHead((struct List*)l))
#endif

#ifndef REMTAIL
#define REMTAIL(l) ((APTR)RemTail((struct List*)l))
#endif

#ifndef REMOVE
#define REMOVE(n) Remove((struct Node*)n)
#endif

#ifndef ADDHEAD
#define ADDHEAD(l,n) AddHead((struct List*)l,(struct Node*)n)
#endif

#ifndef ADDTAIL
#define ADDTAIL(l,n) AddTail((struct List*)l,(struct Node*)n)
#endif

#ifndef ENQUEUE
#define ENQUEUE(l,n) Enqueue((struct List*)l,(struct Node*)n)
#endif

#ifndef NEWLIST
#define NEWLIST(l) NewList((struct List*)l)
#endif

#ifndef ISLISTEMPTY
#define ISLISTEMPTY(l) IsListEmpty(((struct List*)l))
#endif

/*
 * Those functions are safe to use in an empty list, but do NOT remove list members
 * within them !
 */
#define ITERATELIST(node,list) for(node=FIRSTNODE(list);NEXTNODE(node);node=NEXTNODE(node))
#define TRAVERSELIST(node,list) ITERATELIST(node,list)

/*
 * These routines are like the ones above, except that it's safe to remove and/or
 * re-insert/free the node.
 */
#define ITERATELISTSAFE(node,nextnode,list) for(node=FIRSTNODE(list);(nextnode=NEXTNODE(node));node=nextnode)
#define TRAVERSELISTSAFE(node,nextnode,list) ITERATELISTSAFE(node,nextnode,list)

#endif /* VAPOR_MACROS_H */
