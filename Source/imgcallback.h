/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_IMGCALLBACK_H
#define VOYAGER_IMGCALLBACK_H
/*
 * $Id: imgcallback.h,v 1.18 2001/08/28 20:33:15 zapek Exp $
 */

// Image decoder callback table
struct imgcallback {
	APTR (ASM * nets_open)(
		__reg( a0, STRPTR url ),
		__reg( a1, STRPTR referer ),
		__reg( a2, APTR informobj ),
		__reg( a3, APTR gauge ),
		__reg( a4, APTR txtstatus ),
		__reg( d0, ULONG timeout ),
		__reg( d1, ULONG flags )
	);
	int (ASM *nets_state)( __reg( a0, struct nstream *ns ) );
	void (ASM *nets_close)( __reg( a0, struct nstream *ns ) );
	char* (ASM *nets_getdocmem)( __reg( a0, struct nstream *ns ) );
	int (ASM *nets_getdocptr)( __reg( a0, struct nstream *ns ) );
	void (ASM *nets_settomem)( __reg( a0, struct nstream *ns ) );
	char * (ASM *nets_url)( __reg( a0, struct nstream *ns ) );
	char * (ASM *nets_redirecturl)( __reg( a0, struct nstream *ns ) );
	void (ASM *nets_lockdocmem)( void );
	void (ASM *nets_unlockdocmem)( void );
	void (ASM *imgcallback_decode_hasinfo)(
		__reg( a0, APTR obj ),
		__reg( a1, struct BitMap *bm ),
		__reg( d0, int img_x ),
		__reg( d1, int img_y ),
		__reg( a2, struct BitMap *maskbm ),
		__reg( a3, struct MinList *imagelist ) );
	void (ASM *imgcallback_decode_gotscanline)(
		__reg( a0, APTR obj ),
		__reg( d0, int min_touched_y ),
		__reg( d1, int max_touched_y ) );
	void (ASM *imgcallback_decode_done)( __reg( a0, APTR obj ) );
	void (ASM *removeclone)( __reg( a0, struct BitMap *bm ) );
	void (ASM *nets_release_buffer)( __reg( a0, struct nstream *ns ) );
	char * (ASM *nets_errorstring)( __reg( a0, struct nstream *ns ) );

	// New -- V core Version information. new functions go *after*
	int v_major;
	int v_minor;
	int	v_build;

	/* new functions */
	int v_isppc; /* set is V is a MorphOS ELF executable (3.3.88) */

	/*
	 * NOTE: if you extend that structure please fix it in imgdecode/imgdecode.c for
	 * the MorphOS part as well (imgdec_libinit())
	 */
};

#endif /* VOYAGER_IMGCALLBACK_H */
