;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Divides an unsigned 64-bit integer by an unsigned 32-bit integer.
; Result is a unsigned 32-bit integer.
;
; ULONG q_divu020(UQWORD *dividend, ULONG divisor)
; d0                  a0               d0
;
; Note: only supports 68020-68040. Can overflow.
;
; $Id: q_divu020.s,v 1.2 2001/09/27 14:00:44 zapek Exp $

	section "text",code

	xdef	_q_divu020

_q_divu020:
	movem.l	d1-d2,-(sp)
	move.l	(a0),d1     ;MSB
	move.l	4(a0),d2	;LSB
	divu.l	d0,d1:d2
	move.l	d2,d0
	movem.l (sp)+,d1-d2
	rts

	END
