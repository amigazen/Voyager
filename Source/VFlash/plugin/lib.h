#ifndef VFLASH_LIB_H
#define VFLASH_LIB_H
/*
 * $Id: lib.h,v 1.1 2003/04/27 03:13:11 zapek Exp $
 */

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <dos/dos.h>

extern struct Library *VFlashBase;

int lib_init(struct ExecBase *SBase);
void lib_cleanup(void);
ULONG lib_open(void);


struct LibBase
{
	struct Library Lib;
	BPTR SegList;
	struct ExecBase *SBase;
};

#endif /* VFLASH_LIB_H */
