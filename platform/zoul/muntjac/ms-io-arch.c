#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include "board.h"
#include "contiki.h"
#include "mountainsensing/common/ms-io.h"
#include "ds3231-sensor.h"
#include "power-sheriff.h"
#include "dev/adc-zoul.h"

#define DEBUG_ON
#include "mountainsensing/common/debug.h"

void ms_init(void) {
    // Initialize the ADCs
    adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1 + ZOUL_SENSORS_ADC2);

    // Turn sense off by default
    ms_sense_off();
}

void ms_sense_on(void) {
    power_sheriff_high_power();

    // Enable sense
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));
}

void ms_sense_off(void) {
    power_sheriff_low_power();

    // Disable sense
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));
}

bool ms_get_time(uint32_t *seconds) {
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

bool ms_set_time(uint32_t seconds) {
    if (seconds < EARLIEST_EPOCH) {
        return false;
    }

    struct tm t;
	gmtime_r((time_t *) &seconds, &t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    //DEBUG("epoch %" PRIu32 "\n", seconds);

    return ds3231_set_time(&t) == 0;
}

bool ms_get_batt(float *batt) {
    return false;
}

bool ms_get_temp(float *temp) {
    int centi_temp;

    if (ds3231_get_temperature(&centi_temp) != 0) {
        return false;
    }

    *temp = ((float) centi_temp) / 100;
    return true;
}

bool ms_get_humid(float *humid) {
    // Not supported
    return false;
}

bool ms_get_adc1(uint32_t *adc1) {
    *adc1 = adc_zoul.value(ZOUL_SENSORS_ADC1);
    return true;
}

bool ms_get_adc2(uint32_t *adc2) {
    *adc2 = adc_zoul.value(ZOUL_SENSORS_ADC2);
    return true;
}

bool ms_get_rain(uint32_t *rain) {
    // TODO - not implemented yet
    return false;
}

bool ms_get_acc(int32_t *x, int32_t *y, int32_t *z) {
    // Not supported
    return false;
}
