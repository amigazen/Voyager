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
 * $Id: readkey.c,v 1.20 2003/07/06 16:51:34 olli Exp $
 */
#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#ifdef __MORPHOS__
#define NO_VAT_SHORTCUTS
#include <libraries/vat.h>
#include <clib/vat_protos.h>
#else
#include <proto/dos.h>
#include <time.h>
#include <stdlib.h>
#include <proto/vat.h>
#endif /* !__MORPHOS__ */
#endif

/* private */

#if USE_KEYFILES

#include "voyager3_pubkey.h"
#include "keyfile.h"
#include "idea68k.h"
#include "dos_func.h"

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

struct keyfile kf;

//#define RK_DEBUG

#ifndef __MORPHOS__
unsigned char __clz_tab[] =
{
  0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};
#endif /* !__MORPHOS__ */

static size_t mympn_get_str( unsigned char *str, int base, mp_ptr mptr, mp_size_t msize)
{
  mp_limb_t big_base;
#if UDIV_NEEDS_NORMALIZATION || UDIV_TIME > 2 * UMUL_TIME
//  int normalization_steps;
#endif
#if UDIV_TIME > 2 * UMUL_TIME
  mp_limb_t big_base_inverted;
#endif
//  unsigned int dig_per_u;
//  mp_size_t out_len;
  register unsigned char *s;

  big_base = 8;

  s = str;

  /* Special case zero, as the code below doesn't handle it.  */
  if (msize == 0)
	{
	  s[0] = 0;
	  return 1;
	}

	  /* The base is a power of 2.  Make conversion from most
	 significant side.  */
	{

	  mp_limb_t n1, n0;
	  register int bits_per_digit = big_base;
	  register int x;
	  register int bit_pos;
	  register int i;

	  n1 = mptr[msize - 1];
	  count_leading_zeros (x, n1);

	/* BIT_POS should be R when input ends in least sign. nibble,
	   R + bits_per_digit * n when input ends in n:th least significant
	   nibble. */

	  {
	int bits;

	bits = BITS_PER_MP_LIMB * msize - x;
	x = bits % bits_per_digit;
	if (x != 0)
	  bits += bits_per_digit - x;
	bit_pos = bits - (msize - 1) * BITS_PER_MP_LIMB;
	  }

	  /* Fast loop for bit output.  */
	  i = msize - 1;
	  for (;;)
	{
	  bit_pos -= bits_per_digit;
	  while (bit_pos >= 0)
	    {
	      *s++ = (n1 >> bit_pos) & ((1 << bits_per_digit) - 1);
	      bit_pos -= bits_per_digit;
	    }
	  i--;
	  if (i < 0)
	    break;
	  n0 = (n1 << -bit_pos) & ((1 << bits_per_digit) - 1);
	  n1 = mptr[i];
	  bit_pos += BITS_PER_MP_LIMB;
	  *s++ = n0 | (n1 >> bit_pos);
	}

	  *s = 0;

	  return (size_t)(s - str);
	}
}

int load_and_parse_key( char *filename )
{
	BPTR f;
	char buffer[ 512 ], *p;
	int mp_alloc, mp_size;
	int valkey = 0;
	int parts;
	MP_INT pubkey_e, pubkey_n;
	MP_INT indata, outdata;
	int c;

	memset( &kf, 0, sizeof( kf ) );

	f = Open( filename, MODE_OLDFILE );
	if( !f )
		return( -1 );

	pubkey_e._mp_alloc = PUBKEY_E_ALLOC;
	pubkey_e._mp_size = PUBKEY_E_SIZE;
	pubkey_e._mp_d = malloc( PUBKEY_E_ALLOC * 4 );
	memcpy( pubkey_e._mp_d, pubkey_E, PUBKEY_E_ALLOC * 4 );

	pubkey_n._mp_alloc = PUBKEY_N_ALLOC;
	pubkey_n._mp_size = PUBKEY_N_SIZE;
	pubkey_n._mp_d = malloc( PUBKEY_N_ALLOC * 4 );
	memcpy( pubkey_n._mp_d, pubkey_N, PUBKEY_N_ALLOC * 4 );

	if( Read( f, buffer, 12 ) == 12 )
	{
		if( !strncmp( buffer, "$Vapor2KEY$", 11 ) )
		{
			for( parts = 0; parts < 4; parts++ )
			{
				if( Read( f, &mp_alloc, 4 ) != 4 )
					goto giveup;
				if( Read( f, &mp_size, 4 ) != 4 )
					goto giveup;

#ifdef RK_DEBUG
				printf( "part %ld, alloc %ld, size %ld\n", parts, mp_alloc, mp_size );
#endif

				indata._mp_alloc = mp_alloc;
				indata._mp_size = mp_size;
				indata._mp_d = malloc( mp_alloc * 4 );
				Read( f, indata._mp_d, mp_alloc * 4 );

				memset( &outdata, 0, sizeof( outdata ) );

				VAT_MPZPow( &outdata, &indata, &pubkey_e, &pubkey_n);

				// Check whether VAT_MPZPow() is a NOP...
				if( !memcmp( &outdata, &indata, mp_alloc * 4 ) )
					goto giveup;

				c = mympn_get_str( buffer, 256, outdata._mp_d, outdata._mp_size );

				VAT_MPZFree( &outdata );

				if( c > 128 )
					goto giveup;

				p = (char*)&kf;
				p += parts * 128;

				memcpy( p + ( 128 - c ), buffer, c );

			}
		}

		VAT_CalcMD5( (char*)&kf, sizeof( kf ) - 16, buffer );
#ifdef RK_DEBUG
		{
			int x;

			printf( "keydata MD5: " );
			for( x = 0; x < 16; x++ )
				printf( "%02lx ", kf.md5[ x ] );
			printf( "\ncalcued MD5: " );
			for( x = 0; x < 16; x++ )
				printf( "%02lx ", buffer[ x ] );
			printf( "\n" );
		}
#endif

		valkey = !memcmp( buffer, kf.md5, 16 );
	}

giveup:
	Close( f );
	return( valkey );
}

//#define TEST
#ifdef TEST

void main( int argc, char **argv )
{
#if 1
	int rc;

	if( argc != 2 )
	{
		printf( "Usage: ReadKey <file>\n" );
		exit( 0 );
	}

	rc = load_and_parse_key( argv[ 1 ] );
	printf( "load_and_parse_key() returned %ld\n", rc );

	printf( "kf.owner = %s\n", kf.owner );
	printf( "kf.serialtext = %s\n", kf.serialtext );
	printf( "kf.comments = %s\n", kf.comments );
#endif
}

#endif

#endif /* USE_KEYFILES */
