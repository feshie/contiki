#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include "board.h"
#include "contiki.h"
#include "mountainsensing/common/ms-io.h"
#include "ds3231-sensor.h"
#include "lpm.h"
#include "dev/adc-zoul.h"
#include "event-sensor.h"
#include "reset-sensor.h"

#define DEBUG_ON
#include "mountainsensing/common/debug.h"

/**
 * True if senser power is on, false otherwise
 */
static bool is_sense_on = false;

/**
 * Callback allowing / disallowing the CC2538 to enter LPM{1,2}
 */
static bool lpm_permit_pm1(void) {
    // LPM{1,2} are only allowed if we're not sensing
    return !is_sense_on;
}

void ms_init(void) {
    // Initialize the ADCs
    adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1 + ZOUL_SENSORS_ADC2 + ZOUL_SENSORS_ADC3);

    // Register our lpm_permit callback so we can stay in LPM0 during sensing
    lpm_register_peripheral(&lpm_permit_pm1);

    // Turn sense off by default
    ms_sense_off();

    // Enable the rain sensor
    SENSORS_ACTIVATE(event_sensor);
}

void ms_sense_on(void) {
    is_sense_on = true;

    // Enable sense
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));
}

void ms_sense_off(void) {
    is_sense_on = false;

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
    // Batt sensor is on ADC3
    // Munge factor of 1698 gives about the right voltage on multiple boards, within 0.5V.
    *batt = ((float) (adc_zoul.value(ZOUL_SENSORS_ADC3)/1698));
    return true;
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
    // Reset the value so we only get the delta every time
    *rain = event_sensor.value(true);
    return true;
}

bool ms_get_acc(int32_t *x, int32_t *y, int32_t *z) {
    // Not supported
    return false;
}

bool ms_get_reboot(uint16_t *reboot) {
    // Param is ignored by the reset sensor
    *reboot = reset_sensor.value(0);
    return true;
}

bool ms_reset_reboot(void) {
    reset_counter_reset();
    return true;
}
