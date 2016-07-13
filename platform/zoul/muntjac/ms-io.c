#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include "board.h"
#include "contiki.h"
#include "ms-io.h"
#include "sampler.h"
#include "ds3231-sensor.h"
#include "dev/avr-handler.h"
#include "lpm.h"
#include "dev/adc-zoul.h"

#define DEBUG_ON
#include "debug.h"

/**
 * Pointer to the sample we're currently working on.
 * Used by the extra callback.
 */
static Sample *sample_extra;

/**
 * Struct used to get data from an AVR
 */
static struct avr_data data = {
    // Size of the buffer is the size of the buffer in the Sample
    .size = sizeof(((Sample_AVR_t *)0)->bytes)
};

/**
 *
 */
static void extra_callback(bool isSuccess);

/**
 * Read the temperature.
 * @param temp Pointer to write them tmep to.
 * @return True on success, False otherwise.
 */
static bool get_temp(float *temp);

void ms_init(void) {
    // Set the AVR callback
    avr_set_callback(&extra_callback);

    // Initialize the ADCs
    adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1 + ZOUL_SENSORS_ADC2);
}

void ms_sense_on(void) {
    lpm_set_max_pm(LPM_PM0);

    // Enable sense
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));
}

void ms_sense_off(void) {
    // TODO - Save and restore the max_pm in ms_sense_on
    //lpm_set_max_pm(LPM_CONF_MAX_PM);

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

bool ms_get_extra(Sample *sample, SensorConfig *config) {
	sample_extra = sample;

    ms_sense_on();

    if (config->hasADC1) {
        sample->has_ADC1 = true;
        sample->ADC1 = adc_zoul.value(ZOUL_SENSORS_ADC1);
    }

    if (config->hasADC2) {
        sample->has_ADC2 = true;
        sample->ADC2 = adc_zoul.value(ZOUL_SENSORS_ADC2);
    }

    sample->has_temp = get_temp(&sample->temp);

    // If we have no avr, we're done
    if (!config->has_avrID) {
        // No need to wait on anything else
        ms_sense_off();
        return true;
    }

    // Use the buffer in the sample directly
    data.data = sample->AVR.bytes;
    data.len = &sample->AVR.size;
    data.id = config->avrID;

    DEBUG("Getting data from avr %x\n", config->avrID);

	if (avr_get_data(&data)) {
        return false;
    } else {
        ms_sense_off();
        return true;
    }

    /*if (config->has_powerID) {
        // TODO - handle power board
        avr_get_data(sample->power); // Need to get a tiny buffer in Sample to implement this
    }*/
}

void extra_callback(bool isSuccess) {
    DEBUG("Sampled from avr. Success: %d\n", isSuccess);

    sample_extra->has_AVR = isSuccess;

    ms_sense_off();

    sampler_extra_performed();
}

bool get_temp(float *temp) {
    int centi_temp;

    if (ds3231_get_temperature(&centi_temp) != 0) {
        return false;
    }

    *temp = ((float) centi_temp) / 100;
    return true;
}
