#include "clock.h"
#include "ms-io.h"
#include "dev/sht25.h"
#include "dev/battery-sensor.h"
#include "i2cmaster.h"
#include "adxl345.h"

//#define DEBUG_ON
#include "debug.h"

/**
 * Get the battery volatge.
 */
float get_batt(void);

/**
 * Get the temp in C
 */
float get_temp(void);

/**
 * Get the humidity in %
 */
float get_humid(void);

/**
 * Get the accelerometer reading from a single axis.
 * This will enable and disable i2c as required.
 */
static int16_t get_acc(enum ADXL345_AXIS axis);

void ms_init(void) {
    // We don't need to do anything
}

void ms_sense_on(void) {
    // We don't need to do anything
}

void ms_sense_off(void) {
    // We don't need to do anything
}

float get_temp(void) {
    float temp;
    SENSORS_ACTIVATE(sht25);
    temp = ((float) sht25.value(SHT25_VAL_TEMP)) / 100;
    SENSORS_DEACTIVATE(sht25);
    return temp;
}

float get_humid(void) {
    float humid;
    SENSORS_ACTIVATE(sht25);
    humid = ((float) sht25.value(SHT25_VAL_HUM)) / 100;
    SENSORS_DEACTIVATE(sht25);
    return humid;
}

float get_batt(void) {
    float bat_ret;
    SENSORS_ACTIVATE(battery_sensor);
    bat_ret =  (float) battery_sensor.value(0);
    SENSORS_DEACTIVATE(battery_sensor);
    return bat_ret;
}

int16_t get_acc(enum ADXL345_AXIS axis) {
    i2c_enable();
    int16_t acc = accm_read_axis(axis);
    i2c_disable();
    return acc;
}

bool ms_get_time(uint32_t *seconds) {
    *seconds = (uint32_t) clock_seconds();
    return true;
}

bool ms_set_time(uint32_t seconds) {
    clock_set_seconds((unsigned long) seconds);
    return true;
}

bool ms_get_extra(Sample *sample, SensorConfig *config) {
    sample->batt = get_batt();
    sample->has_batt = true;

    sample->temp = get_temp();
    sample->has_temp = true;

    sample->humid = get_humid();
    sample->has_humid = true;

    sample->accX = get_acc(X_AXIS);
    sample->has_accX = true;
    sample->accY = get_acc(Y_AXIS);
    sample->has_accY = true;
    sample->accZ = get_acc(Z_AXIS);
    sample->has_accZ = true;

    return true;
}
