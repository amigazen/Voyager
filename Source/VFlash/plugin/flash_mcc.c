/*
** VFlash MUI custom class
** -----------------------
** - Shockwave Flash display class
**
** © 1999-2000 by David Gerber <zapek@vapor.com>
** All rights reserved
**
** Unregistered MUI class, serial number: 4023706747 (well, I use Olli's number :)
**
** $Id: flash_mcc.c,v 1.6 2004/02/29 17:04:14 zapek Exp $
**
*/

#include "config.h"

/* OS */
#include <proto/exec.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <proto/dos.h>
#ifdef __SASC
#include <dos.h>
#else
#include <clib/alib_protos.h>
#endif
#include <dos/dostags.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

/* GFX card support */
#if USE_CGX
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
extern struct Library *CyberGfxBase;
#endif /* USE_CGX */

#if USE_PICASSO96
#include <proto/picasso96.h>
extern struct Library *P96Base;
#endif /* USE_PICASSO96 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Own includes */
#include "rev.h"
#include <macros/vapor.h>
#include "debug.h"
#include "flash_mcc_private.h"
#include "flash.h"


/* PPC */
#ifdef POWERUP
#include <powerup/ppclib/interface.h>
#include <powerup/ppclib/message.h>
#include <powerup/ppclib/tasks.h>
#include <powerup/ppclib/memory.h>
#include <powerup/proto/ppc.h>
extern struct Library *PPCLibBase;
#endif /* POWERUP */

#ifndef MUI_EHF_GUIMODE
#define MUI_EHF_GUIMODE (1<<1)
#endif

/*
 *Support functions
 */
ULONG getv(APTR obj, ULONG attr)
{
	ULONG v;

	GetAttr(attr, obj, &v);
	return (v);
}

/*
 * c2p support
 */
#if USE_PLANAR
extern void ASM writechunky(
	__reg(a0, UWORD *canvas),
	__reg(a1, UBYTE **planes),
	__reg(d0, ULONG pixels),
	__reg(d1, int numplanes),
	__reg(a2, UBYTE *ctable)
);
#endif /* USE_PLANAR */

#if USE_CLUT
#ifdef __SASC
extern void ASM makeclut(
	__reg(a0, UBYTE *clut),
	__reg(a1, UBYTE *rgbmap),
	__reg(a2, UWORD *src),
	__reg(d0, int size)
);
#else
void dprintf(char *, ...);

void makeclut(UBYTE *clut, UBYTE *rgbmap, UWORD *src, int size)
{
	/* XXX: not sure it works.. check with a CLUT screen */
	#if 0
	while (size)
	{

		size--;
	}
	#endif
	dprintf("clut mode not implemented yet\n");
}
#endif
#endif /* !USE_CLUT */

extern struct vplug_functable *ft;
extern void Fail(STRPTR txt);

#ifdef __SASC
extern STDARGS DoSuperNew(struct IClass *class, APTR obj, ULONG tag1, ...);
#endif

char vflash_cpuid[10];

DEFNEW
{
	struct Data *data;
	struct TagItem *tag;
	STRPTR *argnames = 0;
	STRPTR *argvalues = 0;
	ULONG argcnt = 0;
	ULONG i;

	if (!(obj = (Object *)DoSuperNew(cl, obj,
		InnerSpacing(0, 0),
		MUIA_FillArea, FALSE,
		MUIA_Font, MUIV_Font_Tiny,
		TAG_MORE, msg->ops_AttrList
	)))
	return (0);

	data = INST_DATA(cl, obj);

	muiUserData(obj) = (ULONG)data; /* so the process might access us directly on shutdown */

	DB(("about to create new object..\n"));

#if USE_PICASSO96
	if (!(data->ri = AllocVec(sizeof(struct RenderInfo), MEMF_CLEAR)))
	{
		CoerceMethod(cl, obj, OM_DISPOSE);
		return (0);
	}
#endif /* USE_PICASSO96 */

	if (data->nethandle = (char *)GetTagData(VPLUG_EmbedInfo_NetStream, NULL, msg->ops_AttrList))
	{
		DB(("got nethandler 0x%lx\n", (ULONG)data->nethandle));
		data->baseref = (char *)GetTagData(VPLUG_EmbedInfo_Baseref, NULL, msg->ops_AttrList);

		ft->vplug_net_settomem(data->nethandle); /* otherwise we don't get our progress methods */

		data->length = ft->vplug_net_getdoclen(data->nethandle);

		if (data->length > 0)
		{
			sprintf(data->status, "Loading movie (%ld.%ld KB)...", (LONG)(data->length + 1023) / 1024, (LONG)(data->length / 103) % 10);
		}
		else
		{
			strcpy(data->status, "Loading movie...");
		}

	}

#if POWERUP
	if (PPCLibBase)
	{
		/*
		 * The M68K puts events to be read by the PPC process
		 * -> no caching
		 */
		data->ez = PPCAllocMem(sizeof(struct eventzone), MEMF_PUBLIC | MEMF_NOCACHEPPC | MEMF_NOCACHEM68K);
		data->ez->eventpool = PPCCreatePool(MEMF_ANY | MEMF_NOCACHEPPC | MEMF_NOCACHEM68K, sizeof(struct flashevent) * 32, sizeof(struct flashevent) * 16);
	}
	else
#endif /* POWERUP */
	{
		data->ez = AllocMem(sizeof(struct eventzone), MEMF_PUBLIC);
		data->ez->eventpool = CreatePool(MEMF_ANY, sizeof(struct flashevent) * 32, sizeof(struct flashevent) * 16);
	}

	if (!data->ez || !data->ez->eventpool)
	{
		CoerceMethod(cl, obj, OM_DISPOSE);
		return (0);
	}

	NEWLIST(&data->ez->eventlist);
	InitSemaphore(&data->ez->eventsem);

	DB(("new object created\n"));

	/*
	 * Now getting some options in the <EMBED>
	 */
	if (tag = FindTagItem(VPLUG_EmbedInfo_ArgNames, msg->ops_AttrList))
	{
		argnames = (STRPTR *)tag->ti_Data;
	}

	if (tag = FindTagItem(VPLUG_EmbedInfo_ArgValues, msg->ops_AttrList))
	{
		argvalues = (STRPTR *)tag->ti_Data;
	}

	if (tag = FindTagItem(VPLUG_EmbedInfo_ArgCnt, msg->ops_AttrList))
	{
		argcnt = tag->ti_Data;
	}

	for (i = 0; i < argcnt; i++)
	{
		if (stricmp(argnames[i], "BGCOLOR") == 0)
		{
			/*
			 * BGCOLOR - Background color
			 */
			ft->vplug_colorspec2rgb(argvalues[i], &data->bg_red, &data->bg_green, &data->bg_blue);
		}
	}

	return ((ULONG)obj);
}

