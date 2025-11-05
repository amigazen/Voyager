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
** Document info window class
** --------------------------
**
** Used to display informations regarding the current displayed document
** be it either HTML, FTP, Directory scan, Flash (yeah yeah, soon :) or
** whatever..
**
** Updates nicely alonside the document being loaded :)
**
** Started by David Gerber <zapek@vapor.com>
**
** $Id: docinfowin.c,v 1.44 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#endif


/* private */
#include "network.h"
#include "classes.h"
#include "voyager_cat.h"
#include "copyright.h"
#include "certs.h"
#include "mui_func.h"
#include "methodstack.h"
#include "textinput.h"


extern void closessl( void );

/* global data */
struct MinList docinfolist;
static const char *pages[] = { "General", "SSL", NULL };

/* instance data */
struct Data {
	struct MinNode n;
	APTR obj;
	APTR url;
	APTR httpd;
	APTR mimetype;
	APTR source;
#if USE_SSL
	APTR sslcipher;
	APTR sslversion;
	APTR ssl;
#endif /* USE_SSL */
	APTR mail;
	APTR subject;
	APTR body;
	APTR size;
	APTR sourceid;
	struct nstream *ns;
};


/* list initialization */
void init_docinfolist( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &docinfolist );
}


DECNEW
{
#if USE_SSL
	APTR sslobj;
#endif
	APTR grp, urlobj = NULL, serverobj = NULL, mimeobj = NULL, sourceobj, sizeobj, pagegrp1, pagegrp2 = NULL;
	APTR mailobj, subjectobj, bodyobj;
	char *url;
	APTR dummyobject = MUI_NewObject( MUIC_Area, 0 );
	int sslflag = FALSE;
	int mailtoflag = FALSE;
	struct Data *data;
	struct TagItem *tag = FindTagItem( MA_DocinfoWin_URL, msg->ops_AttrList );

	url = (char *)tag->ti_Data;

	/* check if we already exist */
	for( data = FIRSTNODE( &docinfolist ); NEXTNODE( data ); data = NEXTNODE( data ) )
	{
		if( !strcmp( url, (char *)getv( data->url, MUIA_String_Contents ) ) )
		{
			set( data->obj, MUIA_Window_Activate, TRUE );
			return( 0 );
		}
	}

	if (!(obj = (Object *)DoSuperNew(cl, obj,
									MUIA_Window_Title, GS( DOCINFOWIN_WINTITLE ),
									MUIA_Window_ScreenTitle, copyright,
									MUIA_Window_NoMenus, TRUE,
									MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Moused,
									MUIA_Window_TopEdge, MUIV_Window_TopEdge_Moused,
									MUIA_Background, MUII_RequesterBack,
									MUIA_Window_RootObject, dummyobject,
									MUIA_Window_Width, 320, // workaround since MUI falls back to Topaz/8 or something when it doesn't fit the display ( Text not clipped )
									TAG_DONE))) return(0);

	data = INST_DATA(cl, obj);

	data->obj = obj;

	/* find tags */
	tag = FindTagItem( MA_DocinfoWin_SSL, msg->ops_AttrList );
	if ( tag && tag->ti_Data == TRUE )
		sslflag = TRUE;

	tag = FindTagItem( MA_DocinfoWin_mailto, msg->ops_AttrList );
	if ( tag && tag->ti_Data == TRUE )
		mailtoflag = TRUE;
 

	if ( sslflag )
	{
		grp = RegisterGroup( pages ),
				  MUIA_Register_Frame, TRUE,
				  MUIA_Background, MUII_RegisterBack,
				  Child, pagegrp1 = VGroup, End,
				  Child, pagegrp2 = VGroup, End,
			  End;
	}
	else
		grp = pagegrp1 = VGroup, End;

	
	if(!grp)
	{
		MUI_Request( app, NULL, 0, copyright, GS( CANCEL ), GS( WIN_FAILED ), MUI_Error() );
		CoerceMethod( cl, obj, OM_DISPOSE );
		return( 0 );
	}


	if ( !mailtoflag )
	{
		urlobj    = HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
										MA_Smartlabel_Text, GS( DOCINFOWIN_URL ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SERVER ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_MIMETYPE ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SOURCE ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SIZE ),
									End,
						Child, data->url = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_String_MaxLen, 256,
							MUIA_Textinput_SetMin, FALSE,
						End,
					End;


		serverobj = HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
										MA_Smartlabel_Text, GS( DOCINFOWIN_SERVER ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_URL ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_MIMETYPE ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SOURCE ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SIZE ),
									End,

						Child, data->httpd = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_Textinput_SetMin, FALSE,
						End,
					End;
		
		mimeobj =   HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
									MA_Smartlabel_Text, GS( DOCINFOWIN_MIMETYPE ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SERVER ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_URL ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SOURCE ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SIZE ),
								End,

						Child, data->mimetype = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_String_MaxLen, 40,
							MUIA_Textinput_SetMin, FALSE,
						End,
					End;

		sourceobj = HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
										MA_Smartlabel_Text, GS( DOCINFOWIN_SOURCE ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SERVER ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_MIMETYPE ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_URL ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_SIZE ),
									End,
 
						Child, data->sourceid = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_Textinput_SetMin, FALSE,
						End,
					End;
	 
		sizeobj =   HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
									MA_Smartlabel_Text, GS( DOCINFOWIN_SIZE ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SERVER ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_MIMETYPE ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SOURCE ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_URL ),
								End,
 
						Child, data->size = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_Textinput_SetMin, FALSE,
						End,
					End;

		if ( !urlobj || !serverobj || !mimeobj || !sourceobj || !sizeobj )
		{
			if ( urlobj )
				MUI_DisposeObject( urlobj );

			if ( serverobj )
				MUI_DisposeObject( serverobj );

			if ( mimeobj )
				MUI_DisposeObject( mimeobj );

			if ( sourceobj )
				MUI_DisposeObject( sourceobj );

			if ( sizeobj )
				MUI_DisposeObject( sizeobj );

			CoerceMethod( cl, obj, OM_DISPOSE ); 

			displaybeep();
			return( 0 );
		}

#if USE_SSL
		if ( sslflag )
		{
			sslobj = VGroup,
						 Child, HGroup,
							 
							 Child, NewObject( getsmartlabelclass(), NULL,
									MA_Smartlabel_Text, GS( DOCINFOWIN_SSLVERSION ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SSLCIPHER ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_CERT ),
								End,
 
							 Child, data->sslversion = TextinputObject,
														  TextFrame,
														  MUIA_Background, MUII_TextBack,
														  MUIA_Textinput_NoInput, TRUE,
													  End,
						 End,
						 
						 Child, HGroup,
							 
							 Child, NewObject( getsmartlabelclass(), NULL,
									MA_Smartlabel_Text, GS( DOCINFOWIN_SSLCIPHER ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_CERT ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SSLVERSION ),
								End,
 
							 Child, data->sslcipher = TextinputObject,
														  TextFrame,
														  MUIA_Background, MUII_TextBack,
														  MUIA_Textinput_NoInput, TRUE,
													  End,
						 End,

						 Child, HGroup,
							 
							 Child, NewObject( getsmartlabelclass(), NULL,
									MA_Smartlabel_Text, GS( DOCINFOWIN_CERT ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SSLCIPHER ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SSLVERSION ),
								End,

							 Child, data->ssl = TextinputscrollObject, TextFrame,
													MUIA_Textinput_Editable, FALSE,
													MUIA_Textinput_Multiline, TRUE,
													MUIA_Textinput_WordWrap, FALSE,
												End,
						 End,
					 End;
			
			DoMethod( pagegrp2, OM_ADDMEMBER, ( ULONG )sslobj );
		}
#endif /* USE_SSL */

		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )urlobj );
		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )serverobj );
		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )mimeobj );
		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )sourceobj );
		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )sizeobj );
	}
	else
	{
		char *p, *q;

		mailobj   = HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
									MA_Smartlabel_Text, GS( DOCINFOWIN_MAIL ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_SUBJECT ),
									MA_Smartlabel_MoreText, GS( DOCINFOWIN_BODY ),
								End,

						Child, data->mail = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_Textinput_SetMin, TRUE,
							MUIA_String_MaxLen, 256,
						End,
					End;


		subjectobj = HGroup,
						Child, NewObject( getsmartlabelclass(), NULL,
										MA_Smartlabel_Text, GS( DOCINFOWIN_SUBJECT ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_MAIL ),
										MA_Smartlabel_MoreText, GS( DOCINFOWIN_BODY ),
									End,
 
						Child, data->subject = TextinputObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Textinput_NoInput, TRUE,
						End,
					End;
 

		bodyobj =    HGroup,
											 
					    Child, NewObject( getsmartlabelclass(), NULL,
								MA_Smartlabel_Text, GS( DOCINFOWIN_BODY ),
								MA_Smartlabel_MoreText, GS( DOCINFOWIN_MAIL ),
								MA_Smartlabel_MoreText, GS( DOCINFOWIN_SUBJECT ),
							End,

						Child, data->body = TextinputscrollObject, TextFrame,
											   MUIA_Textinput_Editable, FALSE,
											   MUIA_Textinput_Multiline, TRUE,
											   MUIA_Textinput_WordWrap, FALSE,
						End,
				    End;
 
		if ( !mailobj || !subjectobj || !bodyobj )
		{
			if ( mailobj )
				MUI_DisposeObject( urlobj );

			if ( subjectobj )
				MUI_DisposeObject( serverobj );

			if ( bodyobj )
				MUI_DisposeObject( mimeobj );

			CoerceMethod( cl, obj, OM_DISPOSE ); 

			displaybeep();
			return( 0 );
		}
 
		strcpy( url, strchr( url, ':' ) + 1 );

		/* replace + by spaces */
		p = url;
		
		while( *p )
		{
			if( *p == '+' )
				*p = 0x20;
				p++;
		}

		/* setup the strings, a mailto: looks like that:
		   mailto:porn@vapor.com?Subject=You+Suck&Body=Go+Away */

		if( p = strchr( url, '?' ) ) // we have a subject
		{
			*p = '\0';
			p++;
			p = strchr( p, '=' ) + 1;
			q = p;

			if( p = strchr( p, '&' ) ) // we have a body
			{
				*p = '\0';
				p++;
				p = strchr( p, '=' ) + 1;
				set( data->body, MUIA_String_Contents, p );
			}
			set( data->subject, MUIA_String_Contents, q );
		}
		set( data->mail, MUIA_String_Contents, url );

		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )mailobj );
		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )subjectobj );
		DoMethod( pagegrp1, OM_ADDMEMBER, ( ULONG )bodyobj );

	}

	ADDTAIL( &docinfolist, data );

	set( obj, MUIA_Window_RootObject, grp );
	MUI_DisposeObject( dummyobject );

	D( db_docinfowin, bug( "opening URL %s\n", url ));

	if( !mailtoflag )
	{
		data->ns = nets_open( url, NULL, obj, NULL, NULL, 0, 0 ); /* add ourself as client */
	// HACK! Stream needs a destination, otherwise no notifications are
	// sent after NStream_GotInfo
		nets_settomem( data->ns );

		set( data->url, MUIA_String_Contents, url );
	}

	DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
			 ( ULONG )app, 4, MUIM_Application_PushMethod, ( ULONG )obj, 1, MM_DocinfoWin_Close );

	

	return( (ULONG)obj );
}


