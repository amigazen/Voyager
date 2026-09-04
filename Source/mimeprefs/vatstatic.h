#ifndef MIMEPREFS_VATSTATIC_H
#define MIMEPREFS_VATSTATIC_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

struct Screen;

APTR AllocVecPooled( APTR poolheader, ULONG memsize );
void FreeVecPooled( APTR poolheader, APTR memory );

APTR VUP_BeginCheckUpdate( ULONG product, ULONG version, STRPTR name );
void VUP_Quit( APTR handle );

struct Screen *VAT_GetAppScreen( APTR app );
STRPTR VAT_GetAppScreenName( APTR app );

#endif
