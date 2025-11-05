#define __USE_SYSBASE
#include <proto/dos.h>
#include <proto/exec.h>
#include <string.h>
#include <ctype.h>

#include <time.h>

#define NO_VAT_SHORTCUTS
#include "libraries/vat.h"

#include "globals.h"

int ASM SAVEDS VAT_ExpandTemplateA(
	__reg( a0, char *from ),
	__reg( a1, char *to ),
	__reg( d0, int maxlen ), // Now actually USED for a change :^)
	__reg( a2, APTR argarray ) )
{
	ULONG *idp;
	char **datap;
	int ch;
	char *oldto = to;

	DB( ( "called\n" ) );

	if( --maxlen <= 0 )
		return( 0 );

	while( (*from) && (maxlen--) )
	{
		if( *from != '%' )
		{
			*to++ = *from++;
			continue;
		}
		from++;
		ch 		= *from++;
		idp 	= ( ULONG * ) argarray;
		datap 	= ( char ** ) &idp[ 1 ];
		while( *idp )
		{
			/* replacement char found */
			if( *idp == ch )
			{
				if( *datap )
					maxlen -= (stccpy( to, *datap, maxlen )-2);
				to = strchr( to, 0 );
				break;
			}
			idp++;
			idp++;
			datap++;
			datap++;
		}
		/* not found, Zeichen 1:1 übernehmen <- bah, german sucks */
		if( !*idp )
			*to++ = ch;
	}
	*to = 0;

	ch = strlen( oldto );

	return( ch );
}

void ASM SAVEDS VAT_RemSpaces( __reg( a0, char *string ) )
{
	char *p = stpblk( string );

	DB( ( "called\n" ) );

	if( p != string )
		strcpy( string, p );

	p = strchr( string, 0 ) - 1;
	while( p >= string && isspace( *p ) )
		*p-- = 0;
}

APTR ASM SAVEDS VAT_ScanForURLS( __reg( a0, STRPTR string ) )
{
	struct VATS_URL *urllist = 0, *last = 0;
	APTR pool = NULL;
	TEXT buffer[ 256 ], prefix[ 16 ];
	LONG prefixlen = 0, c, lastblank = TRUE;
	STRPTR beginstr = string, origstring = string;

	DB( ( "called\n" ) );

	while( *string )
	{
		int inurl = FALSE;
		int screwit = FALSE;
		int dots = 0;

		if( *string > 127 )
		{
			lastblank = TRUE;
			string++;
			continue;
		}

		// Quick check
		switch( tolower( *string ) )
		{
			case 'w':
			case 'f':
			case 'h':
			case 'm':
			case '@':
				break;

			case ' ':
			case '\xa0':
			case '\t':
			case '\n':
			case '\r':
			case '(':
			case '[':
			case '«':
			case '<':
			case '\'':
			case '\"':
				lastblank = TRUE;
				string++;
				continue;

			default:
				lastblank = FALSE;
				string++;
				continue;
		}

		if( lastblank && !strnicmp( string, "www.", 4 ) && ( string[ 4 ] && !isspace( string[ 4 ] ) ) )
		{
			strcpy( prefix, "http://" );
			prefixlen = 0;
			inurl = 1;
		}
		else if( lastblank && !strnicmp( string, "ftp.", 4 ) && ( string[ 4 ] && !isspace( string[ 4 ] ) ) )
		{
			strcpy( prefix, "ftp://" );
			prefixlen = 0;
			inurl = 1;
		}
		else if( lastblank && !strnicmp( string, "ftpsearch.", 10 ) && ( string[ 10 ] && !isspace( string[ 10 ] ) ) )
		{
			strcpy( prefix, "http://" );
			prefixlen = 0;
			inurl = 1;
		}
		else if( !strnicmp( string, "http://", 7 ) )
		{
			prefix[ 0 ] = 0;
			prefixlen = 7;
			inurl = 1;
		}
		else if( !strnicmp( string, "https://", 8 ) )
		{
			prefix[ 0 ] = 0;
			prefixlen = 8;
			inurl = 1;
		}
		else if( !strnicmp( string, "ftp://", 6 ) )
		{
			prefix[ 0 ] = 0;
			prefixlen = 6;
			inurl = 1;
		}
		else if( !strnicmp( string, "mailto:", 7 ) )
		{
			prefix[ 0 ] = 0;
			prefixlen = 7;
			inurl = 1;
		}
		else if( *string == '@' )
		{
			// we found a mail address
			char *p = string - 1, *p2 = string + 1;
			while( p >= ( char * )origstring )
			{
				if( *p <= ' ' || ( !isalnum( *p ) && *p != '.' && *p != '-' && *p != '_' ) )
				{
					p++;
					break;
				}
				p--;
			}
			if( p < ( char * )origstring )
				p = origstring;
			if( ( char * )string > p && *p != '!' )
			{
				while( *p2 > ' ' && !isspace( *p2 ) )
				{
					if( *p2 == '*' || *p2 == '@' )
					{
						screwit = TRUE;
						break;
					}

					if( *p2++ == '.' )
					{
						string = p;
						inurl = 2;
						strcpy( prefix, "mailto:" );
						prefixlen = 0;
						dots = 2;
						break;
					}
				}
			}
		}

		if( !inurl )
		{
			string++;
			continue;
		}

		for( c = 0; c < sizeof( buffer ) - 1; c++ )
		{
			if( string[ c ] < ' ' ||
				isspace( string[ c ] ) ||
				string[ c ] > 127
			)
			break;
			buffer[ c ] = string[ c ];
			if( buffer[ c ] == '.' )
				dots++;
		}
		buffer[ c ] = 0;

		// check whether last char in buffer
		// is punctuation or non-url-char

		while( c ) switch( buffer[ c - 1 ] )
		{
			case '.':
				dots--;
			case ',':
			case '"':
			case '\'':
			case '?':
			case '!':
			case '>':
			case ')':
			case ']':
			case '»':
				buffer[ --c ] = 0;
				break;				
			default:
				goto nopunct;
		}
		nopunct:

		if( c <= prefixlen || screwit || (!prefixlen && dots < 2 ) )
		{
			string++;
			continue;
		}

		if( !pool )
			pool = CreatePool( 0, 512, 256 );
		if( !pool )
			return( 0 );

		urllist = AllocPooled( pool, sizeof( *urllist ) + c + strlen( prefix ) + 1 );
		if( !urllist )
			break;

		urllist->next = last;
		urllist->len = c;
		urllist->offset = string - beginstr;
		strcpy( urllist->url, prefix );
		strcat( urllist->url, buffer );

		last = urllist;

		string += c;
	}

	if( pool )
		urllist->private = pool;

	return( urllist );
}

void ASM SAVEDS VAT_FreeURLList( __reg( a0, struct VATS_URL *urllist ) )
{
	DB( ( "called\n" ) );

	if( urllist )
		DeletePool( urllist->private );
}
