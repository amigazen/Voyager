/*
 * $Id: libfunctable.c,v 1.1 2003/04/27 03:13:11 zapek Exp $
 */

/* public */
#include <dos/dosextens.h> /* makes gcc shutup */

/* private */
#include "lib.h"

void LIB_Open(void);
void LIB_Close(void);
void LIB_Expunge(void);
void LIB_Reserved(void);

/* plugin stuff */
struct TagItem *LIB_VPLUG_Query(void);
APTR LIB_VPLUG_ProcessURLMethod(void);
APTR LIB_VPLUG_GetURLData(void);
STRPTR LIB_VPLUG_GetURLMIMEType(void);
void LIB_VPLUG_FreeURLData(void);

APTR LIB_VPLUG_GetClass(void);
BOOL LIB_VPLUG_Setup(void);
void LIB_VPLUG_Cleanup(void);
void LIB_VPLUG_FinalSetup(void);
void LIB_VPLUG_Hook_Prefs(void);

BOOL LIB_VPLUG_GetInfo(void);
int LIB_VPLUG_GetURLDataSize(void);

int LIB_VPLUG_ProcessURLString(void);


ULONG LibFuncTable[] =
{
	FUNCARRAY_BEGIN,
	FUNCARRAY_32BIT_NATIVE,
	(ULONG)&LIB_Open,
	(ULONG)&LIB_Close,
	(ULONG)&LIB_Expunge,
	(ULONG)&LIB_Reserved,
	(ULONG)&LIB_VPLUG_Query,
	(ULONG)&LIB_VPLUG_ProcessURLMethod,
	(ULONG)&LIB_VPLUG_GetURLData,
	(ULONG)&LIB_VPLUG_GetURLMIMEType,
	(ULONG)&LIB_VPLUG_FreeURLData,
	(ULONG)&LIB_VPLUG_GetClass,
	(ULONG)&LIB_VPLUG_Setup,
	(ULONG)&LIB_VPLUG_Cleanup,
	(ULONG)&LIB_VPLUG_FinalSetup,
	(ULONG)&LIB_VPLUG_Hook_Prefs,
	(ULONG)&LIB_VPLUG_GetInfo,
	(ULONG)&LIB_VPLUG_GetURLDataSize,
	(ULONG)&LIB_VPLUG_ProcessURLString,
	0xffffffff,
	FUNCARRAY_END
};
