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
** $Id: sendmailwin.c,v 1.47 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "classes.h"
#include "prefs.h"
#include "voyager_cat.h"
#include "copyright.h"
#include "network.h"
#include "mui_func.h"
#include "time_func.h"
#include "malloc.h"
#include "menus.h"
#include "init.h"
#include "win_func.h"
#include "dos_func.h"
#include "textinput.h"

#ifdef MBX
#define OPENMEANETBASE 1
#ifdef OPENMEANETBASE
#define NetBase MailNetBase

NETBASE;
#endif
#else
static struct Library *SocketBase;
#endif

static int smtpcomm( int s, char *sendstring, int reply1, int reply2 )
{
	char buffer[ 512 ];

	tickapp();

	if( sendstring )
	{
		sprintf( buffer, "%s\r\n", sendstring );
		
		if( send( s, buffer, strlen( buffer ), 0 ) == -1 )
		{
			strcpy( buffer, "(closed link)" );
			goto failed;
		}
	}

	buffer[ 0 ] = 0;

	for(;;)
	{
		int val;
		int cnt;

		for( cnt = 0; cnt < sizeof( buffer ) - 1; )
		{
			UBYTE ch;

			if( recv( s, &ch, 1, 0 ) != 1 )
			{
				strcpy( buffer, "(closed link)" );
				goto failed;
			}

			//Printf( "read: %lc\n", ch );

			if( ch == 10 )
				break;

			if( ch == 13 )
				continue;

			buffer[ cnt++ ] = ch;

		}

		buffer[ cnt ] = 0;

		tickapp();

		if( buffer[ 3 ] == '-' )
			continue;

		val = atoi( buffer );
		if( val == reply1 || val == reply2 )
			return( 0 );
		else
			break;
	}

failed:
	MUI_Request( app, NULL, 0, GS( ERROR ), GS( CANCEL ),
		GS( SENDMAILWIN_ACK_FAILED ),
		(ULONG)( sendstring ? ( ULONG )sendstring : ( ULONG )GS( SENDMAILWIN_ACK_NOTHING ) ),
		(ULONG)buffer
	);

	return( 1 );        
}

static void reallysendmail( int s, char *subject, char *to, char *contents )
{
	int specialchars = FALSE;
	char *p;
	char buffer[ 512 ]; // TOFIX!! find out the max length of a line
#if USE_KEYFILES
	extern ULONG serialnumber;
	extern char hostos[ 8 ];
#else
	ULONG serialnumber=0;
#ifdef MBX
	char hostos[]="CaOS";
#else
	char hostos[]="AmigaOS";
#endif
#endif

	for( p = contents; *p ; p++ )
	{
		if( *p >= 127 )
		{
			specialchars++;
			break;
		}
	}

	sprintf( buffer, "From: %s (%s)\r\n", ( STRPTR )getprefs( DSI_NET_MAILADDR ), ( STRPTR )getprefs( DSI_NET_REALNAME ) );
	send( s, buffer, strlen( buffer ), 0 );
	sprintf( buffer, "To: %s\r\n", to );
	send( s, buffer, strlen( buffer ), 0 );
	sprintf( buffer, "Subject: %s\r\n", subject );
	send( s, buffer, strlen( buffer ), 0 );	

	if( getprefs( DSI_NET_ORGANIZATION ) )
	{
		sprintf( buffer, "Organization: %s\r\n", ( STRPTR )getprefs( DSI_NET_ORGANIZATION ) );
		send( s, buffer, strlen( buffer ), 0 );
	}

	sprintf( buffer, "X-Mailer: Voyager/%s " VERSIONSTRING " (http://www.vapor.com/)\r\n", hostos );
	send( s, buffer, strlen( buffer ), 0 );
	sprintf( buffer, "MIME-Version: 1.0\r\n" );
	send( s, buffer, strlen( buffer ), 0 );
	sprintf( buffer, "Message-Id: <%lx.%lx.%lx.%s>\r\n", timev(), (unsigned long)rand() & 0xffff, serialnumber, ( STRPTR )getprefs( DSI_NET_MAILADDR ) );
	send( s, buffer, strlen( buffer ), 0 );
	if( specialchars )
	{
		sprintf( buffer, "Content-Type: text/plain; charset=iso-8859-1\r\nContent-Transfer-Encoding: quoted-printable\r\n" );
		send( s, buffer, strlen( buffer ), 0 );
	}
	else
	{
		sprintf( buffer, "Content-Type: text/plain; charset=us-ascii\r\n" );
		send( s, buffer, strlen( buffer ), 0 );
	}
	sprintf( buffer, "\r\n" );
	send( s, buffer, strlen( buffer ), 0 );

	if( specialchars )
	{
		int chs = 0;

		for( p = contents; *p; p++ )
		{
			if( *p == '\r' )
			{
				continue;
			}
			else if( *p == '\n' )
			{
				sprintf( buffer, "\r\n" );
				send( s, buffer, strlen( buffer ), 0 );
				chs = 0;
			}
			else
			{
				if( chs > 73 )
				{
					sprintf( buffer, "=\r\n" );
					send( s, buffer, strlen( buffer ), 0 );
					chs = 0;
				}
				if( *p != '.' && ( ( *p >= 32 && *p <= 60 ) || ( *p >= 62 && *p <= 126 ) ) )
				{
					send( s, ( UBYTE const * )p, 1, 0 );
					chs++;
				}
				else
				{
					sprintf( buffer, "=%02lx", (unsigned long)( *p ) );
					send( s, buffer, strlen( buffer ), 0 );
					chs += 3;
				}
			}
		}
	}
	else
	{
		char *cdup = strdup( contents ); /* TOFIX */
		for( p = strtok( cdup, "\r\n" ); p; p = strtok( NULL, "\r\n" ) )
		{
			if( *p == '.' )
			{
				send( s, ( UBYTE const * )'.', 1, 0 );
			}
			sprintf( buffer, "%s\r\n", p );
			send( s, buffer, strlen( buffer ), 0 );
		}
		free( cdup );
	}
}

