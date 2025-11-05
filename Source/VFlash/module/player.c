/*
 * VFlash player functions
 * -----------------------
 * - separated playing functions for better portability
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: player.c,v 1.2 2004/02/28 18:51:07 zapek Exp $
 *
 */

#include "globals.h"
#include "main.h"
#include "gfx.h"
#include "timers.h"
#include "support.h"

/*
 * Prints some useless stats. For
 * debugging only.
 */
void draw_info(void)
{
	DB(("Version: %ld - Frames: %ld - Rate: %ld fps - Width: %ld - Height: %ld\n",
			fi->version, fi->frameCount, fi->frameRate, fi->frameWidth, fi->frameHeight));
	/* RawDoFmt() expects 16-bit for ints */
}


/*
 * Renders the canvas into
 * the target area
 */
void flash_copy(void)
{
	domethod(flashobj, MM_Flash_SyncDraw, fd->clip_x, fd->clip_y, fd->clip_width, fd->clip_height);
}


/*
 * Plays the movie
 */
void play_movie(FlashHandle flashHandle)
{
#ifdef POWERUP
#define vflash_sigs ppcsigs
#define vflash_wait PPCWait
	ULONG ppcsigs = 0;
#else /* !POWERUP */
#define vflash_sigs sigs
#define vflash_wait Wait
	ULONG sigs = 0;
#endif /* !POWERUP */
	struct unixtimeval wd, now;
	int wakeup;
	int eventhandled;

	DB(("playing movie...\n"));

	if (init_timer())
	{
		/*
		 * Start the wakeUp call... if it's TRUE it
		 * means that we have a frame (event) handled and ready
		 * to display
		 */
		wakeup = FlashExec(flashHandle, FLASH_WAKEUP, 0, &wd);
		while (1)
		{
			rtimer();
			/*
			 * Refreshing canvas
			 */
			if (fd->flash_refresh)
			{
				flash_copy();
				fd->flash_refresh = 0;
			}

			if (wakeup)
			{
				DB(("wakeup is true\n"));

				DB(("wd looks like tv_sec == %ld, tv_usec == %ld\n", wd.tv_sec, wd.tv_usec));

				/*
				 * Wait for an event and timeout if it doesn't come since we need
				 * to render the stuff and go to the next frame
				 */
				qtimer((struct timeval *)&wd);
			}

			DB(("waiting on signals %08lx...\n", timersig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_F));
			vflash_sigs	= vflash_wait(timersig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_F);
			DB(("got signal %08lx\n", vflash_sigs));

			/*
			 * Check if we need to exit, if so, we don't need to notify the whole world
			 * we are going to quit as they already know
			 */
			if (vflash_sigs & SIGBREAKF_CTRL_C)
			{
				DB(("caught SIGBREAKF_CTRL_C! exiting in 2 seconds...\n"));
				signalexit = FALSE;
				break;
			}

			/*
			 * Check for events
			 */
			if (vflash_sigs & SIGBREAKF_CTRL_F)
			{
				/*
				 * Let's handle mousebuttons
				 */
				struct flashevent *fe;
				struct FlashEvent rfe;

				while (fe = (APTR)domethod(flashobj, MM_Flash_GetNextEvent))
				{
					/*
					 * copy our structure properly
					 */
					rfe.type = fe->type;
					rfe.x 	 = fe->x;
					rfe.y 	 = fe->y;
					rfe.key  = fe->key;

					DB(("Event: %ld - X,Y: %ld,%ld - Key: %ld\n",
							rfe.type, rfe.x, rfe.y , rfe.key));

					if (FlashExec(flashHandle, FLASH_EVENT, &rfe, &wd))
					{
						wakeup = 1;
					}

					domethod(flashobj, MM_Flash_FreeEvent, fe);

					eventhandled = TRUE;
				}
			}
			else if (vflash_sigs & SIGBREAKF_CTRL_D)
			{
				/*
				 * Let's handle mousemoves then
				 */
				struct FlashEvent rfe;

				rfe.type = FeMouseMove;
				rfe.x 	 = getv(flashobj, MA_Flash_MouseX);
				rfe.y 	 = getv(flashobj, MA_Flash_MouseY);
				rfe.key  = 0; // not needed but...

				DB(("Event: %ld - X,Y: %ld,%ld - Key: %ld\n",
						rfe.type, rfe.x, rfe.y , rfe.key));

				if (FlashExec(flashHandle, FLASH_EVENT, &rfe, &wd))
				{
					wakeup = 1;
				}
				eventhandled = TRUE;
			}
			else
			{
				/*
				 * Nothing, go to the next frame
				 */
				wakeup = FlashExec(flashHandle, FLASH_WAKEUP, 0, &wd);
				eventhandled = FALSE;
			}

			/*
			 * Now check if we need to wait more since there was an event
			 */
			if (!(vflash_sigs & timersig))
			{
				wtimer();
			}
		}
	}
}

/*
 * Sends an URL to Voyager
 */
void showUrl( char *url, char *target, void *udata )
{
	DB( ( "*** SHOWURL: %s, target %s, udata %lx\n", url, target, udata ) );
	if( strnicmp( url, "FSCommand:", 10 ) )
	{
		DB( ( "sending method\n" ) );

		domethod(flashobj, MM_Flash_GotoURL, url, target);
	}
}

void getSwf(char *url, int level, void *client_data)
{
	FlashHandle flashHandle;
	char *buffer;
	long size;

	/*
	 * We simply tell V to go to the following URL instead of
	 * downloading more into our own object. This is not implementable
	 * yet anyway.
	 */
	// hum... this sucks I think... TOFIX !!!
	showUrl(url, NULL, NULL);

#if 0
	flashHandle = (FlashHandle) client_data;
	printf("LoadMovie: %s @ %d\n", url, level);
	//if (readFile(url, &buffer, &size) > 0) {
	FlashParse(flashHandle, level, buffer, size);
	//}
#endif
}

