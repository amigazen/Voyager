;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Substracts 'b' from 'a' which are signed 64-bit integers.
; Result is put into 'res' which is is a signed 64-bit integer.
;
; void q_sub(QWORD *a, QWORD *b, QWORD *res)
;                  a0        a1        a2
;
; $Id: q_sub.s,v 1.1.1.1 2001/07/07 19:34:27 zapek Exp $

	section "text",code

	xdef	_q_sub

_q_sub:
	movem.l d0-d3,-(sp)
	move.l	(a0),d2		;MSB a
	move.l	4(a0),d3	;LSB a
	move.l	(a1),d0		;MSB b
	sub.l	4(a1),d3	;sub LSB b to LSB a
	subx.l	d0,d2		;subx MSB b to MSB a
	move.l	d2,(a2)		;put MSB result
	move.l	d3,4(a2)	;and LSB result

	movem.l (sp)+,d0-d3

	END