int sendmail( char *subject, char *to, char *contents )
{
	struct sockaddr_in sockadr;
	char buffer[ 256 ];
	int s;
	char *host;
#ifdef MBX
	char *mailserver = "smtp.metabox.de";
#else
	char *mailserver = getprefsstr( DSI_NET_SMTP, "mail" ); /* caching.. */
#endif
	int rc = FALSE;

#ifndef MBX
	if( !*getprefsstr( DSI_NET_MAILADDR, "" ) )
	{
		MUI_Request( app, NULL, 0, GS( ERROR ), GS( CANCEL ), "No mail address configured.\nPlease check your configuration!" );
		
		return( FALSE );
	}
#endif /* !MBX */

#ifndef MBX
	if( !( SocketBase = OpenLibrary( "bsdsocket.library", 3 ) ) )
	{
		displaybeep();
		return( FALSE );
	}
#endif

	set( app, MUIA_Application_Sleep, TRUE );

	/*
	 * Connecting...
	 */
	if( ( s = socket( AF_INET, SOCK_STREAM, 0 ) ) != -1 )
	{
		struct hostent *hent;
		
		hent = ( struct hostent * )gethostbyname( mailserver );

		if( hent && ( host = (char*)hent->h_addr ) )
		{
			memset( &sockadr, 0, sizeof( sockadr ) );
			//sockadr.sin_len = sizeof( struct sockaddr_in );
			memcpy( &sockadr.sin_addr, host, sizeof( struct in_addr ) );
			sockadr.sin_family = AF_INET;
			sockadr.sin_port = 25;
		
			if( connect( s, ( struct sockaddr * )&sockadr, sizeof( struct sockaddr ) ) != -1 )
			{
				if( !smtpcomm( s, NULL, 220, 220 ) )
				{
					sprintf( buffer, "HELO %s", mailserver );
					if( !smtpcomm( s, buffer, 250, 250 ) )
					{
#ifdef MBX
						sprintf( buffer, "MAIL FROM:<domeyer@metabox.de>" ); /* the big boss :)	*/
#else
						sprintf( buffer, "MAIL FROM:<%s>", ( STRPTR )getprefs( DSI_NET_MAILADDR ) );
#endif /* !MBX */
						if( !smtpcomm( s, buffer, 250, 250 ) )
						{
							sprintf( buffer, "RCPT TO:<%s>", to );
							if( !smtpcomm( s, buffer, 250, 250 ) )
							{
								if( !smtpcomm( s, "DATA", 354, 354 ) )
								{
									reallysendmail( s, subject, to, contents );
									if( !smtpcomm( s, ".", 250, 250 ) )
									{
										rc = TRUE;
									}
								}
							}
						}
					}
				}

				CloseSocket( s );
			}
			else
			{
#ifdef MBX
				MUI_Request( app, NULL, 0, GS( ERROR ), GS( CANCEL ), "unable to connect\n", 0 );
#else
				MUI_Request( app, NULL, 0, GS( ERROR ), GS( CANCEL ), "unable to connect\n" );
#endif /* !MBX */
			}
		}
		else
		{
#ifdef MBX
			MUI_Request( app, NULL, 0, GS( ERROR ), GS( CANCEL ), "mailserver host not found\n", 0 );
#else
			MUI_Request( app, NULL, 0, GS( ERROR ), GS( CANCEL ), "mailserver host not found\n" );
#endif /* !MBX */
		}
		CloseSocket( s );
	}
#ifndef MBX
	CloseLibrary( SocketBase );
#endif

	set( app, MUIA_Application_Sleep, FALSE );

	return( rc );
}

