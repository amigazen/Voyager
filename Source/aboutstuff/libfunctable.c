/*
 * Ambient - the ultimate desktop
 * ------------------------------
 * (c) 2001-2003 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * $Id: libfunctable.c,v 1.1 2003/06/22 23:04:50 zapek Exp $
 */

//#include "globals.h"

/* public */
#include <dos/dosextens.h> /* makes gcc shutup */

/* private */
#include "lib.h"

void LIB_Open(void);
void LIB_Close(void);
void LIB_Expunge(void);
void LIB_Reserved(void);

STRPTR LIB_VABOUT_GetAboutPtr(void);
APTR LIB_VABOUT_GetVLogo(void);
APTR LIB_VABOUT_GetV3Logo(void);
APTR LIB_VABOUT_GetFlashLogo(void);
APTR LIB_VABOUT_GetSSLLogo(void);
APTR LIB_VABOUT_GetPNGLogo(void);
STRPTR LIB_VABOUT_GetAboutIbeta(void);


ULONG LibFuncTable[] =
{
	FUNCARRAY_BEGIN,
	FUNCARRAY_32BIT_NATIVE,
	(ULONG)&LIB_Open,
	(ULONG)&LIB_Close,
	(ULONG)&LIB_Expunge,
	(ULONG)&LIB_Reserved,
	(ULONG)&LIB_VABOUT_GetAboutPtr,
	(ULONG)&LIB_VABOUT_GetVLogo,
	(ULONG)&LIB_VABOUT_GetSSLLogo,
	(ULONG)&LIB_VABOUT_GetPNGLogo,
	(ULONG)&LIB_VABOUT_GetAboutIbeta,
	(ULONG)&LIB_VABOUT_GetV3Logo,
	(ULONG)&LIB_VABOUT_GetFlashLogo,
	0xffffffff,
	FUNCARRAY_END
};
