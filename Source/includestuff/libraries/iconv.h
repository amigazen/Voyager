#ifndef LIBRARIES_ICONV_H
#define LIBRARIES_ICONV_H

/*
 * $VER: iconv.h 3.1 (24.9.2026)
 *
 * Public API for iconv.library — GNU libiconv 1.11 wrapped as a classic
 * Amiga shared library (amigazen project). API names match MorphOS
 * iconv.library for source portability.
 *
 * GNU LIBICONV: LGPL-2.0-or-later (Free Software Foundation).
 * Amiga library glue: LGPL-2.0-or-later (see LICENSE.md).
 */

#ifndef _LIBICONV_H
#define _LIBICONV_H

#define _LIBICONV_VERSION 0x010B

#ifdef __cplusplus
extern "C" {
#endif

extern int _libiconv_version;

#undef iconv_t
#define iconv_t libiconv_t
typedef void *iconv_t;

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#include <stddef.h>
#include <errno.h>

#ifndef EILSEQ
#define EILSEQ ENOENT
#endif

#ifndef LIBICONV_PLUG
#define iconv_open libiconv_open
#endif

#ifndef LIBICONV_PLUG
#define iconv libiconv
#endif

#ifndef LIBICONV_PLUG
#define iconv_close libiconv_close
#endif

#ifndef LIBICONV_PLUG
#define iconvctl libiconvctl
#endif

#ifndef LIBICONV_PLUG
#define iconvlist libiconvlist
#endif

typedef void (*iconv_unicode_char_hook)(unsigned int uc, void *data);
typedef void (*iconv_wide_char_hook)(unsigned int wc, void *data);

struct iconv_hooks {
	iconv_unicode_char_hook uc_hook;
	iconv_wide_char_hook wc_hook;
	void *data;
};

typedef void (*iconv_unicode_mb_to_uc_fallback)(
	const char *inbuf, size_t inbufsize,
	void (*write_replacement)(const unsigned int *buf, size_t buflen,
		void *callback_arg),
	void *callback_arg,
	void *data);
typedef void (*iconv_unicode_uc_to_mb_fallback)(
	unsigned int code,
	void (*write_replacement)(const char *buf, size_t buflen,
		void *callback_arg),
	void *callback_arg,
	void *data);
typedef void (*iconv_wchar_mb_to_wc_fallback)(
	const char *inbuf, size_t inbufsize,
	void (*write_replacement)(const unsigned int *buf, size_t buflen,
		void *callback_arg),
	void *callback_arg,
	void *data);
typedef void (*iconv_wchar_wc_to_mb_fallback)(
	unsigned int code,
	void (*write_replacement)(const char *buf, size_t buflen,
		void *callback_arg),
	void *callback_arg,
	void *data);

struct iconv_fallbacks {
	iconv_unicode_mb_to_uc_fallback mb_to_uc_fallback;
	iconv_unicode_uc_to_mb_fallback uc_to_mb_fallback;
	iconv_wchar_mb_to_wc_fallback mb_to_wc_fallback;
	iconv_wchar_wc_to_mb_fallback wc_to_mb_fallback;
	void *data;
};

#define ICONV_TRIVIALP            0
#define ICONV_GET_TRANSLITERATE   1
#define ICONV_SET_TRANSLITERATE   2
#define ICONV_GET_DISCARD_ILSEQ   3
#define ICONV_SET_DISCARD_ILSEQ   4
#define ICONV_SET_HOOKS           5
#define ICONV_SET_FALLBACKS       6

#ifdef __cplusplus
}
#endif

#endif /* _LIBICONV_H */

#endif /* LIBRARIES_ICONV_H */
