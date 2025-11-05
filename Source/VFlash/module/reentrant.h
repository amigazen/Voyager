#ifndef VFLASH_REENTRANT_H
#define VFLASH_REENTRANT_H

//TOFIX!! include the appropriate types first (and vflash_global->SQRT)

/*
 * Global variable structure. Only used if VFlash has
 * to be reentrant. This is needed if it's statically linked
 * with Voyager.
 */
struct VFlashGlobalData {
	struct FlashInfo *fi;
	APTR mempool;
	APTR flashobj;
	int signalexit;
	struct FlashPrefs *vfp;
	struct Process *pr;
	unsigned char *SQRT;

};

#endif /* VFLASH_REENTRANT_H */
