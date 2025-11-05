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
** $Id: certificates.c,v 1.29 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/dos.h>
#endif

/* private */
#include "voyager_cat.h"
#include "prefs.h"
#include "authipc.h"
#include "certs.h"
#include "vssl.h"
#include "classes.h"
#include "mui_func.h"

static struct MinList certreqlist;

void init_certreqslist( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &certreqlist );
}

struct certmsg {
	struct Message m;
	int method;
	int quitme;
	int rc;
	char cert_error[ 80 ];
	char cert_info[ 2048 ];
};

struct Data {
	struct MinNode n;
	APTR me;
	struct certmsg *cm;
};

#ifdef MBX
#define MUIA_WINDOW_HEIGHT MUIA_Window_Height, MUIV_Window_Height_Screen( 50 ),
#else
#define MUIA_WINDOW_HEIGHT
#endif

DECNEW
{
	struct Data *data;
	APTR bt_ok, bt_cancel, bt_save;
	APTR txt_info;
	struct certmsg *cm = (APTR)GetTagData( MA_Cert_CertMessage, -1, msg->ops_AttrList );

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('C','E','R','T'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_NoMenus, TRUE,
		MUIA_Window_Title, GS( CERTREQWIN_TITLE ),
		MUIA_WINDOW_HEIGHT
		WindowContents, VGroup, MUIA_Background, MUII_RequesterBack,
			Child, ScrollgroupObject,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_Contents, VGroupV, TextFrame, MUIA_Background, MUII_TextBack, 
					Child, txt_info = TextObject, InnerSpacing( 10, 10 ), End,
				End,
			End,
			Child, HGroup,
				Child, bt_ok = makebutton( MSG_CERTREQWIN_ONCE ),
				Child, bt_save = makebutton( MSG_CERTREQWIN_SAVE ),
				Child, bt_cancel = makebutton( MSG_CERTREQWIN_CANCEL ),
			End,
		End,
	End;

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cert_Close, 0
	);
	DoMethod( bt_ok, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cert_Close, 1
	);
	DoMethod( bt_save, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cert_Close, 2
	);
	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 5, MUIM_Application_PushMethod, ( ULONG )obj, 2, MM_Cert_Close, FALSE
	);

	data->me = obj;
	data->cm = cm;

	ADDTAIL( &certreqlist, data );

	DoMethod( txt_info, MUIM_SetAsString, MUIA_Text_Contents, ( ULONG )GS( CERTREQWIN_INFO ),
		cm->cert_error, cm->cert_info
	);

	return( (ULONG)obj );
}


DECMETHOD( Cert_Close, ULONG )
{
	GETDATA;

	set( obj, MUIA_Window_Open, FALSE );
	REMOVE( data );

	DoMethod( app, MUIM_Application_KillPushMethod, ( ULONG )obj );

	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );

	data->cm->rc = msg[ 1 ];

	Forbid();

	if( !data->cm->quitme )
		ReplyMsg( ( struct Message * )data->cm );

	Permit();

	MUI_DisposeObject( obj );
	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFMETHOD( Cert_Close )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_certreqwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "CertReqWinClass";
#endif

	return( TRUE );
}

void delete_certreqwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getcertreqwin( void )
{
	return( mcc->mcc_Class );
}

void certreq_process( struct certmsg *cm )
{
	APTR o = NewObject( getcertreqwin(), NULL,
		MA_Cert_CertMessage, cm,
		TAG_DONE
	);
	if( !o )
	{
		ReplyMsg( ( struct Message * )cm );
		return;
	}
	DoMethod( app, OM_ADDMEMBER, ( ULONG )o );
	set( o, MUIA_Window_Open, TRUE );
}

#if USE_SSL

static void splitx509name( char *to, char *s )
{
	char *p;
	Forbid();
	for( p = strtok( s, "/" ); p; p = strtok( NULL, "/" ) )
	{
		p = strchr( p, '=' );
		if( p )
		{
			p = stpblk( p + 1 );
			if( *p )
			{
				strcat( to, "   " );
				strcat( to, p );
				strcat( to, "\n" );
			}
		}
	}   
	Permit();
	strcat( to, "\n" );
}


