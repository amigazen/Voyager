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
 * $Id: keyparse.c,v 1.64 2003/07/06 16:51:33 olli Exp $
 */

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/timer.h>
#include <proto/battclock.h>
#ifdef __MORPHOS__
#include <exec/interrupts.h>
#include <utility/date.h>
#endif
#endif

/* private */
#include "keyfile.h"
#include "classes.h"
#include "time_func.h"
#include "mui_func.h"
#include "dos_func.h"


#if USE_KEYFILES

int piratekey;
int demotimedout;
int serialnumber;
int foundoldkey;

//#include <nc_lib.h>

#ifndef NETCONNECT


#define KEYNAMECODE 0xff
#define CH(x) x^KEYNAMECODE //TOFIX!! oh well...
static char /*__chip*/ akeyname[] = { CH('V'), CH('o'), CH('y'), CH('a'), CH('g'), CH('e'), CH('r'), CH('-'),CH('3'), CH('.'), CH('k'), CH('e'), CH('y'), 0, 0, 0 };

static char *keyltab[] = {
	"PROGDIR:",
	"",
	"S:",
	"KEYPATH:",
	"KEYFILES:",
	NULL
};

#define DEFPIRAT(x) x^MUIA_Listview_DoubleClick,
static ULONG piratelist[] = {
	DEFPIRAT(0)
	DEFPIRAT( 100126 )    	// WOA127, source unknown
	DEFPIRAT( 6252 )    	// Noel Clarke, ftp://obelix.dtvk.tpnet.pl
	DEFPIRAT( 3 )         	// Morden :), ftp://obelix.dtvk.tpnet.pl
	DEFPIRAT( 107777 )    	// NC3 Cologne show pirate key
	DEFPIRAT( 101584 )	// Pirate key, unknown source
	DEFPIRAT( 100630 )	// Pirate key, unknown source sOCIETY/rSE [NC3-00018916]
	DEFPIRAT( 100094 )	// WOA95, source unknown
	DEFPIRAT( 88040 )	// 164UKP stolen card order "Martin Leech"
	DEFPIRAT( 1220 )	// Bernd J. Nink, bounced LS
	DEFPIRAT( 1566 )	// Joachim Nink, bounced LS
	DEFPIRAT( 1150 )	// Robert Zimmermann, bounced LS
	DEFPIRAT( 88149 )	// Stolen card
	DEFPIRAT( 88168 )	// Stolen card
	DEFPIRAT( 83780 )   // Alexandre Kairouannais who thinks it's legal to sell its own keyfiles
	DEFPIRAT( 88188 )	// fial@chataddict.com - CC chargeback
	DEFPIRAT( 88190 )	// digitalgeneration@post.com - CC chargeback
	DEFPIRAT( 88154 )	// dmb9@www.com - CC chargeback
	DEFPIRAT( 88253 )	// palma9@www.com - CC chargeback
	DEFPIRAT( 88133 )	// simonh@istar.ca - CC chargeback
	DEFPIRAT( 88260 )	// sabban@ptt.yu - CC chargeback
	DEFPIRAT( 88262 )	// butur@yubc.net Barry Fox - CC chargeback
	DEFPIRAT( 88208 )	// hoederlin@firemail.de Hans Höderlin - CC chargeback
	DEFPIRAT( 88319 )	// disquebleu@ccitt5.net; Carolyn J. Blackburn; Sat Dec 30 22: 11:04 2000 CC Chargeback
	DEFPIRAT( 88007 )   // Deriu Andreana Caterina - warez key
	DEFPIRAT( 83403 )   // Didier Levet selling his machine with vapor keyfiles as advertising point
};  
#define PIRATNUM (sizeof(piratelist)/sizeof(ULONG))

static void checkpirate( ULONG x )
{
	ULONG *p = piratelist;
	int cnt = PIRATNUM;

	while( cnt-- )
	{   
		piratekey |= ( ( *p ^ MUIA_Listview_DoubleClick ) == x );
		p++;
	}
}

static int loadkey( char *kf )
{
	char buffer[ 239 ];
	int rc;

	strcpy( buffer, kf );
	AddPart( buffer, akeyname, sizeof( buffer ) );
	rc = load_and_parse_key( buffer );
	if( !rc )
		foundoldkey = TRUE;
	return( rc > 0 ? TRUE : FALSE );
}

static BPTR trykeyfile( char *path )
{
	char fullname[ 256 ];

	strcpy( fullname, path );
	AddPart( fullname, "Voyager.KEY", sizeof( fullname ) );

	return( Open( fullname, MODE_OLDFILE ) );
}

