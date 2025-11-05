#ifndef VFLASH_TIMERS_H
#define VFLASH_TIMERS_H
/*
 * VFlash portable timers
 * ----------------------
 *
 * $Id: timers.h,v 1.1.1.1 2002/10/07 12:52:16 zapek Exp $
 *
 */

extern int timerqueued;
extern ULONG timersig;

int init_timer(void);
void cleanup_timer(void);
void rtimer(void);
void qtimer(struct timeval *tv);
void wtimer(void);

#endif /* !VFLASH_TIMERS_H */
