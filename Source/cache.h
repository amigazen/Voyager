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


#ifndef VOYAGER_CACHE_H
#define VOYAGER_CACHE_H
/*
 * $Id: cache.h,v 1.11 2003/04/25 19:13:53 zapek Exp $
 */


/*
 * Version of the cache format.
 * Bumprev everytime there's a change in the format
 * or the Global.1 file
 */
#define CACHEVERSION 2

/* Cache verify */

extern APTR verifypool;
extern APTR prunecachewin;
extern ULONG prunecacheinit;
extern struct Process *prunecacheproc;
extern int cache_lastprune;
extern struct SignalSemaphore prunecachesem;

struct verifynode {
	struct verifynode *next;
	UWORD len;
	char url[ 0 ];
};

struct cacheheader {
	time_t expires;         /* expiration date */
	int doclen; 			/* document length */
	UWORD mimelen, urllen;
	char pad[ 128 ];
};

struct cache_stats {
	int total_hits;
	int from_cache;
	int verified_ok;
	int verified_failed;
	int total_fetched;
	int stored_fetched;
};

extern struct cache_stats cache_stats;
extern struct verifynode *verifylist;
extern int estimated_cache_size;
extern BPTR cache_locks[ 256 ];

/* protos */
BPTR chcache( int num );
int checkincache( struct unode *un, time_t *expireptr );
int isverified( char *url );
void un_setup_cache( struct unode *un );
void un_memtocache( struct unode *un );
void addverify( char *url );
void readglobalcache( void );
void write_estimated_cachesize( void );
void writeglobalcache( void );
void makecachepath( char *ext, char *to );
#endif /* VOYAGER_CACHE_H */