void old_loadkey( void )
{
	BPTR f = 0;
	struct Process *me = ( struct Process* )FindTask( NULL );
	APTR oldwp = me->pr_WindowPtr;
	char cmpmd5[ 16 ];
	void (ASM *caller)( __reg( a3, ULONG *tr) );
	char **keyp = keyltab;
	int zero;
	static struct ib {
		char owner[ 80 ];
		ULONG serial;
		time_t creationdate;
		ULONG special1;
		ULONG code; // clr.l (a3); rts
		char md5[ 16 ];
	} ib;

	struct keydir {
		ULONG md5add;
		ULONG encseed;
		UWORD iboffsets[ 4 ];
	} keydir;

	if( serialnumber )
		return;

	me->pr_WindowPtr = (APTR)-1;

	while( *keyp )
	{
		f = trykeyfile( *keyp++ );
		if( f )
			break;
	}
	if( !f )
	{
		char buffer[ 128 ];

		if( GetVar( "KEYPATH", buffer, sizeof( buffer ), 0 ) >= 0 )
			f = trykeyfile( buffer );
		if( !f && GetVar( "KEYFILES", buffer, sizeof( buffer ), 0 ) >= 0 )
			f = trykeyfile( buffer );
	}

	me->pr_WindowPtr = oldwp;

	zero = (ULONG)f;

	if( f )
	{
		int c;
		ULONG *lp;

		piratekey = TRUE;

		Seek( f, 0, OFFSET_END );
		if( Seek( f, 0, OFFSET_BEGINNING ) > 417 )
		{
			Read( f, &keydir, 16 );
			//Printf( "seeking to offset %ld\n", keydir.iboffsets[ 2 ] );
			Seek( f, 512 + keydir.iboffsets[ 2 ], OFFSET_BEGINNING );
			Read( f, &ib, sizeof( ib ) );
			Close( f );
			// decode...
			srand( keydir.encseed + ( 9881 * 2 ) );
			lp = (ULONG*)&ib;
			for( c = 0; c < sizeof( ib ) / 4; c++ )
				*lp++ ^= rand();

			VAT_CalcMD5( (APTR)&ib, sizeof( ib ) - 16, cmpmd5 );
	
			if( !memcmp( ib.md5, cmpmd5, 16 ) )
			{
				CacheClearU();
				caller = (APTR)&ib.code;
				caller( (ULONG*)&zero );
				//memcpy( kf.owner, ib.owner, sizeof( kf.owner ) );
				//Printf( "zero %lx\n", zero );
				piratekey = zero;
				checkpirate( ib.serial );
				foundoldkey = ib.serial;
				return;
			}
		}
		else
			Close( f );

		zero = 0;
		
		return;
	}

	return;
}

void load_key( void )
{
	struct Process *myproc = (struct Process *)FindTask(NULL);
	APTR oldwindowptr = myproc->pr_WindowPtr;
	int rc = 0;
	char **keyl = keyltab;
	char buffer[ 224 ];

	myproc->pr_WindowPtr = (APTR)-1;

	if( GetVar( "KEYPATH", buffer, sizeof( buffer ), 0 ) >= 0 )
		rc = loadkey( buffer );
	if( !rc && GetVar( "KEYFILES", buffer, sizeof( buffer ), 0 ) >= 0 )
		rc = loadkey( buffer );
	while( !rc && *keyl )
		rc = loadkey( *keyl++ );

	myproc->pr_WindowPtr = oldwindowptr;
	if( rc )
	{
		checkpirate( serialnumber = kf.serialnumber );
		
		if( piratekey )
		{
			struct Library *BattClockBase;
			ULONG v = 0;
			struct ClockData cd;

			if( BattClockBase = OpenResource( "battclock.resource" ) )
				v = ReadBattClock();
			if( !v )
			{
				struct timeval tv;
				GetSysTime( &tv );
				v = tv.tv_secs;
			}
			Amiga2Date( v, &cd );
			if( v )
			{
				if( cd.year == 2000 && ( cd.month < 4 || ( cd.month == 4 && cd.mday < 16 ) ) )
					piratekey = FALSE;
				D( db_init, bug( "%ld/%ld/%ld pk %ld\n", cd.mday, cd.month, cd.year, piratekey ) );
			}
		}

		D( db_init, bug( "piratekey %ld\n", piratekey ) );
	}
}

ULONG getserial( void )
{
	if( kf.serialnumber )
		return( kf.serialnumber );
	else
		return( ~0 );
}

char *getowner( void )
{
	if( kf.serialnumber )
		return( kf.owner );
	else
		return( NULL );
}

char *getserialtext( void )
{
	return( kf.serialtext );
}


//
// Timeout stuff
//

static time_t chk_start;

void resetdemotimeout( void )
{
	if( !chk_start )
	{
		time( &chk_start );
	}
}

