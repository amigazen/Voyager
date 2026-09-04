#ifndef CL_LISTS_H
#define CL_LISTS_H

/*
 * Dice-style list helpers. SAS/C has the same APIs in exec/lists.h
 * and clib/alib_protos.h (NewList).
 */

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifdef __SASC
#ifndef CLIB_ALIB_PROTOS_H
#include <clib/alib_protos.h>
#endif
#endif

#ifndef FIRSTNODE
#define FIRSTNODE(l) ((APTR)((struct List *)(l))->lh_Head)
#endif
#ifndef NEXTNODE
#define NEXTNODE(n) ((APTR)((struct Node *)(n))->ln_Succ)
#endif
#ifndef NEWLIST
#define NEWLIST(l) NewList((struct List *)(l))
#endif
#ifndef ADDHEAD
#define ADDHEAD(l,n) AddHead((struct List *)(l),(struct Node *)(n))
#endif
#ifndef ADDTAIL
#define ADDTAIL(l,n) AddTail((struct List *)(l),(struct Node *)(n))
#endif
#ifndef REMOVE
#define REMOVE(n) Remove((struct Node *)(n))
#endif

#endif