DECMETHOD( NStream_GotInfo, APTR)
{
	GETDATA;
	
	D( db_docinfowin, bug("called\n"));

	set( data->httpd, MUIA_String_Contents, data->ns->un->server );
	set( data->mimetype, MUIA_String_Contents,data->ns->un->mimetype );

#if USE_SSL
	if ( data->ns->un->sslcipher )
	{
		set( data->sslcipher, MUIA_String_Contents, data->ns->un->sslcipher );
		set( data->ssl, MUIA_String_Contents, data->ns->un->sslpeercert );
	}

	if( data->ns->un->sslversion )
	{
		set( data->sslversion, MUIA_String_Contents, data->ns->un->sslversion );
	}
#endif /* USE_SSL */

	return( 0 );
}


DECMETHOD( NStream_GotData, APTR )
{
	D( db_docinfowin, bug("called\n"));
	
	DoMethod( obj, MM_DocinfoWin_SetContent, FALSE );

	return( 0 );
}

DECMETHOD( NStream_Done, APTR)
{
	GETDATA;
	D( db_docinfowin, bug("called\n"));

	DoMethod( obj, MM_DocinfoWin_SetContent, TRUE );

	nets_close( data->ns );
	data->ns = NULL;

	return( 0 );
}


/* update the full window */
DECSMETHOD( DocinfoWin_SetContent )
{
	GETDATA;

	switch ( nets_sourceid( data->ns ) )
	{
		case 0:
			set( data->sourceid, MUIA_String_Contents, GS( DOCINFOWIN_SOURCEID_INTERNAL ) );
			break;

		case 1:
			DoMethod( data->sourceid, MUIM_SetAsString, MUIA_Text_Contents,
					  ( ULONG )"%s (%s)", ( ULONG )GS( DOCINFOWIN_SOURCEID_CACHED ),
					  msg->final_update ? ( ULONG )GS( DOCINFOWIN_SOURCEID_DONE ) : ( ULONG )GS( DOCINFOWIN_SOURCEID_INPROGRESS ) );
			break;

		case 2:
			DoMethod( data->sourceid, MUIM_SetAsString, MUIA_Text_Contents,
					  ( ULONG )"%s (%s)", ( ULONG )GS( DOCINFOWIN_SOURCEID_HTTP11 ),
					  msg->final_update ? ( ULONG )GS( DOCINFOWIN_SOURCEID_DONE ) : ( ULONG )GS( DOCINFOWIN_SOURCEID_INPROGRESS ) );
			break;
		case 3:
			DoMethod( data->sourceid, MUIM_SetAsString, MUIA_Text_Contents,
					  ( ULONG )"%s (%s)", ( ULONG )GS( DOCINFOWIN_SOURCEID_HTTP ),
					  msg->final_update ? ( ULONG )GS( DOCINFOWIN_SOURCEID_DONE ) : ( ULONG )GS( DOCINFOWIN_SOURCEID_INPROGRESS ) );
			break;

		case 4:
			DoMethod( data->sourceid, MUIM_SetAsString, MUIA_Text_Contents,
					  ( ULONG )"%s (%s)", ( ULONG )GS( DOCINFOWIN_SOURCEID_FTP ),
					  msg->final_update ? ( ULONG )GS( DOCINFOWIN_SOURCEID_DONE ) : ( ULONG )GS( DOCINFOWIN_SOURCEID_INPROGRESS ) );
			break;
	}
 

	if ( data->ns->un->doclen == -1 )
	{
		if ( data->ns->un->docptr == 0 )
		{
			set( data->size, MUIA_String_Contents, "None" );
		}
		else
		{
			DoMethod( data->size, MUIM_SetAsString, MUIA_Text_Contents,
					  ( ULONG )"%ld", data->ns->un->docptr );
		}
	}
	else
	{
		DoMethod( data->size, MUIM_SetAsString, MUIA_Text_Contents,
				  ( ULONG )"%ld", data->ns->un->docptr );
	}

	return( 0 );
}

