#ifndef VFLASH_CONFIG_H
#define VFLASH_CONFIG_H
/*
 * VFlash config options
 * ---------------------
 * - General config file for the whole build
 *
 * © 2001 by David Gerber <zapek@vapor.com>
 * All Rights Reserved
 *
 * $Id: config.h,v 1.2 2004/02/01 15:18:01 zapek Exp $
 */

#include "compilers.h" /* uses the one from voyager, be sure it's in the include path */


/*
 * Features (alphabetically ordered). Don't forget
 * to update the default features list below !
 *
 * Target platforms: MBX, AMIGAOS, POWERUP, __MORPHOS__
 */
#if defined(MBX)

#define USE_ASYNCIO			0
#define USE_CLUT            0
#define USE_CGX             0
#define USE_INTERNAL_MALLOC 0
#define USE_PICASSO96       0
#define USE_PLANAR          0
#define USE_PREFS_IO		0
#define USE_REENTRANCY      0
#define USE_SOUND           0
#define USE_STATIC_LINK		1

#elif defined(AMIGAOS)

#define USE_ASYNCIO			1
#define USE_CLUT            1
#define USE_CGX             1
#define USE_INTERNAL_MALLOC 0
#define USE_PICASSO96       0 /* XXX: temporary */
#define USE_PLANAR          1
#define USE_PREFS_IO		0 /* XXX: temporary */
#define USE_REENTRANCY      0
#define USE_SOUND           0
#define USE_STATIC_LINK		0

#elif defined(POWERUP)

#define USE_ASYNCIO			1
#define USE_CLUT            1
#define USE_CGX             1
#define USE_INTERNAL_MALLOC 0
#define USE_PICASSO96       1
#define USE_PLANAR          1
#define USE_PREFS_IO		1
#define USE_REENTRANCY      0
#define USE_SOUND           0
#define USE_STATIC_LINK		0

#elif defined(__MORPHOS__)

#define USE_ASYNCIO			1
#define USE_CLUT            1
#define USE_CGX             1
#define USE_INTERNAL_MALLOC 0
#define USE_PICASSO96       0
#define USE_PLANAR          0
#define USE_PREFS_IO		1
#define USE_REENTRANCY      0
#define USE_SOUND           1
#define USE_STATIC_LINK		0

#endif

/*
 * Use asyncio.library if present
 */
#if !USE_ASYNCIO
#undef USE_ASYNCIO
#endif

/*
 * Enable CLUT mode
 */
#if !USE_CLUT
#undef USE_CLUT
#endif

/*
 * Enable CyberGraphX support
 */
#if !USE_CGX
#undef USE_CGX
#endif

/*
 * Use the internal malloc implementation in malloc.c
 */
#if !USE_INTERNAL_MALLOC
#undef USE_INTERNAL_MALLOC
#endif

/*
 * Enable optimized Picasso96 support (USE_CGX required)
 */
#if !USE_PICASSO96
#undef USE_PICASSO96
#endif

/*
 * Enable Amiga legacy planar mode (OCS/ECS and AGA)
 */
#if !USE_PLANAR
#undef USE_PLANAR
#endif

/*
 * Loads and saves the prefs
 */
#if !USE_PREFS_IO
#undef USE_PREFS_IO
#endif

/*
 * Be reentrant. This is not supported ATM.
 */
#if !USE_REENTRANCY
#undef USE_REENTRANCY
#endif

/*
 * Use sound
 */
#if !USE_SOUND
#undef USE_SOUND
#ifndef NOSOUND
#define NOSOUND /* libflash uses that */
#endif /* !NOSOUND */
#endif

/*
 * Link with V
 */
#if !USE_STATIC_LINK
#undef USE_STATIC_LINK
#endif

#endif /* !VFLASH_CONFIG_H */
