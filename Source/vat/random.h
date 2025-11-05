/*

random.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sat Mar  4 14:49:05 1995 ylo

Cryptographically strong random number generator.

*/

/*
 * $Id: random.h,v 1.2 1999/05/15 19:55:46 owagner Exp $
 * $Log: random.h,v $
 * Revision 1.2  1999/05/15 19:55:46  owagner
 * added VAT_NewShowRegUtil() to pass a default product
 *
 * Revision 1.1.1.1  1996/02/18  21:38:10  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.3  1995/09/13  12:00:02  ylo
 * 	Changes to make this work on Cray.
 *
 * Revision 1.2  1995/07/13  01:29:28  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef RANDOM_H
#define RANDOM_H

#include "md5.h"

#define RANDOM_STATE_BITS	8192
#define RANDOM_STATE_BYTES	(RANDOM_STATE_BITS / 8)

/* Structure for the random state. */
typedef struct
{
  unsigned char state[RANDOM_STATE_BYTES];/* Pool of random data. */
  unsigned char stir_key[64];		/* Extra data for next stirring. */
  unsigned int next_available_byte;	/* Index of next available byte. */
  unsigned int add_position;		/* Index to add noise. */
} RandomState;

#endif /* RANDOM_H */
