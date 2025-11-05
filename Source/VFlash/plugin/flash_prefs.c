/*
** Flash Preferences
** -----------------
**
** © 1999-2000 by David Gerber <zapek@vapor.com>
** All rights reserved
**
** $Id: flash_prefs.c,v 1.7 2004/03/07 00:10:10 zapek Exp $
**
*/

#include "config.h"

/* OS */
#include <exec/memory.h>
#include <exec/types.h>
#ifdef __SASC
#include <dos.h>
#endif
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <libraries/dos.h>

#ifdef __MORPHOS__
#include <clib/alib_protos.h>
#endif

/* MUI */
#include <graphics/gfxmacros.h>
#include <libraries/mui.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>

/* Protos */
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/console.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#include <string.h>
#include <stdio.h>

#ifdef __SASC
#include "/debug.h"
#include "/rev.h"
#else
#include "debug.h"
#include "rev.h"
#endif
#include <macros/vapor.h>
#include "flash_mcc_private.h"

#undef GETDATA
#define GETDATA struct PrefsData *data = INST_DATA(cl, obj)

extern ULONG getv(APTR obj, ULONG attr);

extern struct Library *MUIMasterBase;
//extern struct ExecutiveMessage *ExecutiveMsg;
extern struct FlashPrefs *flashprefs;

static char *about_text = "\n\nThanks to:\n\n"
					 "\0333Olivier Debon\n\0332for making a portable Flash player\n\n\n"
					 "\0333Oliver 'Olli' Wagner\n\0332for all his help and encouragement. He also wrote many important parts.\n(I'll win the race next time)\n\n\n"
					 "\0333Ben 'Beej' Preece\n\0332for the logo and the nice reward upon the completion of the plugin :)\n\n\n"
					 "\0333Ralph 'Laire' Schmidt\n\0332for his help with the PPC version and for a nice PPC API\n\n\n"
					 "\0333Frank 'cyfm' Mariak\n\0332for useful explanations about reliable m68k <-> PPC messaging\nand suggestions for optimal CyberGraphX support\n\n\n"
					 "\0333Trond 'Tronan' Werner Hansen\n\0332for helping with the early design of the plugin\n\n\n"
					 "\0333Emmanuel 'Emm' Lesueur\n\0332for the constructor/destructor hint\n\n\n"
					 "\0333Peter 'Paladin' Annuss\n\0332for libnixppc\n\n\n"
					 "\0333Olivier 'pirus' Piras\n\0332for writing rcom, the best null-modem capture tool on earth\n(and almost bug free :)\n\n\n"
					 "\0333Robert 'RobR' Reiswig\n\0332for the magic 'handle it all' Install script\n\n\n"
					 "\0333All the IBeta team\n\0332for all the betatesting during the development\n\n\n"
					 "\033iand everyone taking interest in this project, too many to be named all\033n\n\n\n\n"
					 APPNAME" uses the MUI object library\nMUI is © 1992-2000 by Stefan Stuntz.\nWithout MUI, "APPNAME" wouldn't exist.\n\n\n\nScroll restart\n\n\n\n\n\n";

#define MUIC_Crawling "Crawling.mcc"
#define CrawlingObject MUI_NewObject(MUIC_Crawling

#ifdef __SASC
ULONG STDARGS DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
{
	return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}
#endif


static const char crawl_txt[] = "\033b"APPNAME"\033n "LVERTAG"\nCopyright ©1998-2004 by "AUTHOR"\n<"EMAIL">\nAll Rights Reserved";

static char *modelist[] = {
	"None",
	#if 0
	"audio.device",
	"AHI",
	"CyberSound",
	"DOS Driver",
	#endif
	"Ambient Sound",
	NULL
};

static char *outputlist[] = {
	"Mono",	
	"Stereo",
	NULL
};

static char *resolutionlist[] = {
	"8-bits",
	"16-bits", 
	NULL
};

static char *samplingratelist[] = {
	"5500",
	"11025",
	"22050",
	"44100",
	NULL
};

static char *ahiunit[] = {
	"Music",
	"0",
	"1",
	"2",
	"3",
	NULL
};


