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

/*
 * $Id: gifdecode.h,v 1.6 2001/07/08 14:03:30 owagner Exp $
 */

#ifndef GIFDECODE_H
#define GIFDECODE_H
#define	MAX_LZW_BITS	12	/* maximum LZW code size */
#define LZW_TABLE_SIZE	(1<<MAX_LZW_BITS) /* # of possible LZW symbols */

#include "globals.h"

struct lzw_context {
	struct gifhandle *gifh;

	/* State for GetCode and LZWReadByte */
	char code_buf[256+4];		/* current input data block */
	int last_byte;		/* # of bytes in code_buf */
	int last_bit;			/* # of bits in code_buf */
	int cur_bit;			/* next bit index to read */
	int out_of_blocks;	/* TRUE if hit terminator data block */

	int input_code_size;		/* codesize given in GIF file */
	int clear_code,end_code;	/* values for Clear and End codes */

	int code_size;		/* current actual code size */
	int limit_code;		/* 2^code_size */
	int max_code;			/* first unused code value */
	int first_time;		/* flags first call to LZWReadByte */

	/* Private state for LZWReadByte */
	int oldcode;			/* previous LZW symbol */
	int firstcode;		/* first byte of oldcode's expansion */

	/* LZW symbol table and expansion stack */
	UWORD 	symbol_head[ LZW_TABLE_SIZE ];	/* => table of prefix symbols */
	UBYTE  	symbol_tail[ LZW_TABLE_SIZE ];	/* => table of suffix bytes */
	UBYTE	symbol_stack[ LZW_TABLE_SIZE ];	/* => stack for symbol expansions */
	UBYTE	*sp;			/* stack pointer */
};
#endif /* GIFDECODE_H */
