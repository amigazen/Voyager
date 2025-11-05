;
; fake __tzset() to force GMT0
; without constant reading of ENV:TZ et al
;
; requires small data model
;

	section	"text",code

	xdef	@__tzset
	xdef	___tzset

	xref	___timezone
	xref	___daylight

@__tzset:
___tzset:
	moveq	#0,d0
	move.l	d0,___timezone(a4)
	move.l	d0,___daylight(a4)
	rts

	END
