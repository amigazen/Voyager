/*
 * $Id: vat.c,v 1.37 2002/10/28 14:35:52 zapek Exp $
 */

#include "globals.h"

#ifdef __SASC

#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/rexxsyslib.h>
#include <proto/muimaster.h>
#include <proto/asl.h>
#include <proto/datatypes.h>
#include <proto/battclock.h>
#include <proto/timer.h>
#include <proto/openurl.h>

#endif /* __SASC */

#include <exec/execbase.h>
#include <libraries/mui.h>
#include <devices/timer.h>
#include <string.h>
#include <dos/dostags.h>
#include <dos/var.h>
#include <ctype.h>
#include <dos/notify.h>
#include <datatypes/datatypesclass.h>
#include <exec/memory.h>
#include <time.h>
#include <macros/vapor.h>
#include <rexx/storage.h>
#include <libraries/asl.h>

#ifdef __SASC
#include "/include/gmp.h"
#else
#include "vat_ctype.h"
#include "gmp.h"
#endif /* TOFIX: er... how sucky */

#include <proto/vaporreg.h>

#include "voy_ipc.h"

#include "libraries/vat.h"
#include "malloc.h"

#if INCLUDE_VERSION < 44
struct Library *UtilityBase;
struct Library *RexxSysBase;
#else
struct UtilityBase *UtilityBase;
struct RxsLib *RexxSysBase;
#endif
struct Library *TimerBase;

struct IntuitionBase *IntuitionBase;
struct DosLibrary *DOSBase;
struct ExecBase *SysBase;
struct GfxBase *GfxBase;
struct Library *MUIMasterBase;
struct Library *DataTypesBase;
struct Library *OpenURLBase;
static struct SignalSemaphore MPZSem;

static struct timerequest treq;

#ifndef __MORPHOS__
extern void ASM initpoolsysbase( __reg( a0, APTR ) );
extern void ASM call_rxhandler( void );
extern void ASM call_regutil( void );
#endif /* !__MORPHOS__ */
extern void random_firstinit( void );

extern void initmimeprefs( void );
extern void exitmimeprefs( void );
extern void loadmimeprefs( int lock );

UWORD fmtfunc[] = { 0x16c0, 0x4e75 };
#ifndef __MORPHOS__
//	sprintf() replacement
void __stdargs sprintf( char *to, char *fmt, ... )
{
	RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, to );
}
#endif /* !__MORPHOS__ */

static struct Process *rxhandler_proc;
static struct MsgPort *rxhandler_vipcport;
static struct MsgPort *rxhandler_rxport;
static struct MsgPort *rxhandler_quitport;

#ifdef __MORPHOS__
/*
 * We define separate structures
 * due to the asynchronous nature
 * of CreateNewProc()
 */
struct EmulFunc mos_emulfunc_rx;
struct EmulFunc mos_emulfunc_reg;

struct TagItem mos_tags_rx[ 5 ];
struct TagItem mos_tags_reg[ 5 ];

#endif /* __MORPHOS__ */

struct rxhandler_quitnode {
	struct Message m;
	struct Task *quittask;
	int cnt;
};

#undef OSCHECK
#ifdef OSCHECK
static void checkos( void )
{
	char buffer[ 128 ];
	struct EasyStruct eas;
	struct Library *vlib;
	int vmajor, vminor;

	if( GetVar( "VAPOR/UNSAFE_OS_OK", buffer, sizeof( buffer ), 0 ) > 0 )
		return;

	if( !( vlib = OpenLibrary( "version.library", 0 ) ) )
		return;

	vmajor = vlib->lib_Version;
	vminor = vlib->lib_Revision;

	CloseLibrary( vlib );

	if( vmajor < 41 )
		return;

	memset( &eas, 0, sizeof( eas ) );

	eas.es_StructSize = sizeof( eas );
	eas.es_Title = "Vapor Toolkit Warning";
	eas.es_GadgetFormat = "OK|Do not warn again";
	eas.es_TextFormat = 
		"Your Workbench version %ld.%ld is not\n"
		"currently supported by this application.\n\n"
		"You may still run it on your own\n"
		"risk, but strange effects may occur.\n\n"
		"To avoid this warning in the future,\n"
		"you can set\n"
		" ENV:VAPOR/UNSAFE_OS_OK\n"
		"to \"YES\", or click on 'Don't warn\n"
		"again' below."
	;

	if( EasyRequest( NULL, &eas, NULL, vmajor, vminor ) == 0 )
	{
		BPTR f;

		UnLock( CreateDir( "ENV:VAPOR" ) );
		UnLock( CreateDir( "ENVARC:VAPOR" ) );
		f = Open( "ENV:VAPOR/UNSAFE_OS_OK", MODE_NEWFILE );
		if( f )
		{
			Write( f, "YES", 3 );
			Close( f );
		}
		f = Open( "ENVARC:VAPOR/UNSAFE_OS_OK", MODE_NEWFILE );
		if( f )
		{
			Write( f, "YES", 3 );
			Close( f );
		}
	}
}
#endif