void init_keyname( void )
{
	char *x = akeyname;
	
	D( db_init, bug( "initializing..\n" ) );

	while( *x )
		*x++ ^= KEYNAMECODE;
}

#ifdef __MORPHOS__
struct IntData
{
	struct EmulLibEntry InterruptFunc;
	struct Interrupt Interrupt;
	struct ExecBase *SysBase;
} mosintdata;
#else
static struct Interrupt si;
#endif /* !__MORPHOS__ */
static struct MsgPort mp;
static struct timerequest tr;
static int tr_pending;

#ifdef __MORPHOS__
static void demotimeouthandler( void )
{
	struct ExecBase *SysBase = mosintdata.SysBase;
#else
static void __interrupt SAVEDS demotimeouthandler( void )
{
#endif /* !__MORPHOS__ */
	if( tr_pending )
	{
		demotimedout = TRUE;
		DoMethod( app, MUIM_Application_PushMethod, app, 1, MM_App_DemoTimeout );
	}
}

void start_demotimeout( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !kf.serialnumber || piratekey )
	{
#ifdef __MORPHOS__
		mosintdata.SysBase = SysBase;
		mosintdata.InterruptFunc.Trap = TRAP_LIB;
		mosintdata.InterruptFunc.Extension = 0;
		mosintdata.InterruptFunc.Func = ( void( * )( void ) )demotimeouthandler;
		mosintdata.Interrupt.is_Node.ln_Type = NT_INTERRUPT;
		mosintdata.Interrupt.is_Node.ln_Pri = 0;
		mosintdata.Interrupt.is_Node.ln_Name = "";
		mosintdata.Interrupt.is_Data = &mosintdata;
		mosintdata.Interrupt.is_Code = ( void ( * )( void ) )&mosintdata.InterruptFunc;
		
		mp.mp_SigTask = &mosintdata.Interrupt;
#else
		si.is_Code = demotimeouthandler;
		mp.mp_SigTask = &si;
#endif /* !__MORPHOS__ */
		
		mp.mp_Flags = PA_SOFTINT;
		NEWLIST( &mp.mp_MsgList );
		tr.tr_node.io_Message.mn_ReplyPort = &mp;
		tr.tr_time.tv_secs = 1905;

		OpenDevice( "timer.device", UNIT_VBLANK, ( struct IORequest * )&tr, 0 );
	
		tr.tr_node.io_Command = TR_ADDREQUEST;
		SendIO( ( struct IORequest * )&tr );

		tr_pending = TRUE;
	}
}

void cleanup_demotimeout( void )
{
	if( tr_pending )
	{
		D( db_init, bug( "cleaning up...\n" ) );
		tr_pending = 0;
		AbortIO( ( struct IORequest * )&tr );
		WaitIO( ( struct IORequest * )&tr );
		CloseDevice( ( struct IORequest * )&tr );
	}
}

#else

// NetConnect
#include <nc_lib.h>

/*
 * TOFIX!! Netconnect is not done yet
 */

ULONG getserial( void )
{
	return( atoi( NCL_GetSerial() ) );
}

char *getowner( void )
{
	return( NCL_GetOwner() );
}

void resetdemotimeout( void )
{

}

void checkdemotimeout( void )
{

}

ULONG serialnumber = 0x8000;

struct Library *NetConnectBase;

int initnc( void )
{
	NetConnectBase = OpenLibrary( "netconnect.library", 4 );
	if( !NetConnectBase || !NCL_GetOwner() )
		return( FALSE );
	serialnumber = atoi( NCL_GetSerial() );
	foundoldkey = TRUE;
	return( TRUE );
}

DESTRUCTOR_P(initnc,500)
{
	CloseLibrary( NetConnectBase );
}

#endif


#endif /* USE_KEYFILES */

#if CHECK_TIMEOUT

int check_timeout( void )
{
	char x[ 6 ];
	
	D( db_init, bug( "initializing..\n" ) );

	utunpk( timev(), x );

	if( x[ 0 ] > ( TIMEOUT_YEAR - 2 - 1970 ) && ( x[ 0 ] < ( TIMEOUT_YEAR - 1970 ) || ( x[ 0 ] <= ( TIMEOUT_YEAR - 1970 ) && x[ 1 ] <= TIMEOUT_MONTH ) ) )
	{
		return( TRUE );
	}
	else
	{
		MUI_Request(
			app, NULL, 0, "Timeout!", "OK",
			"\033c\033uThis beta version of Voyager has timed out!\033n\n\n"
			"Please download a new beta immediately from\n"
			"\033bhttp://v3.vapor.com/\033n"
		);
		#if BAIL_ON_TIMEOUT
		return( FALSE );
		#else
		return( TRUE );
		#endif
	}
}

#endif /* CHECK_TIMEOUT */

