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


#ifndef VOYAGER_INIT_H
#define VOYAGER_INIT_H
/*
 * $Id: init.h,v 1.63 2003/11/21 09:34:19 zapek Exp $
 */

#if USE_DOS
extern char myfullpath[ 256 ];
#endif /* USE_DOS */

extern int app_started; /* TRUE if the app started (eg. initstuff() didn't fail*/

/*
 * Constructors (sort of :)
 */
#ifdef VDEBUG
void init_debug( void );
#endif /* VDEBUG */
int initstuff( void );
void init_winlist( void );
void init_docinfolist( void );
void init_certreqslist( void );
void init_historylist( void );
void init_framesetlist( void );
void init_framesethistorylist( void );
void init_fontlist( void );
void init_postdatalist( void );
void init_cookies( void );
void init_inlist( void );
void init_prunecache( void );
void init_global_window_list( void );
int init_clonelist( void );
int init_malloc( void );
int init_css( void );
void init_keyname( void );
int init_prefs( void );
void save_progdir( void );
void init_dnscache( void );
int init_timer( void );
int init_methodstack( void );
void preinit_prefs( void );
void load_key( void );
int init_ipc( void );
int open_muimaster( void );
void old_loadkey( void );
void init_downloadlist( void );
void init_fakebitmap( void );
int init_verify( void );
int init_auth( void );
void init_progdir( void );
int create_ledclass( void );
int create_crossclass( void );
int init_netprocess( void );
int init_locale( void );
void init_internalipc( void );
int mcccheck( void );
void check_for_nlist( void );
int create_js_object( void );
int create_js_objref( void );
int create_ddstringclass( void );
int create_fastlinkclass( void );
int create_fastlinkgroupclass( void );
int create_gaugeclass( void );
int create_stbgaugeclass( void );
int create_pluginwinlistclass( void );
int create_js_array( void );
int create_js_event( void );
int create_js_bool( void );
int create_js_date( void );
int create_js_func( void );
int create_js_link( void );
int create_js_location( void );
int create_js_math( void );
int create_js_mimetype( void );
int create_js_navigator( void );
int create_js_real( void );
int create_js_screen( void );
int create_js_string( void );
int create_js_regexp( void );
int create_js_stb_root( void );
int create_js_stb_cdplayer( void );
int create_urlstringclass( void );
void init_tearoff( void );
int create_js_plugin( void );
int create_lo_form( void );
int create_lo_embed( void );
int create_postwinclass( void );
int create_postmailwinclass( void );
int load_diskobj( void );
int create_winclass( void );
int create_amiconclass( void );
int create_appclass( void );
int create_authbrowserwinclass( void );
int create_cookiebrowserwinclass( void );
int create_docinfowinclass( void );
int create_downloadwinclass( void );
int create_errorwinclass( void );
int create_historylistclass( void );
int create_pluginwinclass( void );
int create_prunecachewinclass( void );
int create_sourceviewclass( void );
int create_sizegroupclass( void );
int create_clockclass( void );
int create_smartreqclass( void );
#if USE_SPLASHWIN
int create_splashwinclass( void );
#endif
int create_buttonclass( void );
int create_toolbarclass( void );
int create_smartlabelclass( void );
int create_printwinclass( void );
int create_searchwinclass( void );
int create_authwinclass( void );
int create_certreqwinclass( void );
int create_cookiewinclass( void );
int create_netinfowinclass( void );
int make_app( void );
void load_cookies( void );
void load_auths( void );
void init_tokenbuff( void );
int init_imgdec( void );
int create_frameborderclass( void );
int init_keyname2( void );
int create_fonttestclass( void );
int create_htmlviewclass( void );
int create_htmlwinclass( void );
int create_frameclass( void );
void probe_mimeprefs( void );
int create_loimageclass( void );
int create_lopipclass( void );
int create_lodivclass( void );
int create_lobrclass( void );
void init_history( void );
int	start_image_decoders( void );
void init_memhandler( void );
void start_demotimeout( void );
void check_update( void );
#if USE_WBSTART
void cleanup_wbstart( void );
#endif /* USE_WBSTART */
#if CHECK_TIMEOUT
int check_timeout( void );
#endif
void start_prunecache( void );
void init_plugins( void );
int create_logroupclass( void );
int create_lobuttonclass( void );
int create_loradioclass( void );
int create_locheckboxclass( void );
int create_loformbuttonclass( void );
int create_loformtextclass( void );
int create_loformfileclass( void );
int create_loformtextfieldclass( void );
int create_loformcycleclass( void );
int create_loformhiddenclass( void );
int create_loform_optionclass( void );
int create_lodummyclass( void );
int create_loanchorclass( void );
int create_lomarginclass( void );
int create_lomapclass( void );
int create_loareaclass( void );
int create_lotableclass( void );
int create_loframesetclass( void );
int create_lohrclass( void );
int create_loliclass( void );
int create_commandclass( void );
int create_scrollgroupclass( void );
#if !USE_EXTERNAL_PREFS
int create_prefswin_toolbarclass( void );
int create_prefswin_languagesclass( void );
int create_prefswin_cacheclass( void );
int create_prefswin_certsclass( void );
int create_prefswin_colorsclass( void );
int create_prefswin_downloadclass( void );
int create_prefswin_fastlinksclass( void );
int create_prefswin_fontsclass( void );
int create_prefswin_generalclass( void );
int create_prefswin_hyperlinksclass( void );
int create_prefswin_imagesclass( void );
int create_prefswin_javascriptclass( void );
int create_prefswin_mailnewsclass( void );
int create_prefswin_networkclass( void );
int create_prefswin_securityclass( void );
int create_prefswin_spoofclass( void );
int create_prefswin_mainclass( void );
int create_prefswin_listclass( void );
int create_prefswin_contextmenuclass( void );
#endif /* !USE_EXTERNAL_PREFS */
#ifdef MBX
int create_pipwindowclass( void );
#endif
	
