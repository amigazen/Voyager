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
** $Id: main.c,v 1.154 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <exec/memory.h>
#include "vup/vupdate.h"
#endif

/* private */
#include "voyager_cat.h"
#include "voy_ipc.h"
#include "debug.h"
#include "copyright.h"
#include "classes.h"
#include "authipc.h"
#include "network.h"
#include "js.h"
#if USE_PLUGINS
#include <proto/v_plugin.h>
#ifdef MBX
#include "md/v_plugin_lib_calls.h"
#endif
#include "plugins.h"
#endif /* USE_PLUGINS */
#include "htmlclasses.h"
#if USE_SPLASHWIN
#include "splashwin.h"
#endif /* USE_SPLASHWIN */
#include "malloc.h"
#include "methodstack.h"
#include "menus.h"
#include "history.h"
#include "netinfo.h"
#include "cookiebrowser.h"
#include "authbrowser.h"
#include <proto/vimgdecode.h>
#include "mui_func.h"
#include "cmanager.h"
#include "htmlwin.h"
#include "prefs.h"
#include "mime.h"
#include "init.h"
#include "nagwin.h"
#include "win_func.h"
#ifdef MBX
#include <mcp_lib_calls.h>
#endif
#if USE_TESTFILE
#include "dos_func.h"
#endif /* USE_TESTFILE */


#ifdef MBX
char lversion[] = { "$VER: "APPNAME"-Met@box " LVERTAG };
#elif BETA_VERSTRING
char lversion[] = { "$VER: "APPNAME"-BETA " LVERTAG };
#elif defined(NETCONNECT)
char lversion[] = { "$VER: "APPNAME"-NC " LVERTAG };
#elif defined(ISSPECIALDEMO)
char lversion[] = { "$VER: "APPNAME"-Special " LVERTAG };
#elif !USE_NET
char lversion[] = { "$VER: "APPNAME"-NoNet " LVERTAG };
#elif GREX_RELEASE
char lversion[]	= { "$VER: "APPNAME"-Special-GRex-CD " LVERTAG };
#else
char lversion[] = { "$VER: "APPNAME" " LVERTAG };
#endif

#if USE_VAT
#ifndef __MORPHOS__
char * __vat_appid = "Voyager " VERTAG;
ULONG __vat_requirements = VATIR_OS3;
#endif /* !__MORPHOS__ */
#endif /* USE_VAT */

#ifdef __SASC
// SAS/C Startup vars
long __stack = 65536;
long __oslibversion = 37;
#endif

//TOFIX: check if that works
#ifdef __MORPHOS__
long __stack = 65536 * 2;
#endif

void displaybeep( void )
{
#ifndef MBX
	DisplayBeep( NULL );
#endif
}

// TOFIX!
#ifdef __MORPHOS__
void dprintf(char *, ... );
#define kprintf dprintf
#endif /* __MORPHOS__ */

void STDARGS reporterror( char *msg, ... )
{
	va_list va;
	char buffer[ 2048 ];

	va_start( va, msg );

	vsprintf( buffer, msg, va );

	va_end( va );

	kprintf( "INTERNAL ERROR: \007" );
	kprintf( buffer );
	kprintf( " [Free Mem: %lx/%lx]\r\n", AvailMem( 0 ), AvailMem( MEMF_LARGEST ) );	
}

static struct MsgPort *internalipcport;
static ULONG internalipcsig;

void init_internalipc( void )
{
	internalipcport = CreatePort( VIPCNAME, -32 );
	if( internalipcport )
	{
		internalipcsig = 1L<<internalipcport->mp_SigBit;
	}
}

void cleanup_internalipc( void )
{
	if( internalipcport )
	{
		struct Message *m;
		
		D( db_init, bug( "cleaning up..\n" ) );
		Forbid();

		while( m = GetMsg( internalipcport ) )
			ReplyMsg( m );

		DeletePort( internalipcport );
		Permit();
	}
}

static void checkinternalipc( void )
{
	struct voyager_msg *m;

	if( !internalipcport )
		return;

	while( m = (struct voyager_msg*)GetMsg( internalipcport ) )
	{
		char *p;
		int flags = m->flags;

		if( m->parms )
		{
			p = strdup( m->parms ); /* TOFIX */
		}
		else
		{
			p = strdup( "" ); /* TOFIX */
		}
		
		ReplyMsg( (struct Message *)m );
		if( flags & VCMD_GOTOURL_FLAG_NEWWIN )
		{
			win_create( "", p, NULL, NULL, FALSE, FALSE, FALSE );
		}
		else
		{
			DoMethod( app, MM_DoLastActiveWin, MM_HTMLWin_SetURL, p, NULL, NULL, MF_HTMLWin_AddURL );
		}
		free( p );
	}
}

