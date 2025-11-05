/*
 * libmath64 © 2001 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * Divides a signed 64-bit integer by a signed 32-bit integer.
 * The result is a 32-bit signed integer.
 *
 * Note: can overflow.
 *
 * $Id: q_div.c,v 1.2 2001/07/15 16:47:57 zapek Exp $
 */

#include "math64.h"

ASM LONG q_div(__reg(a0, QWORD *dividend), __reg(d0, LONG divisor))
{
	LONG ret = (LONG)q_divu((UQWORD *)dividend, (ULONG)divisor);
	
	if ((dividend->hi & 0x80000000) ^ (divisor & 0x80000000))
	{
		return ((LONG)(0x80000000 | ret));
	}
	else
	{
		return (ret);
	}
}
