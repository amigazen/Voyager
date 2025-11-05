#ifndef CLIB_WBSTART_PROTOS_H
#define CLIB_WBSTART_PROTOS_H

struct TagItem;


/*
 * wbstart_protos.h  V2.2
 *
 * Prototypes for wbstart.library functions
 *
 * (c) 1991-1996 Stefan Becker
 */

#ifndef LIBRARIES_WBSTART_H
#include <libraries/wbstart.h>
#endif

/* Library functions */
LONG WBStartTagList(struct TagItem *);

/* Varargs stubs */
LONG WBStartTags(Tag, ...);

#endif /* CLIB_WBSTART_PROTOS_H */
