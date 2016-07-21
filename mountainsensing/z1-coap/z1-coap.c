/**
 * \file
 *          Sampler for the mountain-sensing platform.
 *
 *          This intializes the store, and starts the er_server, and the sampler.
 *
 * \author
 *          Dan Playle      <djap1g12@soton.ac.uk>
 *          Philip Basford  <pjb@ecs.soton.ac.uk>
 *          Graeme Bragg    <gmb1g08@ecs.soton.ac.uk>
 *          Tyler Ward      <tw16g08@ecs.soton.ac.uk>
 *          Kirk Martinez   <km@ecs.soton.ac.uk>
 *          Arthur Fabre    <af1g12@ecs.soton.ac.uk>
 */

#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include "z1-coap.h"
#include "contiki-conf.h"

#include "store.h"
#include "sampler.h"
#include "er-server.h"

PROCESS(feshie_sense_process, "Feshie Sense");

AUTOSTART_PROCESSES(&feshie_sense_process);

PROCESS_THREAD(feshie_sense_process, ev, data) {
    PROCESS_BEGIN();

    // Initialize the store before anything els
    store_init();

    process_start(&sample_process, NULL);

    er_server_init();

    PROCESS_END();
}
