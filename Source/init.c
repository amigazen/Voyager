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


/*
**
** $Id: init.c,v 1.240 2004/11/09 15:51:16 henes Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/dos.h>
#ifdef __SASC
#include <proto/utility.h>
#endif
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <proto/icon.h>
#endif

/* private */
#include "copyright.h"
#include "voyager_cat.h"
#include "classes.h"
#include "network.h"
#include "cache.h"
#include "init.h"
#include "htmlclasses.h"
#include "splashwin.h"
#include "host_os.h"
#include "mui_func.h"
#include "malloc.h"
#include "menus.h"
#include "prefs.h"
#include <proto/vimgdecode.h>
#include "cmanager.h"
#include "dos_func.h"
#ifdef __MORPHOS__
#include "vatstart.h"
#endif /* __MORPHOS__ */
#include "js.h"
#if USE_STB_NAV
#include "mbx_menu.h"
#endif
#include "speedbar.h"
#include "textinput.h"
#include "nlist.h"
#include "popph.h"
#include "rexx.h"

// MUI object vars
APTR app, notify;

APTR menu;

#if USE_DOS
FileLock_p startupcd;
#endif

#if USE_LIBUNICODE
UnicodeData_p UnicodeBase;
#endif

#if ( USE_NET == 0 )
#define VSPEC "(NoNet) "
#elif NETCONNECT
#define VSPEC "(NetConnect)"
#else
#define VSPEC ""
#endif

char copyright[] = { "Voyager " LVERTAG " " VSPEC "(C) 1995-2003 Oliver Wagner & David Gerber, All Rights Reserved" };

int app_started;
static int app_doublestart;

/* check_md5() and VAT patching detection removed - copy protection no longer used */

/*
 * The init does far too much and causes random problems
 * when using the MUIA_Application_SingleTask way. Therefor
 * we simply do the same earlier.
 * Unfortunately we have to use arexx, so if arexx is not
 * running.. too bad
 */
int handle_doublestart( void )
{
	Forbid();
	if( FindPort( "VOYAGER" ) )
	{
		Permit();
#if USE_VAT
		VAT_SendRXMsg( "'LoadURL NEW'", VREXXPORT, VREXXEXT );
#else
		/* VAT not available - can't send Rexx message */
#endif
		app_doublestart = 1;
		return ( FALSE );
	}
	Permit();

	return( TRUE );
}


/*
 * Initialization routine. V used a CONSTRUCTOR/DESTRUCTOR scheme from SAS/C.
 * Now deprecated since it's not portable. They return TRUE on success.
 */
