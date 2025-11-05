#pragma pack(2)
#pragma pack(2)
#ifndef V_PLUGIN_LIB_H
#define V_PLUGIN_LIB_H

/*
** Voyager Plugin definitions
** ==========================
**
** (C) 1997-2001 Oliver Wagner <owagner@vapor.com> & David Gerber <zapek@vapor.com>
** All Rights Reserved
**
** $Id: v_plugin.h,v 1.5 2001/08/09 01:25:16 zapek Exp $
**
** Revision 2 (12-10-97)
** ---------------------
** - extended comments a bit
**
** Revision 3 (04-07-99)
** ---------------------
** - added VPLUG_Query_APIVersion
** - added VPLUG_Query_RegisterMIMEType
** - added function table
**
** Revision 4 (09-07-99)
** ---------------------
** - added vplug_domethoda()
**
** Revision 5 (18-07-99)
** ---------------------
** - added vplug_seturl()
** - added vplug_mergeurl()
** - added VPLUG_EmbedInfo_ParentURL
** - added VPLUG_EmbedInfo_Baseref
**
** Revision 6 (23-01-00)
** ---------------------
** - added VPLUG_Query_RegisterMIMEExtension
** - added VPLUG_GetInfo()
** - fixed for newer GCC and vbcc (Emmanuel Lesueur)
**
** Revision 7 (04-03-00)
** ---------------------
** - added VPLUG_Query_HasURLMethodGetSize
**   and the related plugin entry point
**
** Revision 8 (18-08-00)
** ---------------------
** - added VPLUG_ProcessURLString()
**   Allows plugins to process and rewrite the textual URL
**   entered by the user in the window URL gadget
**
** Revision 9 (12-01-01)
** ---------------------
** - it's now possibly to specify VPLUG_Query_PluginID
**   multiple times
**
** Revision 10 (09-08-01)
** ----------------------
** - added VPLUG_Query_PPC_DirectCallbacks and PPC support
*/

#ifndef __reg
#ifdef _DCC
#define __reg(x,y) __ ## x y
#elif defined __GNUC__
#ifdef __MORPHOS__
#define __reg(x,y) y
#else
#define __reg(x,y) y __asm__(#x)
#endif /* !__MORPHOS__ */
#elif defined __DCC__
#define __reg(x,y) y
#else
#define __reg(x,y) register __ ## x y
#endif
#endif

#ifndef PLFUNC
#if defined __MAXON__ || defined __STORM__ || defined _DCC || defined __GNUC__ || defined __DCC__
#define PLFUNC
#else
#define PLFUNC __asm
#endif
#endif

#ifndef SAVEDS
#if defined __MAXON__ || defined __GNUC__ || defined __DCC__
#define SAVEDS
#endif
#if defined __STORM__ || defined __SASC
#define SAVEDS __saveds
#endif
#ifdef _DCC
#define SAVEDS __geta4
#endif
#endif

#ifndef MBX
#pragma pack()
#pragma pack()
#include <exec/types.h>
#pragma pack(2)
#pragma pack(2)
#pragma pack()
#pragma pack()
#include <utility/tagitem.h>
#pragma pack(2)
#pragma pack(2)
#pragma pack()
#pragma pack()
#include <utility/hooks.h>
#pragma pack(2)
#pragma pack(2)
#endif

#define VPLUG_TAGBASE (TAG_USER+0x87112)

/*
** VPLUG_Query() is supposed to return a static TagList
** which describes the ability and requirements of
** a plugin.
**
** Everything is described in the autodocs
** 
*/

/*
** V1 = Voyager 2.95 and before
** V2 = Voyager 3.0
** V3 = Voyager 3.1
** V4 = Voyager 3.3
** V5 = Voyager 3.3.97
*/
#define VPLUG_API_VERSION 5

#define VPLUG_QUERYBASE (VPLUG_TAGBASE+100)

