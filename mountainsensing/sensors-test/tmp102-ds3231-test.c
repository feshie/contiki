/* test both temp sensors on a feshie node - tmp103 and RTC one
* rtc returned as 100 * C
* Kirk June 2015
*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2cmaster.h"  // Include IC driver
#include "dev/tmp102.h"     // Include sensor driver
#include "dev/ds3231-sensor.h"		// RTC contains a T sensor
   // Poll the sensor second
#define READ_INTERVAL (CLOCK_SECOND)
 
float floor(float x){
  if(x>=0.0f) return (float) ((int)x);
  else        return (float) ((int)x-1);
}

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
	int rtctemp;
 
    tmp102_init();
 
    while (1)
      {
        etimer_set(&et, READ_INTERVAL);          // Set the timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  // wait for its expiration
// actually that timer seems to do nothing at the moment wierdly
 
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
	rtctemp = ds3231_temperature();

	printf ("Temp102 = %c%d.%04d rtc %d\n", minus, tempint, tempfrac,rtctemp);

	//printf ("Temp102 = %c%d.%04d rtc %d.%d\n", minus, tempint, tempfrac,
//(int)(rtctemp/100), (rtctemp - floor(rtctemp))*100 );

      }
  }
  PROCESS_END ();
}
