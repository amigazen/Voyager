#ifndef VOYAGER_SNPRINT_H
#define VOYAGER_SNPRINT_H
/*
 * SNPrintf/VSNPrintf live in utility.library V47 (OS 3.2). Jumping to
 * those LVOs on older utility.library crashes. Route every SNPrintf through snprintf.c (exec RawDoFmt).
 */
#ifdef AMIGAOS
#ifdef SNPrintf
#undef SNPrintf
#endif
#ifdef VSNPrintf
#undef VSNPrintf
#endif
int snprintf( char *str, unsigned int size, const char *format, ... );
int voy_vsnprintf( char *str, unsigned int size, const char *format, APTR args );
#define SNPrintf snprintf
#define VSNPrintf voy_vsnprintf
#endif /* AMIGAOS */
#endif /* VOYAGER_SNPRINT_H */
