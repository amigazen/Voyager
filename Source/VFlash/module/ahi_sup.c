/*
 * AHI support stuffs
 *
 * $Id: ahi_sup.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 */

#if 0 /* dead code :) */

#define AHIBUFFERSIZE 2048 * 2 
#include "ahi_sup.h"

/*
 * AHI sound support
 */
struct MsgPort 			*AHImp;
struct AHIRequest 		*AHIio;
struct AHIRequest 		*AHIio2;
struct AHIRequest		*AHIlink;
int	   					AHIDevice = -1;
int	   					AHIavail;

/*
 * Sound buffer size
 */
int	   buffersize = AHIBUFFERSIZE;

/*
 * We currently use tripple buffering. Sound.cc needs a rewrite
 */
char ahimixbuffer1[AHIBUFFERSIZE];
char ahimixbuffer2[AHIBUFFERSIZE];
char *ahibuf1;
char *ahibuf2;

/*
 * Initialize all the AHI device driver
 */
int	initahi(void)
{
	if (AHImp = CreateMsgPort())
	{
		if (AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest)))
		{
			AHIio->ahir_Version = 4;
			if (AHIDevice = OpenDevice(AHINAME, 0, (struct IORequest *)AHIio, NULL) == 0)
			{
				if (AHIio2 = myAllocPooled(mempool, sizeof(struct AHIRequest)))
				{
					memcpy(AHIio2, AHIio, sizeof(struct AHIRequest));
					DB(("AHI initialized\n"));
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}


/*
 * Cleans up AHI
 */
void closeahi(void)
{
	struct Message *msg;

	if (AHIio)
	{
		Forbid();
		if (!CheckIO((struct IORequest *)AHIio))
		{
			AbortIO((struct IORequest *)AHIio);
			WaitIO((struct IORequest *)AHIio);
		}
		Permit();
	}

	if (AHIio2)
	{
		Forbid();
		if (!CheckIO((struct IORequest *)AHIio2))
		{
			AbortIO((struct IORequest *)AHIio2);
			WaitIO((struct IORequest *)AHIio2);
		}
		Permit();
	}

	if (AHIDevice == 0)
		CloseDevice((struct IORequest *)AHIio);

	if (AHIio)
		DeleteIORequest((struct IORequest *)AHIio);

	if (AHIio2)
		DeleteIORequest((struct IORequest *)AHIio2);

	//while(msg = GetMsg(AHImp))
	//	  ReplyMsg(msg);

	if (AHImp)
		DeleteMsgPort(AHImp);
}


/*
 * Setups basic AHIRequest structure
 */
void configahi(int resolution, int isstereo, long frequency)
{
	ahibuf1 = ahimixbuffer1; ahibuf2 = ahimixbuffer2;

	AHIio->ahir_Std.io_Message.mn_Node.ln_Pri = 0;
	AHIio->ahir_Std.io_Command 	= CMD_WRITE;
	AHIio->ahir_Std.io_Offset	= 0;
	AHIio->ahir_Frequency		= frequency;
	AHIio->ahir_Type			= (resolution ? (isstereo ? AHIST_S16S : AHIST_M16S) : (isstereo ? AHIST_S8S : AHIST_M8S));
	AHIio->ahir_Volume			= 0x10000; // full volume for now
	AHIio->ahir_Position		= 0x8000;  // centered
}


/*
 * Starts playing a stream
 */
void playahi(char *buffer, long length)
{
	char *tmp;

	memcpy(ahibuf1, buffer, length);

	AHIio->ahir_Std.io_Data   = ahibuf1;
	AHIio->ahir_Std.io_Length = length;
	AHIio->ahir_Link		  = AHIlink;
	SendIO((struct IORequest *)AHIio);

	DB(("first request sent, length == %ld, AHIlink == %lx\n", length, AHIlink));

	if (AHIlink)
	{
		Wait(1L << AHImp->mp_SigBit);
		if (WaitIO((struct IORequest *)AHIlink))
		{
			DB(("failed... put error checking around\n"));
			return;
		}
	}

	/*
	 * End of stream, wait until it's finished
	 */
	if (length != BUFFERSIZE)
	{
		DB(("last audio run, waiting...\n"));
		WaitIO((struct IORequest *)AHIio);
		return;
	}

	AHIlink = AHIio;

	/*
	 * Swap the requests and the buffers
	 */
	tmp   = ahibuf1;
	ahibuf1  = ahibuf2;
	ahibuf2  = tmp;

	tmp    = (char *)AHIio;
	AHIio  = AHIio2;
	AHIio2 = (struct AHIRequest *)tmp;
}
#endif
