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
** FTP
** ---
**
** This will probably move to a VPlug one day..
**
** $Id: ftp.c,v 1.35 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "copyright.h"
#include "voyager_cat.h"
#include "network.h"
#include "cache.h"
#include "dns.h"
#include "authipc.h"
#include "ftp.h"
#include "prefs.h"
#include "mime.h"
#include "netinfo.h"
#include "lineread.h"
#include "sur_gauge.h"



/* checks if a line is in EPLF format */
int iseplf( char *line )
{
	if( *line != 43 ) return FALSE;
	while( *line )
	{
		if( *line++ == 9 )
			return TRUE;
	}
	return FALSE;
}


void un_setup_ftp( struct unode *un )
{
	char *host;
	struct dnscachenode *dcn;
	char *p;

	/* convert ascii chars (%20), etc.. */
	while( p = strchr( un->purl.path, '%' ) )
	{
		char hex[ 4 ];
		long v;

		memcpy( hex, p + 1, 2 );
		hex[ 2 ] = 0;
		stch_l( hex, &v );

		*p = (char)v;
		strcpy( p + 1, p + 3 );
	}

	un->retr_mode = RM_FTP;

	sur_gauge_reset( un );

	host = un->purl.host;
	un->port = un->purl.port;

	if( !host )
		host = "undefined";

	D( db_ftp, bug( "setup_ftp(%s) host %s vp %ld\n", un->url, host, un->viaproxy ));

	/* resolve */
	dcn = dnscache_find( host );
	if( dcn )
	{
		un->dcn = dcn;
		un->state = UNS_WAITINGFORNET;
		D( db_ftp, bug( "waiting for net\n" ));
	}
	else
	{
		un->dnsmsg.m.mn_ReplyPort = dnsreply;
		un->dnsmsg.name = host;
		dnsmsg_queue( &un->dnsmsg );
		un->state = UNS_LOOKINGUP;
		sur_text( un, GS( NETST_LOOKINGUP ), host );
	}
}

static void makeftperror( struct unode *un )
{
	un->errorcode = 42;
	sprintf( un->errorstring, GS( NWM_ERROR_SERVERERROR ), un->linereadbuffer );
	makehtmlerror( un );
	exitlineread( un );
	un_netclose( un );
	un->state = UNS_FAILED;
}

static void un_doprotocol_ftp_init( struct unode *un )
{
	if( un->wasparked )
	{
		char buffer[ 256 ];

		sprintf( buffer, "CWD %.240s\r\n", un->ftp_pwd );
		uns_write( un, buffer, -1 );
		un->protocolstate = FPT_REINIT_CWD;
	}
	else
	{
		un->protocolstate = FPT_GOTINIT;
	}

	initlineread( un );
}

