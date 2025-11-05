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


#ifndef VOYAGER_PREFS_H
#define VOYAGER_PREFS_H
/*
 * $Id: prefs.h,v 1.70 2003/04/25 19:13:55 zapek Exp $
 */

/*
 * How to add a prefs item:
 *
 * - add a VFLG (flag) / DSI (data select item I think) here
 * - set the default value in prefs.c/initprefs()
 * - if the setting is intensively checked, add a gp_<prefitem> var in prefs.c
 *   and set it correctly in prefs.c/set_prefs_globals(). add it there too as extern
 * - add it in prefswin_<foo>.c DECNEW
 * - add a storeattr/string() in DECDISPOSE
 *
 */

// Flags
#define VFLG_ACTIONBUT_MODES 0
#define VFLG_LOAD_IMAGES 1
#define VFLG_SHOW_FASTLINKS 2
#define VFLG_IGNOREDOCCOLS 3
//#define VFLG_ASKDLDIR 30 // obsolete
#define VFLG_UNDERLINE_LINKS 16
#define VFLG_HIDE_ICON 17
#define VFLG_IGNORE_FRAMES 18
#define VFLG_IGNORE_BACKGROUNDS 19
#define VFLG_PRINTMODE 20
#define VFLG_AUTOCLOSEDLWIN 21
#define VFLG_SEARCHMODE 22
#define VFLG_SEARCHCASE 23
#define VFLG_USECM 24
#define VFLG_SAVE_ON_EXIT 25
#define VFLG_STOP_ICON 26 // obsolete since V 3.0.67
#define VFLG_AUTOCLEANUP_MODES 27
#define VFLG_SCROLLBARS 28
#define VFLG_FASTLINKS_STRIPTEXT 29
#define VFLG_REMEMBER_AUTH 30
#define VFLG_HOMEPAGEURL_MODES 31
#define VFLG_USE_ERROR_REQUESTER 32
#define VFLG_HOMEPAGE_AUTOLOAD 33
#define VFLG_FONTFACE 34
#define VFLG_TEAROFF 35
#define VFLG_USE_CLOCK 36
#define VFLG_CLOCK_SECONDS 37
#define VFLG_SMOOTH_SCROLL 38
#define VFLG_SINGLEWINDOW 39
#define VFLG_IMG_SHOW_ALT_TEXT 40
#define VFLG_IMG_SHOW_PH_BORDER 41
#define VFLG_FULLSCREEN 42 /* XXX: hm.. used to know if the first window is to open fullscreen.. could be smarter perhaps */

// Dataspace IDs

#define DSI_GRP(x) (x<<16)

#define DSI_FLAGS (DSI_GRP(0))
#define DSI_COLORS (DSI_GRP(1)+200)
#define DSI_FONTS (DSI_GRP(2))
#define DSI_FASTLINKS_LABELS (DSI_GRP(3))
#define DSI_FASTLINKS_URLS (DSI_GRP(4))
#define DSI_HOMEPAGE (DSI_GRP(5))
#define DSI_PROXY (DSI_GRP(6))
#define DSI_CACHE (DSI_GRP(7))
#define DSI_NET (DSI_GRP(8))
//#define DSI_MIME (DSI_GRP(9))
#define DSI_SECURITY (DSI_GRP(10))
#define DSI_SAVEDEST (DSI_GRP(5000))
#define DSI_IMG (DSI_GRP(11))
#define DSI_JS (DSI_GRP(12))
#define DSI_CACHE_AUTOVERIFY (DSI_GRP(13))
#define DSI_DEBUG (DSI_GRP(21))

#define DSI_FONT_FACE_NUM (DSI_FONTS+99)
#define DSI_FONT_FACENAME(x) (DSI_FONTS+100+(x*1000))
#define DSI_FONT_MAP(x,y) (DSI_FONTS+100+(x*1000)+1+y)

#define DSI_BUTTONS_LABELS DSI_GRP(14)
#define DSI_BUTTONS_ACTION DSI_GRP(15)
#define DSI_BUTTONS_ARGS DSI_GRP(16)
#define DSI_BUTTONS_IMAGES DSI_GRP(17)
#define DSI_BUTTONS_SHORTCUT DSI_GRP(18)

#define DSI_BUTTON_STYLE DSI_GRP(19)
#define DSI_MISC (DSI_GRP(20))

/*
 * Warning: GRP 22 to 32 are taken and should be considered
 * as dead
 */

#define DSI_CMENUS_PAGE_LABELS DSI_GRP(33)
#define DSI_CMENUS_PAGE_ACTION DSI_GRP(34)
#define DSI_CMENUS_PAGE_ARGS DSI_GRP(35)
#define DSI_CMENUS_PAGE_DEPTH DSI_GRP(36)

