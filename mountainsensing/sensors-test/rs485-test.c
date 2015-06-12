#include <stdio.h>
#include "contiki.h"
#include "dev/i2cmaster.h"  // Include IC driver
 
#define TMP102_READ_INTERVAL (CLOCK_SECOND/2)  // Poll the sensor every 500 ms
 
PROCESS (temp_process, "Test Temperature process");
AUTOSTART_PROCESSES (&temp_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;

PROCESS_THREAD (temp_process, ev, data)
{
  PROCESS_BEGIN ();

  P4DIR|=0x00000001;
  P4OUT|=0x00000001;
  // turn on pin 4.0 (vsense control)
  uart1_init(0);
  {
    while (1)
      {
        etimer_set(&et, TMP102_READ_INTERVAL);          // Set the timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  // wait for its expiration
 
	uart1_writearray((unsigned char*)"look sending",12);
	printf ("sent");
	
      }
  }
  PROCESS_END ();
}
