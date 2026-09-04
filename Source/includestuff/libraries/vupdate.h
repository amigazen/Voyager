#ifndef LIBRARIES_VUPDATE_H
#define LIBRARIES_VUPDATE_H

/*
 * vapor_update.library was not in the GPL drop. These prototypes
 * match MimePrefs / Voyager call sites so GST and C compile.
 * Libcall pragmas are omitted; offsets for the original library
 * are unknown here.
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

extern struct Library *VUPBase;

APTR VUP_BeginCheckUpdate( ULONG product, ULONG version, STRPTR name );
void VUP_Quit( APTR handle );

#endif
