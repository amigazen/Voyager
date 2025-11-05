/*
**
** Some debugging macros by David Gerber <zapek@vapor.com>
**
** Use: DB(("now crashing\n")) or DBD(("now crashing\n"))
** for a printout with 1 second delay
**
** Works with SAS/C and GCC
**
** $Id: vat_debug.h,v 1.1 2001/03/17 23:50:45 zapek Exp $
**
*/

#if (DEBUG==1)
	
	#ifdef __MORPHOS__
	//#define kprintf(Fmt, args...) \
	//	  ({ULONG _args[] = { args }; RawDoFmt((Fmt), (void*) _args, (void(*)(void)) 0x1, NULL);})
	#define kprintf dprintf
	#endif

	#ifdef __GNUC__ /* GCC */
		#define __FUNC__ __FUNCTION__
	#endif
	
	#ifndef __MORPHOS__
	extern void kprintf(char *, ...);
	#endif
	#define DB(x)   { kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf x ; }
	#define DBD(x)  { Delay(50); kprintf(__FILE__ "[%4ld]:" __FUNC__ "() : ",__LINE__); kprintf x ; }
#else
	#define DB(x)
	#define DBD(x)
#endif
