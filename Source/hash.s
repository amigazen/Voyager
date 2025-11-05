;
;
; $Id: hash.s,v 1.3 2000/07/13 07:58:31 owagner Exp $
;
;


	section "text",code

	xdef    _hash
_hash:                  ; a0 = string
	movem.l	d2/d3,-(sp)

	moveq   #0,d0       ; hashval
	moveq   #0,d1       ; clear bits 8 to 31
	moveq   #32,d2      ; offset 
	move.l	#$711d4309,d3

; find end
hl:
	move.b  (a0)+,d1
	beq.s   hl2
	sub.b   d2,d1   ; v -= 32
	asl.l   #3,d0
	add.l   d1,d0
	eor.l   d3,d0
	bra.s   hl
hl2:
	movem.l	(sp)+,d2/d3
	rts

	END
