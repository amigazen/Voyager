#ifndef V_PLUGIN_LIB_MD
#define V_PLUGIN_LIB_MD 1
//==============================================================================
//== Module Description File
//== VPlug generic module
//==============================================================================
//MKTAG
//==============================================================================
//LIBNAME   "v_plugin_dummy.VPlug"  // Name for created module and tag (DUMMY!)
//TYPE      LNT_MODULE  // Define moduletag type
//FLAGS     "0"     // Define moduletag flags
//IDSTR     "V Plugin (generic)\r\nCopyright 2000 Metabox AG"
//CVSID     "$Id: v_plugin_lib.md,v 1.5 2001/05/19 22:33:04 sircus Exp $"
//MODNAME   "Vplugin"          // Name of Module, with uppercase first letter !
//PREFIX    "VPLUG"      // Prefix for call macro
//BASE      "VPluginBase"  // Name of base in macro macro use
//PRI       "0"     // ModuleTag Priority
//VERS      1               // Version and Revision
//REV       1

//INCLUDE  "system/system.h"
//INCLUDE  "system_lib_calls.h"
//INCLUDE  "libraries/v_plugin.h"

//BEGIN
//PUBLIC
struct TagItem * VPLUG_Query();
APTR VPLUG_ProcessURLMethod( STRPTR url );
APTR VPLUG_GetURLData( APTR handle );
STRPTR VPLUG_GetURLMIMEType( APTR handle );
void VPLUG_FreeURLData( APTR handle );
APTR VPLUG_GetClass( STRPTR mimetype );
BOOL VPLUG_Setup( struct vplug_functable *table );
void VPLUG_Cleanup();
void VPLUG_FinalSetup();
void VPLUG_Hook_Prefs( ULONG methodid, struct vplug_prefs *prefs );
BOOL VPLUG_GetInfo( struct VPlugInfo *, APTR nethandle );
int VPLUG_GetURLDataSize( APTR handle );
int VPLUG_ProcessURLString( STRPTR url );
//END
#endif
