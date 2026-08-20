; =============================================================================
;   carryerr.s — Carry flag to C return value
; =============================================================================
;   Kernal routines report failure in the carry (C=0 success, C=1 error).
;   C has no carry, so every wrapper for such a routine tail-calls this to
;   turn the flag into the `unsigned char` the prototype promises.
; =============================================================================

        .setcpu "65C02"

        .export carryerr

.code

; In:  C = 0 success, C = 1 error
; Out: A/X = 0 or 1

carryerr:
        lda     #$00
        tax                     ; high byte of the return value
        rol     a               ; carry into bit 0
        rts
