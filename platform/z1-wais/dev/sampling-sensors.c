#include "clock.h"
#include "sampling-sensors.h"
#include "dev/sht25.h"
#include "dev/battery-sensor.h"
#include "adxl345.h"

#define DEBUG_ON
#include "debug.h"

void sampler_init(void) {
    // We don't need to do anything
}

float sampler_get_temp(void) {
    float temp;
    SENSORS_ACTIVATE(sht25);
    temp = ((float) sht25.value(SHT25_VAL_TEMP)) / 100;
    SENSORS_DEACTIVATE(sht25);
    return temp;
}

float sampler_get_batt(void) {
    float bat_ret;
    SENSORS_ACTIVATE(battery_sensor);
    bat_ret =  (float) battery_sensor.value(0);
    SENSORS_DEACTIVATE(battery_sensor);
    return bat_ret;
}

int16_t sampler_get_acc_x(void) {
    return accm_read_axis(X_AXIS);
}

int16_t sampler_get_acc_y(void) {
    return accm_read_axis(Y_AXIS);
}

int16_t sampler_get_acc_z(void) {
    return accm_read_axis(Z_AXIS);
}

uint32_t sampler_get_time(void) {
    return (uint32_t) clock_seconds();
}

bool sampler_set_time(uint32_t seconds) {
    clock_set_seconds((unsigned long) seconds);
    return true;
}

bool sampler_get_extra(Sample *sample, SensorConfig *config) {
    SENSORS_ACTIVATE(sht25);
    sample->humid = ((float) sht25.value(SHT25_VAL_HUM)) / 100;
    sample->has_humid = true;
    SENSORS_DEACTIVATE(sht25);
    return true;
}
