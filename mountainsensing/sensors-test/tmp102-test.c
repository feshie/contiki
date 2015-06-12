#include <stdio.h>
#include "contiki.h"
#include "dev/i2cmaster.h"  // Include IC driver
#include "dev/tmp102.h"     // Include sensor driver
 
#define TMP102_READ_INTERVAL (CLOCK_SECOND/2)  // Poll the sensor every 500 ms
 
PROCESS (temp_process, "Test Temperature process");
AUTOSTART_PROCESSES (&temp_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
 
PROCESS_THREAD (temp_process, ev, data)
{
  PROCESS_BEGIN ();
 
  {
    int16_t  tempint;
    uint16_t tempfrac;
    int16_t  raw;
    uint16_t absraw;
    int16_t  sign;
    char     minus = ' ';
 
    tmp102_init();
 
    while (1)
      {
        etimer_set(&et, TMP102_READ_INTERVAL);          // Set the timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  // wait for its expiration
 
        sign = 1;
 
	raw = tmp102_read_temp_raw();  // Reading from the sensor
 
        absraw = raw;
        if (raw < 0) { // Perform 2C's if sensor returned negative data
          absraw = (raw ^ 0xFFFF) + 1;
          sign = -1;
        }
	tempint  = (absraw >> 8) * sign;
        tempfrac = ((absraw>>4) % 16) * 625; // Info in 1/10000 of degree
        minus = ((tempint == 0) & (sign == -1)) ? '-'  : ' ' ;
	printf ("Temp = %c%d.%04d\n", minus, tempint, tempfrac);
      }
  }
  PROCESS_END ();
}
