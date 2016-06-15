#ifndef __RESET_SENSOR_H__
#define __RESET_SENSOR_H__

#include "lib/sensors.h"

extern const struct sensors_sensor reset_sensor;

void reset_counter_reset();

#endif /* __RESET_SENSOR_H__ */
