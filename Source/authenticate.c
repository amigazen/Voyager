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
** Authentication class
** --------------------
** - Handles authentication requests
**
** © 1999 VaporWare CVS team <ibcvs@vapor.com>
** All rights reserved
**
** $Id: authenticate.c,v 1.41 2003/07/06 16:51:32 olli Exp $
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
#include "copyright.h"
#include "prefs.h"
#include "classes.h"
#include "authipc.h"
#include "cookies.h"
#include "certs.h"
#include "splashwin.h"
#include "mui_func.h"
#include "malloc.h"
#include "cache.h"
#include "init.h"
#include "dos_func.h"
#include "textinput.h"


static struct MinList authreqlist;
struct MsgPort *authport;
ULONG authportsigmask;
struct MinList authlist;  // list containing all the authentication (authnodes)
struct SignalSemaphore authlistsem;

extern APTR win_authbrowser;

struct Data {
	struct MinNode n;
	APTR me;
	struct MinList aumlist; // auth request message trigger requester
	char server[ 256 ];
	char realm[ 256 ];
	char *user;
	APTR str_user;
	APTR str_pw;
	APTR chk_pw;
	int ftpmode;
};


static int base64( int iv )
{
	return( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[ iv ] );
}

static void copybase64( char *from, char *to )
{
	char buffer[ 4 ], outbuff[ 4 ];
	int rc;

	while( *from )
	{
		rc = min( strlen( from ), 3 );
		if( !rc )
			break;

		memcpy( buffer, from, rc );
		from += rc;

		if( rc < 2 )
			buffer[ 1 ] = 0;
		if( rc < 3 )
			buffer[ 2 ] = 0;

		outbuff[ 0 ] = base64( buffer[ 0 ] >> 2 );
		outbuff[ 1 ] = base64( ( ( buffer[ 0 ] & 3 ) << 4 ) | ( buffer[ 1 ] >> 4 ) );
		outbuff[ 2 ] = base64( ( ( buffer[ 1 ] & 0xf ) << 2 ) | ( buffer[ 2 ] >> 6 ) );
		outbuff[ 3 ] = base64( buffer[ 2 ] & 0x3f );

		if( rc < 2 )
			outbuff[ 2 ] = '=';
		if( rc < 3 )
			outbuff[ 3 ] = '=';

		memcpy( to, outbuff, 4 );
		to += 4;
	}
	*to = 0;
}


/*
 * Finds if there's a matching auth on the disk (automatically fills the memory cache).
 * If server == NULL that means we only want to completely fill in the memory cache which
 * is needed for the AuthBrowser.
 */
int pauth_get( char *server, char *realm, char *authdata, int ftpmode )
{
	struct authnode *an; 

	/* scanning internal list first */
	if ( !ftpmode )
	{
		ObtainSemaphoreShared( &authlistsem );
		for( an = FIRSTNODE( &authlist ); NEXTNODE( an ); an = NEXTNODE( an ) )
		{
			if( !stricmp( an->server, server ) && !stricmp( an->realm, realm ) )
			{
				/* found in memcache */
				D( db_auth, bug( "found in internal list\n" ) );
				strcpy( authdata, an->authdata );
				ReleaseSemaphore( &authlistsem );
				return( TRUE );
			}
		}
		ReleaseSemaphore( &authlistsem );
	}
	return( FALSE );
}