extern struct MinList pluginlist;


#ifdef MBX
#define USE_SMARTCARDFAKE
#endif

#ifdef USE_SMARTCARDFAKE
#include <drivers/smartcard.h>
#include <inspiration_lib_calls.h>
#define InspirationBase MUIBase->mui_InspirationBase
#endif

#if USE_TESTFILE
static void urltest( void )
{
	static time_t lasttest;
	char buffer[ 512 ], *p;
	extern BPTR urltestfile;

	time_t now = time( 0 );

	if( now - lasttest < 30 )
		return;
	lasttest = now;

	for(;;)
	{
		if( !FGets( urltestfile, buffer, sizeof( buffer ) ) )
		{
			// Rewind
			Seek( urltestfile, 0, OFFSET_BEGINNING );
			{
				if( !FGets( urltestfile, buffer, sizeof( buffer ) ) )
				{
					// Bizarre error, I'd say
					Close( urltestfile );
					urltestfile = NULL;
					return;
				}
			}
		}
		p = strpbrk( buffer, " \t\r\n" );
		if( p )
			*p = 0;
		if( !buffer[ 0 ] || buffer[ 0 ] == ';' )
			continue;

		// Send URL
		DoMethod( app, MM_DoLastActiveWin, MM_HTMLWin_SetURL, buffer, NULL, NULL, MF_HTMLWin_AddURL );
		break;
	}

}
#endif

