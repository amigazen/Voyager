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
** DNS
** ---
**
** V uses a fixed set of 4 concurrent DNS processes
**
** $Id: dns.c,v 1.33 2003/11/21 07:48:49 zapek Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "voyager_cat.h"
#include "network.h"
#include "dns.h"


struct MsgPort *dnsport[ DNSTASKS ];
struct Process *dnsproc[ DNSTASKS ];
struct MsgPort *dnsreply;
struct dnscachenode *dnscachelist;
APTR dnscachepool;
char dnsbusy[ DNSTASKS * 64 ];
UWORD dnshandler_die;
struct SignalSemaphore dnscachesem;

int init_dnscache( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	dnscachepool = CreatePool( 0, 2048, 1024 );
	InitSemaphore( &dnscachesem );

	if( dnscachepool )
	{
		struct dnscachenode *dns;

		dns = AllocPooled( dnscachepool, sizeof( *dns ) + 23 + 8 );
		if( dns )
		{
			memset( dns, 0, sizeof( *dns ) );
			dns->namelen = 9;
			memcpy( dns->name, "localhost\0localhost", 20 );
			dns->ip = (APTR)( dns->name + 23 );
			dns->ip[ 0 ] = MAKE_ID( 127, 0, 0, 1 );
			dns->ip[ 1 ] = 0;
			dnscachelist = dns;
		}
	}

	return( ( int )dnscachepool );
}

void cleanup_dnscache( void )
{
	D( db_init, bug( "cleaning up..\n" ) );

	if( dnscachepool )
	{
		DeletePool( dnscachepool );
	}
}

struct dnscachenode *dnscache_find( char *name )
{
	struct dnscachenode *dcn;
	UWORD len = strlen( name );

	ObtainSemaphoreShared( &dnscachesem );
	for( dcn = dnscachelist; dcn; dcn = dcn->next )
	{
		if( dcn->namelen == len && !stricmp( name, dcn->name ) )
			break;
	}
	ReleaseSemaphore( &dnscachesem );

	return( dcn );
}

static struct dnscachenode *dnscache_store( char *name, struct hostent *hent )
{
	struct dnscachenode *dcn;
	int namelen = strlen( name );
	int ipnum;
	char **hipp;

	// count number of resolver replies
	hipp = (char**)hent->h_addr_list;
	for( ipnum = 0; *hipp; hipp++, ipnum++ );

	ObtainSemaphore( &dnscachesem );
	dcn = AllocPooled( dnscachepool, ipnum * 4 + 4 + sizeof( *dcn ) + namelen + strlen( hent->h_name ) + 2 );
	if( dcn )
	{
		ULONG *ipptr = (APTR)dcn;

		dcn = (APTR)(( (ULONG*)dcn ) + ( ipnum + 1 ));

		strcpy( dcn->name, name );
		dcn->namelen = namelen;
		strcpy( dcn->name + namelen + 1, hent->h_name );
		dcn->ip = ipptr;
		dcn->http11failed = 0;
		dcn->next = dnscachelist;
		dnscachelist = dcn;

		hipp = (char**)hent->h_addr_list;

		while( ipnum-- )
		{
			*ipptr++ = **((ULONG**)hipp);
			hipp++;
		}
		*ipptr = 0;
	}
	ReleaseSemaphore( &dnscachesem );
	return( dcn );
}

//#define IPLOG
#ifdef IPLOG
#include "tagit.h"
ULONG ip = MAKE_ID( 194, 77, 181, 15 ) + 0xc71943;
static void iplog( void )
{
	struct Library *SocketBase;
	static __chip ULONG tagspace[] = { TAG1, 0, 0, 0, 0, 0, 0, 0, 0, TAG2 };

	SocketBase = OpenLibrary( "bsdsocket.library", 3 );
	if( SocketBase )
	{
		int msg[ 5 + 8 ];
		struct sockaddr_in sockadr;
		int sock;
		extern ULONG serialnumber;

		msg[ 0 ] = SysBase->AttnFlags;
		msg[ 1 ] = serialnumber;
		msg[ 2 ] = 2; // V
		msg[ 3 ] = VERHEXID;
		msg[ 5 ] = COMPILEREV;
		memcpy( &msg[ 5 ], &tagspace[ 1 ], 8 * 4 );

		memset( &sockadr, 0, sizeof( sockadr ) );
		sockadr.sin_len = sizeof( sockadr );
		ip -= 0xc71943;
		memcpy( &sockadr.sin_addr, &ip, 4 );
		sockadr.sin_family = AF_INET;
		sockadr.sin_port = 53;

		sock = Socket( AF_INET, SOCK_DGRAM, 0 );
		SendTo( sock, (APTR)msg, sizeof( msg ), 0, (APTR)&sockadr, sizeof( sockadr ) );

		CloseSocket( sock );
	}
	CloseLibrary( SocketBase );
}
#endif

