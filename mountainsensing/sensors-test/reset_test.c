#include <stdio.h>
#include "contiki.h"
#include "dev/i2cmaster.h"  // Include IC driver
#include "dev/reset-sensor.h"     // Include sensor driver
 
#define TMP102_READ_INTERVAL (CLOCK_SECOND/2)  // Poll the sensor every 500 ms
 
PROCESS (temp_process, "Test Event process");
AUTOSTART_PROCESSES (&temp_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;

PROCESS_THREAD (temp_process, ev, data)
{
  PROCESS_BEGIN ();
 
  {
 
    while (1)
      {
        etimer_set(&et, TMP102_READ_INTERVAL);          // Set the timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  // wait for its expiration
 
	printf ("COUNT = %d\n", reset_sensor.value(0));
      }
  }
  PROCESS_END ();
}
