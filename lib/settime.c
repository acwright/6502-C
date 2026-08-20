/* =============================================================================
 *   settime.c — clock_settime() over the DS1511Y
 * =============================================================================
 *
 *   The other half of gettime.c: put a time_t into the battery-backed clock,
 *   so a program can set the machine's time from a serial download, a file,
 *   or something the user typed.
 *
 *       struct timespec ts;
 *       ts.tv_sec  = mktime(&tm);
 *       ts.tv_nsec = 0;
 *       clock_settime(CLOCK_REALTIME, &ts);
 *
 *   Kept in its own module because it pulls in localtime(), which a program
 *   that only wants to read the clock has no reason to carry.
 *
 *   The timezone convention matches gettime.c: localtime() adds _tz.timezone,
 *   so with the default all-zero _tz a UTC time_t is written to the RTC as
 *   UTC, and the two round-trip.
 *
 * =============================================================================
 */

#include <time.h>
#include <errno.h>
#include <stddef.h>

#include "6502.h"

int __fastcall__ clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    const struct tm *tm;
    RtcTime          t;
    RtcDate          d;
    int              year;

    if (clock_id != CLOCK_REALTIME) {
        errno = EINVAL;
        return -1;
    }
    if (!(HW_PRESENT & HW_RTC)) {
        errno = ENODEV;
        return -1;
    }

    tm = localtime(&tp->tv_sec);
    if (tm == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* The DS1511Y keeps the century in its own register and the BIOS documents
     * it as 0-39, so the clock cannot hold a year outside 0-3999. */
    year = tm->tm_year + 1900;
    if (year < 0 || year > 3999) {
        errno = ERANGE;
        return -1;
    }

    t.hours   = (unsigned char)tm->tm_hour;
    t.minutes = (unsigned char)tm->tm_min;
    t.seconds = (unsigned char)tm->tm_sec;

    d.day     = (unsigned char)tm->tm_mday;
    d.month   = (unsigned char)(tm->tm_mon + 1);    /* tm_mon is 0-11 */
    d.year    = (unsigned char)(year % 100);
    d.century = (unsigned char)(year / 100);

    /* Date first, then time: the time is the field that rolls, so writing it
     * last leaves the smallest window in which the two disagree. */
    RtcWriteDate(&d);
    RtcWriteTime(&t);

    return 0;
}
