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
** $Id: printrequest.c,v 1.24 2003/07/06 16:51:34 olli Exp $
**
*/

#include "voyager.h"

/* private */
#include "voyager_cat.h"
#include "copyright.h"
#include "prefs.h"
#include "classes.h"
#include "mui_func.h"
#include "html.h"
#include "network.h"
#include "parse.h"
#include "htmlwin.h"
#include "dos_func.h"


struct Data {
	APTR myvobj;
	APTR cyc_mode;
	APTR grp;
	APTR gauge;
	APTR grp_printer;
	ULONG abortflag;
};

DECCONST
{
	struct Data *data;
	APTR bt_grp, bt_print, bt_printer, bt_printergfx, bt_close, cyc_mode, grp, gauge, bt_abort;
#if USE_TURBOPRINT
	struct DosList *doslist;
	APTR bt_turboprefs;
	int turboprint;
#endif /* USE_TURBOPRINT */
	static STRPTR printmodes[ 4 ];

	printmodes[ 0 ] = GS( PRINT_MODE_GFXBG );
	printmodes[ 1 ] = GS( PRINT_MODE_GFX );
	printmodes[ 2 ] = GS( PRINT_MODE_TEXT );

#if USE_TURBOPRINT
	/*
	 * Try to find the Turboprint: assign
	 * to add the TurboPrefs calling button
	 */
	doslist = LockDosList( LDF_ASSIGNS | LDF_READ );
	
	if( FindDosEntry( doslist, "Turboprint", LDF_ASSIGNS ) )
	{
		turboprint = TRUE;
	}
	else
	{
		turboprint = FALSE;
	}

	UnLockDosList( LDF_ASSIGNS );

#endif /* USE_TURBOPRINT */

	obj = DoSuperNew( cl, obj,
		MUIA_Window_ID, MAKE_ID('P','R','I','T'),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_Title, GS( PRINT_TITLE ),
		WindowContents, VGroup,

			Child, TextObject, MUIA_Text_Contents, GetTagData( MA_Printwin_URL, ( ULONG )NULL, msg->ops_AttrList ), TextFrame, MUIA_Background, MUII_TextBack, End,

			Child, grp = PageGroup,

				Child, VGroup,
		
					Child, HGroup,
						Child, Label2( ( ULONG )GS( PRINT_MODE ) ),
						Child, cyc_mode = Cycle( printmodes ),
					End,
		
					Child, bt_grp = HGroup,
						Child, bt_printer = button( MSG_PRINT_BT_PRINTER, 0 ),
						Child, bt_printergfx = button( MSG_PRINT_BT_PRINTERGFX, 0 ),
					End,
		
					Child, hbar(),
		
					Child, HGroup,
						Child, bt_print = button( MSG_PRINT_BT_PRINT, 0 ),
						Child, bt_close = button( MSG_CLOSE, 0 ),
					End,

				End,

				Child, VGroup, // busy group

					Child, VGroup,
						Child, gauge = GaugeObject, GaugeFrame, MUIA_Gauge_Horiz, TRUE, MUIA_Gauge_InfoText, GS( PRINT_PRINTING ), End,
						Child, ScaleObject, End,
					End,

					Child, bt_abort = button( MSG_PRINT_BT_ABORT, 0 ),
				End,

			End,
		End,
	End;
#if USE_TURBOPRINT
	bt_turboprefs = button( MSG_PRINT_BT_TURBOPRINT, 0 );
	DoMethod( bt_grp, OM_ADDMEMBER, bt_turboprefs );	
#endif /* USE_TURBOPRINT */

	if( !obj )
	{
		return( ( ULONG )NULL );
	}

	data = INST_DATA( cl, obj );

	data->myvobj = (APTR)GetTagData( MA_Printwin_HTMLView, ( ULONG )NULL, msg->ops_AttrList );

	data->cyc_mode = cyc_mode;
	data->grp = grp;
	data->gauge = gauge;

#if USE_TURBOPRINT
	if( !turboprint )
	{
		set( bt_turboprefs, MUIA_ShowMe, FALSE );
	}
#endif /* USE_TURBOPRINT */

	SetAttrs( cyc_mode,
		MUIA_Cycle_Active, getflag( VFLG_PRINTMODE ),
		MUIA_CycleChain, 1,
		TAG_DONE
	);

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_Print_Close
	);
	DoMethod( bt_print, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_Print_Start
	);
	DoMethod( bt_close, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_Print_Close
	);
	DoMethod( bt_printer, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )obj, 2, MM_Print_CallPrefs, ( ULONG )"SYS:Prefs/Printer"
	);
	DoMethod( bt_printergfx, MUIM_Notify, MUIA_Pressed, FALSE,
		( ULONG )obj, 2, MM_Print_CallPrefs, ( ULONG )"SYS:Prefs/PrinterGFX"
	);

#if USE_TURBOPRINT
	if( turboprint )
	{
		DoMethod( bt_turboprefs, MUIM_Notify, MUIA_Pressed, FALSE,
			obj, 2, MM_Print_CallPrefs, "Turboprint:TurboPrefs"
		);
	}
#endif /* USE_TURBOPRINT */

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		notify, 3, MUIM_WriteLong, TRUE, &data->abortflag
	);

	DoMethod( bt_abort, MUIM_Notify, MUIA_Pressed, FALSE,
		notify, 3, MUIM_WriteLong, TRUE, &data->abortflag
	);

	return( (ULONG)obj );
}


DECMETHOD( Print_Close, ULONG )
{
	GETDATA;

	setflag( VFLG_PRINTMODE, getv( data->cyc_mode, MUIA_Cycle_Active ) );

/*
	TOFIX
	DoMethod( data->myvobj, MM_HTMLView_PrintWinClosed );
*/
	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );
	MUI_DisposeObject( obj );

	return( 0 );
}

