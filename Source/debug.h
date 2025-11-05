/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_DEBUG_H
#define VOYAGER_DEBUG_H
/*
 * V debug support
 * ---------------
 * - Select debugging output. Authors are encouraged to put debug output as much as possible
 *   in their sources. It's easy and the betatesters can give very usefull outputs.
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * How to use:
 * -----------
 * Define VDEBUG as 1 if you want normal debugging. This should be enabled in betas so
 * betatesters can chose their debug level anytime. If VDEBUG is defined as 2, the resulting
 * file will be compiled with debug output always enabled.
 *
 * $Id: debug.h,v 1.36 2003/04/25 19:13:53 zapek Exp $
 *
 */

#ifdef VDEBUG

#ifdef __GNUC__ /* GCC */
#define __FUNC__ __FUNCTION__
#endif

#ifdef AMIGAOS
extern void kprintf(char *, ...);
#endif

#ifdef __MORPHOS__
#if 0
#define kprintf(Fmt, args...) \
	  ({ULONG _args[] = { args }; RawDoFmt((Fmt), (void*) _args, (void(*)(void)) 0x1, NULL);})
#endif
void dprintf(char *, ... );
#define kprintf dprintf
#endif

#ifdef MBX
#include <kprintf.h>
#endif /* MBX */
/*
 * Straight debug
 *
 * Example: DB(("you suck %ld times\n", num));
 */
#ifdef MBX /* CaOS' kprintf is buggy */
#define DB(x)   { kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf x ; kprintf( "\r\n" ); }
#define MBXDB(x)
//#define MBXDB DB
//Turn this on for really heavy net debug output
#else
#define DB(x)   { kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf x ; }
#define MBXDB(x)
#endif /* !MBX */

/*
 * Straight debug, obtaining vebosity levels
 *
 * Example: DBL( DEBUG_CHATTY, ("you suck %ld times\n", num));
 */
#ifdef MBX /* CaOS' kprintf is buggy */
#define DBL(lvl,x)   if( lvl <= db_level ) { kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf x ; kprintf( "\r\n" ); }
#else
#define DBL(lvl,x)   if( lvl <= db_level ) { kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf x ; }
#endif /* !MBX */


/*
 * Straight debug with 1 second delay
 *
 * Example: DBD(("you suck %ld times and slowly\n", num));
 */
#define DBD(x)  { Delay(50); kprintf(__FILE__ "[%4ld]:" __FUNC__ "() : ",__LINE__); kprintf x ; }

/*
 * Selective debug with level selection
 *
 * Example: DL(1,db_html,bug("you suck HTML %ld times\n", num));
 *
 * Only outputted if db_level is >= the level given to DL
 */
#if (VDEBUG == 1)
#define DL(lvl,class,x) if ( class && db_level >= lvl ) { ##x; }
#endif
#if (VDEBUG == 2)
#define DL(lvl,class,x) if ( db_level >= lvl ) { ##x; }
#endif


/*
 * Selective debug
 *
 * Example: D(db_html,bug("you suck HTML %ld times\n", num));
 */
#if (VDEBUG == 1)
#define D(class,x) if (class) { ##x; }
#endif
#if (VDEBUG == 2)
#define D(class,x) if (1) { ##x; }
#endif

#define bug kprintf(__FILE__ "[%4ld]/" __FUNC__ "() : ",__LINE__); kprintf


/*
 * ALERT
 *
 * Print extensive warning message, then wait for keypress
 * Only ever use this for Stuff Which Does Not Happen(tm)
 *
 * Example: ALERT( ( "Complete fuckup in state machine" ) );
 *
 */
#ifdef AMIGAOS
extern char STDARGS kgetchar( void );
#define ALERT(m) \
 kprintf( "\r\n**********************************************\r\n" ); \
 kprintf( "*** ALERT! " __FILE__ "[%4ld]/" __FUNC__ "() : ***\r\n",__LINE__); \
 kprintf m; \
 kprintf( "**********************************************\r\n" ); \
 kgetchar()
#endif

#ifdef __MORPHOS__
#define ALERT(m) \
 kprintf( "\r\n**********************************************\r\n" ); \
 kprintf( "*** ALERT! " __FILE__ "[%4ld]/" __FUNC__ "() : ***\r\n",__LINE__); \
 kprintf m; \
 kprintf( "**********************************************\r\n" );
#endif

#ifdef MBX
#define ALERT(m) \
 kprintf( "\r\n**********************************************\r\n" ); \
 kprintf( "*** ALERT! " __FILE__ "[%4ld]/" __FUNC__ "() : ***\r\n",__LINE__); \
 kprintf m; \
 kprintf( "**********************************************\r\n" ); \
 KGetChar()
#endif /* MBX */

#define ASSERT(x) { if (!x) { kprintf("*** assertion failed at: " __FILE__ "[%4ld]/" __FUNC__ "()\n",__LINE__); } }

void dump_image(UBYTE *p, ULONG size, ULONG width);

/*
 * Special debugging features (enabled from config.h)
 */
#if DEBUG_DISPOSE
/*
 * Only for MorphOS ATM, and not nice
 */
#ifdef __MORPHOS__
#undef MUI_DisposeObject
#include <ppcinline/macros.h>
#define MUI_DisposeObject(obj) \
	dprintf( "MUI_DisposeObject(0x%lx)\n", obj ); \
	LP1NR(0x24, MUI_DisposeObject, Object *, obj, a0, \
	, MUIMasterBase, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)
#endif /* __MORPHOS__ */
#endif /* DEBUG_DISPOSE */

/*
 * Debug levels
 */

#define DEBUG_ERROR 0
#define DEBUG_WARNING 1
#define DEBUG_IMPORTANT 2
#define DEBUG_INFO 3
#define DEBUG_CHATTY 4

/*
 * Flags
 */
extern int db_auth;
extern int db_cache;
extern int db_cookie;
extern int db_dns;
extern int db_docinfowin;
extern int db_dlwin;
extern int db_ftp;
extern int db_history;
extern int db_http;
extern int db_js;
extern int db_net;
extern int db_plugin;
extern int db_mail;
extern int db_cacheprune;
extern int db_html;
extern int db_gui;
extern int db_init;
extern int db_forceborder;
extern int db_rexx;
extern int db_css;

extern int db_misc;

extern int db_level;


#else
#define DB(x)
#define DBL(lvl,x)
#define DBD(x)
#define D(class,x)
#define DL(lvl,class,x)
#define bug
#define ALERT(m)
#define ASSERT(x)
#endif /* VDEBUG */

#endif /* VOYAGER_DEBUG_H */
