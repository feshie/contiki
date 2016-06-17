#ifndef __RESET_SENSOR_H__
#define __RESET_SENSOR_H__

#include <stdint.h>
#include "lib/sensors.h"
#include "dev/flash.h"

/**
 * Bytes used by the reset-sensor.
 * Can be used to reserve space in the flash
 */
#define RESET_SENSOR_SIZE FLASH_PAGE_SIZE

extern const struct sensors_sensor reset_sensor;

void reset_counter_reset();

#endif /* __RESET_SENSOR_H__ */
