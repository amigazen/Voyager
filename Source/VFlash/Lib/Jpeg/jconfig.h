/* jconfig.h.  Generated automatically by configure.  */
/* jconfig.cfg --- source file edited by configure script */
/* see jconfig.doc for explanations */

//#ifdef MBX
//#ifdef __DCC__
//#pragma pack(1)		// Do packing !
//#warn "Packing enabled for all structures"
//#endif
//#ifdef __GNUC__
//#warn "Packing must be enabled for all structures"
////** Is there some way to tell GNU not to pad ??
//#endif
//#endif

#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#define NO_GETENV
#undef void
#undef const
#undef CHAR_IS_UNSIGNED
//#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS
#define NEED_SHORT_EXTERNAL_NAMES
/* Define this if you get warnings about undefined structures. */
#undef INCOMPLETE_TYPES_BROKEN

#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED
#define INLINE __inline__
/* These are for configuring the JPEG memory manager. */
#undef DEFAULT_MAX_MEM
#undef NO_MKTEMP

#endif /* JPEG_INTERNALS */