#define DSI_CMENUS_LINK_LABELS DSI_GRP(37)
#define DSI_CMENUS_LINK_ACTION DSI_GRP(38)
#define DSI_CMENUS_LINK_ARGS DSI_GRP(39)
#define DSI_CMENUS_LINK_DEPTH DSI_GRP(40)
 
#define DSI_CMENUS_IMAGE_LABELS DSI_GRP(41)
#define DSI_CMENUS_IMAGE_ACTION DSI_GRP(42)
#define DSI_CMENUS_IMAGE_ARGS DSI_GRP(43)
#define DSI_CMENUS_IMAGE_DEPTH DSI_GRP(44)
 
#define DSI_BUTTON_STYLE_SUNNY DSI_BUTTON_STYLE+0
#define DSI_BUTTON_STYLE_RAISED DSI_BUTTON_STYLE+1
#define DSI_BUTTON_STYLE_BORDERLESS DSI_BUTTON_STYLE+2
#define DSI_BUTTON_STYLE_SMALL DSI_BUTTON_STYLE+3
#define DSI_BUTTON_NUM DSI_BUTTON_STYLE+4

#define getflag(x) getprefslong( DSI_FLAGS + (x), 0 )
#define setflag(x,v) setprefslong( DSI_FLAGS + (x), v )

#define DSI_FASTLINKS_NUM (DSI_GRP(3)+9884)

#define DSI_PROXY_HTTP (DSI_PROXY+0)
#define DSI_PROXY_FTP (DSI_PROXY+1)
#define DSI_PROXY_GOPHER (DSI_PROXY+2)
#define DSI_PROXY_WAIS (DSI_PROXY+3)
#define DSI_NOPROXY (DSI_PROXY+4)
#define DSI_TEMPNOPROXY (DSI_PROXY+5)
#define DSI_PROXY_HTTPS (DSI_PROXY+6)
#define DSI_PROXY_CONFMODE (DSI_PROXY+7)
#define DSI_PROXY_AUTOCONF_URL (DSI_PROXY+8)

#define DSI_PROXY_USE_OFFSET 1024
#define DSI_PROXY_PORT_OFFSET 2048

#define DSI_NET_MAXCON (DSI_NET+0)
#define DSI_NET_MAILADDR (DSI_NET+1)
#define DSI_NET_REALNAME (DSI_NET+2)
#define DSI_NET_SMTP (DSI_NET+3)
#define DSI_NET_NNTP (DSI_NET+4)
#define DSI_NET_TELNET (DSI_NET+5)
#define DSI_NET_MAIL_APP (DSI_NET+6)
#define DSI_NET_NEWS_APP (DSI_NET+7)
#define DSI_NET_USE_MAILAPP (DSI_NET+8)
#define DSI_NET_USE_NEWSAPP (DSI_NET+9)
#define DSI_NET_ORGANIZATION (DSI_NET+10)
#define DSI_NET_LINKS_EXPIRE (DSI_NET+11)
#define DSI_NET_SPOOF (DSI_NET+12)
#define DSI_NET_SPOOFAS (DSI_NET+13)
#define DSI_NET_KEEPFTP (DSI_NET+14)
#define DSI_NET_IGNOREHTTPMIME (DSI_NET+15)
#define DSI_NET_METAREFRESH (DSI_NET+16)
#define DSI_NET_OFFLINEMODE (DSI_NET+17)
#define DSI_NET_CHECKIFONLINE (DSI_NET+18)
#define DSI_NET_SSL2 (DSI_NET+19)
#define DSI_NET_SSL3 (DSI_NET+20)
#define DSI_NET_TLS1 (DSI_NET+21)
#define DSI_NET_BUGS (DSI_NET+22)
#define DSI_NET_SIG (DSI_NET+23)
#define DSI_NET_SPOOF_AS_1 (DSI_NET+24)
#define DSI_NET_SPOOF_AS_2 (DSI_NET+25)
#define DSI_NET_SPOOF_AS_3 (DSI_NET+26)
#define DSI_NET_SPOOF_AS_1_AN (DSI_NET+27)
#define DSI_NET_SPOOF_AS_2_AN (DSI_NET+28)
#define DSI_NET_SPOOF_AS_3_AN (DSI_NET+29)
#define DSI_NET_SPOOF_AS_1_AC (DSI_NET+30)
#define DSI_NET_SPOOF_AS_2_AC (DSI_NET+31)
#define DSI_NET_SPOOF_AS_3_AC (DSI_NET+32)
#define DSI_NET_SPOOF_AS_1_AV (DSI_NET+33)
#define DSI_NET_SPOOF_AS_2_AV (DSI_NET+34)
#define DSI_NET_SPOOF_AS_3_AV (DSI_NET+35)
#define DSI_NET_DOWNLOAD_TIMEOUT (DSI_NET+36)
#define DSI_NET_DOWNLOAD_RETRIES (DSI_NET+37)

