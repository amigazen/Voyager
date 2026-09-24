#ifndef PRAGMAS_ICONV_PRAGMAS_H
#define PRAGMAS_ICONV_PRAGMAS_H

/*
** Lattice / SAS/C / DICE pragma file for iconv.library
*/

#ifndef CLIB_ICONV_PROTOS_H
#include <clib/iconv_protos.h>
#endif

/* "iconv.library" */
#pragma libcall IConvBase libiconv_open 1E 9802
#pragma libcall IConvBase libiconv 24 0BA9805
#pragma libcall IConvBase libiconv_close 2A 801
#pragma libcall IConvBase libiconvctl 30 90803
#pragma libcall IConvBase libiconvlist 36 9802
#pragma libcall IConvBase libiconv_set_relocation_prefix 3C 9802
#pragma libcall IConvBase iconv_canonicalize 42 801

#endif /* PRAGMAS_ICONV_PRAGMAS_H */
