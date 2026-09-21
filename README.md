6502-C
======

C code for the [AC6502](https://github.com/acwright/6502-ACE) family of computer systems, built with [cc65](https://cc65.github.io/).
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

Every program also builds two ways from the same source: `make` for any ACE on
BIOS 1.x with a TMS9918A, and `make VDP=1` for an ACE converted to a
[6502-PICOVDP](https://github.com/acwright/6502-PICOVDP) on BIOS 2.x — see
[Choosing a build](#choosing-a-build).

## What this repository is for

This is where small C programs for the AC6502 family live: demos, samples,
scratchpads, tests, and anything written to show one thing working. Each one is
a directory with its own source, its own Makefile and its own README, and the
top-level `make` builds all of them on top of the shared `6502.lib`.

Anything larger gets a repository of its own. A game or an application has
releases, issues and documentation of its own, and none of that fits in a
directory next to a twenty-line demo.

The same idea in the other two languages:

- [6502-ASM](https://github.com/acwright/6502-ASM) — assembly programs
- [6502-BAS](https://github.com/acwright/6502-BAS) — BASIC listings

There is no separate template repository for C — [HelloWorld](HelloWorld) and
[HelloWorldCart](HelloWorldCart) are the two things to copy, and they are kept
here because a C program needs the library and the headers next to it. The
assembly templates are the equivalents one level down:
[6502-PRG](https://github.com/acwright/6502-PRG) for a program in RAM,
[6502-CRT](https://github.com/acwright/6502-CRT) for a cartridge or a banked
Flash Cart, and [6502-BIN](https://github.com/acwright/6502-BIN) for raw
machine code with no BASIC stub.

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

- `make` or `make all` - Build the program (`make VDP=1` builds the 6502-PICOVDP version)
- `make view` - Display hexdump of the built program
- `make size` - Print the segment sizes from the link map
- `make woz` - Create a Wozmon compatible file using [bin2woz](https://github.com/acwright/bin2woz)
- `make cf` - Create a CompactFlash disk image containing the program
- `make run` - Launch the emulator app with the built program loaded (`ROM=path/to/BIOS.bin` boots that BIOS image instead of the bundled one)
- `make clean` - Remove build artifacts

Cartridge projects build a `.crt` instead of a `.prg`, so they have no `woz`
or `cf` target and add one of their own:

- `make eeprom` - Burn the image to a 28C256 with [minipro](https://gitlab.com/DavidGriffith/minipro)
- `make FLASH=128K|256K|512K|1M` - Build a banked Flash Cart image instead — see [Banked cartridges](#banked-cartridges)
- `make all-flash` - Build all four Flash Cart sizes
- `make flash` - Burn a Flash Cart in circuit through the Flash Helper

At the top level, `make` builds `6502.lib` and `6502-VDP.lib` first and then
every program directory, `make VDP=1` builds every program's VDP version, and
`make check` runs the header checks in `tests/` and the banked-config check.
`make configs` regenerates the four banked cartridge configs.

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
| `6502.h` | The Kernal, the IO registers and the constants, for C (BIOS 1.x, TMS9918A) |
| `6502.inc` | The same thing for ca65 — kept identical to the copy in [6502-ASM](https://github.com/acwright/6502-ASM) |
| `6502-VDP.h`, `6502-VDP.inc` | The same pair for BIOS 2.x and the 6502-PICOVDP — see [Choosing a build](#choosing-a-build) |
| `6502.cfg` | The linker config: where the program, the C stack and the heap go |
| `6502-16K.cfg` | The same, for a cartridge ROM at `$C000` |
| `6502-128K.cfg` … `6502-1M.cfg` | The same, for a banked Flash Cart — generated, see [Banked cartridges](#banked-cartridges) |
| `lib/` | The sources of `6502.lib` and `6502-VDP.lib` — startup, Kernal wrappers, `write()` |
| `<Program>/` | Your `.c`, its Makefile, its README |

### Choosing a build

| | `make` | `make VDP=1` |
|---|---|---|
| Machine | any ACE, TMS9918A | ACE with a 6502-PICOVDP |
| BIOS | 1.x | 2.x |
| Header | `6502.h` | `6502-VDP.h` |
| Library | `6502.lib` | `6502-VDP.lib` |
| Cartridge startup | `lib/crt0cart.o` | `lib/vdp/crt0cart.o` |
| Flash Cart startup | `lib/crt0cart-flash.o` | `lib/vdp/crt0cart-flash.o` |
| Outputs | `HelloWorld.prg` | `HelloWorld-VDP.prg` |

A source that builds both ways picks its header with the `VDP` macro. The
Makefile passes `-D VDP` for cc65 and `--asm-define VDP` for ca65, since
`cl65 -D` does not reach the assembler:

```c
#ifdef VDP
#include "6502-VDP.h"
#else
#include "6502.h"
#endif
```

`6502-VDP.h` is `6502.h` for BIOS 2.x, section for section: the PICOVDP's
second port pair, its registers, bit fields, status values and font IDs as
`VC_*` names from the card's `SPEC.md`; the video variables (`VID_PEN`,
`VDP_CAPS` ...); and wrappers for the thirteen PICOVDP Kernal entries,
`VdpInfo()` through `VdpStatus()`. There is no Monitor on 2.x, so there are no
`MONITOR_*` names, and `TMS_*` name the sixteen colours of palette row 0.

The two libraries come from the same sources. The PICOVDP wrappers
(`lib/vdp.s`) are only in `6502-VDP.lib`, so a legacy program cannot link a
call its ROM does not have. **Nothing built with `VDP=1` runs on a TMS9918A**;
a legacy build that only calls the Kernal also runs on 2.x.

On 2.x the video console colours every cell. `VideoSetColor()` sets the pen,
the colour of text printed from then on, and the border follows the
background; conio's `textcolor()` and `bgcolor()` set the pen the same way.
`waitvsync()` is the Kernal's `WaitVBlank`.

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
(`RtcReadTime`) or pack the pair into an `unsigned int` (`VideoGetCursor`,
`NvStat`).

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

On BIOS 1.x, three conio calls do not fit a TMS9918, which colours the whole
screen rather than each cell: `textcolor()` and `bgcolor()` recolour everything,
`bordercolor()` is the same register as `bgcolor()`, and `revers()` records
the flag but has no attribute to drive. And while conio's *output* follows
`IO_MODE` onto the serial console, its *cursor* is the BIOS's video cursor —
with no video card fitted the text still appears but `wherex()`/`wherey()`
always read 0,0. The comment at the top of `lib/conio.s` spells all of this
out before you rely on any of it.

`video.s` also carries **`waitvsync()`**, which waits for the VDP to finish a
frame so an update lands in the blanking interval. On 1.x the BIOS never reads
the VDP status register and leaves VDP interrupts off, so nothing competes for
the frame flag — but a program that installs its own handler through `IRQ_PTR`
could, so the wait gives up after about five frames rather than hanging. The
VDP build calls `WaitVBlank`, which polls a status register nothing clears.

`6502-VDP.lib` adds **`vdp.s`**, one wrapper for each of 2.x's PICOVDP entries.
Each returns 1 (or 0, or -1 where it also returns a value, as `6502-VDP.h`
says) when there is no PICOVDP or an argument is out of range.

### tests/

Two checks, run by `make check` and by a plain `make`, each on both pairs —
`6502.h`/`6502.inc` and `6502-VDP.h`/`6502-VDP.inc`:

- **`regcheck.c`** asserts at compile time that every field of every Section 5b
  IO block lands on the same address as its Section 5 macro, and that each
  block is exactly as wide as the card's window. Nothing is linked or run — a
  wrong offset is a build error instead of a program that writes to the wrong
  register.
- **`parity.py`** compares every address and every constant a header and its
  include both name, so the two cannot drift apart unnoticed.

## Cartridges

A cartridge overlays `$C000-$FFFF`, replacing BASIC, Wozmon, the CPU vectors
and, on BIOS 1.x, the Monitor, and the machine runs it from reset. The Kernal
stays put underneath — `$A000-$B7FF` plus the character set at `$B800-$BFFF`
on 1.x, all of `$A000-$BFFF` on 2.x — so the whole jump table, and everything
in `6502.h` or `6502-VDP.h` that wraps it, is still yours to call.

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

## Banked cartridges

**Start with the budget, because it goes the way nobody expects.**

| | Code space for the C program |
|---|--:|
| `6502-16K.cfg`, a 32 KB ROM cart | 16,378 bytes |
| `6502-512K.cfg`, a 512 KB Flash Cart | 8,186 bytes |

A C cartridge that does nothing special has **less** room on a banked cart,
not more. Everything the C runtime assumes is flat — `STARTUP`, `CODE`,
`RODATA`, the `DATA` load image — has to be in the fixed region, and the fixed
region is half the size of a fixed cartridge's window. A program that builds
today at 12 KB does not fit a banked build at all.

So: **you move to a banked cart for the banks, not for the code space.** What
you get in exchange is 120 KB to just under 1 MB of `const` data that a fixed
cartridge could not hold.

### The mapper, in one paragraph

`$C000-$DFFF` is an 8 KB window onto one bank at a time. `$E000-$FFFF` is a
fixed 8 KB — the chip's last bank, always — and it holds the CPU vectors,
which is why they can never be banked out. Writing any address in `$E000-$FFFF`
latches the bank register; it reaches no flash. Reset clears it to 0.

### Putting something in a bank

cc65 has no notion of banks, but it does let a translation unit name its
segments, and the linker config maps each `BANKnn` segment to a bank. **The
segment name is the value you write.**

```c
#pragma rodata-name (push, "BANK01")
static const unsigned char level1[] = { ... };   /* now at $C000 in bank 1 */
#pragma rodata-name (pop)

set_bank(0x01);
draw(level1);
```

That is the whole mechanism, and it is enough for the case that matters:
**large `const` data in banks, all code in the fixed region.** It is what the
toolchain supports well, and it is what [HelloWorldCart](HelloWorldCart)
demonstrates under `FLASH=`.

`set_bank` and `current_bank` are declared in `6502.h` and `6502-VDP.h` and
live in `crt0cart-flash.o`, which a cartridge Makefile links when `FLASH=` is
set:

```c
void set_bank(unsigned char bank);        /* select, and record in the shadow */
unsigned char current_bank(void);         /* what set_bank last selected */
```

The register cannot be read back, so `set_bank` keeps a zero-page shadow and
`current_bank` reads it. An interrupt handler that touches banked data saves
`current_bank()`, selects its own, and passes the saved value back to
`set_bank` before it returns.

### Calling a function in a bank: possible, and a trap

It compiles. It links. It crashes, and there is no diagnostic anywhere in the
toolchain.

`#pragma code-name (push, "BANK02")` puts a function in a bank, but the
compiler emits an ordinary `jsr`. So the caller has to select the bank first
and restore it after, the callee must not switch banks itself, and nothing
checks either. Worse, the callee's own `rts` executes whatever the *current*
bank holds at the return address if anything moved the register in between.

**Do not, until you have a reason.** These configs load `CODE` into `FIXED`
for exactly that reason: a C program that stays out of `#pragma code-name`
cannot make the mistake at all.

### What `DATA` has to do with any of this

`DATA`'s load image is pinned to the fixed region — `load = FIXED, run = RAM`
— and the reason is worth knowing because the failure looks like success.
`copydata` runs at reset to move initialized variables out of ROM and into
RAM. Bank 0 is what the register holds at reset, so a load image in a bank
would be read correctly, every time, until the day somebody put something else
in bank 0 — and then every initialized variable in the program would come up
holding that instead. `HelloWorldCart`'s `FLASH=` build puts a recognisable
string in bank 0 and prints an initialized variable next to it, which is the
cheapest possible check that this is still true.

### Naming

| Build | Output | Part |
|---|---|---|
| `make` | `Name.crt`, 32,768 bytes | 28C256 ROM cart |
| `make FLASH=128K` | `Name-128K.crt`, 131,072 bytes | SST39SF010A |
| `make FLASH=256K` | `Name-256K.crt`, 262,144 bytes | SST39SF020A |
| `make FLASH=512K` | `Name-512K.crt`, 524,288 bytes | SST39SF040 |
| `make FLASH=1M` | `Name-1M.crt`, 1,048,576 bytes | two SST39SF040 |

With `VDP=1` the `-VDP` comes first: `Name-VDP-512K.crt`. **The size in the
name is a label; the size in the bytes is what the machine reads** —
everything that loads a `.crt` picks the mapper from the byte count, and only
warns when the name disagrees.

`make run` on a banked image needs 6502-EMULATOR 3.4.0 or later; an older one
drops anything that is not 32,768 bytes without a word and boots to BASIC.
`make flash` burns a Flash Cart in circuit through the Flash Helper, and reads
the `.crt` and nothing else — there is no flag that folds a save into what it
writes. The [Bigger cartridges](https://acwright.github.io/6502-DOCS/assembly/flash-carts)
chapter is the full explanation.

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

## Contributing

A program here is a directory: the top-level Makefile builds `lib/` first and
then picks up anything with a `Makefile` in it, so adding one is the whole of
the mechanism.

1. **Copy the nearest worked example.** [HelloWorld](HelloWorld) for a `.prg`,
   [HelloWorldCart](HelloWorldCart) for a cartridge — its `FLASH=` build is
   also the banked one. Name the directory after the program in CamelCase, and
   give the `.c` file the same name.
2. **Change `TARGET` and `EIGHTTHREE` in the Makefile**, and nothing else to
   begin with. `TARGET` is the program name; `EIGHTTHREE` is the at-most-eight
   characters the file is called on a CompactFlash image, which is why
   `HelloWorld` ships as `HELLOWLD.PRG`.
3. **Use the shared headers, configs and library.** Your Makefile reaches the
   top level for all three — `-I ..` for the headers, `-C ../6502.cfg` (or
   `../6502-16K.cfg` for a cartridge) for the config, `../6502.lib` for the
   library — and keeps no copies of its own. `6502.inc` and `6502-VDP.inc` are
   kept identical to [6502-ASM](https://github.com/acwright/6502-ASM)'s, and
   `make check` compares each header against its include so the pair cannot
   drift apart.
4. **Build both ways if the program can.** `make VDP=1` picks `6502-VDP.h` and
   `6502-VDP.lib` and writes its outputs with a `-VDP` suffix — see
   [Choosing a build](#choosing-a-build). A program that only calls the Kernal
   needs no VDP build; one that touches the PICOVDP has no legacy build.
5. **Keep the usual targets working:** `make`, `make view`, `make size` and
   `make clean`, plus `woz` and `cf` for a `.prg`, or `eeprom`, `flash`,
   `FLASH=` and `all-flash` for a cartridge.
6. **Write a README for the directory** — what the program does, what hardware
   it needs, and anything surprising about what it costs. The size table in
   [What it costs](#what-it-costs) is the reason: on a 30 KB machine, what a
   program links is worth writing down.
7. **Commit what the source builds, not the disk image.** The `.prg` or 32 KB
   `.crt` and its `.lst` are committed so a reader can see what comes out
   without installing cc65. `.img`, `.map` and the larger Flash Cart images are
   in `.gitignore`.

Then open a pull request. If the program turned into something with a life of
its own while you were writing it, give it a repository of its own instead and
[add it to the software list](https://acwright.github.io/6502-DOCS/software/)
on the documentation site.

## Related

- [6502-ACE](https://github.com/acwright/6502-ACE) — the hardware, and the index of the whole family
- [6502-ASM](https://github.com/acwright/6502-ASM) — the same kind of collection for assembly language; `HelloWorld` and `HelloWorldCart` are there too
- [6502-BIOS](https://github.com/acwright/6502-BIOS) — the firmware behind `6502.h` and `6502.inc` (1.x) and `6502-VDP.h` and `6502-VDP.inc` (2.x)
- [6502-PICOVDP](https://github.com/acwright/6502-PICOVDP) — the video card behind `6502-VDP.h`
- [6502-EMULATOR](https://github.com/acwright/6502-EMULATOR) — run these programs without hardware
- [6502-BAS](https://github.com/acwright/6502-BAS) — the same kind of collection for BASIC listings
- [6502-PRG](https://github.com/acwright/6502-PRG) / [6502-CRT](https://github.com/acwright/6502-CRT) / [6502-BIN](https://github.com/acwright/6502-BIN) — the assembly templates for a program, a cartridge and a raw binary
- [6502-DOCS](https://github.com/acwright/6502-DOCS) — the documentation site

## License

MIT License — see [LICENSE](LICENSE).
