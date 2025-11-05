;
; Fast specialised c2p routine
;

; Input:
; D0 = number of bytes
; D1 = number of planes
; A0 = chunky pixels
; A1 = pointer to array of plane pointers
;

	section "text",code

	xdef    _writechunky
_writechunky:
	movem.l a2-a5/d2-d3,-(sp)
	lea     bufferblock(a4),a2  ; buffer bytes

beginblock:
	; clear buffer block
	moveq   #0,d2
	move.l  a2,a3
	move.l  d2,(a3)+
	lea     12(a3),a5
	move.l  d2,(a5)+
	move.l  d2,(a3)+
	move.l  d2,(a5)+
	move.l  d2,(a3)+
	move.l  d2,(a5)+
	move.l  d2,(a3)+
	move.l  #$80000000,d3   ; shift mask
	move.l  d2,(a5)

inblock:
	subq.w  #1,d0
	bmi.s   endblock        ; done with it all

	move.b  (a0)+,d2        ; read next chunky byte
	beq.s   endplanes       ; shortcircuit
	bpl.s   tl7             ; Bit 7 == MI flag
	or.l    d3,28(a2)
tl7:
	btst    #0,d2
	beq.s   tl1
	or.l    d3,(a2)
tl1:
	btst    #1,d2
	beq.s   tl2
	or.l    d3,4(a2)
tl2:
	btst    #2,d2
	beq.s   tl3
	or.l    d3,8(a2)
tl3:
	btst    #3,d2
	beq.s   tl4
	or.l    d3,12(a2)
tl4:
	btst    #4,d2
	beq.s   tl5
	or.l    d3,16(a2)
tl5:
	btst    #5,d2
	beq.s   tl6
	or.l    d3,20(a2)
tl6:
	btst    #6,d2
	beq.s   endplanes
	or.l    d3,24(a2)
endplanes:
	lsr.l   #1,d3
	bcc.s   inblock         ; if carry is clear, we're not yet done
							; with the block
nowordcopy:
	; ok, we can do a full 32 bit copy of
	; buffer to actual bitmap
	move.l  a1,a3
	move.l  a2,a4
	move.l  d1,d2           ; copy just specified number of planes
	bra.s   copyl2
copyl1:
	move.l  (a3),a5         ; fetch real plane ptr
	move.l  (a4)+,(a5)+     ; write buffer long word into plane
	move.l  a5,(a3)+        ; write back modified plane ptr
copyl2:
	dbf     d2,copyl1
checkdone:
	tst.w   d0              ; any more bytes to do?
	bpl beginblock
	movem.l (sp)+,d2-d3/a2-a5
	rts

endblock:
	; ok, current block is complete
	; copy bufferblock to plane pointers
	; two possibilites -- we completed before
	; the first 16 bits were finished, so we
	; must do a word copy only

	and.l   #$ffff8000,d3
	beq.s   nowordcopy

needwordcopy:
	btst.l  #31,d3
	bne.s   copywex

	move.l  a1,a3
	move.l  a2,a4
	move.l  d1,d2           ; copy just specified number of planes
	bra.s   copyw2
copyw1:
	move.l  (a3)+,a5        ; fetch real plane ptr
	move.w  (a4),(a5)       ; write buffer long word into plane
	addq.l  #4,a4           ; next buffer
copyw2:
	dbf     d2,copyw1
copywex:
	movem.l (sp)+,d2-d3/a2-a5
	rts



;
; MakeMask
; --------
;
; A0 = pen array
; A1 = pointer to plane
; D0 = xsize
; D1 = mask pen
; 
;
	xdef    _makemaskline
_makemaskline:
	movem.l d2-d4,-(sp)
	moveq   #0,d4   ; usage flag
maskloopouter:
	moveq   #15,d2  ; current shift value
	moveq   #-1,d3  ; current mask word
maskloop:
	subq.w  #1,d0
	bmi.s   quitmask
	cmp.b   (a0)+,d1
	beq.s   clrit
	or.b	#1,d4 ; Non-Transparent Value found
	dbf     d2,maskloop
	move.w  d3,(a1)+
	bra.s   maskloopouter
clrit:
	or.b	#2,d4	; mask was used
	bclr.l  d2,d3
	dbf     d2,maskloop
	move.w  d3,(a1)+
	bra.s   maskloopouter

quitmask:
	cmp.w   #15,d2
	beq.s   reallyquitmask
	move.w  d3,(a1)
reallyquitmask:
	move.l  d4,d0
	movem.l (sp)+,d2-d4
	rts

;
; PenConvert
; ----------
;
; A0 = source buffer
; A1 = destination buffer
; A2 = pen array
; D0 = size
; D1 = null
;
	xdef _penarrayconvert
pcv2:
	move.b  (a0)+,d1
	move.b  0(a2,d1),(a1)+
_penarrayconvert:
	subq.l  #1,d0
	bpl.s   pcv2
	rts


;
; StripRGBA
;
; A0 = source buffer
; A1 = dest buffer
; D0 = num

	xdef _striprgba
srgba:
;    IFD 68020
	move.w  (a0)+,(a1)+
;    ELSE
;    move.b  (a0)+,(a1)+
;    move.b  (a0)+,(a1)+
;    ENDC
	move.b  (a0)+,(a1)+
	addq.l  #1,a0
_striprgba:
	dbf d0,srgba
	rts

;
; PenArray to ColMap
;
; A0 = penarray
; A1 = RGB dest
; A2 = colmap
; D0 = num

	xdef _penarray2rgb
_penarray2rgb:
	move.l  d2,-(sp)
	moveq   #0,d1
	bra.s   pargb2
pargb:
	move.b  (a0)+,d1
	move.l  d1,d2
	add.w   d1,d2
	add.w   d1,d2
;    IFD 68020
	move.w  (a2,d2),(a1)+
;    ELSE
;    move.b  0(a2,d2),(a1)+
;    move.b  1(a2,d2),(a1)+
;    ENDC
	move.b  2(a2,d2),(a1)+
pargb2:
	dbf d0,pargb
	move.l  (sp)+,d2
	rts

	section "__MERGED",BSS

bufferblock:
	ds.l    8               ; 8 * 4 bytes temp storage

	END
