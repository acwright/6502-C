; =============================================================================
;   rtc.s — Real time clock (DS1511Y)
; =============================================================================
;   The Kernal passes time and date around in A/X/Y, which C cannot receive.
;   These wrappers take a pointer to an RtcTime or RtcDate instead and copy
;   the fields across.
; =============================================================================

        .setcpu "65C02"

        .export _RtcReadTime, _RtcWriteTime
        .export _RtcReadDate, _RtcWriteDate
        .export _RtcReadNVRAM, _RtcWriteNVRAM

        .import popa

        .include "zeropage.inc"
        .include "6502.inc"

.code

; void __fastcall__ RtcReadTime(RtcTime *time);
;   struct layout: 0 = hours, 1 = minutes, 2 = seconds
_RtcReadTime:
        sta     ptr1
        stx     ptr1+1
        jsr     RtcReadTime     ; A = hours, X = minutes, Y = seconds
        sta     tmp1
        stx     tmp2
        sty     tmp3            ; Y is needed for indexing, so park it first
        ldy     #$00
        lda     tmp1
        sta     (ptr1),y
        iny
        lda     tmp2
        sta     (ptr1),y
        iny
        lda     tmp3
        sta     (ptr1),y
        rts

; void __fastcall__ RtcWriteTime(const RtcTime *time);
_RtcWriteTime:
        sta     ptr1
        stx     ptr1+1
        ldy     #$01
        lda     (ptr1),y
        sta     tmp1            ; minutes
        iny
        lda     (ptr1),y
        sta     tmp2            ; seconds
        ldy     #$00
        lda     (ptr1),y        ; A = hours
        ldx     tmp1
        ldy     tmp2
        jmp     RtcWriteTime

; void __fastcall__ RtcReadDate(RtcDate *date);
;   struct layout: 0 = day, 1 = month, 2 = year, 3 = century
_RtcReadDate:
        sta     ptr1
        stx     ptr1+1
        jsr     RtcReadDate     ; A = day, X = month, Y = year
        sta     tmp1
        stx     tmp2
        sty     tmp3
        ldy     #$00
        lda     tmp1
        sta     (ptr1),y
        iny
        lda     tmp2
        sta     (ptr1),y
        iny
        lda     tmp3
        sta     (ptr1),y
        iny
        lda     RTC_BUF_CENT    ; the century travels in a system variable
        sta     (ptr1),y
        rts

; void __fastcall__ RtcWriteDate(const RtcDate *date);
_RtcWriteDate:
        sta     ptr1
        stx     ptr1+1
        ldy     #$03
        lda     (ptr1),y
        sta     RTC_BUF_CENT    ; century
        ldy     #$01
        lda     (ptr1),y
        sta     tmp1            ; month
        iny
        lda     (ptr1),y
        sta     tmp2            ; year
        ldy     #$00
        lda     (ptr1),y        ; A = day
        ldx     tmp1
        ldy     tmp2
        jmp     RtcWriteDate

; unsigned char __fastcall__ RtcReadNVRAM(unsigned char addr);
_RtcReadNVRAM:
        tax
        jsr     RtcReadNVRAM
        ldx     #$00
        rts

; void __fastcall__ RtcWriteNVRAM(unsigned char addr, unsigned char value);
;   value arrives in A, addr is on the C stack.
_RtcWriteNVRAM:
        sta     tmp1            ; value
        jsr     popa            ; A = addr
        tax
        lda     tmp1
        jmp     RtcWriteNVRAM
