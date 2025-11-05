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
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
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
static struct timerequest treq;

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

int init_timer( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	treq.tr_node.io_Message.mn_ReplyPort = CreateMsgPort();
	if( OpenDevice( "timer.device", UNIT_VBLANK, ( struct IORequest * )&treq, 0 ) )
	{
		D( db_init, bug( "timer.device failed to open\n" ) );
		return( FALSE );
	}
	D( db_init, bug( "timer.device opened\n" ) );
	TimerBase = ( struct Library * )treq.tr_node.io_Device;

	/*
	 * Set the start time for timed()
	 */
	ReadEClock( &start_timed );

	return( TRUE );
}


/*
 * Timer cleanup
 */
void cleanup_timer( void )
{
	if( TimerBase )
	{
		D( db_init, bug( "cleaning up..\n" ) );

		CloseDevice( ( struct IORequest * )&treq );
		DeleteMsgPort( treq.tr_node.io_Message.mn_ReplyPort );
	}
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

	r = ReadEClock( &ev );

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

	r = ReadEClock( &ev );

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
 
