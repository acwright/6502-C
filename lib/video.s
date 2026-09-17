; =============================================================================
;   video.s — the video console (TMS9918 on BIOS 1.x, 6502-PICOVDP on 2.x)
; =============================================================================

        .setcpu "65C02"

        .export _InitVideo, _VideoClear, _VideoPutChar
        .export _VideoSetCursor, _VideoGetCursor, _VideoScroll
        .export _VideoSetColor, _VideoChroutRaw
        .export _waitvsync

        .import popa

        .include "zeropage.inc"
        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

.code

; void InitVideo(void);
_InitVideo:
        jmp     InitVideo

; void VideoClear(void);
_VideoClear:
        jmp     VideoClear

; void __fastcall__ VideoPutChar(char c);
_VideoPutChar:
        jmp     VideoPutChar

; void __fastcall__ VideoSetCursor(unsigned char col, unsigned char row);
;   row arrives in A, col is on the C stack.  popa clobbers Y, so row is
;   parked in tmp1 rather than moved into Y first.
_VideoSetCursor:
        sta     tmp1            ; row
        jsr     popa            ; A = col
        tax
        ldy     tmp1
        jmp     VideoSetCursor

; unsigned int VideoGetCursor(void);
;   Returns (row << 8) | col.
_VideoGetCursor:
        jsr     VideoGetCursor  ; X = col, Y = row
        txa                     ; A = col (low byte)
        sty     tmp1
        ldx     tmp1            ; X = row (high byte)
        rts

; void VideoScroll(void);
_VideoScroll:
        jmp     VideoScroll

; void __fastcall__ VideoSetColor(unsigned char color);
_VideoSetColor:
        jmp     VideoSetColor

; void __fastcall__ VideoChroutRaw(char c);
_VideoChroutRaw:
        jmp     VideoChroutRaw

; void waitvsync(void);
;   Block until the VDP finishes a frame, so a screen update lands in the
;   blanking interval instead of halfway down the picture.

.ifdef VDP

;   BIOS 2.x has WaitVBlank, which polls STAT3 rather than STAT0, so it clears
;   no flag anything else may be waiting for and needs no timeout.  With no
;   PICOVDP it waits 2 cs instead, so a loop paced by it keeps its speed.

_waitvsync:
        jmp     WaitVBlank

.else

;   Bit 7 of the TMS9918 status register is set at the end of the active
;   display and cleared by the act of reading it.  One read clears whatever
;   was standing from an earlier frame; the loop then waits for the next.
;
;   The wait is bounded rather than infinite.  Reading the status register is
;   destructive, so anything else that reads it — a BIOS interrupt handler
;   servicing the VDP, if this machine wires the VDP's INT line — consumes the
;   flag this loop is waiting for and could hang it forever.  Giving up after
;   roughly five frames turns that from a lock-up into a dropped sync.  With
;   no video card fitted the status port is a floating bus, so HW_VID is
;   checked before believing anything it says.

_waitvsync:
        lda     HW_PRESENT
        and     #HW_VID
        beq     @done                   ; no video card — nothing to sync to

        lda     VC_STATUS               ; discard a flag left from before
        lda     #$00
        sta     tmp1
        lda     #$20                    ; $2000 polls ~= 5 frames at 1 MHz
        sta     tmp2

@wait:  lda     VC_STATUS
        bmi     @done                   ; bit 7 — frame complete
        dec     tmp1
        bne     @wait
        dec     tmp2
        bne     @wait

@done:  rts

.endif
