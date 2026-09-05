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
 * Date & time functions
 * ---------------------
 *
 * ? 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: time_func.c,v 1.22 2004/01/06 20:23:08 zapek Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <dos/datetime.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <math64.h>
#endif

/* private */
#include "time_func.h"
#include "dos_func.h"

#define UnixTimeOffset (252482400-(6*3600))

struct Library *TimerBase; /* CBM sucked */
static struct MsgPort *timerport;
static struct timerequest *treq;

static struct tzone {
	char *name;
	int offset;
	int dst;
} tzones[] = {
	{ "UT",  0      , FALSE },
	{ "GMT", 0      , FALSE },
	{ "EST", -5 * 60, FALSE },
	{ "EDT", -4 * 60, TRUE },
	{ "CST", -6 * 60, FALSE },
	{ "CDT", -5 * 60, TRUE },
	{ "MST", -7 * 60, FALSE },
	{ "MDT", -6 * 60, TRUE },
	{ "PST", -8 * 60, FALSE },
	{ "PDT", -7 * 60, TRUE },
	{ 0 }
};


/*
 * Timer initialization
 */
static struct EClockVal start_timed;
static int timed_initialized = 0;
/* Only the task that called init_timer() may use timed(); others get 0 */
static struct Task *timed_owner_task = NULL;

int init_timer( void )
{
	D( db_init, bug( "initializing..\n" ) );

	timed_owner_task = FindTask( NULL );

	timerport = CreateMsgPort();
	if( !timerport )
	{
		D( db_init, bug( "CreateMsgPort() failed for timer\n" ) );
		return( FALSE );
	}

	treq = ( struct timerequest * )CreateIORequest( timerport, sizeof( struct timerequest ) );
	if( !treq )
	{
		D( db_init, bug( "CreateIORequest() failed for timer\n" ) );
		DeleteMsgPort( timerport );
		timerport = NULL;
		return( FALSE );
	}

	if( OpenDevice( "timer.device", UNIT_VBLANK, ( struct IORequest * )treq, 0 ) )
	{
		D( db_init, bug( "timer.device failed to open\n" ) );
		DeleteIORequest( ( struct IORequest * )treq );
		DeleteMsgPort( timerport );
		treq = NULL;
		timerport = NULL;
		return( FALSE );
	}
	D( db_init, bug( "timer.device opened\n" ) );
	TimerBase = ( struct Library * )treq->tr_node.io_Device;

	/*
	 * Set the start time for timed()
	 */
	ReadEClock( &start_timed );
	timed_initialized = 1;

	return( TRUE );
}


/*
 * Timer cleanup
 */
void cleanup_timer( void )
{
	if( TimerBase && treq )
	{
		D( db_init, bug( "cleaning up..\n" ) );

		CloseDevice( ( struct IORequest * )treq );
		DeleteIORequest( ( struct IORequest * )treq );
		treq = NULL;
		TimerBase = NULL;
	}
	if( timerport )
	{
		DeleteMsgPort( timerport );
		timerport = NULL;
	}
	timed_owner_task = NULL;
	timed_initialized = 0;
}


/*
 * Gets the current time
 */
time_t time( time_t *tp )
{
	#ifdef MBX
	return(0); // TOFIX!!
	#else
	struct timeval tv;

#if USE_GETSYSTIME
	GetSysTime( (APTR)&tv );
#endif
	tv.tv_secs += UnixTimeOffset;

	if( tp )
		*tp = tv.tv_secs;
	return( (time_t)tv.tv_secs );
	#endif
}


/*
 * Gets the current time
 */
time_t timev( void )
{
	#ifdef MBX
	return(0); // !!! TOFIX FIXME
	#else
	struct timeval tv;

#if USE_GETSYSTIME
	GetSysTime( (APTR)&tv );
#endif
	return( (time_t)tv.tv_secs + UnixTimeOffset );
	#endif
}


#ifndef MBX
/*
 * Works like timev() but doesn't depend on the system time
 */