#define string(x,y,z) TextinputObject,StringFrame,MUIA_String_MaxLen,y,MUIA_String_Contents,x,MUIA_CycleChain,1,End

static struct MUI_CustomClass *lcc;

struct Data {
	char url[ 256 ];

	APTR str_to, str_subject, tf_text;
};


DECNEW
{
	int c;
	APTR grp;
	struct Data *data;
	APTR dummyobject = MUI_NewObject( MUIC_Area, 0 );
	APTR bt_send, bt_cancel;
	char *str_to, *str_subject, *str_contents;

	if( !( c = get_window_number() ) )
	{
		return( 0 );
	}

#ifdef MBX
#define MUIA_WINDOW_WIDTH MUIA_Window_Width, MUIV_Window_Width_Visible( 70 ),
#define MUIA_WINDOW_HEIGHT MUIA_Window_Height, MUIV_Window_Height_Visible( 60 ),
#else
#define MUIA_WINDOW_WIDTH
#define MUIA_WINDOW_HEIGHT
#endif

	obj = DoSuperNew( cl, obj, 
		MUIA_Window_ScreenTitle, (ULONG)copyright,
		MUIA_Window_ID, MAKE_ID('P','N','M', c),
		MUIA_Window_RootObject, dummyobject,
		MUIA_Window_Activate, FALSE,
		MUIA_WINDOW_WIDTH
		MUIA_WINDOW_HEIGHT
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
	{
		D( db_mail, bug( "nah!\n" ));
		return( (ULONG)NULL );
	}

	data = INST_DATA( cl, obj );

	str_to = (STRPTR)GetTagData( MA_PostWin_To, (ULONG)"", msg->ops_AttrList );
	str_subject = (STRPTR)GetTagData( MA_PostWin_Subject, (ULONG)"", msg->ops_AttrList );
	str_contents = (STRPTR)GetTagData( MA_PostWin_Contents, (ULONG)"", msg->ops_AttrList );

	grp = VGroup, 
		Child, ColGroup( 2 ),
			Child, Label2( GS( SENDMAILWIN_TO ) ),
			Child, data->str_to = TextinputObject,
				StringFrame,
				MUIA_String_MaxLen, 1024,
				MUIA_CycleChain, 1,
				MUIA_Textinput_Contents, str_to,
			End,
			Child, Label2( GS( SENDMAILWIN_SUBJECT ) ),
			Child, data->str_subject = TextinputObject,
				StringFrame,
				MUIA_String_MaxLen, 200,
				MUIA_CycleChain, 1,
				MUIA_Textinput_Contents, str_subject,
			End,
		End,

		Child, data->tf_text = TextinputscrollObject,
			MUIA_Textinput_Multiline, TRUE,
			StringFrame,
			MUIA_CycleChain, 1, 
		End,

		Child, HGroup,
			Child, bt_send = SimpleButton( GS( SENDMAILWIN_SEND ) ),
			Child, bt_cancel = SimpleButton( GS( SENDMAILWIN_CANCEL ) ),
		End,
	End;

	if( !grp )
	{
		MUI_Request( app, NULL, 0, copyright, GS( CANCEL ), GS( WIN_FAILED ), MUI_Error() );
		closestuff();
		exit( 20 );
	}

	DoMethod( data->tf_text, MUIM_Textinput_AppendText, str_contents, -1 );

	set( obj, MUIA_Window_RootObject, grp );
	MUI_DisposeObject( dummyobject );

	add_window_menu( obj, GS( SENDMAILWIN_WINTITLE ), c );

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Win_Kill
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_Win_Kill
	);

	DoMethod( bt_send, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 4, MUIM_Application_PushMethod, obj, 1, MM_PostWin_Send
	);

	set( bt_send, MUIA_CycleChain, 1 );
	set( bt_cancel, MUIA_CycleChain, 1 );

	return( (ULONG)obj );
}

