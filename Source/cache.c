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
** Cache handling
** --------------
**
** Cache format:
**
** Cached objects are stored in the Cache dir;
** the URL is MD5 encoded. The most significant
** byte of the MD5 word serves as a directory key
** (CIX_<hex>), the remaining 15 bytes are encoded
** in the filename (A + 4 bits). AmigaDOS file date
** is assumed to reflect the time when the object
** was fetched and stored in the cache.
**
** Old cache format
** ----------------
**
** Every object is prefixed with a fixed size
** header block:
**
** struct cacheheader {
**     time_t expires;
**     int doclen; // document length
**    UWORD mimelen, urllen;
**    char pad[ 128 ];
** };
**
** Following this block are three variable length
** records:
**
**  the full URL (length given by cacheheader.urllen)
**  the MIME type as sent by the server (length given by cacheheader.mimelen)
**  the real object data (length given by cacheheader.doclen)
**
**  None of this records are null terminated.
**
**
** New cache format
** ----------------
** cacheheader moved into comments, there are 5 hexadecimal strings of 8 chars
** [checksum] [ expires] [doclen] [urllen] [mimilen]
** the checksum is all the rest XORed
**
** The routine is not backward compatible anymore,
** there's no way to detect the old cache format
**
**
** $Id: cache.c,v 1.51 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

#include "voyager_cat.h"
#include "classes.h"
#include "cache.h"
#include "network.h"
#include "prefs.h"
#include "methodstack.h"
#include "init.h"
#include "dos_func.h"


APTR verifypool; /* cache verify */

int estimated_cache_size;
int cache_last_prune;
struct verifynode *verifylist;
struct cache_stats cache_stats;
BPTR cache_locks[ 256 ]; /* array for the cache directories */
ULONG gp_cachesize;

int init_verify( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	verifypool = CreatePool( 0, 2048, 1024 );
	return( ( int )verifypool );
}

void cleanup_verify( void )
{
	if( verifypool )
	{
		D( db_init, bug( "cleaning up..\n" ) );
		DeletePool( verifypool );
	}
}

/*
 * Reads and initializes global cache datas. The estimated cache size
 * is stored on the file comment because it'll be updated frequently.
 * To have a better estimation.
 */
