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


#include <system/types.h>
#include <system/system.h>
#include <system_lib_calls.h>
#include <dos_lib_calls.h>
#include <locale_lib_calls.h>
#include <taglists_lib_calls.h>
#include <graphics_lib_calls.h>
#include <mui_lib_calls.h>
#include <clib_lib_calls.h>
#include <net_lib_calls.h>
#include <regions_lib_calls.h>
#include <inspiration_lib_calls.h>
#include <mcp_lib_calls.h>
#include <env_lib_calls.h>
#include <mbxgui_lib_calls.h>
#include <system/types.h>
#include <system/system.h>
#include <system/sysdata.h>

extern SYSBASE;
extern DOSBASE;  // It's now in _main.c:libcaos.a
LOCALEBASE;
TAGLISTSBASE;
GRAPHICSBASE;
INSPIRATIONBASE;
MUIBASE;
extern CLIBBASE;
REGIONSBASE;
MCPBASE;
ENVBASE;
MBXGUIBASE;

extern CHAR _text[],_etext[],_data[],_edata[],_bss[],_ebss[];

static void closemodules(void)
{
	if (MBXGUIBase) {
		CloseModule((struct Module*)MBXGUIBase);
		MBXGUIBase = 0;
	}
	if (EnvBase) {
		CloseModule((struct Module*)EnvBase);
		EnvBase = 0;
	}
	if( McpBase )
	{
		CloseModule( (struct Module *)McpBase );
		McpBase = NULL;
	}
	if( InspirationBase )
	{
		CloseModule( (struct Module *)InspirationBase );
		InspirationBase = NULL;
	}
	if( RegionsBase )
	{
		CloseModule( (struct Module *)RegionsBase );
		RegionsBase = NULL;
	}
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
}

int openmodules( void )
{
//	LocaleBase = (LocaleData_p) OpenModule( LOCALENAME, LOCALEVERSION );
//	if( !LocaleBase )
//		goto err;

	TagListsBase = (TagListsData_p) OpenModule( TAGLISTSNAME, TAGLISTSVERSION );
	if( !TagListsBase )
		goto err;

	GraphicsBase = (GraphicsData_p) OpenModule( GRAPHICSNAME, GRAPHICSVERSION );
	if( !GraphicsBase )
		goto err;

	MUIBase = (MUIData_p) OpenModule( MUINAME, MUIVERSION );
	if( !MUIBase )
		goto err;

	RegionsBase = (RegionsData_p) OpenModule( REGIONSNAME, REGIONSVERSION );
	if( !RegionsBase )
		goto err;

	InspirationBase = (InspirationData_p) OpenModule( INSPIRATIONNAME, INSPIRATIONVERSION );
	if( !InspirationBase )
		goto err;

	McpBase = (McpData_p) OpenModule( MCPNAME, MCPVERSION );
	if( !McpBase )
		goto err;

	EnvBase =  (EnvData_p) OpenModule (ENVNAME, ENVVERSION);
	if (!EnvBase)
		goto err;

	MBXGUIBase = (MBXGUIData_p) OpenModule (MBXGUINAME, MBXGUIVERSION);
	if (!MBXGUIBase)
		goto err;
	
	return 1;

err:
	closemodules();
	return 0;
}

int vmain (void);

int main(int argc, char **argv)
{
	int rc = 0;

	kprintf	("****************************************************************\n");
	kprintf ("Voyager Executable\n");
	kprintf ("****************************************************************\n");
	
	{
		int i;
		extern ULONG startup_iconified;
		extern ULONG use_mcp;
		extern ULONG iconifySignalsMCP;
		
		use_mcp  = FALSE;
		
		for (i=0;i<argc;i++)
		{
			if (!stricmp(argv[i],"ICONIFIED"))
			{
				startup_iconified = TRUE;
 				kprintf("Startup iconified\n");
			}
			if (!stricmp(argv[i],"MCP"))
			{
				use_mcp = TRUE;
				iconifySignalsMCP = FALSE;
 				kprintf("Use mcp\n");
			}
			
		}
	}

	if (openmodules())
	{
		rc = vmain();
		closemodules();
	}
	return rc;
}