#define BMF_ALIGN4K 0x40

DEFTMETHOD(Flash_AllocCanvas)
{
	GETDATA;
	/*
	 * Allocate the canvas
	 */

	data->width = _mwidth( obj );
	data->height = _mheight( obj );
	data->wh = data->width * data->height;

/* TOFIX: the rest is a fucking mess */

#if USE_PLANAR && USE_CLUT
	if (data->gfxmode == MV_Flash_ChunkyCLUTMode || data->gfxmode == MV_Flash_PlanarMode)
#elif USE_PLANAR
	if (data->gfxmode == MV_Flash_PlanarMode)
#elif USE_CLUT
	if (data->gfxmode == MV_Flash_ChunkyCLUTMode)
#endif
	{
#ifdef POWERUP
		if (PPCLibBase)
		{
			data->canvas = PPCAllocVec( data->wh * 2, MEMF_CLEAR | MEMF_PUBLIC);
		}
		else
#endif /* POWERUP */
		{
			data->canvas = AllocVec( data->wh * 2, MEMF_CLEAR | MEMF_PUBLIC);
		}
	}
#if USE_PICASSO96
#if (USE_PLANAR || USE_CLUT)
	else if (P96Base)
#else
	if (P96Base)
#endif
	{
		DB(("allocating P96 canvas buffer...(size_x == %lu, size_y == %lu\n", getv(obj, MUIA_Width), getv(obj, MUIA_Height)));
#ifdef POWERUP
		if (PPCLibBase)
		{
			data->ri->Memory = data->canvas = PPCAllocVec(getv(obj, MUIA_Width) * getv(obj, MUIA_Height) * /*data->bpp*/ 2, MEMF_CLEAR | MEMF_PUBLIC);
		}
		else
#endif /* POWERUP */
		{
			data->ri->Memory = data->canvas = AllocVec(getv(obj, MUIA_Width) * getv(obj, MUIA_Height) * /*data->bpp*/ 2, MEMF_CLEAR | MEMF_PUBLIC);
		}
	}
#endif

#if (USE_PLANAR || USE_CLUT || USE_PICASSO96) && USE_CGX
	//else if (CyberGfxBase)
	if (CyberGfxBase)
#elif USE_CGX
	if (CyberGfxBase)
#endif
	{
		DB(("allocating CGFX canvas buffer...(size_x == %lu, size_y == %lu\n", getv(obj, MUIA_Width), getv(obj, MUIA_Height)));
#ifdef POWERUP
		if (PPCLibBase)
		{
			data->canvas = (struct BitMap *)AllocBitMap(getv(obj, MUIA_Width), getv(obj, MUIA_Height), 16, BMF_CLEAR | BMF_MINPLANES | BMF_ALIGN4K | BMF_SPECIALFMT | SHIFT_PIXFMT(PIXFMT_RGB16), NULL);
		}
		else
#endif /* POWERUP */
		{
			/* XXX: speed up possible here by allocating in vram.. hm, ask cyfm perhaps */
			data->canvas = (struct BitMap *)AllocBitMap(getv(obj, MUIA_Width), getv(obj, MUIA_Height), 16, BMF_CLEAR | BMF_MINPLANES | BMF_SPECIALFMT | SHIFT_PIXFMT(PIXFMT_RGB16), NULL);
		}
	}
	DB(("allocation succeeded, canvas at %lx\n", data->canvas));
	return ((ULONG)data->canvas);
}


DEFMMETHOD(AskMinMax)
{
	DOSUPER;
	msg->MinMaxInfo->MaxWidth  += MUI_MAXMAX;
	msg->MinMaxInfo->MaxHeight += MUI_MAXMAX;
	return(0);
}

#if USE_CLUT
static LONG findpen(LONG *real_rgb, int numcols, LONG r, LONG g, LONG b)
{
	ULONG lsq = 0xffffffff;
	ULONG sq,dr,dg,db,i;
	LONG p = -1;

	for (i=0;i<numcols;i++)
	{
		dr = r - real_rgb[i * 3 + 0];
		dg = g - real_rgb[i * 3 + 1];
		db = b - real_rgb[i * 3 + 2];

		sq = (dr*dr)+(dg*dg)+(db*db);

		if (sq<lsq)
		{
			lsq=sq;
			p=i;
			if (!lsq) break;
		}
	}

	return (p);
}
#endif /* USE_CLUT */


/*
 * I get my size there btw (see plugin api) to implement
 * that one day...
 */