DECNEW
{
	struct Data *data;
	APTR bt_ok, bt_cancel;
	APTR txt_server, txt_realm, str_user, str_pw;
	APTR lab_user, grp_contents;
	APTR chk_pw;

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('A','U','T','H'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, GS( AUTH_TITLE ),
		MUIA_Window_NoMenus, TRUE,
		WindowContents, VGroup,
			Child, TextObject, MUIA_Text_Contents, GS( AUTH_FAILED ), MUIA_ShowMe, GetTagData( MA_Auth_Failed, FALSE, msg->ops_AttrList ), End,

			Child, grp_contents = ColGroup( 2 ),
				Child, Label2( ( ULONG )GS( AUTH_L1 ) ),
				Child, txt_server = TextObject, TextFrame, MUIA_Background, MUII_TextBack, End,
				Child, Label2( ( ULONG )GS( AUTH_L2 ) ),
				Child, txt_realm = TextObject, TextFrame, MUIA_Background, MUII_TextBack, End,
				Child, lab_user = Label2( ( ULONG )GS( AUTH_L3 ) ),
				Child, str_user = TextinputObject, StringFrame, MUIA_CycleChain, 1, MUIA_String_AdvanceOnCR, TRUE, End,
				Child, Label2( ( ULONG )GS( AUTH_L4 ) ),
				Child, str_pw = TextinputObject, StringFrame, MUIA_CycleChain, 1, MUIA_String_Secret, TRUE, End,

				Child, chk_pw = CheckMark( getflag( VFLG_REMEMBER_AUTH ) ),
				Child, TextObject, NoFrame, MUIA_Text_Contents, GS( AUTH_L5 ), MUIA_Text_PreParse, "\033l", End,

			End,
			Child, hbar(),
			Child, HGroup,
				Child, bt_ok = makebutton( MSG_OK ),
				Child, bt_cancel = makebutton( MSG_CANCEL ),
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );
	data->str_user = str_user;
	data->str_pw = str_pw;
	data->chk_pw = chk_pw;
	data->ftpmode = GetTagData( MA_Auth_FTP, 0, msg->ops_AttrList );

	set( chk_pw, MUIA_CycleChain, 1 );

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Auth_Close, FALSE
	);
	DoMethod( str_pw, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Auth_Close, TRUE
	);
	DoMethod( bt_ok, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Auth_Close, TRUE
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Auth_Close, FALSE
	);

	data->me = obj;
	strcpy( data->realm, (char*)GetTagData( MA_Auth_Realm, -1, msg->ops_AttrList ) );
	strcpy( data->server, (char*)GetTagData( MA_Auth_Server, -1, msg->ops_AttrList ) );

	NEWLIST( &data->aumlist );
	ADDTAIL( &data->aumlist, (APTR)GetTagData( MA_Auth_AUM, -1, msg->ops_AttrList ) );

	ADDTAIL( &authreqlist, data );

	set( txt_server, MUIA_Text_Contents, data->server );
	set( txt_realm, MUIA_Text_Contents, data->realm );

	if( data->ftpmode )
	{
		DoMethod( grp_contents, OM_REMMEMBER, ( ULONG )lab_user );
		DoMethod( grp_contents, OM_REMMEMBER, ( ULONG )str_user );

		MUI_DisposeObject( lab_user );
		MUI_DisposeObject( str_user );

		data->str_user = NULL;

		set( obj, MUIA_Window_ActiveObject, data->str_pw );
	}
	else
		set( obj, MUIA_Window_ActiveObject, data->str_user );

	return( (ULONG)obj );
}


DECMETHOD( Auth_Close, ULONG )
{
	struct authmsg *aum;
	char buffer[ 256 ], accessdata[ 256 ];
	int save = FALSE;
	GETDATA;

	accessdata[ 0 ] = 0;

	set( obj, MUIA_Window_Open, FALSE );
	REMOVE( data );

	DoMethod( app, MUIM_Application_KillPushMethod, ( ULONG )obj );

	if( msg[ 1 ] ) // <- Ok button
	{
		struct authnode *an;

		ObtainSemaphore( &authlistsem ); // prevents win_auth from saving
		if( data->ftpmode )
		{
			stccpy( accessdata, getstrp( data->str_pw ), sizeof( accessdata ) );
		}
		else
		{
			sprintf( buffer, "%s:%s", getstrp( data->str_user ), getstrp( data->str_pw ) );
			copybase64( buffer, accessdata );
			//accessdata[ 76 ] = 0;
			if( getv( data->chk_pw, MUIA_Selected ) )
			{
				save = TRUE;
				setflag( VFLG_REMEMBER_AUTH, TRUE );
			}
			else
				setflag( VFLG_REMEMBER_AUTH, FALSE );
		}

		/*
		 * search if the server/realm is already internal, if so,
		 * update the password with what the user just typed
		 */
		for( an = FIRSTNODE( &authlist ); NEXTNODE( an ); an = NEXTNODE( an ) )
		{
			if( !stricmp( an->server, data->server ) && !stricmp( an->realm, data->realm ) )
			{
				D( db_auth, bug( "done updating\n" ) );
				strcpy( an->authdata, accessdata );  // update
				if( data->str_user )
					strcpy( an->authuser, getstrp( data->str_user ) );
				strcpy( an->authpass, getstrp( data->str_pw ) );
				break;
			}
		}
		
		/* add it to the internal list */
		if( !NEXTNODE( an ) )
		{
			an = malloc( sizeof( *an ) );
			memset( an, '\0', sizeof( *an ) );
			an->save = save; 
			strcpy( an->server, data->server );
			strcpy( an->realm, data->realm );
			strcpy( an->authdata, accessdata );
			if( data->str_user )
				strcpy( an->authuser, getstrp( data->str_user ) );
			strcpy( an->authpass, getstrp( data->str_pw ) );
			ADDTAIL( &authlist, an );
		}
		
		D( db_auth, bug("adding to window\n"));

		if( win_authbrowser )
				DoMethod( win_authbrowser, MM_AuthBrowser_Add, ( ULONG )an );
 
		ReleaseSemaphore( &authlistsem );
	}

	Forbid();
	while( aum = REMHEAD( &data->aumlist ) )
	{
		if( !aum->quitme )
		{
			aum->rc = msg[ 1 ];
			strcpy( aum->authdata, accessdata );
			ReplyMsg( ( struct Message * )aum );
		}
	}
	Permit();

	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );
	MUI_DisposeObject( obj );

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMETHOD( Auth_Close )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_authwinclass( void )
{
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "AuthWinClass";
#endif

	return( TRUE );
}

