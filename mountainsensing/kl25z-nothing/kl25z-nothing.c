/**
 * \file
 *  Dummy contiki application that does nothing. Literally.
 *  Might be useful for relay only nodes.
 */
#include <stdbool.h>
#include "contiki.h"
#include <stdio.h>
#include "dev/leds.h"

#define SEND_INTERVAL		10 * CLOCK_SECOND

PROCESS(do_nothing_process, "Process that does nothing");

AUTOSTART_PROCESSES(&do_nothing_process);

PROCESS_THREAD(do_nothing_process, ev, data) {
    static struct etimer et;
	static uint8_t blipcounter;
		
	PROCESS_BEGIN();
	
	blipcounter = 0;
	etimer_set(&et, SEND_INTERVAL);

    while (true) {
        PROCESS_YIELD();
		
		if(etimer_expired(&et)) {
			//leds_off(LEDS_BLUE);
			//leds_off(LEDS_GREEN);
			//leds_off(LEDS_RED);
			
			if(blipcounter % 2)
			{
				//leds_on(LEDS_RED);
			} else {
				//leds_on(LEDS_GREEN);
			}
			
			if(!(blipcounter % 3))
			{
				//leds_on(LEDS_BLUE);
			} 
			
			printf("Blip: %d. Clock: %lu. Sec: %lu\n\r", blipcounter++, clock_time(), clock_seconds());
			
			etimer_restart(&et);
		}
		
    }

    PROCESS_END();
}
