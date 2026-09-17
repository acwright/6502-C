; =============================================================================
;   sound.s — SID / ARMSID
; =============================================================================

        .setcpu "65C02"

        .export _InitSID, _Beep, _SidPlayNote, _SidSilence, _SidSetVolume

        .import popa

        .include "zeropage.inc"
        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

.code

; void InitSID(void);
_InitSID:
        jmp     InitSID

; void Beep(void);
_Beep:
        jmp     Beep

; void __fastcall__ SidPlayNote(unsigned char voice, unsigned int freq);
;   freq arrives in A/X, voice is on the C stack; the routine wants
;   A = voice, X = freq low, Y = freq high.
_SidPlayNote:
        sta     tmp1            ; freq low
        stx     tmp2            ; freq high
        jsr     popa            ; A = voice
        ldx     tmp1
        ldy     tmp2
        jmp     SidPlayNote

; void SidSilence(void);
_SidSilence:
        jmp     SidSilence

; void __fastcall__ SidSetVolume(unsigned char volume);
_SidSetVolume:
        jmp     SidSetVolume
