#ifndef PROTO_ICONV_H
#define PROTO_ICONV_H

/*
** Prototype and compiler-specific header combo for iconv.library
*/

#ifdef _NO_INLINE

#include <clib/iconv_protos.h>

#else

#ifndef __NOLIBBASE__

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

extern struct Library *IConvBase;
#endif /* __NOLIBBASE__ */

#if defined(LATTICE) || defined(__SASC) || defined(_DCC)

#ifndef PRAGMAS_ICONV_PRAGMAS_H
#include <pragmas/iconv_pragmas.h>
#endif

#elif defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)

#ifndef PRAGMA_ICONV_LIB_H
#include <pragma/iconv_lib.h>
#endif

#elif defined(__VBCC__)

#include <clib/iconv_protos.h>

#ifndef INLINE_ICONV_PROTOS_H
#include <inline/iconv_protos.h>
#endif

#elif defined(__GNUC__)

#if defined(mc68000)
#ifndef INLINE_ICONV_H
#include <inline/iconv.h>
#endif
#else
#include <clib/iconv_protos.h>
#endif

#else

#include <clib/iconv_protos.h>

#endif

#endif /* _NO_INLINE */

#endif /* PROTO_ICONV_H */
