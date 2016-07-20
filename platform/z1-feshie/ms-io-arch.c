#include "dev/uart1_i2c_master.h"
#include "dev/ds3231-sensor.h" 	// Clock
#include "dev/adc1-sensor.h" 	// ADC 1
#include "dev/adc2-sensor.h" 	// ADC 2
#include "dev/batv-sensor.h" // Batt
#include "adxl345.h" 		// Accel
#include "dev/event-sensor.h"	//event sensor (rain)
#include "mountainsensing/common/ms-io.h"
#include "utc_time.h"
#include "contiki-conf.h"
#include "contiki.h"
#include "dev/reset-sensor.h"

#define DEBUG_ON
#include "mountainsensing/common/debug.h"

#define ADC_ACTIVATE_DELAY 10 //delay in ticks of the rtimer  PLATFORM DEPENDANT!

/**
 * Earliest time supported by the rtc - 2000/01/01 00:00:00
 */
#define EARLIEST_EPOCH 946684800

static void ms_radio_on(void);
static void ms_radio_off(void);

void ms_init(void) {
    // Set up sense control pin
    SENSE_EN_PORT(SEL) &= ~BV(SENSE_EN_PIN);
    SENSE_EN_PORT(DIR) |= BV(SENSE_EN_PIN);
    SENSE_EN_PORT(REN) &= ~BV(SENSE_EN_PIN);
    SENSE_EN_PORT(OUT) &= ~BV(SENSE_EN_PIN);

    //Make sure all analogue input pins are inputs.
    P6DIR = 0x00;
    P6SEL = 0x00;

    /* Ensure that rain bucket is input. */
    P2DIR &= ~BV(0);

    // Set up radio control pin
    RADIO_EN_PORT(SEL) &= ~BV(RADIO_EN_PIN);
    RADIO_EN_PORT(DIR) |= BV(RADIO_EN_PIN);
    RADIO_EN_PORT(REN) &= ~BV(RADIO_EN_PIN);
    DEBUG("\tTurning on radio\n");
    //Turn on by default
    ms_radio_on();
    ms_sense_off(); //turn off sensors by default

    // Turn the rain sensor on.
    // It needs to be permanently on to count the rain ticks.
    SENSORS_ACTIVATE(event_sensor);
}

void ms_radio_on(void) {
	DEBUG("Turning on radio\n");
	RADIO_EN_PORT(OUT) |= BV(RADIO_EN_PIN);
}

void ms_radio_off(void) {
	DEBUG("Turning off radio\n");
	RADIO_EN_PORT(OUT) &= ~BV(RADIO_EN_PIN);
}

void ms_sense_on(void){
	DEBUG("Turning on sense\n");
	SENSE_EN_PORT(OUT) |= BV(SENSE_EN_PIN);
}

void ms_sense_off(void){
	DEBUG("Turning off sense\n");
	SENSE_EN_PORT(OUT) &= ~BV(SENSE_EN_PIN);
}

bool ms_get_temp(float *temp) {
    *temp = ((float) ds3231_temperature()) / 100;
    return true;
}

bool ms_get_batt(float *batt) {
    ms_sense_on();
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(batv_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    *batt = (float)(batv_sensor.value(0)) / 184.06 - 0.2532;
    SENSORS_DEACTIVATE(batv_sensor);
    return true;
}

bool ms_get_time(uint32_t *seconds) {
    struct tm t;
    ds3231_get_time(&t);

    *seconds = (uint32_t) tm_to_epoch(&t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	//DEBUG("epoch is %" PRIu32 "\n", seconds);
	return true;
}

bool ms_set_time(uint32_t seconds) {
    if (seconds < EARLIEST_EPOCH) {
        return false;
    }

    struct tm t;
    epoch_to_tm((time_t *) &seconds, &t);

	//DEBUG("years %d, months %d, days %d, hours %d, minutes %d, seconds %d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    //DEBUG("epoch %" PRIu32 "\n", seconds);

    return ds3231_set_time(&t) == 0;
}

bool ms_get_rain(uint32_t *rain) {
    *rain = (uint32_t) event_sensor.value(1);
    return true;
}

bool ms_get_adc1(uint32_t *adc1) {
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(adc1_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    *adc1 = adc1_sensor.value(0);
    SENSORS_DEACTIVATE(adc1_sensor);
    return true;
}

bool ms_get_adc2(uint32_t *adc2) {
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(adc2_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    *adc2 = adc2_sensor.value(0);
    SENSORS_DEACTIVATE(adc2_sensor);
    return true;
}

bool ms_get_acc(int32_t *x, int32_t *y, int32_t *z) {
    // Unsupported
    return false;
}

bool ms_get_humid(float *humid) {
    // Unsupported
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
