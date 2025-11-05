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


/*-------------------------------------------------------------------*/
/* Copyright (c) 1993 by SAS Institute Inc., Cary NC                 */
/* All Rights Reserved                                               */
/*                                                                   */
/* SUPPORT:    walker - Doug Walker                                  */
/*-------------------------------------------------------------------*/
#define __USE_SYSBASE 1
#include <exec/types.h>
#include <dos/doshunks.h>
#include <time.h>

#define TIMERINTERVAL 5000   /* in milliseconds */

#if DODEBUG
#include <stdarg.h>
#include <stdio.h>
#define BUG(x) {if(dodebug) bug x ;}
void bug(char *fmt, ...);
static int dodebug;
#define FUNCENTRY BUG((__FUNC__ ": entry\n"))
#else
#define BUG(x)
#define FUNCENTRY
#endif

#define SPROFPORT "SPROF_Profiler"

typedef unsigned long sptime;

struct SPDAT
{
   char *id;        // id of function (NULL for ignore)
   sptime clk;      // Clock at entry
   sptime subrs;    // Amount of time spent in subroutines
   sptime off;      // Amount of time under PROFILE_OFF
};
#define SIZSPDAT sizeof(struct SPDAT)

extern struct SPDAT *spdat;
extern int spcur, spmax;
#define SPINCR 500

typedef struct SPROFMSG
{
   struct Message m;
   ULONG process;
   sptime clk;
   char *id;
   ULONG a7;
   ULONG flags;
} *SPM;

#define SIZSPM sizeof(struct SPROFMSG)

/* Values for the 'flags' field of SPROFMSG */
#define SPROF_INIT   0x00000001  // Initialize connection
#define SPROF_ENTRY  0x00000002  // Function entry
#define SPROF_EXIT   0x00000004  // Function exit
#define SPROF_TERM   0x00000008  // Terminate connection, program continues
#define SPROF_ABORT  0x00000010  // Abort program
#define SPROF_DENIED 0x00000020  // Connection refused

struct GPInfo
{
   char *id;       // Used for sorting while program is in mem
   char *name;     // Name of function
   sptime time;    // Time excluding subroutines
   sptime tottime; // Time including subroutines
   ULONG count;    // Number of calls
};

extern struct GPInfo **GPInfo;
extern int GPCur, GPMax;
#define SIZGPINFO sizeof(struct GPInfo)
#define GPINCR 256

void Report(sptime now);
struct GPInfo *FindGPI(struct GPInfo ***GPInfo, char *id,
                       int *cur, int *tot);

/* Functions defined in timer.c */
long OpenTimer(void);
void _STDCloseTimer(void);
#define CloseTimer() _STDCloseTimer()
void PostTimerReq(long time);
void GetTimerPkt(long time, int wait);