void ASM rxhandler( void )
{
	struct RexxMsg *m;
	int done = FALSE;
	ULONG sigs = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F;
	struct NotifyRequest nr;
	struct MinList quitlist;
	struct rxhandler_quitnode *qn;
	struct MsgPort *timerport;
	struct timerequest *treq;
	int tq = FALSE;

	timerport = CreateMsgPort();
	treq = CreateIORequest( timerport, sizeof( *treq ) );
	OpenDevice( "timer.device", UNIT_VBLANK, (APTR)treq, 0 );
	sigs |= 1L << timerport->mp_SigBit;

	rxhandler_vipcport = CreateMsgPort();
	sigs |= 1L << rxhandler_vipcport->mp_SigBit;

	rxhandler_rxport = CreateMsgPort();
	sigs |= 1L << rxhandler_rxport->mp_SigBit;

	rxhandler_quitport = CreateMsgPort();
	sigs |= 1L << rxhandler_quitport->mp_SigBit;

	memset( &nr, 0, sizeof( nr ) );

	NEWLIST( &quitlist );

	nr.nr_Name = "ENV:MIME.prefs";
	nr.nr_stuff.nr_Signal.nr_SignalNum = SIGBREAKB_CTRL_F;
	nr.nr_stuff.nr_Signal.nr_Task = FindTask( NULL );
	nr.nr_Flags = NRF_SEND_SIGNAL;

	StartNotify( &nr );

#ifdef CHECKOS
	checkos();
#endif

	while( !done )
	{
		ULONG rsigs;

		if( !tq )
		{
			treq->tr_node.io_Command = TR_ADDREQUEST;
			treq->tr_time.tv_secs = 11;
			treq->tr_time.tv_micro = 11;
			SendIO( (APTR)treq );
			tq = TRUE;
		}

		rsigs = Wait( sigs );

		if( CheckIO( (APTR)treq ) )
		{
			WaitIO( (APTR)treq );
			tq = FALSE;

			
			//kprintf( "timer check\n" );

			// check quitlist
			for( qn = FIRSTNODE( &quitlist ); NEXTNODE( qn ); qn = NEXTNODE( qn ) )
			{
				//kprintf( "node %lx, cnt %ld\n", qn, qn->cnt );
				if( qn->cnt-- == 0 )
				{
					Signal( qn->quittask, SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F );
				}
			}
		}

		if( rsigs & SIGBREAKF_CTRL_C )
			done = TRUE;

		if( rsigs & SIGBREAKF_CTRL_F )
		{
			loadmimeprefs( TRUE );
		}

		while( m = (APTR)GetMsg( rxhandler_rxport ) )
		{
			ClearRexxMsg( m, 1 );
			DeleteRexxMsg( m );
		}

		while( m = (APTR)GetMsg( rxhandler_vipcport ) )
		{
			FreeVec( m );
		}

		while( qn = (APTR)GetMsg( rxhandler_quitport ) )
		{
			qn->cnt = 8;
			ADDTAIL( &quitlist, qn );
		}
	}

	if( tq )
	{
		AbortIO( (APTR)treq );
		WaitIO( (APTR)treq );
	}

	EndNotify( &nr );

	DeleteMsgPort( rxhandler_rxport );
	DeleteMsgPort( rxhandler_quitport );
	DeleteMsgPort( rxhandler_vipcport );

	CloseDevice( (APTR)treq );
	DeleteIORequest( (APTR)treq );
	DeleteMsgPort( timerport );

	Forbid(); /* poof, let's vanish into the void */
	rxhandler_proc = 0;
}

int lib_init( struct ExecBase *SBase )
{
	SysBase = SBase;

	return( TRUE );
}