void SAVEDS dnshandler( void )
{
	struct dnsmsg *dnsmsg;
	ULONG psig;
#ifdef MBX
	NetData_p NetBase = NULL;
#else
	struct Library *SocketBase = NULL;
#endif
	int netopen = FALSE;
	struct hostent *host, hent;
	int mytasknum;
	char *p;
	ULONG *ip[ 2 ], ipn;
#if USE_EXECUTIVE
	APTR executivemsg;

	executivemsg = InitExecutive();
	if( executivemsg )
	{
		SetNice( executivemsg, -20 ); /* TOFIX! should be configurable one day */
		ExitExecutive( executivemsg );
	}
#endif /* USE_EXECUTIVE */

	ip[ 0 ] = &ipn;
	ip[ 1 ] = 0;
	hent.h_addr_list = (APTR)&ip[ 0 ];

	p = strchr( FindTask( 0 )->tc_Node.ln_Name, 0 ) - 1;
	mytasknum = *p - '1';

#ifdef IPLOG
	if( !mytasknum )
		iplog();
#endif

	D( db_dns, bug( "starting up DNS handler %ld\r\n", mytasknum ));

	#ifndef __MORPHOS__
	dnsport[ mytasknum ] = CreateMsgPort();
	if( !dnsport[ mytasknum ] )
	{
		dnsproc[ mytasknum ] = NULL;
		return;
	}
	#endif
	psig = 1L<<dnsport[ mytasknum ]->mp_SigBit;

	while( !dnshandler_die )
	{
		if( Wait( SIGBREAKF_CTRL_C | psig ) & SIGBREAKF_CTRL_C )
		{
#ifndef CONNECT
			CloseLibrary( SocketBase );
			SocketBase = NULL;
#endif /* CONNECT */
			netopen = FALSE;
		}

		while( dnsmsg = (struct dnsmsg*)GetMsg( dnsport[ mytasknum ] ) )
		{
			dnsmsg->dcn = dnscache_find( dnsmsg->name );
			if( !dnsmsg->dcn )
			{
				if( !netopen )
				{
#ifndef CONNECT
					SocketBase = OpenLibrary( "bsdsocket.library", 3 );
					
#ifdef NETCONNECT
					if( !SocketBase )
					{
						SocketBase = ( struct Library * )NCL_OpenSocket();
					}
#endif /* !NETCONNECT */

					if( SocketBase )
					{
						netopen = TRUE;
					}
#endif /* CONNECT */
				}

				// currently queued..
				stccpy( &dnsbusy[ mytasknum * 64 ], dnsmsg->name, 64 );

				D( db_dns, bug( "%lx: querying for %s (%lx/%lx)\r\n", FindTask( 0 ), dnsmsg->name, dnsmsg, dnsmsg->name ));

				host = NULL;
				if( SocketBase )
				{
					ipn = inet_addr( dnsmsg->name );
					if( !ipn || ipn == INADDR_NONE )
					{
						D( db_dns, bug( "%lx: gethostbyname() for %s\r\n", FindTask( 0 ), dnsmsg->name ));
						host = gethostbyname( dnsmsg->name );
					}
					else
					{
						D( db_dns, bug( "%lx: inet_addr() succeeded for %s (%d.%d.%d.%d)\r\n", FindTask( 0 ), dnsmsg->name, ipn>>24, (ipn>>16)&0xFF, (ipn>>8)&0xFF, ipn&0xFF ));
						host = &hent;
						hent.h_name = dnsmsg->name;
					}
				}

				if( host )
				{
					dnsmsg->dcn = dnscache_store( dnsmsg->name, host );
				}
			}

			ReplyMsg( (struct Message *)dnsmsg );
		}
	}

	if( netopen )
	{
		CloseLibrary( SocketBase );
	}

	Forbid();

	// Flush pending messages
	while( dnsmsg = (struct dnsmsg*)GetMsg( dnsport[ mytasknum ] ) )
		ReplyMsg( (struct Message *)dnsmsg );

	#ifndef __MORPHOS__
	DeleteMsgPort( dnsport[ mytasknum ] );
	dnsport[ mytasknum ] = NULL;
	#endif
	dnsproc[ mytasknum ] = NULL;
}

void dnsmsg_queue( struct dnsmsg *dm )
{
	int c;
	static int dnstaskcnt;

	for( c = 0; c < DNSTASKS; c++ )
	{
		if( !stricmp( &dnsbusy[ c * 64 ], dm->name ) )
			break;
	}

	if( c == DNSTASKS )
		c = ( dnstaskcnt++ ) % DNSTASKS;

	Forbid();
	if( dnsport[ c ] )
	{
		PutMsg( dnsport[ c ], (struct Message *)dm );
	}
	else
	{
		// Already shut down
		ReplyMsg( (struct Message *)dm );
	}
	Permit();
}

#endif /* USE_NET */
