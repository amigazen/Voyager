#ifndef INLINE_ICONV_H
#define INLINE_ICONV_H

/*
** GCC format inlines for iconv.library
*/

#define libiconv_open(tocode, fromcode) \
({ \
	register void *b __asm("a6") = IConvBase; \
	register const char *p0 __asm("a0") = (tocode); \
	register const char *p1 __asm("a1") = (fromcode); \
	register APTR r __asm("d0"); \
	__asm volatile ("jsr a6@(-30:W);" : "+r"(b), "=r"(r) : "r"(p0), "r"(p1) : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
	(iconv_t)r; \
})

#define libiconv(cd, inbuf, inbytesleft, outbuf, outbytesleft) \
({ \
	register void *b __asm("a6") = IConvBase; \
	register iconv_t p0 __asm("a0") = (cd); \
	register const char **p1 __asm("a1") = (inbuf); \
	register size_t *p2 __asm("a2") = (inbytesleft); \
	register char **p3 __asm("a3") = (outbuf); \
	register size_t *p4 __asm("d0") = (outbytesleft); \
	register ULONG r __asm("d0"); \
	__asm volatile ("jsr a6@(-36:W);" : "+r"(b), "=r"(r), "+r"(p4) : "r"(p0), "r"(p1), "r"(p2), "r"(p3) : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
	(size_t)r; \
})

#define libiconv_close(cd) \
({ \
	register void *b __asm("a6") = IConvBase; \
	register iconv_t p0 __asm("a0") = (cd); \
	register LONG r __asm("d0"); \
	__asm volatile ("jsr a6@(-42:W);" : "+r"(b), "=r"(r) : "r"(p0) : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
	(int)r; \
})

#define libiconvctl(cd, request, argument) \
({ \
	register void *b __asm("a6") = IConvBase; \
	register iconv_t p0 __asm("a0") = (cd); \
	register int p1 __asm("d0") = (request); \
	register void *p2 __asm("a1") = (argument); \
	register LONG r __asm("d0"); \
	__asm volatile ("jsr a6@(-48:W);" : "+r"(b), "=r"(r), "+r"(p1) : "r"(p0), "r"(p2) : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
	(int)r; \
})

#define libiconvlist(do_one, data) \
do { \
	register void *b __asm("a6") = IConvBase; \
	register APTR p0 __asm("a0") = (APTR)(do_one); \
	register APTR p1 __asm("a1") = (data); \
	__asm volatile ("jsr a6@(-54:W);" : "+r"(b) : "r"(p0), "r"(p1) : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
} while (0)

#define libiconv_set_relocation_prefix(orig_prefix, curr_prefix) \
do { \
	register void *b __asm("a6") = IConvBase; \
	register const char *p0 __asm("a0") = (orig_prefix); \
	register const char *p1 __asm("a1") = (curr_prefix); \
	__asm volatile ("jsr a6@(-60:W);" : "+r"(b) : "r"(p0), "r"(p1) : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
} while (0)

#define iconv_canonicalize(name) \
({ \
	register void *b __asm("a6") = IConvBase; \
	register const char *p0 __asm("a0") = (name); \
	register APTR r __asm("d0"); \
	__asm volatile ("jsr a6@(-66:W);" : "+r"(b), "=r"(r) : "r"(p0) : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
	(const char *)r; \
})

#endif /* INLINE_ICONV_H */
