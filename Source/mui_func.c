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
 * MUI shortcuts and handy functions
 * ---------------------------------
 * - Please don't add useless stuff here. Use direct MUI macros/functions
 * whenever possible because they're the same for all apps.
 *
 * © 2000 by Vapor CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: mui_func.c,v 1.13 2003/07/06 16:51:34 olli Exp $
 *
*/

#include "voyager.h"

/* private */
#include "mui_func.h"
#include "voyager_cat.h"
#include "menus.h"
#include "textinput.h"


STRPTR mui_getstrptr( APTR obj )
{
	STRPTR p = 0;

	GetAttr( MUIA_String_Contents, obj, (ULONG*)&p );
	return( p );
}

ULONG mui_getv( APTR obj, ULONG attr )
{
	ULONG v;

	GetAttr( attr, obj, &v );
	return( v );
}

int getmenucheck( ULONG menuid )
{
	if( menu )
	{
		APTR mit = (APTR)DoMethod( menu, MUIM_FindUData, menuid );
		if( mit )
			return( (int)getv( mit, MUIA_Menuitem_Checked ) );
	}
	return( FALSE );
	//return( (int)DoMethod( app, MUIM_Application_GetMenuCheck, menuid ) );
}

APTR makebutton( ULONG id )
{
	APTR o = MUI_MakeObject( MUIO_Button, ( ULONG )GSI( id ) );
	if( o )
		set( o, MUIA_CycleChain, 1 );
	return( o );
}


// ***************************************************

#ifdef GETOBJECTDIMENSIONS
void STDARGS getobjectdimensions( APTR class, APTR wobj, int *xsize, int *ysize, ... )
{
	APTR obj;
	va_list va;

	va_start( va, ysize );

#ifdef __MORPHOS__
	obj = ( APTR )NewObjectA( class, NULL, ( struct TagItem * )va->overflow_arg_area );
#else
	obj = ( APTR )NewObjectA( class, NULL, ( struct TagItem * )va );
#endif /* !__MORPHOS__ */
	if( obj )
	{
		muiGlobalInfo( obj ) = muiGlobalInfo( wobj );
		DoMethod( obj, MUIM_GlobalSetup, ( ULONG )muiGlobalInfo( wobj ) );
		_parent( obj ) = wobj;
		muiRenderInfo( obj ) = muiRenderInfo( wobj );
		DoMethod( obj, MUIM_Setup, ( ULONG )muiRenderInfo( wobj ) );
		DoMethod( obj, MUIM_AskMinMax, ( ULONG )NULL );

		//*xsize = max( _minwidth( obj ), _defwidth( obj ) );       
		//*ysize = max( _minheight( obj ), _defheight( obj ) );     
		*xsize = _minwidth( obj );
		*ysize = _minheight( obj );

		DoMethod( obj, MUIM_Cleanup );
		//DoMethod( obj, MUIM_GlobalCleanup );

		MUI_DisposeObject( obj );
	}

	va_end( va );
}
#endif

APTR hbar( void )
{
	return( MUI_MakeObject( MUIO_HBar, 2 ) );
}


/*
 * Get the public screen name
 */
char *getpubname( APTR winobj )
{
#ifndef MBX
	struct PubScreenNode *psn;
	struct Screen *scr;
	struct List *psl;

	get( winobj, MUIA_Window_Screen, &scr );

	psl = LockPubScreenList();

	for( psn = FIRSTNODE( psl ); NEXTNODE( psn ); psn = NEXTNODE( psn ) )
		if( psn->psn_Screen == scr )
			break;

	if( !NEXTNODE( psn ) )
		psn = NULL;

	UnlockPubScreenList();

	return( psn ? psn->psn_Node.ln_Name : "Workbench" );
#else
	return( "Workbench" ); //TOFIX!!
#endif
}


/// ParseHotkey
char ParseHotKey(char *string_num)
{
char *Button;
char Key = '\0';

	if(string_num)
	   {
	   Button = strchr(string_num, '_');

	   if(Button)
		 Key = ToLower(Button[1]);
	   }

	return(Key);
}


