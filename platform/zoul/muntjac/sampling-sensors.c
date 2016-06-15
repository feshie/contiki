#include "contiki.h"
#include "sampling-sensors.h"
#include "sampler.h"

#define DEBUG_ON
#include "debug.h"

void sampler_init(void) {
}

float sampler_get_temp(void) {
    return 12345;
}

float sampler_get_batt(void) {
    return 12345;
}

int16_t sampler_get_acc_x(void) {
    return 12345;
}

int16_t sampler_get_acc_y(void) {
    return 12345;
}

int16_t sampler_get_acc_z(void) {
    return 12345;
}

uint32_t sampler_get_time(void) {
    return 12345;
}

bool sampler_set_time(uint32_t seconds) {
    return true;
}

bool sampler_get_extra(Sample *sample, SensorConfig *config) {
    return true;
}
