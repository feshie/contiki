/**
 * \file
 *          A sniffer implementation for RPL 6LoWPAN networks.
 * \author
 *          Ed Crampin      <ec6g13@soton.ac.uk>
 */
#include <stdbool.h>
#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "dev/leds.h"

void sniffer_callback()
{
	static int packet_length;
	static uint8_t buffer[176];

	packetbuf_copyto(buffer);
	packet_length = packetbuf_totlen();

	if(packet_length > 0) {
		leds_on(LEDS_RED);
		slip_write(buffer, packet_length);
    	leds_off(LEDS_RED);
	}
}

PROCESS(sniffer_process, "RPL Sniffer");

AUTOSTART_PROCESSES(&sniffer_process);

PROCESS_THREAD(sniffer_process, ev, data) {
    PROCESS_BEGIN();

    while (true) {
        PROCESS_YIELD();
    }

    PROCESS_END();
}