int initstuff( void )
{
#ifdef __MORPHOS__
	if( !init_vat() ) return( FALSE );
#endif /* !__MORPHOS__ */

#if !defined( MBX ) && defined( VDEBUG )
	init_debug();
#endif /* MBX */

	if ( !handle_doublestart() ) return( FALSE );

#if USE_LIBUNICODE
	UnicodeBase = (UnicodeData_p)OpenModule( UNICODENAME, UNICODEVERSION );
#endif

#if USE_MALLOC
	if( !init_malloc() ) return( FALSE );
#endif
	
	/*
	 * List initializations
	 */
	init_docinfolist();
#if USE_NET
	init_certreqslist();
#endif /* USE_NET */
	init_historylist();
	init_fontlist();
	init_postdatalist();
#if USE_NET
	init_cookies();
#endif /* USE_NET */
#ifndef MBX
	init_inlist();
#endif
	init_global_window_list();

#if USE_NET
	init_prunecache();
#endif /* USE_NET */
	
#ifndef MBX
	if( !init_clonelist() ) return( FALSE );
#endif /* !MBX */

	if( !init_timer() ) return( FALSE );

#if USE_KEYFILES
	init_keyname();
#endif /* USE_KEYFILES */
	if( !init_prefs() ) return( FALSE );

	save_progdir();
#if USE_NET
	init_dnscache();
#endif /* USE_NET */

	if( !init_methodstack() ) return( FALSE );
	preinit_prefs();

#if USE_KEYFILES
	load_key();
#endif /* USE_KEYFILES */

#if USE_REXX
	if( !init_ipc() ) return( FALSE );
#endif

#ifndef MBX
	if( !open_mathlibs() ) return( FALSE );
	if( !open_muimaster() ) return( FALSE );
	if( !open_vaportoolkit() ) return( FALSE );
	VoyLog(( "[INIT] After open_vaportoolkit, about to check USE_KEYFILES\n" ));
	VoyFlush();
#endif /* !MBX */

#if USE_KEYFILES
	VoyLog(( "[INIT] Calling old_loadkey()...\n" ));
	VoyFlush();
	old_loadkey();
	VoyLog(( "[INIT] old_loadkey() complete\n" ));
	VoyFlush();
#endif /* USE_KEYFILES */
#ifndef MBX
	VoyLog(( "[INIT] About to call init_fakebitmap()...\n" ));
	VoyFlush();
	init_fakebitmap();
	VoyLog(( "[INIT] After init_fakebitmap()\n" ));
	VoyFlush();
#endif /* !MBX */

#if USE_NET
	VoyLog(( "[INIT] Calling init_verify()...\n" ));
	if( !init_verify() ) { VoyLog(( "[INIT] init_verify() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] init_verify() succeeded\n" ));
#endif /* USE_NET */
#if USE_NET
	VoyLog(( "[INIT] Calling init_auth()...\n" ));
	if( !init_auth() ) { VoyLog(( "[INIT] init_auth() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] init_auth() succeeded\n" ));
#endif /* USE_NET */

	VoyLog(( "[INIT] Calling init_progdir()...\n" ));
	init_progdir();
	VoyLog(( "[INIT] Calling create_ledclass()...\n" ));
	if( !create_ledclass() ) { VoyLog(( "[INIT] create_ledclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_ledclass() succeeded\n" ));

#if USE_STB_NAV
	if( !create_crossclass() ) return( FALSE );
#endif

	VoyLog(( "[INIT] Calling init_netprocess()...\n" ));
	if( !init_netprocess() ) { VoyLog(( "[INIT] init_netprocess() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] init_netprocess() succeeded\n" ));

	VoyLog(( "[INIT] Calling init_locale()...\n" ));
	if( !init_locale() ) { VoyLog(( "[INIT] init_locale() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] init_locale() succeeded\n" ));
	VoyLog(( "[INIT] Calling init_internalipc()...\n" ));
	init_internalipc();
	VoyLog(( "[INIT] init_internalipc() complete\n" ));

	VoyLog(( "[INIT] Calling mcccheck()...\n" ));
	if( !mcccheck() ) { VoyLog(( "[INIT] mcccheck() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] mcccheck() succeeded\n" ));
#if USE_NLIST
	check_for_nlist();
#endif /* USE_NLIST	*/
	
	if( !init_css() ) return( FALSE );

	if( !create_js_object() ) return( FALSE );
	if( !create_js_objref() ) return( FALSE );
	
	if( !create_js_array() ) return( FALSE );
	if( !create_js_event() ) return( FALSE );
	if( !create_js_bool() ) return( FALSE );
	if( !create_js_date() ) return( FALSE );
	if( !create_js_func() ) return( FALSE );
	if( !create_js_link() ) return( FALSE );
	if( !create_js_location() ) return( FALSE );
	if( !create_js_math() ) return( FALSE );
	if( !create_js_mimetype() ) return( FALSE );
	if( !create_js_navigator() ) return( FALSE );
	if( !create_js_real() ) return( FALSE );
	if( !create_js_screen() ) return( FALSE );
	if( !create_js_string() ) return( FALSE );
	if( !create_js_regexp() ) return( FALSE );
#ifdef MBX
	if( !create_js_stb_root() ) return( FALSE );
	if( !create_js_stb_cdplayer() ) return( FALSE );
#endif
	if( !create_urlstringclass() ) return( FALSE );
	if( !create_ddstringclass() ) return( FALSE );
#if USE_NET
	if( !create_fastlinkclass() ) return( FALSE );
	if( !create_fastlinkgroupclass() ) return( FALSE );
#endif /* USE_NET */
	if( !create_gaugeclass() ) return( FALSE );
#if USE_STB_NAV
	if( !create_stbgaugeclass() ) return( FALSE );
#endif
#if USE_PLUGINS
	if( !create_pluginwinlistclass() ) return( FALSE );
#endif /* USE_PLUGINS */
#if !USE_EXTERNAL_PREFS
	if( !create_prefswin_mainclass() ) return( FALSE );
	if( !create_prefswin_listclass() ) return( FALSE );
	if( !create_prefswin_toolbarclass() ) return( FALSE );
	if( !create_prefswin_languagesclass() ) return( FALSE );
#if USE_NET
	if( !create_prefswin_cacheclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_prefswin_certsclass() ) return( FALSE );
#endif /* USE_NET */
	if( !create_prefswin_colorsclass() ) return( FALSE );
#if USE_NET
	if( !create_prefswin_downloadclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_prefswin_fastlinksclass() ) return( FALSE );
#endif /* USE_NET */
	if( !create_prefswin_fontsclass() ) return( FALSE );
	if( !create_prefswin_generalclass() ) return( FALSE );
	if( !create_prefswin_hyperlinksclass() ) return( FALSE );
	if( !create_prefswin_imagesclass() ) return( FALSE );
	if( !create_prefswin_javascriptclass() ) return( FALSE );
#if USE_NET
	if( !create_prefswin_mailnewsclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_prefswin_networkclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_prefswin_securityclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_prefswin_spoofclass() ) return( FALSE );
#endif /* USE_NET */
#ifndef MBX
	if( !create_prefswin_contextmenuclass() ) return( FALSE );
#endif
#endif /* !USE_EXTERNAL_PREFS */

	if( !create_commandclass() ) return( FALSE );

#if USE_CLOCK
	if( !create_clockclass() ) return( FALSE );
#endif /* USE_CLOCK */

#if USE_TEAROFF
	init_tearoff();
#endif /* USE_TEAROFF */
	
	if( !create_js_plugin() ) return( FALSE );
	if( !create_lo_form() ) return( FALSE );
	if( !create_lo_embed() ) return( FALSE );
	
#if 0
	if( !create_postwinclass() ) return( FALSE );
#endif
#if USE_NET
	if( !create_postmailwinclass() ) return( FALSE );
#endif /* USE_NET */
	if( !create_htmlviewclass() ) return( FALSE );
	if( !create_htmlwinclass() ) return( FALSE );
#if USE_DOS
	if( !load_diskobj() ) return( FALSE );
#endif /* USE_DOS */
	if( !create_amiconclass() ) return( FALSE );
	VoyLog(( "[INIT] Calling create_appclass()...\n" ));
	if( !create_appclass() ) { VoyLog(( "[INIT] create_appclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_appclass() succeeded\n" ));
#if USE_NET
	VoyLog(( "[INIT] Calling create_authbrowserwinclass()...\n" ));
	if( !create_authbrowserwinclass() ) { VoyLog(( "[INIT] create_authbrowserwinclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_authbrowserwinclass() succeeded\n" ));
#if USE_NET
	VoyLog(( "[INIT] Calling create_cookiebrowserwinclass()...\n" ));
	if( !create_cookiebrowserwinclass() ) { VoyLog(( "[INIT] create_cookiebrowserwinclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_cookiebrowserwinclass() succeeded\n" ));
#endif /* USE_NET */
	VoyLog(( "[INIT] Calling create_docinfowinclass()...\n" ));
	if( !create_docinfowinclass() ) { VoyLog(( "[INIT] create_docinfowinclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_docinfowinclass() succeeded\n" ));
#ifndef MBX
#if USE_NET
	VoyLog(( "[INIT] Calling create_downloadwinclass()...\n" ));
	if( !create_downloadwinclass() ) { VoyLog(( "[INIT] create_downloadwinclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_downloadwinclass() succeeded\n" ));
#endif /* USE_NET */
#endif
	VoyLog(( "[INIT] Calling create_errorwinclass()...\n" ));
	if( !create_errorwinclass() ) { VoyLog(( "[INIT] create_errorwinclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_errorwinclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_historylistclass()...\n" ));
	if( !create_historylistclass() ) { VoyLog(( "[INIT] create_historylistclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_historylistclass() succeeded\n" ));
#if USE_PLUGINS
	if( !create_pluginwinclass() ) return( FALSE );
#endif /* USE_PLUGINS */
#if USE_NET
	if( !create_prunecachewinclass() ) return( FALSE );
#endif /* USE_NET */
	if( !create_sourceviewclass() ) return( FALSE );
	if( !create_sizegroupclass() ) return( FALSE );
#if USE_SPLASHWIN
	if( use_splashwin )
	{
		if( !create_splashwinclass() ) return( FALSE );
	}
#endif /* USE_SPLASHWIN */
#ifndef MBX
	if( !create_smartreqclass() ) return( FALSE );
#endif /* !MBX */
	if( !create_buttonclass() ) return( FALSE );
	if( !create_toolbarclass() ) return( FALSE );

	if( !create_smartlabelclass() ) return( FALSE );
	if( !create_printwinclass() ) return( FALSE );
	if( !create_searchwinclass() ) return( FALSE );
#if USE_NET
	if( !create_authwinclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_certreqwinclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_cookiewinclass() ) return( FALSE );
#endif /* USE_NET */
#if USE_NET
	if( !create_netinfowinclass() ) return( FALSE );
#endif /* USE_NET */

//	  if( !create_scrollgroupclass(  ) ) return( FALSE );

	VoyLog(( "[INIT] Calling create_logroupclass()...\n" ));
	if( !create_logroupclass() ) { VoyLog(( "[INIT] create_logroupclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_logroupclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_lobuttonclass()...\n" ));
	if( !create_lobuttonclass() ) { VoyLog(( "[INIT] create_lobuttonclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lobuttonclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loradioclass()...\n" ));
	if( !create_loradioclass() ) { VoyLog(( "[INIT] create_loradioclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loradioclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_locheckboxclass()...\n" ));
	if( !create_locheckboxclass() ) { VoyLog(( "[INIT] create_locheckboxclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_locheckboxclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loformbuttonclass()...\n" ));
	if( !create_loformbuttonclass() ) { VoyLog(( "[INIT] create_loformbuttonclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loformbuttonclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loformtextclass()...\n" ));
	if( !create_loformtextclass() ) { VoyLog(( "[INIT] create_loformtextclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loformtextclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loformfileclass()...\n" ));
	if( !create_loformfileclass() ) { VoyLog(( "[INIT] create_loformfileclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loformfileclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loformtextfieldclass()...\n" ));
	if( !create_loformtextfieldclass() ) { VoyLog(( "[INIT] create_loformtextfieldclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loformtextfieldclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loformcycleclass()...\n" ));
	if( !create_loformcycleclass() ) { VoyLog(( "[INIT] create_loformcycleclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loformcycleclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loformhiddenclass()...\n" ));
	if( !create_loformhiddenclass() ) { VoyLog(( "[INIT] create_loformhiddenclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loformhiddenclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loform_optionclass()...\n" ));
	if( !create_loform_optionclass() ) { VoyLog(( "[INIT] create_loform_optionclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loform_optionclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_lodummyclass()...\n" ));
	if( !create_lodummyclass() ) { VoyLog(( "[INIT] create_lodummyclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lodummyclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_lotableclass()...\n" ));
	if( !create_lotableclass() ) { VoyLog(( "[INIT] create_lotableclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lotableclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loframesetclass()...\n" ));
	if( !create_loframesetclass() ) { VoyLog(( "[INIT] create_loframesetclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loframesetclass() succeeded\n" ));
#ifdef MBX
	VoyLog(( "[INIT] Calling create_pipwindowclass()...\n" ));
	if (!create_pipwindowclass()) { VoyLog(( "[INIT] create_pipwindowclass() failed!\n" )); return ( FALSE ); }
	VoyLog(( "[INIT] create_pipwindowclass() succeeded\n" ));
#endif	
	VoyLog(( "[INIT] Calling make_app()...\n" ));
	if( !make_app() ) { VoyLog(( "[INIT] make_app() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] make_app() succeeded\n" ));

/* check_md5() removed - copy protection no longer used */

#if USE_NET
	VoyLog(( "[INIT] Calling load_cookies()...\n" ));
	load_cookies();
	VoyLog(( "[INIT] load_cookies() complete\n" ));
#endif /* USE_NET */
#if USE_NET
	VoyLog(( "[INIT] Calling load_auths()...\n" ));
	load_auths();
	VoyLog(( "[INIT] load_auths() complete\n" ));
#endif /* USE_NET */
	VoyLog(( "[INIT] Calling init_tokenbuff()...\n" ));
	init_tokenbuff();
	VoyLog(( "[INIT] init_tokenbuff() complete\n" ));

	VoyLog(( "[INIT] Calling find_host_os()...\n" ));
	find_host_os();
	VoyLog(( "[INIT] find_host_os() complete\n" ));

	VoyLog(( "[INIT] Calling init_imgdec()...\n" ));
	if( !init_imgdec() ) { VoyLog(( "[INIT] init_imgdec() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] init_imgdec() succeeded\n" ));

	VoyLog(( "[INIT] Calling create_frameborderclass()...\n" ));
	if( !create_frameborderclass() ) { VoyLog(( "[INIT] create_frameborderclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_frameborderclass() succeeded\n" ));
	
	VoyLog(( "[INIT] Calling init_keyname2()...\n" ));
	if( init_keyname2() ) { VoyLog(( "[INIT] init_keyname2() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] init_keyname2() succeeded\n" ));
	
	VoyLog(( "[INIT] Calling create_fonttestclass()...\n" ));
	if( !create_fonttestclass() ) { VoyLog(( "[INIT] create_fonttestclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_fonttestclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_frameclass()...\n" ));
	if( !create_frameclass() ) { VoyLog(( "[INIT] create_frameclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_frameclass() succeeded\n" ));

	VoyLog(( "[INIT] Calling probe_mimeprefs()...\n" ));
	probe_mimeprefs();
	VoyLog(( "[INIT] probe_mimeprefs() complete\n" ));
	VoyLog(( "[INIT] Calling create_lohrclass()...\n" ));
	if( !create_lohrclass() ) { VoyLog(( "[INIT] create_lohrclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lohrclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loliclass()...\n" ));
	if( !create_loliclass() ) { VoyLog(( "[INIT] create_loliclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loliclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loanchorclass()...\n" ));
	if( !create_loanchorclass() ) { VoyLog(( "[INIT] create_loanchorclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loanchorclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_lomarginclass()...\n" ));
	if( !create_lomarginclass() ) { VoyLog(( "[INIT] create_lomarginclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lomarginclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_lomapclass()...\n" ));
	if( !create_lomapclass() ) { VoyLog(( "[INIT] create_lomapclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lomapclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loareaclass()...\n" ));
	if( !create_loareaclass() ) { VoyLog(( "[INIT] create_loareaclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loareaclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_loimageclass()...\n" ));
	if( !create_loimageclass() ) { VoyLog(( "[INIT] create_loimageclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_loimageclass() succeeded\n" ));
#if USE_LO_PIP
	VoyLog(( "[INIT] Calling create_lopipclass()...\n" ));
	if( !create_lopipclass() ) { VoyLog(( "[INIT] create_lopipclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lopipclass() succeeded\n" ));
#endif
	VoyLog(( "[INIT] Calling create_lodivclass()...\n" ));
	if( !create_lodivclass() ) { VoyLog(( "[INIT] create_lodivclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lodivclass() succeeded\n" ));
	VoyLog(( "[INIT] Calling create_lobrclass()...\n" ));
	if( !create_lobrclass() ) { VoyLog(( "[INIT] create_lobrclass() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] create_lobrclass() succeeded\n" ));

	VoyLog(( "[INIT] Calling init_history()...\n" ));
	init_history();
	VoyLog(( "[INIT] init_history() complete\n" ));

	VoyLog(( "[INIT] Calling start_image_decoders()...\n" ));
	if( !start_image_decoders() ) { VoyLog(( "[INIT] start_image_decoders() failed!\n" )); return( FALSE ); }
	VoyLog(( "[INIT] start_image_decoders() succeeded\n" ));

	VoyLog(( "[INIT] Calling init_memhandler()...\n" ));
	init_memhandler();
	VoyLog(( "[INIT] init_memhandler() complete\n" ));
#if USE_KEYFILES
	start_demotimeout();
#endif /* USE_KEYFILES */
#if USE_VAPOR_UPDATE
	check_update();
#endif /* USE_VAPOR_UPDATE */
#if CHECK_TIMEOUT
#ifndef MBX
	if( !check_timeout() ) return( FALSE );
#endif
#endif /* CHECK_TIMEOUT */

#if USE_NET
	start_prunecache();
#endif /* USE_NET */
#if USE_PLUGINS
	init_plugins();
#endif /* USE_PLUGINS */

	D( db_init, bug( "all right, initstuff() succeeded\n" ) );
	VoyLog(( "[INIT] initstuff() complete, returning TRUE\n" ));

	return( TRUE );
}


/*
 * Shutdown function
 */
void closestuff( void )
{
	if( app_doublestart )
	{
#ifdef __MORPHOS__
		close_vat();
#endif /* __MORPHOS__ */
		return;
	}

	// This moved onto top
	cleanup_methodstack();
	cleanup_windows();
	cleanup_memhandler();
	save_history();
#if USE_VAPOR_UPDATE
	close_vapor_update();
#endif
#if USE_KEYFILES
	cleanup_demotimeout();
#endif /* USE_KEYFILE */
#if USE_NET
	free_autoproxy();
#endif /* USE_NET */
#if USE_NET
	save_authentications();
#endif /* USE_NET */
#if USE_NET
	save_cookies();
#endif /* USE_NET */

	js_gc_cleanup();

#ifdef MBX
	// This is a bit "out of sync", but
	// some of the STB objects need a
	// valid "app" pointer during
	// deinitialization
	delete_js_stb_root();
	delete_js_stb_cdplayer();
#endif
	close_app();
	close_image_decoders();

	cleanup_cachebitmap();
	
	D( db_init, bug( "deleting custom classes..\n" ) );
#ifdef MBX
	delete_pipwindowclass();
#endif
	delete_lobrclass();
	delete_lodivclass();
#if USE_LO_PIP
	delete_lopipclass();
#endif
	delete_loimageclass();
	delete_loareaclass();
	delete_lomapclass();
	delete_lomarginclass();
	delete_loanchorclass();
	delete_loliclass();
	delete_lohrclass();
	delete_locheckboxclass();
	delete_loradioclass();
	delete_lobuttonclass();
	delete_loformbuttonclass();
	delete_loformtextfieldclass();
	delete_loformfileclass();
	delete_loformtextclass();
	delete_loformcycleclass();
	delete_loform_optionclass();
	delete_loformhiddenclass();
	
	delete_frameclass();
	delete_fonttestclass();
	delete_frameborderclass();

#if USE_NET
	delete_netinfowinclass();
#endif /* USE_NET */
#if USE_NET
	delete_cookiewinclass();
#endif /* USE_NET */
#if USE_NET
	delete_certreqwinclass();
#endif /* USE_NET */
#if USE_NET
	delete_authwinclass();
#endif /* USE_NET */
	delete_searchwinclass();
	delete_printwinclass();
	delete_smartlabelclass();
	delete_toolbarclass();
	delete_buttonclass();
#ifndef MBX
	delete_smartreqclass();
#endif /* !MBX */
#if USE_SPLASHWIN
	if( use_splashwin )
	{
		delete_splashwinclass();
	}
#endif /* USE_SPLASHWIN */
	delete_sizegroupclass();
	delete_sourceviewclass();
#if USE_NET
	delete_prunecachewinclass();
#endif /* USE_NET */
#if USE_PLUGINS
	delete_pluginwinclass();
#endif /* USE_PLUGINS */
	delete_historylistclass();
	delete_errorwinclass();
#ifndef MBX
#if USE_NET
	delete_downloadwinclass();
#endif /* USE_NET */
#endif
	delete_docinfowinclass();
#if USE_NET
	delete_cookiebrowserwinclass();
	delete_authbrowserwinclass();
#endif /* USE_NET */
	delete_appclass();
	delete_amiconclass();
	
	delete_htmlwinclass();
	delete_htmlviewclass();
#if USE_NET
	delete_postmailwinclass();
#endif /* USE_NET */
#if 0
	delete_postwinclass();
#endif
#if USE_DOS
	free_diskobject();
#endif /* USE_DOS */

	delete_loframesetclass();
	delete_lotableclass();
	delete_lodummyclass();
	delete_logroupclass();
	
//	  delete_scrollgroupclass();

	delete_lo_embed();
	delete_lo_form();
	delete_js_plugin();
	delete_urlstringclass();
	delete_js_regexp();
	delete_js_string();
	delete_js_screen();
	delete_js_real();
	delete_js_navigator();
	delete_js_mimetype();
	delete_js_math();
	delete_js_location();
	delete_js_link();
	delete_js_func();
	delete_js_date();
	delete_js_bool();
	delete_js_event();
	delete_js_array();
#if USE_CLOCK
	delete_clockclass();
#endif /* USE_CLOCK */

	delete_commandclass();

#if !USE_EXTERNAL_PREFS
#ifndef MBX
	delete_prefswin_contextmenuclass();
#endif
	delete_prefswin_languagesclass();
	delete_prefswin_toolbarclass();
#if USE_NET
	delete_prefswin_cacheclass();
#endif /* USE_NET */
#if USE_NET
	delete_prefswin_certsclass();
#endif /* USE_NET */
	delete_prefswin_colorsclass();
#if USE_NET
	delete_prefswin_downloadclass();
#endif /* USE_NET */
#if USE_NET
	delete_prefswin_fastlinksclass();
#endif /* USE_NET */
	delete_prefswin_fontsclass();
	delete_prefswin_generalclass();
	delete_prefswin_hyperlinksclass();
	delete_prefswin_imagesclass();
	delete_prefswin_javascriptclass();
#if USE_NET
	delete_prefswin_mailnewsclass();
#endif /* USE_NET */
#if USE_NET
	delete_prefswin_networkclass();
#endif /* USE_NET */
#if USE_NET
	delete_prefswin_securityclass();
#endif /* USE_NET */
#if USE_NET
	delete_prefswin_spoofclass();
#endif /* USE_NET */
	delete_prefswin_listclass();
	delete_prefswin_mainclass();
#endif /* !USE_EXTERNAL_PREFS */
#if USE_PLUGINS
	delete_pluginwinlistclass();
#endif /* USE_PLUGINS */
	delete_gaugeclass();
#if USE_STB_NAV
	delete_stbgaugeclass();
#endif
#if USE_NET
	delete_fastlinkgroupclass();
	delete_fastlinkclass();
#endif /* USE_NET */
	delete_ddstringclass();
	delete_js_objref();
	delete_js_object();

	cleanup_css();

#if USE_PLUGINS
	cleanup_plugins();
#endif /* USE_PLUGINS */
#if USE_TEAROFF
	cleanup_tearoff();
#endif /* USE_TEAROFF */
#ifndef MBX
	cleanup_clonelist();
	cleanup_inlist();
#endif /* !MBX */
	cleanup_historylist();

#if USE_WBSTART
	cleanup_wbstart();
#endif /* USE_WBSTART */

	cleanup_internalipc();
	close_locale();
	cleanup_netprocess();

#if USE_STB_NAV
	delete_crossclass();
#endif

	delete_ledclass();

#if USE_NET
	cleanup_authentications();
#endif /* USE_NET */
#if USE_NET
	cleanup_verify();
#endif /* USE_NET */
	restore_progdir();
	cleanup_fonts();

	cleanup_alloca();

#if USE_CGX
	close_cybergfx();
#endif /* USE_CGX */
#ifndef MBX
	close_vaportoolkit();
	close_muimaster();
	close_mathlibs();
#endif /* !MBX */

#if USE_NET
	free_cachelocks();
#endif /* USE_NET */
#if USE_REXX
	cleanup_ipc();
#endif
	cleanup_prefs();
#if USE_NET
	cleanup_dnscache();
#endif /* USE_NET */
	cleanup_timer();

#if USE_MALLOC
	cleanup_malloc();
#endif

#if USE_LIBUNICODE
	CloseModule( (Module_p)UnicodeBase );
#endif

#ifdef __MORPHOS__
	close_vat();
#endif /* __MORPHOS__ */
}

//
//  Open MUIMaster
//
#ifndef MBX
struct Library *MUIMasterBase;
struct Library *MUIGfxBase;
struct Library *VaporToolkitBase;

/*
 * The image decoder used to be a shared library and still defines the SAS/C
 * math library bases itself. Because those globals now live in this
 * executable, SAS/C's own auto-open module is not linked in, and the decoder's
 * __UserLibInit (which used to open them) is a no-op under static linking.
 * Nothing opened them, so the first IEEE double operation - the table column
 * width maths in lo_table.c - jumped through a NULL base and took the machine
 * down. Open them here, before any layout can run.
 */
#ifdef _FFP
extern struct Library *MathBase;
#endif
#if defined( _IEEE ) && !defined( __MORPHOS__ )
extern struct Library *MathIeeeDoubBasBase;
#endif

int open_mathlibs( void )
{
#ifdef _FFP
	if( !MathBase )
		MathBase = OpenLibrary( "mathffp.library", 0 );
	if( !MathBase )
	{
		VoyLog(( "[INIT] ERROR: cannot open mathffp.library\n" ));
		VoyFlush();
		return( FALSE );
	}
#endif

#if defined( _IEEE ) && !defined( __MORPHOS__ )
	if( !MathIeeeDoubBasBase )
		MathIeeeDoubBasBase = OpenLibrary( "mathieeedoubbas.library", 0 );
	if( !MathIeeeDoubBasBase )
	{
		VoyLog(( "[INIT] ERROR: cannot open mathieeedoubbas.library\n" ));
		VoyFlush();
		return( FALSE );
	}
#endif

	return( TRUE );
}

void close_mathlibs( void )
{
#if defined( _IEEE ) && !defined( __MORPHOS__ )
	if( MathIeeeDoubBasBase )
	{
		CloseLibrary( MathIeeeDoubBasBase );
		MathIeeeDoubBasBase = NULL;
	}
#endif
#ifdef _FFP
	if( MathBase )
	{
		CloseLibrary( MathBase );
		MathBase = NULL;
	}
#endif
}

int open_muimaster( void )
{
	struct EasyStruct eas;
	
	D( db_init, bug( "initializing..\n" ) );

	while( 1 )
	{
		MUIMasterBase = (void *)OpenLibrary( MUIMASTER_NAME, 18 );
		MUIGfxBase = (void*)OpenLibrary( "muigfx.library", 1 );

		if( MUIMasterBase && MUIGfxBase )
		{
#if ALPHA_WARNING
			char dummy[ 32 ];

			D( db_init, bug( "muimaster.library opened\n" ) );

			if( GetVar( "I_KNOW_V_IS_IBETA", dummy, sizeof( dummy ), 0 ) <= 0 )
				if( !MUI_Request( 0, 0, 0, "Voyager", "Start|Cancel", "This is a *internal* test version not for distribution.\nDo NOT use it unless you are an IBETA member.\n\nPlease inform <owagner@vapor.com> immediately\nif you have found this file on a public site or IRC net." ) )
					return( FALSE );
#endif /* ALPHA_WARNING */
			return( TRUE );
		}
		eas.es_StructSize = sizeof( eas );
		eas.es_Flags = 0;
		eas.es_Title = copyright;
		eas.es_TextFormat = "" APPNAME " requires at least V18\nof \"" MUIMASTER_NAME "\"\nPlease install MUI 3.7 or higher!";
		eas.es_GadgetFormat = "Retry|Cancel";
		if( !EasyRequest( NULL, &eas, NULL ) )
			return( FALSE );
   }
}

void close_muimaster( void )
{
	if( MUIMasterBase )
	{
		D( db_init, bug( "cleaning up..\n" ) );

		CloseLibrary( MUIGfxBase );
		CloseLibrary( MUIMasterBase );
	}
}

int open_vaportoolkit( void )
{
	char libpath[ 256 ];
	
	D( db_init, bug( "opening vapor_toolkit.library..\n" ) );
	
	/* Construct path: PROGDIR:libs/vapor_toolkit.library */
	strncpy( libpath, "PROGDIR:Libs/vapor_toolkit.library", sizeof(libpath) - 1 );
	libpath[ sizeof(libpath) - 1 ] = '\0';
	
	VaporToolkitBase = OpenLibrary( libpath, 0 );
	if( !VaporToolkitBase )
	{
		D( db_init, bug( "failed to open vapor_toolkit.library\n" ) );
		VoyLog(( "[INIT] WARNING: Could not open %s\n", libpath ));
		VoyFlush();
		return( FALSE );
	}
	
	D( db_init, bug( "vapor_toolkit.library opened successfully\n" ) );
	VoyLog(( "[INIT] vapor_toolkit.library opened successfully\n" ));
	VoyFlush();
	return( TRUE );
}

close_vaportoolkit( )
{
	if( VaporToolkitBase )
	{
		D( db_init, bug( "closing vapor_toolkit.library..\n" ) );
		CloseLibrary( VaporToolkitBase );
		VaporToolkitBase = NULL;
	}
}
#endif

static __far struct mccchk {
	STRPTR name;
	int minver, minrev;
} mccs[] = {
	{ MUIC_Textinput      , 29, 0 },
	{ MUIC_Textinputscroll, 29, 0 },
	{ "Listtree.mcc"      , 15, 0 }, /* XXX: not sure about the version.. check */
	{ "Busy.mcc"          , 16, 0 }, /* XXX: not sure about the version.. check */
#if USE_SPEEDBAR
	{ MUIC_SpeedBar       , 11, 5 },
	{ MUIC_SpeedButton    , 11, 0 },
#endif /* USE_SPEEDBAR */
#if USE_TEAROFF
	{ "TearOffPanel.mcc"  , 13, 5 },
	{ "TearOffBay.mcc"    , 13, 5 },
#endif /* USE_TEAROFF */
#if USE_POPPH
	{ MUIC_Popplaceholder , 14, 5 },
#endif /* USE_POPPH */
	//"CompactWindow.mcc", 12, 6,
	{ NULL }
};

/*
 * Opens all the MCC in the mccs[] array and shows a requester
 * with classes causing problems.
 */
#define MCCCHK_OK 0
#define MCCCHK_FAILED 1
#define MCCCHK_MISSING 2
int mcccheck( void )
{
	int c;
	char message[ 1024 ];
	int error = 0;
	int status = 0;
	char verinfo[ 64 ];
	APTR o;
	int ver;
	int rev;
	char *msgend;
	STRPTR classname;
	STRPTR altname;
	char altnamebuf[ 128 ];
	ULONG len;
	
	D( db_init, bug( "initializing..\n" ) );
	VoyLog(( "[MCC] mcccheck() starting...\n" ));
	VoyFlush();

	strcpy( message, voyager_catalog_str( MSG_MCCCHECK_TITLE, MSG_MCCCHECK_TITLE_STR ) );

	for( c = 0; mccs[ c ].name; c++ )
	{
		VoyLog(( "[MCC] Checking class: %s (required: v%ld.%ld)\n", mccs[ c ].name, (long)mccs[ c ].minver, (long)mccs[ c ].minrev ));
		VoyFlush();
		
		classname = mccs[ c ].name;
		o = MUI_NewObject( classname, TAG_DONE );
		
		/* If failed and name ends with .mcc, try without extension */
		if( !o && classname != NULL )
		{
			len = strlen( classname );
			if( len > 4 && !strcmp( classname + len - 4, ".mcc" ) )
			{
				strncpy( altnamebuf, classname, len - 4 );
				altnamebuf[ len - 4 ] = '\0';
				altname = altnamebuf;
				VoyLog(( "[MCC] Trying alternative name: %s\n", altname ));
				VoyFlush();
				o = MUI_NewObject( altname, TAG_DONE );
			}
		}
		
		if( o )
		{
			ver = getv( o, MUIA_Version );
			rev = getv( o, MUIA_Revision );
#ifdef __SASC
			SNPrintf( verinfo, sizeof(verinfo), "%d.%d", ver, rev );
#else
			sprintf( verinfo, "%d.%d", ver, rev );
#endif

			VoyLog(( "[MCC] Class %s found: v%s\n", mccs[ c ].name, verinfo ));
			VoyFlush();
			D( db_init, bug( "creation of of %s (%s) successfull\n", mccs[ c ].name, verinfo ) );

			if( ver < mccs[ c ].minver || ( ver == mccs[ c ].minver && rev < mccs[ c ].minrev ) )
			{
				VoyLog(( "[MCC] ERROR: Class %s version %s is too old (required: v%ld.%ld)\n", mccs[ c ].name, verinfo, (long)mccs[ c ].minver, (long)mccs[ c ].minrev ));
				VoyFlush();
				error++;
				status = MCCCHK_FAILED;
			}
			else
			{
				VoyLog(( "[MCC] Class %s version OK\n", mccs[ c ].name ));
				VoyFlush();
				status = MCCCHK_OK;
			}
			
			MUI_DisposeObject( o );
		}
		else
		{
			VoyLog(( "[MCC] ERROR: Class %s not found!\n", mccs[ c ].name ));
			VoyFlush();
			D( db_init, bug( "creation of %s failed\n", mccs[ c ].name ) );
			strcpy( verinfo, "-" );
			error++;
			status = MCCCHK_MISSING;
		}

		msgend = strchr( message, 0 );
#ifdef __SASC
		SNPrintf( msgend, sizeof(message) - (msgend - message), GS( MCCCHECK_LINE ),
			mccs[ c ].name, mccs[ c ].minver, mccs[ c ].minrev,
			verinfo
		);
#else
		sprintf( msgend, GS( MCCCHECK_LINE ),
			mccs[ c ].name, mccs[ c ].minver, mccs[ c ].minrev,
			verinfo
		);
#endif
		
		switch ( status )
		{
			case MCCCHK_OK:
				msgend = strchr( message, 0 );
#ifdef __SASC
				SNPrintf( msgend, sizeof(message) - (msgend - message), ": %s", GS( MCCCHECK_GOOD ) );
#else
				sprintf( msgend, ": %s", GS( MCCCHECK_GOOD ) );
#endif
				break;

			case MCCCHK_FAILED:
				msgend = strchr( message, 0 );
#ifdef __SASC
				SNPrintf( msgend, sizeof(message) - (msgend - message), ": %s", GS( MCCCHECK_BAD ) );
#else
				sprintf( msgend, ": %s", GS( MCCCHECK_BAD ) );
#endif
				break;

			case MCCCHK_MISSING:
				msgend = strchr( message, 0 );
#ifdef __SASC
				SNPrintf( msgend, sizeof(message) - (msgend - message), ": %s", GS( MCCCHECK_MISSING ) );
#else
				sprintf( msgend, ": %s", GS( MCCCHECK_MISSING ) );
#endif
				break;
		}
	}

	if( error )
	{
		VoyLog(( "[MCC] ERROR: %ld class(es) failed check\n", (long)error ));
		VoyLog(( "[MCC] Error message: %s\n", message ));
		VoyFlush();
		MUI_Request( NULL, NULL, 0,
			(char*)voyager_catalog_str( MSG_ERROR, MSG_ERROR_STR ),
			(char*)voyager_catalog_str( MSG_CANCEL, MSG_CANCEL_STR ),
			message, 0 ); 
		return( FALSE );
	}

	VoyLog(( "[MCC] All classes OK!\n" ));
	VoyFlush();
	return( TRUE );
}

//
//  Startup parsing..
//

extern struct WBStartup * _WBenchMsg;
struct RDArgs *rda;
struct myargs {
	ULONG iconified;
	char **urls;
	char *mimetype;
	char *configfile;
	ULONG nosplashwin;
#if USE_TESTFILE
	char *testfile;
#endif
} myargs;
ULONG startup_iconified;
#ifdef MBX
ULONG use_mcp;
ULONG iconifySignalsMCP;
#endif
char startup_cfgfile[ 256 ];
#if USE_TESTFILE
BPTR urltestfile;
#endif
char myfullpath[ 256 ];
struct DiskObject *diskobj;
char **openurls = NULL; /* Initialize to NULL to avoid address errors */

/* arguments */
#if USE_TESTFILE
#define RDARGS "ICONIFIED=ICONIFY/S,URL/M,MIMETYPE=MT/K,CONFIG=PREFS/K,NOSPLASHWIN/S,URLTESTFILE/K"
#else
#define RDARGS "ICONIFIED=ICONIFY/S,URL/M,MIMETYPE=MT/K,CONFIG=PREFS/K,NOSPLASHWIN/S"
#endif

#if !defined( MBX )
int load_diskobj( void )
{
	char progname[ 128 ];
	int c;
	struct Process *pr = (APTR)FindTask( 0 );
	APTR oldwinptr = pr->pr_WindowPtr;
	FileLock_p programcd;
	
	D( db_init, bug( "loading diskobj and parsing args\n" ) );

	pr->pr_WindowPtr = (APTR)-1;

	strcpy( startup_cfgfile, "PROGDIR:Voyager.prefs" );

	if( !_WBenchMsg )
	{
		// Shell-Startup
		rda = ReadArgs( RDARGS, (LONG*)&myargs, NULL );
		if( !rda )
		{
			PrintFault( IoErr(), "" APPNAME "" );
			return( FALSE );
		}

		if( myargs.configfile )
			strcpy( startup_cfgfile, myargs.configfile );

		// convert URLs
		if( myargs.urls )
		{
			for( c = 0; myargs.urls[ c ]; c++ );

			openurls = malloc( c * 4 + 4 );
			if( openurls )
			{
				memset( openurls, '\0', c * 4 + 4 ); /* TOFIX: maybe not needed */
				for( c = 0; myargs.urls[ c ]; c++ )
				{
					FileLock_p l = Lock( myargs.urls[ c ], SHARED_LOCK );
					char *pre = NULL;
					if( l )
					{
						UnLock( l );
						pre = "file:///";
					}
					else if( !url_hasscheme( myargs.urls[ c ] ) )
						pre = "http://";
					if( pre && ( openurls[ c ] = malloc( strlen( myargs.urls[ c ] ) + strlen( pre ) + 1 ) ) )
						sprintf( openurls[ c ], "%s%s", pre, myargs.urls[ c ] );
					else
						openurls[ c ] = myargs.urls[ c ];
				}
			}
		}

#if USE_TESTFILE
		if( myargs.testfile )
			urltestfile = Open( myargs.testfile, MODE_OLDFILE );
#endif
		startup_iconified = myargs.iconified;
#if USE_SPLASHWIN
		use_splashwin = !myargs.nosplashwin;
#endif /* USE_SPLASHWIN */
		NameFromLock( GetProgramDir(), myfullpath, sizeof( myfullpath ) );
		
		// switch to the current directory so everything loaded is relative to it (images, etc..)
		if( programcd = Lock( myfullpath, SHARED_LOCK ) )
			startupcd = CurrentDir( programcd );
		
		GetProgramName( progname, sizeof( progname ) );
		AddPart( myfullpath, progname, sizeof( myfullpath ) );
		diskobj = GetDiskObject( startup_cfgfile );
		if( !diskobj )
			diskobj = GetDiskObjectNew( myfullpath );
	}
	else
	{
		// WBStartup
		struct WBArg *wbarg;
		char fullpath[ 256 ];
		//int num = _WBenchMsg->sm_NumArgs > 1 ? 1 : 0;
		int num = _WBenchMsg->sm_NumArgs;
		char *tt = NULL;
		int cnt = 0;

		wbarg = _WBenchMsg->sm_ArgList;

		NameFromLock( wbarg->wa_Lock, myfullpath, 256 );
		AddPart( myfullpath, wbarg->wa_Name, 256 );

		// load program icon
		do
		{
			BPTR f;

			if( diskobj )
				FreeDiskObject( diskobj );

			NameFromLock( wbarg->wa_Lock, fullpath, 256 );
			AddPart( fullpath, wbarg->wa_Name, 256 );
			diskobj = GetDiskObjectNew( fullpath );
			
			/*
			 * We skip the first icon because it's ours
			 * then process every one that has been pressed
			 * with shift.
			 * If there's a prefsfile, load it. Otherwise,
			 * load them into new windows.
			 */
			f = Open( fullpath, MODE_OLDFILE );
			if( f )
			{
				ULONG id = 0;

#define PREFSFILEID MAKE_ID('V','Y','A',0)
				Read( f, &id, sizeof( id ) );
				Close( f );

				if( cnt++ )
				{
					if( id == PREFSFILEID )
					{
						strcpy( startup_cfgfile, fullpath );
					}
					else
					{
						int c;

						if( !openurls )
						{
							openurls = malloc( 64 * 4 );
						}

						if( openurls )
						{
							memset( openurls, '\0', 64 * 4 );

							for( c = 0; c < 64; c++ )
							{
								if( !openurls[ c ] )
								{
									char buffer[ 300 ];

									sprintf( buffer, "file://localhost/%s", fullpath );
									openurls[ c ] = strdup( buffer ); /* TOFIX */
									break;
								}
							}
						}
					}
				}
			}

			if( !( tt = FindToolType( diskobj->do_ToolTypes, "ICONIFY" ) ) )
			{
				tt = FindToolType( diskobj->do_ToolTypes, "ICONIFIED" );
			}

			if( tt )
			{
				startup_iconified = TRUE;
			}

			if( FindToolType( diskobj->do_ToolTypes, "NOSPLASHWIN" ) )
			{
#if USE_SPLASHWIN
				use_splashwin = FALSE;
#endif /* USE_SPLASHWIN */
			}

			wbarg++;

		} while( --num );
	}

	pr->pr_WindowPtr = oldwinptr;

	return( TRUE );
}

void free_diskobject( void )
{
	if( diskobj )
		FreeDiskObject( diskobj );
	if( startupcd )
		UnLock( CurrentDir( startupcd ) );
#if USE_TESTFILE
	if( urltestfile )
		Close( urltestfile );
#endif
}
#endif /* MBX */

#if USE_MENUS

#define MENU(x) (STRPTR)MSG_MENU_##x

/*
	Shortcut usage:

	ABCDEFGHIJKLMNOPQRSTUVWXYZ?
	 X XXXX XXX XXX XXX
*/

static __far struct NewMenu newmenus[] = {
#ifdef MBX
NM_TITLE, MENU(BROWSER),                0,   0, 0, 0,
#else
NM_TITLE, MENU(AMBROWSE),               0,   0, 0, 0,
#endif
 NM_ITEM, MENU(OPENFILE),               "O", 0, 0, (APTR)MENU_OPENFILE,
 NM_ITEM, MENU(SAVEHTML),               "S",   NM_ITEMDISABLED, 0, (APTR)MENU_SAVEHTML,
 NM_ITEM, MENU(SAVETEXT),               0,   NM_ITEMDISABLED, 0, (APTR)MENU_SAVETEXT,
 NM_ITEM, MENU(WIN_PRINT),              "P",   NM_ITEMDISABLED, 0, (APTR)MENU_WIN_PRINT,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(ABOUT),                  "?",  0, 0, (APTR)MENU_ABOUT,
 NM_ITEM, MENU(ABOUTMUI),               0,   0, 0, (APTR)MENU_ABOUTMUI,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(ICONIFY),                "I",  0, 0, (APTR)MENU_ICONIFY,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(QUIT),                   "Q",  0, 0, (APTR)MUIV_Application_ReturnID_Quit,

NM_TITLE, MENU(EDIT),                   0,   0, 0, (APTR)MENU_EDIT,
 NM_ITEM, MENU(WIN_FIND),               "F", NM_ITEMDISABLED, 0, (APTR)MENU_WIN_FIND,

NM_TITLE, MENU(WINDOWS),                0,   0, 0, (APTR)MENU_WINDOWS,
 NM_ITEM, MENU(NEWWIN),                 "N", 0, 0, (APTR)MENU_WIN_NEW,
 NM_ITEM, MENU(CLOSEWIN),				0,	 0, 0, (APTR)MENU_WIN_CLOSE,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(SETWINSIZE),				0,   0, 0, (APTR)MENU_SETWINSIZE,
  NM_SUB, MENU(SETWINSIZE_1),           0,   0, 0, (APTR)MENU_SETWINSIZE_1,
  NM_SUB, MENU(SETWINSIZE_2),           0,   0, 0, (APTR)MENU_SETWINSIZE_2,
  NM_SUB, MENU(SETWINSIZE_3),           0,   0, 0, (APTR)MENU_SETWINSIZE_3,
 NM_ITEM, MENU(SETWINSIZE_FS),			"U", 0, 0, (APTR)MENU_SETWINSIZE_FS,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
#if USE_NET
 NM_ITEM, MENU(NETINFO),                0,   0, 0, (APTR)MENU_NETINFO,
#endif /* USE_NET */
#if USE_NET
 NM_ITEM, MENU(DOWNLOADS),              "D",   0, 0, (APTR)MENU_DOWNLOADS,
#endif /* USE_NET */
 NM_ITEM, MENU(JSSNOOP),                "J",   NM_ITEMDISABLED, 0, (APTR)MENU_JSSNOOP,
#if USE_NET
 NM_ITEM, MENU(COOKIEBROWSER),          0,   0, 0, (APTR)MENU_COOKIEBROWSER,
 NM_ITEM, MENU(AUTHBROWSER),            0,   0, 0, (APTR)MENU_AUTHBROWSER,
#endif /* USE_NET */
 NM_ITEM, MENU(ERRORWIN),               0,   0, 0, (APTR)MENU_ERRORWIN,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(WIN_SOURCE),             0,   0, 0, (APTR)MENU_WIN_SOURCE,
 NM_ITEM, MENU(WIN_DOCINFOWIN),         0,   0, 0, (APTR)MENU_WIN_DOCINFOWIN,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,

#if USE_CMANAGER
NM_TITLE, MENU(BOOKMARKS),              0,   0, 0, (APTR)MENU_BM,
 NM_ITEM, MENU(GOTOBOOK),               "G",  0, 0, (APTR)MENU_BM_GOTO,
 NM_ITEM, MENU(BOOKMARKS_EDIT),         "B",  0, 0, (APTR)MENU_BM_EDIT,
 NM_ITEM, MENU(BOOKMARKS_SEARCH),       0, 0, 0, (APTR)MENU_BM_SEARCH,
 NM_ITEM, NM_BARLABEL,                  0, 0, 0, (APTR)0,
 NM_ITEM, MENU(BOOK_IMPORT),            0, 0, 0, (APTR)0,
  NM_SUB, MENU(BOOK_IMPORT_VOYAGER),    0, 0, 0, (APTR)MENU_BM_IMPORT_VOYAGER,
  NM_SUB, MENU(BOOK_IMPORT_IBROWSE),    0, 0, 0, (APTR)MENU_BM_IMPORT_IBROWSE,
  NM_SUB, MENU(BOOK_IMPORT_AWEB),       0, 0, 0, (APTR)MENU_BM_IMPORT_AWEB,
 NM_ITEM, MENU(BOOK_EXPORT),            0, 0, 0, (APTR)0,
  NM_SUB, MENU(BOOK_EXPORT_WWW),        0, 0, 0, (APTR)MENU_BM_EXPORT_WWW,
  NM_SUB, MENU(BOOK_EXPORT_FTP),        0, 0, 0, (APTR)MENU_BM_EXPORT_FTP,
  NM_SUB, MENU(BOOK_EXPORT_WWW_AND_FTP),0, 0, 0, (APTR)MENU_BM_EXPORT_WWW_AND_FTP,
 NM_ITEM, NM_BARLABEL,                  0, 0, 0, (APTR)0,
 NM_ITEM, MENU(BOOK_LOAD ),             0, 0, 0, (APTR)MENU_BM_LOAD,
 NM_ITEM, MENU(BOOK_SAVE ),             0, 0, 0, (APTR)MENU_BM_SAVE,
 NM_ITEM, MENU(BOOK_SAVEAS ),           0, 0, 0, (APTR)MENU_BM_SAVEAS,
#endif /* USE_CMANAGER */

NM_TITLE, MENU(CACHE),                  0,  0, 0, (APTR)MENU_CACHE,
#if USE_NET
 NM_ITEM, MENU(CACHECONTENTS),          0,  0, 0, (APTR)MENU_CACHECONTENTS,
 NM_ITEM, NM_BARLABEL,                  0,  0, 0, NULL,
#endif /* USE_NET */
 NM_ITEM, MENU(CACHEFLUSHIMAGES),       0,  0, 0, (APTR)MENU_CACHEFLUSHIMAGES,
 NM_ITEM, MENU(CACHEFLUSH),             0,  0, 0, (APTR)MENU_CACHEFLUSH,
#if USE_NET
 NM_ITEM, MENU(CACHEPRUNE),             0,  0, 0, (APTR)MENU_CACHEPRUNE,
#endif /* USE_NET */

NM_TITLE, MENU(SETTINGS),               0,   0, 0, NULL,
#if USE_NET
 NM_ITEM, MENU(SET_OFFLINEMODE),        0,       CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_OFFLINEMODE,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
#endif /* USE_NET */
 NM_ITEM, MENU(SET_GUI),                "E", 0, 0, (APTR)MENU_SET_GUI,
 NM_ITEM, MENU(SET_MIME),               0, 0, 0, (APTR)MENU_SET_MIME,
#if USE_PLUGINS
 NM_ITEM, MENU(SET_PLUGINS),            0, 0, 0, (APTR)MENU_SET_PLUGINS,
#endif /* USE_PLUGINS */
 NM_ITEM, MENU(MUIOPTS),                0, 0, 0, (APTR)MENU_MUIOPTS,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(SET_LOADIMAGES),         0,   0, 0, 0,
  NM_SUB, MENU(SET_LOADIMAGES_NONE),    0,   CHECKIT, 2 + 4, (APTR)MENU_SET_LOADIMAGES_NONE,
  NM_SUB, MENU(SET_LOADIMAGES_IMAPS),   0,   CHECKIT, 1 + 4, (APTR)MENU_SET_LOADIMAGES_IMAPS,
  NM_SUB, MENU(SET_LOADIMAGES_ALL),     0,   CHECKIT, 1 + 2, (APTR)MENU_SET_LOADIMAGES_ALL,
  NM_SUB, MENU(SET_LOADIMAGES_BACKGROUNDS),     0,   CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_LOADIMAGES_BACKGROUNDS,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(SET_NOFRAMES),           0,   CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_IGNOREFRAMES,
#if USE_NET
 NM_ITEM, MENU(SET_NOPROXY),            0,   CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_NOPROXY,
 NM_ITEM, MENU(SET_KEEPFTP),            "K",   CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_KEEPFTP,
 NM_ITEM, MENU(SET_SPOOF),              0,   CHECKIT | MENUTOGGLE, 0, (APTR)0,
  NM_SUB, MENU(SET_SPOOF_DEFAULT),		0,   CHECKIT | CHECKED, 2 + 4 + 8, (APTR)MENU_SET_SPOOF_0,
  NM_SUB, MENU(SET_SPOOF),    			0,   CHECKIT, 1 + 4 + 8, (APTR)MENU_SET_SPOOF_1,
  NM_SUB, MENU(SET_SPOOF),    			0,   CHECKIT, 1 + 2 + 8, (APTR)MENU_SET_SPOOF_2,
  NM_SUB, MENU(SET_SPOOF),    			0,   CHECKIT, 1 + 2 + 4, (APTR)MENU_SET_SPOOF_3,
 NM_ITEM, MENU(SET_IGNOREMIME),         "M",   CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_IGNOREMIME,
 NM_ITEM, MENU(SET_METAREFRESH),        "R",     CHECKIT | MENUTOGGLE, 0, (APTR)MENU_SET_METAREFRESH,
#endif /* USE_NET */
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(SET_LOAD),               0,  0, 0, (APTR)MENU_SET_LOAD,
 NM_ITEM, MENU(SET_SAVE),               0,  0, 0, (APTR)MENU_SET_SAVE,
 NM_ITEM, MENU(SET_SAVEAS),             0,  0, 0, (APTR)MENU_SET_SAVEAS,
#ifdef VDEBUG
NM_TITLE, MENU(DEBUG),				   	0,  0,  0,  NULL,
 NM_ITEM, MENU(DEBUG_LEVEL_0),			0,   CHECKIT | CHECKED, 2 + 4 + 8 + 16, (APTR)MENU_SET_DEBUG_LEVEL_0,
 NM_ITEM, MENU(DEBUG_LEVEL_1),			0,   CHECKIT, 1 + 4 + 8 + 16, (APTR)MENU_SET_DEBUG_LEVEL_1,
 NM_ITEM, MENU(DEBUG_LEVEL_2),			0,   CHECKIT, 1 + 2 + 8 + 16, (APTR)MENU_SET_DEBUG_LEVEL_2,
 NM_ITEM, MENU(DEBUG_LEVEL_3),			0,   CHECKIT, 1 + 2 + 4 + 16, (APTR)MENU_SET_DEBUG_LEVEL_3,
 NM_ITEM, MENU(DEBUG_LEVEL_4),			0,   CHECKIT, 1 + 2 + 4 + 8, (APTR)MENU_SET_DEBUG_LEVEL_4,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(DEBUG_AUTH),				0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_AUTH,
 NM_ITEM, MENU(DEBUG_CACHE),			0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_CACHE,
 NM_ITEM, MENU(DEBUG_CACHEPRUNE),		0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_CACHEPRUNE,
 NM_ITEM, MENU(DEBUG_COOKIE),			0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_COOKIE,
 NM_ITEM, MENU(DEBUG_CSS),				0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_CSS,
 NM_ITEM, MENU(DEBUG_DOCINFOWIN),		0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_DOCINFOWIN,
 NM_ITEM, MENU(DEBUG_DOWNLOADWIN),		0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_DOWNLOADWIN,
 NM_ITEM, MENU(DEBUG_DNS),			    0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_DNS,
 NM_ITEM, MENU(DEBUG_FTP),			    0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_FTP,
 NM_ITEM, MENU(DEBUG_GUI ),				0,	CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_GUI,
 NM_ITEM, MENU(DEBUG_HISTORY),			0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_HISTORY,
 NM_ITEM, MENU(DEBUG_HTML),				0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_HTML,
 NM_ITEM, MENU(DEBUG_HTTP),				0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_HTTP,
 NM_ITEM, MENU(DEBUG_INIT),				0,	CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_INIT,
 NM_ITEM, MENU(DEBUG_JS),			    0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_JS,
 NM_ITEM, MENU(DEBUG_MAIL),				0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_MAIL,
 NM_ITEM, MENU(DEBUG_MISC),				0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_MISC,
 NM_ITEM, MENU(DEBUG_NET),			    0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_NET,
 NM_ITEM, MENU(DEBUG_PLUGIN),			0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_PLUGIN,
 NM_ITEM, MENU(DEBUG_REXX),	            0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_REXX,
 NM_ITEM, MENU(DEBUG_IMGDEC),           0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_IMGDEC,
 NM_ITEM, NM_BARLABEL,                  0,   0, 0, NULL,
 NM_ITEM, MENU(DEBUG_FORCEBORDER),		0,  CHECKIT | MENUTOGGLE, 0, ( APTR )MENU_SET_DEBUG_FORCEBORDER,
#endif /* DEBUG */
NM_END };

static void initnewmenus( void )
{
	struct NewMenu *nmp = newmenus;

	while( nmp->nm_Type )
	{
		if( nmp->nm_Label != NM_BARLABEL )
			nmp->nm_Label = (char*)GSI( (ULONG)nmp->nm_Label );
		nmp++;
	}
}

#endif /* USE_MENUS */

#if USE_REXX
extern struct MUI_Command rexxcmds[];
extern ULONG use_rexx;

static void initrexx( void )
{
//	  int c;

	Forbid();
	if( FindPort( "AREXX" ) )
	{
		use_rexx = TRUE;
		Permit();
		return;
	}
	Permit();

/* XXX: beware of the Permit(  ) above if I ever put that code back again */
#if 0
	SystemTags( "RUN >NIL: SYS:System/RexxMast", TAG_DONE );

	for( c = 0; c < 20; c++ )
	{
		if( FindPort( "AREXX" ) )
		{
			use_rexx = TRUE;
			Permit();
			return;
		}
		Delay( 3 );
	}

	Permit();

	MUI_Request( app, 0, 0, GS( ERROR ), GS( OK ), GS( NOREXX ) );
#endif
}
#endif /* USE_REXX */

#if USE_SPLASHWIN
extern APTR splashwin;
#endif /* USE_SPLASHWIN */
static int buildapp( void )
{
	static STRPTR classlist[] = { 
		"Listtree.mcc",
		"Textinput.mcc",
		"Textinputscroll.mcc",
#if USE_CMANAGER
		"CManager.mcc",
#endif /* USE_CMANAGER */
#if USE_BUSY
		"Busy.mcc",
#endif /* USE_BUSY */
#if USE_TEAROFF
		"TearOff.mcc",
#endif /* USE_TEAROFF */
#if USE_SPEEDBAR
		"SpeedBar.mcc",
#endif /* USE_SPEEDBAR */
#if USE_POPHOTKEY
		"Pophotkey.mcc",
#endif /* USE_POPHOTKEY */
#if USE_TEAROFF
		"TearOff.mcp",
#endif /* USE_TEAROFF */
#if USE_NLIST
		"NListviews.mcc",
#endif /* USE_NLIST */
		NULL };

#if USE_MENUS
	int c; // miscellaneous menu setup for() loop variable

	initnewmenus();
#endif

#if USE_REXX
	initrexx();
#endif

	app = NewObject( getappclass(), NULL,
		MUIA_Application_Title, "" APPNAME "",
		MUIA_Application_Version, lversion,
		MUIA_Application_Copyright, "(C) 1995-2003 Oliver Wagner & David Gerber",
		MUIA_Application_Author, "Oliver Wagner & David Gerber",
		MUIA_Application_UsedClasses, classlist,
		MUIA_Application_Description, GS( APP_DESC ),
		MUIA_Application_Base, "VOYAGER",
		MUIA_Application_HelpFile, GS( APP_GUIDENAME ),
		MUIA_Application_SingleTask, TRUE,
#if USE_MENUS
		MUIA_Application_Menustrip, menu = MUI_MakeObject( MUIO_MenustripNM, ( ULONG )newmenus, MUIO_MenustripNM_CommandKeyCheck ),
#endif
#if USE_REXX
		MUIA_Application_Commands, rexxcmds,
#endif
#ifndef MBX
		MUIA_Application_DiskObject, diskobj,
#endif
		MUIA_Application_Iconified, startup_iconified,
#if USE_STB_NAV
		MUIA_Application_Window, build_stb_menu(),
		MUIA_Application_Window, build_url_window(),					 
#endif
	End;

#if USE_MENUS
	VoyLog(( "[INIT] menustrip=%lx app=%lx\n", (ULONG)menu, (ULONG)app ));
	VoyFlush();
#endif /* USE_MENUS */

	if( !app )
	{
		int rc = MUI_Error();

		if( !rc || rc == 6 )
			return( FALSE );

		MUI_Request( NULL, NULL, 0, copyright, GS( CANCEL ), GS( APP_FAILED ), rc );
		return( FALSE );
	}

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		if( splashwin = NewObject( getsplashwinclass(), NULL, TAG_DONE ) )
		{
			D( db_init, bug( "SplashWin created\n" ) );
			DoMethod( app, OM_ADDMEMBER, splashwin );
			set( splashwin, MUIA_Window_Open, TRUE );
		}
	}
#endif /* USE_SPLASHWIN */

	D( db_init, bug( "App created\n" ) );

	notify = MUI_NewObject( MUIC_Notify, TAG_DONE );

#if USE_MENUS

	if (menu)
	{
		/*
		 * Setup of the menu notifications. Please keep them ordered and
		 * no not use MUIM_Application_NewInput anymore.
		 */


		/*
		 * Voyager
		 */

		/* Open file... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_OPENFILE,
			app, 2, MM_DoLastActiveWin, MM_HTMLWin_OpenFile
		);

		/* Save as HTML... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_SAVEHTML,
			app, 2, MM_DoLastActiveWin, MM_HTMLWin_SaveHTML
		);
		
		/* Save as plain text... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_SAVETEXT,
			app, 2, MM_DoLastActiveWin, MM_HTMLWin_SaveTEXT
		);

		/* Print... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_WIN_PRINT,
				  app, 2, MM_DoLastActiveWin, MM_HTMLWin_Print
		);

		/* About... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_ABOUT,
			app, 6, MM_DoLastActiveWin, MM_HTMLWin_SetURL, "About:", NULL, NULL, MF_HTMLWin_AddURL | MF_HTMLWin_Reload
		);

		/* About MUI...*/
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_ABOUTMUI,
				  app, 2, MUIM_Application_AboutMUI, NULL );

		/* Iconify */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_ICONIFY,
			app, 3, MUIM_Set, MUIA_Application_Iconified, TRUE
		);


		/*
		 * Edit
		 */

		/* Find in page... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_WIN_FIND,
			app, 2, MM_DoLastActiveWin, MM_HTMLWin_ShowFind
		);

		
		/*
		 * Windows
		 */
		
		/* Open new window... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_WIN_NEW,
			app, 1, MM_App_NewWindow
		);

		/* Close window... */
		DoMethod( findmenu(MENU_WIN_CLOSE), MUIM_Notify, MUIA_Menuitem_Trigger,
			MUIV_EveryTime, app, 5, MUIM_Application_PushMethod,
				app, 2, MM_DoLastActiveWin, MM_HTMLWin_Close
		);

		/* Toggle Fullscreen... */
		DoMethod( findmenu(MENU_SETWINSIZE_FS), MUIM_Notify, MUIA_Menuitem_Trigger,
			MUIV_EveryTime, app, 2, MM_DoLastActiveWin, MM_HTMLWin_ToggleFullScreen
		);

		/* Network status... */
#if USE_NET
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_NETINFO,
			app, 1, MM_App_OpenNetinfoWindow
		);
#endif /* USE_NET */

		/* Downloads... */
#if USE_NET
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_DOWNLOADS,
			app, 1, MM_App_OpenDownloadWin
		);
#endif /* USE_NET */

		/* Error window... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_ERRORWIN,
			app, 1, MM_App_OpenErrorWindow
		);

		/* View page source... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_WIN_SOURCE,
			app, 2, MM_DoLastActiveWin, MM_HTMLWin_ShowSource
		);

		/* View page info... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_WIN_DOCINFOWIN,
			app, 2, MM_DoLastActiveWin, MM_HTMLWin_OpenDocInfoWin
		);
		

		/*
		 * Bookmarks
		 */

		/* Bookmark Manager... */
#if USE_CMANAGER
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_BM_EDIT,
			app, 1, MM_App_OpenBMWin
		);
#endif

		/*
		 * Cache
		 */
		
		/* Goto disk cache */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_CACHECONTENTS,
			app, 6, MM_DoLastActiveWin, MM_HTMLWin_SetURL, "About:Cache", NULL, NULL, MF_HTMLWin_AddURL | MF_HTMLWin_Reload
		);

		/* Flush images from memory */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_CACHEFLUSH,
			app, 2, MM_App_CacheFlush, MV_App_CacheFlush_Mem
		);

		/* Flush images from disk */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_CACHEPRUNE,
			app, 2, MM_App_CacheFlush, MV_App_CacheFlush_Disk
		);


		/*
		 * Settings
		 */

		/* Offline browsing */
		DoMethod( findmenu( MENU_SET_OFFLINEMODE ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &gp_offlinemode
		);

		/* Settings... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_SET_GUI,
			app, 1, MM_App_OpenPrefsWindow
		);

		/* Plugins... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_SET_PLUGINS,
			app, 1, MM_App_OpenPluginsWindow
		);

		/* MUI settings... */
		DoMethod( app, MUIM_Notify, MUIA_Application_MenuAction, MENU_MUIOPTS,
			app, 2, MUIM_Application_OpenConfigWindow, 0
		);

		/* Disable Proxies */
		DoMethod( findmenu( MENU_SET_NOPROXY ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &gp_noproxy
		);
		
		/* Image loading */
		DoMethod( findmenu( MENU_SET_LOADIMAGES_ALL ), MUIM_Notify, MUIA_Menuitem_Checked, TRUE,
			notify, 3, MUIM_WriteLong, 0, &gp_loadimages
		);
		DoMethod( findmenu( MENU_SET_LOADIMAGES_IMAPS ), MUIM_Notify, MUIA_Menuitem_Checked, TRUE,
			notify, 3, MUIM_WriteLong, 1, &gp_loadimages
		);
		DoMethod( findmenu( MENU_SET_LOADIMAGES_NONE ), MUIM_Notify, MUIA_Menuitem_Checked, TRUE,
			notify, 3, MUIM_WriteLong, 2, &gp_loadimages
		);
		DoMethod( findmenu( MENU_SET_LOADIMAGES_BACKGROUNDS ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_TriggerValue,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &gp_loadimages_bg
		);


		/* Keep FTP connections... */
		DoMethod( findmenu( MENU_SET_KEEPFTP ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &gp_keepftp
		);
		
		/* Spoof as... */
		for( c = 0; c < 4; c++ )
		{
			DoMethod( findmenu( MENU_SET_SPOOF_0 + c ), MUIM_Notify, MUIA_Menuitem_Checked, TRUE,
				notify, 3, MUIM_WriteLong, c, &gp_spoof
			);
			DoMethod( findmenu( MENU_SET_SPOOF_0 + c ), MUIM_Notify, MUIA_Menuitem_Checked, TRUE,
				app, 1, MM_App_ApplySpoof
			);
		}
		
		/* Ignore server sent MIME types */
		DoMethod( findmenu( MENU_SET_IGNOREMIME ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &gp_ignorehttpmime
		);
		
		/* Disable META refresh */
		DoMethod( findmenu( MENU_SET_METAREFRESH ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &gp_metarefresh
		);


#ifdef VDEBUG
		for( c = 0; c < 5; c++ )
		{
			DoMethod( findmenu( MENU_SET_DEBUG_LEVEL_0 + c ), MUIM_Notify, MUIA_Menuitem_Checked, TRUE,
				notify, 3, MUIM_WriteLong, c, &db_level
			);
		}

		DoMethod( findmenu( MENU_SET_DEBUG_AUTH ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_auth
		);

		DoMethod( findmenu( MENU_SET_DEBUG_FORCEBORDER ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_forceborder
		);

		DoMethod( findmenu( MENU_SET_DEBUG_CACHE ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_cache
		);

		DoMethod( findmenu( MENU_SET_DEBUG_COOKIE ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_cookie
		);

		DoMethod( findmenu( MENU_SET_DEBUG_DNS ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_dns
		);

		DoMethod( findmenu( MENU_SET_DEBUG_DOCINFOWIN ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_docinfowin
		);

		DoMethod( findmenu( MENU_SET_DEBUG_DOWNLOADWIN ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_dlwin
		);

		DoMethod( findmenu( MENU_SET_DEBUG_FTP ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_ftp
		);

		DoMethod( findmenu( MENU_SET_DEBUG_HISTORY ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_history
		);

		DoMethod( findmenu( MENU_SET_DEBUG_HTTP ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_http
		);

		DoMethod( findmenu( MENU_SET_DEBUG_JS ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_js
		);

		DoMethod( findmenu( MENU_SET_DEBUG_NET ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_net
		);

		DoMethod( findmenu( MENU_SET_DEBUG_PLUGIN ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_plugin
		);

		DoMethod( findmenu( MENU_SET_DEBUG_MAIL ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_mail
		);

		DoMethod( findmenu( MENU_SET_DEBUG_CACHEPRUNE ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_cacheprune
		);

		DoMethod( findmenu( MENU_SET_DEBUG_HTML ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_html
		);

		DoMethod( findmenu( MENU_SET_DEBUG_CSS ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_css
		);

		DoMethod( findmenu( MENU_SET_DEBUG_GUI ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_gui
		);

		DoMethod( findmenu( MENU_SET_DEBUG_MISC ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_misc
		);

		DoMethod( findmenu( MENU_SET_DEBUG_INIT ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_init
		);

		DoMethod( findmenu( MENU_SET_DEBUG_REXX ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			notify, 3, MUIM_WriteLong, MUIV_TriggerValue, &db_rexx
		);

		DoMethod( findmenu( MENU_SET_DEBUG_IMGDEC ), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
			app, 1, MM_App_SetImgDebug
		);
#endif /* DEBUG */

	}

#endif /* USE_MENUS */
 
	return( TRUE );
}


int make_app( void )
{
	if( buildapp() )
	{
		cfg_load( startup_cfgfile );
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

void close_app( void )
{
	if( app )
	{
		D( db_init, bug( "disposing app stuff..\n" ) );

#if USE_CMANAGER
		D( db_init, bug( "saving bookmarks..\n" ) );
		bm_cleanup();
#endif /* USE_CMANAGER */

		if ( getflag( VFLG_SAVE_ON_EXIT ) )
		{
			D( db_init, bug( "VFLG_SAVE_ON_EXIT set thus saving config file..\n" ) );
			cfg_save( startup_cfgfile );
		}

		//TOFIX!! Olli did put that due to public screen not closing
		// on exit.. if it happens again check why
		//DoMethod( app, MUIM_Application_ClosePublic );
		
		D( db_init, bug( "disposing appobject\n" ) );
		MUI_DisposeObject( app );

		D( db_init, bug( "disposing notify\n" ) );
		MUI_DisposeObject( notify );
	}
}

int prefsfreq( char *title, int save, char *to, char *pattern )
{
#ifndef MBX
	struct FileRequester *fr;
	char temp1[ 256 ], temp2[ 32 ], *p;
	int rc = FALSE;

	strcpy( temp1, to );
	p = FilePart( temp1 );
	stccpy( temp2, p, sizeof( temp2 ) );
	*p = 0;

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_TitleText, title,
		ASLFR_Screen, VAT_GetAppScreen( app ),
		ASLFR_InitialDrawer, temp1,
		ASLFR_InitialFile, temp2,
		ASLFR_DoSaveMode, save,
		ASLFR_RejectIcons, TRUE,
		ASLFR_InitialPattern, pattern,
		ASLFR_DoPatterns, pattern,
		TAG_DONE
	);
	if( !fr )
	{
		displaybeep();
		return( FALSE );
	}

	if( MUI_AslRequestTags( fr, TAG_DONE) )
	{
		strcpy( to, fr->fr_Drawer );
		AddPart( to, fr->fr_File, 256 );
		rc = TRUE;
	}

	MUI_FreeAslRequest( fr );

	return( rc );
#else
	return( FALSE );
#endif /* MBX */
}

void save_progdir( void )
{
#if USE_VAT
	VAT_SetLastUsedDir( "Voyager" );
#endif
}


