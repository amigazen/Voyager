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


#ifndef VOYAGER_NETWORK_CALLBACK_H
#define VOYAGER_NETWORK_CALLBACK_H
/*
 * Shared between Voyager and the imgdecoders
 *
 * $Id: network_callback.h,v 1.8 2003/04/25 19:13:55 zapek Exp $
*/

/*
 * nets_open() flags
 */
#define NOB_RELOAD    0 /* reloads */
#define NOB_ADDURL    1 /* adds the URL to the historylist */
#define NOB_TIMESTAMP 2 /* timestamps the recv() in milliseconds */
#define NOB_PROGRESS  3 /* docptr */

#define NOF_RELOAD    (1UL << NOB_RELOAD)
#define NOF_ADDURL    (1UL << NOB_ADDURL)
#define NOF_TIMESTAMP (1UL << NOB_TIMESTAMP)
#define NOF_PROGRESS  (1UL << NOB_PROGRESS)


APTR ASM SAVEDS nets_open(
	__reg( a0, STRPTR url ),
	__reg( a1, STRPTR referer ),
	__reg( a2, APTR informobj ),
	__reg( a3, APTR gauge ),
	__reg( a4, APTR txtstatus ),
	__reg( d0, ULONG timeout ),
	__reg( d1, ULONG flags ) );
int ASM SAVEDS nets_state( __reg( a0, struct nstream *ns ) );
void ASM SAVEDS nets_close( __reg( a0, struct nstream *ns ) );
char * ASM SAVEDS nets_getdocmem( __reg( a0, struct nstream *ns ) );
int ASM SAVEDS nets_getdocptr( __reg( a0, struct nstream *ns ) );
void ASM SAVEDS nets_settomem( __reg( a0, struct nstream *ns ) );
char * ASM SAVEDS nets_url( __reg( a0, struct nstream *ns ) );
char * ASM SAVEDS nets_fullurl( __reg( a0, struct nstream *ns ) );
char * ASM SAVEDS nets_redirecturl( __reg( a0, struct nstream *ns ) );
void ASM SAVEDS nets_lockdocmem( void );
void ASM SAVEDS nets_unlockdocmem( void );
void ASM SAVEDS nets_release_buffer( __reg( a0, struct nstream *ns ) );
char ASM *nets_errorstring( __reg( a0, struct nstream *ns ) );
char ASM SAVEDS *nets_getheader( __reg( a0, struct nstream *ns ), __reg( a1, char *header ) );


#ifdef __GNUC__
struct BitMap;
#endif

/*
 * Doesn't really belong there but... bah
 */
void ASM imgcallback_decode_hasinfo(
	__reg( a0, APTR obj ),
	__reg( a1, struct BitMap *bm ),
	__reg( d0, int img_x ),
	__reg( d1, int img_y ),
	__reg( a2, struct BitMap *maskbm ),
	__reg( a3, struct MinList *imagelist )
);
void ASM imgcallback_decode_gotscanline(
	__reg( a0, APTR obj ),
	__reg( d0, int min_touched_y ),
	__reg( d1, int max_touched_y )
);
void ASM imgcallback_decode_done( __reg( a0, APTR obj ) );

#endif /* VOYAGER_NETWORK_CALLBACK_H */
