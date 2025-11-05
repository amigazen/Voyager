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
** $Id: plugins.c,v 1.73 2004/02/07 14:22:43 zapek Exp $
**
*/

#include "voyager.h"

#if USE_PLUGINS

/* system */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "voyager_cat.h"
#include <proto/v_plugin.h>
#ifdef MBX
#define NOCALLMACROS /* TOFIX: same */
#include "md/v_plugin_lib_calls.h"
#endif
#include "network.h"
#include "classes.h"
#include "urlparser.h"
#include "plugins.h"
#include "splashwin.h"
#include "methodstack.h"
#include "malloc.h"
#include "prefs.h"
#include "htmlclasses.h"
#include "mui_func.h"
#include "dos_func.h"


#ifndef MBX
static ULONG __far prefs_colormap[24] = {
	0x95959595, 0x95959595, 0x95959595,
	0x00000000, 0x00000000, 0x00000000,
	0xffffffff, 0xffffffff, 0xffffffff,
	0x3b3b3b3b, 0x67676767, 0xa2a2a2a2,
	0x7b7b7b7b, 0x7b7b7b7b, 0x7b7b7b7b,
	0xafafafaf, 0xafafafaf, 0xafafafaf,
	0xaaaaaaaa, 0x90909090, 0x7c7c7c7c,
	0xffffffff, 0xa9a9a9a9, 0x97979797
};

static UWORD __chip i_plugin[3][28] = {
	{
		0x0000,0x0000,0x3541,0x0200,0x20ff,0x5a00,0x27a1,0x1e00,
		0x2deb,0xba00,0x0fab,0xaa00,0x17ff,0xba00,0x0460,0x7a00,
		0x0f53,0xfa00,0x0ed3,0x5a00,0x00ff,0xf200,0x7faa,0xbe00,
		0x00ff,0xf000,0x0000,0x0000
	},
	{
		0x7fff,0xfc00,0x5fff,0xfe00,0x5f47,0xa400,0x587e,0xe400,
		0x5154,0x4c00,0x7274,0x4c00,0x7830,0x4c00,0x79df,0x8c00,
		0x72bc,0x1c00,0x73af,0x5c00,0x4054,0x1c00,0x207f,0xe000,
		0x00d5,0x5000,0x0000,0x0000
	},
	{
		0x0000,0x0200,0x3fef,0xec00,0x30e5,0x2500,0x3059,0x9d00,
		0x3a68,0x0500,0x3060,0x1500,0x3864,0x0500,0x25fc,0x7d00,
		0x2338,0x0d00,0x2308,0xad00,0x3f55,0x5d00,0x4000,0x0100,
		0x1f00,0x0f00,0x007f,0xf000
	},
};

#define BITMAP(x) {4,14,0,3,0,(PLANEPTR)&##x##[0][0],(PLANEPTR)&##x##[1][0],(PLANEPTR)&##x##[2][0]} 

static struct BitMap plugin_bm  = BITMAP( i_plugin );
#endif /* !MBX */

struct MinList pluginlist;
static int initdone;

struct vplug_functable ftable;
#ifdef __MORPHOS__
struct vplug_functable ftableppc;
#endif /* __MORPHOS__ */