void cert_getinfo( X509 *cert, char *to, VSSLBASE )
{
	X509_NAME *nc;
	ASN1_UTCTIME *tr;
	char buff[ 128 ];
	int c, n;

	nc = VSSL_X509_get_subject_name( cert );
	if( nc )
	{
		char *s = VSSL_X509_NameOneline( nc );
		if( s )
		{
			strcpy( to, "SUBJECT:\n" );
			splitx509name( to, s );
			VSSL_X509_FreeNameOneline( s );
		}
	}
	nc = VSSL_X509_get_issuer_name( cert );
	if( nc )
	{
		char *s = VSSL_X509_NameOneline( nc );
		if( s )
		{
			strcat( to, "ISSUER:\n" );
			splitx509name( to, s );
			VSSL_X509_FreeNameOneline( s );
		}
	}

	VSSL_X509_serialnumber( cert, buff );
	sprintf( strchr( to, 0 ), " Serial number: %s\n", buff );
	to = strchr( to, 0 );

	tr = VSSL_X509_get_notBefore( cert );
	if( tr )
	{
		strcat( to, " Valid from " );
		VSSL_ASN1_UTCTIME_sprint( strchr( to, 0 ), tr );
		strcat( to, "\n" );
	}

	tr = VSSL_X509_get_notAfter( cert );
	if( tr )
	{
		strcat( to, " Valid until " );
		VSSL_ASN1_UTCTIME_sprint( strchr( to, 0 ), tr );
		strcat( to, "\n\n" );
	}

	if( n = VSSL_X509_fingerprint( cert, buff ) )
	{
		strcat( to, "FINGERPRINT:\n " );
		for( c = 0; c <n; c++ )
		{
			sprintf( strchr( to, 0 ), " %02x", buff[ c ] );
		}
		strcat( to, "\n" );
	}
}

int certreq_ask( X509 *cert, char *error, VSSLBASE, struct unode *un )
{
	struct MsgPort *replyport = CreateMsgPort();
	struct certmsg cm;

	memset( &cm, 0, sizeof( cm ) );

	stccpy( cm.cert_error, error, sizeof( cm.cert_error ) );
	strcpy( cm.cert_info, un->sslpeercert );
	//cert_getinfo( cert, cm.cert_info, VSSLBase );


	// assemble cert info

	cm.method = 2;
	cm.m.mn_ReplyPort = replyport;

	PutMsg( authport, (struct Message *)&cm );

	for(;;)
	{
		if( GetMsg( replyport ) )
			break;
		if( Wait( ( 1L<<replyport->mp_SigBit ) | SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C )
		{
			cm.quitme = TRUE;
			Signal( authport->mp_SigTask, authportsigmask );
			break;
		}
	}

	DeleteMsgPort( replyport );

#if USE_DOS
	if( cm.rc == 2 )
	{
		char name[ 128 ];
		FileLock_p f;
		int cnt;
		ULONG h = VSSL_X509_NameHash( cert );

		// save the damned cert
		UnLock( CreateDir( "PROGDIR:Certificates" ) );

		for( cnt = 0; cnt < 16383; cnt++ )
		{
			sprintf( name, "PROGDIR:Certificates/%08lx.%ld", h, cnt );
			if( !( f = Lock( name, SHARED_LOCK ) ) )
				break;
			UnLock( f );
		}
		VSSL_WriteCertPEM( cert, name );
	}
#endif

	return( cm.rc );
}
#endif

void certreq_checkclose( void )
{
	struct Data *data, *nd;

	for( data = FIRSTNODE( &certreqlist ); nd = NEXTNODE( data ); data = nd )
	{
		if( data->cm->quitme )
		{
			DoMethod( app, MUIM_Application_PushMethod, ( ULONG )data->me, 2, MM_Cert_Close, FALSE );
		}
	}
}

#endif /* USE_NET */
