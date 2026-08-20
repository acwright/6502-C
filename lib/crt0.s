; =============================================================================
;   crt0.s — C runtime startup for the AC6502
; =============================================================================
;
;   Loaded at $0800 and started from BASIC with SYS 2060 ($080C).
;
;   The system is already fully initialized by the time a C program runs:
;   hardware probed, interrupts enabled, keyboard active, IO_MODE set and
;   every Kernal routine available.  So there is no KernalInit here — this
;   file only has to build the environment C expects on top of that:
;
;     1. Remember the CPU stack pointer so we can hand control back to BASIC
;     2. Point the cc65 parameter stack (c_sp) at the top of usable RAM
;     3. Zero the BSS segment
;     4. Run the constructors (initlib)
;     5. Call main()
;     6. Run the destructors (donelib) and RTS back to BASIC
;
;   Returning from main() and calling exit() both land on the same path, so
;   a program can leave either way.
;
; =============================================================================

        .setcpu "65C02"

        .export         _exit
        .export         __STARTUP__ : absolute = 1

        .import         _main
        .import         zerobss, initlib, donelib
        .import         __STACKSTART__

        ; Keep stdio's console path in the link.  cl65 always appends the
        ; target library last, so by the time none.lib's fwrite asks for
        ; _write, 6502.lib has already been scanned and the linker will not
        ; go back for it.  crt0 is always linked, so pulling _write in from
        ; here is what guarantees printf() and friends resolve.  It has to be
        ; .forceimport rather than .import, because ca65 discards an import
        ; nothing in the module actually references.
        ;
        ; The tax is 252 bytes — write() plus the character I/O module it
        ; calls — in a program that never touches stdio.  Drop this line if
        ; you need them back and are willing to link lib/write.c by hand.
        .forceimport    _write

        .include        "zeropage.inc"

; =============================================================================
;   BASIC Startup Stub
; =============================================================================
;   A tokenized BASIC line: 10 SYS 2060
;   Identical to the stub an assembly .prg carries, and for the same reason:
;   it must be the first thing in the file so that it lands at $0800 and the
;   code that follows lands at $080C (decimal 2060).
;
;   STARTUP is the first segment in 6502.cfg, and this is the first fragment
;   of STARTUP, which is what pins it there.

        .segment "STARTUP"

BasicStartup:
        .byte   $0A, $08, $0A, $00, $A5, $32, $30, $36, $30, $00, $00, $00

; =============================================================================
;   Start — Program entry point ($080C)
; =============================================================================

Start:
        ; Remember where BASIC left the CPU stack.  _exit restores it rather
        ; than balancing pushes and pops, so a program that exits from deep
        ; inside a call chain still returns cleanly.
        tsx
        stx     SPSave

        ; Point the cc65 parameter stack at the top of usable RAM.  It grows
        ; downwards, so the first byte it touches is __STACKSTART__ - 1.
        lda     #<__STACKSTART__
        sta     c_sp
        lda     #>__STACKSTART__
        sta     c_sp+1

        ; Clear BSS.  Note this happens *after* SPSave is written — SPSave
        ; lives in DATA for exactly that reason.
        jsr     zerobss

        ; Run the constructors, then main().
        jsr     initlib
        jsr     _main

        ; main() returned: fall through with its return value still in A/X.

; =============================================================================
;   _exit — the single way out
; =============================================================================
;   In: A/X = exit code (ignored; BASIC has nowhere to put it)

_exit:
        jsr     donelib
        ldx     SPSave
        txs
        rts                     ; Return to BASIC

; =============================================================================
;   Data
; =============================================================================
;   In DATA, not BSS: zerobss would wipe it between the tsx above and the
;   ldx below.

        .segment "DATA"

SPSave: .byte   $00
