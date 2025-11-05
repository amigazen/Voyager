;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Divides a signed 64-bit integer by a signed 32-bit integer.
; Result is a signed 32-bit integer.
;
; LONG q_div020(QWORD *dividend, LONG divisor)
; d0               a0              d0
;
; Note: only supports 68020-68040. Can overflow.
;
; $Id: q_div020.s,v 1.1 2001/07/15 15:16:19 zapek Exp $

	section "text",code

	xdef	_q_div020

_q_div020:
	movem.l	d1-d7,-(sp)
	move.l	(a0),d1     ;MSB
	move.l	4(a0),d2	;LSB
	divs.l	d0,d1:d2
	move.l	d2,d0
	movem.l (sp)+,d1-d7
	rts

	END
