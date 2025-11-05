#ifndef MATH64_H
#define MATH64_H
/*
 * libmath64
 *
 * © 2001 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * $Id: math64.h,v 1.9 2001/09/27 14:00:44 zapek Exp $
 */

#include "types64.h"

#ifndef ASM
#define ASM __asm
#endif

#ifndef __reg
#define __reg(x,y) register __ ## x y
#endif

ASM	LONG q_div(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor));
ASM	ULONG q_divu(__reg(a0, UQWORD *dividend), __reg(d0, ULONG divisor));
ASM	ULONG q_modu(__reg(a0, UQWORD *dividend), __reg(d0, ULONG divisor));
ASM	LONG q_mod(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor));
ASM LONG q_div020(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor));
ASM ULONG q_divu020(__reg(a0, UQWORD *dividend), __reg(d0, ULONG divisor));
ASM void q_div64(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor), __reg(a1, QWORD *res));
ASM void q_divu64(__reg(a0, UQWORD *dividend), __reg(d0, ULONG divisor), __reg(a1, UQWORD *res));

ASM LONG q_mod020(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor));
ASM void q_add(__reg(a0, QWORD *a), __reg(a1, QWORD *b), __reg(a2, QWORD *res));
ASM void q_sub(__reg(a0, QWORD *a), __reg(a1, QWORD *b), __reg(a2, QWORD *res));
ASM void q_mul020(__reg(d0, LONG multiplicand), __reg(d1, LONG multiplier), __reg(a0, QWORD *res));
ASM void q_mulu(__reg(d0, ULONG multiplicand), __reg(d1, ULONG multiplier), __reg(a0, QWORD *res));
ASM double qtod(__reg(a0, QWORD *q));
ASM double uqtoud(__reg(a0, UQWORD *q));

#endif /* MATH64_H */