static void doloop( void )
{
	LONG id;
	ULONG sig = 0;
	int Done = FALSE;
#if USE_PLUGINS
	struct plugin *plugin;
#endif

	#ifdef USE_SMARTCARDFAKE
	MsgQueue_p scqueue=NULL;
	DrvIOReqStd_p scior=NULL;
	BOOL scdrv=FALSE;
	UDWORD scsig=-1;
	for (;;)
	{
		if ( (scqueue = CreateMsgQueue("TestSuite", 0, TRUE)) )
		{
			if ( (scior = CreateStdIO( scqueue )) )
			{
				if(!OpenDriver( SMARTCARDDRVNAME, 0, (DrvIOReq_p)scior, 0 ) )
				{
					scdrv = TRUE;

					scior->ior_Command = CMD_SCDINIT;	// general init and interrupt enable
					DoIO( (DrvIOReq_p)scior);

					// *** Add signal bit to smartcard driver for this process    ***
					// *** We need to get informed about a CardChanged interrupt. ***
					if ( -1 != ( scsig = AllocSignal( -1 ) ) )
					{
						SmartCardSignal_s scd_signal;
						scd_signal.scdss_SigProcess = FindProcess(NULL);
						scd_signal.scdss_SignalBit = scsig;
						scior->ior_Command = CMD_SCD_ADD_SIGNAL;
						scior->ior_DataPtr = (VPTR)&scd_signal;
						scior->ior_Length = sizeof( SmartCardSignal_s );
						DoIO( (DrvIOReq_p)scior );
						break;
					}
				}
			}
		}
		break;
	}
	#endif

	while( !Done )
	{
		id = DoMethod( app, MUIM_Application_NewInput, &sig );

		checkmethods();

		switch( id )
		{
			case MUIV_Application_ReturnID_Quit:
				Done = TRUE;
				break;

#if USE_NET
			case MENU_COOKIEBROWSER:
				opencookiebrowserwin();
				break;
#endif /* USE_NET */

#if USE_NET
			case MENU_AUTHBROWSER:
				openauthbrowserwin();
				break;
#endif /* USE_NET */

			case ID_PM_FLUSH_URLS:
				if( MUI_Request( app, 0, 0, GS( NOTE ), GS( FLUSH_CACHE_GAD ), GS( CLICKLINKS_FLUSH ), 0 ) )
				{
					set( app, MUIA_Application_Sleep, TRUE );
					flushurlhistory();                
					set( app, MUIA_Application_Sleep, FALSE );
					MUI_Request( app, 0, 0, GS( NOTE ), GS( OK ), GS( CLICKLINKS_FLUSHED ), 0 );
				}
				break;

			case MENU_CACHEFLUSHIMAGES:
				set( app, MUIA_Application_Sleep, TRUE );
				imgdec_flushimages();
				set( app, MUIA_Application_Sleep, FALSE );
				MUI_Request( app, 0, 0, GS( NOTE ), GS( OK ), GS( IMG_CACHE_FLUSHED ), 0 );
				break;

			case MENU_SET_LOAD:
				{
					char buff[ 256 ];

					strcpy( buff, startup_cfgfile );

					if( prefsfreq( GS( PREFSIO_LOAD_TITLE ), FALSE, buff, "#?.prefs" ) )
					{
						set( app, MUIA_Application_Iconified, TRUE );
						cfg_load( buff );
#if USE_NET
						doallwins( MM_HTMLWin_SetupFastlinks );
#endif /* USE_NET */
						doallwins( MM_HTMLWin_SetupIcon );
						doallwins( MM_HTMLWin_SetupToolbar );
						set( app, MUIA_Application_Iconified, FALSE );
					}
				}
				break;

			case MENU_SET_SAVE:
				cfg_save( startup_cfgfile );
				
#if USE_PLUGINS
				/* tell every plugin to save itself */
				for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
				{
					if( plugin->hasprefs )
						VPLUG_Hook_Prefs( VPLUGPREFS_Save, &plugin->prefs );
				}
#endif
				break;

			case MENU_SET_SAVEASDEF:
				cfg_save( "PROGDIR:Voyager.prefs" );
				break;

			case MENU_SET_SAVEAS:
				{
					char buff[ 256 ];

					strcpy( buff, startup_cfgfile );

					if( prefsfreq( GS( PREFSIO_SAVE_TITLE ), TRUE, buff, "#?.prefs" ) )
					{
						cfg_save( buff );
						strcpy( startup_cfgfile, buff );
					}
				}
				break;

#if USE_CMANAGER
			case MENU_BM_LOAD:
				{
					char buff[ 256 ];

					buff[0] = '\0';

					if( prefsfreq( GS( PREFSIO_BMLOAD_TITLE ), FALSE, buff, "#?.data" ) )
					{
						bm_load( buff );
					}
				}
				break;

			case MENU_BM_SAVE:
				bm_save( FALSE );
				break;

			case MENU_BM_SEARCH:
				{
					if( bm_openwin() )
					{
						DoMethod( cm_obj, MUIM_CManager_Search );
					}
				}
				break;
#endif

			case MENU_SET_MIME:
				runmimeprefs();
				break;

#ifndef MBX
			case MENU_JSSNOOP:
				js_opensnoop();
				break;
#endif

#if USE_CMANAGER
			case ID_BM_OPEN:
				{
					DoMethod( app, MM_App_OpenBMWin );
					break;
				}

			case MENU_BM_SAVEAS:
				{
					bm_save( TRUE );
					break;
				}

			case MENU_BM_IMPORT_VOYAGER:
				{
					char buff[ 256 ];

					*buff = '\0';

					if( prefsfreq( GS( PREFSIO_BM_IMPORT ), FALSE, buff, "#?.html" ) )
					{
						if( bm_create() )
						{
							DoMethod( cm_obj, MUIM_CManager_Import, MUIV_CManager_Import_Voyager, buff, MUIV_CManager_Import_Filter );
						}
					}
					break;
				}
			case MENU_BM_IMPORT_IBROWSE:
				{
					char buff[ 256 ];

					*buff = '\0';

					if( prefsfreq( GS( PREFSIO_BM_IMPORT ), FALSE, buff, "#?.html" ) )
					{
						if( bm_create() )
						{
							DoMethod( cm_obj, MUIM_CManager_Import, MUIV_CManager_Import_IB, buff, MUIV_CManager_Import_Filter );
						}
					}
					break;
				}
 
			case MENU_BM_IMPORT_AWEB:
				{
					char buff[ 256 ];

					*buff = '\0';

					if( prefsfreq( GS( PREFSIO_BM_IMPORT ), FALSE, buff, "#?.hotlist" ) )
					{
						if( bm_create() )
						{
							DoMethod( cm_obj, MUIM_CManager_Import, MUIV_CManager_Import_AWeb, buff, MUIV_CManager_Import_Filter );
						}
					}
					break;
				}

			case MENU_BM_EXPORT_WWW:
				{
					char buff[ 256 ];

					*buff = '\0';

					if( prefsfreq( GS( PREFSIO_BM_EXPORT ), TRUE, buff, "#?.html" ) )
					{
						if( bm_create() )
						{
							DoMethod( cm_obj, MUIM_CManager_Export, MUIV_CManager_Export_HTML_WWW, buff, NULL );
						}
					}
					break;
				}
 
			case MENU_BM_EXPORT_FTP:
				{
					char buff[ 256 ];

					*buff = '\0';

					if( prefsfreq( GS( PREFSIO_BM_EXPORT ), TRUE, buff, "#?.html" ) )
					{
						if( bm_create() )
						{
							DoMethod( cm_obj, MUIM_CManager_Export, MUIV_CManager_Export_HTML_FTP, buff, NULL );
						}
					}
					break;
				}
 
			case MENU_BM_EXPORT_WWW_AND_FTP:
				{
					char buff[ 256 ];

					*buff = '\0';

					if( prefsfreq( GS( PREFSIO_BM_EXPORT ), TRUE, buff, "#?.html" ) )
					{
						if( bm_create() )
						{
							DoMethod( cm_obj, MUIM_CManager_Export, MUIV_CManager_Export_HTML_URLs, buff, NULL );
						}
					}
					break;
				}

			case MENU_BM_GOTO: /* display the bookmarks as HTML page in the current window */
				{
					if( bm_create() )
					{
						DoMethod( cm_obj, MUIM_CManager_Export, MUIV_CManager_Export_HTML_URLs, "T:VTempBookmarks.html", NULL );
						DoMethod( app, MM_DoActiveWin, MM_HTMLWin_SetURL, "file:///T:VTempBookmarks.html", NULL, NULL, MF_HTMLWin_AddURL | MF_HTMLWin_Reload );
					}
					break;
				}
#endif /* USE_CMANAGER */

#ifndef MBX
			case MENU_SETWINSIZE_1:
			case MENU_SETWINSIZE_2:
			case MENU_SETWINSIZE_3:
				{
					extern APTR lastactivewin;
					struct Window *w = 0;
					static int __far xs[ 3 ] = { 640, 800, 1024 };
					static int __far ys[ 3 ] = { 480, 600, 768 };

					if( lastactivewin )
					{
						get( _win( lastactivewin ), MUIA_Window_Window, &w );
						if( w )
						{
							ChangeWindowBox( w, w->LeftEdge, w->TopEdge,
								xs[ id - MENU_SETWINSIZE_1 ],
								ys[ id - MENU_SETWINSIZE_1 ]
							);
						}
					}
				}
				break;
#endif

		}

		if( Done )
			break;

		if( sig )
		{
#if USE_NET
			ULONG waitsig = sig | internalipcsig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_E | authportsigmask;
#else
			ULONG waitsig = sig | internalipcsig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_E;
#endif /* !USE_NET */

			#ifdef USE_SMARTCARDFAKE
			if (scsig!=-1) waitsig |= (1<<scsig);
			#endif

			sig = Wait( waitsig );

			#if USE_TESTFILE
			{
				extern BPTR urltestfile;

				if( urltestfile )
					urltest();
			}
			#endif

			#ifdef USE_SMARTCARDFAKE
			if (scsig!=-1 && (sig & (1<<scsig)))
			{
				DWORD cardstatus = 0L;
			  	scior->ior_Command = CMD_SCD_QUERYSTATUS;
			  	scior->ior_DataPtr = (VPTR)&cardstatus;
			  	scior->ior_Length = sizeof( DWORD );
			  	DoIO( (DrvIOReq_p)scior );
			  	if( cardstatus & SCD_QSTAT_CARDINSERTED )
				{
			  		KPrintF( "Smartcard was inserted.\r\n" );
					SendInspireMessage(GetActiveWindow(),IDCMP_RAWKEY,0x2ce,0x0000,NULL);
					SendInspireMessage(GetActiveWindow(),IDCMP_RAWKEY,0x2ce|0x8000,0x0000,NULL);
				}
			  	else
				{
			  		KPrintF( "Smartcard was removed.\r\n" );
					SendInspireMessage(GetActiveWindow(),IDCMP_RAWKEY,0x2cd,0x0000,NULL);
					SendInspireMessage(GetActiveWindow(),IDCMP_RAWKEY,0x2cd|0x8000,0x0000,NULL);
				}
			}
			#endif

			if( sig & SIGBREAKF_CTRL_C )
				Done = TRUE;

			if( sig & SIGBREAKF_CTRL_F )
				set( app, MUIA_Application_Iconified, FALSE );

#if USE_NET
			if( sig & authportsigmask )
				auth_process();
#endif /* USE_NET */
		}

		checkmethods();
		checkinternalipc();

#if 0
{
static int lcnt;
if(lcnt++%20==0)
kprintf("free %ld/%ld\n",AvailMem(0),AvailMem(MEMF_LARGEST));
}
#endif

	}

	#ifdef USE_SMARTCARDFAKE
	if (scdrv) CloseDriver( (DrvIOReq_p)scior );
	if (scior) DeleteStdIO( scior );
	if (scqueue) DeleteMsgQueue( scqueue, TRUE);
	if (scsig!=-1) FreeSignal(scsig);
	#endif
}

