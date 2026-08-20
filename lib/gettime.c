/* =============================================================================
 *   gettime.c — clock_gettime() over the DS1511Y
 * =============================================================================
 *
 *   cc65's time.h is almost entirely target-independent: time(), localtime(),
 *   gmtime(), mktime(), strftime(), ctime() and asctime() are all in the
 *   library already and work anywhere.  What they need from a machine is one
 *   function — clock_gettime() — and without it the whole header fails to
 *   link.  Supplying it here is what makes the rest of time.h come alive:
 *
 *       time_t now = time(NULL);
 *       strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", localtime(&now));
 *
 *   The AC6502 is better placed for this than most 8-bit machines.  A C64 has
 *   to fake a clock from the jiffy counter and loses it on every reset; the
 *   DS1511Y is battery-backed, so the time here survives the power going off.
 *
 *   TIMEZONE
 *   --------
 *   mktime() finishes by subtracting _tz.timezone, so the value the RTC holds
 *   is whatever _tz says it is.  _tz defaults to all zeroes, which means the
 *   DS1511Y is read as UTC and time() returns a true epoch count.  A program
 *   that keeps local time in the clock instead should set _tz before calling,
 *   and localtime() will then agree with the wall.
 *
 *   Sub-second time is not available: the DS1511Y is read to the second, so
 *   tv_nsec is always 0.  clock() is a separate matter and is not provided —
 *   it needs a free-running tick this machine has no counter for, and
 *   CLOCKS_PER_SEC is not defined for target `none` in any case.
 *
 * =============================================================================
 */

#include <time.h>
#include <errno.h>
#include <stddef.h>

#include "6502.h"

int __fastcall__ clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    struct tm tm;
    RtcTime   t;
    RtcDate   d, again;

    if (clock_id != CLOCK_REALTIME) {
        errno = EINVAL;
        return -1;
    }
    if (!(HW_PRESENT & HW_RTC)) {
        errno = ENODEV;
        return -1;
    }

    /* The date and the time are two separate Kernal calls, so midnight can
     * fall between them and pair a new day with an old time — an error of a
     * whole day, once a day.  Read the date on both sides of the time; if it
     * moved, the clock has rolled over and the second reading is the real
     * one. */
    RtcReadDate(&d);
    RtcReadTime(&t);
    RtcReadDate(&again);

    if (d.day     != again.day   || d.month   != again.month ||
        d.year    != again.year  || d.century != again.century) {
        d = again;
        RtcReadTime(&t);
    }

    tm.tm_sec   = t.seconds;
    tm.tm_min   = t.minutes;
    tm.tm_hour  = t.hours;                              /* 24h, already binary */
    tm.tm_mday  = d.day;
    tm.tm_mon   = d.month - 1;                          /* tm_mon is 0-11 */
    tm.tm_year  = (d.century * 100) + d.year - 1900;    /* years since 1900 */
    tm.tm_wday  = 0;                                    /* mktime fills these in */
    tm.tm_yday  = 0;
    tm.tm_isdst = 0;                                    /* cc65's mktime ignores it */

    tp->tv_sec  = mktime(&tm);
    tp->tv_nsec = 0;

    return 0;
}

/* time() lives here rather than being taken from cc65's library, and the
 * reason is link order rather than behaviour.
 *
 * ld65 scans 6502.lib before the target library, so a reference that only
 * appears inside the target library cannot reach back.  cc65's own time()
 * imports clock_gettime, so pulling it in would leave that import dangling:
 *
 *     common/time.s:30: Warning: Unresolved external '_clock_gettime'
 *
 * Defining time() here satisfies the call out of 6502.lib, cc65's version is
 * never pulled, and nothing dangles.  The behaviour is the same, including
 * returning (time_t)-1 and leaving errno set when the clock cannot be read. */
time_t __fastcall__ time(time_t *t)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        ts.tv_sec = (time_t)-1L;
    }

    if (t != NULL) {
        *t = ts.tv_sec;
    }

    return ts.tv_sec;
}
