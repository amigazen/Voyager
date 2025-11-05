/*
 * VFlash.module
 * -------------
 * - This is the main player for VFlash.
 *
 * © 1999-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: main.c,v 1.6 2004/02/29 17:04:14 zapek Exp $
 *
*/

#include "globals.h"
#include "main.h"
#include "player.h"
#include "gfx.h"
#include "support.h"
#include "init.h"
#include "mempool.h"

const char ver[] = "$VER: VFlash " LVERTAG " © 1999-2004 by David Gerber <zapek@vapor.com>";

#if !USE_REENTRANCY
struct vplug_functable  *ft;
struct FlashInfo 		*fi;
APTR					mempool;
APTR	   				flashobj;
int						signalexit = TRUE;
struct FlashPrefs		*vfp;
struct Process			*pr;
#endif /* !USE_REENTRANCY */

/*
 * PowerUP's task switching should be minimized
 */
#ifdef POWERUP
unsigned long _MSTEP = 65536;
#else
unsigned long _MSTEP = 32768;
#endif

#if USE_INTERNAL_MALLOC
void free(void *data);
extern APTR 				   globalmempool;
#endif /* INTERNAL_MALLOC */

#ifdef POWERUP
struct ExecBase			*SysBase;
struct StartupData 		*StartupData;
#endif /* POWERUP */


/*
 * AHI
 */
#if 0
#ifndef NOSOUND
#include "ahi_sup.h"
#endif
#endif


/*
 * Disable CTRL_C handling
 */
void __chkabort(void) {return;}
#ifdef POWERUP
int __nocommandline = 1;
int __isatty(int d) // fix a weird IsInteractive() bug in libnixppc
{
	return(0);
}
#endif /* POWERUP */

int __stdio = 0;

void exitfunc( void )
{
	DB(("exiting... doing a set on flashobj which is at 0x%lx\n", flashobj));

#ifndef POWERUP
	Forbid();
#endif /* !POWERUP */

	/*
	 * Tell the flashobj we're not here anymore
	 */
	if (flashobj) /* XXX: shouldn't be there I think */
	{
		struct Data *data = muiUserData(flashobj);
		DB(("intuitionbase: 0x%lx\n", IntuitionBase));
		//set(flashobj, MA_Flash_Running, FALSE);
		data->isrunning = FALSE;
	}

#ifdef POWERUP
	StartupData->exitstatus = TRUE; // tell the 68k all went well and that he can remove the ElfObject safely
#else /* !POWERUP */
	Permit();
#endif /* !POWERUP */
	DB(("before closestuff\n"));
	closestuff();
	DB(("after closestuff, we are exiting now, bye bye\n"));
#ifndef POWERUP
	Forbid(); /* no matching permit because we'll be RemTask()ed soon */
#endif /* !POWERUP */
}


#define NUMARGS 6 /* number of arguments TOFIX: this shouldn't be there */

int main(int argc, char **argv)
{
	char *buffer;
	long size;
	void * flashHandle;

	int status;

	DB(("let's go!\n"));

#ifdef POWERUP
	SysBase = (struct ExecBase *) *((ULONG*)4L);
#endif /* POWERUP */

	atexit(exitfunc);

	fi = initstuff();

#ifndef POWERUP
	if (argc != NUMARGS)
	{
		Fail("IPC error between VFlash <-> module. Check if both versions match.\n");
		return (1);
	}
#endif /* !POWERUP */


#ifdef POWERUP
	/*
	 * Get our startup message
	 */
	if (StartupData = (struct StartupData *) PPCGetTaskAttr(PPCTASKTAG_STARTUP_MSGDATA))
	{

		/*
		 * rework that part
		 */
		buffer 		 = StartupData->docmem;
		size		 = StartupData->size;
		flashobj   	 = StartupData->obj;
		vfp			 = StartupData->vfp;
		ft           = StartupData->ft;
	}
	else
	{
		return (1); /* something fucked up there */
	}
#else /* !POWERUP */
	sscanf((char *)argv[1], "%lx", (long unsigned int *)&buffer);
	sscanf((char *)argv[2], "%lx", (long unsigned int *)&size);
	sscanf((char *)argv[3], "%lx", (long unsigned int *)&flashobj);
	sscanf((char *)argv[4], "%ld", (long unsigned int *)&vfp);
	sscanf((char *)argv[5], "%lx", (long unsigned int *)&ft);
#endif /* !POWERUP */

	DB(("buffer at %lx, size = %ld, flashobj at 0x%lx, vfp at 0x%lx\n", buffer, size, flashobj, vfp));

	flashHandle = FlashNew();

	if (flashHandle == 0)
	{
		Fail("Flash file parsing error\n");
		return (1);
	}

	do
	{
		status = FlashParse(flashHandle, 0, buffer, size);
	}
	while (status & FLASH_PARSE_NEED_DATA);

	domethod(flashobj, MM_Flash_UnLockdocmem);

	/*
	 * We don't need the buffer anymore. It would be
	 * smart to add a "flush docs" or something to the
	 * API.
	 */

	DB(("Flash parsed\n"));

	FlashGetInfo(flashHandle, fi);

	/*
	 * The original player setups some weird
	 * size here... I don't think it's needed...
	 * fi.frameWidth = 640 * 20;
	 * fi.frameHeight = 441 * 20;
	 */
#if DEBUG
	draw_info();
#endif

	if (flash_graphic_init(flashHandle))
	{
		DB(("init sound...\n"));
#if USE_SOUND
		FlashSoundInit(flashHandle, vfp); // hmmmm
#endif /* USE_SOUND */

		DB(("init flash methods...\n"));
		FlashSetGetUrlMethod(flashHandle, showUrl, 0);
		FlashSetGetSwfMethod(flashHandle, getSwf, (void *)flashHandle);

		DB(("done.. playing movie now\n"));
		play_movie(flashHandle);
	}

	DB(("closing Flash...\n"));
	FlashClose(flashHandle);

	return (0);
}

