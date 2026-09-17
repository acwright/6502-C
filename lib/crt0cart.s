; =============================================================================
;   crt0cart.s — C runtime startup for an AC6502 cartridge
; =============================================================================
;
;   The cartridge counterpart of crt0.s.  Same job — build the environment C
;   expects and call main() — but from a completely different starting point,
;   and the difference is the whole reason this file exists:
;
;     crt0.s      .prg at $0800, reached from BASIC by RUN.  The machine is
;                 already up: hardware probed, interrupts on, IO_MODE set.
;                 Returning from main() goes back to BASIC.
;
;     crt0cart.s  ROM at $C000, reached by the CPU's RESET vector.  Nothing
;                 has run yet — not the stack pointer, not KernalInit — and
;                 there is nothing underneath to return to.
;
;   So this file does everything crt0.s can take for granted:
;
;     1. Reset the CPU stack pointer, because nothing else did
;     2. KernalInit — probe and initialize every card (leaves IRQs off)
;     3. Interrupts on, now that the vectors are live
;     4. Point the cc65 parameter stack (c_sp) at the top of usable RAM
;     5. Copy DATA from the cartridge into RAM, and zero BSS
;     6. Run the constructors (initlib), then call main()
;     7. Run the destructors and halt — a cartridge has nowhere to go
;
;   Step 5 is the one with no counterpart in crt0.s.  In a .prg, initialized
;   variables are already sitting in RAM because the loader put them there.
;   In a cartridge they are in ROM, where they cannot be written to, so the
;   linker gives DATA a load address in the cart and a run address in RAM and
;   copydata moves it across.  See 6502-16K.cfg.
;
;   This module is deliberately NOT in 6502.lib: it exports the same symbols
;   as crt0.s, and the linker cannot be asked to choose.  Cartridge Makefiles
;   name ../lib/crt0cart.o on the command line instead, which satisfies the
;   __STARTUP__ import before 6502.lib is ever scanned for it.
;
; =============================================================================

        .setcpu "65C02"

        .export         _exit
        .export         __STARTUP__ : absolute = 1

        .import         _main
        .import         zerobss, copydata, initlib, donelib
        .import         __STACKSTART__

        ; Same reasoning as crt0.s: cl65 appends the target library last, so
        ; none.lib's fwrite asks for _write after 6502.lib has already been
        ; scanned.  Forcing the import from the startup module — which is
        ; always linked — is what makes printf() and friends resolve.
        .forceimport    _write

        .include        "zeropage.inc"
        .ifdef VDP
        .include        "6502-VDP.inc"
        .else
        .include        "6502.inc"
        .endif

; =============================================================================
;   CartReset — the RESET vector points here ($C000)
; =============================================================================

        .segment "STARTUP"

CartReset:
        ; Nothing has set the stack pointer for us, and KernalInit is a JSR.
        ldx     #$ff
        txs

        ; Probe and initialize every card.  Returns with interrupts still
        ; disabled so a cartridge can install its own handlers first.
        jsr     KernalInit

        ; Interrupts on: the RAM vectors are live and the trampolines below
        ; are already pointed at them.  Keyboard and serial input work from
        ; here on.
        cli

        ; Point the cc65 parameter stack at the top of usable RAM.  It grows
        ; downwards, so the first byte it touches is __STACKSTART__ - 1.
        lda     #<__STACKSTART__
        sta     c_sp
        lda     #>__STACKSTART__
        sta     c_sp+1

        ; Move the initialized variables out of ROM and into RAM, then clear
        ; the uninitialized ones.  A .prg gets both for free from its loader.
        jsr     copydata
        jsr     zerobss

        ; Run the constructors, then main().
        jsr     initlib
        jsr     _main

        ; main() returned: fall through with its return value still in A/X.

; =============================================================================
;   _exit — the single way out, which on a cartridge is not a way out
; =============================================================================
;   In: A/X = exit code (ignored; there is nobody to report it to)
;
;   crt0.s restores the CPU stack pointer BASIC left and RTSes.  Here there is
;   no BASIC and no return address, so the destructors run and the machine
;   sits in a loop with interrupts still enabled — RESET is the way on.

_exit:
        jsr     donelib

@Halt:
        bra     @Halt

; =============================================================================
;   Interrupt trampolines
; =============================================================================
;   The cartridge owns the hardware vectors at $FFFA, but KernalInit has
;   already pointed the RAM vectors at the default handlers.  Bouncing through
;   them keeps keyboard and serial input working without writing a handler,
;   and leaves IRQ_PTR / NMI_PTR free for a program that wants its own.

IrqTrampoline:
        jmp     (IRQ_PTR)               ; Dispatch through the RAM IRQ vector

NmiTrampoline:
        jmp     (NMI_PTR)               ; Dispatch through the RAM NMI vector

; =============================================================================
;   CPU vectors — the cartridge owns $FFFA-$FFFF
; =============================================================================

        .segment "VECTORS"

        .word   NmiTrampoline           ; NMI
        .word   CartReset               ; RESET — the cartridge entry point
        .word   IrqTrampoline           ; IRQ
