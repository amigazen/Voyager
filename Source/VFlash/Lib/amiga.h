#ifndef FLASH_AMIGA_H
#define FLASH_AMIGA_H
/*
**
** Structures and stuffs...
**
**
** $Id: amiga.h,v 1.4 2004/03/07 00:10:09 zapek Exp $
*/


/*
 * If I get my hands over the guy who had the "brilliant" idea
 * of defining struct timeval with ULONGs, he'll remember me...
 */
struct unixtimeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

void GetTimeOfDay(struct unixtimeval *tv);

extern unsigned char *SQRT; /* array for computing circles, etc... */

struct mpega_openbuf {
	char *buf;
	int bufsize;
};

void * mpega_open(struct mpega_openbuf *ob);
void mpega_close(void * handle);
long mpega_decode(void * handle, char *buf, int stereo);
unsigned long mpega_getsize(void * handle);
void extract_streamnum(char *buf, int size);

#endif /* FLASH_AMIGA_H */
