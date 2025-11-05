#ifndef CLIB_V_PLUGIN_PROTOS_H
#define CLIB_V_PLUGIN_PROTOS_H

struct vplug_functable;
struct VPlugInfo;
struct vplug_prefs;


/*
 * Voyager plugin definitions
 * --------------------------
 * © 2000-2001 by VaporWare
 *
 * $Id: v_plugin_protos.h,v 1.1.1.1 2001/04/16 16:18:22 zapek Exp $
 *
 */

#include <libraries/v_plugin.h>

/* API V1 */
struct TagItem *VPLUG_Query( void );
APTR VPLUG_ProcessURLMethod( STRPTR url );
APTR VPLUG_GetURLData( APTR handle );
STRPTR VPLUG_GetURLMIMEType( APTR handle );
void VPLUG_FreeURLData( APTR handle );

/* API V2 additions */
APTR VPLUG_GetClass( STRPTR mimetype );
BOOL VPLUG_Setup( struct vplug_functable *table );
void VPLUG_Cleanup( void );
void VPLUG_FinalSetup( void );
void VPLUG_Hook_Prefs( ULONG methodid, struct vplug_prefs *prefs );

/* API V3 additions */
BOOL VPLUG_GetInfo( struct VPlugInfo *, APTR nethandle );
int VPLUG_GetURLDataSize( APTR handle );

/* API V4 additions */
int VPLUG_ProcessURLString( STRPTR url );

#endif /* !CLIB_V_PLUGIN_PROTOS_H */
