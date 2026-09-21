Hello World Cart
================

The same greeting as [HelloWorld](../HelloWorld), built as a cartridge ROM
instead of a `.prg`.

The assembly version of this program lives in
[6502-ASM](https://github.com/acwright/6502-ASM); this is the same cartridge
written for cc65, and the same 32 KB image comes out the other end.

A cartridge overlays `$C000-$FFFF`, replacing BASIC, Wozmon, the CPU vectors
and, on BIOS 1.x, the Monitor, and the machine runs it from reset. There is no loader and
nothing underneath to return to:

| | Where it lives | What starts it | Where it ends |
|---|---|---|---|
| `HelloWorld` | RAM at `$0800` | BASIC `RUN` | back in BASIC |
| `HelloWorldCart` | ROM at `$C000` | the RESET vector | a halt loop |

`HelloWorldCart.c` is the interesting part precisely because it is dull —
`main()` is `main()`, and `6502.h` is unchanged, because the Kernal is
still there underneath the cartridge window (`$A000-$B7FF` and the character
set at `$B800-$BFFF` on BIOS 1.x, all of `$A000-$BFFF` on 2.x). What changes is everything around it:

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

Build and run for an ACE with a 6502-PICOVDP on BIOS 2.x (`HelloWorldCart-VDP.crt`):

    make VDP=1
    make VDP=1 run

Until the emulator bundles BIOS 2.0 for the PICOVDP card, add
`ROM=path/to/BIOS.bin` to boot a 2.0 image.

Burn an EEPROM (28C256, via [minipro](https://gitlab.com/DavidGriffith/minipro)):

    make eeprom

## As a Flash Cart

`make FLASH=128K|256K|512K|1M` builds the same program as a banked Flash Cart
image, with two things different. The link takes `crt0cart-flash.o` instead of
`crt0cart.o`, which clears the bank register's shadow at reset and carries
`set_bank` and `current_bank`; and the C program's budget drops from 16,378
bytes to 8,186, because a banked cart's fixed region is half the size of a
fixed cart's window. `make size` shows it.

The build also switches on a short worked example, which is the part worth
reading:

```c
#pragma rodata-name (push, "BANK01")
static const char banked_greeting[] = "...and this one lives in bank $01.";
#pragma rodata-name (pop)

set_bank(0x01);
PrintStr(banked_greeting);
```

`PrintStr` is in the Kernal at `$A090`, not in the window, so the bank stays
selected across the call. Everything after `set_bank` is an ordinary read of
ordinary memory — once a bank is selected it is not special, and that is the
whole idea.

Bank 0 holds a recognisable filler string, and `main()` prints an initialized
variable beside it. That is the cheap check on the one thing that would fail
silently: `DATA`'s load image is pinned to the fixed region, and if it were in
a bank the startup would copy bank 0's filler into every initialized variable
instead — correctly, every time, until somebody used bank 0 for something.

    make FLASH=512K
    make FLASH=512K run
    make FLASH=512K size
    make all-flash

`make run` on a banked image needs 6502-EMULATOR 3.4.0 or later; an older one
drops it without a word and boots to BASIC. `make eeprom` refuses a flash
image; `make flash FLASH=512K` burns one in circuit through the Flash Helper.

See the repository README's [Banked
cartridges](../README.md#banked-cartridges) for the rules, chiefly that C code
stays in the fixed region and only `const` data goes in a bank.
