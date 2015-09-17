/**
 * @file
 * Convenience layer for storing readings and the config.
 * Implements locking, and SPI sharing with the CC1120 driver.
 *
 * Every sample is assigned a unique id for it's lifetime on flash.
 * The id of a sample may be reused once it has been deleted.
 * The store allows deleting any given sample, by gracefully dealing with files that do not exist.
 *
 * Callers are always responsible for allocating the required memory.
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

/**
 * Initialize the data store.
 * Includes finding the latest reading.
 */
void store_init(void);

/**
 * Store a sample in the flash.
 * This will also set sample's id field.
 * @param *sample The sample to save.
 * @return A unique id for the sample on success, `false` on failure.
 */
uint16_t store_save_sample(Sample *sample);

/**
 * Get a given sample from the flash,
 * @param id The id of the sample.
 * @param *sample The Sample to write the sample to.
 * @return `true` on success, `false` otherwise.
 */
bool store_get_sample(uint16_t id, Sample *sample);

/**
 * Get a given sample from the flash,
 * in the form of an encoded protocol buffer.
 * @param id The id of the sample.
 * @param buffer An allocated buffer at least Samle_size big to which the sample protocol buffer will be written.
 * @return The number of bytes written to the buffer on success, `false` otherwise.
 */
uint8_t store_get_raw_sample(uint16_t id, uint8_t buffer[Sample_size]);

/**
 * Get the most recent sample from the flash.
 * @param *sample The Sample to write the sample to.
 * @return `true` on success, `false` otherwise.
 */
bool store_get_latest_sample(Sample *sample);

/**
 * Get the most recent sample from the flash,
 * in the form of an encoded protocol buffer.
 * @param buffer An allocated buffer at last Sample_size big to which the sample protocol buffer will be written.
 * @return The number of bytes written to the buffer on success, `false` otherwise.
 */
uint8_t store_get_latest_raw_sample(uint8_t buffer[Sample_size]);

/**
 * Get the identifer of the most recent sample stored.
 * @return The identifier of the latest sample.
 */
uint16_t store_get_latest_sample_id(void);

/**
 * Delete a given sample from the flash.
 * @param id The id of the sample to delete.
 * @return `true` on success, `false` otherwise.
 */
bool store_delete_sample(uint16_t id);

/**
 * Save the configuration to flash.
 * @param *config The configuration to save.
 * @return `true` on success, `false` otherwise.
 */
bool store_save_config(SensorConfig *config);

/**
 * Get the configuration from the flash.
 * @param *config. The configuration to write to.
 * @return `true` on success, `false` otherwise.
 */
bool store_get_config(SensorConfig *config);

/**
 * Get the configuration from the flash.
 * @param An allocated buffer at least SensorConfig_size big to which the config protocol buffer will be written.
 * @return The number of bytes written to the buffer on success, `false` otherwise.
 */
uint8_t store_get_raw_config(uint8_t buffer[SensorConfig_size]);

#endif // ifndef Z1_COAP_STORE_H
