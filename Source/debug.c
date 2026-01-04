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
** Debugging vars
**
** $Id: debug.c,v 1.22 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public - always define these, even if VDEBUG is not set */
int db_auth = 0;
int db_cache = 0;
int db_cookie = 0;
int db_dns = 0;
int db_docinfowin = 0;
int db_dlwin = 0;
int db_ftp = 0;
int db_history = 0;
int db_http = 0;
int db_js = 0;
int db_net = 0;
int db_plugin = 0;
int db_mail = 0;
int db_cacheprune = 0;
int db_html = 0;
int db_gui = 0;
int db_init = 0;
int db_forceborder = 0;
int db_rexx = 0;
int db_css = 0;
int db_misc = 0;

int db_level = 1;

#ifdef VDEBUG

#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <devices/inputevent.h>
#include <proto/exec.h>
#endif

#ifndef MBX
#include <proto/input.h>
struct Library *InputBase; /* grr. CBM's headers are buggy */
struct MsgPort *mp;
struct IOStdReq *io;

void init_debug( void )
{
	if( mp = CreateMsgPort() )
	{
		if( io = ( struct IOStdReq * )CreateIORequest( mp, sizeof( struct IOStdReq ) ) )
		{
			if( !( OpenDevice( "input.device", 0, ( struct IORequest *)io, 0 ) ) )
			{
#ifdef __MORPHOS__
				InputBase = ( struct Device * )io->io_Device;
#else
				InputBase = ( struct Library * )io->io_Device;
#endif /* !__MORPHOS__ */

				if( PeekQualifier() & ( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | IEQUALIFIER_CAPSLOCK ) )
				{
					db_init = TRUE;
				}

				CloseDevice( ( struct IORequest * )io );
				DeleteIORequest( io );
				DeleteMsgPort( mp );
			}
		}
	}
}

void dump_image(UBYTE *p, ULONG size, ULONG width)
{
	ULONG i = 0;

	while (size--)
	{
		if (!(i % width))
		{
			kprintf("\n");
		}
		kprintf("%02lx ", (ULONG)(*p++));
		i++;
	}
	kprintf("\n");
}

#endif /* !MBX */

#endif
