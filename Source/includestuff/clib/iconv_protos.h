#ifndef CLIB_ICONV_PROTOS_H
#define CLIB_ICONV_PROTOS_H

/*
** 'C' prototypes for iconv.library (32-bit integers).
**
** Generated to match SDK/SFD/iconv_lib.sfd
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIBRARIES_ICONV_H
#include <libraries/iconv.h>
#endif

/* "iconv.library" */
iconv_t libiconv_open(const char *tocode, const char *fromcode);
size_t libiconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
	char **outbuf, size_t *outbytesleft);
int libiconv_close(iconv_t cd);
int libiconvctl(iconv_t cd, int request, void *argument);
void libiconvlist(int (*do_one)(unsigned int namescount,
	const char * const *names, void *data), void *data);
void libiconv_set_relocation_prefix(const char *orig_prefix,
	const char *curr_prefix);
const char *iconv_canonicalize(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* CLIB_ICONV_PROTOS_H */