/*
 * Destructors
 */
void closestuff( void );
void cleanup_windows( void );
void cleanup_memhandler( void );
void save_history( void );
void close_vapor_update( void );
void cleanup_demotimeout( void );
void free_autoproxy( void );
void save_authentications( void );
void save_cookies( void );
void close_image_decoders( void );
void close_app( void );
void delete_loimageclass( void );
void delete_lopipclass( void );
void delete_lodivclass( void );
void delete_lobrclass( void );
void delete_lohrclass( void );
void delete_loliclass( void );
void delete_frameclass( void );
void delete_fonttestclass( void );
void delete_frameborderclass( void );
void delete_netinfowinclass( void );
void delete_cookiewinclass( void );
void delete_certreqwinclass( void );
void delete_authwinclass( void );
void delete_searchwinclass( void );
void delete_printwinclass( void );
void delete_smartlabelclass( void );
void delete_buttonclass( void );
void delete_toolbarclass( void );
#if USE_SPLASHWIN
void delete_splashwinclass( void );
#endif
void delete_sizegroupclass( void );
void delete_sourceviewclass( void );
void delete_prunecachewinclass( void );
void delete_pluginwinclass( void );
void delete_historylistclass( void );
void delete_errorwinclass( void );
void delete_downloadwinclass( void );
void delete_docinfowinclass( void );
void delete_cookiebrowserwinclass( void );
void delete_authbrowserwinclass( void );
void delete_appclass( void );
void delete_amiconclass( void );
void delete_winclass( void );
void delete_postmailwinclass( void );
void delete_postwinclass( void );
void free_diskobject( void );
void delete_lo_form( void );
void delete_lo_embed( void );
void delete_js_plugin( void );
void delete_urlstringclass( void );
void delete_js_string( void );
void delete_js_regexp( void );
void delete_js_stb_root( void );
void delete_js_stb_cdplayer( void );
void delete_js_screen( void );
void delete_js_real( void );
void delete_js_navigator( void );
void delete_js_mimetype( void );
void delete_js_math( void );
void delete_js_location( void );
void delete_js_link( void );
void delete_js_func( void );
void delete_js_date( void );
void delete_js_bool( void );
void delete_js_array( void );
void delete_js_event( void );
void delete_pluginwinlistclass( void );
void delete_gaugeclass( void );
void delete_stbgaugeclass( void );
void delete_fastlinkgroupclass( void );
void delete_fastlinkclass( void );
void delete_ddstringclass( void );
void delete_js_objref( void );
void delete_js_object( void );
void delete_htmlviewclass( void );
void delete_htmlwinclass( void );
void delete_smartreqclass( void );
void cleanup_plugins( void );
void cleanup_tearoff( void );
void cleanup_clonelist( void );
void cleanup_inlist( void );
void cleanup_historylist( void );
void cleanup_internalipc( void );
void close_locale( void );
void cleanup_netprocess( void );
void delete_ledclass( void );
void delete_crossclass( void );
void cleanup_authentications( void );
void cleanup_verify( void );
void restore_progdir( void );
void cleanup_fonts( void );
void cleanup_alloca( void );
void close_cybergfx( void );
void close_muimaster( void );
void free_cachelocks( void );
void cleanup_ipc( void );
void cleanup_prefs( void );
void cleanup_dnscache( void );
void cleanup_methodstack( void );
void cleanup_timer( void );
void cleanup_css( void );
void cleanup_malloc( void );
void cleanup_cachebitmap( void );
void delete_logroupclass( void );
void delete_loradioclass( void );
void delete_locheckboxclass( void );
void delete_lobuttonclass( void );
void delete_loformbuttonclass( void );
void delete_loformtextclass( void );
void delete_loformfileclass( void );
void delete_loformtextfieldclass( void );
void delete_loformcycleclass( void );
void delete_loformhiddenclass( void );
void delete_loform_optionclass( void );
void delete_lodummyclass( void );
void delete_loanchorclass( void );
void delete_lomarginclass( void );
void delete_lomapclass( void );
void delete_loareaclass( void );
void delete_lotableclass( void );
void delete_loframesetclass( void );
void delete_commandclass( void );
void delete_clockclass( void );
void delete_scrollgroupclass( void );
#if !USE_EXTERNAL_PREFS
void delete_prefswin_toolbarclass( void );
void delete_prefswin_languagesclass( void );
void delete_prefswin_cacheclass( void );
void delete_prefswin_certsclass( void );
void delete_prefswin_colorsclass( void );
void delete_prefswin_downloadclass( void );
void delete_prefswin_fastlinksclass( void );
void delete_prefswin_fontsclass( void );
void delete_prefswin_generalclass( void );
void delete_prefswin_hyperlinksclass( void );
void delete_prefswin_imagesclass( void );
void delete_prefswin_javascriptclass( void );
void delete_prefswin_mailnewsclass( void );
void delete_prefswin_networkclass( void );
void delete_prefswin_securityclass( void );
void delete_prefswin_spoofclass( void );
void delete_prefswin_mainclass( void );
void delete_prefswin_listclass( void );
void delete_prefswin_contextmenuclass( void );
#endif /* !USE_EXTERNAL_PREFS */
#ifdef MBX
void delete_pipwindowclass( void );
#endif

#endif /* VOYAGER_INIT_H */