extern char **openurls;
extern int piratekey;

#define KEYNAMECODE 89
#define CH(x) x^KEYNAMECODE

static char pirateurl[] = { 
	CH('h'),CH('t'),CH('t'),CH('p'),CH(':'),CH('/'),CH('/'),
	CH('p'),CH('i'),CH('r'),CH('a'),CH('t'),CH('s'),CH('.'),
	CH('v'),CH('a'),CH('p'),CH('o'),CH('r'),CH('.'),
	CH('c'),CH('o'),CH('m'),CH('/'),CH('p'),CH('i'),CH('r'),CH('a'),CH('t'),CH('e'),CH('.'),
	CH('h'),CH('t'),CH('m'),CH('l'),
0, 0, 0 };

int init_keyname2( void )
{
	char *x = pirateurl;
	
	D( db_init, bug( "initializing..\n" ) );

	while( *x )
		*x++ ^= KEYNAMECODE;

	return( *x );
}

#if USE_KEYFILES
extern int serialnumber;
#endif

#ifdef __MORPHOS__
extern struct ExecBase *SysBase; /* needs global one (startup code) */
#endif

/*
 * This is used to avoid argc/argv parsing
 */
#ifdef AMIGAOS
void STDARGS __main( char *dummy )
#endif
#ifdef __MORPHOS__
int main( int argc, char *argv[  ] )
#endif
#ifdef MBX
int vmain( void )
#endif
{
	/*
	 * HACK! so that isspace() recognizes ascii 11 (vtab)
	 */
#if ( defined(__GNUC__) || defined(__DCC__) )
	((char*)_ctype_)[ 11 + 1 ] |= _S;
#else
	__ctype[ 11 + 1 ] |= _S;
#endif
	
	if( !initstuff() )
	{
		D( db_init, bug( "*** COULDN'T INITIALIZE! SHUTTING DOWN..\n" ) );
		closestuff();
		goto ex;
	}
	app_started = TRUE;

#if USE_KEYFILES
	if( !serialnumber )
	{
#if NEED_KEYFILE
		MUI_Request( app, NULL, 0, "Voyager BETA Notice", "*Goodbye",
			"\033cThis is a beta version of Voyager which\n"
			"is available to registered users ONLY!"
		);
		closestuff();
		goto ex;
#endif /* NEED_KEYFILE */
		if( !donag( 1 ) )
		{
			closestuff();
			goto ex;
		}
	}
#endif /* !USE_KEYFILE */

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_DONE ) );
	}
#endif

#if USE_KEYFILES
	if( piratekey )
	{
		win_create( "PieRat", pirateurl, NULL, NULL, FALSE, TRUE, FALSE );
	}
	else
#endif /* USE_KEYFILES */
	if( openurls )
	{
		while( *openurls )
		{
			win_create( "", *openurls, NULL, NULL, FALSE, FALSE, FALSE/*getflag( VFLG_FULLSCREEN )*/ );
			openurls++;
		}
	}
	else
	{
		win_create( "", getflag( VFLG_HOMEPAGE_AUTOLOAD ) ? getprefs( DSI_HOMEPAGE ) : "" , NULL, NULL, FALSE, FALSE, FALSE /*getflag( VFLG_FULLSCREEN )*/ );
	}

#if USE_SPLASHWIN
	if( splashwin )
	{
		set( splashwin, MUIA_Window_Open, FALSE );
		DoMethod( app, OM_REMMEMBER, splashwin );
		MUI_DisposeObject( splashwin );
	}
#endif

	doloop();

	closestuff();

ex:  

#ifndef MBX
	exit( 0 );
#else
	return( 0 );
#endif

	//**  initialjump( (HOOKFUNC)( func1 ), (HOOKFUNC)( func2 ) );
}


