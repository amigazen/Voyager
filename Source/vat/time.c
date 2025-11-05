#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <time.h>

#include "globals.h"

#define UnixTimeOffset (252482400-(6*3600))
time_t ASM SAVEDS VAT_Time( __reg( a0, time_t *tp ) )
{
	struct timeval tv;

	DB( ( "called\n" ) );

	GetSysTime( &tv );
	tv.tv_secs += UnixTimeOffset;
	if( tp )
		*tp = tv.tv_secs;
	return( (time_t)tv.tv_secs );
}

time_t ASM SAVEDS VAT_Timev( void )
{
	struct timeval tv;

	DB( ( "called\n" ) );

	GetSysTime( &tv );
	return( (time_t)tv.tv_secs + UnixTimeOffset );
}