DECDISPOSE
{
	remove_window_menu( obj );

	return( DOSUPER );
}

DECMETHOD( Win_Kill, APTR )
{
	set_window_close( obj );

	DoMethod( app, MM_App_CheckWinRemove );

	return( 0 );
}

extern int sendmail( char *subject, char *to, char *contents );
DECMETHOD( PostWin_Send, APTR )
{
	GETDATA;
	int success = FALSE;

#ifdef OPENMEANETBASE
	if( !MailNetBase )
	{
		MailNetBase = (NetData_p) OpenModule( NETNAME, NETVERSION );
		if( !MailNetBase )
			return( FALSE );
	}
#endif

	set( app, MUIA_Application_Sleep, TRUE );

	success = sendmail( getstrp( data->str_subject ), getstrp( data->str_to ), getstrp( data->tf_text ) );

	set( app, MUIA_Application_Sleep, FALSE );

	if( success )
		DoMethod( obj, MM_Win_Kill );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Win_Kill )
DEFMETHOD( PostWin_Send )
ENDMTABLE

int create_postmailwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "PostmailWinClass";
#endif
	return( TRUE );
}

void delete_postmailwinclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
#ifdef OPENMEANETBASE
	if( MailNetBase )
		CloseModule( (Module_p)MailNetBase );
#endif
}

static APTR getpostmailwinclass( void )
{
	return( lcc->mcc_Class );
}


/*
 * Opens a new mail composer window
 */
void createmailwin( char *to, char *subject, char *contents )
{
	APTR o;
	char *sigbuf = 0;
	int usesig = FALSE;
#if USE_DOS
	BPTR fh, lock;
	D_S( struct FileInfoBlock, fib );
#endif /* USE_DOS */
	
	set( app, MUIA_Application_Sleep, TRUE );
	
#if USE_DOS
	if( lock = Lock( getprefsstr( DSI_NET_SIG, "S:.signature" ), MODE_OLDFILE ) )
	{
		if( Examine( lock, fib ) == DOSTRUE && ( sigbuf = malloc( fib->fib_Size + strlen( contents ) + 1 ) ) )
		{
			if( fh = OpenFromLock( lock ) )
			{
				memset( sigbuf, '\0', fib->fib_Size + strlen( contents ) + 1 );
				strcpy( sigbuf, contents );
				if( Read( fh, sigbuf + strlen( contents ), fib->fib_Size ) == fib->fib_Size )
				{
					usesig = TRUE;
				}
				Close( fh );
			}
		}
		else
		{
			UnLock( lock );
		}
	}
#endif /* USE_DOS */
	
	if( !( o = NewObject( getpostmailwinclass(), NULL, MA_PostWin_To, to, MA_PostWin_Subject, subject, MA_PostWin_Contents, usesig ? sigbuf : contents, TAG_DONE ) ) )
	{
		set( app, MUIA_Application_Sleep, FALSE );
		MUI_Request( app, NULL, 0, copyright, GS( CANCEL ), GS( WIN_FAILED ), MUI_Error() );
		closestuff();
		exit( 20 );
	}
	DoMethod( app, OM_ADDMEMBER, o );
	set( app, MUIA_Application_Sleep, FALSE );
	set( o, MUIA_Window_Open, TRUE );

	if( sigbuf )
	{
		free( sigbuf );
	}
}
#endif /* USE_NET */
