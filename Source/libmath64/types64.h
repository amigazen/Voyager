#ifndef TYPES_64_H
#define TYPES_64_H
/*
 * libmath64
 *
 * © 2001 by David Gerber <zapek@meanmachine.ch>
 * All Rights Reserved
 *
 * $Id: types64.h,v 1.2 2001/07/07 20:12:21 zapek Exp $
 */

#ifdef __SASC

#include <exec/types.h>

struct qword_s {
	LONG hi;
	ULONG lo;
};

struct qword_u {
	ULONG hi;
	ULONG lo;
};

typedef struct qword_s QWORD;
typedef struct qword_u UQWORD;

#endif /* __SASC */

#ifdef __GNUC__

typedef long long QWORD;
typedef unsigned long long UQWORD;

#endif /* __GNUC__ */

#endif /* TYPES_64_H */