void delete_authwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getauthwin( void )
{
	return( mcc->mcc_Class );
}
 
int init_auth( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( authport = CreateMsgPort() )
	{
		authportsigmask = 1L<<authport->mp_SigBit;
		NEWLIST( &authlist );
		InitSemaphore( &authlistsem );
		NEWLIST( &authreqlist );
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


void cleanup_authentications( void )
{
	if( authport )
	{
		D( db_init, bug( "cleaning up..\n" ) );

		DeleteMsgPort( authport );
	}
}


/*
 * Loads the auths from disk and puts them in memory
 */
void load_auths( void )
{
#if USE_DOS
	struct AsyncFile *f;
	struct authnode *an; 
	char buffer[ 256 ];
	
	D( db_init, bug( "loading..\n" ) );
	
#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_AUTH ) );
	}
#endif
 
	/* scanning from disk cache */
	makecachepath( "Authcache.1", buffer );

	if( f = OpenAsync( buffer, MODE_READ, 1024 ) )
	{
		char authdata[ 128 ];
		char realmb[ 256 ];
		char authbuffer[ 258 ];
		char user[ 128 ], pw[ 128 ];

		while( FGetsAsyncNoLF( f, buffer, sizeof( buffer ) ) )
		{
			if( !FGetsAsyncNoLF( f, realmb, sizeof( realmb ) ) )
				break;
			if( !FGetsAsyncNoLF( f, user, sizeof( user ) ) )
				break;
			if( !FGetsAsyncNoLF( f, pw, sizeof( pw ) ) )
				break;

			D( db_auth, bug("adding entry for host %s to the internal list\n", buffer));
			*authdata = '\0';

			sprintf( authbuffer, "%s:%s", user, pw );
			copybase64( authbuffer, authdata );
				
			D( db_auth, bug("accessdata looks like: %s\n", authdata));

			an = malloc( sizeof( *an ) );
			memset( an, '\0', sizeof( *an ) );
			an->save = TRUE;
			strcpy( an->server, buffer );
			strcpy( an->realm, realmb );
			strcpy( an->authdata, authdata );
			strcpy( an->authuser, user );
			strcpy( an->authpass, pw );
			ADDTAIL( &authlist, an );

		}
		CloseAsync( f );
	}
#endif
}
 
/*
 * Save authentications to disk
 */
void save_authentications( void )
{
	if ( authport )
	{
	#if USE_DOS
		char pathbak[ 256 ], path[ 256 ];
		struct AsyncFile *f;

		if( !app_started || ISLISTEMPTY( &authlist ) )
		{
			return;
		}
		
		D( db_init, bug( "saving authentications to disk..\n" ) );
		
		makecachepath( "Authcache.1", path ); 

		makecachepath( "Authcache.1.bak", pathbak ); 
		
		Rename( path, pathbak );

		if( f = OpenAsync( path, MODE_WRITE, 1024 ) )
		{
			struct authnode *an; 
			ObtainSemaphore( &authlistsem );
			
			for( an = FIRSTNODE( &authlist ); NEXTNODE( an ); an = NEXTNODE( an ) )
			{
				D( db_auth, bug("saving authnode %s\n", an->server));
				if (an->save)
					FPrintfAsync( f, "%s\n%s\n%s\n%s\n", ( int )an->server, ( int )an->realm, ( int )an->authuser, ( int )an->authpass );
			}
			ReleaseSemaphore( &authlistsem );
			CloseAsync( f );
		}

		D( db_auth, bug("cleaning up\n"));

		/* clean up */
		DeleteFile( pathbak );
		D( db_auth, bug("done\n"));
	#endif
	}
}

// Note! This function is called from other
// tasks!
int auth_query( struct parsedurl *purl, char *server, char *realm, int failedalready, char *authdata, int ftpmode )
{
	struct authmsg aum;
	struct MsgPort *replyport = CreateMsgPort();

	D( db_auth, bug("server == %s, realm == %s\n", server, realm ));

	if( !failedalready && purl->username && purl->password )
	{
		char temp[ 128 ];

		sprintf( temp, "%.63s:%.63s", purl->username, purl->password );
		copybase64( temp, authdata );
		return( TRUE );
	}

	if( !failedalready )
	{
		/*
		 * Try to get the auth from the cache.
		 */
		if( pauth_get( server, realm, authdata, ftpmode ) )
		{
			return( TRUE );
		}
	}

	memset( &aum, 0, sizeof( aum ) );
	stccpy( aum.server, server, sizeof( aum.server ) );
	stccpy( aum.realm, realm, sizeof( aum.realm ) );
	aum.failedalready = failedalready;
	aum.ftpmode = ftpmode;
	aum.m.mn_ReplyPort = replyport;
	PutMsg( authport, ( struct Message * )&aum );

	for(;;)
	{
		if( GetMsg( replyport ) )
			break;
		if( Wait( ( 1L<<replyport->mp_SigBit ) | SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C )
		{
			aum.quitme = TRUE;
			Signal( authport->mp_SigTask, authportsigmask );
			break;
		}
	}

	DeleteMsgPort( replyport );

	if( aum.rc )
		strcpy( authdata, aum.authdata );

	return( aum.rc );
}

void auth_process( void )
{
	struct authmsg *aum;
	struct Data *data;

	while( aum = ( struct authmsg *) GetMsg( authport ) )
	{
		int found = FALSE;

		if( aum->method == 1 )
		{
			cookie_process( (APTR)aum );
			continue;
		}
		else if( aum->method == 2 )
		{
			certreq_process( (APTR)aum );
			continue;
		}

		for( data = FIRSTNODE( &authreqlist ); NEXTNODE( data ); data = NEXTNODE( data ) )
		{
			if( !stricmp( data->server, aum->server ) && !stricmp( data->realm, aum->realm ) )
			{
				ADDTAIL( &data->aumlist, aum );
				found = TRUE;
				break;
			}
		}

		if( !found )
		{
			APTR o;

			o = NewObject( getauthwin(), NULL,
				MA_Auth_Server, aum->server,
				MA_Auth_Realm, aum->realm,
				MA_Auth_Failed, aum->failedalready,
				MA_Auth_AUM, aum,
				MA_Auth_FTP, aum->ftpmode,
				TAG_DONE
			);
			if( !o )
			{
				aum->failedalready = TRUE;
				ReplyMsg( ( struct Message * )aum );
				continue; // main loop
			}
			DoMethod( app, OM_ADDMEMBER, ( ULONG )o );
			set( o, MUIA_Window_Open, TRUE );
		}
	}

	for( data = FIRSTNODE( &authreqlist ); NEXTNODE( data ); data = NEXTNODE( data ) )
	{
		struct authmsg *aum;

		for( aum = FIRSTNODE( &data->aumlist ); NEXTNODE( aum ); aum = NEXTNODE( aum ) )
		{
			if( aum->quitme )
			{
				DoMethod( app, MUIM_Application_PushMethod, ( ULONG )data->me, 2, MM_Auth_Close, FALSE );
				break;
			}
		}
	}

	cookie_checkclose();
	certreq_checkclose();
}

#endif /* USE_NET */
