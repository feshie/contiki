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

#define DEBUG_ON
#include "debug.h"

#define ADC_ACTIVATE_DELAY 10 //delay in ticks of the rtimer  PLATFORM DEPENDANT!

//#define NO_RTC
//#define NO_ACC

/**
 * Earliest time supported by the rtc - 2000/01/01 00:00:00
 */
#define EARLIEST_EPOCH 946684800

/**
 * If defined, do not turn sensor power off
 */
//#define SENSE_ON

static uint16_t get_rain(void);

static uint16_t get_ADC1(void);

static uint16_t get_ADC2(void);

void sampler_init(void) {
#ifdef SENSE_ON
    ms1_sense_on();
    DEBUG("Sensor power permanently on\n");
#endif

    // Turn the rain sensor on.
    // It needs to be permanently on to count the rain ticks.
    SENSORS_ACTIVATE(event_sensor);
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

	DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	DEBUG("epoch is %" PRIu32 "\n", seconds);
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

	DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    DEBUG("epoch %" PRIu32 "\n", seconds);

    return ds3231_set_time(&t) == 0;
#endif // ifdef NO_RTC
}

bool sampler_get_extra(Sample *sample, SensorConfig *config) {
    /*static int i;
    static uint8_t j;
    static uint8_t avr_id;
    static struct etimer avr_timeout_timer;
    static uint8_t avr_recieved;
    static uint8_t avr_retry_count;*/

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

    /*avr_recieved = 0;
    avr_retry_count = 0;

    DEBUG("Sampling from %d AVRs\n", config->avrIDs_count);

    PROCESS_CONTEXT_BEGIN(&sample_process);

    for (i = 0; i < config->avrIDs_count; i++) {
        avr_id = config->avrIDs[i] & 0xFF; //only use 8bits
        DEBUG("AVR ID: %d\n", avr_id);
        avr_recieved = 0;
        avr_retry_count = 0;

        do {
            protobuf_send_message(avr_id, PROTBUF_OPCODE_GET_DATA, NULL, (int)NULL);
            DEBUG("Retry %d\n", avr_retry_count);

            etimer_set(&avr_timeout_timer, CLOCK_SECOND * AVR_TIMEOUT_SECONDS);

            DEBUG("Yielding for timeout\n");

            PROCESS_WAIT_EVENT_UNTIL(ev == protobuf_event || etimer_expired(&avr_timeout_timer));

            if (ev == PROCESS_EVENT_TIMER) {
                DEBUG("AVR Timeout Reached!\n");
                avr_retry_count++;

            } else if (ev == protobuf_event) {

                if (data != NULL) {
                    DEBUG("\tavr data recieved on retry %d\n", avr_retry_count);
                    protobuf_data_t *pbd;
                    pbd = data;
                    sample->has_AVR = 1;
                    sample->AVR.size = pbd->length;
                    for(j=0; j < pbd->length; j++){
                        sample->AVR.bytes[j] = pbd->data[j];
                    }
                    DEBUG("\tRecieved %d bytes\t", pbd->length);
                    for(j = 0; j < pbd->length; j++) {
                        DEBUG("%d:", pbd->data[j]);
                    }
                    DEBUG("\n");
                    //process data
                    if (avr_id < 0x10) {
                        //it's a temp accel chain and so needs to be read twice to get valid data
                        //otherwise just once is ok
                        // It will break out of this loop when avr_recieved == 2
                        avr_recieved++;
                    } else {
                        avr_recieved = 2;
                    }
                }
            }

        DEBUG("avr_recieved = %d\n", avr_recieved);
        } while(avr_recieved < 2 && avr_retry_count < PROTOBUF_RETRIES);
    }

    PROCESS_CONTEXT_END(&sample_process);*/

#ifndef SENSE_ON
    ms1_sense_off();
#endif
    return true;
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
