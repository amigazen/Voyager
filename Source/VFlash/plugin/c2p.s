;
; Fast specialised c2p routine
;

	section	"text",code

	xdef	_makeclut
;	a0 = clut dest array
;	a1 = rgbmap (UBYTE*)
;	a2 = src canvas
;	d0 = size (pixels)

_makeclut:
	moveq	#0,d1
mcl:
	move.w	(a2)+,d1
	subq.l	#1,d0
	bmi.s	mcend
	move.b	0(a1,d1.l),(a0)+
	bra.s	mcl
mcend:
	rts



; Input:
; D0 = number of bytes
; D1 = number of planes
; A0 = chunky pixels
; A1 = pointer to array of plane pointers
; A2 = rgb16to8 table
;

	xdef	_writechunky
_writechunky:
	movem.l	a2-a6/d2-d4,-(sp)
	move.l	a2,a6
	lea		bufferblock(a4),a2	; buffer bytes
	moveq	#0,d4

beginblock:
	; clear buffer block
	moveq	#0,d2
	move.l	a2,a3
	move.l	d2,(a3)+
	move.l	d2,(a3)+
	move.l	d2,(a3)+
	move.l	d2,(a3)+
	move.l	d2,(a3)+
	move.l	d2,(a3)+
	move.l	d2,(a3)+
	move.l	d2,(a3)

	move.l	#$80000000,d3	; shift mask
inblock:
	subq.w	#1,d0
	bmi.s	endblock		; done with it all

	move.w	(a0)+,d4		; read next chunky byte
	move.b	(a6,d4.l),d2

	btst	#0,d2
	beq.s	tl1
	or.l	d3,(a2)
tl1:
	btst	#1,d2
	beq.s	tl2
	or.l	d3,4(a2)
tl2:
	btst	#2,d2
	beq.s	tl3
	or.l	d3,8(a2)
tl3:
	btst	#3,d2
	beq.s	tl4
	or.l	d3,12(a2)
tl4:
	btst	#4,d2
	beq.s	tl5
	or.l	d3,16(a2)
tl5:
	btst	#5,d2
	beq.s	tl6
	or.l	d3,20(a2)
tl6:
	btst	#6,d2
	beq.s	tl7
	or.l	d3,24(a2)
tl7:
	btst	#7,d2
	beq.s	endplanes
	or.l	d3,28(a2)
endplanes:
	lsr.l	#1,d3
	bcc.s	inblock			; if carry is clear, we're not yet done
							; with the block
endblock:
	; ok, current block is complete
	; copy bufferblock to plane pointers
	; two possibilites -- we completed before
	; the first 16 bits were finished, so we
	; must do a word copy only

	and.l	#$ffffc000,d3
	bne.s	needwordcopy

	; ok, we can do a full 32 bit copy of
	; buffer to actual bitmap
	move.l	a1,a3
	move.l	a2,a4
	move.l	d1,d2			; copy just specified number of planes
	bra.s	copyl2
copyl1:
	move.l	(a3),a5			; fetch real plane ptr
	move.l	(a4)+,(a5)+		; write buffer long word into plane
	move.l	a5,(a3)+		; write back modified plane ptr
copyl2:
	dbf		d2,copyl1
checkdone:
	tst.w	d0				; any more bytes to do?
	bpl	beginblock
	movem.l	(sp)+,d2-d4/a2-a6
	rts

needwordcopy:
	btst.l	#31,d3
	bne.s	copywex

	move.l	a1,a3
	move.l	a2,a4
	move.l	d1,d2			; copy just specified number of planes
	bra.s	copyw2
copyw1:
	move.l	(a3)+,a5		; fetch real plane ptr
	move.w	(a4),(a5)		; write buffer long word into plane
	addq.l	#4,a4			; next buffer
copyw2:
	dbf		d2,copyw1
copywex:
	movem.l	(sp)+,d2-d4/a2-a6
	rts

	section	"__MERGED",BSS

bufferblock:
	ds.l	8				; 8 * 4 bytes temp storage

	END
