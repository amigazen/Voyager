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

#ifdef VDEBUG

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <devices/inputevent.h>
#include <proto/exec.h>
#endif

int db_auth;
int db_cache;
int db_cookie;
int db_dns;
int db_docinfowin;
int db_dlwin;
int db_ftp;
int db_history;
int db_http;
int db_js;
int db_net;
int db_plugin;
int db_mail;
int db_cacheprune;
int db_html;
int db_gui;
int db_init;
int db_forceborder;
int db_rexx;
int db_css;
int db_misc;

int db_level = 1;

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
