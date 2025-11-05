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
** $Id: js_date.c,v 1.22 2003/07/06 16:51:33 olli Exp $
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <dos/datetime.h>
#include <proto/timer.h>
#endif
#include <math.h>

/* private */
#include "classes.h"
#include "js.h"
#include "time_func.h"
#include "mui_func.h"
#include "dos_func.h"


#define UnixTimeOffset (252482400-(6*3600))

extern int locale_timezone_offset;

struct Data {
	struct timeval tv;
};

BEGINPTABLE

DPROP( getDate,			funcptr )
DPROP( getDay,			funcptr )
DPROP( getHours,		funcptr )
DPROP( getMinutes,		funcptr )
DPROP( getMonth,		funcptr )
DPROP( getSeconds,		funcptr )
DPROP( getTime,			funcptr )
DPROP( getTimezoneOffset, funcptr )
DPROP( getMilliseconds, funcptr )
DPROP( getYear,			funcptr )
DPROP( getFullYear,		funcptr )
DPROP( setDate,			funcptr )
DPROP( setHours,		funcptr )
DPROP( setMinutes,		funcptr )
DPROP( setMonth,		funcptr )
DPROP( setSeconds,		funcptr )
DPROP( setTime,			funcptr )
DPROP( setYear,			funcptr )
DPROP( setFullYear,		funcptr )
DPROP( setMilliseconds, funcptr )

// The same, with UTC correction
DPROP( getUTCDate,		funcptr )
DPROP( getUTCDay,		funcptr )
DPROP( getUTCHours,		funcptr )
DPROP( getUTCMinutes,	funcptr )
DPROP( getUTCMonth,		funcptr )
DPROP( getUTCSeconds,	funcptr )
DPROP( getUTCTime,		funcptr )
DPROP( getUTCTimezoneOffset, funcptr )
DPROP( getUTCMilliseconds, funcptr )
DPROP( getUTCYear,		funcptr )
DPROP( getUTCFullYear,	funcptr )
DPROP( setUTCDate,		funcptr )
DPROP( setUTCHours,		funcptr )
DPROP( setUTCMinutes,	funcptr )
DPROP( setUTCMonth,		funcptr )
DPROP( setUTCSeconds,	funcptr )
DPROP( setUTCTime,		funcptr )
DPROP( setUTCYear,		funcptr )
DPROP( setUTCFullYear,	funcptr )
DPROP( setUTCMilliseconds,funcptr )

DPROP( parse,			funcptr )
DPROP( toGMTString,		funcptr )
DPROP( toUTCString,		funcptr )
DPROP( toLocaleString,	funcptr )
DPROP( UTC,				funcptr )
ENDPTABLE

static void doset( APTR obj, struct Data *data, struct TagItem *tags )
{
	struct TagItem *tag;

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_JS_Date_Seconds:
			data->tv.tv_secs = tag->ti_Data;
			break;

		case MA_JS_Date_Micros:
			data->tv.tv_micro = tag->ti_Data;
			break;

		case MA_JS_Datestring:
			{
				data->tv.tv_secs = convertrfcdate( (char*)tag->ti_Data );
				data->tv.tv_micro = 0;
			}
			break;
	}
}

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Date",
		TAG_MORE, msg->ops_AttrList
	);

	if( obj )
	{
		struct Data *data = INST_DATA( cl, obj );
#if USE_GETSYSTIME
		GetSysTime( &data->tv );
#endif
		data->tv.tv_secs += UnixTimeOffset;

		doset( obj, data, msg->ops_AttrList );
	}

	return( (ULONG)obj );
}

DECSET
{
	GETDATA;

	doset( obj, data, msg->ops_AttrList );

	return( DOSUPER );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );

	if( !pt )
		return( DOSUPER );

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}
	
	return( FALSE );
}

