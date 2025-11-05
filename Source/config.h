/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_CONFIG_H
#define VOYAGER_CONFIG_H
/*
 * $Id: config.h,v 1.117 2005/01/01 10:57:26 zapek Exp $
 */

/*
 * Quick debugging options
 */
/* Turns on Historylist debugging, will go away once I'm sure this is fixed */
#define HISTORYLIST_DEBUG 0
/* Network socket trashing debugging */
#define NETWORK_DEBUG 0


/*
 * Release options (mutually exclusive !)
 */
#define IBETA_RELEASE 1
#define BETA_RELEASE  0
#define FINAL_RELEASE 0
#define GREX_RELEASE 0
#define PEGASOS_RELEASE 0

/*
 * Timeout
 */
#define TIMEOUT_YEAR 2006 /* year */
#define TIMEOUT_MONTH 1  /* last running month (1-12) */


/*
 * Release builds
 */

/*
 * IBeta release:
 */
#if IBETA_RELEASE
#if BETA_RELEASE || FINAL_RELEASE || PEGASOS_RELEASE
#error Build options are mutually exclusive !
#endif
#define EXTRA_DEBUG 1
#define BETA_VERSTRING 0
#define ALPHA_WARNING 1
#define NEED_KEYFILE 1
#define USE_KEYFILES 1
#define VAT_ECRYPT_CHECK 0
#define CHECK_TIMEOUT 0
#define BAIL_ON_TIMEOUT 0
#endif /* IBETA_RELEASE */

/*
 * Public beta release:
 */
#if BETA_RELEASE
#if IBETA_RELEASE || FINAL_RELEASE || PEGASOS_RELEASE
#error Build options are mutually exclusive !
#endif
#define EXTRA_DEBUG 0
#define BETA_VERSTRING 1
#define ALPHA_WARNING 0
#define NEED_KEYFILE 0
#define USE_KEYFILES 1
#define VAT_ECRYPT_CHECK 0
#define CHECK_TIMEOUT 1
#define BAIL_ON_TIMEOUT 0
#endif /* BETA_RELEASE */

/*
 * Final release:
 */
#if FINAL_RELEASE
#if IBETA_RELEASE || BETA_RELEASE || PEGASOS_RELEASE
#error Build options are mutually exclusive !
#endif
#define EXTRA_DEBUG 0
#define BETA_VERSTRING 0
#define ALPHA_WARNING 0
#define NEED_KEYFILE 0
#define USE_KEYFILES 1
#define VAT_ECRYPT_CHECK 0
#define CHECK_TIMEOUT 0
#define BAIL_ON_TIMEOUT 0
#endif /* FINAL_RELEASE */

/*
 * Final release (GRex):
 */
#if GREX_RELEASE
#if IBETA_RELEASE || BETA_RELEASE || PEGASOS_RELEASE
#error Build options are mutually exclusive !
#endif
#define EXTRA_DEBUG 0
#define BETA_VERSTRING 0
#define ALPHA_WARNING 0
#define NEED_KEYFILE 0
#define USE_KEYFILES 0
#define VAT_ECRYPT_CHECK 0
#define CHECK_TIMEOUT 0
#define BAIL_ON_TIMEOUT 1
#endif /* FINAL_RELEASE */

#if PEGASOS_RELEASE
#if IBETA_RELEASE || BETA_RELEASE || FINAL_RELEASE
#error Build options are mutually exclusive !
#endif
#define EXTRA_DEBUG 0
#define BETA_VERSTRING 0
#define ALPHA_WARNING 0
#define NEED_KEYFILE 0
#define USE_KEYFILES 0
#define VAT_ECRYPT_CHECK 0
#define CHECK_TIMEOUT 1
#define BAIL_ON_TIMEOUT 1
#endif /* PEGASOS_RELEASE */

/*
 * Release flags
 */

/*
 * Turns on extra debugging (menus), Can't Happen (tm) checks,
 * MUI 3.9 subclasses naming, etc...
 */
#if !EXTRA_DEBUG
#undef EXTRA_DEBUG
#endif

/*
 * Appends "-BETA" to the version string.
 */
#if !BETA_VERSTRING
#undef BETA_VERSTRING
#endif

/*
 * Displays a "This version if for IBETA members only"
 * requester on startup.
 */
#if !ALPHA_WARNING
#undef ALPHA_WARNING
#endif

/*
 * Needs a keyfile to run.
 */
#if !NEED_KEYFILE
#undef NEED_KEYFILE
#endif

