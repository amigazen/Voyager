; $Id: textfit.s,v 1.3 2000/10/14 13:49:18 owagner Exp $

	section "text",code

	include "utility/hooks.i"
	include "intuition/classes.i"
	include "graphics/clip.i"
	include "exec/nodes.i"
	include "exec/lists.i"
	include "exec/funcdef.i"
	include "exec/exec_lib.i"
	include 'exec/types.i'
	include 'graphics/text.i'

;
; Fast textfit
; ------------
;
; A0 = text pointer
; A1 = pointer to TextFont
; D0.w = length of text
; D1.w = pixel width
; D2.b = soft styles
;
	xdef    _ptextfit
_ptextfit:
	ext.l   d0
	btst    #1,d2   ; Bold?
	beq.s   nsub1
	subq.w  #1,d1
nsub1:
	btst #2,d2  ; Italic?
	beq.s   nsub2
	subq.w  #1,d1
nsub2:
	btst    #5,tf_Flags(a1)
	beq.s   isfixed
	movem.l d2-d6/a2-a3,-(sp)
; Setup tf_CharSpace
;
; Calculate style offset
	move.l  tf_CharSpace(a1),a2
	move.l  tf_CharKern(a1),a3
; Setup LoChar (in D2)
	moveq   #0,d2
	moveq   #0,d3
	move.l  d0,d5
	move.w  tf_XSize(a1),d6
	move.b  tf_LoChar(a1),d2
	move.b  tf_HiChar(a1),d3
	sub.l   d2,d3
	bra.s   loop1
loop:
	moveq   #0,d4
	move.b  (a0)+,d4
	sub.l   d2,d4
	bmi.s   setmiss ; below lochar
	cmp.l   d3,d4
	bhi.s   setmiss ; above hichar
cont:
	add.w   d4,d4
	sub.w   0(a2,d4.w),d1
	sub.w   0(a3,d4.w),d1
;   bmi.s   exit
loop1:
	dbmi    d0,loop
exit:
	sub.w   d0,d5
	move.w  d5,d0
	subq.w  #1,d0
	movem.l (sp)+,d2-d6/a2-a3
	rts
setmiss:
	move.w  d6,d4
	bra.s   cont

isfixed:
	move.l  d0,a0
	move.l  d1,d0
	move.w  tf_XSize(a1),d1
	divu    d1,d0
	and.l   #$ffff,d0
	cmp.l   d0,a0
	bhi.s   exitfixed
	move.l  a0,d0
exitfixed:
	rts

;
;   FAAAST TextLength()
;
;
; A0 = text pointer
; A1 = pointer to TextFont
; D0 = length of text
; D1 = soft styles

	xdef    _ptextlen
_ptextlen:
	move.l  d2,-(sp)
	moveq   #0,d2
	btst    #1,d1   ; Bold?
	beq.s   lnsub1
	addq.w  #1,d2
lnsub1:
	btst    #2,d1   ; Italic?
	beq.s   lnsub2
	addq.w  #1,d2
lnsub2:
	btst    #5,tf_Flags(a1)
	beq.s   lenisfixed
	move.l  d2,d1
	movem.l d3-d6/a2-a3,-(sp)
; Setup tf_CharSpace
	move.l  tf_CharSpace(a1),a2
	move.l  tf_CharKern(a1),a3
; Setup LoChar (in D2)
	moveq   #0,d2
	moveq   #0,d3
	move.l  d0,d5
	move.w  tf_XSize(a1),d6
	move.b  tf_LoChar(a1),d2
	move.b  tf_HiChar(a1),d3
	sub.l   d2,d3
	bra.s   lloop1
lloop:
	moveq   #0,d4
	move.b  (a0)+,d4
	sub.l   d2,d4
	bmi.s   lsetmiss    ; below lochar
	cmp.l   d3,d4
	bhi.s   lsetmiss    ; above hichar
lcont:
	add.w   d4,d4
	add.w   0(a2,d4.w),d1
	add.w   0(a3,d4.w),d1