DECMETHOD( DocinfoWin_Close, ULONG )
{
	GETDATA;

	if( data->ns ) nets_close( data->ns );
	REMOVE( data );
	killpushedmethods( obj );
	set( obj, MUIA_Window_Open, FALSE );
	DoMethod( app, OM_REMMEMBER, ( ULONG )obj );
	MUI_DisposeObject( obj );
	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFMETHOD( DocinfoWin_Close )
DEFMETHOD( NStream_GotInfo )
DEFMETHOD( NStream_GotData )
DEFMETHOD( NStream_Done )
DEFSMETHOD( DocinfoWin_SetContent )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_docinfowinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "DocInfoWinClass";
#endif

	return( TRUE );
}

void delete_docinfowinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getdocinfowin( void )
{
	return( mcc->mcc_Class );
}

/* create the Document Information Window, set it then open it */
int	createdocinfowin( char *url )
{
	APTR docinfowin;
#if USE_SSL
	int ssltag = FALSE;
#endif /* USE_SSL */
	int mailtotag = FALSE;

	/* scan to reject anything else than those URLs */
	if ( strnicmp( url, "http://", 6 ) )
	{
#if USE_SSL
		if ( strnicmp( url, "https://", 7 ) )
		{
#endif /* USE_SSL */
			if ( strnicmp( url, "mailto:", 7 ) )
			{
				displaybeep();
				return( FALSE );
			}
			else
				mailtotag = TRUE;
#if USE_SSL
		}
		else
			ssltag = TRUE;
#endif /* USE_SSL */
	}
	

	docinfowin = NewObject( getdocinfowin(), NULL,
							MA_DocinfoWin_URL, url,
#if USE_SSL
							MA_DocinfoWin_SSL, ssltag,
#endif /* USE_SSL */
							MA_DocinfoWin_mailto, mailtotag,
							TAG_DONE );
	
	if ( docinfowin )
	{
		DoMethod( app, OM_ADDMEMBER, ( ULONG )docinfowin );
	}
	else
	{
		displaybeep();
	}
	
	if ( docinfowin )
	{
		set( docinfowin, MUIA_Window_Open, TRUE );
	}

	return( TRUE );
}

