/**
 * Adapted from Minix - http://www.cise.ufl.edu/~cop4600/cgi-bin/lxr/http/source.cgi/lib/ansi/gmtime.c 
 */

/*
* gmtime - convert the calendar time into broken down time
*/
/* $Header: /users/cosc/staff/paul/CVS/minix1.7/src/lib/ansi/gmtime.c,v 1.2 1996/04/10 21:04:22 paul Exp $ */

#include "utc_time.h"

#define YEAR0           1900                    /* the first year */
#define EPOCH_YR        1970            /* EPOCH = Jan 1 1970 00:00:00 */
#define SECS_DAY        (24L * 60L * 60L)
#define LEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)  (LEAPYEAR(year) ? 366 : 365)
#define FIRSTSUNDAY(timp)       (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define FIRSTDAYOF(timp)        (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
#define TIME_MAX        ULONG_MAX
#define ABB_LEN         3

static const int _ytab[2][12] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

void epoch_to_tm(register const time_t *timer, struct tm * const timep) {
    time_t time = *timer;
    register unsigned long dayclock, dayno;
    int year = EPOCH_YR;

    dayclock = (unsigned long)time % SECS_DAY;
    dayno = (unsigned long)time / SECS_DAY;

    timep->tm_sec = dayclock % 60;
    timep->tm_min = (dayclock % 3600) / 60;
    timep->tm_hour = dayclock / 3600;
    while (dayno >= YEARSIZE(year)) {
        dayno -= YEARSIZE(year);
        year++;
    }
    timep->tm_year = year - YEAR0;
    timep->tm_mon = 0;

    while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
        dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
        timep->tm_mon++;
    }

    timep->tm_mday = dayno + 1;
}