DEFNEW
{
	APTR soundswitch, audiogrp, stereo, resolution, samplingrate, audiomaingrp, audiogrp_child;

	/* Executive */
	//	  if (!(ExecutiveMsg))
	//		  *PrefList[2] = NULL;


	obj = (Object *)DoSuperNew(cl, obj,
					/*Child, pagegrp = RegisterGroup(PrefList),
						MUIA_Register_Frame, TRUE,
						MUIA_Background, MUII_RegisterBack,*/

						/* general */
						Child, VGroup,

							Child, VSpace(0),

							Child, HGroup,
							Child, HSpace(0),
							Child, VGroup, GroupFrameT("Sound"),
								Child, HGroup,
									Child, Label1("Mode:"), Child, soundswitch = Cycle(modelist),
								End,

								Child, audiomaingrp = VGroup,
									#if 0
									Child, ColGroup(2), /* common options */
										Child, Label1("Output:"), Child, stereo = Cycle(outputlist),
										Child, Label1("Resolution:"), Child, resolution = Cycle(resolutionlist),
										Child, Label1("Sampling rate:"), Child, samplingrate = Cycle(samplingratelist),
										// don't forget to put a buffer size too
									End,
									#endif
									Child, audiogrp = VGroup, /* particular options */
										Child, audiogrp_child = HVSpace,
									End,
								End,
							End,
							/* put something here :) */
							Child, HSpace(0),
							End,

							Child, VSpace(0),

							Child, CrawlingObject,
								TextFrame,
								MUIA_Background, MUII_TextBack,
								MUIA_FixHeightTxt, "\n\n\n",
								Child, VGroup,
									Child, TextObject,
										MUIA_Text_Contents, crawl_txt,
										MUIA_Text_PreParse, MUIX_C,
									End,
									Child, TextObject,
										MUIA_Text_Contents, about_text,
										MUIA_Text_PreParse, MUIX_C,
									End,
									Child, TextObject,
										MUIA_Text_Contents, crawl_txt,
										MUIA_Text_PreParse, MUIX_C,
									End,
								End,
							End,
						End,
	TAG_DONE);

	if (obj)
	{
		int objsdone = FALSE;
		struct PrefsData *data = INST_DATA(cl, obj);
		DB(("prefsobject created\n"));
		
		/*
		 * Create the audio objects
		 */
		data->audioobjs[0] = VGroup, // none
			Child, HVSpace,
		End;

		if (data->audioobjs[0])
		{
			data->audioobjs[1] = ColGroup(2), // audio.device
				Child, Label1("Volume:"), Child, data->audio.sl_volume = SliderObject,
					MUIA_Slider_Max, 64,
					MUIA_Numeric_Value, flashprefs->vfp_audio.volume,
				End,
			End;

			if (data->audioobjs[1])
			{
				data->audioobjs[2] = ColGroup(2), // AHI
					Child, Label1("AHI Unit:"), Child, data->ahi.cyc_unit = CycleObject,
						MUIA_Font, MUIV_Font_Button,
						MUIA_Cycle_Entries, ahiunit,
						MUIA_Cycle_Active, flashprefs->vfp_ahi.unit,
					End,
				End;

				if (data->audioobjs[2])
				{
					data->audioobjs[3] = HGroup, // CyberSound
						Child, HVSpace,
					End;

					if (data->audioobjs[3])
					{
						data->audioobjs[4] = HGroup, // DOS driver
							Child, Label1("DOS Driver:"), Child, data->dosdriver.str_name = StringObject,
								StringFrame,
								MUIA_String_MaxLen, 255,
								MUIA_String_Contents, flashprefs->vfp_dosdriver.name,
							End,
						End;

						if (data->audioobjs[4])
						{
							data->audioobjs[5] = HGroup, // Ambient Sound
								Child, Label1("Volume:"), Child, data->audio.sl_volume = SliderObject,
									MUIA_Slider_Max, 64,
									MUIA_Numeric_Value, flashprefs->vfp_audio.volume,
								End,
							End;

							if (data->audioobjs[5])
							{
#ifdef CRIPPLED_SOUND
								data->audioobjs[6] = HGroup,
									Child, Label("Not yet implemented"),
								End;

								if (data->audioobjs[6])
								{
#endif /* CRIPPLED_SOUND */
									objsdone = TRUE;
#ifdef CRIPPLED_SOUND
								}
#endif /* CRIPPLED_SOUND */
							}
						}
					}
				}
			}
		}

		if (!objsdone)
		{
			int i = 0;

			while (data->audioobjs[i])
			{
				MUI_DisposeObject(data->audioobjs[i]);
				i++;
			}
		
			MUI_DisposeObject(obj);
			return(0);
		}

		data->soundswitch  = soundswitch;
		#if 0
		data->stereo	   = stereo;
		data->resolution   = resolution;
		data->samplingrate = samplingrate;
		#endif
		data->audiogrp	   = audiogrp;
		data->audiomaingrp = audiomaingrp;
		data->audiogrp_child = audiogrp_child;

		DoMethod(data->soundswitch, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 2, MM_Prefs_ChangeAudioGroup, MUIV_TriggerValue);

		set(data->soundswitch, MUIA_ShortHelp, "Use Ambient's advanced sound processing system");

		#if 0
		if (!getv(data->soundswitch, MUIA_Cycle_Active))
		{
			set(audiomaingrp, MUIA_Disabled, TRUE);
		}
		#endif

		#if 0
		setcycle(data->resolution, flashprefs->vfp_resolution);
		setcycle(data->stereo, flashprefs->vfp_stereo);
		setcycle(data->samplingrate, flashprefs->vfp_samplingrate);
		#endif

		/*
		 * Attempts to recover mistakes from the past.
		 */
		switch (flashprefs->vfp_soundswitch)
		{
			case 0:
				setcycle(data->soundswitch, 0);
				break;

			default:
				setcycle(data->soundswitch, 1);
		}

		setcycle(data->soundswitch, flashprefs->vfp_soundswitch);

		/* executive */
		/*
		if (ExecutiveMsg)
		{
			set(data->niceval, MUIA_Numeric_Value, GetNice(ExecutiveMsg));
			DoMethod(data->priorityval, MUIM_SetAsString, MUIA_Text_Contents, "%d", GetTaskPri(ExecutiveMsg));
		}
		*/

		//DoMethod(prefspic, MUIM_Notify, MUIA_Pressed, FALSE, aboutwin, 3, MUIM_Set, MUIA_Window_Open, TRUE);

	}
	return((ULONG)obj);
}