lloop1:
	dbf     d0,lloop
	move.l  d1,d0
	movem.l (sp)+,d3-d6/a2-a3
	move.l  (sp)+,d2
	rts
lsetmiss:
	move.w  d6,d4
	bra.s   lcont
lenisfixed:
	move.w  tf_XSize(a1),d1
	mulu    d1,d0
	add.w   d2,d0
	move.l  (sp)+,d2
	rts

	xdef    _patextlen
; A0 = text pointer
; A1 = pointer to space array
; D0 = length of text
; D1 = soft styles
_patextlen:
	movem.l d2/d3,-(sp)
	moveq   #0,d2
	moveq   #0,d3
	btst    #1,d1   ; Bold?
	beq.s   alnsub1
	addq.w  #1,d2
alnsub1:
	btst    #2,d1   ; Italic?
	beq.s   alnsub2
	addq.w  #1,d2
	bra.s   alnsub2
alns1:
	move.b  (a0)+,d3
	move.b  0(a1,d3),d3
	add.w   d3,d2
alnsub2:
	dbf d0,alns1
	move.l  d2,d0
	movem.l (sp)+,d2/d3
	rts

; A0 = text pointer
; A1 = pointer to width array
; D0.w = length of text
; D1.w = pixel width
; D2.b = soft styles
;
	xdef    _patextfit
_patextfit:
	movem.l d3/d4,-(sp)
	moveq   #0,d3
	move.l  d0,d4
	btst    #1,d2   ; Bold?
	beq.s   ansub1
	moveq   #1,d3
ansub1:
	btst #2,d2  ; Italic?
	beq.s   ansub2
	addq.w  #1,d3
ansub2:
	sub.w   d3,d1
	bgt.s   ansub3
	moveq   #0,d0
	bra.s   anquit
anloop:
	move.b  (a0)+,d3
	move.b  0(a1,d3),d3
	sub.w   d3,d1
ansub3:
	dbmi    d0,anloop
	sub.w   d0,d4
	subq.w  #1,d4
	move.l  d4,d0
anquit:
	movem.l (sp)+,d3/d4
	rts


;***************************************************************************************************************************

;BOOL __asm _IsRectangleVisibleInLayer(_a0 struct Layer *l,_d0 WORD x0,_d1 WORD y0,_d2 WORD x1,_d3 WORD y1)
;
;	struct ClipRect *cr;
;	x0 += layer->bounds.MinX;
;	y0 += layer->bounds.MinY;
;	x1 += layer->bounds.MinX;
;	y1 += layer->bounds.MinY;
;	for (cr=layer->ClipRect;cr;cr=cr->Next)
;	{
;		if (x0 > cr->bounds.MaxX) continue;
;		if (y0 > cr->bounds.MaxY) continue;
;		if (x1 < cr->bounds.MinX) continue;
;		if (y1 < cr->bounds.MinY) continue;
;		return(TRUE);
;	}
;	return(FALSE);

	XDEF __IsRectangleVisibleInLayer
__IsRectangleVisibleInLayer:
	movem.l	d2/d3/d7,-(sp)

	move.l	lr_MinX(a0),d7
	add.w	d7,d1
	add.w	d7,d3
	swap	d7
	add.w	d7,d0
	add.w	d7,d2
	;add.w	lr_MinX(a0),d0
	;add.w	lr_MinY(a0),d1
	;add.w	lr_MinX(a0),d2
	;add.w	lr_MinY(a0),d3

	lea	lr_ClipRect(a0),a0
testcr:
	move.l	(a0),d7
	beq	nothing
	move.l	d7,a0

	move.l	cr_MaxX(a0),d7
	cmp.w	d7,d1
	bgt	testcr
	swap	d7
	cmp.w	d7,d0
	bgt	testcr

	move.l	cr_MinX(a0),d7
	cmp.w	d7,d3
	blt	testcr
	swap	d7
	cmp.w	d7,d2
	blt	testcr

	moveq.l	#1,d0
	movem.l	(sp)+,d2/d3/d7
	rts
nothing:
	moveq.l	#0,d0
	movem.l	(sp)+,d2/d3/d7
	rts

	END
