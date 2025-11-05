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
 * Certificates
 * ------------
 *
 * © 2000 VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prefswin_certs.c,v 1.29 2003/07/06 16:51:34 olli Exp $
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif

/* private */
#include "prefswin.h"
#include "certs.h"
#include "malloc.h"
#include "mui_func.h"
#include "dos_func.h"
#include "textinput.h"


static struct MinList certlist;
static int certsloaded;

struct certnode {
	struct MinNode n;
	int isactive, wasactive;
	char name[ 80 ];
	char filename[ 16 ];
	char *info;
};

struct Data {
	APTR chk_ssl2;
	APTR chk_ssl3;
	APTR chk_tls1;
	APTR chk_sslbugs;
	APTR txt_certinfo;
	APTR bt_cert_act;
	APTR bt_cert_deact;
	APTR lv_certs;
};


static void renamecert( struct certnode *n )
{
#if USE_DOS
	char hash[ 16 ];
	char frompath[ 128 ], topath[ 128 ];
	BPTR tl;
	int c;

	strcpy( hash, n->filename );
	*strchr( hash, '.' ) = 0;

	// find a free hash slot
	for( c = 0; c < 16384; c++ )
	{
		sprintf( topath, "PROGDIR:Certificates%s/%s.%ld",
			n->isactive ? "" : "/Inactive",
			hash,
			( long int )c
		);
		if( !( tl = Lock( topath, SHARED_LOCK ) ) )
		{
			sprintf( frompath, "PROGDIR:Certificates%s/%s",
				n->wasactive ? "" : "/Inactive",
				n->filename
			);
			Rename( frompath, topath );
			return;
		}
		UnLock( tl );
	}
#endif
}

void cleanupcerts( int doit )
{
	struct certnode *n;

	if( certsloaded )
	{
#if USE_DOS
		UnLock( CreateDir( "PROGDIR:Certificates" ) );
		UnLock( CreateDir( "PROGDIR:Certificates/Inactive" ) );
#endif
		while( n = REMHEAD( &certlist ) )
		{
			if( doit )
			{
				if( n->isactive != n->wasactive )
					renamecert( n );
			}

			if( n->info )
				free( n->info );
			free( n );
		}
		certsloaded = FALSE;
	}
}

/*
 * Only files
 */
#if USE_DOS && USE_SSL
MUI_HOOK( certscan, ULONG *ptype, struct ExAllData *ed )
{
	return( ed->ed_Type < 0 );
}
#endif

#define EAC_BUFFERSIZE 2048 /* ExAll buffer */

#if USE_DOS && USE_SSL
static void loadcerts( char *dir, int isactive, struct Library *VSSLBase )
{
	BPTR l;
	BPTR oldcd;
	char buffer[ 2048 ];
	struct ExAllControl *eac;
	struct ExAllData *ead;
	APTR ex_buffer;

	l = Lock( dir, SHARED_LOCK );
	if( !l )
		return;

	oldcd = CurrentDir( l );

	if( ( eac = AllocDosObject( DOS_EXALLCONTROL, NULL ) ) )
	{
		if( ( ex_buffer = malloc( EAC_BUFFERSIZE ) ) )
		{
			int more;

			eac->eac_LastKey = 0;
			eac->eac_MatchFunc = &certscan_hook;
			do
			{
				more = ExAll( l, ex_buffer, EAC_BUFFERSIZE, ED_TYPE, eac );

				if( ( !more ) && ( IoErr() != ERROR_NO_MORE_ENTRIES ) )
				{
					displaybeep();
					break;
				}

				ead = ( struct ExAllData * )ex_buffer;
				do
				{
					if( ead->ed_Type < 0 )
					{
						if( strlen( ead->ed_Name ) >= 10 && strchr( ead->ed_Name, '.' ) )
						{
							X509 *newcert;

							newcert = VSSL_ReadCertPEM( ead->ed_Name );
							if( newcert )
							{
								struct certnode *cn = malloc( sizeof( *cn ) );
								if( cn )
								{
									X509_NAME *n;
									memset( cn, '\0', sizeof( *cn ) );
									stccpy( cn->filename, ead->ed_Name, sizeof( cn->filename ) );
									ADDTAIL( &certlist, cn );

									cn->isactive = cn->wasactive = isactive;

									cert_getinfo( newcert, buffer, VSSLBase );

									cn->info = strdup( buffer ); /* TOFIX */

									n = VSSL_X509_get_subject_name( newcert );
									if( n )
									{
										char *s = VSSL_X509_NameOneline( n );
										//Printf( "%s\n", s );
										if( s )
										{
											char *p = strstr( s, "O=" );
											int c;
											if( !p )
											{
												p = strrchr( s, '/' );
												if( p )
													p = p + 1;
												else
													p = s;
											}
											else
												p += 2;

											for( c = 0; p[ c ] && p[ c ] != '/' && c < sizeof( cn->name ) - 1; c++ )
												cn->name[ c ] = p[ c ];
											cn->name[ c ] = 0;
											VSSL_X509_FreeNameOneline( s );
										}
									}
								}
								VSSL_X509_Free( newcert );
							}
						}
					}
					ead = ead->ed_Next;
				} while( ead );
			} while( more );
			free( ex_buffer );
		}
		else
		{
			displaybeep();
		}
		FreeDosObject( DOS_EXALLCONTROL, eac );
	}
	else
	{
		displaybeep();
	}
	CurrentDir( oldcd );
	UnLock( l );
}
#endif /* USE_DOS && USE_SSL */

