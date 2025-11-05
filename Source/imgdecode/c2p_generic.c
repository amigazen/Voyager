/*
 * Portable specialized C2P routines
 * ---------------------------------
 * - see c2p.s if you want fast stuffs on a 68k :)
 *
 * © 2000-2003 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: c2p_generic.c,v 1.20 2003/07/06 16:51:35 olli Exp $
 *
*/

#if defined( __MORPHOS__ ) || defined( MBX )

#ifdef MBX
#include "mbx.h"
#endif /* MBX */

#include "globals.h"

#include "c2p.h"

void writechunky( UBYTE *chunky, UBYTE **planes, ULONG pixels, int numplanes )
{
	/*
	 * This function does nothing. Since it's only needed
	 * for the MorphOS version and CGFX5 has a native AGACGX driver,
	 * not to forget that the blitter doesn't work correctly
	 * with a PPC around... Saves us the hassle :)
	 */
}


int makemaskline( UBYTE *penarray, APTR planeptr, ULONG xsize, UBYTE maskpen )
{
	int mask_used = 0, non_trans = 0;
	UWORD *pptr = planeptr;
	UWORD shiftval = 15;
	UWORD mask = 0xffff;

	shiftval = 15;
	mask = 0xffff;

	while( xsize )
	{
		if( *penarray++ == maskpen )
		{
			/* clear */
			mask &= ~( 1 << shiftval );
			mask_used = 2;
		}
		else
		{
			/* non transparent value found */
			non_trans = 1;
		}
		
		if( !( shiftval-- ) )
		{
			*pptr++ = mask;

			if( xsize )
			{
				shiftval = 15;
				mask = 0xffff;
			}
		}
		xsize--;
	}
	
	if( shiftval < 15 )
	{
		*pptr = mask;
	}

	return( non_trans | mask_used );
	//return( ( non_trans ? 1 : 0 ) | ( mask_used ? 2 : 0 ) );
}


void penarrayconvert( char * source, char * dest, char * penarray, ULONG size, int null )
{
	while( size )
	{
		*dest++ = penarray[ *source++ ];
		*dest++ = penarray[ *source++ ];
		*dest++ = penarray[ *source++ ];
		size--;	
	}
}


/*
 * Remove the alpha channel
 */
void striprgba( char * source, char * dest, UWORD num )
{
	while( num-- )
	{
		*(( WORD * )dest) = *(( WORD * )source);
		dest += 2;
		source += 2;
		*dest = *source;
		dest++;
		source += 2;
	}
}


/*
 * Source is an array of pens.
 * Multiply its value by 3 and use it as an index to add the
 * 3 bytes from the colmap into dest
 */
void penarray2rgb( char * source, char * dest, char * colmap, UWORD num )
{
	char *srcptr;

	while( num )
	{
		srcptr = colmap + ( 3 * *source++ );
		*((UWORD*)dest)++ = *((UWORD*)srcptr)++;
		//*dest++ = *srcptr++;
		//*dest++ = *srcptr++;
		*dest++ = *srcptr;
		num--;
	}
}


/*
 * Source is an array of pens.
 * Multiply its value by 3 and use it as an index to add the
 * 3 bytes from the colmap into dest
 * Also merge alpha channel
 */
int penarray2argb_trans( char * source, char * dest, char * colmap, UWORD num, char transcolor )
{
	char *srcptr;
	int mask_used = 0, non_trans = 0;

	while( num )
	{
		char src = *source++;

		srcptr = colmap + ( 3 * src );

		if( src == transcolor )
		{
			*dest++ = 0x00;
			mask_used = 2;
		}
		else
		{
			*dest++ = 0xff;
			non_trans = 1;
		}

		*((UWORD*)dest)++ = *((UWORD*)srcptr)++;
		//*dest++ = *srcptr++;
		//*dest++ = *srcptr++;
		*dest++ = *srcptr;
		num--;
	}

	//return( ( non_trans ? 1 : 0 ) | ( mask_used ? 2 : 0 ) );
	return( non_trans | mask_used );
}

#endif /* __MORPHOS__ */
