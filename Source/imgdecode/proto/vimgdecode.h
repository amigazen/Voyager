#ifndef PROTO_VIMGDECODE_H
#define PROTO_VIMGDECODE_H
#ifdef MBX
#include "mbx.h"
#else
#include <exec/types.h>
#endif /* !MBX */
extern struct Library *VIDBase;
#include <clib/vimgdecode_protos.h>
/*
 * The libcall pragmas are deliberately not included: the decoder is linked
 * into the executable, so VIDBase is not a real library base and every
 * pragma-generated jsr through it would crash. See the comment in
 * <clib/vimgdecode_protos.h>.
 */
#endif /* !PROTO_VIMGDECODE_H */
