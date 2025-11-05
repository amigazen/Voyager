#define __USE_SYSBASE
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <intuition/intuitionbase.h>
#include <string.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <proto/timer.h>
#include <stdlib.h>

#include "random.h"
#include "globals.h"

#define GET_32BIT(cp) (((unsigned long)(unsigned char)(cp)[0] << 24) | \
  		       ((unsigned long)(unsigned char)(cp)[1] << 16) | \
		       ((unsigned long)(unsigned char)(cp)[2] << 8) | \
		       ((unsigned long)(unsigned char)(cp)[3]))

#define PUT_32BIT(cp, value) do { \
  (cp)[0] = (value) >> 24; \
  (cp)[1] = (value) >> 16; \
  (cp)[2] = (value) >> 8; \
  (cp)[3] = (value); } while (0)

typedef signed long word32;

static struct SignalSemaphore randsem;
static RandomState state;
static int rand_isinit;

void random_firstinit( void )
{
	InitSemaphore( &randsem );
}

static void random_xor_noise( unsigned int i, word32 value )
{
  value ^= GET_32BIT(state.state + 4 * i);
  PUT_32BIT(state.state + 4 * i, value);
}

static void random_stir( void )
{
  word32 iv[4];
  unsigned int i;

  /* Start IV from last block of random pool. */
  iv[0] = GET_32BIT(state.state);
  iv[1] = GET_32BIT(state.state + 4);
  iv[2] = GET_32BIT(state.state + 8);
  iv[3] = GET_32BIT(state.state + 12);

  /* First CFB pass. */
  for (i = 0; i < RANDOM_STATE_BYTES; i += 16)
	{
	  Transform( (APTR)iv, (APTR)state.stir_key);
	  iv[0] ^= GET_32BIT(state.state + i);
	  PUT_32BIT(state.state + i, iv[0]);
	  iv[1] ^= GET_32BIT(state.state + i + 4);
	  PUT_32BIT(state.state + i + 4, iv[1]);
	  iv[2] ^= GET_32BIT(state.state + i + 8);
	  PUT_32BIT(state.state + i + 8, iv[2]);
	  iv[3] ^= GET_32BIT(state.state + i + 12);
	  PUT_32BIT(state.state + i + 12, iv[3]);
	}

  /* Get new key. */
  memcpy(state.stir_key, state.state, sizeof(state.stir_key));

  /* Second CFB pass. */
  for (i = 0; i < RANDOM_STATE_BYTES; i += 16)
	{
	  Transform( (APTR)iv, (APTR)state.stir_key );
	  iv[0] ^= GET_32BIT(state.state + i);
	  PUT_32BIT(state.state + i, iv[0]);
	  iv[1] ^= GET_32BIT(state.state + i + 4);
	  PUT_32BIT(state.state + i + 4, iv[1]);
	  iv[2] ^= GET_32BIT(state.state + i + 8);
	  PUT_32BIT(state.state + i + 8, iv[2]);
	  iv[3] ^= GET_32BIT(state.state + i + 12);
	  PUT_32BIT(state.state + i + 12, iv[3]);
	}
  
  state.add_position = 0;

  /* Some data in the beginning is not returned to aboid giving an observer
	 complete knowledge of the contents of our random pool. */
  state.next_available_byte = sizeof(state.stir_key);
}

static void random_add_noise( const void *buf, unsigned int bytes )
{
	unsigned int pos = state.add_position;
	const char *input = buf;
	while (bytes > 0)
	{
		if (pos >= RANDOM_STATE_BYTES)
		{
			pos = 0;
			random_stir();
		}
		state.state[pos] ^= *input;
		input++;
		bytes--;
		pos++;
	}
	state.add_position = pos;
}

static void random_add_noise_from_file( char *fn )
{
	char buffer[ 1024 ];
	BPTR f;
	int rc;

	if( f = Open( fn, MODE_OLDFILE ) )
	{
		random_add_noise( &f, sizeof( f ) );
		while( ( rc = Read( f, buffer, sizeof( buffer ) ) ) > 0 )
			random_add_noise( buffer, rc );
		Close( f );
	}
}

static void random_add_noise_from_prefs_file( char *pf )
{
	char buffer[ 128 ];

	sprintf( buffer, "ENV:Sys/%s.prefs", pf );
	random_add_noise_from_file( buffer );
}

static void random_acquire_light_environmental_noise( void )
{
	struct timeval tv;
	struct Screen *iscr;
	struct DateStamp ds;

	DateStamp( &ds );

	random_stir();

	GetSysTime( &tv );
	
	random_xor_noise(
		   (unsigned int)(state.state[0] + 256*state.state[1]) % 
		     (RANDOM_STATE_BYTES / 4),
		   (word32) rand() );

	random_xor_noise( 0, tv.tv_secs );
	random_xor_noise( 1, tv.tv_micro );

	random_xor_noise( 2, AvailMem( 0 ) );
	random_xor_noise( 3, AvailMem( MEMF_CHIP ) );
	random_xor_noise( 4, AvailMem( MEMF_FAST ) );
	random_xor_noise( 5, AvailMem( MEMF_LARGEST ) );

	random_xor_noise( 6, SysBase->IdleCount );
	random_xor_noise( 7, SysBase->DispCount );
	random_xor_noise( 8, SysBase->Quantum );
	random_xor_noise( 9, SysBase->Elapsed );

	random_xor_noise( 10, (word32)SysBase->ThisTask->tc_SPReg );
	random_xor_noise( 11, (word32)SysBase->ThisTask->tc_SPLower );
	random_xor_noise( 12, (word32)SysBase->ThisTask->tc_SPUpper );

	random_stir();

	// Block "noise"

	random_add_noise( &ds, sizeof( ds ) );

	Forbid();
	iscr = IntuitionBase->FirstScreen;
	random_add_noise( iscr, sizeof( *iscr ) );
	Permit();

	random_stir();
}

