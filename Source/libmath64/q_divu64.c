/*
 * libmath64 © 2001 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * Divides an unsigned 64-bit integer by an unsigned 32-bit integer.
 * The result is a 64-bit unsigned integer. Uses the D Algorithm from
 * Knuth. Posted by Thomas Richter on comp.sys.amiga.programmer.
 * Modified by me to handle 64-bit results.
 *
 * $Id: q_divu64.c,v 1.1 2001/07/15 15:16:19 zapek Exp $
 */

#include "math64.h"

ASM void q_divu64(__reg(a0, UQWORD *dividend), __reg(d0, ULONG divisor), __reg(a1, UQWORD *res))
{
	ULONG bits;
	ULONG d1, dhi, dlo;

	d1 = 0x00;
	dhi = dividend->hi;
	dlo = dividend->lo;

	for (bits = 0; bits < 64; bits++)
	{
		/* now left-shift d1, dhi, dlo */
		d1 <<= 1;
		if (dhi & 0x80000000)
		{
			d1 |= 0x01;
		}
		dhi <<= 1;
		
		if (dlo & 0x80000000)
		{
			dhi |= 0x01;
		}
		dlo	<<= 1;
		
		if (d1 >= divisor)
		{
			d1 -= divisor;
			dlo	|= 0x01;
		}
	}

	//dividend->lo = d1; /* remainder is lost.. who cares */

	res->hi = dhi;
	res->lo = dlo;
}