#define VPLUG_Query_Version (VPLUG_QUERYBASE+0)                 /* ULONG */
#define VPLUG_Query_Revision (VPLUG_QUERYBASE+1)                /* ULONG  */
#define VPLUG_Query_Copyright (VPLUG_QUERYBASE+2)               /* STRPTR */
#define VPLUG_Query_Infostring (VPLUG_QUERYBASE+3)              /* STRPTR */
#define VPLUG_Query_APIVersion (VPLUG_QUERYBASE+6)	            /* ULONG */
#define VPLUG_Query_HasPrefs (VPLUG_QUERYBASE+7)                /* LONG */
#define VPLUG_Query_PluginID (VPLUG_QUERYBASE+8)                /* STRPTR */
#define VPLUG_Query_RegisterURLMethod (VPLUG_QUERYBASE+4)       /* STRPTR */
#define VPLUG_Query_HasURLMethodGetSize (VPLUG_QUERYBASE+10)    /* BOOL */
#define VPLUG_Query_RegisterMIMEType (VPLUG_QUERYBASE+5)        /* STRPTR */
#define VPLUG_Query_RegisterMIMEExtension (VPLUG_QUERYBASE+9)   /* STRPTR */
#define VPLUG_Query_HasProcessURLString (VPLUG_QUERYBASE+11)    /* BOOL */
#define VPLUG_Query_PPC_DirectCallbacks (VPLUG_QUERYBASE+12)    /* BOOL (V5) */

/*
** Arguments passed to <EMBED>
** ArgNames is an array of STRPTR with the names,
** ArgValues is an array of STRPTR with the values, or ""
** ArgCnt is the number of arguments
*/
#define VPLUG_EmbedInfo_ArgNames (VPLUG_QUERYBASE+4002)
#define VPLUG_EmbedInfo_ArgValues (VPLUG_QUERYBASE+4004)
#define VPLUG_EmbedInfo_ArgCnt (VPLUG_QUERYBASE+4005)
#define VPLUG_EmbedInfo_ParentURL (VPLUG_QUERYBASE+4006)
#define VPLUG_EmbedInfo_Baseref (VPLUG_QUERYBASE+4007)

/*
** The complete URL of the SRC (<EMBED SRC="...">)
*/
#define VPLUG_EmbedInfo_URL (VPLUG_QUERYBASE+4000)              /* STRPTR */

/*
** The complete URL of the page containing the <EMBED>
*/
#define VPLUG_EmbedInfo_Container (VPLUG_QUERYBASE+4001)		/* STRPTR */


/* APTR handle to network stream, already opened. Do NOT vplug_nets_close()
** it! The plugin can call vplug_settofile() or vplug_settomem() to
** get the file to whereever it wants the data. The network handler
** then does send VPLUG_NetStream_GotData and VPLUG_NetStream_GotDone
** methods to the embedded objects, informing of download progress.
*/
#define VPLUG_EmbedInfo_NetStream (VPLUG_QUERYBASE+4003)

/*
** Those are methods sent to the plugin.
** GotInfo is sent when V got all the header of the stream,
** GotData is sent everytime V gets one chunk of data and
** GotDone is sent when V is done with the loading of the
** stream
*/
#define VPLUG_NetStream_GotInfo 0x851ba045
#define VPLUG_NetStream_GotData 0x851ba046
#define VPLUG_NetStream_GotDone 0x851ba047

/*
** Return codes from VPLUG_ProcessURLString()
**
** Plugins can return any of the following codes:
**
*/
/*
** No change occured
*/
#define VPLUG_ProcessURLString_NoChange 0
/*
** Plugin rewrote URL in place, proceed with the
** rewritten URL
*/
#define VPLUG_ProcessURLString_Rewritten 1
/*
** URL was handled completely, go back to previous
** entry, do not reload
*/
#define VPLUG_ProcessURLString_Handled -1


/****************************************************************************/

/*
** Callback table
** If a plugin implements API spec >= 2, V will call
** VPLUG_Setup() with a pointer to this
** object. The plugin can call functions in that
** table anytime.
*/
#define VPLUG_FUNCTABVERSION 6

struct vplug_functable 
{
	int vplug_functabversion;

	APTR context; /* ***PRIVATE!!! DO NOT TOUCH!!!*** */

	APTR (PLFUNC * vplug_net_openurl)(
			__reg(a0, STRPTR url),
			__reg(a1, STRPTR referer),
			__reg(a2, APTR informobj),
			__reg(d0, int reload)
	);

