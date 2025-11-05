/*
 * libmath64 © 2001 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * Converts a QWORD to a double.
 *
 * $Id: qtod.c,v 1.2 2001/07/15 01:15:22 zapek Exp $
 */

#include "math64.h"
#include <limits.h>

ASM double qtod(__reg(a0, QWORD *q))
{
	return ((double)(q->hi * (UINT_MAX + 1.0) + q->lo));
}