/*
 * vapor_toolkit.library is instructed to check if V is ecrypted.
 */
#if !VAT_ECRYPT_CHECK
#undef VAT_ECRYPT_CHECK
#endif

/*
 * The version does timeout.
 */
#if !CHECK_TIMEOUT
#undef CHECK_TIMEOUT
#endif

/*
 * Exits when there's a timeout.
 */
#if !BAIL_ON_TIMEOUT
#undef BAIL_ON_TIMEOUT
#endif


#if ( IBETA_RELEASE == 1 ) || defined( MBX )
#ifndef VDEBUG
#define VDEBUG 1
#endif /* VDEBUG */

/*
 * Special debugging switches
 */
#define DEBUG_DISPOSE 0 /* MUI_DisposeObject() replaced by a debug macro */
#define DEBUG_MEMORY  0 /* mungwall-like debugging, including pools. TOFIX: NYI */

#endif /* IBETA_RELEASE || MBX */

#ifdef __STDC__
struct imgclient;
struct nstream;
struct unode;
struct mcnode;
struct parsedurl;
struct cookiemsg;
struct DateStamp;
struct certmsg;
struct dnscachenode;
struct dnsmsg;
#endif


/*
 * Compiler's registers and misc stuffs
 */
#ifndef MBX
#include <macros/compilers.h>
#endif /* !MBX */

#ifdef MBX
#define VAPOR_H_BROKEN
#include "mbx.h"
#endif /* MBX */

/*
 * Features (alphabetically ordered). Don't forget
 * to update the default features list below !
 */

#if defined( MBX )

#define USE_ABOUTLIB       0
#define USE_ALPHA          1 /* MBX uses its own special alpha system by default */
#define USE_AUTOCOMPLETE   0
#define USE_BLOCKING_CONN  0
#define USE_BUSY           0
#define USE_CGX            0
#define USE_CLIPBOARD      0
#define USE_CLOCK          0
#define USE_CMANAGER       0
#define USE_CONNECT_PROC   0
#define USE_DOS            0
#define USE_DBUF_RESIZE    0
#define USE_EXECUTIVE      0
#define USE_GETSYSTIME     0
#define USE_EXTERNAL_PREFS 0
#define USE_JS             1
#define USE_JSERRORLOG     0
#define USE_LIBUNICODE     1
#define USE_LO_PIP         1
#define USE_MALLOC         1
// #define USE_MENUS          1 // NO MORE F*CKING MENUS ON MBX UNTIL YOU F*CKING GUYS FINALLY LEARN NOT TO CALL DOMETHOD(NULL) !!!!!!!!!
#define USE_MIAMI          0
#define USE_NET            1
#define USE_NETFILE        1
#define USE_NLIST          0
#define USE_PLUGINS        0
#define USE_POPHOTKEY      0
#define USE_POPPH          0
#define USE_REXX           0
#define USE_SINGLEWINDOW   1 /* this seems to be required for mbx.. why isn't it under STB_NAV? */
#define USE_SMOOTH_SCROLLING 0 /* doesn't work properly */
#define USE_SPEEDBAR       0
#define USE_SPLASHWIN      0
#define USE_SSCREEN        0
#define USE_SSL            1
#define USE_STB_NAV        1
#define USE_TEAROFF        0
#define USE_TESTFILE       0
#define USE_TMPRAS         0
#define USE_TURBOPRINT     0
#define USE_VAPOR_UPDATE   0
#define USE_VAT            0
#define USE_WBSTART        0

#elif defined( AMIGAOS )

#define USE_ABOUTLIB       1
#define USE_ALPHA          0
#define USE_AUTOCOMPLETE   1
#define USE_BLOCKING_CONN  0
#define USE_BUSY           1
#define USE_CGX            1
#define USE_CLIPBOARD      1
#define USE_CLOCK          1
#define USE_CMANAGER       1
#define USE_CONNECT_PROC   0
#define USE_DOS            1
#define USE_DBUF_RESIZE    0
#define USE_EXECUTIVE      1
#define USE_GETSYSTIME     1
#define USE_EXTERNAL_PREFS 0
#define USE_JS             1
#define USE_JSERRORLOG     1
#define USE_LIBUNICODE     0
#define USE_LO_PIP         0
#define USE_MALLOC         1
#define USE_MENUS          1
#define USE_MIAMI          1
#define USE_NET            1
#define USE_NETFILE        1
#define USE_NLIST          1
#define USE_PLUGINS        1
#define USE_POPHOTKEY      1
#define USE_POPPH          1
#define USE_REXX           1
#define USE_SINGLEWINDOW   0 /* nasty hack */
#define USE_SMOOTH_SCROLLING 0 /* doesn't work properly */
#define USE_SPEEDBAR       1
#define USE_SPLASHWIN      1
#define USE_SSCREEN        1
#define USE_SSL            1
#define USE_STB_NAV        0
#define USE_TEAROFF        1
#define USE_TESTFILE       1
#define USE_TMPRAS         1
#define USE_TURBOPRINT     1
#define USE_VAPOR_UPDATE   1
#define USE_VAT            1
#define USE_WBSTART        1

