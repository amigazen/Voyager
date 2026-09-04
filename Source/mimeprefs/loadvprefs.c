#include "mimeprefs.h"

#define PREFSFILEID MAKE_ID('V','Y','²',0)

static APTR prefspool;
struct MinList prefslist;

struct prefsnode {
	struct MinNode n;
	ULONG id;
	ULONG size;
	char data[ 0 ];
};

static ULONG readlong( BPTR f )
{
	ULONG v = 0;
	Read( f, &v, sizeof( v ) );
	return( v );
}

//
// Encoding
//

static void __inline codebytes( UBYTE *data, int size )
{
	while( size-- )
		*data++ ^= rand() & 0xff;
}

#define DSI_GRP(x) (x<<16)
#define DSI_MIME (DSI_GRP(9))
#define DSI_MIME_EXT_OFFSET 2048
#define DSI_MIME_ACTION_OFFSET 4096
#define DSI_MIME_APP_OFFSET (2048+4096)
#define DSI_SAVEDEST (DSI_GRP(5000))

static __inline struct prefsnode *mkpn( ULONG id, ULONG size, APTR data )
{
	struct prefsnode *n;

	n = AllocPooled( prefspool, sizeof( *n ) + size );
	n->id = id;
	n->size = size;
	if( size && data )
		memcpy( n->data, data, size );

	ADDTAIL( &prefslist, n );

	return( n );
}

static __inline struct prefsnode * fpn( ULONG id )
{
	struct prefsnode *n, *nn;

	for( n = FIRSTNODE( &prefslist ); nn = NEXTNODE( n ); n = nn )
		if( n->id == id )
			return( n );

	return( NULL );
}

static __inline void rmpn( struct prefsnode *pn )
{
	REMOVE( pn );
	//FreePooled( prefspool, pn, sizeof( *pn ) + pn->size );
}

int importvprefs( char *filename )
{
	BPTR f;
	ULONG id, size;
	ULONG numnodes;
	struct prefsnode *n;
	int c;
	char *savedest;
	extern STRPTR mimetype_opts[];
	char *p;

	f = Open( filename, MODE_OLDFILE );
	if( !f )
		return( -1 );

	id = readlong( f );

	if( id != PREFSFILEID )
	{
		Close( f );
		SetIoErr( ERROR_OBJECT_WRONG_TYPE );
		return( -2 );
	}
	numnodes = readlong( f );	
	srand( readlong( f ) );

	set( lv_mime, MUIA_List_Quiet, TRUE );
	DoMethod( lv_mime, MUIM_List_Clear );

	NEWLIST( &prefslist );
	prefspool = CreatePool( 0, 2048, 1024 );

	while( numnodes-- )
	{
		id = readlong( f );
		size = readlong( f );

		if( size > 1024 )
			break;

		if( n = fpn( id ) )
			rmpn( n );

		n = mkpn( id, size, NULL );
		if( size )
		{
			if( Read( f, n->data, size ) == size )
			{
				codebytes( n->data, size );
			}
			else
			{
				rmpn( n );
				break;
			}
		}
	}

	Close( f );

	n = fpn( DSI_SAVEDEST );
	if( n )
		savedest = n->data;
	else
		savedest = "";

	// convert
	for( c = 0; ; c++ )
	{
		struct mimeinfo mi;
		int d;

		n = fpn( DSI_MIME + c );
		if( !n )
			break;

		//Printf( "found %ld %s\n", c, n->data );

		memset( &mi, 0, sizeof( mi ) );

		for( d = 0; mimetype_opts[ d ]; d++ )
		{
			if( !strncmp( n->data, mimetype_opts[ d ], strlen( mimetype_opts[ d ] ) ) )
			{
				mi.basetype = d;
				break;
			}
		}

		if( !strchr( n->data, '/' ) )
			break;

		strcpy( mi.type, strchr( n->data, '/' ) + 1 );

		n = fpn( DSI_MIME + DSI_MIME_EXT_OFFSET + c );
		if( !n )
			break;
		strcpy( mi.ext, n->data );
		for( p = mi.ext; *p; p++ )
			if( *p == ',' )
				*p = ' ';
		
		n = fpn( DSI_MIME + DSI_MIME_APP_OFFSET + c );
		if( !n )
			break;
		strcpy( mi.app, n->data );

		n = fpn( DSI_MIME + DSI_MIME_ACTION_OFFSET + c );
		if( !n )
			break;
		
		switch( *((ULONG*)n->data ) )
		{
			default:
				mi.act = 0;
				break;
			case 1:
				mi.act = 2;
				mi.use_internal = TRUE;
				break;
			case 2:
				mi.act = 1;
				break;
			case 3:
				mi.act = 3;
				break;
		}

		strcpy( mi.dir, savedest );

		DoMethod( lv_mime, MUIM_List_InsertSingle, &mi, MUIV_List_Insert_Bottom );
	}

	DeletePool( prefspool );

	set( lv_mime, MUIA_List_Quiet, FALSE );


	return( 0 );
}
