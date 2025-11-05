#ifndef PROTO_VIMGDECODE_H
#define PROTO_VIMGDECODE_H
#ifdef MBX
#include "mbx.h"
#else
#include <exec/types.h>
#endif /* !MBX */
extern struct Library *VIDBase;
#include <clib/vimgdecode_protos.h>
#ifdef __SASC
#include <pragmas/vimgdecode_pragmas.h>
#endif /* __SASC */
#endif /* !PROTO_VIMGDECODE_H */
