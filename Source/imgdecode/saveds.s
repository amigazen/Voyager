
	section	text,code

store:
	dc.l	0,0

	xdef	_mystoreds
_mystoreds:
	lea	store(pc),a0
	movem.l	a4/a6,(a0)
	rts

	xdef	_mygetds
_mygetds:
	movem.l	store(pc),a4/a6
	rts

	end
