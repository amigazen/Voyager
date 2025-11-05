/*
 * libmath64 test (not pretty)
 *
 * $Id: test.c,v 1.13 2001/09/29 14:15:20 zapek Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <proto/exec.h>
#include <devices/timer.h>
#include <proto/timer.h>

static struct timerequest tio;

#include "math64.h"

#define LOOPTIMES 5000 /* reasonable value */

struct in {
	union {
		QWORD q;
		UQWORD uq;
		double d;
		LONG i;
		ULONG ui;
	} a;
	union {
		QWORD q;
		UQWORD uq;
		double d;
		LONG i;
		ULONG ui;
	} b;
};

struct out {
	union {
		QWORD q;
		UQWORD uq;
		double d;
		LONG i;
		ULONG ui;
	} r;
};

static struct in *in_array;
static struct out *out_array;

enum {
	FI_INT,
	FI_UINT,
	FI_DOUBLE,
	FI_QWORD,
	FI_UQWORD
};

void fill_in(struct in *ina, int modea, int modeb)
{
	ULONG i;

	for (i = 0; i < LOOPTIMES; i++)
	{
		switch (modea)
		{
			case FI_DOUBLE:
				ina->a.d = rand();
				break;
			
			case FI_INT:
				ina->a.i = rand();
				break;

			case FI_UINT:
				ina->a.ui = rand();
				break;

			case FI_QWORD:
				ina->a.q.hi = rand();
				ina->a.q.lo = rand();
				break;

			case FI_UQWORD:
				ina->a.uq.hi = rand();
				ina->a.uq.lo = rand();
				break;
		}

		switch (modeb)
		{
			case FI_DOUBLE:
				ina->b.d = rand();
				break;
			
			case FI_INT:
				ina->b.i = rand();
				break;
				
			case FI_UINT:
				ina->b.ui = rand();
				break;

			case FI_QWORD:
				ina->b.q.hi = rand();
				ina->b.q.lo = rand();
				break;
			
			case FI_UQWORD:
				ina->b.uq.hi = rand();
				ina->b.uq.lo = rand();
				break;
		}
		ina++;
	}
}

enum {
	PE_NONE,
	PE_DIE
};

void puterror(STRPTR s, int f)
{
	printf("error: %s\n", s);

	if (f & PE_DIE)
	{
		exit(0);
	}
}

struct Library *TimerBase;
static struct EClockVal start_timed;

ULONG timedm( void )
{
	struct EClockVal ev;
	ULONG r;
	QWORD ret;

	r = ReadEClock( &ev );

	q_sub( ( QWORD * )&ev, ( QWORD * )&start_timed, &ret );
	return( ( ULONG )q_div( &ret, r / 1000 ) );
}


#define FBUFSIZE 64

void test_divu(void)
{
	ULONG i;
	ULONG fail = 0;
	ULONG start, stop;

	char buf1[FBUFSIZE]; /* yeah well, but SAS/C's %g.xx is buggy anyway */
	char buf2[FBUFSIZE];

	fill_in(in_array, FI_UQWORD, FI_UINT);

	start = timedm();
	for (i = 0; i <LOOPTIMES; i++)
	{
		out_array[i].r.ui = q_divu(&in_array[i].a.uq, in_array[i].b.ui);
	} 
	stop = timedm();

	printf("elapsed time: %lds %ldms\n", (stop - start) / 1000, (stop - start) % 1000);

	for (i = 0; i < LOOPTIMES; i++)
	{
		memset(buf1, '\0', FBUFSIZE);
		memset(buf2, '\0', FBUFSIZE);

		sprintf(buf1, "%g", (double)out_array[i].r.ui);
		sprintf(buf2, "%g", uqtoud(&in_array[i].a.uq) / (double)in_array[i].b.ui);
		
		if (strcmp(buf1, buf2))
		{
			printf("error: integer %s does not equal float %s\n", buf1, buf2);
			printf("(quad: hi:%lu lo:%lu) %g / %lu == %g\n", in_array[i].a.uq.hi, in_array[i].a.uq.lo, uqtoud(&in_array[i].a.uq), in_array[i].b.ui, uqtoud(&in_array[i].a.uq) / in_array[i].b.ui);
			fail++;
		}
	}

	if (fail)
	{
		printf("test_divu(): %lu tests out of %lu failed\n", fail, LOOPTIMES);
	}
}


