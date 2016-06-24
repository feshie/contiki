#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include "contiki.h"
#include "sampling-sensors.h"
#include "sampler.h"
#include "ds3231-sensor.h"

#define DEBUG_ON
#include "debug.h"

void sampler_init(void) {
}

float sampler_get_temp(void) {
    int temp;

    if (ds3231_get_temperature(&temp) != 0) {
        return ERROR_VALUE;
    }

    return ((float) temp) / 100;
}

float sampler_get_batt(void) {
    return ERROR_VALUE;
}

int16_t sampler_get_acc_x(void) {
    return ERROR_VALUE;
}

int16_t sampler_get_acc_y(void) {
    return ERROR_VALUE;
}

int16_t sampler_get_acc_z(void) {
    return ERROR_VALUE;
}

uint32_t sampler_get_time(void) {
    struct tm t;
    if (ds3231_get_time(&t) != 0) {
		DEBUG("Error reading time from DS3231\n");
		return (uint32_t) ERROR_VALUE;
	}

    uint32_t seconds = (uint32_t) mktime(&t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	//DEBUG("epoch is %" PRIu32 "\n", seconds);
	return seconds;
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
    return true;
}
