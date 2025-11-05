;
; libmath64 © 2001 by David	Gerber <zapek@meanmachine.ch>
; All Rights Reserved
;
; Adds 2 signed 64-bit integers 'a' and 'b'.
; Result is a signed 64-bit integer put into 'res'.
;
; void q_add(QWORD *a, QWORD *b, QWORD *res)
;                  a0        a1        a2
;
; $Id: q_add.s,v 1.1.1.1 2001/07/07 19:34:27 zapek Exp $

	section "text",code

	xdef	_q_add

_q_add:
	movem.l d0-d3,-(sp)
	move.l	(a0),d2		;MSB a
	move.l	4(a0),d3	;LSB a
	move.l	(a1),d0		;MSB b
	add.l	4(a1),d3	;add LSB b to LSB a
	addx.l	d0,d2		;addx MSB b to MSB a
	move.l	d2,(a2)		;put MSB result
	move.l	d3,4(a2)	;put LSB result

	movem.l (sp)+,d0-d3

	END
