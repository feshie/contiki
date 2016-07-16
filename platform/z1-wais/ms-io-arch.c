#include "clock.h"
#include "mountainsensing/common/ms-io.h"
#include "dev/sht25.h"
#include "dev/battery-sensor.h"
#include "i2cmaster.h"
#include "adxl345.h"

//#define DEBUG_ON
#include "mountainsensing/common/debug.h"

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

bool ms_get_temp(float *temp) {
    SENSORS_ACTIVATE(sht25);
    *temp = ((float) sht25.value(SHT25_VAL_TEMP)) / 100;
    SENSORS_DEACTIVATE(sht25);
    return true;
}

bool ms_get_humid(float *humid) {
    SENSORS_ACTIVATE(sht25);
    *humid = ((float) sht25.value(SHT25_VAL_HUM)) / 100;
    SENSORS_DEACTIVATE(sht25);
    return true;
}

bool ms_get_batt(float *batt) {
    SENSORS_ACTIVATE(battery_sensor);
    *batt = (float) battery_sensor.value(0);
    SENSORS_DEACTIVATE(battery_sensor);
    return true;
}

bool ms_get_time(uint32_t *seconds) {
    *seconds = (uint32_t) clock_seconds();
    return true;
}

bool ms_set_time(uint32_t seconds) {
    clock_set_seconds((unsigned long) seconds);
    return true;
}

bool ms_get_acc(int32_t *x, int32_t *y, int32_t *z) {
    *x = get_acc(X_AXIS);
    *y = get_acc(Z_AXIS);
    *z = get_acc(Z_AXIS);
    return true;
}

int16_t get_acc(enum ADXL345_AXIS axis) {
    i2c_enable();
    int16_t acc = accm_read_axis(axis);
    i2c_disable();
    return acc;
}

bool ms_get_adc1(uint32_t *adc1) {
    return false;
}

bool ms_get_adc2(uint32_t *adc2) {
    return false;
}

bool ms_get_rain(uint32_t *rain) {
    return false;
}
