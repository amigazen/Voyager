/*
 * VFlash memory allocations
 * -------------------------
 * - internal memory allocation functions. Needed
 * for speed in early versions of VFlash. Now
 * libnix supports _MSTEP so internal malloc
 * is unecessary.
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: memory.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#include "globals.h"
#include "mempool.h"

#if USE_INTERNAL_MALLOC
#ifdef POWERUP
/* pool sizes */
#define PPC_PUDDLESIZE 65536
#define PPC_TRESHSIZE 65536

APTR globalmempool;

void meminit(void)
{
	if (!(globalmempool = myCreatePool(MEMF_ANY, PPC_PUDDLESIZE, PPC_TRESHSIZE)))
	{
		exit(-1);
	}
}

void memcleanup(void)
{
	if (globalmempool)
	{
		myDeletePool(globalmempool);
	}
}

void *malloc(size_t size)
{
	ULONG *ptr;

	if (!size)
		return(NULL);

	if (!(ptr = myAllocPooled(globalmempool, size + 4)))
		return(NULL);

	*ptr++ = size + 4;

	return(ptr);
}


void *calloc(int num, size_t size)
{
	void *memblock;

	DB(("called calloc()\n"));

	size = num * size;

	if (memblock = malloc(size))
	{
		memset(memblock, 0L, size);
		return(memblock);
	}
	else
		return(NULL);
}


void free(void *data)
{
	if (data)
	{
		ULONG *ptr = data;
		myFreePooled(globalmempool, --ptr, *ptr);
	}
}


void *realloc(void *ptr, size_t size)
{
	ULONG *newblock;

	if (!ptr)
	{
		if(size)
			return(malloc(size));
		else
			return(NULL);
	}

	if (!size)
	{
		free(ptr);
		return(NULL);
	}


	if (!(newblock = myAllocPooled(globalmempool, size + 4)))
		return(NULL);

	*newblock++ = size + 4;

	memcpy(newblock, ptr, *(ULONG *)((ULONG)ptr - 4) > size ? size : *(ULONG *)((ULONG)ptr - 4));

	free(ptr);

	return(newblock);
}
/* Constructor/Deconstructor things, borrowed from Apdf */
__asm__(".section \".init\"");
__asm__(".long meminit");
__asm__(".long 78");

__asm__(".section \".fini\"");
__asm__(".long memcleanup");
__asm__(".long 78");
#else
#error "Target doesn't support internal memory allocations"
#endif /* POWERUP */
#endif /* USE_INTERNAL_MALLOC */
