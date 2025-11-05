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
** $Id: mbxinit.c,v 1.8 2003/07/06 16:51:34 olli Exp $
*/

#include <system/types.h>
#include <system/system.h>
#include <system_lib_calls.h>
#include <dos_lib_defs.h>
#include <locale_lib_defs.h>
#include <taglists_lib_defs.h>
#include <graphics_lib_defs.h>
#include <timer_drv_defs.h>
#include <mui_lib_defs.h>
#include <clib_lib_defs.h>
#include <voyager_lib_defs.h>
#include "mbx.h"
#include <mbxgui_lib_calls.h>
#include <modules/mbxgui/classes.h>

VoyagerData_p VLibBase;

//TOFIX!!! HACK!!

int _exit(void)
{
KPrintF( "Came to _exit()" );
while(1);
return 0;
}

//TOFIX!!! HACK!!

int sbrk(void)
{
KPrintF( "Came to sbrk()" );
while(1);
return 0;
}

static void closemodules()
{
/* Done in startup code....
	if( FileIOBase )
	{
		CloseModule( (struct Module *)FileIOBase ); 
		FileIOBase = NULL;
	}
*/
	if( LocaleBase )
	{
		CloseModule( (struct Module *)LocaleBase ); 
		LocaleBase = NULL;
	}
	if( TagListsBase )
	{
		CloseModule( (struct Module *)TagListsBase ); 
		TagListsBase = NULL;
	}
	if( GraphicsBase )
	{
		CloseModule( (struct Module *)GraphicsBase ); 
		GraphicsBase = NULL;
	}
	if( MUIBase )
	{
		CloseModule( (struct Module *)MUIBase ); 
		MUIBase = NULL;
	}
	if( CLibBase )
	{
		CloseModule( (struct Module *)CLibBase );
		CLibBase = NULL;
	}

 	SysBase = NULL;
}

Module_p Voyager_modInit( struct SystemData *sysbase, Module_p modbase, MemList_p memList)
{
	VLibBase = (VoyagerData_p)modbase;

	SysBase = sysbase;

	FileIOBase = (FileIOData_p) OpenModule( FILEIONAME, FILEIOVERSION );
	//if( !FileIOBase )
	//	goto err;

	LocaleBase = (LocaleData_p) OpenModule( LOCALENAME, LOCALEVERSION );
	if( !LocaleBase )
		goto err;

	TagListsBase = (TagListsData_p) OpenModule( TAGLISTSNAME, TAGLISTSVERSION );
	if( !TagListsBase )
		goto err;

	GraphicsBase = (GraphicsData_p) OpenModule( GRAPHICSNAME, GRAPHICSVERSION );
	if( !GraphicsBase )
		goto err;

/* !!! TOFIX: OpenDriver, not OpenModule. Requires IORequest & port stuff though :/. TimerBase might not be used at all anyway!
	TimerBase = (MUIData_p) OpenModule( TIMERNAME, TIMERVERSION );
	if( !TimerBase )
		goto err;
*/

	MUIBase = (MUIData_p) OpenModule( MUINAME, MUIVERSION );
	if( !MUIBase )
		goto err;

	CLibBase = (CLibData_p) OpenModule( CLIBNAME, CLIBVERSION );
	if ( !CLibBase )
		goto err;

	return modbase;

err:
	closemodules();
 	return(NULL);
}

Module_p Voyager_modExpunge( struct SystemData *sysbase, VPTR VBase )
{
	closemodules();
	return (Module_p)VBase;
}

Module_p Voyager_modOpen( struct SystemData *sysbase, VPTR VBase )
{
	return (Module_p)VBase;
}

Module_p Voyager_modClose( struct SystemData *sysbase, VPTR VBase )
{
	return (Module_p)VBase;
}


