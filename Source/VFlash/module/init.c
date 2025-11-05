/*
 * VFlash initialization & cleanup
 * -------------------------------
 *
 * © 2000-2001 by David Gerber <zapek@vapor.com>
 *
 * $Id: init.c,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */

#include "globals.h"
#include "init.h"
#include "main.h"
#include "gfx.h"
#include "timers.h"
#include "mempool.h"

/*
 * Initialize various stuffs
 */
struct FlashInfo * initstuff(void)
{
	struct FlashInfo *fit;

	DB(("Welcome to VFlash (%s, %s) foobar\n", __DATE__, __TIME__));

	if (init_libs())
	{
		if ((mempool = myCreatePool(MEMF_PUBLIC | MEMF_CLEAR, 4096, 2048)))
		{
			DB(("mempool ok at 0x%lx\n", mempool));
			if ((fit = (struct FlashInfo *)myAllocPooled(mempool, sizeof(struct FlashInfo))))
			{
				DB(("FlashInfo allocated\n"));
				if (init_gfx())
				{
					DB(("gfx init done\n"));
					if ((vfp = (struct FlashPrefs *)myAllocPooled(mempool, sizeof(struct FlashPrefs))))
					{
						DB(("now calculating sqrt..\n"));
						if (makesqrt())
						{
							DB(("sqrt calculated\n"));
							//AHIavail = initahi();
							remove_dosreq();

							return (struct FlashInfo *)fit;
						}
					}
				}
			}
		}
	}

//	  if (IntuitionBase)
//	  {
//		  DisplayBeep(NULL);
//	  }

	return(NULL);
}


/*
 * Cleanup
 */
void closestuff(void)
{
	DB(("calling exittimer()\n"));
	cleanup_timer();

	//closeahi();

	DB(("AHI closed successfully\n"));

	DB(("freeing SQRT...\n"));
	if (SQRT)
	{
		free(SQRT);
	}

	DB(("deleting the mempool\n"));
	if (mempool)
	{
		myDeletePool(mempool);
		fi = 0;
	}

	enable_dosreq();

	DB(("closing dos.library...\n"));

	close_libs();
}
