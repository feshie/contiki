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
#include "event-sensor.h"
#include "pb_decode.h"
#include "power.pb.h"

#define DEBUG_ON
#include "debug.h"

/**
 * True if the AVRs have been handled and we should do the PowerBoards now, False otherwise.
 */
static bool is_avr_complete;

/**
 * Buffer to store incomming Power board data.
 * We can't use the Sample struct directly, as we need to decode the incomming data.
 */
static uint8_t power_data[PowerInfo_size];

/**
 * Length (not size) use of the power_data buffer.
 */
static uint8_t power_len;

/**
 * Pointer to the sample we're currently working on.
 * Used by the avr callback.
 */
static Sample *current_sample;

/**
 * Pointer to the config that was set when we starting sampling.
 * Used by the avr callback.
 */
static SensorConfig *current_config;

/**
 * Struct used to get data from an AVR
 */
static struct avr_data data;

/**
 * Callback invoked by the avr-handler.
 * @param isSuccess True if data was succesfully received, false otherwise.
 */
static void avr_callback(bool isSuccess);

/**
 * Start receiving data from an AVR.
 * @return True if the get-data operation was succesfully initiated, false otherwise.
 */
static bool start_avr(void);

/**
 * Finalize our state when we are done receiving data from an AVR.
 * @param isSuccess True if data was succesfully received, false otherwise.
 */
static void end_avr(bool isSuccess);

/**
 * Start receiving data from a power board.
 * @return True if the get-data operation was succesfully initiated, false otherwise.
 */
static bool start_power(void);

/**
 * Finalize our state when we are done receiving data from a power board.
 * @param isSuccess True if data was succesfully received, false otherwise.
 */
static void end_power(bool isSuccess);

/**
 * Read the temperature.
 * @param temp Pointer to write them tmep to.
 * @return True on success, False otherwise.
 */
static bool get_temp(float *temp);

void ms_init(void) {
    // Set the AVR callback
    avr_set_callback(&avr_callback);

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
	current_sample = sample;
    current_config = config;

    ms_sense_on();

    if (config->hasADC1) {
        sample->has_ADC1 = true;
        sample->ADC1 = adc_zoul.value(ZOUL_SENSORS_ADC1);
    }

    if (config->hasADC2) {
        sample->has_ADC2 = true;
        sample->ADC2 = adc_zoul.value(ZOUL_SENSORS_ADC2);
    }

    if (config->hasRain) {
        sample->has_rain = true;
        sample->rain = get_rain();
    }

    sample->has_temp = get_temp(&sample->temp);

    // If we have no AVR, consider it done
    is_avr_complete = !config->has_avrID;

    // No avr
    if (is_avr_complete) {
        // If we have a power board, and it has been queud successfully for getting data,
        // Let the sampler know we'll call it back
        if (config->has_powerID && start_power()) {
            return false;
        }

        // Otherwise we're done
        ms_sense_off();
        return true;
    }

    // If we have an AVR, and it has been queud successfully for getting data,
    // Let the sampler know we'll call it back
    if (config->has_avrID && start_avr()) {
        return false;
    }

    // Otherwise we're done
    ms_sense_off();
    return true;
}

void avr_callback(bool isSuccess) {
    // If the avr isn't done yet, this must be the end of it
    if (!is_avr_complete) {
        end_avr(isSuccess);
        is_avr_complete = true;

        // If we have a power board and we've succesfully queued a get data, wait until we get called again
        if (current_config->has_powerID && start_power()) {
            return;
        }

    // Otherwise it must be the end of the power board
    } else {
        end_power(isSuccess);
    }

    ms_sense_off();
    sampler_extra_performed();
}

bool start_avr(void) {
    // Use the buffer in the sample directly
    data.data = current_sample->AVR.bytes;
    data.len = &current_sample->AVR.size;
    data.id = current_config->avrID;
    // Size of the buffer is the size of the buffer in the Sample
    data.size = sizeof(current_sample->AVR.bytes);

    DEBUG("Getting data from avr 0x%02X\n", current_config->avrID);

    return avr_get_data(&data);
}

void end_avr(bool isSuccess) {
    DEBUG("Sampled from avr. Success: %d\n", isSuccess);
    current_sample->has_AVR = isSuccess;
}

bool start_power(void) {
    // Use the power_data buffer
    data.data = power_data;
    data.len = &power_len;
    data.id = current_config->powerID;
    data.size = sizeof(power_data);

    DEBUG("Getting data from power 0x%02X\n", current_config->powerID);

    return avr_get_data(&data);
}

void end_power(bool isSuccess) {
    DEBUG("Sampled from power board. Success: %d\n", isSuccess);

    if (!isSuccess) {
        return;
    }

    pb_istream_t istream = pb_istream_from_buffer(data.data, *data.len);
    if (!pb_decode(&istream, PowerInfo_fields, &current_sample->power)) {
        return;
    }

    current_sample->which_battery = Sample_power_tag;
    return;
}

bool get_temp(float *temp) {
    int centi_temp;

    if (ds3231_get_temperature(&centi_temp) != 0) {
        return false;
    }

    *temp = ((float) centi_temp) / 100;
    return true;
}
