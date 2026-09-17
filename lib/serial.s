; =============================================================================
;   serial.s — Serial card (R65C51 ACIA)
; =============================================================================

        .setcpu "65C02"

        .export _InitSC, _SerialChrout

        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

.code

; void InitSC(void);
_InitSC:
        jmp     InitSC

; void __fastcall__ SerialChrout(char c);
_SerialChrout:
        jmp     SerialChrout