DECSMETHOD( JS_CallMethod )
{
	GETDATA;
	static double rval;
	static char strval[ 64 ];
	struct tm *tm;
	int tzcorrect = 0;

	// Nifty hack -- if the getUTCxx() functions are called,
	// present our "normal" functions with a timezone corrected value
	// tzcorrect is used for the setXX() functions; it's 0 for local time
	// versions and has the correct offset for the UTC versions
	if( msg->pid > JSPID_DATE_UTC_MARK_FIRST && msg->pid < JSPID_DATE_UTC_MARK_LAST )
	{
		time_t t;

		tzcorrect = locale_timezone_offset;
		t = (time_t)(data->tv.tv_secs - tzcorrect);
		tm = localtime( &t );
	}
	else
	{
		tm = localtime( (time_t*)&data->tv.tv_secs );
	}

	switch( msg->pid )
	{
		case JSPID_toGMTString:
		case JSPID_toUTCString:
			{
				int ts = data->tv.tv_secs;

				ts -= locale_timezone_offset;

				stccpy( strval, ctime( (time_t*)&ts ), 26 );

				*msg->typeptr = expt_string;
				*msg->dataptr = strval;
				*msg->datasize = strlen( strval );
			}
			return( TRUE );

		case JSPID_toLocaleString:
#ifdef MBX
			{
				int ts = data->tv.tv_secs;
				stccpy( strval, ctime( (time_t*)&ts ), 26 );
			}

#else
			{
				struct DateStamp *ds = __timecvt( data->tv.tv_secs );
				struct DateTime dt;
				char ds_date[ 32 ], ds_time[ 32 ];

				memset( &dt, 0, sizeof( dt ) );
				dt.dat_Stamp = *ds;
				dt.dat_Format = FORMAT_DOS;
				dt.dat_StrDate = ds_date;
				dt.dat_StrTime = ds_time;
				DateToStr( &dt );
				sprintf( strval, "%s %s", ds_time, ds_date );
			}
#endif
			*msg->typeptr = expt_string;
			*msg->dataptr = strval;
			*msg->datasize = strlen( strval );
			return( TRUE );

		// Getter functions
		// Since we did timezone correction above,
		// we don't need to bother doing it here

		case JSPID_getDate:
		case JSPID_getUTCDate:
			rval = tm->tm_mday;
			break;

		case JSPID_getDay:
		case JSPID_getUTCDay:
			rval = tm->tm_wday;
			break;

		case JSPID_getHours:
		case JSPID_getUTCHours:
			rval = tm->tm_hour;
			break;

		case JSPID_getMinutes:
		case JSPID_getUTCMinutes:
			rval = tm->tm_min;
			break;

		case JSPID_getMonth:
		case JSPID_getUTCMonth:
			rval = tm->tm_mon;
			break;

		case JSPID_getSeconds:
		case JSPID_getUTCSeconds:
			rval = tm->tm_sec;
			break;

		case JSPID_getMilliseconds:
		case JSPID_getUTCMilliseconds:
			rval = data->tv.tv_micro / 1000;
			break;

		case JSPID_getYear:
		case JSPID_getUTCYear:
			if( tm->tm_year > 99 )
				rval = tm->tm_year + 1900;
			else
				rval = tm->tm_year;
			break;

		case JSPID_getFullYear:
		case JSPID_getUTCFullYear:
			rval = tm->tm_year + 1900;
			break;

		case JSPID_getTime:
			rval = (double)data->tv.tv_secs * 1000.0 + (double)data->tv.tv_micro / 1000.0;
			break;

		case JSPID_getUTCTime:
			rval = (double)(data->tv.tv_secs - tzcorrect ) * 1000.0 + (double)data->tv.tv_micro / 1000.0;
			break;

		// Setter functions

		case JSPID_setDate:
		case JSPID_setUTCDate:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );
			tm->tm_mday = (int)rval;
			data->tv.tv_secs = mktime( tm ) + tzcorrect;
			break;

		case JSPID_setUTCHours:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );
			tm->tm_hour = (int)rval;
			data->tv.tv_secs = mktime( tm ) + tzcorrect;
			break;

		case JSPID_setMinutes:
		case JSPID_setUTCMinutes:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );
			tm->tm_min = (int)rval;
			data->tv.tv_secs = mktime( tm ) + tzcorrect;
			break;

		case JSPID_setMonth:
		case JSPID_setUTCMonth:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );
			tm->tm_mon = (int)rval;
			data->tv.tv_secs = mktime( tm ) + tzcorrect;
			break;

		case JSPID_setSeconds:
		case JSPID_setUTCSeconds:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );
			tm->tm_sec = (int)rval;
			data->tv.tv_secs = mktime( tm ) + tzcorrect;
			break;

		case JSPID_setYear:
		case JSPID_setUTCYear:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );

			if( rval > 99 )
				tm->tm_year = rval - 1900;
			else
				tm->tm_year = rval;

			data->tv.tv_secs = mktime( tm ) + tzcorrect;
			break;

		case JSPID_setMilliseconds:
		case JSPID_setUTCMilliseconds:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );

			if( rval < 0 )
				rval = 0;
			else if( rval >= 1000 )
				rval = 999;

			data->tv.tv_micro = rval * 1000;
			break;

		case JSPID_setTime:
		case JSPID_setUTCTime:
			if( msg->argcnt-- < 1 )
				return( 0 );
			rval = exprs_pop_as_real( msg->es );
			data->tv.tv_secs = rval / 1000 + tzcorrect;
			data->tv.tv_micro = ( (int)rval % 1000 ) * 1000;
			break;

		case JSPID_getTimezoneOffset:
			rval = -( locale_timezone_offset / 60.0 );
			break;

		case JSPID_UTC:
			if( msg->argcnt < 3 )
				return( 0 );

			memset( tm, 0, sizeof( *tm ) );

			rval = exprs_pop_as_real( msg->es );
			tm->tm_year = ( rval <= 99 ) ? rval : rval - 1900;

			rval = exprs_pop_as_real( msg->es );
			tm->tm_mon = rval;

			rval = exprs_pop_as_real( msg->es );
			tm->tm_mday = rval;
		
			msg->argcnt -= 3;

			if( msg->argcnt-- > 0 )
			{
				rval = exprs_pop_as_real( msg->es );
				tm->tm_hour = rval;
			}
			if( msg->argcnt-- > 0 )
			{
				rval = exprs_pop_as_real( msg->es );
				tm->tm_min = rval;
			}
			if( msg->argcnt-- > 0 )
			{
				rval = exprs_pop_as_real( msg->es );
				tm->tm_sec = rval;
			}

			rval = mktime( tm ) * 1000.0;
			break;

		case JSPID_parse:
			{
				char buffer[ 128 ];
				struct timeval tvold = data->tv, tvnew;

				if( msg->argcnt-- < 1 )
					return( 0 );
				exprs_pop_as_string( msg->es, buffer, sizeof( buffer ) );
				set( obj, MA_JS_Datestring, buffer );
				tvnew = data->tv;
				data->tv = tvold;

				rval = tvnew.tv_secs * 1000.0 + tvnew.tv_micro / 1000.0;
			}
			break;
	
		default:
			return( DOSUPER );
	}

	rval = floor( rval );

	*msg->typeptr = expt_real;
	*msg->dataptr = &rval;

	return( TRUE );
}

DECSMETHOD( JS_ToString )
{
	GETDATA;
	char to[ 64 ];
	char tz[ 16 ];

	stccpy( to, ctime( (time_t*)&data->tv.tv_secs ), 26 );

	sprintf( tz, " GMT%+03d%02d ",
		locale_timezone_offset / 3600,
		( locale_timezone_offset / 60 ) % 60
	);

	strins( &to[ 20 ], tz );

	if( msg->tosize && *msg->tosize )
	{
		stccpy( msg->tobuffer, to, *msg->tosize );
	}
	else
		strcpy( msg->tobuffer, to );

	if( msg->tosize )
		*msg->tosize = strlen( to );

	return( TRUE );
}

DECSMETHOD( JS_ToReal )
{
	GETDATA;

	*msg->realptr = data->tv.tv_secs * 1000 +
		data->tv.tv_micro / 1000;

	return( TRUE );
}

BEGINMTABLE
DEFNEW
DEFSET
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ToString )
DEFSMETHOD( JS_ToReal )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_date( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_DateClass";
#endif

	return( TRUE );
}

void delete_js_date( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_date( void )
{
	return( mcc->mcc_Class );
}
