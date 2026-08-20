; =============================================================================
;   gpio.s — Keyboard and joysticks (65C22 VIA)
; =============================================================================

        .setcpu "65C02"

        .export _InitKB, _ReadJoystick1, _ReadJoystick2
        .export _KBDisable, _KBEnable

        .include "6502.inc"

.code

; void InitKB(void);
_InitKB:
        jmp     InitKB

; unsigned char ReadJoystick1(void);
_ReadJoystick1:
        jsr     ReadJoystick1
        ldx     #$00
        rts

; unsigned char ReadJoystick2(void);
_ReadJoystick2:
        jsr     ReadJoystick2
        ldx     #$00
        rts

; void KBDisable(void);
_KBDisable:
        jmp     KBDisable

; void KBEnable(void);
_KBEnable:
        jmp     KBEnable
