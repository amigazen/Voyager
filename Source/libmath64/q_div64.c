/*
 * libmath64 © 2001 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * Divides a signed 64-bit integer by a signed 32-bit integer.
 * The result is a 64-bit signed integer.
 *
 * $Id: q_div64.c,v 1.2 2001/07/15 16:47:57 zapek Exp $
 */

#include "math64.h"

ASM void q_div64(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor), __reg(a1, QWORD *res))
{
	q_divu64((UQWORD *)dividend, (ULONG)divisor, (UQWORD *)res);
	
	if ((dividend->hi & 0x80000000) ^ (divisor & 0x80000000))
	{
		res->hi |= 0x80000000;
	}
}