	int (PLFUNC * vplug_net_state)(
			__reg(a0, APTR nethandle)
	);

	void (PLFUNC * vplug_net_close)(
			__reg(a0, APTR nethandle)
	);

	STRPTR (PLFUNC * vplug_net_mimetype)(
			__reg(a0, APTR nethandle)
	);

	APTR (PLFUNC * vplug_net_getdocmem)(
			__reg(a0, APTR nethandle)
	);

	int (PLFUNC * vplug_net_getdocptr)(
			__reg(a0, APTR nethandle)
	);
	
	int (PLFUNC * vplug_net_getdoclen)(
			__reg(a0, APTR nethandle)
	);

	void (PLFUNC * vplug_net_settomem)(
			__reg(a0, APTR nethandle)
	);

	void (PLFUNC * vplug_net_settofile)(
			__reg(a0, APTR nethandle),
			__reg(a1, STRPTR filename),
			__reg(d0, int resume)
	);

	STRPTR (PLFUNC * vplug_net_redirecturl)(
			__reg(a0, APTR nethandle)
	);
	
	STRPTR (PLFUNC * vplug_net_url)(
			__reg(a0, APTR nethandle)
	);

	void (PLFUNC * vplug_net_lockdocmem)(void);
	
	void (PLFUNC * vplug_net_unlockdocmem)(void);

	void (PLFUNC * vplug_net_abort)(
			__reg(a0, APTR nethandle)
	);

	STRPTR (PLFUNC * vplug_net_errorstring)(			
			__reg(a0, APTR nethandle)
	);

	int (PLFUNC * vplug_domethoda)(
		__reg(a0, APTR obj),
		__reg(a1, APTR Msg)
	);

	void (PLFUNC * vplug_seturl)(
		__reg(a0, STRPTR url),
		__reg(a1, STRPTR target),
		__reg(d0, int reload)
	);

	void (PLFUNC * vplug_mergeurl)(
		__reg(a0, STRPTR url),
		__reg(a1, STRPTR partial),
		__reg(a2, STRPTR dest)
	);
	
	void (PLFUNC * vplug_colorspec2rgb)(
		__reg(a0, STRPTR colorspec),
		__reg(d0, ULONG *red),
		__reg(d1, ULONG *green),
		__reg(d2, ULONG *blue)
	);
};

/*
 * Network handle states
 */
	/* states */
#define	VNS_FAILED 	  -1
#define VNS_INPROGRESS 0
#define VNS_DONE	   1
 

/****************************************************************************/

/*
** holds your plugin's custom preference page information
** A pointer to this structure, which is already allocated for you 
** by Voyager, is passed to the VPLUG_Hook_Prefs() function.
*/
struct vplug_prefs {
	char *label;			/* list item label, defaults to plugin name */
	struct BitMap *bitmap;	/* 24x14 list icon bitmap, defaults to plugin image */
	APTR colormap;			/* bitmap's colormap, defaults to MWB palette (8 col.) */
	APTR object;			/* preferences object */
};

/* 
** These are the methods you are expected to handle in _Hook_Prefs()
** whenever Voyager wants to know or do stuff with your prefs.
** The description of the method id is what Voyager expects you to
** do or what to return when it is requested. Check the autodocs for more infos
*/
enum {
	VPLUGPREFS_first = 16384,
	VPLUGPREFS_Setup,
	VPLUGPREFS_Cleanup,
	VPLUGPREFS_Create,
	VPLUGPREFS_Dispose,
	VPLUGPREFS_Use,
	VPLUGPREFS_Load,
	VPLUGPREFS_Save,
};

/* where to store the settings */
#define VPLUGPREFSPATH "PROGDIR:Plugins/Data"


/****************************************************************************/

/*
** Plugin library calls
*/

/*
 * Embedding objects, asking for size. This structure might extend in the future.
 * Just modify the fields of VPlugInfo if you need to.
 */
struct VPlugInfo {
	ULONG x;			/* horizontal size */
	ULONG y;   			/* vertical size */
	STRPTR wintitle;    /* title of the window */
};

#endif /* V_PLUGIN_H */

#pragma pack()
#pragma pack()
