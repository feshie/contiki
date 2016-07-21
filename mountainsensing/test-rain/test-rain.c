/**
 * \file
 *  Dummy contiki application that des nothing. Literally.
 *  Might be useful for relay only nodes.
 */
#include <stdbool.h>
#include <stdio.h>	
#include <math.h>
#include "contiki.h"
#include "ms-io.h"

float floor2(float x){
  if(x>=0.0f) return (float) ((int)x);
  else        return (float) ((int)x-1);
}

PROCESS(do_nothing_process, "Process that does nothing");

AUTOSTART_PROCESSES(&do_nothing_process);

PROCESS_THREAD(do_nothing_process, ev, data) {
    static struct etimer sample_timer;
	static uint32_t rain_val;
	
	PROCESS_BEGIN();

	ms_init();
	ms_sense_on();
	
    while (true) {
		
		ms_get_rain(&rain_val);
		
		printf("Rain Count: %ld\n\n", rain_val);
		
        etimer_set(&sample_timer, CLOCK_SECOND * 10);
	
		PROCESS_WAIT_EVENT();
    }

    PROCESS_END();
}