#if 0
static void printtext( APTR obj, struct Data *data, struct nstream *stream )
{
	int slen = nets_getdocptr( stream );
	char *txt = nets_getdocmem( stream ), *otxt;
	BPTR f;
	int error = FALSE;

	set( data->gauge, MUIA_Gauge_Max, slen );

#if USE_DOS
	f = Open( "PRT:", MODE_NEWFILE );
	if( !f )
	{
		MUI_Request( app, obj, 0, GS( ERROR ), GS( CANCEL ), GS( PRINT_OPENERR), 0 );
		return;
	}

	if( !strnicmp( nets_mimetype( stream ), "text/html", 9 ) )
	{
		int premode = FALSE, lastblank = FALSE, eatup = FALSE;
		int bp = 0, ilc = 0;
		char buffer[ 80 ];
		int dummy;

		otxt = txt;

		while( *txt && !data->abortflag && !error )
		{
			int ch = gettoken( &txt, &dummy );
			char *special;

			set( data->gauge, MUIA_Gauge_Current, txt - otxt );

			if( !ch )
				break;

			if( ch < 256 )
			{
				if( eatup )
					continue;

				if( premode )
				{
					if( ch == '\n' )
						ilc = 0;

					if( FPutC( f, ch ) < 0 )
						error = TRUE;
					continue;
				}

				if( isspace( ch ) )
				{
					if( lastblank )
						continue;
					lastblank = TRUE;
					if( bp )
						FWrite( f, buffer, bp, 1 );
					if( FPutC( f, ' ' ) < 0 )
						error = TRUE;
					bp = 0;
				}
				else
				{
					lastblank = FALSE;
					buffer[ bp++ ] = ch;
					if( bp == sizeof( buffer ) )
					{
						FWrite( f, buffer, bp, 1 );
						bp = 0;
					}
				}

				if( ++ilc > 79 )
				{
					if( FPutC( f, '\n' ) < 0 )
						error = TRUE;
					ilc = 0;
				}
				continue;
			}

			special = NULL;

			switch( ch )
			{
				case ht_h1:
				case ht_h2:
				case ht_h3:
				case ht_h4:
				case ht_h5:
				case ht_h6:
					special = "\033[6w\033[1m\n";
					break;

				case ht_h1 | HTF_NEGATE:
				case ht_h2 | HTF_NEGATE:
				case ht_h3 | HTF_NEGATE:
				case ht_h4 | HTF_NEGATE:
				case ht_h5 | HTF_NEGATE:
				case ht_h6 | HTF_NEGATE:
					special = "\033[0w\033[22m\n";
					break;

				case ht_b:
					special = "\033[1m";
					break;
				case ht_b | HTF_NEGATE:
					special = "\033[22m";
					break;

				case ht_i:
					special = "\033[3m";
					break;
				case ht_i | HTF_NEGATE:
					special = "\033[23m";
					break;

				case ht_u:
					special = "\033[4m";
					break;
				case ht_u | HTF_NEGATE:
					special = "\033[24m";
					break;

				case ht_title:
					special = "\033[4m\033[1m\033[6w\n";
					break;
				case ht_title | HTF_NEGATE:
					special = "\033[0m\033[0w\n";
					break;

				case ht_br:
				case ht_td:
				case ht_td | HTF_NEGATE:
				case ht_tr:
				case ht_tr | HTF_NEGATE:
				case ht_th:
				case ht_th | HTF_NEGATE:
				case ht_table:
				case ht_table | HTF_NEGATE:
					if( ilc )
						special = "\n";
					break;

				case ht_hr:
					special = "\n_________________________________________________________________________\n";
					break;
			}

			if( special )
			{
				if( strchr( special, '\n' ) )
					lastblank = TRUE, ilc = 0;
				error = FPuts( f, special );
			}
		}
	}
	else
	{
		// plain text
		Write( f, txt, slen );
		Flush( f );
	}
	set( data->gauge, MUIA_Gauge_Current, slen );
	FPutC( f, '\f' );
	Close( f );
#endif
}
#endif

DECMETHOD( Print_Start, ULONG )
{
	GETDATA;
	//int mode = getv( data->cyc_mode, MUIA_Cycle_Active );

	set( data->grp, MUIA_Group_ActivePage, 1 );

/*
	TOFIX
	if( mode == 2 )
	{
		printtext( obj, data, (APTR)getv( data->myvobj, MA_HTMLView_StreamHandle ) );
	}
	else
	{
		DoMethod( data->myvobj, MM_HTMLView_DoPrint, ( ULONG )data->gauge, ( ULONG )&data->abortflag, mode + 1 );
	}
*/
	return( DoMethod( obj, MM_Print_Close ) );
}

DECSMETHOD( Print_CallPrefs )
{
	char buffer[ 256 ];

	sprintf( buffer, "%s PUBSCREEN \"%s\"",
		msg->path,
		getpubname( obj )
	);
#if USE_DOS
	SystemTags( buffer,
		SYS_Asynch, TRUE,
		SYS_Input, Open( "NIL:", MODE_NEWFILE ),
		SYS_Output, Open( "NIL:", MODE_NEWFILE ),
		TAG_DONE
	);
#endif

	return( 0 );
}

BEGINMTABLE
DEFNEW
DEFMETHOD( Print_Close )
DEFMETHOD( Print_Start )
DEFSMETHOD( Print_CallPrefs )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_printwinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "PrintWinClass";
#endif

	return( TRUE );
}

void delete_printwinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprintwinclass( void )
{
	return( mcc->mcc_Class );
}