DEFMMETHOD(Setup)
{
	GETDATA;
	ULONG bpp = 1;
	ULONG red;
	ULONG green;
	ULONG blue;
	struct ColorMap *cmap = _screen(obj)->ViewPort.ColorMap;
	struct BitMap *bm = _rp(obj)->BitMap;

	DB(("setup...\n"));

	if (!DOSUPER)
	{
		return(FALSE);
	}

	data->shutting_down = FALSE;

	/*
	 * Allocate pens
	 */
	red = (data->bg_red << 24) | (data->bg_red << 16) | (data->bg_red << 8) | data->bg_red;
	green = (data->bg_green << 24) | (data->bg_green << 16) | (data->bg_green << 8) | data->bg_green;
	blue = (data->bg_blue << 24) | (data->bg_blue << 16) | (data->bg_blue << 8) | data->bg_blue;

	data->pen_bg = ObtainBestPen(cmap,
		red, green, blue, TAG_DONE
	);
	data->pen_fg = ObtainBestPen(cmap,
		red ^ ~0, green ^ ~0, blue ^ ~0, TAG_DONE
	);
	data->pen_bar = ObtainBestPen(cmap,
		0, 0xaaaaaaaa, 0, TAG_DONE
	);

	if (data->pen_bg == -1 || data->pen_fg == -1 || data->pen_bar == -1)
	{
		Fail("Not enough free pens.\n");
		return (FALSE);
	}

	/*
	 * Find out about the pixel format
	 */
	data->gfxmode = MV_Flash_PlanarMode; /* TOFIX: well, rewrite the following */

#if USE_CGX
	if (CyberGfxBase)
	{
		if (GetCyberMapAttr(bm, CYBRMATTR_ISCYBERGFX))
		{
			bpp = GetCyberMapAttr(bm, CYBRMATTR_BPPIX);
			data->pixfmt = GetCyberMapAttr(bm, CYBRMATTR_PIXFMT);
			data->gfxmode = MV_Flash_CGFXMode;

#if USE_PICASSO96
			if (P96Base) /* P96 uses different format values */
			{
				if (p96GetBitMapAttr(bm, P96BMA_ISP96))
				{
					data->gfxmode = MV_Flash_P96Mode;
					p96UnlockBitMap(bm, p96LockBitMap(bm, (UBYTE *)data->ri, sizeof(struct RenderInfo))); /* fill-in the RenderInfo */
				}
			}
#endif /* USE_PICASSO96 */
		}
	}
#endif /* USE_CGX */

	DB(("bpp=%ld\n",bpp));

	if(bpp == 1)
	{
		/*
		 * CLUT mode - build a colormap cube
		 */
		LONG r,g,b;
		ULONG real_rgb[ 256 * 3 ];
		int depth = bm->Depth;
		int numcols = 1L<<depth;

		for (r=0; r < 8 ; r++)
		{
			for (g=0; g < 8 ; g++)
			{
				for (b=0; b < 4 ; b++)
				{
					data->cmp8[(r<<5)|(g<<2)|b] = ObtainBestPen(cmap,
						r << 29 | r << 26 | r << 23,
						g << 29 | r << 26 | r << 23,
						b << 30 | b << 28 | b << 26 | b << 24,
						OBP_Precision, PRECISION_EXACT,
						TAG_DONE
					);
				}
			}
		}

		// Find real colors on screen
		data->rgb16_to_8 = AllocMem( 65536, 0 );
		DB(("getting real color map (%ld)\n",numcols));
		GetRGB32(cmap, 0, numcols, real_rgb );
		for( r = 0; r < 256 * 3; r++ )
			real_rgb[ r ] >>= 24;

#define RGBADDR(r,g,b) ((r)<<11)|((g)<<5)|(b)

//#define COLOR_HIRES
#ifdef COLOR_HIRES
		for (r=0; r < 32; r++)
		{
			DB(("r=%ld\n",r));
			for (g=0; g < 64 ; g++)
			{
				for (b=0; b < 32 ; b++)
				{
					data->rgb16_to_8[ RGBADDR(r,g,b) ] = findpen( (LONG*)real_rgb, numcols, r<<3, g<<2, b<<3 );
				}
			}
		}
#else
		for (r=0; r < 32; r += 2)
		{
			DB(("r=%ld\n",r));
			for (g=0; g < 64 ; g += 2)
			{
				for (b=0; b < 32 ; b += 2)
				{
					int pen = findpen( (LONG*)real_rgb, numcols, r<<3, g<<2, b<<3 );
					data->rgb16_to_8[ RGBADDR(r,g,b) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+1,g+0,b+0) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+1,g+1,b+0) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+1,g+1,b+1) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+1,g+0,b+1) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+0,g+0,b+0) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+0,g+1,b+0) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+0,g+1,b+1) ] = pen;
					data->rgb16_to_8[ RGBADDR(r+0,g+0,b+1) ] = pen;
				}
			}
		}
#endif

		// If not Planar, it's an CGFX/P96 chunky screen
		if( data->gfxmode != MV_Flash_PlanarMode )
			data->gfxmode = MV_Flash_ChunkyCLUTMode;
		DB(("done with this\n"));
	}

	/*
	 * Handle mouse movements & buttons.
	 * Keyboard should be implemented one day...
	 */
	data->ehn.ehn_Object = obj;
	data->ehn.ehn_Class = cl;
	data->ehn.ehn_Events = IDCMP_INTUITICKS | IDCMP_MOUSEBUTTONS;
	data->ehn.ehn_Flags = MUI_EHF_GUIMODE;

	DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehn);

	if (data->gotdone_done)
	{
		/*
		 * Data already arrived, launch the decoder
		 */
		DoMethod(obj, MM_Flash_Launch);
	}

	data->setup_done = TRUE;

	return(TRUE);
}


