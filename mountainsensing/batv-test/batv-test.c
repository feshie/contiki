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
	static float bat_val;
	
	PROCESS_BEGIN();

	ms_init();
	ms_sense_on();
	
    while (true) {
		ms_get_batt(&bat_val);
		float voltage = bat_val;
		
		float f1 = bat_val/1698;
		int d1 = f1;
		float f2 = f1 - d1;
		int d2 = trunc(f2 * 10000);
	
		
		printf("Batv Value: %f\n", bat_val);
		
		printf("Batv Value: %d.%d\n", d1, d2);

		printf("Approx Voltage: %ld.%02d\n\n", (long)voltage, (int)((voltage - floor2(voltage))*100) );
		
        etimer_set(&sample_timer, CLOCK_SECOND * 10);
	
		PROCESS_WAIT_EVENT();
    }

    PROCESS_END();
}