time_t timed( void )
{
	struct EClockVal ev;
	ULONG r;
#ifdef __SASC
	QWORD ret;
#endif /* __SASC */
#ifdef __GNUC__
	QWORD b, e;
#endif /* __GNUC__ */

	/* Only use timer in the task that called init_timer() (main process) */
	if( FindTask( NULL ) != timed_owner_task )
		return( (time_t)0 );
	if( !timed_initialized )
		return( (time_t)0 );

	r = ReadEClock( &ev );
	if( !r )
		return( (time_t)0 );

#ifdef __SASC
	q_sub( ( QWORD * )&ev, ( QWORD * )&start_timed, &ret );
	return( ( time_t )q_div( &ret, r ) );
#endif

#ifdef __GNUC__
	e = ev.ev_hi;
	e = ( e << 32L ) | ev.ev_lo;
	b = start_timed.ev_hi;
	b = ( b << 32L ) | start_timed.ev_lo;
	return( ( e - b ) / r );
#endif
}


/*
 * Same as timed() but returns the milliseconds for
 * increased precision.
 */
ULONG timedm( void )
{
	struct EClockVal ev;
	ULONG r;
#ifdef __SASC
	QWORD ret;
#endif /* __SASC */
#ifdef __GNUC__
	QWORD b, e;
#endif /* __GNUC__ */

	if( FindTask( NULL ) != timed_owner_task )
		return( 0UL );
	if( !timed_initialized )
		return( 0UL );

	r = ReadEClock( &ev );
	if( !r )
		return( 0UL );

#ifdef __SASC
	q_sub( ( QWORD * )&ev, ( QWORD * )&start_timed, &ret );
	return( ( ULONG )q_div( &ret, r / 1000 ) );
#endif

#ifdef __GNUC__
	e = ev.ev_hi;
	e = ( e << 32L ) | ev.ev_lo;
	b = start_timed.ev_hi;
	b = ( b << 32L ) | start_timed.ev_lo;
	return( ( e - b ) / ( r / 1000 ) );
#endif
}


/*
 * Just like GetSysTime() but not dependent on the system
 * clock and the timeval starts from when AmIRC was
 * started.
 */
void getlocaltime( struct timeval *dest )
{
	struct EClockVal ev;
	QWORD ret;
	QWORD t;
	ULONG r;

	r = ReadEClock( &ev );

#ifdef __SASC
	q_sub( ( QWORD * )&ev, ( QWORD * )&start_timed, &ret );
	dest->tv_secs = q_div( &ret, r );
	q_mulu( q_mod( &ret, r ), 1000000, &t );
	dest->tv_micro = q_div( &t, r );
#endif

#ifdef __GNUC__
	t = ev.ev_hi;
	t = ( t << 32L ) | ev.ev_lo;
	ret = start_timed.ev_hi;
	ret = ( ret << 32L ) | start_timed.ev_lo;
	ret = t - ret;
	dest->tv_secs = ret / r;
	dest->tv_micro = ret % r * 1000000 / r;
#endif
}

#endif

/*
 * SAS/C's utpack() doesn't consider the year 2000 as a
 * leap year. But unix doesn't as well.. same problem
 * as with MD2.. sigh.. investigate ( it works now though )
 */
long UtPack( const char *x)
{
	long ut;

	ut = utpack( x );

	if( ut >= 951782400 )
	{
		ut += 3600 * 24;
	}

	return( ut );
}


/*
 * Converts a datestamp to a string
 */
char *datestamp2string( struct DateStamp *ds )
{
#ifndef MBX
	struct DateTime dt;
	char buff1[ 16 ], buff2[ 16 ];
	static char dts[ 32 ];
	char *p;

	memset( &dt, 0, sizeof( dt ) );
	dt.dat_Stamp = *ds;
	dt.dat_StrDate = buff1;
	dt.dat_StrTime = buff2;
	DateToStr( &dt );

	strcpy( dts, buff1 );
	p = strchr( dts, 0 ) - 1;
	while( isspace( *p ) )
		p--;
	p[ 1 ] = ' ';
	strcpy( p + 2, buff2 );
	p = strchr( p, 0 ) - 1;
	while( isspace( *p ) )
		p--;
	p[ 1 ] = 0;

	return( dts );
#else
	return "NYI";
#endif
}