DEFMMETHOD(Cleanup)
{
	GETDATA;
	int c;
	struct ColorMap *cmap = _screen(obj)->ViewPort.ColorMap;

	DB(("got cleanup\n"));
	data->shutting_down = TRUE;

	if (data->pen_fg != -1)
	{
		ReleasePen(cmap, data->pen_fg);
	}

	if (data->pen_bg != -1)
	{
		ReleasePen(cmap, data->pen_bg);
	}

	if (data->pen_bar != -1)
	{
		ReleasePen(cmap, data->pen_bar);
	}

#if USE_CLUT || USE_PLANAR
	if (data->gfxmode == MV_Flash_ChunkyCLUTMode || data->gfxmode == MV_Flash_PlanarMode )
	{
		for (c = 0; c < 256; c++)
		{
			ReleasePen(cmap, data->cmp8[c]);
		}
		FreeMem(data->rgb16_to_8, 65536);
		if (data->tempclut)
		{
			FreeVec( data->tempclut );
			data->tempclut = 0;
		}
	}
#endif /* USE_CLUT || USE_PLANAR */

	DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehn);

	/*
	 * Tell player process to piss off.
	 */
	Forbid();
	if (data->isrunning)
	{
#ifdef POWERUP
		if (data->ppctask)
		{
			/*
			 * Shutting down PPC task
			 */
			PPCSignalTask(data->ppctask, SIGBREAKF_CTRL_C);
			DB(("task signaled (SIGBREAKF_CTRL_C)\n"));
		}
		else
#endif /* POWERUP */
		{
			/*
			 * Shut down the M68k task
			 */
			Signal(data->process, SIGBREAKF_CTRL_C);
		}
	}
	Permit();

	if (data->tempbm)
	{
		FreeBitMap(data->tempbm);
		data->tempbm = NULL;
	}

	return(DOSUPER);
}


DEFDISPOSE
{
	GETDATA;

	DB(("start of dispose\n"));

	Forbid();
	if (data->isrunning)
	{
#ifdef POWERUP
		if (data->ppctask)
		{
			void *M68kMsg;

			/*
			 * Signal again to be sure
			 * (Cleanup might not have been called)
			 */
			PPCSignalTask(data->ppctask, SIGBREAKF_CTRL_C);

			/*
			 * Now wait for the final message
			 * of the PPC task...
			 */
			for (;;)
			{
				if ((M68kMsg = PPCGetMessage(data->ReplyPort)))
				{
					DB(("got a message !\n"));
					if (PPCGetMessageAttr(M68kMsg, PPCMSGTAG_MSGID) == 1)
					{
						DB(("that was our shutdown message\n"));
						break;
					}
				}
				else
				{
					DB(("waiting...\n"));
					PPCWaitPort(data->ReplyPort);
				}
			}
			PPCDeleteMessage(M68kMsg);
			PPCDeletePort(data->ReplyPort);
			PPCFreeVec(data->startupdata);

			/*
			 * And we should check data->startupdata somewhere
			 * there to see if it exited properly
			 */

			DB(("deleting messages\n"));

			PPCUnLoadObject(data->elfobject);
			DB(("ElfObject cleaned up\n"));
		}
		else
#endif /* POWERUP */
		{
			while (data->isrunning)
			{
				DB(("telling process to stop..\n"));
				Signal(data->process, SIGBREAKF_CTRL_C);
				Delay(2);
			}
		}
	}
	Permit();

	DB(("process stopped\n"));

	if (data->canvas)
	{
#if USE_CGX
		if(data->gfxmode == MV_Flash_CGFXMode)
		{
			FreeBitMap(data->canvas);
		}
		else // P96, Chunky, AGA
#endif /* USE_CGX */
		{
#ifdef POWERUP
			if (PPCLibBase)
			{
				PPCFreeVec(data->canvas);
			}
			else
#endif /* POWERUP */
			{
				FreeVec(data->canvas);
			}
		}
	}

	if (data->ri)
	{
		FreeVec(data->ri);
	}

	if (data->ez->eventpool)
	{
#ifdef POWERUP
		if (PPCLibBase)
		{
			PPCDeletePool(data->ez->eventpool);
		}
		else
#endif /* POWERUP */
		{
			DeletePool(data->ez->eventpool);
		}
	}

	if (data->ez)
	{
#ifdef POWERUP
		if (PPCLibBase)
		{
			PPCFreeMem(data->ez, sizeof(struct eventzone));
		}
		else
#endif /* POWERUP */
		{
			FreeMem(data->ez, sizeof(struct eventzone));
		}
	}

	DB(("end of dispose, bye bye object\n"));

	return (DOSUPER);
}


DEFGET
{
	GETDATA;
	ULONG *store = ((struct opGet *)msg)->opg_Storage;

	switch (((struct opGet *)msg)->opg_AttrID)
	{
		case MA_Flash_MouseX:
			*store = data->mouse_x;
			return(TRUE);

		case MA_Flash_MouseY:
			*store = data->mouse_y;
			return(TRUE);
		case MA_Flash_bpp:
			*store = 2;   //TOFIX
			return(TRUE);

		case MA_Flash_GFXMode:
			*store = data->gfxmode;
			return(TRUE);

#if USE_CGX
		case MA_Flash_CGFXFormat:
			*store = data->pixfmt;
			return(TRUE);
#endif /* USE_CGX */

#if USE_PICASSO96
		case MA_Flash_P96Format:
			*store = data->ri->RGBFormat;
			return(TRUE);
#endif /* USE_PICASSO96 */
	}

	return(DOSUPER);
}


