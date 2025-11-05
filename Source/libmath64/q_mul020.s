;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Multiplies a signed 32-bit integer by a signed 32-bit integer.
; Result is a signed 64-bit integer.
;
; void q_mul020(LONG multiplicand, LONG multiplier, QWORD *res)
;                 d0                    d1                a0
; 
; Note: only supports 68020-68040.
;
; $Id: q_mul020.s,v 1.3 2001/09/27 14:00:44 zapek Exp $

	section "text",code

	xdef	_q_mul020

_q_mul020:
	move.l	d2,-(sp)
	muls.l	d0,d2:d1
	move.l	d2,(a0)
	move.l	d1,4(a0)
	move.l 	(sp)+,d2
	rts

	END
