/*
 * $Id: lib.h,v 1.4 2003/04/25 19:13:59 zapek Exp $
*/

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/dos.h>

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

