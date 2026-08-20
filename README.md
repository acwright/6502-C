6502-C
======

C code for the [A.C. Wright 6502](https://github.com/acwright/6502-ACE) family of computer systems, built with [cc65](https://cc65.github.io/).
> 📖 **Guide:** [AC6502 Documentation](https://acwright.github.io/6502-DOCS/) — the user's and programmer's guide for the whole family.
> The assembly counterpart of this repository is [6502-ASM](https://github.com/acwright/6502-ASM).

A C program here compiles to exactly the same thing an assembly program does:
a `.prg` that loads at `$0800`, carries a BASIC stub so `RUN` starts it, and
returns to BASIC when it finishes. Nothing about the machine changes because
the source is C — the Kernal is still the Kernal, and `6502.h` is how you
call it.

The same is true one level down: a C program can also be a cartridge ROM that
the machine runs from reset, with no BASIC and no loader underneath it. That
is a different linker config and a different startup module, not a different
language — see [Cartridges](#cartridges).

## Building Programs

Each program directory contains its own Makefile. To build a program, navigate to its directory and use `make`.

### Prerequisites

#### CC65 Compiler

On macOS, install via Homebrew:
```bash
brew install cc65
```

For other platforms or installation methods, refer to the [cc65 project](https://github.com/cc65/cc65).

#### bin2woz

Install from NPM (recommended):
```bash
npm install -g bin2woz
```

For more information, see the [bin2woz project](https://github.com/acwright/bin2woz).

#### cffs

Install from NPM:
```bash
npm install -g cffs-image-tool
```

The `cffs` tool is used to create CompactFlash disk images and add files to them. It's required for the `make cf` target.

For more information, see the [cffs project](https://github.com/acwright/cffs).

#### 6502 CLI

Installed via the [6502-EMULATOR](https://github.com/acwright/6502-EMULATOR) app's Settings → Command Line → Install. Required for the `make run` target.

### Available Targets

- `make` or `make all` - Build the program
- `make view` - Display hexdump of the built program
- `make size` - Print the segment sizes from the link map
- `make woz` - Create a Wozmon compatible file using [bin2woz](https://github.com/acwright/bin2woz)
- `make cf` - Create a CompactFlash disk image containing the program
- `make run` - Launch the emulator app with the built program loaded
- `make clean` - Remove build artifacts

Cartridge projects build a `.crt` instead of a `.prg`, so they have no `woz`
or `cf` target and add one of their own:

- `make eeprom` - Burn the image to a 28C256 with [minipro](https://gitlab.com/DavidGriffith/minipro)

At the top level, `make` builds `6502.lib` first and then every program
directory, and `make check` runs the header checks in `tests/` on their own.

### Example

```bash
cd <directory-name>
make        # Build the program
make size   # See where the bytes went
make run    # Launch the emulator
```

## How a C program is put together

Six pieces, five of them shared at the top level:

| File | What it is |
|---|---|
| `6502.h` | The Kernal, the IO registers and the constants, for C |
| `6502.inc` | The same thing for ca65 — kept identical to the copy in [6502-ASM](https://github.com/acwright/6502-ASM) |
| `6502.cfg` | The linker config: where the program, the C stack and the heap go |
| `6502-16K.cfg` | The same, for a cartridge ROM at `$C000` |
| `lib/` | The sources of `6502.lib` — startup, Kernal wrappers, `write()` |
| `<Program>/` | Your `.c`, its Makefile, its README |

### 6502.h

The C counterpart of `6502.inc`, section for section, name for name. Hardware
registers and system variables are volatile lvalues; every Kernal jump table
entry has a `__fastcall__` wrapper of the same name, which is a JSR and a
register shuffle:

```c
#include "6502.h"

PrintStr("READY.");
VideoSetColor(TMS_LT_GREEN << 4 | TMS_BLACK);
if (HW_PRESENT & HW_SID) { SidPlayNote(0, SID_NOTE_A4); }
```

The IO registers also come as one struct per card, overlaid on that card's base
address, for the cases where a flat macro name cannot say what you mean:

```c
SID.voice[i].freq = SID_NOTE_A4;   /* index the voices, or write both */
GPIO.pcr          = GPIO_PCR_CB2_LO;   /* frequency bytes in one go */
if (ST.status & ST_STATUS_BSY) { ... }
```

`SID.voice[0].freq_lo` is the same address as `SID_V1_FREQ_LO` and compiles to
the same absolute store — the structs are a second view of Section 5, not a
replacement, and the two mix freely. A *variable* index is the one thing that
costs extra, since `SID.voice[i]` has to build a pointer.

Routines that report failure in the carry return `unsigned char` — 0 for
success, 1 for error — so they read as `if (FsLoadFileAddr(...) != 0)`.
Routines that return several values in A/X/Y take a pointer instead
(`RtcReadTime`) or pack the pair into an `unsigned int` (`VideoGetCursor`).

`XModemLoad`/`XModemSave` and `StReadSector`/`StWriteSector` have no wrappers:
their arguments are zero page pointers the routine advances in place, which is
not something a C prototype describes well. Drive them through the `XFER_PTR`,
`CF_BUF_PTR` and `CF_LBA` macros, or from assembly.

### 6502.cfg

```
$003A-$00FF   Zero page — the cc65 runtime takes the first 26 bytes
$0800-$77FF   Program: startup, code, rodata, data, bss, then the heap
$7800-$7FFF   C parameter stack, growing down from $8000
```

The zero page window is the range the BIOS documents as free for user
programs. Everything below `$3A` belongs to BASIC, the Kernal or the Monitor,
so a stock cc65 config — which helps itself to `$0080` upwards — would quietly
corrupt the machine underneath you. That is the one thing you cannot get away
without changing.

The C stack here is cc65's parameter stack, not the CPU stack; the CPU stack
stays at `$0100-$01FF` where the hardware puts it. `malloc` takes whatever is
left between the end of BSS and the bottom of the parameter stack, so
shrinking `__STACKSIZE__` hands that space to the heap and vice versa.

### lib/

`6502.lib` is built from `lib/` and holds three things:

**`crt0.s`** — the startup. It emits the same 12-byte tokenized `10 SYS 2060`
stub an assembly `.prg` carries, points `c_sp` at the top of RAM, clears BSS,
runs the constructors and calls `main()`. There is no `KernalInit` in it: a
`.prg` runs on a machine that is already up, with hardware probed, interrupts
enabled and `IO_MODE` set. Returning from `main()` and calling `exit()` both
restore the CPU stack pointer saved on entry and `RTS` back to BASIC.

**`crt0cart.s`** — the same job for a cartridge, which starts from reset with
none of that done. See [Cartridges](#cartridges). It is built by `lib/` but
deliberately *not* archived into `6502.lib`: it exports the same symbols as
`crt0.s`, and the linker cannot be asked to choose between two modules that
both satisfy `__STARTUP__`. Cartridge Makefiles name `../lib/crt0cart.o` on
the command line, which resolves the import before the library is scanned.

**One module per hardware area** — `chario.s`, `video.s`, `sound.s`, `fs.s`,
`gpio.s`, `serial.s`, `rtc.s`, `system.s`, `print.s`. Split up rather than
combined so a program that only prints a string does not also link the RTC and
the filesystem.

**The parts of cc65's own library that need a machine underneath them.** Each
is the small piece cc65 leaves to the target, and supplying it brings a whole
standard header to life:

| Module | Gives you |
|---|---|
| `write.c` | `<stdio.h>` — everything written goes to `Chrout`, and `'\n'` becomes CR+LF on the way out, so `printf("...\n")` lands where you expect |
| `conio.s` | `<conio.h>` — `clrscr`, `gotoxy`, `cputs`, `cprintf`, `cgetc`, `kbhit`, `cputhex8/16`, `chline`, `cvline`. Output follows `IO_MODE`, so it works on the serial console too |
| `gettime.c` | `<time.h>` — `time`, `localtime`, `gmtime`, `mktime`, `strftime`, `ctime`, backed by the DS1511Y |
| `settime.c` | `clock_settime`, to put a `time_t` back into the clock |

The clock is the one the C64 cannot match: a stock C64 fakes the time from the
jiffy counter and loses it at every reset, while the DS1511Y is battery-backed
and keeps it across a power cycle. `mktime` finishes by subtracting
`_tz.timezone`, and `_tz` defaults to all zeroes, so the RTC is read as UTC
unless a program says otherwise.

`clock()` is deliberately absent. It needs a free-running tick, and this
machine has no counter for one — nor is `CLOCKS_PER_SEC` even defined for
target `none`.

Three conio calls do not fit a TMS9918, which colours the whole screen rather
than each cell: `textcolor()` and `bgcolor()` recolour everything,
`bordercolor()` is the same register as `bgcolor()`, and `revers()` records
the flag but has no attribute to drive. And while conio's *output* follows
`IO_MODE` onto the serial console, its *cursor* is the BIOS's video cursor —
with no video card fitted the text still appears but `wherex()`/`wherey()`
always read 0,0. The comment at the top of `lib/conio.s` spells all of this
out before you rely on any of it.

`video.s` also carries **`waitvsync()`**, which waits for the VDP to finish a
frame so an update lands in the blanking interval. The BIOS never reads the
VDP status register and leaves VDP interrupts off, so nothing competes for the
frame flag — but a program that installs its own handler through `IRQ_PTR`
could, so the wait gives up after about five frames rather than hanging.

### tests/

Two checks, run by `make check` and by a plain `make`:

- **`regcheck.c`** asserts at compile time that every field of every Section 5b
  IO block lands on the same address as its Section 5 macro, and that each
  block is exactly as wide as the card's window. Nothing is linked or run — a
  wrong offset is a build error instead of a program that writes to the wrong
  register.
- **`parity.py`** compares every address `6502.h` and `6502.inc` both name, so
  the two cannot drift apart unnoticed.

## Cartridges

A cartridge overlays `$C000-$FFFF`, replacing BASIC, the Monitor, Wozmon and
the CPU vectors, and the machine runs it from reset. The Kernal
(`$A000-$B7FF`) and character set (`$B800-$BFFF`) stay put underneath, so the
whole jump table — and everything in `6502.h` that wraps it — is still yours
to call.

[HelloWorldCart](HelloWorldCart) is the worked example, and the interesting
thing about its `main()` is that it looks exactly like the `.prg` version's.
The difference is entirely in the two pieces underneath it.

### 6502-16K.cfg

```
$003A-$00FF   Zero page — the cc65 runtime takes the first 26 bytes
$0800-$77FF   RAM: data, bss, then the heap
$7800-$7FFF   C parameter stack, growing down from $8000
$C000-$FFF9   Cartridge: startup, code, rodata, and the DATA load image
$FFFA-$FFFF   CPU vectors — NMI, RESET, IRQ
```

The split is the thing to understand. Code and constants live in ROM and stay
there; initialized variables cannot, because a program writes to them. So
`DATA` is given a **load** address in the cartridge and a **run** address in
RAM, and the startup copies it across before `main()` — which is what `DATA`
and `BSS` carrying `define = yes` is for, since that publishes the
`__DATA_LOAD__`/`__DATA_RUN__`/`__DATA_SIZE__` and `__BSS_*__` symbols
`copydata`, `zerobss` and `malloc` need. An assembly cartridge has nothing to
copy and no config line for it; this is the one place the C version is
genuinely doing more work.

16K is the code space you budget against, but the **file is 32K**: it spans
`$8000-$FFFF` so it can be burned straight to a 28C256 or 27C256, and the low
half is padding the machine never reads as cartridge ROM.

### crt0cart.s

Everything `crt0.s` gets for free, done by hand, because at reset nothing has
run yet:

| | `crt0.s` (`.prg`) | `crt0cart.s` (`.crt`) |
|---|---|---|
| Reached by | BASIC `RUN`, via a `10 SYS 2060` stub | the RESET vector at `$FFFC` |
| CPU stack | BASIC set it; saved for the return | `LDX #$FF / TXS` — nothing else did |
| Hardware | already probed and running | `KernalInit`, then `CLI` |
| `DATA` | put in RAM by the loader | `copydata`, out of ROM |
| Leaving `main()` | restore SP and `RTS` to BASIC | destructors, then halt |

It also owns the CPU vectors at `$FFFA` and points IRQ and NMI at trampolines
that `JMP (IRQ_PTR)` / `JMP (NMI_PTR)`. `KernalInit` has already pointed those
RAM vectors at the default handlers, so keyboard and serial input keep working
without a line of handler code, and a program that wants its own can still
write `IRQ_PTR` from C.

## What it costs

`printf` is the whole formatter; on a 30 KB machine that is worth knowing
about before you put one in a loop:

| Program | `.prg` size |
|---|---|
| `PrintStr` only | 434 bytes |
| The same, plus one `printf` | 2,511 bytes |
| `clrscr` + `cputsxy` + `cputc` | 716 bytes |
| One `cprintf` | 2,189 bytes |
| One `time()` | 2,285 bytes |

`PrintStr` and `Chrout` stay cheap because they are the Kernal's own code,
already in ROM. Reach for `printf` when formatting is what you actually want,
and for `PrintStr`/`PrintDecU16` when it isn't.

`cprintf` is the same formatter as `printf` without the stdio layer under it,
which is worth about 350 bytes and no more — the formatter is the expensive
part either way. Plain conio, with no formatting, is cheap. `time()` costs what
it does because `mktime` is real 32-bit arithmetic; reading `RtcReadTime()`
straight is a fraction of that when a `time_t` is not what you actually need.

252 of that first figure is `write()` and the character I/O module behind it,
which `crt0.s` pulls in unconditionally so that stdio always resolves — see
the comment there for why, and for how to get the bytes back.

## Related

- [6502-ACE](https://github.com/acwright/6502-ACE) — the hardware, and the index of the whole family
- [6502-ASM](https://github.com/acwright/6502-ASM) — the same programs in assembly
- [6502-BIOS](https://github.com/acwright/6502-BIOS) — the firmware behind `6502.h` and `6502.inc`
- [6502-EMULATOR](https://github.com/acwright/6502-EMULATOR) — run these programs without hardware
- [6502-BAS](https://github.com/acwright/6502-BAS) — the same idea for BASIC listings
- [6502-DOCS](https://github.com/acwright/6502-DOCS) — the documentation site

## License

MIT License — see [LICENSE](LICENSE).