DEFSET
{
	struct TagItem *tag, *tagp = msg->ops_AttrList;
	GETDATA;

	while (tag = NextTagItem(&tagp)) switch (tag->ti_Tag)
	{
#if USE_PICASSO96
		case MA_Flash_P96Modulo:
			data->ri->BytesPerRow = tag->ti_Data;
			break;

		case MA_Flash_P96Format:
			data->ri->RGBFormat = tag->ti_Data;
			break;
#endif /* USE_PICASSO96 */

		case MA_Flash_Running:
			data->isrunning = tag->ti_Data;
			break;

		case MA_Flash_bpp:
			data->bpp = tag->ti_Data;
			break;
	}
	return(DOSUPER);
}


DEFTMETHOD(VPLUG_NetStream_GotData)
{
	GETDATA;

	int ptr = ft->vplug_net_getdocptr(data->nethandle);

	if (data->length)
	{
		sprintf(data->status, "Loading movie (%ld.%ld/%ld.%ld KB)...", (LONG)(ptr + 1023) / 1024, (LONG)(ptr / 103) % 10, (LONG)(data->length + 1023) / 1024, (LONG)(data->length / 103) % 10);
	}
	else
	{
		sprintf(data->status, "Loading movie (%ld.%ld KB)...", (LONG)(ptr + 1023) / 1024, (LONG)(ptr / 103) % 10);
	}

	MUI_Redraw(obj, MADF_DRAWUPDATE);
	return(0);
}

extern struct FlashPrefs *flashprefs;


DEFTMETHOD(VPLUG_NetStream_GotDone)
{
	GETDATA;

	if (!data->setup_done)
	{
		/* wait for MUIM_Setup */
		data->gotdone_done = TRUE;
	}
	else
	{
		DoMethod(obj, MM_Flash_Launch);
	}
	return(0);
}


