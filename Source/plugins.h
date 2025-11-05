/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_PLUGINS_H
#define VOYAGER_PLUGINS_H
/*
 * $Id: plugins.h,v 1.12 2001/07/01 22:03:17 owagner Exp $
 */

#include <libraries/v_plugin.h>

#define VPluginBase plugin->libbase

struct plugin {
	struct MinNode n;
	char name[ 32 ];
	struct Library *libbase;
	struct TagItem *querylist;
	int api_version;
	int hasprefs;
	int prefsobject_exists;
	int hasurlstring;
	int isppc;
	struct vplug_prefs prefs;
	APTR js_obj;
};

extern struct MinList pluginlist;
extern APTR pluginswin;

STRPTR plugin_processurl( char *url, int *size );
APTR plugin_mimetype( STRPTR mimetype, STRPTR mimeextension );
APTR plugin_mimeextension( STRPTR mimeextension, int returnclass );
int plugin_processurlstring( char *url );

#endif /* VOYAGER_PLUGINS_H */