APTR button( ULONG label, ULONG helpid )
{
	APTR obj = MUI_MakeObject( MUIO_Button, ( ULONG )GSI( label ) );
	SetAttrs( obj, MUIA_CycleChain, 1, helpid ? MUIA_ShortHelp : TAG_IGNORE, ( ULONG )GSI( helpid ), TAG_DONE );
	return( obj );
}

APTR ebutton( ULONG label, ULONG helpid )
{
	APTR obj = MUI_MakeObject( MUIO_Button, ( ULONG )GSI( label ) );
	SetAttrs( obj, MUIA_CycleChain, 1, helpid ? MUIA_ShortHelp : TAG_IGNORE, ( ULONG )GSI( helpid ), MUIA_Text_SetVMax, FALSE, TAG_DONE );
	return( obj );
}

APTR string( STRPTR def, int maxlen, int sh )
{
	APTR o;

	o = TextinputObject, StringFrame, MUIA_String_MaxLen, maxlen,
		MUIA_String_Contents, def,
		MUIA_CycleChain, 1,
	End;
	return( o );
}

#ifndef __MORPHOS__
APTR STDARGS DoSuperNew( struct IClass *cl, APTR obj, ULONG tag1, ... )
{
	return( (APTR)DoSuperMethod( cl, obj, OM_NEW, &tag1, NULL ) );
}
#endif /* !__MORPHOS__ */

void tickapp( void )
{
	DoMethod( app, MUIM_Application_InputBuffered );
}

//
// function to filter out MUI escape codes except bold
//

#define FILTERBUFSIZE 1024

char *filter_escapecodes( char *src )
{
	static char tmp[ FILTERBUFSIZE ];
	char *dst = tmp;
	int x = 0;

	while( *src && x++< (FILTERBUFSIZE-1) )
	{
		if( *src == 27 )
		{
			if( src[1] != 'b' )
				*dst++ = '*';
			else
				*dst++ = *src;
		}
		else
			*dst++ = *src;

		src++;
	}
	*dst = 0;

	return( tmp );
}


#if 0
void dprintf( char *, ... );

ULONG	DoMethodA(Object	*MyObject,
				  Msg		MyMsg)
{
Class		*MyClass;

  if ( MyObject )
  {
	#if 1
	//if ( MyMsg->MethodID == 0x851bc7e8 || MyMsg->MethodID == 0x851bc748)
	{
		MyClass	=(struct IClass*) (((ULONG*) MyObject)[-1]);
		dprintf( "DoMethodA: Object 0x%lx, Msg 0x%lx, ID: 0x%lx, Class: 0x%lx, Classname: <%s>\n", MyObject, MyMsg, MyMsg->MethodID, MyClass, MyClass->cl_ID );
	}

	#else
	dprintf("DoMethodA: Object 0x%lx Msg 0x%lx ID: 0x%lx\n",
							 MyObject,
							 MyMsg,
							 MyMsg->MethodID);
	#endif
	}
	else
	{
		dprintf( "eek! nullobject!\n" );
	}

  if (MyObject)
  {
	MyClass	=(struct IClass*) (((ULONG*) MyObject)[-1]);

	REG_A0	=(ULONG) MyClass;
	REG_A1	=(ULONG) MyMsg;
	REG_A2	=(ULONG) MyObject;
	return((*MyEmulHandle->EmulCallDirect68k)(MyClass->cl_Dispatcher.h_Entry));
  }
  else
  {
	return(0);
  }
}
#endif


#define DCN(x) char MUIC_##x[]= { #x ".mui" }

DCN( Application );
DCN( Register );
DCN( Cycle );
DCN( Image );
DCN( Popasl );
DCN( String );
DCN( Poppen );
DCN( Slider );
DCN( Scrollgroup );
DCN( Window );
DCN( Floattext );
DCN( Menu );
DCN( Bitmap );
DCN( Virtgroup );
DCN( Scrollbar );
DCN( Notify );
DCN( Area );
DCN( Group );
DCN( Rectangle );
DCN( Popscreen );
DCN( Popobject );
DCN( Balance );
DCN( Text );
DCN( List );
DCN( Gauge );
DCN( Listview );
DCN( Scale );
DCN( Dataspace );
DCN( Menuitem );
DCN( Menustrip );
DCN( Bodychunk );