DEFTMETHOD(Flash_Launch)
{
	struct ExecBase *SysBase = *((struct ExecBase **)4);
	char args[128]; /* bah */
	char program[42] = "PROGDIR:Plugins/VFlash_";
	char varbuf[2];
	int ppcmode = FALSE;
	int morphosmode = FALSE;
	BPTR seglist;
	#ifdef POWERUP
	BPTR ppclock;
	#endif

	GETDATA;

	DB(("got done\n"));

	morphosmode = (int)FindResident("MorphOS");

#ifdef POWERUP
	/*
	 * Switch in 68K mode if ENV:Vapor/VFLASHNOPPC is == 1
	 * or there's no VFlash_ppc.module
	 */
	if (!morphosmode && PPCLibBase)
	{
		if (!((GetVar("Vapor/VFLASHNOPPC", varbuf, sizeof(varbuf), 0) == 1) && (!strcmp("1", varbuf))))
		{
			if (ppclock = Lock("PROGDIR:Plugins/VFlash_ppc.module", ACCESS_READ))
			{
				ppcmode = TRUE;

				if (PPCLibBase->lib_Version == 46)
				{
					if (PPCLibBase->lib_Revision < 31)
					{
						if (!((GetVar("Vapor/VFLASHNOPPCLIBSANITYCHECK", varbuf, sizeof(varbuf), 0) == 1) && (!strcmp("1", varbuf))))
						{
							Fail("ppc.library 46.31 or higher needed.\nYour version has a bug which makes VFlash unuseable.\nYou can still run it but you are on your own.\nNo bug reports welcome.\nSet ENV:Vapor/VFLASHNOPPCLIBSANITYCHECK\nto make that warning go away.\n");
						}
					}
				}

				if (ppcmode)
				{
					strcat(program, "ppc");
					strcpy(vflash_cpuid, "PPC");
				}
				UnLock(ppclock);
			}
		}
	}
#endif /* POWERUP */

	if (morphosmode)
	{
		strcat(program, "m604e");
		strcpy(vflash_cpuid, "MorphOS");
	}
	else if (!ppcmode)
	{
		/*
		 * Find the right processor version
		 */
		if ((GetVar("Vapor/VFLASHDEBUG", varbuf, sizeof(varbuf), 0) != -1) && (!strcmp("1", varbuf)))
		{
			/*
			 * We are in debug mode
			 */
			strcat(program, "debug");
		}
		else if ((SysBase->AttnFlags & AFF_68060) && (SysBase->AttnFlags & AFF_68881))
		{
			strcat(program, "68060");
			strcpy(vflash_cpuid, "68060 FPU");
		}
	    else if ((SysBase->AttnFlags & AFF_68040) && (SysBase->AttnFlags & AFF_FPU40))
		{
			strcat(program, "68040fpu");
			strcpy(vflash_cpuid, "68040 FPU");
		}
	    else if ((SysBase->AttnFlags & AFF_68020) && (SysBase->AttnFlags & AFF_68881))
		{
			strcat(program, "68030fpu");
			strcpy(vflash_cpuid, "68030 FPU");
		}
	    else if (SysBase->AttnFlags & AFF_68020)
		{
			strcat(program, "68020");
			strcpy(vflash_cpuid, "68020");
		}
	}
	strcat(program, ".module");

	sprintf(data->status, "Decoding (%s engine)...", vflash_cpuid);

	MUI_Redraw(obj, MADF_DRAWOBJECT);

	DB(("launching %s\n", program));

#if POWERUP
	if ((ppcmode ? (long)(data->elfobject = (void *)PPCLoadObject(program)) : (long)(seglist = LoadSeg(program))))
#else
	if ((seglist = LoadSeg(program)))
#endif /* !POWERUP */
	{
		ft->vplug_net_lockdocmem();
		sprintf(args, "%lx %lx %lx %lu %lx\n",
									   (ULONG)ft->vplug_net_getdocmem(data->nethandle),
									   (ULONG)ft->vplug_net_getdocptr(data->nethandle),
									   (ULONG)obj,
									   (ULONG)flashprefs,
									   (ULONG)ft);

		DB( ( "arguments: %s", args ) );

#if POWERUP
		if (ppcmode)
		{
			struct TagItem dummytag;
			dummytag.ti_Tag = TAG_DONE;

			DB(("building message\n"));

			DB(("creating reply port\n"));
			if (data->ReplyPort = PPCCreatePort(&dummytag))
			{
				DB(("creating StartupMsg which will be replied upon PPC task's exit\n"));
				if (data->startupmsg = PPCCreateMessage(data->ReplyPort, sizeof(struct StartupData)))
				{
					if (data->startupdata = PPCAllocVec(sizeof(struct StartupData), MEMF_ANY))
					{
						struct Process *pr;
						DB(("creating PPC task\n"));

						data->startupdata->docmem 		  = ft->vplug_net_getdocmem(data->nethandle);
						data->startupdata->size			  = ft->vplug_net_getdocptr(data->nethandle);
						data->startupdata->obj			  = (APTR)obj;
						data->startupdata->vfp			  = flashprefs;
						data->startupdata->ft             = ft;
						data->startupdata->exitstatus	  = FALSE;

						/*
						 * Some stuffs to avoid the compiler to mess
						 * with IsInteractive() and stdio/err/in we don't really need.
						 * Update: the bug was in libnixppc so the following might not
						 * be needed. But it doesn't hurt...
						 */
						pr = (struct Process *)FindTask(NULL);

						data->isrunning = TRUE;

						/*
						 * Now launch the task
						 */
						if (data->ppctask = PPCCreateTaskTags(data->elfobject,
															  PPCTASKTAG_NAME, 				(ULONG)APPNAME" PPC Player",
															  PPCTASKTAG_STACKSIZE, 		20000,
															  PPCTASKTAG_BREAKSIGNAL, 		TRUE,
															  PPCTASKTAG_STARTUP_MSG, 	    (ULONG)data->startupmsg,
															  PPCTASKTAG_STARTUP_MSGDATA,   (ULONG)data->startupdata,
															  PPCTASKTAG_STARTUP_MSGLENGTH, sizeof(struct StartupData),
															  PPCTASKTAG_STARTUP_MSGID,     1,
															  NP_Input, Input(),
															  NP_CloseInput, FALSE,
															  NP_Output, Output(),
															  NP_CloseOutput, FALSE,
															  NP_Error, pr->pr_CLI ? pr->pr_CES : Output(),
															  NP_CloseError, FALSE,
															  TAG_DONE))

						{
							DB(("PPC task created, pid == %lx\n", data->ppctask));
							// launched
						}
						else
						{
							data->isrunning = FALSE;
							PPCFreeVec(data->startupdata);
							PPCDeleteMessage(data->startupmsg);
							PPCDeletePort(data->ReplyPort);
							PPCUnLoadObject(data->elfobject);
							strcpy( data->status, "ERROR: couldn't start player module?!?" );
							MUI_Redraw( obj, MADF_DRAWOBJECT );
				        }
					}
				}
			}
		}
		else
#endif /* POWERUP */
		{
			data->isrunning = TRUE;

#define STACKSIZE 20000

			if (data->process = CreateNewProcTags(NP_Seglist, 		seglist,
												  NP_FreeSeglist,   TRUE,  /* has to be specified according to Amiga Guru Book (wow, bug in dos.library) */
												  NP_Name, 			APPNAME" Player",
												  NP_CurrentDir, 	NULL,
												  NP_HomeDir,		NULL,
												  NP_StackSize, 	STACKSIZE, /* that's what I use on my system, I'll experiment a bit more one day */
												  #ifdef __MORPHOS__
												  NP_PPCStackSize,  STACKSIZE * 2,
												  #endif
												  NP_CopyVars, 		FALSE,
												  NP_Priority, 		0,
												  NP_Cli, 			TRUE,
												  NP_CommandName,   APPNAME" Player Thread",
	                                              NP_Arguments, 	args,
	                                              TAG_DONE))
			{
				DB(("process created, pid == %lx\n", data->process));
			}
			else
			{
				data->isrunning = FALSE;
				UnLoadSeg(seglist);
				strcpy( data->status, "ERROR: couldn't start player module?!?" );
				MUI_Redraw(obj, MADF_DRAWOBJECT);
	        }
		}

	}
	else
	{
		strcpy(data->status, "ERROR: couldn't load player module?!?");
		MUI_Redraw(obj, MADF_DRAWOBJECT);
	}

	return(0);
}


DEFTMETHOD(Flash_Redraw)
{
	MUI_Redraw(obj, MADF_DRAWUPDATE);
	return(0);
}


DEFTMETHOD(Flash_ClearDrawArea)
{
	MUI_Redraw(obj, MADF_DRAWOBJECT);
	return(0);
}

DEFTMETHOD(Flash_UnLockdocmem)
{
	ft->vplug_net_unlockdocmem();
	return(0);
}

DEFSMETHOD(Flash_SyncDraw)
{
	GETDATA;
	ULONG damsg[] = { MM_Flash_Redraw };
	DB( ( "got syncdraw, calling dma\n" ) );
	if (!data->shutting_down)
	{
		data->clip_x 	  = msg->clip_x;
		data->clip_y 	  = msg->clip_y;
		data->clip_width  = msg->clip_width;
		data->clip_height = msg->clip_height;
		ft->vplug_domethoda(obj, &damsg);
	}
	DB(("after dma\n"));
	return(0);
}


