#include "dev/uart1_i2c_master.h"
#include "ms1-io.h"
#include "dev/ds3231-sensor.h" 	// Clock
#include "dev/adc1-sensor.h" 	// ADC 1
#include "dev/adc2-sensor.h" 	// ADC 2
#include "dev/temperature-sensor.h" // Temp
#include "dev/batv-sensor.h" // Batt
#include "adxl345.h" 		// Accel
#include "dev/event-sensor.h"	//event sensor (rain)
#include "sampling-sensors.h"
#include "utc_time.h"
#include "dev/avr-handler.h"
#include "sampler.h"

#define DEBUG_ON
#include "debug.h"

#define ADC_ACTIVATE_DELAY 10 //delay in ticks of the rtimer  PLATFORM DEPENDANT!

#define AVR_RETRIES 3

/**
 * Earliest time supported by the rtc - 2000/01/01 00:00:00
 */
#define EARLIEST_EPOCH 946684800

/**
 * If defined, do not turn sensor power off
 */
//#define SENSE_ON

/**
 * If defined, do not attempt to read the time from the RTC
 */
//#define NO_RTC

/**
 * If defined, do not attempt to read the acceleration from the acelerometer
 */
//#define NO_ACC

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
 * Get the value of the rain sensor (number of bucket tips).
 */
static uint16_t get_rain(void);

/**
 * Get the value of adc1.
 */
static uint16_t get_ADC1(void);

/**
 * Get the value of adc1.
 */
static uint16_t get_ADC2(void);

void sampler_init(void) {
#ifdef SENSE_ON
    ms1_sense_on();
    DEBUG("Sensor power permanently on\n");
#endif

    // Turn the rain sensor on.
    // It needs to be permanently on to count the rain ticks.
    SENSORS_ACTIVATE(event_sensor);

    // Set the AVR callback
    avr_set_callback(&extra_callback);
}

float sampler_get_temp(void) {
    float temp;
    SENSORS_ACTIVATE(temperature_sensor);
    temp = (float)(((temperature_sensor.value(0)*2.500)/4096)-0.986)*282;
    SENSORS_DEACTIVATE(temperature_sensor);
    return temp;
}

float sampler_get_batt(void) {
    ms1_sense_on();
    float bat_ret;
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(batv_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    bat_ret =  (float)(batv_sensor.value(0)) / 273.067;
    SENSORS_DEACTIVATE(batv_sensor);
#ifndef SENSE_ON
    ms1_sense_off();
#endif
    return bat_ret;
}

int16_t sampler_get_acc_x(void) {
#ifdef NO_ACC
    #warning "ACC disabled"
    return 12345;
#else
    return accm_read_axis(X_AXIS);
#endif
}

int16_t sampler_get_acc_y(void) {
#ifdef NO_ACC
    return 12345;
#else
    return accm_read_axis(Y_AXIS);
#endif
}

int16_t sampler_get_acc_z(void) {
#ifdef NO_ACC
    return 12345;
#else
    return accm_read_axis(Z_AXIS);
#endif
}

uint32_t sampler_get_time(void) {
#ifdef NO_RTC
    #warning "RTC disabled"
    return (uint32_t)12345;
#else
    struct tm t;
    ds3231_get_time(&t);

    uint32_t seconds = (uint32_t) tm_to_epoch(&t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	//DEBUG("epoch is %" PRIu32 "\n", seconds);
	return seconds;
#endif // ifdef NO_RTC
}

bool sampler_set_time(uint32_t seconds) {
#ifdef NO_RTC
    return true;
#else
    if (seconds < EARLIEST_EPOCH) {
        return false;
    }

    struct tm t;
    epoch_to_tm((time_t *) &seconds, &t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    //DEBUG("epoch %" PRIu32 "\n", seconds);

    return ds3231_set_time(&t) == 0;
#endif // ifdef NO_RTC
}

bool sampler_get_extra(Sample *sample, SensorConfig *config) {
    sample_extra = sample;

    // Use the buffer in the sample directly
    data.data = sample->AVR.bytes;
    data.len = &sample->AVR.size;

    ms1_sense_on();

    if (config->hasADC1) {
        sample->has_ADC1 = true;
        sample->ADC1 = get_ADC1();
    }

    if (config->hasADC2) {
        sample->has_ADC2 = true;
        sample->ADC2 = get_ADC2();
    }

    if (config->hasRain) {
        sample->has_rain = true;
        sample->rain = get_rain();
    }

    // If there are no avrs, we're done
    if (config->avrIDs_count < 1) {
#ifndef SENSE_ON
        ms1_sense_off();
#endif
        // No need to wait on anything else
        return true;
    }

    uint8_t avr_id = (uint8_t) config->avrIDs[0];

    DEBUG("Getting data from avr %x\n", avr_id);
    if (avr_get_data(avr_id, &data)) {
        return false;
    } else {
#ifndef SENSE_ON
        ms1_sense_off();
#endif
        return true;
    }
}

static void extra_callback(bool isSuccess) {
    DEBUG("Sampled from avr. Success: %d\n", isSuccess);

    sample_extra->has_AVR = isSuccess;

#ifndef SENSE_ON
    ms1_sense_off();
#endif
    sampler_extra_performed();
}

uint16_t get_rain(void) {
    return event_sensor.value(1);
}

uint16_t get_ADC1(void) {
    uint16_t adc1_ret;
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(adc1_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    adc1_ret =  adc1_sensor.value(0);
    SENSORS_DEACTIVATE(adc1_sensor);
    return adc1_ret;
}

uint16_t get_ADC2(void) {
    uint16_t ret;
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(adc2_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    ret =  adc2_sensor.value(0);
    SENSORS_DEACTIVATE(adc2_sensor);
    return ret;
}