#define DSI_SECURITY_ASK_MAILTO (DSI_SECURITY+0)
#define DSI_SECURITY_WARN_POST (DSI_SECURITY+2)
#define DSI_SECURITY_NO_REFERER (DSI_SECURITY+3)
#define DSI_SECURITY_NO_MAILADDR (DSI_SECURITY+4)
#define DSI_SECURITY_ASK_COOKIE_TEMP (DSI_SECURITY+5)
#define DSI_SECURITY_ASK_COOKIE_PERM (DSI_SECURITY+6)
#define DSI_SECURITY_NO_SSLCACHE (DSI_SECURITY+7)

#define DSI_CACHE_SIZE (DSI_CACHE+0)
#define DSI_CACHE_DIR (DSI_CACHE+1)
#define DSI_CACHE_VERIFY (DSI_CACHE+2)
#define DSI_CACHE_MEMSIZE (DSI_CACHE+3)
#define DSI_CACHE_IMAGES (DSI_CACHE+4)
//#define DSI_CACHE_LASTPRUNE (DSI_CACHE+5)
//#define DSI_CACHE_ESTIMATEDSIZE (DSI_CACHE+6) // not used anymore
//#define DSI_CACHE_STATS (DSI_CACHE+7)

//#define DSI_MIME_EXT_OFFSET 2048
//#define DSI_MIME_ACTION_OFFSET 4096
//#define DSI_MIME_APP_OFFSET (2048+4096)

#define DSI_IMG_JPEG_QUANT (DSI_IMG+0)
#define DSI_IMG_JPEG_DITHER (DSI_IMG+1)
#define DSI_IMG_JPEG_DCT (DSI_IMG+2)
#define DSI_IMG_JPEG_FANCY (DSI_IMG+3)
#define DSI_IMG_JPEG_SMOOTHING (DSI_IMG+4)
#define DSI_IMG_DOUBLEBUFFER (DSI_IMG+7)
#define DSI_IMG_JPEG_PROGRESSIVE (DSI_IMG+8)
//#define DSI_IMG_STALLDETECT (DSI_IMG+9) // obsolete
#define DSI_IMG_GIF_DITHER (DSI_IMG+10)
#define DSI_IMG_PNG_DITHER (DSI_IMG+11)
#define DSI_IMG_STOP_ANIMGIF (DSI_IMG+12)

#define DSI_JS_ENABLE (DSI_JS+0)
#define DSI_JS_DEBUG (DSI_JS+1)
#define DSI_JS_ERRORLOG (DSI_JS+2)
#define DSI_JS_LOGFILE (DSI_JS+3)

#define DSI_MISC_COOKIEBROWSER_SORT_COLUMN (DSI_MISC+0)
#define DSI_MISC_COOKIEBROWSER_SORT_REVERSE (DSI_MISC+1)

#define	DSI_NAG_FIRSTINSTALLTIME (DSI_MISC+2)
#define	DSI_NAG_USECOUNT (DSI_MISC+3)
#define	DSI_NAG_UPTIME (DSI_MISC+4)

/* debug */
#define DSI_DEBUG_AUTH (DSI_DEBUG + 1)
#define DSI_DEBUG_CACHE (DSI_DEBUG + 2)
#define DSI_DEBUG_CACHEPRUNE (DSI_DEBUG + 3)
#define DSI_DEBUG_COOKIE (DSI_DEBUG + 4)
#define DSI_DEBUG_DOCINFOWIN (DSI_DEBUG + 5)
#define DSI_DEBUG_DOWNLOADWIN (DSI_DEBUG + 6)
#define DSI_DEBUG_DNS (DSI_DEBUG + 7)
#define DSI_DEBUG_FTP (DSI_DEBUG + 8)
#define DSI_DEBUG_GUI (DSI_DEBUG + 9)
#define DSI_DEBUG_HISTORY (DSI_DEBUG + 10)
#define DSI_DEBUG_HTML (DSI_DEBUG + 11)
#define DSI_DEBUG_HTTP (DSI_DEBUG + 12)
#define DSI_DEBUG_INIT (DSI_DEBUG + 13)
#define DSI_DEBUG_JS (DSI_DEBUG + 14)
#define DSI_DEBUG_MAIL (DSI_DEBUG + 15)
#define DSI_DEBUG_MISC (DSI_DEBUG + 16)
#define DSI_DEBUG_NET (DSI_DEBUG + 17)
#define DSI_DEBUG_PLUGIN (DSI_DEBUG + 18)
#define DSI_DEBUG_REXX (DSI_DEBUG + 19)
#define DSI_DEBUG_CSS (DSI_DEBUG + 20)