int lib_open( void )
{
	DB( ( "k, in libopen now, SysBase == 0x%lx..\n", SysBase ) );
	if( ( DOSBase = (struct DosLibrary*)OpenLibrary( "dos.library", 37 ) ) )
	{
		DB( ( "ok ?\n" ) );

#ifndef __MORPHOS__
		initpoolsysbase( SysBase );
#endif /* !__MORPHOS__ */

		if( init_globalpool() )
		{
			if( ( IntuitionBase = (struct IntuitionBase*)OpenLibrary( "intuition.library", 37 ) ) )
			{
				if( ( GfxBase = (struct GfxBase*) OpenLibrary( "graphics.library", 37 ) ) )
				{
#if INCLUDE_VERSION < 44
					if( ( UtilityBase = OpenLibrary( "utility.library", 37 ) ) )
					{
						if( ( RexxSysBase = OpenLibrary( "rexxsyslib.library", 1 ) ) )
						{
#else
					if( ( UtilityBase = (struct UtilityBase *)OpenLibrary( "utility.library", 37 ) ) )
					{
						if( ( RexxSysBase = (struct RxsLib *)OpenLibrary( "rexxsyslib.library", 1 ) ) )
						{
#endif
							if( OpenDevice( "timer.device", UNIT_MICROHZ, (APTR)&treq, 0  ) == 0 )
							{
								TimerBase = (APTR)treq.tr_node.io_Device; /* beware: we use TimerBase to check if libOpen succeeded */

								random_firstinit();

								initmimeprefs();

								InitSemaphore( &MPZSem );

#define VAT_RXHANDLER_NAME "Vapor Toolkit Lib Rexx Reply Handler"
#define VAT_RXHANDLER_PRI 10
#define VAT_RXHANDLER_STACKSIZE_M68K 4096
#define VAT_RXHANDLER_COPYVARS FALSE

#ifdef __MORPHOS__
								mos_emulfunc_rx.Trap = TRAP_FUNC;
								mos_emulfunc_rx.Address = ( ULONG )rxhandler;
								mos_emulfunc_rx.StackSize = VAT_RXHANDLER_STACKSIZE_M68K * 2; /* to be on the safe side */
								mos_emulfunc_rx.Extension = 0;
								mos_emulfunc_rx.Arg1 = ( ULONG )SysBase;
								mos_emulfunc_rx.Arg2 = 0;
								mos_emulfunc_rx.Arg3 = 0;
								mos_emulfunc_rx.Arg4 = 0;
								mos_emulfunc_rx.Arg5 = 0;
								mos_emulfunc_rx.Arg6 = 0;
								mos_emulfunc_rx.Arg7 = 0;
								mos_emulfunc_rx.Arg8 = 0;

								mos_tags_rx[ 0 ].ti_Tag = NP_Entry;
								mos_tags_rx[ 0 ].ti_Data = ( ULONG )&mos_emulfunc_rx.Trap;
								mos_tags_rx[ 1 ].ti_Tag = NP_Name;
								mos_tags_rx[ 1 ].ti_Data = ( ULONG )VAT_RXHANDLER_NAME;
								mos_tags_rx[ 2 ].ti_Tag = NP_Priority;
								mos_tags_rx[ 2 ].ti_Data = VAT_RXHANDLER_PRI;
								mos_tags_rx[ 3 ].ti_Tag = NP_CopyVars;
								mos_tags_rx[ 3 ].ti_Data = VAT_RXHANDLER_COPYVARS;
								mos_tags_rx[ 4 ].ti_Tag = TAG_DONE;

								rxhandler_proc = CreateNewProc( &mos_tags_rx[ 0 ] );
#else
								rxhandler_proc = CreateNewProcTags(
									NP_Entry, call_rxhandler,
									NP_Name, VAT_RXHANDLER_NAME,
									NP_Priority, VAT_RXHANDLER_PRI,
									NP_StackSize, VAT_RXHANDLER_STACKSIZE_M68K,
									NP_CopyVars, VAT_RXHANDLER_COPYVARS,
									TAG_DONE
								);
#endif /* !__MORPHOS__ */
								return ( TRUE );
							}
						}
					}
				}
			}
		}
	}
	return( FALSE );
}

void lib_cleanup( void );

long SAVEDS ASM __UserLibInit( __reg( a6, struct Library *libbase ) )
{
	if( lib_init( *( (struct ExecBase **)4 ) ) )
	{
		if( lib_open() )
		{
			libbase->lib_Node.ln_Pri = -111;

			return( 0 ); /* success */
		}
		lib_cleanup();
	}

	return( -1 );
}

void lib_cleanup( void )
{
	while( rxhandler_proc )
	{
		Signal( ( struct Task * )rxhandler_proc, SIGBREAKF_CTRL_C );
		Delay( 1 );
	}

	exitmimeprefs();

	if( TimerBase )
	{
		CloseDevice( (APTR)&treq );
	}

	cleanup_globalpool();

	CloseLibrary( (struct Library *)DOSBase );
	CloseLibrary( (struct Library *)GfxBase );
	CloseLibrary( (struct Library *)IntuitionBase );
	CloseLibrary( (struct Library *)UtilityBase );
	CloseLibrary( (struct Library *)RexxSysBase );
	CloseLibrary( MUIMasterBase );
	CloseLibrary( DataTypesBase );
}

#ifdef __SASC
void SAVEDS ASM __UserLibCleanup(void)
{
	lib_cleanup();
}
#endif

//

void SAVEDS ASM VAT_SetLastUsedDir( __reg( a0, char *appid ) )
{
	char buff[ 128 ];
	char path[ 128 ];
	BPTR f;
	int envarcchanged = TRUE;

	DB( ( "called\n" ) );

	NameFromLock( GetProgramDir(), buff, sizeof( buff ) );

	if( !( f = Lock( "ENV:Vapor", SHARED_LOCK ) ) )
		f = CreateDir( "ENV:Vapor" );
	UnLock( f );
	if( !( f = Lock( "ENVARC:Vapor", SHARED_LOCK ) ) )
		f = CreateDir( "ENVARC:Vapor" );
	UnLock( f );
	
	sprintf( path, "ENVARC:Vapor/%s_LASTUSEDDIR", appid );
	
	f = Open( path, MODE_OLDFILE );
	if( f )
	{
		char buff2[ 128 ];
	
		if( Read( f, buff2, sizeof( buff2 ) ) == strlen( buff ) )
		{
			if( !memcmp( buff, buff2, strlen( buff ) ) )
				envarcchanged = FALSE;
		}
		Close( f );
	}

	if( envarcchanged )
	{
		f = Open( path, MODE_NEWFILE );
		if( f )
		{
			Write( f, buff, strlen( buff ) );
			Close( f );
		}
	}
	
	sprintf( path, "ENV:Vapor/%s_LASTUSEDDIR", appid );
	f = Open( path, MODE_NEWFILE );
	if( f )
	{
		Write( f, buff, strlen( buff ) );
		Close( f );
	}
}

struct pathlist
{
	BPTR next;
	BPTR lock;
};

static int testseg( char *name )
{
	BPTR seglist;

	seglist = LoadSeg( name );

	if( seglist )
		UnLoadSeg( seglist );

	return( ( seglist ) ? 1 : 0 );
}

static int testseginlock( char *name, BPTR lock )
{
	int rc;
	BPTR oldcd;

	oldcd = CurrentDir( lock );
	rc = testseg( name );
	CurrentDir( oldcd );
	return( rc );
}

static void makefullname( char *name, BPTR dir, char *fullpath )
{
	if( !fullpath )
		return;

	NameFromLock( dir, fullpath, 256 );
	AddPart( fullpath, name, 256 );
}

int ASM SAVEDS VAT_CheckProgramInPathFull(
	__reg( a0, char *name ),
	__reg( a1, char *fullpath )
)
{
	char buffer[ 256 ], *p;
	BPTR cd;
	int rc;
	struct CommandLineInterface *cli;
	struct pathlist *pl;
	struct Process *pr = (struct Process*)FindTask( 0 );

	DB( ( "called\n" ) );

	strcpy( buffer, name );
	p = strchr( buffer, ' ' );
	if( p )
		*p = 0;

	if( fullpath )
		strcpy( fullpath, name );

	/* Resident? */
	Forbid();
	if( FindSegment( buffer, NULL, FALSE ) )
	{
		Permit();
		return( 1 );
	}
	Permit();

	/* Erst CD */
	if( testseg( buffer ) )
	{
		makefullname( name, pr->pr_CurrentDir, fullpath );
		return( 1 );
	}

	/* dann C: */
	cd = Lock( "C:", SHARED_LOCK );
	rc = testseginlock( buffer, cd );
	if( rc )
		makefullname( name, cd, fullpath );
	UnLock( cd );

	if( rc )
		return( 1 );

	/* Cli? */
	cli = BADDR( pr->pr_CLI );
	if( !cli )
		return( 0 );

	pl = BADDR( cli->cli_CommandDir );

	while( pl )
	{
		if( testseginlock( buffer, pl->lock ) )
		{
			makefullname( name, pl->lock, fullpath );
			return( 1 );
		}
		pl = BADDR( pl->next );	
	}

	return( 0 );
}

struct Library * ASM SAVEDS VAT_OpenLibrary(
	__reg( a0, char *libname ),
	__reg( d0, long libversion ) )
{
	struct Library *l;
	char tpath[ 128 ];
	int triedflush = FALSE;

	DB( ( "called\n" ) );

retry:
	l = OpenLibrary( libname, libversion );
	if( !l && !strpbrk( libname, ":/" ) )
	{
		sprintf( tpath, "LIBS/%s", libname );
		l = OpenLibrary( tpath, libversion );
	}
	if( !l && !strpbrk( libname, ":/" ) )
	{
		sprintf( tpath, "/LIBS/%s", libname );
		l = OpenLibrary( tpath, libversion );
	}

	if( !l && !triedflush )
	{
		APTR x;
		triedflush = TRUE;
		x = AllocVec( 0xfffffff, 0 );
		if( x )
			FreeVec( x );
		goto retry;
	}

	if(!l)
	{
		struct EasyStruct eas;
		char titletext[ 80 ];
		char dirtext[ 80 ];
		char appid[ 80 ];

		if( MYPROC->pr_WindowPtr == ( APTR ) -1 )
			return( NULL );

		if( !GetProgramName( titletext, 80 ) )
			strncpy( titletext, MYPROC->pr_Task.tc_Node.ln_Name, 79 );

		if( GetVar( "__VATAPPID", appid, sizeof( appid ), GVF_LOCAL_ONLY ) > 0 )
			strcpy( titletext, appid );
		else
			strcpy( appid, titletext );

		strcpy( dirtext, libname );
		*PathPart( dirtext ) = 0;
		if( !dirtext[ 0 ] )
			strcpy( dirtext, "LIBS:" );

		eas.es_StructSize = sizeof( struct EasyStruct );
		eas.es_Flags = 0;
		eas.es_Title = titletext;
		eas.es_TextFormat = "Error: %s requires (at least) V%ld of\n\"%s\"\nin \"%s\"";
		eas.es_GadgetFormat = "Retry|Cancel";

		if( EasyRequest( MYPROC->pr_WindowPtr, &eas, 0, ( ULONG )appid, ( ULONG )libversion, ( ULONG )FilePart( libname ), ( ULONG )dirtext ) )
			goto retry;
	}
	return(l);
}

char *libarray[] = {
	"graphics",
	"utility",
	"workbench",
	"icon",
	"commodities",
	"layers",
	"iffparse",
	"cybergraphics",
	"datatypes",
	"diskfont",
	"rexxsyslib",
	"asl",
	"intuition",
	"mathtrans",
	"mathffp",
	"mathieeedoubtrans",
	"mathieeedoubbas",
	"vapor_update"
};

struct Library * ASM SAVEDS VAT_OpenLibraryCode( __reg( d0, int code ) )
{
	char buffer[ 32 ];

	DB( ( "called\n" ) );

	sprintf( buffer, "%s.library", libarray[ code ] );
	return( VAT_OpenLibrary( buffer, 0 ) );
}

int ASM SAVEDS VAT_Initialize(
	__reg( a0, STRPTR appid ),
	__reg( a1, APTR *libvec ),
	__reg( a2, STRPTR ctypevec ),
	__reg( d0, ULONG requirements )
)
{
	char *req1 = "";
	char *req2 = "";
	char *req3 = "";

	DB( ( "called\n" ) );

	if( !MUIMasterBase )
		MUIMasterBase = OpenLibrary( "muimaster.library", 6 );

#ifdef __MORPHOS__
	if( ctypevec )
		memcpy( ctypevec, vat_ctype, 0x104 );
#else
	if( ctypevec )
		memcpy( ctypevec, __ctype, 0x104 );
#endif

	SetVar( "__VATAPPID", appid, -1, GVF_LOCAL_ONLY );

	if( requirements & VATIR_OS3 )
	{
		if( SysBase->LibNode.lib_Version < 39 )
			req1 = "· OS 3.0 or higher\n";
	}
	if( requirements & VATIR_020 )
	{
		if( ! ( SysBase->AttnFlags & AFF_68020 ) )
			req2 = "· 68020 CPU (or better)\n";
	}
	if( requirements & VATIR_FPU )
	{
		if( ! ( SysBase->AttnFlags & AFF_68881 ) )
			req3 = "· 68881 FPU (or better)\n";
	}

	if( *req1 || *req2 || *req3 )
	{
		struct EasyStruct eas;

		if( MYPROC->pr_WindowPtr == ( APTR ) -1 )
			return( -1 );

		eas.es_StructSize = sizeof( struct EasyStruct );
		eas.es_Flags = 0;
		eas.es_Title = appid;
		eas.es_TextFormat = "Error: %s requires an\nAmiga System equipped with\n%s%s%sPlease obtain a version\nsuitable for your system.";
		eas.es_GadgetFormat = "Cancel";

		EasyRequest( MYPROC->pr_WindowPtr, &eas, 0, ( ULONG )appid, ( ULONG )req1, ( ULONG )req2, ( ULONG )req3 );

		return( -1 );
	}

	if( libvec )
	{
		libvec[ 0 ] = VAT_OpenLibraryCode( VATOC_GFX );
		libvec[ 1 ] = VAT_OpenLibraryCode( VATOC_UTIL );
		libvec[ 2 ] = VAT_OpenLibraryCode( VATOC_WB );
		libvec[ 3 ] = VAT_OpenLibraryCode( VATOC_LAYERS );
	}

	return( 0 );
}

void ASM SAVEDS VAT_Cleanup( __reg( a0, APTR *libvec ) )
{
	int c;

	DB( ( "called\n" ) );

	SetVar( "__VATAPPID", "", 0, GVF_LOCAL_ONLY );

	if( libvec )
	{
		for( c = 0; c < 4; c++ )
			CloseLibrary( *libvec++ );
	}
}

int ASM SAVEDS VAT_CheckVATVersion( __reg( d0, ULONG version ) )
{
	DB( ( "called\n" ) );
	
	if( version > VAT_VERSION )
	{
		struct EasyStruct eas;
		char titletext[ 80 ];
		char appid[ 80 ];

		if( MYPROC->pr_WindowPtr == ( APTR ) -1 )
			return( -1 );

		if( !GetProgramName( titletext, 80 ) )
			strncpy( titletext, MYPROC->pr_Task.tc_Node.ln_Name, 79 );

		if( GetVar( "__VATAPPID", appid, sizeof( appid ), GVF_LOCAL_ONLY ) > 0 )
			strcpy( titletext, appid );
		else
			strcpy( appid, titletext );

		eas.es_StructSize = sizeof( struct EasyStruct );
		eas.es_Flags = 0;
		eas.es_Title = titletext;
		eas.es_TextFormat = "Error: %s requires (at least)\nV%ld of vapor_toolkit.library\nin LIBS:, but you only have\nV%ld installed. Please update.\n\nIf you think you have updated correctly,\nyou might have to reboot because an\nolder version of the library is still in memory.";
		eas.es_GadgetFormat = "Cancel";

		EasyRequest( MYPROC->pr_WindowPtr, &eas, 0, ( ULONG )appid, ( ULONG )version, ( ULONG )VAT_VERSION );

		return( -1 );
	}

	return( 0 );
}

int ASM SAVEDS VAT_SendRXMsg( __reg( a0, STRPTR cmd ), __reg( a1, STRPTR basename ), __reg( a2, STRPTR suffix ) )
{
	struct RexxMsg *rm = CreateRexxMsg( rxhandler_rxport, suffix, basename );
	struct MsgPort *rexxhost;
	TEXT buffer[ 1024 ];

	DB( ( "called\n" ) );

	if( !rm )
		return( -1 );


	if( *cmd != '"' && *cmd != 39 )	// is command not rexx code?
	{
		STRPTR mark = strchr( cmd, ' ' );
		BPTR l;

		if( mark )
			*mark = 0;	// split file/args

		strcpy( buffer, "CALL \"PROGDIR:Rexx/" );
		strcat( buffer, cmd );
		if( !strchr( buffer, '.' ) )
		{
			strcat( buffer, "." );
			strcat( buffer, suffix );
		}
		l = Lock( &buffer[6], SHARED_LOCK );
		if( l )
		{
			NameFromLock( l, &buffer[6], 256 );
			UnLock( l );
		}
		else
			strcpy( &buffer[6], cmd );

		strcat( buffer, "\"(\"" );

		if( mark )		// do we have any arguments behind filename
		{
			STRPTR p = &buffer[ strlen( buffer ) ];
			*p = *mark++ = ' ';

			// copy args doubling any quotes for rexx func call
			while( *mark )
			{
				*p = *mark++;
				if( *p++ == '"' )
					*p++ = '"';
			}
			*p = 0;
		}

		strcat( buffer, "\")" );

		rm->rm_Args[ 0 ] = buffer;
		FillRexxMsg( rm, 1, 0 );
		rm->rm_Action = RXCOMM | RXFF_NOIO | RXFF_STRING;
	}
	else
	{
		rm->rm_Args[ 0 ] = cmd;
		FillRexxMsg( rm, 1, 0 );
		rm->rm_Action = RXCOMM | RXFF_NOIO;
	}

	Forbid();
	rexxhost = FindPort( "REXX" );
	if( rexxhost )
	{
		PutMsg( rexxhost, (APTR)rm );
	}
	else
	{
		ClearRexxMsg( rm, 1 );
		DeleteRexxMsg( rm );
	}
	Permit();

	return( 0 );
}

void ASM SAVEDS VAT_ShowURL( __reg( a0, char *url ), __reg( a1, STRPTR rexxsuffix ), __reg( a2, APTR muiapp ) )
{
	struct MsgPort *vport;
	struct voyager_msg *vmsg;
	BPTR l;

	DB( ( "called\n" ) );

	if( !FindPort( VIPCNAME ) )
	{
		char buffer[ 256 ];
		OpenURLBase = OpenLibrary( "openurl.library", 0 );

		if( rexxsuffix )
			sprintf( buffer, "PROGDIR:Rexx/SendBrowser.%s", rexxsuffix );

		if( OpenURLBase )
		{
			URL_Open( url, TAG_DONE );
			CloseLibrary( OpenURLBase );
			return;
		}
		else if( rexxsuffix && ( l = Lock( buffer, SHARED_LOCK ) ) )
		{
			char buffer[ 800 ];
			STRPTR basename = 0;

			if( muiapp )
				get( muiapp, MUIA_Application_Base, &basename );

			UnLock(l);
			sprintf( buffer, "SendBrowser.%s \"%s\"", rexxsuffix, url );
			VAT_SendRXMsg( buffer, rexxsuffix, basename );
			return;
		}
		else if( GetVar( "Vapor/Voyager_LASTUSEDDIR", buffer, sizeof( buffer ), 0 ) <= 0 )
		{
			struct FileRequester *fr;
			char *pubname = "Workbench";

retry:
			if( muiapp )
			{
				get( muiapp, MUIA_Application_PubScreenName, &pubname );
				if( !pubname || !*pubname || !strcmp( pubname, "Default" ) )
					pubname = "Workbench";
			}	
			fr = MUI_AllocAslRequestTags( ASL_FileRequest,
				ASLFR_PubScreenName, ( ULONG )pubname,
				ASLFR_InitialFile, ( ULONG )"V",
				ASLFR_TitleText, ( ULONG )"Select your web browser...",
				TAG_DONE
			);
			if( fr )
			{
				if( !MUI_AslRequestTags( fr, TAG_DONE ) )
				{
					MUI_FreeAslRequest( fr );
					return;
				}
				strcat( buffer, fr->fr_Drawer );
				AddPart( buffer, fr->fr_File, sizeof( buffer ) );
				MUI_FreeAslRequest( fr );
			}
		}
		else
		{
			BPTR l;

			AddPart( buffer, "V", sizeof( buffer ) );
			l = Lock( buffer, SHARED_LOCK );
			if( !l )
			{
				goto retry;
			}
			UnLock( l );
		}

		// execute it
		strins( buffer, "\"" );
		strcat( buffer, "\" \"" );
		strcat( buffer, url );
		strcat( buffer, "\"" );

		SystemTags( buffer,
			SYS_Asynch, TRUE,
			SYS_Input, Open( "NIL:", MODE_NEWFILE ),
			SYS_Output, Open( "NIL:", MODE_NEWFILE ),
			TAG_DONE
		);

		return;
	}

	vmsg = AllocVec( sizeof( *vmsg ) + strlen( url ) + 1, MEMF_CLEAR );
	if( vmsg )
	{
		vmsg->m.mn_ReplyPort = rxhandler_vipcport;

		vmsg->cmd = VCMD_GOTOURL;
		vmsg->parms = (char*)(vmsg + 1);
		strcpy( vmsg->parms, url );

		Forbid();
		if( vport = FindPort( VIPCNAME ) )
		{
			PutMsg( vport, (struct Message*)vmsg );
		}
		else
			FreeVec( vmsg );
		Permit();
	}
}

void ASM SAVEDS VAT_MPZPow(
	__reg( a0, MP_INT *od ),
	__reg( a1, MP_INT *id ),
	__reg( a2, MP_INT *pe ),
	__reg( a3, MP_INT *pn )
)
{
	DB( ( "called\n" ) );

	ObtainSemaphore( &MPZSem );	
	mpz_powm( od, id, pe, pn );
	ReleaseSemaphore( &MPZSem );
}

void ASM SAVEDS VAT_MPZFree( __reg( a0, MP_INT *od ) )
{
	DB( ( "called\n" ) );
	
	ObtainSemaphore( &MPZSem );	
	mpz_clear( od );
	ReleaseSemaphore( &MPZSem );
}

static struct Screen *regscr;
static STRPTR regappname;

void ASM regutil_func( void )
{
	struct Library *VaporRegBase;

	DB( ( "called\n" ) );

	VaporRegBase = VAT_OpenLibrary( "vapor_registration.library", 7 );
	if( VaporRegBase )
	{
		VREG_ShowRegProduct( regscr, regappname );
		CloseLibrary( VaporRegBase );
	}
}

extern struct Screen * ASM SAVEDS VAT_GetAppScreen( __reg( a0, APTR app ) );

void run_regutil( APTR ptr, int net, STRPTR name )
{
	if( net )
	{
		regscr = ptr ? VAT_GetAppScreen( ptr ) : NULL;
		regappname = name;
	}
	else
	{
		regscr = ptr;
		regappname = NULL;
	}

#define VAT_REGUTIL_NAME "Vapor Regutil"
#define VAT_REGUTIL_STACKSIZE_M68K 24 * 1024
#define VAT_REGUTIL_PRI 0
#define VAT_REGUTIL_COPYVARS FALSE

#ifdef __MORPHOS__
	mos_emulfunc_reg.Trap = TRAP_FUNC;
	mos_emulfunc_reg.Address = ( ULONG )regutil_func;
	mos_emulfunc_reg.StackSize = VAT_REGUTIL_STACKSIZE_M68K * 2; /* to be on the safe side */
	mos_emulfunc_reg.Extension = 0;
	mos_emulfunc_reg.Arg1 = ( ULONG )SysBase;
	mos_emulfunc_reg.Arg2 =	0;
	mos_emulfunc_reg.Arg3 =	0;
	mos_emulfunc_reg.Arg4 =	0;
	mos_emulfunc_reg.Arg5 =	0;
	mos_emulfunc_reg.Arg6 =	0;
	mos_emulfunc_reg.Arg7 =	0;
	mos_emulfunc_reg.Arg8 =	0;

	mos_tags_reg[ 0 ].ti_Tag = NP_Entry;
	mos_tags_reg[ 0 ].ti_Data = ( ULONG )&mos_emulfunc_reg.Trap;
	mos_tags_reg[ 1 ].ti_Tag = NP_Name;
	mos_tags_reg[ 1 ].ti_Data = ( ULONG )VAT_REGUTIL_NAME;
	mos_tags_reg[ 2 ].ti_Tag = NP_Priority;
	mos_tags_reg[ 2 ].ti_Data = VAT_REGUTIL_PRI;
	mos_tags_reg[ 3 ].ti_Tag = NP_CopyVars;
	mos_tags_reg[ 3 ].ti_Data = VAT_REGUTIL_COPYVARS;
	mos_tags_reg[ 4 ].ti_Tag = TAG_DONE;

	CreateNewProc( &mos_tags_reg[ 0 ] );
#else
	CreateNewProcTags(
		NP_Entry, call_regutil,
		NP_Name, VAT_REGUTIL_NAME,
		NP_Priority, VAT_REGUTIL_PRI,
		NP_StackSize, VAT_REGUTIL_STACKSIZE_M68K,
		NP_CopyVars, FALSE,
		TAG_DONE
	);
#endif

	Delay( 25 );
}

void ASM SAVEDS VAT_ShowRegUtil( __reg( a0, struct Screen *scr ) )
{
	DB( ( "called\n" ) );
	
	run_regutil( scr, FALSE, NULL );
}


void ASM SAVEDS VAT_NewShowRegUtil( __reg( a0, APTR app ), __reg( a1, STRPTR name ) )
{
	DB( ( "called\n" ) );
	
	run_regutil( app, TRUE, name );
}

int ASM SAVEDS VAT_GetDataType(
	__reg( a0, STRPTR filename ),
	__reg( a1, ULONG *group_id ),
	__reg( a2, ULONG *dt_id ),
	__reg( a3, STRPTR namebuffer )
)
{
	struct DataType *dt;
	BPTR f;
	int rc = FALSE;

	DB( ( "called\n" ) );

	if( !DataTypesBase )
	{
		DataTypesBase = OpenLibrary( "datatypes.library", 0 );
		if( !DataTypesBase )
			return( 0 );		
	}

	f = Lock( filename, SHARED_LOCK );

	if( !f )
		return( NULL );

	dt = ObtainDataType( DTST_FILE, (APTR)f, TAG_DONE );

	if( dt )
	{
		struct DataTypeHeader *dh = dt->dtn_Header;
		if( dh )
		{
			if( group_id )
				*group_id = dh->dth_GroupID;
			if( dt_id )
				*dt_id = dh->dth_ID;
			if( namebuffer )
				stccpy( namebuffer, dh->dth_Name, 256 );

			rc = TRUE;
		}
		ReleaseDataType( dt );
	}

	UnLock( f );

	return( rc );
}

int ASM SAVEDS VAT_IsAmigaGuideFile( __reg( a0, STRPTR filename ) )
{
	ULONG gid, id;

	DB( ( "called\n" ) );

	if( VAT_GetDataType( filename, &gid, &id, NULL ) )
	{
		//Printf( "gid = %lx, id = %lx\n", gid, id );
		if( gid == GID_DOCUMENT && id == MAKE_ID('a','m','i','g' ) )
			return( TRUE );
	}
	return( FALSE );
}

int ASM SAVEDS VAT_FGets(
	__reg( a0, BPTR f ),
	__reg( a1, STRPTR buffer ),
	__reg( d0, int size )
)
{
	int rc = (int)FGets( f, buffer, size - 1 );
	char *p;
	
	DB( ( "called\n" ) );

	if( rc > 0 )
	{
		p = strpbrk( buffer, "\r\n" );
		if( p )
			*p = 0;
	}
	return( rc );
}

void ASM SAVEDS VAT_CheckEcrypt( __reg( d0, time_t checkwhen ) )
{
	struct Library *BattClockBase;
	ULONG v = 0;

	DB( ( "called\n" ) );

	// First, find BattClockBase on Exec's list
	Forbid();
	for( BattClockBase = FIRSTNODE( &SysBase->ResourceList ); NEXTNODE( BattClockBase ); BattClockBase = NEXTNODE( BattClockBase ) )
	{
		if( !strcmp( BattClockBase->lib_Node.ln_Name, "battclock.resource" ) )
			break;
	}

	if( NEXTNODE( BattClockBase ) )
		v = ReadBattClock();

	Permit();

	if( !v )
	{
		struct timeval tv;
		GetSysTime( &tv );
		v = tv.tv_secs;
	}

	//kprintf( "v = %lx, checkwhen = %lx\n", v, checkwhen );

	if( v > checkwhen )
	{
		struct rxhandler_quitnode *qn = AllocMem( sizeof( *qn ), MEMF_CLEAR );
		qn->quittask = SysBase->ThisTask;
		PutMsg( rxhandler_quitport, (struct Message*)qn );
	}
}
