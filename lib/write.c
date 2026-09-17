/* =============================================================================
 *   write.c — stdio's way out to the console
 * =============================================================================
 *
 *   cc65's stdio funnels every stream write through write(), so supplying
 *   this one function is what makes printf(), puts() and putchar() work.
 *
 *   There is no file descriptor table behind this: stdout and stderr both go
 *   to Chrout, which the BIOS routes to video or serial according to IO_MODE.
 *
 *   C says a line ends with '\n'.  The console wants CR then LF, the same
 *   pair an assembly program emits by hand, so the translation happens here
 *   and printf("...\n") lands where you expect.
 *
 * =============================================================================
 */

#include <unistd.h>

#ifdef VDP
#include "6502-VDP.h"
#else
#include "6502.h"
#endif

int __fastcall__ write(int fd, const void *buf, unsigned count)
{
    const char *p = (const char *)buf;
    unsigned n = count;

    (void)fd;                   /* every descriptor is the console */

    while (n--) {
        if (*p == '\n') {
            Chrout(CHAR_CR);
        }
        Chrout(*p++);
    }

    return (int)count;
}
