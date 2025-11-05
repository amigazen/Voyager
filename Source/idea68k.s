;-------------------------------------------------------------------------
; idea68k.a 
;
; New version of assembler idea cipher
;
; Author: Risto Paasivirta <paasivir@jyu.fi>
;
;-------------------------------------------------------------------------
;

		section	text,code

;-------------------------------------------------------------------------
; Functions defined in this module
;-------------------------------------------------------------------------
; Internal functions


		xdef	__mul_idea		; mul mod 65537
		xdef	__inv_idea		; inv mod 65537
		xdef	__en_key_idea		; userkey to encrypt subk.
		xdef	__de_key_idea		; invert subkey
		xdef	__cipher_idea		; cipher core

;-------------------------------------------------------------------------
; Key handling

		xdef	_idea_key_schedule	; full key schedule

;-------------------------------------------------------------------------
; En/Decryption

		xdef	_idea_ebc_encrypt	; electronic code book
		xdef	_idea_cbc_encrypt	; cipher block chaining

;-------------------------------------------------------------------------
; Constants
;-------------------------------------------------------------------------
; subkey size

ZSIZE		equ	108

;-------------------------------------------------------------------------
; Internal functions
;-------------------------------------------------------------------------
; macro: imul dea,dn , d7=scratch
; Arg 1 accessed only once (can be (an)+) Handles zero value arguments.
;

imul		macro
		move.w	\2,d7
		beq.b	imul0\@
		mulu.w	\1,d7
		beq.b	imul1\@
		move.w	d7,\2
		swap	d7
		sub.w	d7,\2
		moveq	#0,d7
		addx.w	d7,\2	
		bra.b	imul3\@

imul0\@		move.w	\1,\2		
imul1\@		neg.w	\2
		addq.w	#1,\2
imul3\@
		endm

;-------------------------------------------------------------------------
; word16 _mul_idea(word16 a,word16 b)
;
; Return a * b mod 65537

__mul_idea	move.l	d7,-(sp)
		move.w	10(sp),d0
		imul	14(sp),d0
		move.l	(sp)+,d7
		rts

;-------------------------------------------------------------------------
; word16 _inv_idea(word16 a)
; d0 = inv(d0)
;
; Return multiplicative inverse of a mod 65537
;

__inv_idea	move.w	6(sp),d0

inv		cmp.w	#2,d0		; inv(0)=0,inv(1)=1
		bcs.b	1$

		cmp.w	#3,d0
		bcc.b	2$

		move.w	#32769,d0	; inv(2)
1$		rts

2$		movem.l	d2-d5,-(sp)
		move.l	#$10001,d1	; d1 = n1
		moveq	#1,d2		; d2 = b2
		moveq	#0,d3		; d3 = b1		

inv_loop	divu.w	d0,d1
		move.l	d1,d4
		swap	d4		; r = d4
		tst.w	d4
		beq.b	inv_done

		move.w	d2,d5
		muls.w	d1,d5
		exg	d3,d2
		sub.l	d5,d2
		moveq	#0,d1
		move.w 	d0,d1
		move.w	d4,d0
		bra.b	inv_loop

inv_done	tst.l	d2
		bpl.b	1$

		move.l	#$10001,d0
		add.l	d2,d0
		bra.b	2$

1$		move.l	d2,d0

2$		movem.l	(sp)+,d2-d5
		rts

;-------------------------------------------------------------------------
; _en_key_idea(a0=userkey,a1=subkey)
;
; Create 54-word encryption subkey from 8-word user key
;

__en_key_idea	movem.l	4(sp),a0-a1

en_key_idea	movem.l	d2-d4,-(sp)
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)

		suba.w	#12,a1

		moveq	#47,d0
		moveq	#0,d1
		moveq	#14,d4

1$		move.w	d1,d2
		addq.w	#2,d2
		and.w	d4,d2
		move.w	0(a1,d2.w),d3
		swap	d3
		addq.w	#2,d2
		and.w	d4,d2
		move.w	0(a1,d2.w),d3
		lsr.l	#7,d3
		move.w	d3,16(a1,d1.w)
		addq.w	#2,d1
		move.w	d1,d2
		and.w	#16,d2
		adda.w	d2,a1
		and.w	d4,d1
		dbra	d0,1$

		movem.l	(sp)+,d2-d4
		rts

;-------------------------------------------------------------------------
; _de_key_idea(a0=subkey,a1=inverted_subkey)
;
; Invert 54-word subkey for idea decrypting
;

__de_key_idea	movem.l	4(sp),a0-a1

de_key_idea	movem.l	d2-d3,-(sp)

		moveq	#96,d2
		moveq	#0,d3
		bra.b	2$

1$		move.l	8(a0,d2.w),-4(a1,d3.w)

2$		move.w	0(a0,d2.w),d0
		bsr	inv
		move.w	d0,0(a1,d3.w)
		move.l	2(a0,d2.w),d0
		neg.w	d0
		swap	d0
		neg.w	d0
		move.l	d0,2(a1,d3.w)
		move.w	6(a0,d2.w),d0
		bsr	inv
		move.w	d0,6(a1,d3.w)
		add.w	#12,d3
		sub.w	#12,d2
		bcc.b	1$

		move.l	2(a1),d0
		swap	d0
		move.l	d0,2(a1)
		move.l	98(a1),d0
		swap	d0
		move.l	d0,98(a1)

		movem.l	(sp)+,d2-d3
		rts

