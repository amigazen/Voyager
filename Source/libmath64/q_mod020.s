;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Divides a signed 64-bit integer by a signed 32-bit integer.
; Result is a signed 32-bit integer with the rest.
;
; LONG q_mod020(QWORD *dividend, LONG divisor)
; d0                a0               d0
;
; Note: only supports 68020-68040.
;
; $Id: q_mod020.s,v 1.2 2001/09/27 14:00:44 zapek Exp $

	section "text",code

	xdef	_q_mod020

_q_mod020:
	movem.l	d1-d2,-(sp)
	move.l	(a0),d1     ;MSB
	move.l	4(a0),d2	;LSB
	divs.l	d0,d1:d2
	move.l	d1,d0
	movem.l (sp)+,d1-d2
	rts

	END