DEFMMETHOD(Draw)
{
	GETDATA;

	DoSuperMethodA(cl, obj, msg);

//	DB(("data->canvas == %lx, bpp=%ld, f=%ld, x=%ld, y=%ld, xs=%ld, ys=%ld, mw=%ld, mh=%ld\n", data->canvas, data->bpp, msg->flags&MADF_DRAWOBJECT, data->clip_x, data->clip_y, data->clip_width, data->clip_height,data->width,data->height));

	if (!data->shutting_down)
	{
	    if (data->canvas)
		{
			int x = data->clip_x;
			int y = data->clip_y;
			int xs = data->clip_width;
			int ys = data->clip_height;

			if( msg->flags & MADF_DRAWOBJECT )
			{
				if (data->isrunning)
				{
					x = 0;
					y = 0;
					xs = data->width;
					ys = data->height;
				}
				else
				{
					SetAPen(_rp(obj), data->pen_bg);
					RectFill(_rp(obj), _mleft(obj), _mtop(obj), _mright(obj), _mbottom(obj));
				}
			}

			switch (data->gfxmode)
			{
#if USE_CLUT
				case MV_Flash_ChunkyCLUTMode:
				{
					/*
					 * CLUT mode
					*/

					if( !data->tempclut )
					{
						data->tempclut = AllocVec( data->wh, 0 );
					}

					makeclut( data->tempclut, data->rgb16_to_8, data->canvas, data->wh );

					WritePixelArray( data->tempclut, x, y, data->width,
						_rp( obj ),
						_mleft( obj ) + x, _mtop( obj ) + y,
						xs, ys,
						RECTFMT_LUT8
					);

				}
				break;
#endif /* USE_CLUT */

#if USE_PICASSO96
				case MV_Flash_P96Mode:
					p96WritePixelArray(data->ri, x, y, _rp(obj), _mleft(obj) + x, _mtop(obj) + y, xs, ys );
					break;
#endif /* USE_PICASSO96 */

#if USE_CGX
				case MV_Flash_CGFXMode:
					BltBitMapRastPort(data->canvas, x, y, _rp(obj), _mleft(obj) + x, _mtop(obj) + y, xs, ys, 0xc0 );
					break;
#endif /* USE_CGX */

#if USE_PLANAR
				case MV_Flash_PlanarMode:
	            {
					int depth = _rp( obj )->BitMap->Depth;

					if (!data->tempbm)
					{
						data->tempbm = AllocBitMap( data->width, data->height, depth, 0, _rp(obj)->BitMap);
					}

					if (data->tempbm)
					{
						int c, n;
						UBYTE *planes[8];
						UWORD *cp = ((UWORD*)data->canvas) + data->width * y + x;
						int offs = 0;

						for (c = 0; c < ys; c++)
						{
							for (n = 0; n < depth; n++)
							{
								planes[n] = (UBYTE*)data->tempbm->Planes[n] + offs;
							}

							writechunky(cp, planes, xs, depth, data->rgb16_to_8 );

							cp += data->width;
							offs += data->tempbm->BytesPerRow;
						}
						BltBitMapRastPort(data->tempbm, 0, 0, _rp( obj ), _mleft(obj) + x, _mtop(obj) + y, xs, ys, 0xc0 );
					}
				}
				break;
#endif /* USE_PLANAR */
			}
		}
		else
		{
			APTR clip;
			if (msg->flags & MADF_DRAWOBJECT)
			{
				/*
				 * There we update the loading/decoding status
				 */
				int y = _mtop(obj) + _font(obj)->tf_Baseline + 2;

				DB(("MADF_DRAWOBJECT, with data->status == %s\n", data->status));

				SetFont(_rp(obj), _font(obj));
				SetAPen(_rp(obj), data->pen_bg);
				RectFill(_rp(obj), _mleft(obj), _mtop(obj), _mright(obj), _mbottom(obj));

				SetAPen(_rp(obj), data->pen_fg);

				clip = MUI_AddClipping(muiRenderInfo(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj));

#define copy APPNAME" SWF Player " LVERTAG

				Move(_rp(obj), _mleft(obj) + 2, y);
				Text(_rp(obj), copy, strlen(copy));
				y += _font(obj)->tf_YSize;
				Move(_rp(obj), _mleft(obj) + 2, y);
				Text(_rp(obj), data->status, strlen(data->status));
				data->bar_y = _font(obj)->tf_YSize * 2 - _font(obj)->tf_Baseline + 2;

				MUI_RemoveClipping(muiRenderInfo(obj), clip);
			}
			else
			{
				/*
				 * Updating the progress bar
				 */
				int prog = ft->vplug_net_getdocptr( data->nethandle );
				int y = _mtop(obj) + _font(obj)->tf_Baseline + 2;
				if (prog < data->length)
				{
					SetFont(_rp(obj), _font(obj));
					clip = MUI_AddClipping(muiRenderInfo(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj));
					SetAPen(_rp(obj), data->pen_bar);
					RectFill(_rp(obj),
						_mleft(obj) + 2, data->bar_y + y,
						_mleft(obj) + (prog * (_mwidth(obj) - 4)) / data->length, data->bar_y + y + 3
					);
					y += _font(obj)->tf_YSize;
					SetAPen(_rp(obj), data->pen_bg);
					RectFill(_rp(obj), _mleft(obj) + 2, y - _font(obj)->tf_Baseline, _mleft(obj) + 2 + TextLength(_rp(obj), data->status, strlen(data->status)), y + _font(obj)->tf_YSize - _font(obj)->tf_Baseline);
					SetAPen(_rp(obj), data->pen_fg);
					Move(_rp(obj), _mleft(obj) + 2, y);
					Text(_rp(obj), data->status, strlen(data->status));

					MUI_RemoveClipping(muiRenderInfo(obj), clip);
				}
			}
		}
	}
	return(0);
}