#define DSI_DEBUG_LEVEL (DSI_DEBUG+21)

/* languages */
#define DSI_MISC_LANGUAGES (DSI_MISC+5)

/*
 * Global vars. Use them for frequently accessed pref items.
 */
extern int gp_underline_links;
extern int gp_no_referer;
extern int gp_dbblit;
extern int gp_memcachesize;
extern int gp_cacheimg;
extern int gp_noproxy;
extern int gp_spoof;
extern int gp_maxnetproc;
extern int gp_keepftp;
extern int gp_ignorehttpmime;
extern int gp_metarefresh;
extern int gp_usecm;
extern int gp_offlinemode;
extern int gp_checkifonline;
extern char gp_cachedir[ 256 ];
extern int gp_javascript;
extern int gp_cache_verify;
extern ULONG gp_cachesize;
extern int gp_fontface;
extern char gp_languages[ 256 ];
extern int gp_tearoff;
extern ULONG gp_download_timeout;
extern ULONG gp_download_retries;
extern int gp_loadimages;
extern int gp_loadimages_bg;
extern int gp_image_stop_animgif;
extern int gp_image_show_alt_text;
extern int gp_image_show_ph_border;
extern int gp_smooth_scroll;
extern int gp_singlewindow;

/*
 * Misc ones
 */
extern char startup_cfgfile[ 256 ];

void set_prefs_globals( void );
int loadprefsfrom( char *filename );
int saveprefsas( char *filename );
int prefsfreq( char *title, int save, char *to, char *pattern );

void cfg_load( char *filename );
void cfg_save( char *filename );
void exchangeprefs_clone( ULONG id1, ULONG id2 );
void killprefs( ULONG id );
void loadtearoff( STRPTR filename );
void savetearoff( STRPTR filename );

int loadprefs( char * );
int saveprefs( char * );
void setprefs(ULONG , ULONG , APTR );
void setprefsstr(ULONG , STRPTR );
void setprefslong(ULONG , ULONG );
void setprefs_clone(ULONG , ULONG , APTR );
void setprefsstr_clone(ULONG , STRPTR );
void setprefslong_clone(ULONG , ULONG );
void initprefsclone( void );
void freeprefsclone( int writeback );
#ifdef __SASC
extern APTR ASM getprefs( __reg( d1, ULONG id ) );
#else
APTR getprefs( ULONG id );
#endif

STRPTR getprefsstr(ULONG , STRPTR );
ULONG getprefslong(ULONG , ULONG );
void killprefsrange( ULONG fromid, ULONG toid );
STRPTR getprefsstr_clone(ULONG , STRPTR );
ULONG getprefslong_clone(ULONG , ULONG );
APTR getprefs_clone( ULONG );
void killprefs_clone( ULONG id );
void copyprefs_clone( ULONG fromid, ULONG toid );
void addtobookmarks( char *url, char *title );

extern char startup_cfgfile[ 256 ], startup_bmfile[ 256 ];

enum buttonfuncs {
	BFUNC_SEP,
	
	/*
	 * OLD AND OUTDATED! DO NOT USE! DO NOT TOUCH!
	 */
	bfunc_rexx,
	bfunc_js,

	bfunc_back,
	bfunc_forward,
	bfunc_find,   // replaced with a OpenURL search:
	bfunc_home,
	bfunc_loadimages,
	bfunc_print,
	bfunc_reload,
	bfunc_stop,
	/*
	 * New and great, use :)
	 */
	BFUNC_COMMAND,
	BFUNC_AMIGADOS,
	BFUNC_WORKBENCH,
	BFUNC_SCRIPT,
	BFUNC_JAVASCRIPT,
	BFUNC_AREXX
};

#define BFUNC_BAR -1 /* small hack: this is not a function but a way to display a bar */

/*
 * Preference window button editor
 */
enum {
	BT_ADD_SPACER,
	BT_ADD,
	BT_REMOVE,
	BT_SELECT,
	BT_MODE,
	BT_COPY,
	BT_BUILD,
	BT_LEFT,
	BT_RIGHT
};

#endif /* VOYAGER_PREFS_H */