void readglobalcache( void )
{
#if USE_DOS
	struct AsyncFile *f;
	char path[ 256 ];
	char buffer[ 256 ];

	makecachepath( "Global.1", path );

	if( f = OpenAsync( path, MODE_READ, 1024 ) )
	{
		/*
		 * Version string. Not handled ATM.
		 */
		if( FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
		{
			/*
			 * Time of last prunning
			 */
			if( FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
			{
				if( stch_l( buffer, (long*)&cache_last_prune ) )
				{
					/*
					 * Cache stats
					 */
					if( FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
					{
						BPTR l;

						if( sscanf( buffer, "%08X%08X%08X%08X%08X%08X", &cache_stats.total_hits,
							&cache_stats.from_cache,
							&cache_stats.verified_ok,
							&cache_stats.verified_failed,
							&cache_stats.total_fetched,
							&cache_stats.stored_fetched ) == 6 )
						{
						
							/*
							 * Estimated cache size
							 */
							if( l = Lock( path, SHARED_LOCK ) )
							{
								D_S( struct FileInfoBlock, fib );

								if( Examine( l, fib ) == DOSTRUE )
								{
									sscanf( fib->fib_Comment, "%08X", &estimated_cache_size );
								}
								UnLock( l );
							}
						}
					}
				}
			}
		}

		CloseAsync( f );
	}
#endif /* USE_DOS */
}

/*
 * Feel free to add backup to the following routine.
 * As the file content is useless ATM, I won't bother..
 */
void writeglobalcache( void )
{
#if USE_DOS
	struct AsyncFile *f;
	char path[ 256 ];

	makecachepath( "Global.1", path );

	if( f = OpenAsync( path, MODE_WRITE, 1024 ) )
	{
		/*
		 * Version string
		 */
		FPrintfAsync( f, "$" );
		FPrintfAsync( f, "VER: VoyagerGlobalCache %ld.0\n", CACHEVERSION );
		/*
		 * Time of last prunning
		 */
		FPrintfAsync( f, "%08lX\n", cache_last_prune );
		/*
		 * Cache stats
		 */
		FPrintfAsync( f, "%08X%08X%08X%08X%08X%08X", cache_stats.total_hits,
													 cache_stats.from_cache,
													 cache_stats.verified_ok,
													 cache_stats.verified_failed,
													 cache_stats.total_fetched,
													 cache_stats.stored_fetched
		);

		CloseAsync( f );
	}

	/*
	 * Now we can save the estimated cache size here
	 */
	 write_estimated_cachesize();
#endif
}


/*
 * Write the estimated cache size into the filecomments
 * of <cachepath>/Global.1
 */
void write_estimated_cachesize( void )
{
#if USE_DOS
	char path[ 256 ];
	char buffer[ 10 ];

	makecachepath( "Global.1", path );

	sprintf( buffer, "%08X", estimated_cache_size );
	SetComment( path, buffer );
#endif
}

/*
 * Checks if a location has been verified already.
 * Depends on the settings (once, upon every access, never)
 */
int isverified( char *url )
{
	UWORD len = strlen( url );
	struct verifynode *vn = verifylist;

	if( !isonline() || gp_cache_verify == 2 )
	{
		return( TRUE ); /* don't verify */
	}

	while( vn )
	{
		if( vn->len == len && !strcmp( vn->url, url ) )
			return( TRUE ); /* verified */
		vn = vn->next;
	}
	return( FALSE );
}


/*
 * Adds an URL to the verify list
 */
void addverify( char *url )
{
	struct verifynode *vn;
	UWORD len = strlen( url );

	if( gp_cache_verify == 1 || !gp_cachesize ) /* verify upon every access */
		return;

	vn = AllocPooled( verifypool, sizeof( *vn ) + len + 1 );
	if( vn )
	{
		vn->len = len;
		strcpy( vn->url, url );
		vn->next = verifylist;
		verifylist = vn;
	}
}

/*
 * Change the current directory to the correct
 * subdir in the cache matching the URL
 */
BPTR chcache( int num )
{
#if USE_DOS
	if( !cache_locks[ num ] ) /* subdir doesn't exist, create it */
	{
		char buffer[ 232 ], tmp[ 16 ];
		BPTR l;

		strcpy( buffer, gp_cachedir );
		sprintf( tmp, "CIX_%02lx", ( long int )num );
		AddPart( buffer, tmp, sizeof( buffer ) );

		l = Lock( buffer, SHARED_LOCK );
		if( !l )
		{
			UnLock( CreateDir( buffer ) );
			l = Lock( buffer, SHARED_LOCK );
		}
		cache_locks[ num ] = l;
	}
	return( CurrentDir( cache_locks[ num ] ) );
#else
	return 0;
#endif
}


void free_cachelocks( void )
{
#if USE_DOS
	int c;
#endif

	D( db_init, bug( "cleaning up..\n" ) );

	if ( prunecacheinit )
	{
		Forbid();
		while( !AttemptSemaphore( &prunecachesem ) )
		{
			D( db_cache, bug( "trying to flush cache process..\n" ) );
			Signal( ( struct Task * )prunecacheproc, SIGBREAKF_CTRL_C );
			Delay( 2 );
		}
		Permit();
		ReleaseSemaphore( &prunecachesem );

		#if USE_DOS
		for( c = 0; c < 256; c++ )
		{
			if( cache_locks[ c ] )
				UnLock( cache_locks[ c ] );
		}
		#endif
	
		if( app_started )
		{
			writeglobalcache();
		}
	}
}

#if USE_DOS
static void makepname( UBYTE *md5, char *pname )
{
	int c;

	for( c = 1; c < 16; c++ )
	{
		*pname++ = 'A' + ( md5[ c ] >> 4 );
		*pname++ = 'A' + ( md5[ c ] & 0xf );
	}
	*pname = 0;
}
#endif

/*
 * Check if an unode is in the disk cache
 */
int checkincache( struct unode *un, time_t *expires )
{
#if USE_DOS
	UBYTE md5[ 16 ];
	char pname[ 32 ];
	BPTR oldcd;
	int rc = FALSE;
	time_t exp;
	D_S(struct FileInfoBlock,fib);

	if( !gp_cachesize )
		return( rc );

	VAT_CalcMD5( un->url, strlen( un->url ), md5 );
	oldcd = chcache( md5[ 0 ] );
	makepname( md5, pname );

	D( db_cache, bug("dir == %lx, pname == %s\n", md5[ 0 ], pname));

	if( un->l = Lock( pname, SHARED_LOCK ) )
	{
		Examine( un->l, fib );
		un->cachedate = __datecvt( &fib->fib_Date );
		if( sscanf( fib->fib_Comment, "%08lx%08lx", &exp, &exp ) == 2 )
		{
			*expires = exp;
			D( db_cache, bug("is in cache\n"));
			rc = TRUE;
		}
	}

	CurrentDir( oldcd );

	return( rc );
#else
	return FALSE;
#endif
}

void un_setup_cache( struct unode *un )
{
#if USE_DOS
	int failed = FALSE;
	ULONG checksum;
	ULONG urllen;
	ULONG mimelen;
	char filename[ 256 ];
	un->fromcache = TRUE;

	NameFromLock( un->l, filename, 255 );

	if( un->fromfile = OpenFromLock( un->l ) )
	{
		struct cacheheader ch;
		D_S(struct FileInfoBlock, fib);

		if ( ExamineFH( un->fromfile, fib) )
		{

			if ( fib->fib_Comment[ 0 ] ) /* get the cache header from the comment */
			{
				if ( strlen( fib->fib_Comment ) != 40 || sscanf( fib->fib_Comment, "%08X%08X%08X%08X%08X", ( int * )&checksum, ( int * )&ch.expires, ( int * )&ch.doclen, ( int * )&urllen, ( int * )&mimelen ) != 5 )
				{
					D( db_cache, bug("bad comment length or parsing\n"));
					failed = TRUE;
				}
				else
				{
					if ( checksum == ch.expires ^ ch.doclen ^ urllen ^ mimelen )
					{
						ch.urllen = urllen;
						ch.mimelen = mimelen;
						if( ch.mimelen >= sizeof( un->mimetype ) )
							failed = TRUE;
						D( db_cache, bug( "got cache header (comment): checksum == %lu, ch.expires == %lu, ch.doclen == %lu, ch.urllen == %lu, ch.mimelen == %lu\n", checksum, ch.expires, ch.doclen, ch.urllen, ch.mimelen ));
					}
					else
					{
						failed = TRUE;
						D( db_cache, bug("checksum failed, it's %lu, where ch.expires == %lu, ch.doclen == %lu, urllen == %lu, mimelen == %lu, content of comment is %s\n", checksum, ch.expires, ch.doclen, urllen, mimelen, fib->fib_Comment));
					}
				}
			}
			else
			{
				failed = TRUE;
			}

			if (!failed)
			{
				Seek( un->fromfile, ch.urllen, OFFSET_CURRENT );   
				Read( un->fromfile, un->mimetype, ch.mimelen );
				un->mimetype[ ch.mimelen ] = 0;
				D( db_cache, bug("un->mimetype == %s\n", un->mimetype));
				un->doclen = ch.doclen;
				un->state = UNS_WAITING;
				un->l = NULL;
				return;
			}
		}
	}
	else
		UnLock( un->l );
	
	un->l = NULL;
	
	if( failed )
	{
		/* cache error! */
		sprintf( un->errorstring, GS( NWM_ERROR_CACHEOPENFAILED ), IoErr() );
		un->errorcode = -1;
		un->state = UNS_FAILED;
		makehtmlerror( un );
	}
#endif
}

/*
 * Stores an unode into the disk cache
 */
void un_memtocache( struct unode *un )
{
#if USE_DOS
	UBYTE md5[ 16 ];
	char pname[ 32 ];
	BPTR oldcd;
	ULONG checksum;
	struct cacheheader ch;
	char buffer[ 34 ];
	BPTR f;

	if( !un->membuff || !gp_cachesize || un->postid )
		return;

	cache_stats.stored_fetched++;

	memset( &ch, 0, sizeof( ch ) );

	VAT_CalcMD5( un->url, strlen( un->url ), md5 );
	oldcd = chcache( md5[ 0 ] );
	makepname( md5, pname );

	ch.urllen = strlen( un->url );
	ch.mimelen = strlen( un->mimetype );
	ch.expires = un->cacheexpires;

	checksum = un->docptr ^ ch.urllen ^ ch.mimelen ^ ch.expires;
	
	/* mmm, expiration date isn't saved ? */
	sprintf( buffer, "%08X%08X%08X%08X%08X", ( int )checksum, ( int )ch.expires, ( int )un->docptr, ( int )strlen( un->url ), ( int )strlen( un->mimetype ) );

	f = Open( pname, MODE_OLDFILE );
	
	if( f )
	{
		D_S(struct FileInfoBlock,fib);
		ExamineFH( f, fib );
		estimated_cache_size -= fib->fib_Size;
	}
	else
		f = Open( pname, MODE_NEWFILE );
	
	if( f )
	{
		Write( f, un->url, ch.urllen );
		Write( f, un->mimetype, ch.mimelen );
		Write( f, un->membuff, un->docptr );
		Close( f );

		D( db_cache, bug("wrote cache with comments %s\n", buffer));
		SetComment( pname, buffer );

		estimated_cache_size += strlen( un->url ) + strlen( un->mimetype ) + un->docptr;
	}

	if( prunecachewin )
	{
		pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetCurrentSize, estimated_cache_size / 1024 );
	}
	write_estimated_cachesize();

	CurrentDir( oldcd );

	addverify( un->url );
#endif
}

#endif /* USE_NET */
