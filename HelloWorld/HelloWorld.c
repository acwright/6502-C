/* =============================================================================
 *   HelloWorld.c — Prints "Hello, World!" to the output device
 * =============================================================================
 *
 *   Built as a .prg that loads at $0800 and is started from BASIC with RUN.
 *   crt0.s puts the BASIC stub at the front of the file, sets up the C stack
 *   and calls main(); returning from main() hands control back to BASIC.
 *
 *   The system is already fully initialized when this runs — hardware probed,
 *   interrupts enabled, keyboard active, IO_MODE set — so there is nothing to
 *   bring up first.  Whatever Chrout writes to goes to the video card or the
 *   serial port according to IO_MODE, and the program never has to know which.
 *
 *   Two ways of printing are shown, and they cost very different amounts:
 *
 *     PrintStr()  is a Kernal call — a JSR and one Chrout per character.
 *     printf()    is the C library — flexible, and about 2 KB of code.
 *
 *   Both are worth having.  Reach for PrintStr() in a loop that runs often,
 *   and printf() when formatting is what you actually want.
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

    /* Green text on a black backdrop, set through the video card's IO block.
     *
     * A TMS9918 register write is two bytes to the same port: the value, then
     * the register number with VC_REG_WRITE set.  Register 7 holds the text
     * colour in its high nibble and the backdrop in its low one, and both take
     * effect across the whole screen at once — there is nothing to redraw.
     *
     * VideoSetColor((TMS_LT_GREEN << 4) | TMS_BLACK) is the Kernal's one-call
     * version of exactly this, and is what you would normally reach for.  It
     * is spelled out here to show an IO block in use: VC.reg is the same
     * address as the VC_REG macro, and compiles to the same two stores. */
    if (HW_PRESENT & HW_VID) {
#ifdef VDP
        /* On the PICOVDP every cell carries its own colour, taken from the
         * pen when it is written, and register 7's low nibble is only the
         * border.  So the text colour is the pen, set through the Kernal,
         * which also makes the border follow the background. */
        VideoSetColor((TMS_LT_GREEN << 4) | TMS_BLACK);
#else
        VC.reg = (TMS_LT_GREEN << 4) | TMS_BLACK;
        VC.reg = VC_REG_WRITE | VC_REG_COLOR;
#endif
    }

    /* The cheap way: straight out through the Kernal. */
    PrintCRLF();
    PrintStr("Hello, World!");
    PrintCRLF();

    /* The C way.  '\n' becomes CR+LF on the way out — see lib/write.c. */
    printf("cc65 on AC6502, BIOS v%u.%u\n",
           VER_MAJOR(version), VER_MINOR(version));

    return 0;                   /* Return to BASIC */
}
