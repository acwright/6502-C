Hello World Cart
================

The same greeting as [HelloWorld](../HelloWorld), built as a cartridge ROM
instead of a `.prg`.

The assembly version of this program lives in
[6502-ASM](https://github.com/acwright/6502-ASM); this is the same cartridge
written for cc65, and the same 32 KB image comes out the other end.

A cartridge overlays `$C000-$FFFF`, replacing BASIC, the Monitor, Wozmon and
the CPU vectors, and the machine runs it from reset. There is no loader and
nothing underneath to return to:

| | Where it lives | What starts it | Where it ends |
|---|---|---|---|
| `HelloWorld` | RAM at `$0800` | BASIC `RUN` | back in BASIC |
| `HelloWorldCart` | ROM at `$C000` | the RESET vector | a halt loop |

`HelloWorldCart.c` is the interesting part precisely because it is dull —
`main()` is `main()`, and `6502.h` is unchanged, because the Kernal
(`$A000-$B7FF`) and character set (`$B800-$BFFF`) are still there underneath
the cartridge window. What changes is everything around it:

**[`6502-16K.cfg`](../6502-16K.cfg)** splits the program in two. Code and
constants go in ROM and stay there; initialized variables cannot, because a
program writes to them, so `DATA` is *loaded* into the cartridge at `$CA09`
and *run* from RAM at `$0800`. The file it produces is 32 KB spanning
`$8000-$FFFF` — the low half is padding, so the image can be burned straight
to a 28C256.

**[`lib/crt0cart.s`](../lib/crt0cart.s)** does everything the `.prg` startup
gets for free. It resets the stack pointer, calls `KernalInit` to probe and
initialize the cards, enables interrupts, copies `DATA` into RAM, clears BSS,
runs the constructors and calls `main()`. It also owns the CPU vectors at
`$FFFA`, and bounces IRQ and NMI through `IRQ_PTR`/`NMI_PTR` so keyboard and
serial input keep working without writing a handler.

It is deliberately not in `6502.lib` — it exports the same symbols as
`crt0.s`, and the linker cannot be asked to choose — so the Makefile names
`../lib/crt0cart.o` on the command line.

Install:

    brew install cc65

Build:

    make

View:

    make view

Size:

    make size

Run:

    make run

Burn an EEPROM (28C256, via [minipro](https://gitlab.com/DavidGriffith/minipro)):

    make eeprom
