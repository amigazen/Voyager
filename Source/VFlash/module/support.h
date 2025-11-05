#ifndef VFLASH_SUPPORT_H
#define VFLASH_SUPPORT_H
/*
 * VFlash support functions
 * ------------------------
 *
 * $Id: support.h,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#if defined(AMIGAOS) || defined(__MORPHOS__)
APTR oldwinptr;
#endif

ULONG getv(APTR obj, ULONG attr);
void Fail(STRPTR txt);
#ifdef POWERUP
/* DoMethod() replacement for PowerUP */
#undef DoMethod
#define DoMethod(obj, methodid, tags...) \
({ \
	ULONG _tags[] = { tags }; \
	struct opSet os; \
	os.MethodID = methodid; \
	os.ops_AttrList = (struct TagItem *)_tags; \
	os.ops_GInfo = NULL; \
	PPCDoMethodA(obj, (Msg)&os); \
})
#endif /* POWERUP */

#endif /* !VFLASH_SUPPORT_H */