MUI_HOOK( cert_disp, STRPTR *array, struct certnode *cn )
{
	if( !cn )
	{
	}
	else
	{
		*array++ = cn->isactive ? "×" : " ";
		*array++ = cn->name;
	}
	return( 0 );
}

MUI_HOOK( cert_cmp, struct certnode *cn2, struct certnode *cn1 )
{
	return( (LONG)stricmp( cn1->name, cn2->name ) );
}


/*
 * Displays the selected cert and sets the buttons' states
 * accordingly
 */
DECMETHOD( Prefswin_Certs_Display, ULONG )
{
	GETDATA;
	struct certnode *cn;

	DoMethod( data->lv_certs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &cn );
	if( cn )
	{
		set( data->txt_certinfo, MUIA_Textinput_Contents, cn->info );
		set( data->bt_cert_act, MUIA_Disabled, cn->isactive );
		set( data->bt_cert_deact, MUIA_Disabled, !cn->isactive );
	}
	return( 0 );
}


/*
 * Activate/Deactivate the cert
 */
DECSMETHOD( Prefswin_Certs_ChangeState )
{
	GETDATA;
	struct certnode *cn;

	DoMethod( data->lv_certs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &cn );
	if( cn )
	{
		if( msg->mode == MV_Prefswin_Certs_Switch )
		{
			cn->isactive = ((cn->isactive + 1) & 0x1);
		}
		else
		{
			cn->isactive = msg->mode;
		}
		DoMethod( data->lv_certs, MUIM_List_Redraw, MUIV_List_Redraw_Active );
		DoMethod( obj, MM_Prefswin_Certs_Display );
	}
	return( 0 );
}


