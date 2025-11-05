#ifndef _PPCINLINE_MPEGA_H
#define _PPCINLINE_MPEGA_H

#ifndef CLIB_MPEGA_PROTOS_H
#define CLIB_MPEGA_PROTOS_H
#endif

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
//#ifndef  UTILITY_TAGITEM_H
//#include <utility/tagitem.h>
//#endif
#ifndef  LIBRARIES_TTENGINE_H
#include <libraries/mpega.h>
#endif

#ifndef MPEGA_BASE_NAME
#define MPEGA_BASE_NAME MPEGABase
#endif

#define MPEGA_open(filename, ctrl) \
        LP2(0x1e, MPEGA_STREAM*, MPEGA_open, STRPTR, filename, a0, MPEGA_CTRL*, ctrl, a1, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MPEGA_close(mpds) \
        LP1NR(0x24, MPEGA_close, MPEGA_STREAM*, mpds, a0, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MPEGA_decode_frame(mpds, pcm) \
        LP2(0x2a, LONG, MPEGA_decode_frame, MPEGA_STREAM*, mpds, a0, WORD**, pcm, a1, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MPEGA_seek(mpds, pos) \
        LP2(0x30, LONG, MPEGA_seek, MPEGA_STREAM*, mpds, a0, ULONG, pos, d0, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MPEGA_time(mpds, pos) \
        LP2(0x36, LONG, MPEGA_time, MPEGA_STREAM*, mpds, a0, ULONG*, pos, a1, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MPEGA_find_sync(buf, len) \
        LP2(0x3c, LONG, MPEGA_find_sync, BYTE*, buf, a0, ULONG, len, d0, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MPEGA_scale(mpds, scale) \
        LP2(0x42, LONG, MPEGA_scale, MPEGA_STREAM*, mpds, a0, LONG, scale, d0, \
        , MPEGA_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_MPEGA_H  */
