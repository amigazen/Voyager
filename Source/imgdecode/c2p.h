/*
 * $Id: c2p.h,v 1.2 2001/10/28 10:02:45 zapek Exp $
 */

void ASM writechunky(
	__reg( a0, UBYTE *chunky ),
	__reg( a1, UBYTE **planes ),
	__reg( d0, ULONG pixels ),
	__reg( d1, int numplanes )
);

int ASM makemaskline(
	__reg( a0, UBYTE *penarray ),
	__reg( a1, APTR planeptr ),
	__reg( d0, ULONG xsize ),
	__reg( d1, UBYTE maskpen )
);

void ASM penarrayconvert(
	__reg( a0, char * source ),
	__reg( a1, char * dest ),
	__reg( a2, char * penarray ),
	__reg( d0, ULONG size ),
	__reg( d1, int null )
);

void ASM striprgba(
	__reg( a0, char * source ),
	__reg( a1, char * dest ),
	__reg( d0, UWORD num )
);

void ASM penarray2rgb(
	__reg( a0, char * source ),
	__reg( a1, char * dest ),
	__reg( a2, char * colmap ),
	__reg( d0, UWORD num )
);

