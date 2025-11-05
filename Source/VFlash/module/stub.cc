
#ifdef __GCC__
#include <stdlib.h>
#include <stddef.h>
#include <new>

// const nothrow_t nothrow = {};

void * operator new(size_t size, const nothrow_t&)
{
	return malloc(size);
}

void operator delete(void *p)
{
	return free(p);
}
#endif

#ifdef __DCPLUSPLUS__

// We link with libd.a of DIAB runtime here. The code will hook on libcaos.a
// code, using our malloc.

#endif