#elif defined( __MORPHOS__ )

#define USE_ABOUTLIB       1
#define USE_ALPHA          1
#define USE_AUTOCOMPLETE   1
#define USE_BLOCKING_CONN  0
#define USE_BUSY           1
#define USE_CGX            1
#define USE_CLIPBOARD      1
#define USE_CLOCK          1
#define USE_CMANAGER       1
#define USE_CONNECT_PROC   0
#define USE_DOS            1
#define USE_DBUF_RESIZE    0
#define USE_EXECUTIVE      0
#define USE_GETSYSTIME     1
#define USE_EXTERNAL_PREFS 0
#define USE_JS             1
#define USE_JSERRORLOG     1
#define USE_LIBUNICODE     0
#define USE_LO_PIP         0
#define USE_MALLOC         0 /* can't be enabled because libnix has its own malloc */
#define USE_MENUS          1
#define USE_MIAMI          0
#define USE_NET            1
#define USE_NETFILE        1
#define USE_NLIST          0
#define USE_PLUGINS        1
#define USE_POPHOTKEY      1
#define USE_POPPH          1
#define USE_REXX           1 //TOFIX!! remove rexxsyslib dependencies or nag CISC/Piru
#define USE_SINGLEWINDOW   0 /* nasty hack */
#define USE_SMOOTH_SCROLLING 0 /* doesn't work properly */
#define USE_SPEEDBAR       1
#define USE_SPLASHWIN      1
#define USE_SSCREEN        1
#define USE_SSL            1
#define USE_STB_NAV        0
#define USE_TEAROFF        0
#define USE_TESTFILE       1
#define USE_TMPRAS         1
#define USE_TURBOPRINT     1
#define USE_VAPOR_UPDATE   0 //TOFIX!!
#define USE_VAT            1
#define USE_WBSTART        1

#endif

/*
 * Special builds.
 */
#if GREX_RELEASE

#undef USE_NET
#define USE_NET 0

#undef USE_PLUGINS
#define USE_PLUGINS 0

#undef USE_CMANAGER
#define USE_CMANAGER 0

#endif


/*
 * Default features
 * 
 * Now also sorted alphabetically
 */

/*
 * Use voyager_about.vlib
 */
#if !USE_ABOUTLIB
#undef USE_ABOUTLIB
#endif

/*
 * Use alpha (CGFX V43 only)
 */
#if !USE_ALPHA
#undef USE_ALPHA
#endif

/*
 * Use auto-complete in URL gadget
 */
#if !USE_AUTOCOMPLETE
#undef USE_AUTOCOMPLETE
#endif

/*
 * Use blocking connect. Mainly for Nuc^H^H^H TCI/IP stacks with broken
 * WaitSelect()
 */
#if !USE_BLOCKING_CONN
#undef USE_BLOCKING_CONN
#endif

/*
 * Use Busy.mcc
 */
#if !USE_BUSY
#undef USE_BUSY
#endif

/*
 * Use the clipboard
 */
#if !USE_CLIPBOARD
#undef USE_CLIPBOARD
#endif

/*
 * Use CyberGraphX
 */
#if !USE_CGX
#undef USE_CGX
#endif

/*
 * Use stupid clock for RobR's ebay auctions otherwise he'll blame us
 * for losing money
 */
#if !USE_CLOCK
#undef USE_CLOCK
#endif

/*
 * Use Contact Manager as bookmarks
 */
#if !USE_CMANAGER
#undef USE_CMANAGER
#endif

/*
 * Use own connect processes. Mainly for Nuc^H^H^H TCP/IP stacks with
 * no way to put connect() in NBIO.
 */
#if !USE_CONNECT_PROC
#undef USE_CONNECT_PROC
#endif

/*
 * Use dos.library calls
 */
#if !USE_DOS
#undef USE_DOS
#endif