void test_divu64(void)
{
	ULONG i;
	ULONG fail = 0;
	ULONG start, stop;

	char buf1[FBUFSIZE]; /* yeah well, but SAS/C's %g.xx is buggy anyway */
	char buf2[FBUFSIZE];

	fill_in(in_array, FI_UQWORD, FI_UINT);

	start = timedm();
	for (i = 0; i <LOOPTIMES; i++)
	{
		q_divu64(&in_array[i].a.uq, in_array[i].b.ui, &out_array[i].r.uq);
	} 
	stop = timedm();
	printf("elapsed time: %lds %ldms\n", (stop - start) / 1000, (stop - start) % 1000);

	for (i = 0; i < LOOPTIMES; i++)
	{
		memset(buf1, '\0', FBUFSIZE);
		memset(buf2, '\0', FBUFSIZE);

		sprintf(buf1, "%g", uqtoud(&out_array[i].r.uq));
		sprintf(buf2, "%g", uqtoud(&in_array[i].a.uq) / (double)in_array[i].b.ui);
		
		if (strcmp(buf1, buf2))
		{
			printf("error: integer %s does not equal float %s\n", buf1, buf2);
			printf("(quad: hi:%lu lo:%lu) %g / %lu == %g\n", in_array[i].a.uq.hi, in_array[i].a.uq.lo, uqtoud(&in_array[i].a.uq), in_array[i].b.ui, uqtoud(&in_array[i].a.uq) / in_array[i].b.ui);
			fail++;
		}
	}

	if (fail)
	{
		printf("test_divu64(): %lu tests out of %lu failed\n", fail, LOOPTIMES);
	}
}

void test_mul020(void)
{
	ULONG i;
	ULONG fail = 0;
	ULONG start, stop;

	char buf1[FBUFSIZE]; /* yeah well, but SAS/C's %g.xx is buggy anyway */
	char buf2[FBUFSIZE];

	fill_in(in_array, FI_INT, FI_INT);

	start = timedm();
	for (i = 0; i <LOOPTIMES; i++)
	{
		q_mul020(in_array[i].a.i, in_array[i].b.i, &out_array[i].r.q);
	} 
	stop = timedm();
	printf("elapsed time: %lds %ldms\n", (stop - start) / 1000, (stop - start) % 1000);

	for (i = 0; i < LOOPTIMES; i++)
	{
		memset(buf1, '\0', FBUFSIZE);
		memset(buf2, '\0', FBUFSIZE);

		sprintf(buf1, "%g", qtod(&out_array[i].r.q));
		sprintf(buf2, "%g", (double)in_array[i].a.i * (double)in_array[i].b.i);
		
		if (strcmp(buf1, buf2))
		{
			printf("error: integer %s does not equal float %s\n", buf1, buf2);
			printf("in: %ld * %ld == %g\n", in_array[i].a.i, in_array[i].b.i, qtod(&out_array[i].r.q));
			fail++;
		}
	}

	if (fail)
	{
		printf("test_mul020(): %lu tests out of %lu failed\n", fail, LOOPTIMES);
	}
}

void test_mulu(void)
{
	ULONG i;
	ULONG fail = 0;
	ULONG start, stop;

	char buf1[FBUFSIZE]; /* yeah well, but SAS/C's %g.xx is buggy anyway */
	char buf2[FBUFSIZE];

	fill_in(in_array, FI_INT, FI_INT);

	start = timedm();
	for (i = 0; i <LOOPTIMES; i++)
	{
		q_mulu(in_array[i].a.i, in_array[i].b.i, &out_array[i].r.q);
	} 
	stop = timedm();
	printf("elapsed time: %lds %ldms\n", (stop - start) / 1000, (stop - start) % 1000);

	for (i = 0; i < LOOPTIMES; i++)
	{
		memset(buf1, '\0', FBUFSIZE);
		memset(buf2, '\0', FBUFSIZE);

		sprintf(buf1, "%g", qtod(&out_array[i].r.q));
		sprintf(buf2, "%g", (double)in_array[i].a.i * (double)in_array[i].b.i);
		
		if (strcmp(buf1, buf2))
		{
			printf("error: integer %s does not equal float %s\n", buf1, buf2);
			printf("in: %ld * %ld == %g\n", in_array[i].a.i, in_array[i].b.i, qtod(&out_array[i].r.q));
			fail++;
		}
	}

	if (fail)
	{
		printf("test_mulu(): %lu tests out of %lu failed\n", fail, LOOPTIMES);
	}
}


main()
{
	struct timeval tv;

	if (!(in_array = malloc((LOOPTIMES) * sizeof(struct in))))
		puterror("not enough memory", PE_DIE);

	if (!(out_array = malloc((LOOPTIMES) * sizeof(struct out))))
		puterror("not enough memory", PE_DIE);

	if (tio.tr_node.io_Message.mn_ReplyPort = CreateMsgPort())
	{
		if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)&tio, 0) == 0)
		{
			ULONG start, stop;
			TimerBase = (struct Library *)tio.tr_node.io_Device;

			ReadEClock( &start_timed );


			GetSysTime(&tv);
			srand(tv.tv_micro);

			printf("libmath64 tester\n\nnumber of tests per suite: %ld\n", LOOPTIMES);

			//printf("testing 64-bit division with 32-bit result..\n");
			//test_divu();
			//printf("test finished\n");

			printf("------\n64-bit division with 64-bit result..\n");
			test_divu64();
			printf("test finished\n");

			printf("------\ntesting 32-bit multiplication with 64-bit result (020-040 hardware instruction)..\n");
			test_mul020();
			printf("test finished\n");

			printf("------\ntesting 32-bit multiplication with 64-bit result..\n");
			test_mulu();
			printf("test finished\n");

			CloseDevice((struct IORequest *)&tio);
		}
		DeleteMsgPort(tio.tr_node.io_Message.mn_ReplyPort);
	}


}
