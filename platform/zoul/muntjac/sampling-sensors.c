#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include "contiki.h"
#include "sampling-sensors.h"
#include "sampler.h"
#include "ds3231-sensor.h"

#define DEBUG_ON
#include "debug.h"

/**
 * Read the temperature.
 * @param temp Pointer to write them tmep to.
 * @return True on success, False otherwise.
 */
static bool get_temp(float *temp);

void sampler_init(void) {
}

bool sampler_get_time(uint32_t *seconds) {
    struct tm t;

    if (ds3231_get_time(&t) != 0) {
		*seconds = ERROR_EPOCH;
		return false;
	}

    *seconds = (uint32_t) mktime(&t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	//DEBUG("epoch is %" PRIu32 "\n", seconds);

	return true;
}

bool sampler_set_time(uint32_t seconds) {
    if (seconds < EARLIEST_EPOCH) {
        return false;
    }

    struct tm t;
	gmtime_r((time_t *) &seconds, &t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    //DEBUG("epoch %" PRIu32 "\n", seconds);

    return ds3231_set_time(&t) == 0;
}

bool sampler_get_extra(Sample *sample, SensorConfig *config) {

    sample->has_temp = get_temp(&sample->temp);

    // Everything completed synchronously
    return true;
}

bool get_temp(float *temp) {
    int centi_temp;

    if (ds3231_get_temperature(&centi_temp) != 0) {
        return false;
    }

    *temp = ((float) centi_temp) / 100;
    return true;
}
