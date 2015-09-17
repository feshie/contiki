/**
 * \file
 *         adapted from Battery and Temperature IPv6 Demo for Zolertia Z1
 * \author
 *          Dan Playle      <djap1g12@soton.ac.uk>
 */

#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "store.h"
#include "readings.pb.h"
#include "settings.pb.h"

PROCESS(serial_dumper_process, "Serial Dumper");

AUTOSTART_PROCESSES(&serial_dumper_process);

/**
 * Dump a binary buffer over serial as a hex line.
 * @param buffer The buffer to print
 * @param len The length of the buffer
 */
static void dump_buffer(uint8_t *buffer, uint8_t len);

PROCESS_THREAD(serial_dumper_process, ev, data) {
    PROCESS_BEGIN();

    // Initialize the store before anything else
    store_init();

    printf("+++SERIALDUMP+++SAMPLE+++START+++\n");
    {
        uint8_t buffer[Sample_size];
        uint8_t buffer_len;
        uint16_t id;

        // Keep going as long as the ID is valid
        for (id = store_get_latest_sample_id(); id > 0; id--) {
            if ((buffer_len = store_get_raw_sample(id, buffer))) {
                dump_buffer(buffer, buffer_len);
            }
        }

    }
    printf("+++SERIALDUMP+++SAMPLE+++END+++\n");

    printf("+++SERIALDUMP+++CONFIG+++START+++\n");
    {
        uint8_t buffer[SensorConfig_size];
        uint8_t buffer_len;

        if ((buffer_len = store_get_raw_config(buffer))) {
            dump_buffer(buffer, buffer_len);
        }

    }
    printf("+++SERIALDUMP+++CONFIG+++END+++\n");

    PROCESS_END();
}

void dump_buffer(uint8_t *buffer, uint8_t len) {
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x", buffer[i]);
    }
    printf("\n");
}
