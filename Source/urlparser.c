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
** $Id: urlparser.c,v 1.27 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"
#include "urlparser.h"
#include "malloc.h"
#include "dos_func.h"

int url_hasscheme( char *url )
{
	if( isalpha( *url ) )
	{
		url++;
		while( isalpha( *url ) )
			url++;
		if( *url == ':' )
			return( TRUE );
	}
	return( FALSE );
}

void uri_split( char *url, struct parsedurl *out )
{
	char *p, *after_scheme;

	memset( out, 0, sizeof( *out ) );

	url = stpblk( url );

	after_scheme = url;

	for( p = url; *p; p++ )
	{
		if( *p=='/' || *p=='#' || *p=='?' )
			break;
		if( *p == ':' )
		{
			*p = 0;
			if( stricmp( after_scheme, "URL" ) )
			{
				out->scheme = after_scheme;
				after_scheme = p + 1;
				break;
			}
			else
				after_scheme = p + 1;
		}
	}

	p = after_scheme;
	if( *p == '/' )
	{
		if( p[ 1 ] == '/' )
		{
			// host specified
			out->host = p + 2;
			*p = 0;
			p = strpbrk( out->host, "/?#" );
			if( p )
			{
				if( *p == '/' )
				{
					*p++ = 0;
					out->path = p;
				}
				else if( *p == '?' )
				{
					*p++ = 0;
					out->path = "";
					out->args = p; 
				}
				else if( *p == '#' )
				{
					*p++ = 0;
					out->path = "";
					out->fragment = p;
				}
			}
			else
				out->path = strchr( out->host, 0 );
		}
		else
			out->path = p + 1;
	}
	else
	{
		out->pathrelative = TRUE;
		out->path = p;
	}

	// break host further down
	if( out->host )
	{
		p = strchr( out->host, '@' );
		if( p )
		{
			out->username = out->host;
			*p++ = 0;
			out->host = p;
			p = strchr( out->username, ':' );
			if( p )
			{
				*p++ = 0;
				out->password = p;
			}
		}
		p = strchr( out->host, ':' );
		if( p )
		{
			*p++ = 0;
			out->port = atoi( p );
		}
		strlwr( out->host );
	}

	if( out->scheme && !out->port )
	{
		if( !stricmp( out->scheme, "file" ) )
		{
			if( out->host && out->host[ 0 ] && stricmp( out->host, "localhost" ) )
			{
				out->scheme = "ftp";
			}
		}

		if( !stricmp( out->scheme, "http" ) )
			out->port = 80;
		else if( !stricmp( out->scheme, "ftp" ) )
			out->port = 21;
		else if( !stricmp( out->scheme, "gopher" ) )
			out->port = 70;
		else if( !stricmp( out->scheme, "https" ) )
			out->port = 443;
		else if( !stricmp( out->scheme, "telnet" ) )
			out->port = 23;
	}

	if( out->path )
	{
		p = strchr( out->path, '#' );
		if( p )
		{
			if( p == out->path )
				out->path = NULL;
			*p++ = 0;
			while( *p == '#' )
				p++;
			out->fragment = p;
		}
	}
	if( out->path )
	{
		p = strchr( out->path, '?' );
		if( p )
		{
			if( p == out->path )
			{
				out->path = "";
			}
			*p++ = 0;
			out->args = p;
		}
	}
}

static void __inline strcpy_add( char **to, char *what, char *maxptr )
{
	char *top = *to;

	while( *what && top < maxptr )
	{
		*top++ = *what++;
	}
	*to = top;
}

#define add_s(x) strcpy_add( &to, x, maxp )

void uri_remerge( struct parsedurl *u, char *to )
{
	char *maxp = to + MAXURLSIZE - 2;

	if( u->scheme )
	{
		add_s( u->scheme );
		add_s( ":" );
	}

	if( u->host )
	{
		add_s( "//" );
		if( u->username )
		{
			if( u->password )
			{
				add_s( u->username );
				add_s( ":" );
				add_s( u->password );
			}
			else
			{
				add_s( u->username );
			}
			add_s( "@" );
		}
		add_s( u->host );
		if( u->port )
		{
			if( !u->scheme ||
				( !stricmp( u->scheme, "http" ) && u->port != 80 ) ||
				( !stricmp( u->scheme, "ftp" ) && u->port != 21 ) ||
				( !stricmp( u->scheme, "gopher" ) && u->port != 70 ) ||
				( !stricmp( u->scheme, "https" ) && u->port != 443 ) ||
				( !stricmp( u->scheme, "telnet" ) && u->port != 23 ) )
			{
				sprintf( to, ":%ld", (long int)u->port );
				to = strchr( to, 0 );
			}
		}
		add_s( "/" );
	}

	if( u->path )
		add_s( u->path );

	if( u->args )
	{
		add_s( "?" );
		add_s( u->args );
	}

	if( u->fragment )
	{
		add_s( "#" );
		add_s( u->fragment );
	}

	*to = 0;

}

static void simplify( char *path )
{
	char *p, *p2;

	while((p = strstr( path, "/./" )))
		strcpy( p, p + 2 );

	while((p = strstr( path + 1, "/../" )))
	{
		p2 = p - 1;
		while( p2 > path && *p2 != '/' )
			p2--;
		strcpy( p2 + 1, p + 4 );
	}
	while( !strncmp( path, "/../", 4 ) )
		strcpy( path, path + 3 );

}

