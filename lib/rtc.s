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
        .export _NvStat, _NvRead, _NvWrite, _NvErase, _NvFind, _NvFormat

        .import carryerr
        .import popa

        .include "zeropage.inc"
        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

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

; =============================================================================
;   NVRAM save slots (BIOS v1.6)
; =============================================================================
;   NvStat and NvRead return the status in A and the owner ID in Y; the
;   wrappers pack them as A = status, X = owner.  NV_ERROR ($FF) stands in for
;   the cases where the Kernal leaves A and Y undefined.

NV_ERROR        = $FF           ; 6502.h: no RTC, or slot not 0-15
NV_NONE         = $FF           ; 6502.h: NvFind matched nothing

; unsigned int __fastcall__ NvStat(unsigned char slot);
;   Carry from NvStat means only no RTC or a bad slot.
_NvStat:
        tax
        jsr     NvStat
        bcs     NvError
NvPack:
        pha
        tya
        tax                     ; X = owner
        pla                     ; A = status
        rts

; unsigned int __fastcall__ NvRead(unsigned char slot, void *buf);
;   buf arrives in A/X, slot is on the C stack.  Carry from NvRead also means
;   "not NV_VALID", when A and Y still hold the answer, so the two cases that
;   leave them undefined are ruled out here first.
_NvRead:
        sta     ptr1
        stx     ptr1+1
        jsr     popa            ; A = slot
        tax
        lda     HW_PRESENT
        and     #HW_RTC
        beq     NvError
        cpx     #NV_SLOTS
        bcs     NvError
        lda     ptr1
        ldy     ptr1+1
        jsr     NvRead
        jmp     NvPack

NvError:
        lda     #NV_ERROR
        tax
        rts

; unsigned char __fastcall__ NvWrite(unsigned char slot, unsigned char owner,
;                                    const void *buf);
;   buf arrives in A/X; owner then slot are on the C stack.
_NvWrite:
        sta     ptr1
        stx     ptr1+1
        jsr     popa            ; A = owner
        sta     NV_ID
        jsr     popa            ; A = slot
        tax
        lda     ptr1
        ldy     ptr1+1
        jsr     NvWrite
        jmp     carryerr

; unsigned char __fastcall__ NvErase(unsigned char slot);
_NvErase:
        tax
        jsr     NvErase
        jmp     carryerr

; unsigned char __fastcall__ NvFind(unsigned char owner);
_NvFind:
        jsr     NvFind          ; X = slot
        bcs     @None
        txa
        ldx     #$00
        rts
@None:
        lda     #NV_NONE
        ldx     #$00
        rts

; unsigned char NvFormat(void);
_NvFormat:
        jsr     NvFormat
        jmp     carryerr
