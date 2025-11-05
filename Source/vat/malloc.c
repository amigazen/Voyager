/*
 * $Id: malloc.c,v 1.5 2001/09/26 18:13:47 zapek Exp $
 */

#define __USE_SYSBASE
#include <proto/exec.h>
#include <exec/memory.h>
#include <stdlib.h>
#include <string.h>

// Dummies for gmp.lib (ATM, can be used for something else too)

static APTR globalpool;

int init_globalpool( void )
{
	return( ( int )( globalpool = CreatePool( MEMF_ANY, 1024, 512 ) ) );
}

void cleanup_globalpool( void )
{
	if( globalpool )
	{
		DeletePool( globalpool );
	}
}

/*
 * Those functions are non-reentrant. But it's enough as of today
 * as only gmb.lib uses them (and the calls to it are non-reentrant
 * and protected already).
 */

void *malloc( size_t size )
{
	ULONG *ptr;

	if (!size)
		return(NULL);

	if (!(ptr = AllocPooled(globalpool, size + 4)))
		return(NULL);

	*ptr++ = size + 4;

	return(ptr);
}

void free( void *data )
{
	if (data)
	{
		ULONG *ptr = data;
		FreePooled(globalpool, --ptr, *ptr);
	}
}

void *realloc( void *ptr, size_t size)
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


	if (!(newblock = AllocPooled(globalpool, size + 4)))
		return(NULL);

	*newblock++ = size + 4;

	memcpy(newblock, ptr, *(ULONG *)((ULONG)ptr - 4) > size ? size : *(ULONG *)((ULONG)ptr - 4));

	free(ptr);
	
	return(newblock);
}

