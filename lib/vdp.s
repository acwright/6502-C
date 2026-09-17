; =============================================================================
;   vdp.s — the 6502-PICOVDP Kernal entries (BIOS 2.x)
; =============================================================================
;   Archived only into 6502-VDP.lib.  These entries are bare RTS slots on a
;   1.x ROM, so a legacy program must not be able to link them: 6502.lib does
;   not carry this module, and 6502.h does not declare them.
;
;   Every entry reports "no card, or an argument out of range" in the carry,
;   having written nothing.  The wrappers turn that into the C return value
;   6502-VDP.h documents for each: 0/1 through carryerr, or a sentinel where
;   the routine also returns a value.
; =============================================================================

        .setcpu "65C02"

        .export _VdpInfo, _VdpWriteReg, _VdpSetMode
        .export _VdpPoke, _VdpPeek, _VdpSetPalette
        .export _WaitVBlank, _VdpLoadFile, _VdpLoadFont
        .export _VdpSprite, _VdpSetScroll, _VdpLayer, _VdpStatus

        .import popa, popax
        .import carryerr

        .include "zeropage.inc"
        .include "6502-VDP.inc"

.code

; unsigned int VdpInfo(void);
;   Returns (VDP_CAPS << 8) | VDP_FW, or 0 when there is no PICOVDP console.
;   The routine leaves A = FW and X = CAPS, which is that pair already; only
;   the no-card case needs zeroing, since a card without the font reports
;   what it has alongside the carry.
_VdpInfo:
        jsr     VdpInfo
        bcc     @done
        lda     #$00
        tax
@done:  rts

; unsigned char __fastcall__ VdpWriteReg(unsigned char reg, unsigned char value);
;   value arrives in A, reg on the C stack.  popa clobbers Y, not X, but it
;   returns in A, so value is parked in tmp1.
_VdpWriteReg:
        sta     tmp1            ; value
        jsr     popa            ; A = reg
        tax
        lda     tmp1
        jsr     VdpWriteReg
        jmp     carryerr

; unsigned char __fastcall__ VdpSetMode(unsigned char mode);
_VdpSetMode:
        jsr     VdpSetMode
        jmp     carryerr

; unsigned char __fastcall__ VdpPoke(unsigned int addr, unsigned char value);
_VdpPoke:
        sta     tmp1            ; value
        jsr     popax           ; A/X = addr
        stx     tmp2
        tax                     ; X = addr low
        ldy     tmp2            ; Y = addr high
        lda     tmp1
        jsr     VdpPoke
        jmp     carryerr

; unsigned char __fastcall__ VdpPeek(unsigned int addr);
;   Returns the byte, or 0 with no card.
_VdpPeek:
        stx     tmp1
        tax                     ; X = addr low
        ldy     tmp1            ; Y = addr high
        jsr     VdpPeek
        bcc     @done
        lda     #$00
@done:  ldx     #$00
        rts

; unsigned char __fastcall__ VdpSetPalette(unsigned char entry, unsigned int rgb);
;   rgb is 0x0RGB: its high byte is the routine's A ($0R), its low byte Y ($GB).
_VdpSetPalette:
        sta     tmp1            ; $GB
        stx     tmp2            ; $0R
        jsr     popa            ; A = entry
        tax
        lda     tmp2
        ldy     tmp1
        jsr     VdpSetPalette
        jmp     carryerr

; unsigned char WaitVBlank(void);
_WaitVBlank:
        jsr     WaitVBlank
        jmp     carryerr

; unsigned char __fastcall__ VdpLoadFile(const char *name, unsigned int vaddr);
_VdpLoadFile:
        sta     FS_IO_ADDR
        stx     FS_IO_ADDR+1
        jsr     popax           ; A/X = name
        sta     STR_PTR
        stx     STR_PTR+1
        jsr     VdpLoadFile
        jmp     carryerr

; unsigned char __fastcall__ VdpLoadFont(unsigned char font);
_VdpLoadFont:
        jsr     VdpLoadFont
        jmp     carryerr

; unsigned char __fastcall__ VdpSprite(unsigned char sprite, const VdpSpriteAttr *attr);
;   The four bytes of *attr are VDP_P0-VDP_P3, in that order.
_VdpSprite:
        sta     ptr1
        stx     ptr1+1
        ldy     #3
@copy:  lda     (ptr1),y
        sta     VDP_P0,y
        dey
        bpl     @copy
        jsr     popa            ; A = sprite
        tax
        jsr     VdpSprite
        jmp     carryerr

; unsigned char __fastcall__ VdpSetScroll(unsigned char layer, unsigned int x,
;                                         unsigned char y);
;   y arrives in A; x (0-511) and then layer are on the C stack.
_VdpSetScroll:
        sta     tmp1            ; y
        jsr     popax           ; A/X = x
        sta     tmp2            ; x bits 7:0
        txa
        and     #$01
        sta     VDP_P0          ; x bit 8
        jsr     popa            ; A = layer
        tax
        lda     tmp2
        ldy     tmp1
        jsr     VdpSetScroll
        jmp     carryerr

; unsigned char __fastcall__ VdpLayer(unsigned char layer, unsigned char on);
_VdpLayer:
        sta     tmp1            ; on
        jsr     popa            ; A = layer
        tax
        lda     tmp1
        jsr     VdpLayer
        jmp     carryerr

; int __fastcall__ VdpStatus(unsigned char reg);
;   Returns the status register's value, or -1 with no card or reg > 15.
_VdpStatus:
        tax
        jsr     VdpStatus
        bcs     @none
        ldx     #$00
        rts
@none:  lda     #$FF
        tax
        rts
