
	section	text,code

	xdef	_rev16

_rev16:	; a0 = srcptr, a1 = destptr, d0 = size (longwords!)
	move.l	(a0)+,d1
	rol.w	#8,d1
	swap	d1
	rol.w	#8,d1
	swap	d1
	move.l	d1,(a1)+
	subq.l	#1,d0
	bne.s	_rev16
	rts

	end
