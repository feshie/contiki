/**
 * @file
 * Utility functions for operating on UTC times.
 * Convenient functions for converting between the epoch and the tm struct.
 */

#ifndef UTC_TIME_H
#define UTC_TIME_H

#include <stdint.h>

typedef uint32_t time_t;

/**
 * tm
 *
 * An abbreviated version of tm struct from time.h. See Open Group for full
 * documentation of this structure.
 */
struct tm {
	int tm_sec; 	// seconds [0,61]
	int tm_min; 	// minutes [0,59]
	int tm_hour; 	// hour [0,23]
	int tm_mday; 	// day of month [1,31]
	int tm_mon; 	// month of year [0,11]
	int tm_year; 	// years since 1900
};

/**
 * Convert a UNIX epoch to a tm struct.
 * @param timer The UNIX epoch
 * @param tmp The tm struct to conver to
 */
void epoch_to_tm(const time_t *timer, struct tm * const tmp);

/**
 * Convert a tm struct to a UNIX epoch.
 * @param tmp The tm struct to convert
 * @return The UNIX epoch
 */
time_t tm_to_epoch(struct tm * const tmp);

#endif // ifndef UTC_TIME_H
