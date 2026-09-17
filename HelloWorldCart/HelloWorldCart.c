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

    /* Returning is allowed — crt0cart.s runs the destructors and halts.
     * A cartridge with something to do would loop here instead. */
    return 0;
}
