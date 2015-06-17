#ifndef Z1_COAP_STORE_H
#define Z1_COAP_STORE_H

/**
 * Thread safe Convenience layer for storing readings and the config.
 * Implements locking, and SPI sharing with the CC1120 driver.
 */

#include "contiki.h"
#include <stdint.h>
#include <stdbool.h>
#include "settings.pb.h"
#include "readings.pb.h"

PROCESS_NAME(store_process);

/**
 * NOTE: TODO - these functions will modify the data passed to them on failure.
 * Callers are responsible for only passing copies of it in if they want to reuse it once the call returns false.
 */

/**
 * Store reading in the flash.
 * Returns:
 *  a unique id for the reading on success
 *  -1 if the flash is full
 */
int16_t store_save_sample(Sample *sample);

/**
 * Get a given reading from flash.
 */
bool store_get_sample(int16_t id, Sample *sample);

bool store_get_latest_sample(Sample *sample);

bool store_delete_sample(int16_t id);

bool store_save_config(SensorConfig *config);

bool store_get_config(SensorConfig *config);


#endif // ifndef Z1_COAP_STORE_H