static void random_acquire_environmental_noise( void )
{
	struct timeval tv1, tv2;
	struct RastPort *rp;
	int x, mx, y, my, cnt;
	char buffer[ RANDOM_STATE_BYTES ];

	GetSysTime( &tv1 );

	random_add_noise_from_prefs_file( "font" );
	random_add_noise_from_prefs_file( "icontrol" );
	random_add_noise_from_prefs_file( "input" );
	random_add_noise_from_prefs_file( "locale" );
	random_add_noise_from_prefs_file( "overscan" );
	random_add_noise_from_prefs_file( "palette" );
	random_add_noise_from_prefs_file( "pointer" );
	random_add_noise_from_prefs_file( "printer" );
	random_add_noise_from_prefs_file( "printergfx" );
	random_add_noise_from_prefs_file( "screenmode" );
	random_add_noise_from_prefs_file( "serial" );
	random_add_noise_from_prefs_file( "sound" );
	random_add_noise_from_prefs_file( "system" );
	random_add_noise_from_prefs_file( "wbconfig" );
	random_add_noise_from_prefs_file( "wbpattern" );
	random_add_noise_from_file( "ENV:Sys/palette.ilbm" );

	Forbid();
	rp = &IntuitionBase->FirstScreen->RastPort;
	mx = IntuitionBase->FirstScreen->Width;
	my = IntuitionBase->FirstScreen->Height;
	Permit();

	random_add_noise( rp, sizeof( *rp ) );

	for( x = 27, y = 17, cnt = 0; cnt < sizeof( buffer ); cnt++ )
	{
		if( x > mx )
		{
			x = 0;
			y++;
		}
		if( y > my )
			break;
		buffer[ cnt ] = ReadPixel( rp, x, y );

		x += 3;
	}
	random_add_noise( buffer, sizeof( buffer ) );

	// Get time
	GetSysTime( &tv2 );
	SubTime( &tv2, &tv1 );
	random_add_noise( &tv2, sizeof( tv2 ) );

	random_acquire_light_environmental_noise();
}

static unsigned int random_get_byte( void )
{
	if (state.next_available_byte >= RANDOM_STATE_BYTES)
	{
		random_acquire_light_environmental_noise();
	}
	return state.state[state.next_available_byte++];
}

static void random_save( void )
{
	char buf[RANDOM_STATE_BYTES / 2];  /* Save only half of its bits. */
	int i;
	BPTR f;

	random_acquire_light_environmental_noise();

	for (i = 0; i < sizeof(buf); i++)
		buf[i] = random_get_byte();

	random_acquire_light_environmental_noise();

	f = Open( "S:VaporToolkit.Randseed", MODE_NEWFILE );
	if( f )
	{
		Write( f, buf, sizeof( buf ) );
		Close( f );
	}
}

static void random_initialize( void )
{
	BPTR f;
	int rc;
	char buffer[ 1024 ];

//	state.add_position = 0;
	state.next_available_byte = sizeof( state.stir_key );

	f = Open( "S:VaporToolkit.Randseed", MODE_OLDFILE );
	if( f )
	{
		rc = Read( f, buffer, sizeof( buffer ) );
		Close( f );
		if( rc )
			random_add_noise( buffer, rc );
	}
	else
	{
		random_acquire_environmental_noise();
		random_save();
	}

	random_acquire_light_environmental_noise();

	rand_isinit = TRUE;
}

// Interface functions

void ASM SAVEDS VAT_RandomStir( void )
{
	DB( ( "called\n" ) );
	
	ObtainSemaphore( &randsem );
	if( !rand_isinit )
		random_initialize();
	random_stir();
	ReleaseSemaphore( &randsem );
}

int ASM SAVEDS VAT_RandomByte( void )
{
	int x;

	DB( ( "called\n" ) );

	ObtainSemaphore( &randsem );
	if( !rand_isinit )
		random_initialize();
	x = random_get_byte();
	ReleaseSemaphore( &randsem );

	return( x );
}

int ASM SAVEDS VAT_Random( void )
{
	int x;

	DB( ( "called\n" ) );

	ObtainSemaphore( &randsem );
	if( !rand_isinit )
		random_initialize();

	x = random_get_byte() << 24;
	x |= random_get_byte() << 16;
	x |= random_get_byte() << 8;
	x |= random_get_byte();

	ReleaseSemaphore( &randsem );

	return( x );
}