/*
 * Put the correct audio group
 */
DEFSMETHOD(Prefs_ChangeAudioGroup)
{
#if 1
	GETDATA;

	DoMethod(data->audiogrp, MUIM_Group_InitChange);

	if (data->audiogrp_child)
	{
		DoMethod(data->audiogrp, OM_REMMEMBER, data->audiogrp_child);

		if (!data->group_changed)
		{
			MUI_DisposeObject(data->audiogrp_child);
			data->group_changed = TRUE;
		}
	}

	if (msg->group == 1)
	{
		DoMethod(data->audiogrp, OM_ADDMEMBER, data->audioobjs[5]);
		data->audiogrp_child = data->audioobjs[5];
	}
	
	DoMethod(data->audiogrp, MUIM_Group_ExitChange);
#else
	GETDATA;

	if (DoMethod(data->audiogrp, MUIM_Group_InitChange))
	{
		if (data->audiogrp_child)
		{
			DoMethod(data->audiogrp, OM_REMMEMBER, data->audiogrp_child);
			
			if (!data->group_changed)
			{
				MUI_DisposeObject(data->audiogrp_child);
				data->group_changed = TRUE;
			}
		}

#ifdef CRIPPLED_SOUND
		if (msg->group < 4)
#else
		if (msg->group == MV_Prefs_None)
#endif /* CRIPPLED_SOUND */
		{
			set(data->audiomaingrp, MUIA_Disabled, TRUE);
		}
		else
		{
			set(data->audiomaingrp, MUIA_Disabled, FALSE);
		}
		
#ifdef CRIPPLED_SOUND
		if (msg->group && msg->group < 4)
		{
			DoMethod(data->audiogrp, OM_ADDMEMBER, data->audioobjs[6]);
			data->audiogrp_child = data->audioobjs[6];
		}
		else
		{
			DoMethod(data->audiogrp, OM_ADDMEMBER, data->audioobjs[msg->group]);
			data->audiogrp_child = data->audioobjs[msg->group];
		}
#else
		DoMethod(data->audiogrp, OM_ADDMEMBER, data->audioobjs[msg->group]);
		data->audiogrp_child = data->audioobjs[msg->group];
#endif /* CRIPPLED_SOUND */
		DoMethod(data->audiogrp, MUIM_Group_ExitChange);
		return(0);
	}
	
	DisplayBeep(NULL);
#endif
	
	return(0);
}


DEFMETHOD(Prefs_StorePrefs, ULONG)
{
	GETDATA;

	/*
	 * Fixup for mistakes from the past.
	 */
	get(data->soundswitch, MUIA_Cycle_Active, &flashprefs->vfp_soundswitch);
	if (flashprefs->vfp_soundswitch == 1)
	{
		flashprefs->vfp_soundswitch = 5;
	}
	#if 0
	get(data->resolution, MUIA_Cycle_Active, &flashprefs->vfp_resolution);
	get(data->stereo, MUIA_Cycle_Active, &flashprefs->vfp_stereo);
	get(data->samplingrate, MUIA_Cycle_Active, &flashprefs->vfp_samplingrate);
	#endif
	get(data->audio.sl_volume, MUIA_Slider_Level, &flashprefs->vfp_audio.volume);
	get(data->ahi.cyc_unit, MUIA_Cycle_Active, &flashprefs->vfp_ahi.unit);
	strcpy(flashprefs->vfp_dosdriver.name, (char *)getv(data->dosdriver.str_name, MUIA_String_Contents));

	DB(("prefs successfully copied to internal structure\n"));

	/* executive */
	/*
	get(data->niceval, MUIA_Numeric_Value, &lkp->lkp_niceval);
	if (ExecutiveMsg)
	{
		SetNice(ExecutiveMsg, lkp->lkp_niceval);
	}
	*/

	//set(obj, MUIA_Window_Open, FALSE);

	DB(("closed\n"));

	return(TRUE);
}


#ifdef __SASC
	#error 68k build is broken
	putreg(REG_A4, cl->cl_UserData); // restore the near data pointer
#endif /* __SASC */
	
BEGINMTABLE2(prefs)
DECNEW
DECMETHOD(Prefs_StorePrefs)
DECSMETHOD(Prefs_ChangeAudioGroup)
ENDMTABLE
