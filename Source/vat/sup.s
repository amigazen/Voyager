	section	"text",code

* Stubs for Pool.lib
	xref	_AsmAllocPooled
	xref	_AsmCreatePool
	xref	_AsmDeletePool
	xref	_AsmFreePooled

poolsysbase:
	dc.b	'Hell'
dsegptr:
	dc.b	'oWor'
libbaseptr:
	dc.b	'ld:)'

	xdef	_initpoolsysbase
_initpoolsysbase:
	lea	poolsysbase(pc),a1
;	movem.l	a0/a4/a6,(a1)+
	move.l	a0,(a1)+
	move.l	a4,(a1)+
	move.l	a6,(a1)
	rts

	xdef	_VAT_CreatePool
_VAT_CreatePool:
	move.l	a6,-(sp)
	move.l	poolsysbase(pc),a6
;	and.l	#~$3000000,d0	; clear MEMF_VMEM / MEMF_PROTECT
	jsr	_AsmCreatePool(pc)
	move.l	(sp)+,a6
	rts

	xdef	_VAT_DeletePool
_VAT_DeletePool:
	move.l	a6,-(sp)
	move.l	poolsysbase(pc),a6
	jsr	_AsmDeletePool(pc)
	move.l	(sp)+,a6
	rts

	xdef	_VAT_AllocPooled
_VAT_AllocPooled:
	move.l	a6,-(sp)
	move.l	poolsysbase(pc),a6
	jsr	_AsmAllocPooled(pc)
	move.l	(sp)+,a6
	rts

	xdef	_VAT_FreePooled
_VAT_FreePooled:
	move.l	a1,d1	; zero ptr?
	beq.s	fpr
	move.l	a6,-(sp)
	move.l	poolsysbase(pc),a6
	jsr	_AsmFreePooled(pc)
	move.l	(sp)+,a6
fpr:
	rts

	xdef	_VAT_FreeVecPooled	; a0 = pool, a1 = data
_VAT_FreeVecPooled:
	move.l	a1,d0
	beq.s	fvp
	move.l	-(a1),d0
	move.l	a6,-(sp)
	move.l	poolsysbase(pc),a6
	jsr	_AsmFreePooled(pc)
	move.l	(sp)+,a6
fvp:
	rts

	xdef	_VAT_AllocVecPooled
_VAT_AllocVecPooled:
	addq.l	#4,d0
	movem.l	d0/a6,-(sp)
	move.l	poolsysbase(pc),a6
	jsr	_AsmAllocPooled(pc)
	movem.l	(sp)+,d1/a6
	tst.l	d0
	beq.s	avp_fail
	move.l	d0,a0
	move.l	d1,(a0)+
	move.l	a0,d0
avp_fail:
	rts

	xdef	_VAT_StrDupPooled	; a0 = pool, a1 = string
_VAT_StrDupPooled:
	move.l	a2,-(sp)
	move.l	a1,a2
sdl1:
	tst.b	(a1)+
	bne.s	sdl1
	move.l	a1,d0
	sub.l	a2,d0	; abziehen, 0-Byte stimmt dann gleich ;-) / substract, 0 Byte is correct then :-p

	bsr.s	_VAT_AllocVecPooled
	tst.l	d0
	beq.s	sdl2	; war nix
	move.l	d0,a0

sdl3:
	move.b	(a2)+,(a0)+	; strcpy()
	bne.s	sdl3

sdl2:
	move.l	(sp)+,a2
	rts


; Support für FPrintfAsync

	xdef	_call_dofpi
	xref	_dofpi
_call_dofpi:
	move.l	a6,-(sp)
	move.l	(a3),a6
	jsr		_dofpi(pc)
	move.l	(sp)+,a6
	rts


	xdef	_call_rxhandler
	xref	_rxhandler
_call_rxhandler:
	move.l	dsegptr(pc),a4
	move.l	libbaseptr(pc),a6
	jmp		_rxhandler(pc)

	xdef	_call_regutil
	xref	_regutil_func
_call_regutil:
	move.l	dsegptr(pc),a4
	move.l	libbaseptr(pc),a6
	jmp		_regutil_func(pc)

	end
