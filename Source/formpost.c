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
**
** $Id: formpost.c,v 1.19 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "malloc.h"
#include "mui_func.h"


static struct MinList postdatalist, formstorelist;
static int maxid;

struct formdata {
	struct MinNode n;
	int len;
	int id;
	UBYTE enctype;
	char data[ 0 ]; // Already encoded
};

void init_postdatalist( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &postdatalist );
	NEWLIST( &formstorelist );
}

int formp_storedata( char *data, int len, int enctype )
{
	struct formdata *nfd;

	if( len < 0 )
		len = strlen( data );

	nfd = malloc( sizeof( *nfd ) + len );
	memset( nfd, '\0', sizeof( *nfd ) + len ); /* TOFIX: maybe not necessary */
    nfd->id = ++maxid;
	nfd->len = len;
	nfd->enctype = enctype;
	memcpy( nfd->data, data, len );

	Forbid();
	ADDTAIL( &postdatalist, nfd );
	Permit();

	return( nfd->id );
}

char *formp_getdata( int id, int *lenp, int *enctype )
{
	struct formdata *fd;

	Forbid();
	for( fd = FIRSTNODE( &postdatalist ); NEXTNODE( fd ); fd = NEXTNODE( fd ) )
	{
		if( fd->id == id )
		{
			Permit();
			*lenp = fd->len;
			*enctype = fd->enctype;
			return( fd->data );
		}
	}
	Permit();
	return( 0 );
}

//
// Form remembering functions
//

struct formsnode {
	struct MinNode n;
	char *url;
	int fid, fsid;
	int size;
	char data[ 0 ];
};

void formstore_add( char *url, int formelementid, int formsubid, char *data, int size )
{
	struct formsnode *fn;

	if( size < 0 )
		size = strlen( data ) + 1;

	//Printf( "fs_add(%s,%ld,%ld,%lx,%ld)\n", url, formelementid, formsubid, data, size );

	for( fn = FIRSTNODE( &formstorelist ); NEXTNODE( fn ); fn = NEXTNODE( fn ) )
	{
		if( fn->fid == formelementid && fn->fsid == formsubid && !strcmp( fn->url, url ) )
		{
			REMOVE( fn );
			free( fn->url );
			free( fn );
			break;
		}
	}

	fn = malloc( sizeof( *fn ) + size );
	memset( fn, '\0', sizeof( *fn ) + size ); /* TOFIX: maybe not necessary */
    fn->fid = formelementid;
	fn->fsid = formsubid;
	fn->url = strdup( url ); /* TOFIX */
	fn->size = size;
	memcpy( fn->data, data, size );

	ADDTAIL( &formstorelist, fn );
}

char *formstore_get( char *url, int formelementid, int formsubid, int *size )
{
	struct formsnode *fn;

	//Printf( "fs_get(%s,%ld,%ld,%lx)\n", url, formelementid, formsubid, size );

	for( fn = FIRSTNODE( &formstorelist ); NEXTNODE( fn ); fn = NEXTNODE( fn ) )
	{
		if( fn->fid == formelementid && fn->fsid == formsubid && !strncmp( fn->url, url, 255 ) )
		{
			if( size )
				*size = fn->size;
			return( fn->data );
		}
	}
	return( 0 );
}
