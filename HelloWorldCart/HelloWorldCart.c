/* =============================================================================
 *   HelloWorldCart.c — The same greeting, burned into a cartridge
 * =============================================================================
 *
 *   HelloWorld in this repository is a .prg: BASIC loads it into RAM and RUN
 *   reaches it.  This is the same program as a cartridge ROM, which is a
 *   different thing entirely — it overlays $C000-$FFFF, replacing the Monitor,
 *   BASIC, Wozmon and the CPU vectors, and the machine runs it from reset.
 *
 *   That difference is the point of having both here:
 *
 *     HelloWorld       .prg   loaded into RAM at $0800, started from BASIC
 *     HelloWorldCart   .crt   ROM at $C000, started by the RESET vector
 *
 *   None of that shows up in this file, and that is what it is here to
 *   demonstrate.  main() is main() either way; the work of turning a reset
 *   into a running C program is done before it is called:
 *
 *     6502-16K.cfg   puts code and constants in ROM, variables in RAM
 *     lib/crt0cart.s resets the stack, calls KernalInit, copies DATA into
 *                    RAM, clears BSS, and owns the CPU vectors at $FFFA
 *
 *   Two things do change, and both are consequences of there being nothing
 *   underneath:
 *
 *     - The hardware is not up yet when the cartridge gains control, so
 *       crt0cart.s calls KernalInit.  A .prg never has to.
 *     - There is nothing to return to.  Returning from main() lands in a
 *       halt loop in crt0cart.s rather than back in BASIC.
 *
 *   The Kernal is still there underneath the cartridge window ($A000-$B7FF
 *   and the character set on BIOS 1.x, all of $A000-$BFFF on 2.x), so the
 *   whole jump table — and everything in 6502.h or 6502-VDP.h that wraps it
 *   — is available exactly as it always was.
 *
 *   THE FLASH CART BUILD  (make FLASH=128K|256K|512K|1M)
 *   ----------------------------------------------------
 *   Everything below in `#ifdef FLASH` is the banked build, and `make` with
 *   no FLASH= produces the same 32,768 bytes it always has.  What it adds is
 *   the one thing a C program on a banked cart actually does: a `const` array
 *   in a bank, read after a set_bank().  The README section *Banked
 *   cartridges* is the argument; this is the worked example of it.
 *
 * =============================================================================
 */

#include <stdio.h>

/* make VDP=1 builds for an ACE with a 6502-PICOVDP on BIOS 2.x;
 * the default builds for the TMS9918A on BIOS 1.x. */
#ifdef VDP
#include "6502-VDP.h"
#else
#include "6502.h"
#endif

#ifdef FLASH

/* --- Something to put in a bank -------------------------------------------
 *
 * `#pragma rodata-name` is the whole mechanism.  cc65 has no notion of banks;
 * it just puts this array in a segment called BANK01, and the linker config
 * puts that segment in the bank that appears at $C000-$DFFF when the register
 * holds 1.  The segment name IS the value you write.
 *
 * Note what is NOT here: no `#pragma code-name`.  All the code in this file
 * stays in the fixed region, which is where the configs put CODE, and calling
 * a function in a bank is the trap the README argues against at length.
 */

#pragma rodata-name (push, "BANK01")
static const char banked_greeting[] = "...and this one lives in bank $01.";
#pragma rodata-name (pop)

/* Bank 0 gets a recognisable pattern, for the check in main().  This is the
 * cheap version of catching the DATA-load-address trap: bank 0 is what the
 * register holds at reset, so a config that left DATA's load image in a bank
 * would copy THIS into the initialized variables and appear to work until
 * somebody used bank 0 for something.  The config pins DATA to FIXED, and the
 * assertion below is what says so out loud.
 */

#pragma rodata-name (push, "BANK00")
static const char bank0_filler[] = "BANK0BANK0BANK0BANK0BANK0BANK0BA";
#pragma rodata-name (pop)

/* An ordinary initialized variable.  crt0cart.s copies it out of the fixed
 * region and into RAM before main() runs. */
static char data_check[] = "initialized data survived";

#endif  /* FLASH */

int main(void)
{
    unsigned int version = KernalVersion();

    /* Safe with no video card fitted — VideoClear checks HW_PRESENT first,
     * and on a serial console there is nothing to clear. */
    VideoClear();

    /* Green text on a black backdrop, the one-call way.  HelloWorld.c spells
     * the same thing out as two register writes through the video card's IO
     * block; this is what you would normally reach for.  On the PICOVDP it
     * sets the pen, the colour of the text printed from here on. */
    VideoSetColor((TMS_LT_GREEN << 4) | TMS_BLACK);

    /* The cheap way: straight out through the Kernal. */
    PrintStr("Hello from Cartridge!");
    PrintCRLF();

    /* The C way.  printf() works here for the same reason it works in a .prg:
     * write() goes out through Chrout, which follows IO_MODE.  KernalInit
     * picked video or serial before main() was ever called. */
    printf("cc65 on AC6502, BIOS v%u.%u\n",
           VER_MAJOR(version), VER_MINOR(version));

#ifdef FLASH
    /* The bank register is cleared by reset, so this is bank 0 — and the
     * initialized variable must read as itself and not as bank 0's filler. */
    printf("bank %u: %s\n", current_bank(), data_check);

    /* Select bank 1 and read out of the window.  Everything after set_bank()
     * is an ordinary read of ordinary memory; PrintStr has no idea the string
     * it is given is in a bank, because once selected a bank is not special.
     *
     * PrintStr is in the Kernal at $A090, not in the window, so the bank stays
     * selected across the call.  A call into a BANKED function would be the
     * problem, and there is no such thing in this program. */
    set_bank(0x01);
    PrintStr(banked_greeting);
    PrintCRLF();

    /* Put it back, so anything that runs after this finds what it expects.
     * Nothing here does, but a routine that leaves the register somewhere its
     * caller did not choose is the other half of the bug set_bank's shadow
     * exists to make findable. */
    set_bank(0x00);

    /* Referenced so the linker keeps BANK00; the value is never used. */
    (void)bank0_filler;
#endif

    /* Returning is allowed — crt0cart.s runs the destructors and halts.
     * A cartridge with something to do would loop here instead. */
    return 0;
}