DEFMMETHOD(HandleEvent)
{
	GETDATA;
	int x, y;
	struct flashevent *fe;
	static int lastx, lasty;
	int sig = 0;
	ULONG rc = 0;

	if (!msg->imsg)
	{
		return(rc);
	}

	x = (int)msg->imsg->MouseX - (int)_left(obj);
	y = (int)msg->imsg->MouseY - (int)_top(obj);

	/*
	 * Check if we are within our boundaries
	 */
	if (x < 0 || y < 0 || x > _width(obj) || y > _height(obj))
	{
		/*
		 * No. We send one mousebutton handling
		 * to clear selected buttons
		 */
		if (!data->is_off_window)
		{
			data->is_off_window = TRUE;

			if (data->eventticks)
			{
				data->mouse_x = x;
				data->mouse_y = y;
				lastx = x;
				lasty = y;
				sig = SIGBREAKF_CTRL_D;
			}
			data->eventticks ^= 1;
			rc = MUI_EventHandlerRC_Eat;
		}
		else
		{
			return(rc);
		}
	}
	else
	{
		data->is_off_window = FALSE;

		switch (msg->imsg->Class)
		{
			case IDCMP_INTUITICKS:
				if ((x != lastx || y != lasty) && data->eventticks)
				{
					data->mouse_x = x;
					data->mouse_y = y;
					lastx = x;
					lasty = y;
					sig = SIGBREAKF_CTRL_D;
				}
				data->eventticks ^= 1;
				rc = MUI_EventHandlerRC_Eat;
				break;

			case IDCMP_MOUSEBUTTONS:
	            if (msg->imsg->Code == SELECTDOWN || msg->imsg->Code == SELECTUP)
				{
					ObtainSemaphore(&data->ez->eventsem);
#ifdef POWERUP
					if (PPCLibBase)
					{
						fe = PPCAllocPooled(data->ez->eventpool, sizeof(*fe));
					}
					else
#endif /* POWERUP */
					{
						fe = AllocPooled(data->ez->eventpool, sizeof(*fe));
					}
					fe->x = x;
					fe->y = y;
					fe->type = (msg->imsg->Code == SELECTDOWN) ? FeButtonPress : FeButtonRelease;
					ADDTAIL(&data->ez->eventlist, fe);
					ReleaseSemaphore(&data->ez->eventsem);
					sig = SIGBREAKF_CTRL_F;
					rc = MUI_EventHandlerRC_Eat;
				}
				break;
		}
	}

	/*
	 * If we have an event, signal the
	 * running tasks to process them
	 */
	if (sig)
	{
		Forbid();
#ifdef POWERUP
		if (data->ppctask)
		{
			//kprintf("sig (%lu) signal swalloved\n", sig);
			PPCSignalTask(data->ppctask, sig);
		}
#endif /* POWERUP */
		if (data->process)
		{
			Signal(data->process, sig);
		}
		Permit();
	}
	return(rc);
}


/*
 * Gets the next event (mousebuttons only)
 */
DEFSMETHOD(Flash_GetNextEvent)
{
	GETDATA;
	struct flashevent *fe;

	ObtainSemaphore(&data->ez->eventsem);
	fe = REMHEAD(&data->ez->eventlist);
	ReleaseSemaphore(&data->ez->eventsem);

	return((ULONG)fe);
}


DEFSMETHOD(Flash_FreeEvent)
{
	GETDATA;

	ObtainSemaphore(&data->ez->eventsem);
#if POWERUP
	if (PPCLibBase)
	{
		PPCFreePooled(data->ez->eventpool, msg->event, sizeof(struct flashevent));
	}
	else
#endif /* POWERUP */
	{
		FreePooled(data->ez->eventpool, msg->event, sizeof(struct flashevent));
	}
	ReleaseSemaphore(&data->ez->eventsem);

	return(0);
}


DEFSMETHOD(Flash_GetCMP8)
{
	GETDATA;
	memcpy(msg->area, data->cmp8, sizeof(data->cmp8));
	return(0);
}


DEFSMETHOD(Flash_GotoURL)
{
	GETDATA;
	char url[2048];

	ft->vplug_mergeurl(data->baseref, msg->url, url);
	ft->vplug_seturl(url, msg->target, 0);
	return(0);
}

#ifdef __SASC
	#error 68k build is broken
	putreg(REG_A4, cl->cl_UserData); // restore the near data pointer
#endif

BEGINMTABLE2(flash)
DECGET
DECNEW
DECDISPOSE
DECMMETHOD(Setup)
DECMMETHOD(Cleanup)
DECSET
DECTMETHOD(Flash_Redraw)
DECTMETHOD(Flash_Launch)
DECTMETHOD(Flash_UnLockdocmem)
DECTMETHOD(Flash_AllocCanvas)
DECTMETHOD(Flash_ClearDrawArea)
case VPLUG_NetStream_GotDone	: return(handleMM_VPLUG_NetStream_GotDone 	(cl, obj, (APTR)msg));
case VPLUG_NetStream_GotData	: return(handleMM_VPLUG_NetStream_GotData	(cl, obj, (APTR)msg));
DECSMETHOD(Flash_SyncDraw)
DECSMETHOD(Flash_GetNextEvent)
DECSMETHOD(Flash_FreeEvent)
DECMMETHOD(AskMinMax)
DECMMETHOD(Draw)
DECMMETHOD(HandleEvent)
DECSMETHOD(Flash_GetCMP8)
DECSMETHOD(Flash_GotoURL)
ENDMTABLE

