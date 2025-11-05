;
; Assembler support module
;
; $Id: sup.s,v 1.5 2001/10/07 23:23:12 zapek Exp $

	section "text",code

; fpn( d1 ID )

	xdef    _fpn
	xdef    _fpn_clone
	xref    _prefslist
	xref    _clonelist
_fpn_clone:
	move.l  _clonelist(a4),d0
	bra.s   fpl1
_fpn:
	move.l  _prefslist(a4),d0   ; -> head
fpl1:
	move.l  d0,a0
	move.l  (a0),d0             ; -> next
	beq.s   fpl2
	cmp.l   8(a0),d1
	bne.s   fpl1
	move.l  a0,d0   
fpl2:
	rts

	xdef    _getprefs
_getprefs:
	move.l  _prefslist(a4),d0   ; -> head
gpl1:
	move.l  d0,a0
	move.l  (a0),d0             ; -> next
	beq.s   gpl2
	cmp.l   8(a0),d1
	bne.s   gpl1
	lea     16(a0),a0
	move.l  a0,d0
gpl2:
	rts


;
; fake __tzset() to force GMT0
; without constant reading of ENV:TZ et al
;
; requires small data model
;

	section "text",code

	xdef    @__tzset
	xdef    ___tzset

	xref    ___timezone
	xref    ___daylight

@__tzset:
___tzset:
	moveq   #0,d0
	move.l  d0,___timezone(a4)
	move.l  d0,___daylight(a4)
ij1:
	rts

;
;
;   
;	 xdef    _initialjump
;	 xref    _serialnumber
;_initialjump:
;	 move.l  a2,-(sp)
;	 lea     _serialnumber+33(a4),a1
;	 tst.l   -33(a1)
;	 bne.s   ij1
;	 jmp     (a0)

	xdef    _replace160
_replace160: ; a0 = buffer
	move.b  #160,d0
rp160loop:
	move.b  (a0)+,d1
	beq.s   exrp160
	cmp.b   d0,d1
	bne.s   rp160loop
	move.b  #' ',-1(a0)
	bra.s   rp160loop
exrp160:
	rts

	END
