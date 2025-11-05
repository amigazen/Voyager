#ifndef _PROTO_MPEGA_H
#define _PROTO_MPEGA_H

#include <clib/mpega_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
MPEGABase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/mpega.h>
#else
#include <inline/mpega.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/mpega_pragmas.h>
#else
#include <pragmas/mpega_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_MPEGA_H  */