/*
 * Converts a date to a string
 */
char *date2string( time_t t )
{
#ifndef MBX
	return( datestamp2string( __timecvt( t ) ) );
#else
	return( "NYI" );
#endif
}


/* 1978-01-01 (DateStamp day 0) was a Sunday. */
static int v_isleap( int y )
{
	if( ( y % 4 ) != 0 )
		return( 0 );
	if( ( y % 100 ) != 0 )
		return( 1 );
	if( ( y % 400 ) == 0 )
		return( 1 );
	return( 0 );
}

static int v_month_days( int y, int m )
{
	static const UBYTE dim[ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if( m < 1 || m > 12 )
		return( 0 );
	if( m == 2 && v_isleap( y ) )
		return( 29 );
	return( (int)dim[ m - 1 ] );
}

/*
 * RFC 1123 GMT from a Voyager/SAS/C time_t via DateStamp fields only.
 * Do not use gmtime, utunpk, or Amiga2Date (ClockData came back as
 * "Sat, 00     11322 00:2026:00 GMT" and still 400'd amiga.com).
 */
int format_rfc1123_gmt( time_t t, char *out, int outlen )
{
#ifndef MBX
	struct DateStamp *ds;
	LONG totmin;
	ULONG days;
	int year, mon, mday, hour, min, sec, wday, ydays, mdays;
	char tmp[ 40 ];
	extern int locale_timezone_offset;

	if( !out || outlen < 30 || !t )
		return( FALSE );

	ds = __timecvt( t );
	if( !ds || ds->ds_Days < 0 || ds->ds_Minute < 0 || ds->ds_Minute > 24 * 60 - 1 )
		return( FALSE );

	totmin = (LONG)ds->ds_Days * 1440L + (LONG)ds->ds_Minute;
	totmin -= locale_timezone_offset / 60;
	if( totmin < 0 )
		return( FALSE );

	days = (ULONG)( totmin / 1440L );
	min = (int)( totmin % 1440L );
	hour = min / 60;
	min = min % 60;
	sec = (int)( ds->ds_Tick / TICKS_PER_SECOND );
	if( sec < 0 )
		sec = 0;
	if( sec > 59 )
		sec = 59;

	year = 1978;
	for( ;; )
	{
		ydays = v_isleap( year ) ? 366 : 365;
		if( days < (ULONG)ydays )
			break;
		days -= (ULONG)ydays;
		year++;
		if( year > 2037 )
			return( FALSE );
	}

	mon = 1;
	for( ;; )
	{
		mdays = v_month_days( year, mon );
		if( mdays <= 0 || days < (ULONG)mdays )
			break;
		days -= (ULONG)mdays;
		mon++;
		if( mon > 12 )
			return( FALSE );
	}
	mday = (int)days + 1;

	if( year < 1996 || year > 2037
		|| mon < 1 || mon > 12
		|| mday < 1 || mday > 31
		|| hour < 0 || hour > 23
		|| min < 0 || min > 59 )
		return( FALSE );

	/* DateStamp day 0 = Sunday 1 Jan 1978 */
	wday = (int)( ( totmin / 1440L ) % 7L );
	if( wday < 0 )
		wday += 7;

	sprintf( tmp, "%3.3s, %02d %3.3s %04d %02d:%02d:%02d GMT",
		&"SunMonTueWedThuFriSat"[ wday * 3 ],
		mday,
		&"JanFebMarAprMayJunJulAugSepOctNovDec"[ ( mon - 1 ) * 3 ],
		year,
		hour, min, sec
	);
	/* Must be "20xx" or "19xx" in the year slot, never 11322. */
	if( tmp[ 12 ] != '1' && tmp[ 12 ] != '2' )
		return( FALSE );
	if( tmp[ 5 ] == '0' && tmp[ 6 ] == '0' )
		return( FALSE );
	if( (int)strlen( tmp ) >= outlen )
		return( FALSE );
	strcpy( out, tmp );
	return( TRUE );
#else
	(void)t;
	(void)out;
	(void)outlen;
	return( FALSE );
#endif
}


/*
 * Converts an RFC date into a time_t
 */
time_t convertrfcdate( char *uudate )
{
	char *p;
	char x[6];
	long y;
	int tzoffs = 0, dst = 0;
	struct tzone *tzone = tzones;

	utunpk( timev(), x );

	p = strchr( uudate, ',' );
	if( !p )
		p = uudate;
	else
		p++;

	while( *p && !isdigit( *p ) )
		p++;

/* *p -> date time */
	p = stpblk( p );

/* Tag */
	x[ 2 ] = atoi( p );
	while( isdigit( *p ) || *p==' ' || *p=='\t' || *p == '-' )
		p++;
/* Monat */
	if( !strnicmp( p, "Jan",3 ) ) x[ 1 ] = 1;
	else if( !strnicmp( p, "Feb", 3 ) ) x[ 1 ] = 2;
	else if( !strnicmp( p, "Mar", 3 ) ) x[ 1 ] = 3;
	else if( !strnicmp( p, "Apr", 3 ) ) x[ 1 ] = 4;
	else if( !strnicmp( p, "May", 3 ) ) x[ 1 ] = 5;
	else if( !strnicmp( p, "Jun", 3 ) ) x[ 1 ] = 6;
	else if( !strnicmp( p, "Jul", 3 ) ) x[ 1 ] = 7;
	else if( !strnicmp( p, "Aug", 3 ) ) x[ 1 ] = 8;
	else if( !strnicmp( p, "Sep", 3 ) ) x[ 1 ] = 9;
	else if( !strnicmp( p, "Oct", 3 ) ) x[ 1 ] = 10;
	else if( !strnicmp( p, "Nov", 3 ) ) x[ 1 ] = 11;
	else if( !strnicmp( p, "Dec", 3 ) ) x[ 1 ] = 12;
	else
		goto doex;
	p = stpblk( &p[ 4 ] );
	y = atoi( p );
	if( y>1900 )
		y -= 1970;
	else
	{
		if( y < 80 )
			y = ( 2000 + y - 1970 );
		else
			y -= 70;
	}
	x[ 0 ] = y;

/* Hour */
	while( isdigit( *p) )
		p++;
	p = stpblk( p );
	x[3] = atoi( p );
	p = strchr( p, ':' );
	if( !p )
		goto doex;
	x[ 4 ] = atoi( ++p );
	while( *p && *p!=':' && !isspace( *p ) )
		p++;
	if( *p == ':' )
	{
		x[ 5 ] = atoi( ++p );
		while( *p && !isspace( *p ) )
			p++;
	}
	else
		x[ 5 ] = 0;

/* Time Zone */
	p = stpblk( p );

	if( *p )
	{
		extern struct Locale *locale;
		extern int locale_timezone_offset;

		if( *p == '+' || *p == '-' )
		{
			char bf1[ 4 ], bf2[ 4 ];

			bf1[ 0 ] = p[ 1 ];
			bf1[ 1 ] = p[ 2 ];
			bf1[ 2 ] = 0;
			bf2[ 0 ] = p[ 3 ];
			bf2[ 1 ] = p[ 4 ];
			bf2[ 2 ] = 0;
			tzoffs = ( atoi( bf1 ) * 60 ) + atoi( bf2 );
			if( *p == '-' )
				tzoffs = -tzoffs;
		}
		else
		{
			while( tzone->name )
			{
				if( !strnicmp( tzone->name, p, strlen( tzone->name ) ) )
					break;
				tzone++;
			}
			tzoffs = tzone->offset;
			dst = tzone->dst;
		}

		/*
		 * Use locale.library to have a value for our current
		 * timezone regardless of the server.
		 */

		if( locale )
			tzoffs -= locale_timezone_offset / 60;
	}

doex:
	return( UtPack( x ) - ( tzoffs * 60 ) );
}
 
