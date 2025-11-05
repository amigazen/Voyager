/*
 * $Id: lib.h,v 1.2 2002/10/28 14:35:52 zapek Exp $
*/

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <ppcinline/exec.h>

#include <public/quark/quark.h>
#include <public/proto/quark/syscall_protos.h>

extern struct ExecBase *SysBase;  

int lib_init( struct ExecBase *SBase );
void lib_cleanup( void );

struct LibBase
{
	struct Library Lib;
	BPTR SegList;
	struct ExecBase *SBase;
};

//#define SysBase VimgdecodeBase->SBase