void uri_canon( char *from, char *to )
{
	char *fromw = strdup( from );
	struct parsedurl p;

	uri_split( fromw, &p );
	uri_remerge( &p, to );

	free( fromw );
}

void uri_mergeurl( char *from, char *add, char *to )
{
	struct parsedurl fromp, addp;
	char *fromw, *addw;
	char *pathtemp = NULL;

	addw = strdup( add );

	uri_split( addw, &addp );

	if( addp.scheme && addp.host )
	{
		// full URL
		uri_remerge( &addp, to );
		free( addw );
		return;
	}

	fromw = strdup( from );
	uri_split( fromw, &fromp );

	if( addp.scheme && ( stricmp( addp.scheme, fromp.scheme ) ) )
	{
		// full URL
		free( addw );
		free( fromw );
		stccpy( to, add, MAXURLSIZE );
		return;
	}

	if( !addp.scheme )
		addp.scheme = fromp.scheme;

	if( !addp.host )
	{
		addp.host = fromp.host;
		addp.port = fromp.port;
		addp.username = fromp.username;
		addp.password = fromp.password;
	}

	if( addp.path )
	{
		int hasfilescheme = addp.scheme && !stricmp( addp.scheme, "file" );
		if( addp.pathrelative )
		{
			char *p;
			int ptlen = ( fromp.path ? strlen( fromp.path ) : 0 ) + strlen( addp.path ) + 10;
			if((pathtemp = malloc( ptlen )))
			{
				if( fromp.path )
					strcpy( pathtemp, fromp.path );
				else
					pathtemp[ 0 ] = 0;
				// special handling for file:// URLs
				if( hasfilescheme )
				{
					char tmpnew[ 256 ];
					*PathPart( pathtemp ) = 0;
					stccpy( tmpnew, addp.path, sizeof( tmpnew ) );
					while((p = strstr( tmpnew, "../" )))
						strcpy( p, p + 2 );
					while((p = strstr( tmpnew, "./" )))
						strcpy( p, p + 2 );
					AddPart( pathtemp, tmpnew, ptlen );
					// collapse "//". This fucks up paths which reference parents of root (e.g. "MyAssign:///myfile.html")
					while((p = strstr( pathtemp, "//" )))
					{
						char *pp;
						*p++ = '\0';
						pp = PathPart( pathtemp );
						if( *( pp - 1 ) == ':' )
							p++;
						strcpy( pp, p );
					}
					addp.path = pathtemp;
				}
				else if( addp.pathrelative )
				{
					// merge this
					p = strrchr( pathtemp, '/' );
					p = p ? p + 1 : pathtemp;
					strcpy( p, addp.path );
					strins( pathtemp, "/" );
					simplify( pathtemp );
					addp.path = pathtemp + 1;
				}
			}
		}
		else if( hasfilescheme )
		{
			// the volume name seems a more sensible base than V's current dir
			char *p;
			if( fromp.path && ( p = strchr( fromp.path, ':' ) ) && !strchr( addp.path, ':' ) )
			{
				*(p+1)='\0';
				if((pathtemp = malloc( strlen( fromp.path ) + strlen( addp.path ) + 1 )))
				{
					strcpy( pathtemp, fromp.path );
					strcat( pathtemp, addp.path );
					addp.path = pathtemp;
				}
			}
		
		}
	}
	else
	{
		addp.path = fromp.path;
		addp.args = addp.args ? addp.args : fromp.args;
	}

	uri_remerge( &addp, to );

	free( fromw );
	free( addw );
	if( pathtemp )
		free( pathtemp );
}

void uri_decode(char *url)
{
	char hex[ 4 ];
	char *p;
	long v;

	/* convert ascii chars (%20), etc... */
	while( p = strchr( url, '%' ) )
	{
		memcpy( hex, p + 1, 2 );
		hex[ 2 ] = 0;
		stch_l( hex, &v );

		*p = (char)v;
		strcpy( p + 1, p + 3 );
	}
}

#ifdef URLDEBUG
static void printpp( struct parsedurl *p )
{
	printf( "scheme '%s'\n", p->scheme );
	printf( "host '%s'\n", p->host );
	printf( "username '%s'\n", p->username );
	printf( "password '%s'\n", p->password );
	printf( "path '%s'\n", p->path );
	printf( "args '%s'\n", p->args );
	printf( "frag '%s'\n", p->fragment );
	printf( "port %ld rel %ld\n", p->port, p->pathrelative );
}
void main( int argc, char **argv )
{
	struct parsedurl p1, p2;
	char buffer[ 256 ];

	if( argc != 3 )
		exit( 20 );

	uri_split( strdup( argv[ 1 ] ), &p1 );
	uri_split( strdup( argv[ 2 ] ), &p2 );

	printf( "URL1 %s\n", argv[ 1 ] );
	printpp( &p1 );
	printf( "URL2 %s\n", argv[ 2 ] );
	printpp( &p2 );

	uri_mergeurl( argv[ 1 ], argv[ 2 ], buffer );
	printf( "merged: %s\n", buffer );
}
#endif
