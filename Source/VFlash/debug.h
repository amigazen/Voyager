/*
**
** Some debugging macros by David Gerber <zapek@vapor.com>
**
** Use: DB(("now crashing\n")) or DBD(("now crashing\n"))
** for a printout with 1 second delay
**
** Works with SAS/C and GCC
**
** $Id: debug.h,v 1.2 2003/04/27 03:13:10 zapek Exp $
**
*/

#if (DEBUG==1)
	
	#ifdef __GNUC__ /* GCC */
		#define __FUNC__ __FUNCTION__
		#define kprintf dprintf
	#endif
	
	#ifdef PPC
	
		/*
		 * Define if PowerUP debug output has to go through
		 * the internal serial port.
		 * Otherwise use the magic function to send stuffs too
		 * sushi (discontinued)
		 */
		#define PPC_RAW_SERIAL

		#ifdef PPC_RAW_SERIAL
			#ifdef __cplusplus  /* C++ */
				extern "C" { void PPCkprintf(const char *, ...); };
				#define DB(x)   { PPCkprintf(__FILE__ "/[%4ld]" ,__LINE__); PPCkprintf x ; }
			#else
				extern void PPCkprintf(const char *, ...);
				#define DB(x)   { PPCkprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); PPCkprintf x ; }
			#endif
		#else
			#include <exec/types.h>
			#include <exec/memory.h>
			#include <powerup/ppclib/interface.h>

			extern struct ExecBase *SysBase;
			extern UWORD execdebug[];

			#ifdef __cplusplus /* C++ */
				extern "C" { void kprintf( char *cstring, ...); };
			#else
				extern void kprintf( char *cstring, ...);
			#endif

			#define DB(x)	{ kprintf x ; }
		#endif /* PPC_RAW_SERIAL */
	#else
	    #ifdef __cplusplus  /* C++ */
		    extern "C" { void kprintf(char *, ...); };
			#ifndef MBX
				extern "C" { void Delay( long ); };
			#endif
			#define DB(x)   { kprintf(__FILE__ "/[%4ld]" ,__LINE__); kprintf x ; }
			#ifndef MBX
				#define DBD(x)  { Delay(50); kprintf(__FILE__ "/",__LINE__); kprintf x ; }
			#endif
		#else
		    extern void kprintf(char *, ...);
		    #define DB(x)   { kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf x ; }
		    #define DBD(x)  { Delay(50); kprintf(__FILE__ "[%4ld]:" __FUNC__ "() : ",__LINE__); kprintf x ; }
	    #endif
	#endif
#else
	#define DB(x)
	#define DBD(x)
#endif