/*
 * Use double buffer on resize.
 */
#if !USE_DBUF_RESIZE
#undef USE_DBUF_RESIZE
#endif

/*
 * Use subset of DOS for file:/// support
 */
#if !USE_NETFILE
#undef USE_NETFILE
#endif

/*
 * Enable Executive support. Will optimize the relative priorities of
 * the different processes by using nice values instead of fixed
 * priorities.
 */
#if !USE_EXECUTIVE
#undef USE_EXECUTIVE
#endif

/*
 * Use GetSysTime()
 */
#if !USE_GETSYSTIME
#undef USE_GETSYSTIME
#endif

/*
 * Use an external MCC for the prefs
 */
#if !USE_EXTERNAL_PREFS
#undef USE_EXTERNAL_PREFS
#endif

/*
 * Enable JS (this just removes the script
 * processing, no other code, nor the DOM).
 */
#if !USE_JS
#undef USE_JS
#endif

/*
 * Enable the use of the Javascript error logfile
 */
#if !USE_JSERRORLOG
#undef USE_JSERRORLOG
#endif

/*
 * Enable keyfile check
 */
#if !USE_KEYFILES
#undef USE_KEYFILES
#endif

/*
 * Use libunicode for iso-8859-x->UTF8 conversions
 */
#if !USE_LIBUNICODE
#undef USE_LIBUNICODE
#endif

/*
 * Enable PIP functions
 */
#if !USE_LO_PIP
#undef USE_LO_PIP
#endif

/*
 * Use gadtools menu structures
 */
#if !USE_MENUS
#undef USE_MENUS
#endif

/*
 * Use the internal malloc implementation in malloc.c
 */
#if !USE_MALLOC
#undef USE_MALLOC
#endif

/*
 * Add support for backdoored TCP/IP stacks
 */
#if !USE_MIAMI
#undef USE_MIAMI
#endif

/*
 * Use network support
 */
#if !USE_NET
#undef USE_NET
#endif

/*
 * Use NList.mcc instead of List.mui if available
 */
#if !USE_NLIST
#undef USE_NLIST
#endif

/*
 * Enable external plugins (#?.VPlug) support
 */
#if !USE_PLUGINS
#undef USE_PLUGINS
#endif

/*
 * Use Pophotkey.mcc
 */
#if !USE_POPHOTKEY
#undef USE_POPHOTKEY
#endif

/*
 * Use Popplaceholder.mcc
 */
#if !USE_POPPH
#undef USE_POPPH
#endif

/*
 * Enable V's ARexx port
 */
#if !USE_REXX
#undef USE_REXX
#endif

/*
 * Use smooth scrolling. Not working
 * properly atm.
 */
#if !USE_SMOOTH_SCROLLING
#undef USE_SMOOTH_SCROLLING
#endif

/*
 * Use SpeedBar.mcc / SpeedButton.mcc
 */
#if !USE_SPEEDBAR
#undef USE_SPEEDBAR
#endif

/*
 * Use the splash window
 */
#if !USE_SPLASHWIN
#undef USE_SPLASHWIN
#endif

/*
 * Fiddles around with struct Screen
 */
#if !USE_SSCREEN
#undef USE_SSCREEN
#endif

/*
 * Use VSSL.library
 */
#if !USE_SSL
#undef USE_SSL
#endif

/*
 * Use STB navigation/GUI
 */
#if !USE_STB_NAV
#undef USE_STB_NAV
#endif

/*
 * Use TearOffBay.mcc / TearOffPanel.mcc
 */
#if !USE_TEAROFF
#undef USE_TEAROFF
#endif

/*
 * Use struct TmpRas
 */
#if !USE_TMPRAS
#undef USE_TMPRAS
#endif

/*
 * Add support for Irseesoft's Turboprint
 */
#if !USE_TURBOPRINT
#undef USE_TURBOPRINT
#endif

/*
 * Add support for massive URL testing
 */
#if !USE_TESTFILE
#undef USE_TESTFILE
#endif

/*
 * Use vapor_update.library (when available) to check for updates
 */
#if !USE_VAPOR_UPDATE
#undef VAPOR_UPDATE
#endif

/*
 * Use vapor_toolkit.library functions
 */
#if !USE_VAT
#undef USE_VAT
#endif

/*
 * Use wbstart.library to run programs in Workbench mode
 */
#if !USE_WBSTART
#undef USE_WBSTART
#endif


#endif /* VOYAGER_CONFIG_H */