DECNEW
{
	struct Data *data;
	APTR lv_certs, txt_certinfo, bt_cert_act, bt_cert_deact;
	APTR chk_ssl2, chk_ssl3, chk_tls1, chk_sslbugs;
 
	struct certnode *cn;
#if !defined( MBX ) && USE_SSL
	struct Library *VSSLBase;
#endif

	if( !certsloaded )
	{
#ifndef MBX
#if USE_SSL
		VSSLBase = OpenLibrary( "PROGDIR:Plugins/voyager_ssl.vlib", 6 );
		if( !VSSLBase )
		{
			obj = DoSuperNew( cl, obj,
				GroupSpacing( 0 ), GroupFrame,
				Child, VSpace( 0 ),
				Child, HGroup,
					Child, HSpace( 0 ),
					Child, TextObject, MUIA_Text_Contents, GS( PREFS_CERTS_NOSSL ), TextFrame, MUIA_Background, MUII_TextBack, End,
					Child, HSpace( 0 ),
				End,
				Child, VSpace( 0 ),
			End;

			if( obj )
			{
				return( ( ULONG )obj );
			}
			else
			{
				return( 0 );
			}
		}
		NEWLIST( &certlist );
		loadcerts( "PROGDIR:Certificates", TRUE, VSSLBase );
		loadcerts( "PROGDIR:Certificates/Inactive", FALSE, VSSLBase );
		certsloaded = TRUE;
		CloseLibrary( VSSLBase );
#endif
#endif
	}

	obj	= DoSuperNew( cl, obj,

		Child, HGroup, GroupFrameT( GS( PREFS_CERTS_TITLE ) ),

			Child, VGroup, MUIA_Group_Spacing, 0,

				Child, lv_certs = ListviewObject, MUIA_CycleChain, 1,
					MUIA_Listview_List, ListObject, InputListFrame,
						MUIA_Background,MUII_ListBack,
						MUIA_List_DisplayHook, &cert_disp_hook,
						MUIA_List_CompareHook, &cert_cmp_hook,
						MUIA_List_Format, "BAR,",
					End,
				End,

				Child, HGroup, MUIA_Group_Spacing, 0,
					Child, bt_cert_act = makebutton( MSG_PREFS_CERTS_ACTIVATE ),
					Child, bt_cert_deact = makebutton( MSG_PREFS_CERTS_DEACTIVATE ),
				End,
			End,

			Child, BalanceObject, End,

			Child, txt_certinfo = TextinputscrollObject, TextFrame,
				MUIA_Background,MUII_TextBack,
				MUIA_Textinput_NoInput, TRUE,
				MUIA_Textinput_Multiline, TRUE,
				MUIA_Textinput_WordWrap, FALSE,
			End,

		End,

		Child, HGroup, MUIA_Font, MUIV_Font_Tiny, /* TOFIX: put catalogs support */
			Child, Label( ( STRPTR )"SSLv_2?" ),
			Child, chk_ssl2 = pcheck( DSI_NET_SSL2, "SSLv_2?" ),
			Child, HSpace( 0 ),
			Child, Label( ( STRPTR )"SSLv_3?" ),
			Child, chk_ssl3 = pcheck( DSI_NET_SSL3, "SSLv_3?" ),
			Child, HSpace( 0 ),
			Child, Label( ( STRPTR )"TLSv_1?" ),
			Child, chk_tls1 = pcheck( DSI_NET_TLS1, "TLSv_1?" ),
			Child, HSpace( 0 ),
			Child, Label( ( STRPTR )"_Bugs?" ),
			Child, chk_sslbugs = pcheck( DSI_NET_BUGS, "_Bugs?" ),
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );
	data->chk_ssl2 = chk_ssl2;
	data->chk_ssl3 = chk_ssl3;
	data->chk_tls1 = chk_tls1;
	data->chk_sslbugs = chk_sslbugs;
	data->txt_certinfo = txt_certinfo;
	data->bt_cert_act = bt_cert_act;
	data->bt_cert_deact = bt_cert_deact;
	data->lv_certs = lv_certs;

	for( cn = FIRSTNODE( &certlist ); NEXTNODE( cn ); cn = NEXTNODE( cn ) )
	{
		DoMethod( lv_certs, MUIM_List_InsertSingle, cn, MUIV_List_Insert_Sorted );
	}

	DoMethod( lv_certs, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		obj, 1, MM_Prefswin_Certs_Display
	);

	DoMethod( bt_cert_act, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MM_Prefswin_Certs_ChangeState, MV_Prefswin_Certs_Enable
	);
	DoMethod( bt_cert_deact, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MM_Prefswin_Certs_ChangeState, MV_Prefswin_Certs_Disable
	);

	DoMethod( lv_certs, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
		obj, 2, MM_Prefswin_Certs_ChangeState, MV_Prefswin_Certs_Switch
	);

	set( bt_cert_act, MUIA_Disabled, TRUE );
	set( bt_cert_deact, MUIA_Disabled, TRUE );

	/* Help */
	set( bt_cert_act, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_CERT_ACT ) );
	set( bt_cert_deact, MUIA_ShortHelp, GS( SH_PREFSWIN_BT_CERT_DEACT ) );
	set( chk_ssl2, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_SSL2 ) );
	set( chk_ssl3, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_SSL3 ) );
	set( chk_tls1, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_TLS1 ) );
	set( chk_sslbugs, MUIA_ShortHelp, GS( SH_PREFSWIN_CHK_SSLBUGS ) );

	return( ( ULONG )obj );
}


DECDISPOSE
{
	GETDATA;

	if( certsloaded )
	{
		storeattr( data->chk_ssl2, MUIA_Selected, DSI_NET_SSL2 );
		storeattr( data->chk_ssl3, MUIA_Selected, DSI_NET_SSL3 );
		storeattr( data->chk_tls1, MUIA_Selected, DSI_NET_TLS1 );
		storeattr( data->chk_sslbugs, MUIA_Selected, DSI_NET_BUGS );
	}
	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFDISPOSE
DEFMETHOD( Prefswin_Certs_Display )
DEFSMETHOD( Prefswin_Certs_ChangeState )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prefswin_certsclass( void )
{
	D( db_init, bug( "initializing..\n" ) );

	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "Prefswin_CertsClass";
#endif

	return( TRUE );
}

void delete_prefswin_certsclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprefswin_certsclass( void )
{
	return( mcc->mcc_Class );
}

#endif /* USE_NET */
