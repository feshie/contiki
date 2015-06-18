/**
 * @file
 * Thread safe Convenience layer for storing readings and the config.
 * Implements locking, and SPI sharing with the CC1120 driver.
 * The store runs in it's own thread, and synchronously handles events.
 * Calling these functions WILL cause the curent thread to be blocked.
 *
 * Every sample is assigned a unique id for it's lifetime on flash.
 * The id of a sample may be reused once it has been deleted.
 *
 * NOTE: These functions will modify the pointer passed to them on failure.
 * Callers are responsible for only passing copies of it in if they want to reuse it should the call returns false.
 *
 * @author Arthur Fabre <af1g12@ecs.soton.ac.uk>
 */

#ifndef Z1_COAP_STORE_H
#define Z1_COAP_STORE_H

#include "contiki.h"
#include <stdint.h>
#include <stdbool.h>
#include "settings.pb.h"
#include "readings.pb.h"

PROCESS_NAME(store_process);

/**
 * Store a sample in the flash.
 * @param *sample The sample to save.
 * @return A unique id for the sample on success. -1 on failure.
 * NOTE: On failure, *sample will be overwritten.
 */
int16_t store_save_sample(Sample *sample);

/**
 * Get a given sample from the flash,
 * in the form of an encoded protocol buffer.
 * @param id The id of the sample.
 * @param *buffer The buffer to write the protocol buffer to. Should be at least Sample sized.
 * @return `true` on success, `false` otherwise.
 * NOTE: On failure, *buffer will be overwritten.
 */
bool store_get_raw_sample(int16_t id, uint8_t *buffer);

/**
 * Get the most recent sample from the flash,
 * in the form of an encoded protocol buffer.
 * @param *buffer The buffer to write the protocol buffer to. Should be at least Sample sized.
 * @return `true` on success, `false` otherwise.
 * NOTE: On failure, *buffer will be overwritten.
 */
bool store_get_latest_raw_sample(uint8_t *buffer);

/**
 * Delete a given sample from the flash.
 * @param *id The id of the sample to delete.
 * @return `true` on success, `false` otherwise.
 * NOTE: On failure, *id will be overwritten.
 */
bool store_delete_sample(int16_t *id);

/**
 * Save the configuration to flash.
 * @param *config The configuration to save.
 * @return `true` on success, `false` otherwise.
 * NOTE: On failure, *config will be overwritten.
 */
bool store_save_config(SensorConfig *config);

/**
 * Get the configuration from the flash.
 * @param *config. The configuration to write to.
 * @return `true` on success, `false` otherwise.
 * NOTE: On failure, *config will be overwritten.
 */
bool store_get_config(SensorConfig *config);

#endif // ifndef Z1_COAP_STORE_H
