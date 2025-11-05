#ifndef CLIB_SCREENNOTIFY_H
#define CLIB_SCREENNOTIFY_H

struct Screen;
struct MsgPort;


/*
 * clib/screennotify_protos.h
 *
 * ANSI C prototypes for screennotify.library functions
 *
 * $VER: screennotify_protos.h 1.0 (26.03.95)
 *
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

APTR AddCloseScreenClient(struct Screen *, struct MsgPort *, BYTE);
APTR AddPubScreenClient(struct MsgPort *, BYTE);
APTR AddWorkbenchClient(struct MsgPort *, BYTE);
BOOL RemCloseScreenClient(APTR);
BOOL RemPubScreenClient(APTR);
BOOL RemWorkbenchClient(APTR);

#endif
