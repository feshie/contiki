/**
 * Standard interface between z1-coap and the various supported platforms.
 *
 * Required readings / sensors are implemented as functions (eg get_time()),
 * whilst all optional readings should be filled in the sampler_get_extra.
 */

#ifndef SAMPLING_SENSORS_H
#define SAMPLING_SENSORS_H

#include <stdbool.h>
#include <stdint.h>
#include "settings.pb.h"
#include "readings.pb.h"

/**
 * Epoch value indicating it is not a valid epoch.
 */
#define ERROR_EPOCH 12345

/**
 * Initialize any hardware required for sampling.
 * (This does not imply anything is enabled).
 */
void sampler_init(void);

/**
 * Get the current time.
 * @param seconds Pointer to write the number of seconds since the Unix epoch. On error, ERROR_EPOCH will be written to it.
 * @return True on success, False otherwise
 *
 * @note Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
bool sampler_get_time(uint32_t *seconds);

/**
 * Set the current time.
 * @param seconds The number of seconds since the Unix epoch.
 * @return True on success, False otherwise
 *
 * @note Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
bool sampler_set_time(uint32_t seconds);

/**
 * Get any extra platform specific readings.
 * These should be directly inserted into the sample struct.
 *
 * @return True if all the operations were completed synchronously,
 * False if some operations are being completed asychronously (in this case
 * sampler_extra_performed() should be called to notify the sampler when
 * the processing is complete).
 *
 * @note: config may change once sampler_get_extra returns,
 * the config should not be used in any asynchronous operations without
 * first copying it somewhere safe.
 */
bool sampler_get_extra(Sample *sample, SensorConfig *config);

#endif // ifndef SAMPLING_SENSORS_H
