; =============================================================================
;   chario.s — Character I/O and console selection
; =============================================================================

        .setcpu "65C02"

        .export _Chrout, _Chrin, _ChrinWait
        .export _WriteBuffer, _ReadBuffer, _BufferSize
        .export _SetIOMode, _GetIOMode

        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

.code

; void __fastcall__ Chrout(char c);
_Chrout:
        jmp     Chrout

; int Chrin(void);
;   Returns the character, or -1 when the buffer is empty.
_Chrin:
        jsr     Chrin
        bcc     @Empty
        ldx     #$00
        rts
@Empty:
        lda     #$FF
        tax                     ; A/X = $FFFF = -1
        rts

; char ChrinWait(void);
;   Chrin is a poll, not a read — spin here until it hands something back.
_ChrinWait:
        jsr     Chrin
        bcc     _ChrinWait
        ldx     #$00
        rts

; void __fastcall__ WriteBuffer(char c);
_WriteBuffer:
        jmp     WriteBuffer

; char ReadBuffer(void);
_ReadBuffer:
        jsr     ReadBuffer
        ldx     #$00
        rts

; unsigned char BufferSize(void);
_BufferSize:
        jsr     BufferSize
        ldx     #$00
        rts

; void __fastcall__ SetIOMode(unsigned char mode);
_SetIOMode:
        jmp     SetIOMode

; unsigned char GetIOMode(void);
_GetIOMode:
        jsr     GetIOMode
        ldx     #$00
        rts
