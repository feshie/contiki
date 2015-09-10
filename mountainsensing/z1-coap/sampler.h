/**
 * @file
 * Process that periodically takes samples from onboard sensors and stores them in flash.
 * The sensors to sample as well as the sampling interval are defined by the SampleConfig stored in the Store.
 * The sampler can be instrcuted to reload it from the Store by calling sampler_refresh_config()
 *
 * @author
 *      Dan Playle      <djap1g12@soton.ac.uk>
 *      Philip Basford  <pjb@ecs.soton.ac.uk>
 *      Graeme Bragg    <gmb1g08@ecs.soton.ac.uk>
 *      Tyler Ward      <tw16g08@ecs.soton.ac.uk>
 *      Kirk Martinez   <km@ecs.soton.ac.uk>
 *      Arthur Fabre    <af1g12@ecs.soton.ac.uk>
 */

#ifndef SAMPLER_H
#define SAMPLER_H

#include "contiki.h"

/**
 * Process the sampler runs as.
 */
PROCESS_NAME(sample_process);

/**
 * Get the Sampler to reload it's config from flash.
 * This is a non blocking asynchronous call.
 */
void sampler_refresh_config(void);

/**
 * Notify the sampler that an asynchronous request as part of the sampler_get_extra
 * interface has been completed (whether it was succesfull or not).
 */
void sampler_extra_performed(void);

#endif // ifndef SAMPLER_H
