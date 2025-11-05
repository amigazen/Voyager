/*
 *  gcc -noixemul -I../include -o printmethods printmethods.c
 */
#include <stdio.h>
#include <exec/types.h>
#include <utility/tagitem.h>

#include "classes.h"

main(  )
{
	printf( "tag begin: 0x%lx, tag end: 0x%lx\n", MA_dummy, MA_dummyend );
}