#ifdef __MORPHOS__
#define DECLPFPPC(type,name) static type vfunc_ppc_##name
#define DECLPF(type,name) \
	static type vfunc_##name##(void); \
	static type GATE2_vfunc_##name##( void ); \
	struct EmulLibEntry GATE_vfunc_##name = \
	{ \
		TRAP_LIB, 0, (void (*)(void)) vfunc_##name \
	}; \
	static type vfunc_##name##( void ) \
	{ \
		return( GATE2_vfunc_##name##() ); \
	} \
	static type GATE2_vfunc_##name##( void )
#else
#define DECLPF(type,name) static type ASM SAVEDS vfunc_##name

#endif

/*
 * nets_# forwarding functions
 *
 * For MorphOS there's a real PPC version (PPC plugins)
 * and one with 68k gates (68k plugins and plugins wishing
 * to use a 68k callback table).
 */
#ifdef __MORPHOS__
DECLPFPPC( APTR, net_openurl )( __reg( a0, STRPTR url ), __reg( a1, STRPTR ref ), __reg( a2, APTR infoobj ), __reg( d0, int reload ) )
{
	return( nets_open( url, ref, infoobj, NULL, NULL, 0, reload ? NOF_RELOAD : 0 ) );
}

DECLPF( APTR, net_openurl )
{
	STRPTR url = ( STRPTR )REG_A0;
	STRPTR ref = ( STRPTR )REG_A1;
	APTR infoobj = ( APTR )REG_A2;
	int reload = ( int )REG_D0;
#else
DECLPF( APTR, net_openurl )( __reg( a0, STRPTR url ), __reg( a1, STRPTR ref ), __reg( a2, APTR infoobj ), __reg( d0, int reload ) )
{
#endif /* !__MORPHOS__ */
	return( nets_open( url, ref, infoobj, NULL, NULL, 0, reload ? NOF_RELOAD : 0 ) );
}


#ifdef __MORPHOS__
DECLPFPPC( int, net_state )( __reg(a0, APTR handle ) )
{
	return( nets_state( handle ) );
}

DECLPF( int, net_state )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( int, net_state )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_state( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( void, net_close )( __reg(a0, APTR handle ) )
{
	nets_close( handle );
}

DECLPF( void, net_close )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( void, net_close )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	nets_close( handle );
}


#ifdef __MORPHOS__
DECLPFPPC( void, net_abort )( __reg(a0, APTR handle ) )
{
	nets_abort( handle );
}

DECLPF( void, net_abort )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( void, net_abort )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	nets_abort( handle );
}


#ifdef __MORPHOS__
DECLPFPPC( STRPTR, net_mimetype )( __reg(a0, APTR handle ) )
{
	return( nets_mimetype( handle ) );
}

DECLPF( STRPTR, net_mimetype )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( STRPTR, net_mimetype )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_mimetype( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( STRPTR, net_url )( __reg(a0, APTR handle ) )
{
	return( nets_url( handle ) );
}

DECLPF( STRPTR, net_url )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( STRPTR, net_url )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_url( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( STRPTR, net_redirecturl )( __reg(a0, APTR handle ) )
{
	return( nets_redirecturl( handle ) );
}

DECLPF( STRPTR, net_redirecturl )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( STRPTR, net_redirecturl )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_redirecturl( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( STRPTR, net_errorstring )( __reg(a0, APTR handle ) )
{
	return( nets_redirecturl( handle ) );
}

DECLPF( STRPTR, net_errorstring )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( STRPTR, net_errorstring )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_redirecturl( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( APTR, net_getdocmem )( __reg(a0, APTR handle ) )
{
	return( nets_getdocmem( handle ) );
}

DECLPF( APTR, net_getdocmem )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( APTR, net_getdocmem )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_getdocmem( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( int, net_getdocptr )( __reg(a0, APTR handle ) )
{
	return( nets_getdocptr( handle ) );
}

DECLPF( int, net_getdocptr )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( int, net_getdocptr )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_getdocptr( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( int, net_getdoclen )( __reg(a0, APTR handle ) )
{
	return( nets_getdoclen( handle ) );
}

DECLPF( int, net_getdoclen )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( int, net_getdoclen )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	return( nets_getdoclen( handle ) );
}


#ifdef __MORPHOS__
DECLPFPPC( void, net_settomem )( __reg(a0, APTR handle ) )
{
	nets_settomem( handle );
}

DECLPF( void, net_settomem )
{
	APTR handle = ( APTR )REG_A0;
#else
DECLPF( void, net_settomem )( __reg(a0, APTR handle ) )
{
#endif /* !__MORPHOS__ */
	nets_settomem( handle );
}


#ifdef __MORPHOS__
DECLPFPPC( void, net_settofile )( __reg(a0, APTR handle ),__reg(a1, STRPTR filename ), __reg(d0, int resume ) )
{
	nets_settofile( handle, filename, resume );
}

DECLPF( void, net_settofile )
{
	APTR handle = ( APTR )REG_A0;
	STRPTR filename = ( STRPTR )REG_A1;
	int resume = ( int )REG_D0;
#else
DECLPF( void, net_settofile )( __reg(a0, APTR handle ),__reg(a1, STRPTR filename ), __reg(d0, int resume ) )
{
#endif /* !__MORPHOS__ */
	nets_settofile( handle, filename, resume );
}


#ifdef __MORPHOS__
DECLPFPPC( void, net_lockdocmem )( void )
{
	nets_lockdocmem();
}
DECLPF( void, net_lockdocmem )
#else
DECLPF( void, net_lockdocmem )( void )
#endif /* !__MORPHOS__ */
{
	nets_lockdocmem();
}


#ifdef __MORPHOS__
DECLPFPPC( void, net_unlockdocmem )( void )
{
	nets_unlockdocmem();
}
DECLPF( void, net_unlockdocmem )
#else
DECLPF( void, net_unlockdocmem )( void )
#endif /* !__MORPHOS__ */
{
	nets_unlockdocmem();
}


#ifdef __MORPHOS__
DECLPFPPC( int, domethoda)( __reg(a0, APTR obj ), __reg(a1, APTR msg ) )
{
	return( ( int )pushsyncmethod( obj, msg ) );
}

DECLPF( int, domethoda)
{
	APTR obj = ( APTR )REG_A0;
	APTR msg = ( APTR )REG_A1;
#else
DECLPF( int, domethoda)( __reg(a0, APTR obj ), __reg(a1, APTR msg ) )
{
#endif /* !__MORPHOS__ */
	return( ( int )pushsyncmethod( obj, msg ) );
}

void seturl_sub( STRPTR url, STRPTR target, int reload )
{
	if( ( url = strdup( url ) ) )
	{
	    if( target )
		{
			if( !( target = strdup( target ) ) )
			{
				free( url );
				displaybeep();
				return;
			}
		}

		pushmethod( app, 6,
			MM_DoActiveWin, MM_HTMLWin_SetURL,
			url,
			NULL,
			target,
			( reload ? MF_HTMLWin_Reload : 0 ) | MF_HTMLWin_AddURL | MF_HTMLWin_FreeURL | MF_HTMLWin_FreeTarget
		);
	}
	else
	{
		displaybeep(); /* TOFIX: should be displayed in the status bar, really */
	}
}

#ifdef __MORPHOS__
DECLPFPPC( void, seturl)( __reg(a0, STRPTR url ), __reg( a1, STRPTR target ), __reg(d0, int reload ) )
{
	seturl_sub( url, target, reload );
}

DECLPF( void, seturl)
{
	STRPTR url = ( STRPTR )REG_A0;
	STRPTR target = ( STRPTR )REG_A1;
	int reload = ( int )REG_D0;
#else
DECLPF( void, seturl)( __reg(a0, STRPTR url ), __reg( a1, STRPTR target ), __reg(d0, int reload ) )
{
#endif /* !__MORPHOS__ */
	seturl_sub( url, target, reload );
}


#ifdef __MORPHOS__
DECLPFPPC( void, mergeurl)( __reg(a0, STRPTR url ), __reg(a1, STRPTR part ), __reg(a2, STRPTR dest ) )
{
	uri_mergeurl( url, part, dest );
}

DECLPF( void, mergeurl)
{
	STRPTR url = ( STRPTR )REG_A0;
	STRPTR part = ( STRPTR )REG_A1;
	STRPTR dest = ( STRPTR )REG_A2;
#else
DECLPF( void, mergeurl)( __reg(a0, STRPTR url ), __reg(a1, STRPTR part ), __reg(a2, STRPTR dest ) )
{
#endif /* !__MORPHOS__ */
	uri_mergeurl( url, part, dest );
}

extern ULONG colspec2rgb24( char *cs );
#ifdef __MORPHOS__
DECLPFPPC( void, colorspec2rgb )( __reg(a0, STRPTR colorspec ), __reg(d0, ULONG *red ), __reg(d1, ULONG *green ), __reg(d2, ULONG *blue ) )
{
	ULONG v;
	v = colspec2rgb24( colorspec );
	*red = v >> 16;
	*green = ( v >> 8 ) & 0xff;
	*blue = v & 0xff;
}

DECLPF( void, colorspec2rgb )
{
	STRPTR colorspec = ( STRPTR )REG_A0;
	ULONG *red = ( ULONG *)REG_D0;
	ULONG *green = ( ULONG *)REG_D1;
	ULONG *blue = ( ULONG *)REG_D2;
#else
DECLPF( void, colorspec2rgb )( __reg(a0, STRPTR colorspec ), __reg(d0, ULONG *red ), __reg(d1, ULONG *green ), __reg(d2, ULONG *blue ) )
{
#endif /* !__MORPHOS__ */
	ULONG v;
	v = colspec2rgb24( colorspec );
	*red = v >> 16;
	*green = ( v >> 8 ) & 0xff;
	*blue = v & 0xff;
}

#define EAC_BUFFERSIZE 2048 /* ExAll buffer */

void init_plugins( void )
{
	struct plugin *plugin;
#if USE_DOS
	BPTR l;
	char pattern[ 64 ];
	struct ExAllControl *eac;
	struct ExAllData *ead;
	APTR ex_buffer;
#endif

	D( db_init, bug( "initializing..\n" ) );

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_PLUGINS ) );
	}
#endif

	NEWLIST( &pluginlist );
	initdone = TRUE;

	/*
	 * Setup functable (s)
	 */
	ftable.vplug_functabversion = VPLUG_FUNCTABVERSION;
#ifdef __MORPHOS__
	ftableppc.vplug_functabversion = VPLUG_FUNCTABVERSION;
#define SFUNC(x) ftable.vplug_##x = ( APTR )&GATE_vfunc_##x ; ftableppc.vplug_##x = vfunc_ppc_##x
#else
#define SFUNC(x) ftable.vplug_##x = vfunc_##x
#endif /* !__MORPHOS__ */

	SFUNC( net_openurl );
	SFUNC( net_state );
	SFUNC( net_close );
	SFUNC( net_mimetype );
	SFUNC( net_getdocmem );
	SFUNC( net_getdocptr );
	SFUNC( net_getdoclen );
	SFUNC( net_settomem );
	SFUNC( net_settofile );
	SFUNC( net_redirecturl );
	SFUNC( net_url );
	SFUNC( net_lockdocmem );
	SFUNC( net_unlockdocmem );
	SFUNC( net_abort );
	SFUNC( net_errorstring );
	SFUNC( domethoda );
	SFUNC( seturl );
	SFUNC( mergeurl );
	SFUNC( colorspec2rgb );

#ifdef MBX
/*
 * Since OpenModule() doesn't support DOS, we have to
 * add ugly hacks. Only VFlash supported as plugin for now.
 */
	if( ( plugin = malloc( sizeof( *plugin ) ) ) )
	{
		struct TagItem *tag, *tagp;

		memset( plugin, 0, sizeof( *plugin ) );

		strcpy( plugin->name, "vflash_mcf" );
		plugin->querylist = VPLUG_Query();
		plugin->api_version = 1;
		plugin->isppc = FALSE;

		tagp = plugin->querylist;

		while( tag = NextTagItem( &tagp ) ) switch( tag->ti_Tag )
		{
			case VPLUG_Query_APIVersion:
				plugin->api_version = tag->ti_Data;
				break;

			case VPLUG_Query_HasProcessURLString:
				plugin->hasurlstring = tag->ti_Data;
				break;
		}

		ADDTAIL( &pluginlist, plugin );

		if( plugin->api_version > 1 )
		{
			VPLUG_Setup( &ftable );
		
			VPLUG_FinalSetup();
		}
	}
	else
	{
		displaybeep();
	}
#else /* !MBX */

#if USE_DOS
	l = Lock( "PROGDIR:Plugins", SHARED_LOCK );
	if( !l )
		return;

#ifdef MBX
	if( ( eac = AllocMem( sizeof( eac ), MEMF_CLEAR ) ) )
#else /* !MBX */
	if( ( eac = AllocDosObject( DOS_EXALLCONTROL, NULL ) ) )
#endif /* !MBX */
	{
		if( ( ex_buffer = malloc( EAC_BUFFERSIZE ) ) )
		{
			if( ParsePatternNoCase( "#?.VPlug(%|.elf)", pattern, sizeof( pattern ) ) > -1 )
			{
				int more;

				eac->eac_MatchString = pattern;
				eac->eac_LastKey = 0;
				do
				{
					more = ExAll( l, ex_buffer, EAC_BUFFERSIZE, ED_NAME, eac );

					if( ( !more ) && ( IoErr() != ERROR_NO_MORE_ENTRIES ) )
					{
						displaybeep();
						break; /* TOFIX */
					}

					if( eac->eac_Entries > 0 )
					{
					
						char fullname[ 256 ];
						struct Library *lb;
						ead = ( struct ExAllData * )ex_buffer;

						do
						{
							strcpy( fullname, "PROGDIR:Plugins" );
							AddPart( fullname, ead->ed_Name, sizeof( fullname ) );
							
							/*
							 * Find out PPC plugins by their name
							 */
							if( strlen( ead->ed_Name ) > 4 )
							{
								if( stricmp( ".elf", ( unsigned char const * )ead->ed_Name + strlen( ead->ed_Name ) - 4 ) == 0 )
								{
									fullname[ strlen( fullname ) - 4 ] = '\0'; /* remove '.elf' */
								}
							}

							lb = OpenLibrary( fullname, 0 );        

							if( lb )
							{
								struct TagItem *tag, *tagp;
								int callback68k = TRUE;

								plugin = malloc( sizeof( *plugin ) ); /* TOFIX: tsk tsk */

								memset( plugin, 0, sizeof( *plugin ) );

								strcpy( plugin->name, ead->ed_Name );
								*strchr( plugin->name, '.' ) = '\0';

								plugin->libbase = lb;
								plugin->querylist = VPLUG_Query();
								plugin->api_version = 1;
								
								/* no no, this is not messy :) */
								Forbid();
								plugin->isppc = ( ( *( ULONG * )*( ( ULONG * )lb - 1 ) & 0xff000000 ) == 0xff000000 ) ? TRUE : FALSE;
								Permit();

								tagp = plugin->querylist;

								while( tag = NextTagItem( &tagp ) ) switch( tag->ti_Tag )
								{
									case VPLUG_Query_APIVersion:
										plugin->api_version = tag->ti_Data;
										break;

									case VPLUG_Query_HasPrefs:
										plugin->hasprefs = tag->ti_Data;
										break;

									case VPLUG_Query_HasProcessURLString:
										plugin->hasurlstring = tag->ti_Data;
										break;
								
									case VPLUG_Query_PPC_DirectCallbacks:
										callback68k = FALSE;
										break;
								}

								/*
								 * If we are V68k we must disallow PPC plugins
								 * not using a 68k callback table for
								 * obvious reasons.
								 */
#ifdef AMIGAOS
								if( plugin->isppc && !callback68k )
								{
									CloseLibrary( lb );
									free( plugin );
								}
								else
#endif
								{
									ADDTAIL( &pluginlist, plugin );

									/*
									 * Call setup with the correct
									 * table format
									 */
									if( plugin->api_version > 1 )
									{
#ifdef __MORPHOS__
										if( plugin->isppc && !callback68k )
										{
											VPLUG_Setup( &ftableppc );
										}
										else
#endif /* __MORPHOS__ */
										{
											VPLUG_Setup( &ftable );
										}
									}
						 		
#ifndef MBX
									if( !plugin->prefs.bitmap )
									{
										plugin->prefs.bitmap = &plugin_bm;
									}
#endif

									if( !plugin->prefs.colormap )
										plugin->prefs.colormap = prefs_colormap;
									if( !plugin->prefs.label )
										plugin->prefs.label = plugin->name;
								}
							}
							ead = ead->ed_Next;
						} while( ead );
					}
				} while( more );
			}
			else
			{
				displaybeep();
			}
			free( ex_buffer );
			
		}
#ifdef MBX
		FreeMem( eac, sizeof( eac ) );
#else /* !MBX */
		FreeDosObject( DOS_EXALLCONTROL, eac );
#endif /* !MBX */

		for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
		{
			// Call setup
			if( plugin->api_version > 1 )
			{
				VPLUG_FinalSetup();
				VPLUG_Hook_Prefs( VPLUGPREFS_Setup, &plugin->prefs );
				VPLUG_Hook_Prefs( VPLUGPREFS_Load, &plugin->prefs );
			}
		}
	}
	else
	{
		displaybeep(); /* TOFIX: we should really try to display something at that point... or fail */
	}

	UnLock( l );
#endif /* USE_DOS */

#endif /* !MBX */

	return;
}

void cleanup_plugins( void )
{
	struct plugin *plugin;

	D( db_init, bug( "cleaning up..\n" ) );
	
	if( initdone )
	{
		while( plugin = REMHEAD( &pluginlist ) )
		{
			if( plugin->api_version > 1 )
			{
				if( plugin->hasprefs )
				{
					if( getflag( VFLG_SAVE_ON_EXIT ) )
						VPLUG_Hook_Prefs( VPLUGPREFS_Save, &plugin->prefs );
					
					VPLUG_Hook_Prefs( VPLUGPREFS_Cleanup, &plugin->prefs );
				}
			
				VPLUG_Cleanup(); 
			}

#if USE_DOS
			CloseLibrary( plugin->libbase );
#endif
		}
	}
}


/*
 * Iterate through the plugins to find one
 * which handles the URL method
 */
STRPTR plugin_processurl( char *url, int *size )
{
	struct plugin *plugin;

	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		struct TagItem *tag;

		while( ( tag = FindTagItem( VPLUG_Query_RegisterURLMethod, plugin->querylist ) ) )
		{
	        if( tag )
			{
				if( !strnicmp( url, (STRPTR)tag->ti_Data, strlen( (STRPTR)tag->ti_Data ) ) )
				{
					APTR urh;
					STRPTR res = NULL;
					// yeah, we found an appropriate URL
					urh = VPLUG_ProcessURLMethod( url );
					if( urh )
					{
						STRPTR gurl = VPLUG_GetURLData( urh );
						int datasize = -1;

						if( GetTagData( VPLUG_Query_HasURLMethodGetSize, FALSE, plugin->querylist ) )
							datasize = VPLUG_GetURLDataSize( urh );

						if( datasize < 0 )
							datasize = strlen( gurl ) + 1;

						if( gurl )
						{
							res = malloc( datasize );
							memcpy( res, gurl, datasize );
							*size = datasize;
						}
						VPLUG_FreeURLData( urh );

						if( res )
							return( res );
					}
				}
			}
		}
	}

	return( NULL );
}

/*
 * Iterate through the plugins to find one
 * which processes the URL
 */
int plugin_processurlstring( char *url )
{
	struct plugin *plugin;
	int rc = 0;

	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		if( plugin->hasurlstring )
		{
			rc = VPLUG_ProcessURLString( url );
			if( rc < 0 )
				break;
		}
	}

	return( rc );
}


/*
 * Iterate through the plugins to find
 * one which handles the mimetype
 */
APTR plugin_mimetype( STRPTR mimetype, STRPTR mimeextension )
{
	struct plugin *plugin;

	for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
	{
		struct TagItem *tag, *tagp;

		tagp = plugin->querylist;

		while( tag = NextTagItem( &tagp ) )
		{
			if( tag->ti_Tag == VPLUG_Query_RegisterMIMEType )
			{
				if( !stricmp( (STRPTR)tag->ti_Data, mimetype ) )
				{
					return( VPLUG_GetClass( mimetype ) );
				}
			}
		}
	}
	
	/*
	 * Try to find by extension then
	 */
	if ( mimeextension )
	{
		return( plugin_mimeextension( mimeextension, TRUE ) );
	}
	else
	{
		return( 0 );
	}
}


/*
 * Iterate through the plugins to find
 * one which handles the mime extension
 */
APTR plugin_mimeextension( STRPTR mimeextension, int returnclass )
{
	struct plugin *plugin;
	
	if( mimeextension )
	{
		for( plugin = FIRSTNODE( &pluginlist ); NEXTNODE( plugin ); plugin = NEXTNODE( plugin ) )
		{
			struct TagItem *tag, *tagp;

			tagp = plugin->querylist;

			while( tag = NextTagItem( &tagp ) )
			{
				if( tag->ti_Tag == VPLUG_Query_RegisterMIMEExtension && mimeextension )
				{
					char tmp[ 1024 ];
					char *p;
					
					stccpy( tmp, ( STRPTR )tag->ti_Data, sizeof( tmp ) );

					p = strtok( tmp, " " );
					while( p )
					{
						if( !stricmp( p, mimeextension + 1 ) )
						{
							return( returnclass ? VPLUG_GetClass( mimeextension + 1 ) : plugin );
						}
						p = strtok( NULL, " " );
					}
				}
			}
		}
	}
	return( 0 );
}

#endif /* USE_PLUGINS */