;-------------------------------------------------------------------------
; _cipher_idea(a0=src,a1=dest,a2=subkey)
;
; En/Decipher 4-word block from src to dest using 54-word subkey as key
;

round		macro
		imul	(a0)+,d0
		add.w	(a0)+,d1		
		add.w	(a0)+,d2
		imul	(a0)+,d3
		move.w	d0,d4
		eor.w	d2,d4
		imul	(a0)+,d4

		move.w	d1,d5
		eor.w	d3,d5
		add.w	d4,d5
		imul	(a0)+,d5
		move.w	d5,d6
		add.w	d4,d6

		eor.w	d5,d0
		move.w	d1,d4
		move.w	d2,d1
		eor.w	d5,d1
		move.w	d4,d2
		eor.w	d6,d2
		eor.w	d6,d3
		endm

__cipher_idea	move.l	a2,-(sp)
		movem.l	8(sp),a0-a2
		bsr.b	cipher_idea
		move.l	(sp)+,a2
		rts

cipher_idea	movem.l	d2-d7,-(sp)
		movem.w	(a0),d0-d3
		movea.l	a2,a0

		ifnd	UNROLL
		moveq	#7,d6
cipher_loop	swap	d6
		round
		swap	d6
		dbra	d6,cipher_loop
		endc

		ifd	UNROLL
		round
		round
		round
		round
		round
		round
		round
		round
		endc

		imul	(a0)+,d0
		exg	d1,d2
		add.w	(a0)+,d1
		add.w	(a0)+,d2
		imul	(a0)+,d3

		movem.w	d0-d3,(a1)
		movem.l	(sp)+,d2-d7
		rts

;-------------------------------------------------------------------------
; User callable routines 
;-------------------------------------------------------------------------
; idea_key_schedule(word16 *uk, word16 *ks)
;
; Create en/decryption keys from 8-word user key uk to 108-word key 
; schedule ks.
;

_idea_key_schedule
		movem.l	4(sp),a0-a1
		bsr	en_key_idea
		movea.l	8(sp),a0
		movea.l	a0,a1
		adda.w	#ZSIZE,a1
		bra	de_key_idea

;-------------------------------------------------------------------------
; idea_ebc_encrypt(word16 *src,word16 *dst,word16 *ks,int mode)
;
; En/decrypt 4 word (8-byte) block from src to dst using electronic code
; book mode.
;

_idea_ebc_encrypt
		move.l	a2,-(sp)
		movem.l	8(sp),a0-a2/d0
		tst.l	d0
		bne.s	1$
		adda.w	#ZSIZE,a2
1$
		bsr	cipher_idea
		movea.l	(sp)+,a2
		rts

;-------------------------------------------------------------------------
; idea_cbc_encrypt(word16 *src, word16 *dst, int len, word16 *ks,
;   word16 *ivec,int mode)
;
; Encrypt (mode != 0) or decrypt (mode == 0) src to dst in cipher block 
; chaining mode. Len must be multiple of 8 bytes. Ivec contains final
; vector at return.
;

_idea_cbc_encrypt
		movem.l	a2-a5/d2-d4,-(sp)

		lea	32(sp),a0
		move.l	(a0)+,a3	; src
		move.l	(a0)+,a4	; dst
		move.l	(a0)+,d2	; len
		move.l	(a0)+,a2	; ks
		move.l	(a0)+,a5	; ivec

		tst.l	(a0)		; mode?
		beq	cbc_decrypt

cbc_en_loop	move.l	(a3)+,d3	; get data to encrypt
		move.l	(a3)+,d4
		eor.l	d3,(a5)		; eor with ivec		
		eor.l	d4,4(a5)

		movea.l	a5,a0		; cipher ivec to dst
		movea.l	a4,a1
		bsr	cipher_idea
		move.l	(a4)+,(a5)	; copy cipher to ivec
		move.l	(a4)+,4(a5)

		subq	#8,d2
		bhi.b	cbc_en_loop
		
cbc_exit	movem.l	(sp)+,a2-a5/d2-d4
		rts

cbc_decrypt	adda.w	#ZSIZE,a2	; decrypt key

cbc_de_loop	move.l	(a3)+,d3	; store cipher data (next ivec)
		move.l	(a3)+,d4

		lea	-8(a3),a0	; decrypt to src
		movea.l	a4,a1
		bsr	cipher_idea

		move.l	(a5)+,d0	; swap next/curr ivec
		move.l	(a5),d1
		move.l	d4,(a5)
		move.l	d3,-(a5)

		eor.l	d0,(a4)+	; eor deciphered with ivec
		eor.l	d1,(a4)+

		subq	#8,d2
		bhi.b	cbc_de_loop

		bra.b	cbc_exit

;-------------------------------------------------------------------------
		end
;-------------------------------------------------------------------------

