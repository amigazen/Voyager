/*
 * VFlash PPC debug
 * ----------------
 * - not used anymore.. mostly for reference
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: ppc_debug.c,v 1.1.1.1 2002/10/07 12:52:15 zapek Exp $
 *
 */

#include "globals.h"

#if (PPC && (DEBUG == 1))

#if 0 // not used anymore... at least for me :)
#include <stdarg.h>

	/*
	; kprintf() replacement for PPC build

	section "text",code

	.loop:
	move.b (a2)+,d0
	beq.s .end
	jsr -$204(a6)
	bra .loop
	.end:
	rts

	END
	*/
	UWORD execdebug[] =
	{
		0x101a,
		0x6706,
		0x4eae,
		0xfdfc,
		0x60f6,
		0x4e75
	};

#define KPRINTF_BUFSIZE 2048

	void kprintf(char *cstring, ...)
	{
		struct Caos *MyCaos;
		APTR buffer;
		va_list va;

		/* Handle 8 arguments, should be enough */
		APTR args[8];
		ULONG i = 0;
		ULONG j;
		char *p = cstring;

		while (*p)
		{
			if (*p == '%')
			{
				i++;
			}
			p++;
		}

		va_start(va, cstring);
		for (j = 0; j < i; j++)
		{
			args[j] = va_arg(va, APTR);
		}
		va_end(va);

		if (buffer = PPCAllocVec(KPRINTF_BUFSIZE, MEMF_PUBLIC))
		{
			PPCsprintf(buffer, cstring, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);

			if (MyCaos = (struct Caos *) PPCAllocVec(sizeof(struct Caos), MEMF_PUBLIC | MEMF_CLEAR))
			{
				MyCaos->caos_Un.Function = (APTR)&execdebug;
				MyCaos->M68kCacheMode	 = IF_CACHEFLUSHNO; // flush should be needed I think. I don't care anymore
				MyCaos->PPCCacheMode	 = IF_CACHEFLUSHNO;
				MyCaos->a2				 = (ULONG)buffer;
				MyCaos->a6				 = (ULONG)SysBase;

				PPCCallM68k(MyCaos);
				PPCFreeVec(MyCaos);
			}
			PPCFreeVec(buffer);
		}
	}
#endif /* PPC */
#endif
