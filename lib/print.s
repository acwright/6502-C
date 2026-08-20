; =============================================================================
;   print.s — General output utilities
; =============================================================================

        .setcpu "65C02"

        .export _PrintStr, _PrintCRLF, _PrintDecU16

        .include "zeropage.inc"
        .include "6502.inc"

.code

; void __fastcall__ PrintStr(const char *str);
;   cc65 hands the pointer over in A/X; PrintStr wants it in A/Y.
_PrintStr:
        stx     tmp1
        ldy     tmp1
        jmp     PrintStr

; void PrintCRLF(void);
_PrintCRLF:
        jmp     PrintCRLF

; void __fastcall__ PrintDecU16(unsigned int value);
;   A/X is already what the routine wants.
_PrintDecU16:
        jmp     PrintDecU16
