; $Id: mynexto.s,v 1.3 2000/05/21 13:23:57 owagner Exp $

	section "text",code

	xdef _myNextObject      ; o = ObjectState (A0)
_myNextObject:
	move.l  (a0),a1         ; *o
	move.l  (a1)+,d0         ; o->Next = NULL
	beq.s   doret
	move.l  d0,(a0)
	move.l  a1,d0
	addq.l  #8,d0           ; Root-Object-Offset (note we added 4 before in the postinc)
doret:
	rts

	END
