#ifndef VABOUT_LIB_H
#define VABOUT_LIB_H
/*
 * $Id: lib.h,v 1.1 2003/06/22 23:04:48 zapek Exp $
 */

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <dos/dos.h>

extern struct Library *VAboutBase;

int lib_init(struct ExecBase *SBase);
void lib_cleanup(void);
int lib_open(void);

struct LibBase
{
	struct Library Lib;
	BPTR SegList;
	struct ExecBase *SBase;
};

#endif /* VABOUT_LIB_H */
