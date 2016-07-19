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
#include "net/ipv6/uip-ds6.h"
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

    printf("+++SERIALDUMP+++NODEID+++\n");
    {
        uip_ds6_addr_t *lladdr = uip_ds6_get_link_local(-1);
        printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               lladdr->ipaddr.u8[8],
               lladdr->ipaddr.u8[9],
               lladdr->ipaddr.u8[10],
               lladdr->ipaddr.u8[11],
               lladdr->ipaddr.u8[12],
               lladdr->ipaddr.u8[13],
               lladdr->ipaddr.u8[14],
               lladdr->ipaddr.u8[15]
        );
    }

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