/* connected and getting our first line */
static void un_doprotocol_ftp_gotinit( struct unode *un )
{
	char buffer[ 256 ];

	if( atoi( un->linereadbuffer ) != 220 )
	{
		makeftperror( un );
		return;
	}

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		purgeline( un );
		return;
	}

	D( db_ftp, bug( "got initial ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	sur_txt( un, GS( NETST_FTP_SENDING_USERNAME ) );

	purgeline( un );

	/* sending username */
	sprintf( buffer, "USER %.220s\r\n",
		un->purl.username ? un->purl.username : "anonymous"
	);
	D( db_ftp, bug( "doftp: sending %s\n", buffer ));
	uns_write( un, buffer, -1 );

	un->protocolstate = FPT_USERREPLY;
}

static void un_doprotocol_ftp_userreply( struct unode *un )
{
	char buffer[ 256 ], *pw;

	D( db_ftp, bug( "got user ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( atoi( un->linereadbuffer ) == 331 ) // needs password
	{

		if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
		{
			purgeline( un );
			return;
		}
		sur_txt( un, GS( NETST_FTP_SENDING_PASSWORD ) );

		purgeline( un );

		if( un->purl.username && !un->purl.password )
		{
			sprintf( un->authrealm, "FTP-Login \"%s\"", un->purl.username );

			auth_query( &un->purl, un->dcn->name, un->authrealm, FALSE, un->authdata, TRUE );
			pw = un->authdata;
		}
		else if( un->purl.password )
		{
			pw = un->purl.password;
		}
		else if( getprefslong( DSI_SECURITY_NO_MAILADDR, FALSE ) || !*getprefsstr( DSI_NET_MAILADDR, "" ) )
		{
			pw = "voyager@";
		}
		else
			pw = getprefs( DSI_NET_MAILADDR );

		sprintf( buffer, "PASS %.220s\r\n", pw );
		D( db_ftp, bug( "doftp: sending %s\n", buffer ));
		uns_write( un, buffer, -1 );
		un->protocolstate = FPT_PWREPLY;
	}
	else if( atoi( un->linereadbuffer ) == 230 ) // no need to login (famous koobera.math.uic.edu's anonftpd behaviour)
	{
		D( db_ftp, bug("koobera mode\n"));
		purgeline( un );
		
		uns_write( un, "TYPE I\r\n", 8 );
		un->protocolstate = FPT_TYPEREPLY;
	}
	else
	{
		makeftperror( un );
		return;
	}
}

static void un_doprotocol_ftp_reinit_cwd( struct unode *un )
{
	D( db_ftp, bug( "got REINIT ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		purgeline( un );
		return;
	}

	if( atoi( un->linereadbuffer ) != 250 )
	{
		makeftperror( un );
		return;
	}

	purgeline( un );

	// ok
	uns_write( un, "PASV\r\n", 6 );
	un->protocolstate = FPT_PASVREPLY;
}

static void un_doprotocol_ftp_pwreply( struct unode *un )
{
	D( db_ftp, bug( "got pw ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( isdigit( un->linereadbuffer[ 0 ] ) )
		pushreplybuffer( un, &un->linereadbuffer[ 4 ] );
	else
		pushreplybuffer( un, un->linereadbuffer );

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		purgeline( un );
		return;
	}

	if( atoi( un->linereadbuffer ) != 230 )
	{
		makeftperror( un );
		return;
	}

	purgeline( un );

	pushreplybuffer( un, NULL );

	uns_write( un, "TYPE I\r\n", 8 );
	un->protocolstate = FPT_TYPEREPLY;
}

static void un_doprotocol_ftp_typereply( struct unode *un )
{
	D( db_ftp, bug( "got type i ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( atoi( un->linereadbuffer ) != 200 )
	{
		makeftperror( un );
		return;
	}

	//pushreplybuffer( un, &un->linereadbuffer[ 4 ] );

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		purgeline( un );
		return;
	}

	purgeline( un );

	//pushreplybuffer( un, NULL );

	D( db_ftp, bug( "doftp: sending PWD\n" ));
	uns_write( un, "PWD\r\n", 5 );
	un->protocolstate = FPT_PWDREPLY;
}

static void un_doprotocol_ftp_pwdreply( struct unode *un )
{
	char *p;

	D( db_ftp, bug( "got PWD ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( atoi( un->linereadbuffer ) != 257 )
	{
		makeftperror( un );
		return;
	}

	// ok, we store the PWD response
	un->ftp_pwd = unstrdup( un, &un->linereadbuffer[ 4 ] );
	while( *un->ftp_pwd && *un->ftp_pwd != '"' )
		un->ftp_pwd++;
	p = ++un->ftp_pwd;
	while( *p )
	{
		if( *p == '"' )
		{
			if( p[ 1 ] == '"' )
			{
				strcpy( p, p + 1 );
				p++;
			}
			else
			{
				*p = 0;
				break;
			}
		}
		p++;
	}
	D( db_ftp, bug( "stored ftp_pwd '%s'\n", un->ftp_pwd ));

	purgeline( un );

	// ok
	uns_write( un, "PASV\r\n", 6 );
	un->protocolstate = FPT_PASVREPLY;
}

static void un_doprotocol_ftp_preaborreply( struct unode *un )
{
	D( db_ftp, bug( "got preABOR ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	purgeline( un );
	reinitlineread( un );
	un->protocolstate = FPT_ABORREPLY;
}

static void un_doprotocol_ftp_aborreply( struct unode *un )
{
	D( db_ftp, bug( "got postABOR ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	purgeline( un );

	un->protocolstate = FPT_POSTABORREPLY;
}

static void un_doprotocol_ftp_postaborreply( struct unode *un )
{
	D( db_ftp, bug( "got ABOR ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));
	D( db_ftp, bug("un->offset == %lu\n", un->offset));

	purgeline( un );

	uns_write( un, "PASV\r\n", 6 ); // here we come back from a resume so we need to initiate a PASV
	un->protocolstate = FPT_PASVREPLY; 
}

static void un_doprotocol_ftp_pasvreply( struct unode *un )
{
	char buffer[ 256 ], *p;
	int c, vals[ 6 ];
	int rc;
	struct sockaddr_in sockadr;

	D( db_ftp, bug( "got PASV ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( atoi( un->linereadbuffer ) != 227 )
	{
		makeftperror( un );
		return;
	}

	p = strchr( un->linereadbuffer, '(' );
	if( !p )
	{
		p = strchr( un->linereadbuffer, '=' ); // okay, that's anonftpd
		if( !p )
		{
			makeftperror( un );
			return;
		}
	}

	stccpy( buffer, p + 1, sizeof( buffer ) );

	Forbid();
	for( c = 0; c < 6; c++ )
	{
		p = strtok( c ? NULL : buffer, "," );
		if( !p )
		{
			Permit();
			makeftperror( un );
			return;
		}
		vals[ c ] = atoi( p );
	}
	Permit();

	purgeline( un );

	sur_txt( un, GS( NETST_FTP_CONNECTING ) );

	un->ftp_pasvip = MAKE_ID( vals[ 0 ], vals[ 1 ], vals[ 2 ], vals[ 3 ] );
	un->ftp_pasvport = MAKE_ID( 0, 0, vals[ 4 ], vals[ 5 ] );

	D( db_ftp, bug( "ftp passive IP %lx, port %ld\n", un->ftp_pasvip, un->ftp_pasvport ));

	// now connect..

	un->sock_pasv = socket( AF_INET, SOCK_STREAM, 0 );

	sockadr.sin_len = sizeof( sockadr );
	memcpy( &sockadr.sin_addr, &un->ftp_pasvip, 4 );
	sockadr.sin_family = AF_INET;
	sockadr.sin_port = un->ftp_pasvport;
	
	rc = connect( un->sock_pasv, (struct sockaddr*)&sockadr, sizeof( sockadr ) );

	if( rc )
	{
		makeneterror( un, GS( NWM_ERROR_NOCONNECT ), rc );
		un->state = UNS_FAILED;
		un_netclose( un );
		return;
	}

	D( db_ftp, bug( "ftp: pasv sock connected ok\n" ));

	// OK, we're done with that

	// no pathname means it's actually a directory (hehe)
	if( !un->purl.path || !un->purl.path[ 0 ] )
	{
		strcpy( buffer, "LIST\r\n" );
		un->protocolstate = FPT_LISTREPLY;
	}
	else
	{
		// now lets first try to RETR the file
		D( db_ftp, bug("un->offset is == %lu\n", un->offset));
		if( un->offset && un->range != -1 )
		{
			D( db_ftp, bug("sending REST\n"));
			sprintf( buffer, "REST %lu\r\n", un->offset );
			uns_write( un, buffer, -1 );
			un->protocolstate = FPT_RESTREPLY;
			return;
		}
		D( db_ftp, bug("sending RETR\n"));
		sprintf( buffer, "RETR %.240s\r\n",
			un->purl.path
		);
		
		un->protocolstate = FPT_RETRREPLY;
	}

	D( db_ftp, bug( "sending %s", buffer ));

	uns_write( un, buffer, -1 );
}

static void un_doprotocol_ftp_restreply( struct unode *un )
{
	char buffer[ 256 ];

	D( db_ftp, bug( "got REST ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	purgeline( un );

	un->range = -1; // resume is on its way

	sprintf( buffer, "RETR %.240s\r\n", un->purl.path );

	un->protocolstate = FPT_RETRREPLY;
	
	uns_write( un, buffer, -1 );
}

static void un_doprotocol_ftp_listreply( struct unode *un )
{
	D( db_ftp, bug( "got LIST ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( atoi( un->linereadbuffer ) != 150 )
	{
		makeftperror( un );
		return;
	}

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		purgeline( un );
		return;
	}

	sur_txt( un, GS( NETST_FTP_LISTING ) );

	purgeline( un );

	un->sockstategot = 0;
	un->linereadstate = 0;
	dolineread( un );
	if( un->linereadstate )
	{
		un->ftp_got_quick_done = TRUE;
	}

	exitlineread( un );
	initlineread_pasv( un );

	un->protocolstate = FPT_LISTREAD;

	un->state = UNS_READING;

	strcpy( un->mimetype, "text/html" );
	un->tomem = TRUE; // hacky

	pushfmt( un, "<HEAD><TITLE>FTP directory %s</TITLE><BODY><H2>FTP directory <B>%s</B> on <B>%s</B></H2><HR><PRE>",
		un->purl.path, un->purl.path, un->dcn->name
	);

	if( !ISLISTEMPTY( &un->replybufferlist ) )
	{
		struct rbline *rb;

		for( rb = FIRSTNODE( &un->replybufferlist ); NEXTNODE( rb ); rb = NEXTNODE( rb ) )
		{
			if( rb->data )
				pushfmt( un, "%s\n", rb->data );
		}

		pushstring( un, "<HR>" );
	}

	un->doinform = TRUE;

	D( db_ftp, bug( "entering rl\n" ));
}

static void un_doprotocol_ftp_retrreply( struct unode *un )
{
	char buffer[ 256 ], *p;

	D( db_ftp, bug( "got RETR ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		purgeline( un );
		return;
	}

	if( atoi( un->linereadbuffer ) == 150 || atoi( un->linereadbuffer ) == 350 )
	{
		// try to guess the document length from the RETR reply

		un->doclen = -1;
		stccpy( buffer, un->linereadbuffer, sizeof( buffer ) );
		p = strrchr( buffer, '(' );
		if( p )
		{
			int len = atoi( p + 1 );
			if( len >= 0 )
				un->doclen = len;
		}

		exitlineread( un );

		// ok, file sent in progress
		if( !mime_findbyextension( un->purl.path, NULL, NULL, NULL, un->mimetype ) )
			strcpy( un->mimetype, "text/plain" );

		un->state = UNS_WAITING;
		un->protocolstate = FPT_BEGINREADDATA;

		D( db_ftp, bug( "entering wait state, length is %lu\n", un->doclen ));
		
		netinfo_setmax( un->ledobjnum, un->doclen );
	}
	else
	{
		purgeline( un );
		un->protocolstate = FPT_CWDREPLY;

		sprintf( buffer, "CWD %.240s\r\n",
			un->purl.path
		);
		D( db_ftp, bug( "sending %s", buffer ));

		uns_write( un, buffer, -1 );
	}
}

static void un_doprotocol_ftp_beginreaddata( struct unode *un )
{
	int rc;

	D( db_ftp, bug("offset is == %lu, un->range == %lu\n", un->offset, un->range));
	if( un->offset && un->range != -1 ) // try to resume
	{
		/* close the PASV socket first */
		if( un->sock_pasv >= 0 )
		{
			CloseSocket( un->sock_pasv );
			
			un->sock_pasv = -1;
		}

		/* sending ABOR in OOB */
		rc = send( un->sock, "ABOR\r\n", 6, MSG_OOB );

		D( db_ftp, bug("trying to resume (rc == %ld)\n", rc));
		
		un->protocolstate = ( rc < 0 ) ? FPT_INIT : FPT_PREABORREPLY;
		
		return;
	}

	un->protocolstate = FPT_READDATA;
	un->sockstate = SSW_PSR;
}

static void un_doprotocol_ftp_readdata( struct unode *un )
{
	int rc = 0;

	D( db_ftp, bug("now reading some data, un->sockstategot == %lu\n", un->sockstategot));
	
	if( un->sockstategot & SSW_PSR )
	{
		rc = uns_pread( un, un->linereadbuffer, LINEREADBUFFERSIZE );
		
		if( rc <= 0 )
		{
			// end of data
			if( un->doclen <= 0 )
			{
				/*
				 * Unknown size, we assume end of stream and
				 * we do save the file properly.
				 */
				if( gp_keepftp )
				{
					un->protocolstate = FPT_DOPARK;
				}
				else
				{
					un->state = UNS_DONE;
					un_netclose( un );
				}
				
				un->doinform = TRUE;
				sur_gauge_clear( un );
				un->keepinmem = TRUE;
				
				if( !un->nocache )
					un_memtocache( un );

				return;
			}

			// FIXME!
			un_netclose( un );
			un->state = UNS_FAILED;
			un->sockstate = 0;
			return;
		}
	}
	else if( !un->stalled )
	{
		// not flagged
		// check for stall
		if( now - un->lastdata >= 5 )
		{
			un->informptr = un->docptr + 2048;
			un->stalled = TRUE;
			un->doinform = TRUE;
			sur_led( un, 4 );
			sur_gauge_report( un );
			return;
		}
	}

	// push data
	if( !rc )
		return; // nothing to push, really

	pushdata( un, un->linereadbuffer, rc );

	if( un->docptr > un->informptr )
	{
		un->informptr = un->docptr + 2048;
		un->doinform = TRUE;
	}

	if( un->stalled )
	{
		un->stalled = FALSE;
		sur_led( un, 3 ); // reading again
	}
	un->lastdata = now;

	sur_gauge_report( un );

	if( un->doclen > 0 && un->docptr >= un->doclen )
	{
		D( db_ftp, bug("done(%s)\n", un->url ));
		// done
		if( gp_keepftp )
		{
			un->protocolstate = FPT_DOPARK;
		}
		else
		{
			un_netclose( un );
			un->state = UNS_DONE;
		}
		un->doinform = TRUE;
		sur_gauge_clear( un );
		un->keepinmem = TRUE;

		// write to cache..
		if( !un->nocache )
			un_memtocache( un );
	}
}

static void un_doprotocol_ftp_cwdreply( struct unode *un )
{
	D( db_ftp, bug( "got RETR ftp reply %s rc %ld\n", un->linereadbuffer, un->linereadstate ));

	if( un->linereadbuffer[ 3 ] == '-' || !isdigit( un->linereadbuffer[ 0 ] ) )
	{
		if( isdigit( un->linereadbuffer[ 0 ] ) )
			pushreplybuffer( un, &un->linereadbuffer[ 4 ] );
		else
			pushreplybuffer( un, un->linereadbuffer );
		purgeline( un );
		return;
	}

	if( atoi( un->linereadbuffer ) != 250 )
	{
		makeftperror( un );
		return;
	}


	purgeline( un );
	pushreplybuffer( un, NULL );

	// now, do the list

	uns_write( un, "LIST\r\n", -1 );
	un->protocolstate = FPT_LISTREPLY;
}

static void un_doprotocol_ftp_dopark( struct unode *un );
static void un_doprotocol_ftp_listread( struct unode *un )
{
	char buffer[ 512 ];
	char urlbuf[ 514 ];
	char dots[ 40 ], *p, *filename, *p2, x[ 8 ], sizespec = 'K';
	time_t t;
	int size, asize;
	struct tm tm, tmnow;

	if( un->linereadstate < 0 )
	{
		pushfmt( un, "<HR></PRE><FONT SIZE=-1>Total %ld directories, %ld files (%ldK)" INTERNALDOC,
			un->ftp_dircount, un->ftp_filecount,
			( un->ftp_totalfilesize + 1023 ) / 1024
		);
		D( db_ftp, bug( "done reading LIST\n" ));
		exitlineread( un );
		if( gp_keepftp )
		{
			if( un->ftp_got_quick_done )
			{
				un_doprotocol_ftp_dopark( un );
			}
			else
			{
				un->protocolstate = FPT_DOPARK;
				initlineread( un );
			}
		}
		else
		{
			un_netclose( un );
			un->state = UNS_DONE;
		}

		sur_gauge_clear( un );
		un->keepinmem = TRUE;
		return;
	}

	D( db_ftp, bug( "got %s\n", un->linereadbuffer ));

	stccpy( buffer, un->linereadbuffer, sizeof( buffer ) );
	purgeline( un );

	/* check for EPLF response first */
	if( iseplf(buffer) )
	{
		/*
		 * Heaven.
		 */
		int flagcwd = 0;
		time_t when = 0;
		int flagsize = 0;
		unsigned long size = 0;
		char *line = (char *)&buffer;

		if( *line++ != '+' )
			return;

		while( *line )
		{
			switch( *line )
			{
				case 's':
					flagsize = 1;
					size = 0;
					while (*++line && (*line != ','))
						size = size * 10 + (*line - '0');
					break;
				
				case 'm':
					while (*++line && (*line != ','))
						when = when * 10 + (*line - '0');
					break;
 
				case '\t': // last char
					if( strlen( line + 1 ) < 39 )
					{
						memset( dots, '.', 39 );
						dots[ 39 - strlen( line + 1 ) ] = 0;
					}
					else
						dots[ 0 ] = 0;
 
					if( flagcwd )
					{
						/* directory */
						pushfmt( un, "<IMG SRC=\"internal-gopher-menu\" WIDTH=\"13\" HEIGHT=\"17\" ALT=\"[DIR]\"> <A HREF=\"%s/\">%s</A>%s  [%24.24s]\n",
								line + 1, line + 1, dots, ctime( &when ) );
						un->ftp_dircount++;
					}
					else
					{
						/* file */
						char mimeb[ 128 ], *mime;

						if( !mime_findbyextension( line + 1, NULL, NULL, 0, mimeb ) )
						{
							mime = "unknown";
						}
						else
						{
							if( !strnicmp( mimeb, "application/", 12 ) )
								mime = "bin";
							else if( !strnicmp( mimeb, "text/", 5 ) )
								mime = "text";
							else if( !strnicmp( mimeb, "image/", 6 ) )
								mime = "image";
							else if( !strnicmp( mimeb, "audio/", 6 ) )
								mime = "audio";
							else if( !strnicmp( mimeb, "video/", 6 ) )
								mime = "video";
							else
								mime = "unknown";
						}

						pushfmt( un, "<IMG SRC=\"internal-gopher-%s\" WIDTH=\"13\" HEIGHT=\"17\" ALT=\"[DIR]\"> <A HREF=\"%s\">%s</A>%s  [%24.24s] %5ldK\n",
							mime, line + 1, line + 1, dots, ctime( &when ), ( size + 1023 ) / 1024
						);
						un->ftp_filecount++;
						un->ftp_totalfilesize += size;
					}
					return;
				
				case '/': // directory
					flagcwd = 1;
				default:
					while (*line) if (*line++ == ',') break;
			}
		}

		return;

	}

	/*
	 * Hell.
	 */
	if( strlen( buffer ) < 30 )
		return;

	/* softlinks */
	p = strstr( buffer, "->" );
	if( p )
	{
		p--;
		*p-- = 0;
		while( ( !isspace( *p ) || ( isspace( *p ) && !isdigit( *( p - 1 ) ) ) ) && p >= buffer )
			p--;
	}
	else
	{
		p = strrchr( buffer, ' ' );

		if ( p )
		{
			while( ( !isspace( *p ) || ( isspace( *p ) && !isdigit( *( p - 1 ) ) ) ) && p >= buffer )
				p--;
		}
	}

	/*
	 * Now p should point to a filename, with or
	 * without space. The character preceding the
	 * space MUST be a char ( yeah, ftp list sucks )
	 */
	p = stpblk( p );

	if( !p )
		return;

	filename = p;

	if( strchr( filename, '/' ) ) // we just want the filename, not the path
		filename = strchr( filename, '/') + 1;

	strcpy( urlbuf, un->url );
	if ( urlbuf[ strlen( urlbuf ) - 1 ] != '/' )  // and the URL must end with a /
		strcat( urlbuf, "/" );

	if( !strcmp( filename, "." ) || !strcmp( filename, ".." ) )
		return;

	p -= 13;
	p2 = p - 1;
	while( isspace( *p2 ) && p2 > buffer )
		p2--;
	while( !isspace( *p2 ) && p2 > buffer )
		p2--;

	size = atoi( p2 );

	time( &t );
	tm = *localtime( &t );
	tmnow = tm;

	if( !strnicmp( p, "Jan", 3 ) )
		tm.tm_mon = 0;
	else if( !strnicmp( p, "Feb", 3 ) )
		tm.tm_mon = 1;
	else if( !strnicmp( p, "Mar", 3 ) )
		tm.tm_mon = 2;
	else if( !strnicmp( p, "Apr", 3 ) )
		tm.tm_mon = 3;
	else if( !strnicmp( p, "May", 3 ) )
		tm.tm_mon = 4;
	else if( !strnicmp( p, "Jun", 3 ) )
		tm.tm_mon = 5;
	else if( !strnicmp( p, "Jul", 3 ) )
		tm.tm_mon = 6;
	else if( !strnicmp( p, "Aug", 3 ) )
		tm.tm_mon = 7;
	else if( !strnicmp( p, "Sep", 3 ) )
		tm.tm_mon = 8;
	else if( !strnicmp( p, "Oct", 3 ) )
		tm.tm_mon = 9;
	else if( !strnicmp( p, "Nov", 3 ) )
		tm.tm_mon = 10;
	else if( !strnicmp( p, "Dec", 3 ) )
		tm.tm_mon = 11;

	tm.tm_mday = atoi( p + 4 );
	p += 7;

	if( strchr( p, ':' ) )
	{
		tm.tm_hour = atoi( p );
		tm.tm_min = atoi( p + 3 );

		// Check weather date is actually from last year
		if( tm.tm_mon > tmnow.tm_mon || 
			( tm.tm_mon == tmnow.tm_mon && tm.tm_mday > tmnow.tm_mday )
		)
			tm.tm_year--;
	}
	else
	{
		if( atoi( p ) < 30 )
			tm.tm_year = atoi( p ) + 30;
		else if( atoi( p ) < 100 )
			tm.tm_year = atoi( p ) - 70;
		else
			tm.tm_year = atoi( p ) - 1900;
	}

	t = mktime( &tm );
	utunpk( t, x );

	// we've now got everything together
	if( strlen( filename ) < 39 )
	{
		memset( dots, '.', 39 );
		dots[ 39 - strlen( filename ) ] = 0;
	}
	else
		dots[ 0 ] = 0;

	if( buffer[ 0 ] == 'd' )
	{
		pushfmt( un, "<IMG SRC=\"internal-gopher-menu\" WIDTH=\"13\" HEIGHT=\"17\" ALT=\"[DIR]\"> <A HREF=\"%s%s/\">%s</A>%s  [%02ld.%02ld.%04ld]\n",
			urlbuf, filename, filename, dots, x[ 2 ], x[ 1 ], x[ 0 ] + 1970
		);
		un->ftp_dircount++;
	}
	else
	{
		char mimeb[ 128 ], *mime;

		if( !mime_findbyextension( filename, NULL, NULL, 0, mimeb ) )
		{
			mime = "unknown";
		}
		else
		{
			if( !strnicmp( mimeb, "application/", 12 ) )
				mime = "bin";
			else if( !strnicmp( mimeb, "text/", 5 ) )
				mime = "text";
			else if( !strnicmp( mimeb, "image/", 6 ) )
				mime = "image";
			else if( !strnicmp( mimeb, "audio/", 6 ) )
				mime = "audio";
			else if( !strnicmp( mimeb, "video/", 6 ) )
				mime = "video";
			else
				mime = "unknown";
		}

		asize = ( size + 1023 ) / 1024;
		if( asize > 99999 )
		{
			asize = ( asize + 1023 ) / 1024;
			if( asize < 100000 )
			{
				sizespec = 'M';
			}
			else
			{
				asize = ( asize + 1023 ) / 1024;
				sizespec = 'G';
			}
		}

		pushfmt( un, "<IMG SRC=\"internal-gopher-%s\" WIDTH=\"13\" HEIGHT=\"17\" ALT=\"[DIR]\"> <A HREF=\"%s%s\">%s</A>%s  [%02ld.%02ld.%04ld] %5ld%c\n",
			mime, urlbuf, filename, filename, dots, x[ 2 ], x[ 1 ], x[ 0 ] + 1970, asize, sizespec
		);

		un->ftp_filecount++;
		un->ftp_totalfilesize += size;
	}

}

static void un_doprotocol_ftp_dopark( struct unode *un )
{
	D( db_ftp, bug( "dopark got %s\n", un->linereadbuffer ));

	sur_gauge_clear( un );

	if( !un->ftp_got_quick_done && ( atoi( un->linereadbuffer ) != 226 ) )
	{
		un_netclose( un );
		un->state = UNS_DONE;
		return;
	}

	// close passive socket
	if( un->sock_pasv >= 0 )
	{
		CloseSocket( un->sock_pasv );
		un->sock_pasv = -1;
	}
	// park server connection
	un_netpark( un );
	un->state = UNS_DONE;
	exitlineread( un );
}

void un_doprotocol_ftp( struct unode *un )
{
	D( db_ftp, bug( "doprot_ftp(%s) ps == %ld, ns == %ld\n", un->url, un->protocolstate, un->state ));
	while( un->state == UNS_CONNECTED || un->state == UNS_READING )
	{
		if( un->lineread && un->protocolstate != FPT_DOPARK ) /* TOFIX: kludge, could be better */
		{
			dolineread( un );
			if( !un->linereadstate )
			{
				D( db_ftp, bug( "nothing to read.. returning\n" ) );
				return; // didn't get no data
			}

			if( un->linereadstate < 0 && un->lineread == 1 )
			{
				int rc;

				rc = Errno(  );
				
				// header error?!?
				makeneterror( un, GS( NWM_ERROR_NODATA ), rc );
				un->state = UNS_FAILED;
				exitlineread( un );
				sur_led( un, 0 );
				return;
			}

			// "Microsoft"-Workaround
			while( isspace( un->linereadbuffer[ 0 ] ) )
				strcpy( &un->linereadbuffer[ 0 ], &un->linereadbuffer[ 1 ] );
		}

		switch( un->protocolstate )
		{
			case FPT_INIT: // wait for initial reply from server
				un_doprotocol_ftp_init( un );
				break;

			case FPT_GOTINIT: // we got the initial reply..
				un_doprotocol_ftp_gotinit( un );
				break;

			case FPT_USERREPLY: // USER reply
				un_doprotocol_ftp_userreply( un );
				break;

			case FPT_PWREPLY: // PASS reply
				un_doprotocol_ftp_pwreply( un );
				break;

			case FPT_PWDREPLY: // PWD reply
				un_doprotocol_ftp_pwdreply( un );
				break;

			case FPT_PREABORREPLY: // this one just reads unusefull data
				un_doprotocol_ftp_preaborreply( un );
				break;
			
			case FPT_ABORREPLY: // possible ABOR reply (used for resume)
				un_doprotocol_ftp_aborreply( un );
				break;

			case FPT_POSTABORREPLY:
				un_doprotocol_ftp_postaborreply( un );
				break;

			case FPT_PASVREPLY: // Initial PASV reply
				un_doprotocol_ftp_pasvreply( un );
				break;

			case FPT_RESTREPLY: // Restore (resume)
				un_doprotocol_ftp_restreply( un );
				break;
			
			case FPT_RETRREPLY: // RETR reply
				un_doprotocol_ftp_retrreply( un );
				return;

			case FPT_LISTREPLY: // LIST reply
				un_doprotocol_ftp_listreply( un );
				return;

			case FPT_LISTREAD:
				un_doprotocol_ftp_listread( un );
				return;

			case FPT_CWDREPLY: // CWD reply
				un_doprotocol_ftp_cwdreply( un );
				return;

			case FPT_BEGINREADDATA: // begin
				un_doprotocol_ftp_beginreaddata( un );
				return;

			case FPT_READDATA: // actually read
				un_doprotocol_ftp_readdata( un );
				return;

			case FPT_TYPEREPLY: // TYPE I reply
				un_doprotocol_ftp_typereply( un );
				break;

			case FPT_REINIT_CWD: // reinit reply
				un_doprotocol_ftp_reinit_cwd( un );
				break;

			case FPT_DOPARK: // eat up final reply to get ready for parking
				un_doprotocol_ftp_dopark( un );
				break;

			default:
				return;
		}
	}
}

#endif /* USE_NET */
