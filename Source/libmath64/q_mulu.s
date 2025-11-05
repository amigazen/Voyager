;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Multiplies an unsigned 32-bit integer by an unsigned 32-bit integer.
; Result is an unsigned 64-bit integer.
;
; Inspiration taken from http://www.cwru.edu/cse/eeap/282/21_math_instr.html
; but this page has a lot of mistakes, really. Probably written by students
; in a druken fury.
;
; void q_mulu(ULONG multiplicand, ULONG multiplier, UQWORD *res)
;                 d0                    d1                a0
; 
; $Id: q_mulu.s,v 1.3 2001/09/29 14:13:38 zapek Exp $

	section "text",code

	xdef	_q_mulu

_q_mulu:
	movem.l	d2-d5,-(sp)

	clr.l	d4			;clear result MSB
	move.l	d4,d5		;clear result LSB
	moveq.l	#31,d3		;loop count
loop:
	add.l	d5,d5		;shift product left one bit (d4:d5)
	addx.l	d4,d4		;extended

	add.l	d1,d1		;shift multiplier
	bcc.s	skip_multi
	add.l	d0,d5		;if carry then add multiplicand
	bcc.s	skip_multi
	addq.l	#1,d4		;extended
skip_multi:
	dbra	d3,loop
	move.l	d4,(a0)		;put MSB result
	move.l	d5,4(a0)    ;put LSB result
	movem.l (sp)+,d2-d5
	rts

	END
