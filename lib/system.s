; =============================================================================
;   system.s — Timing and version
; =============================================================================

        .setcpu "65C02"

        .export _SysDelay, _KernalVersion

        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

.code

; void __fastcall__ SysDelay(unsigned int centiseconds);
;   A/X is already the 16-bit count the routine wants.
_SysDelay:
        jmp     SysDelay

; unsigned int KernalVersion(void);
;   Returns A = major in the low byte, X = minor in the high byte, which is
;   exactly how the routine leaves them.
_KernalVersion:
        jmp     KernalVersion
