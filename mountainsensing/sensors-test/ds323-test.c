#include <stdio.h>
#include "contiki.h"
#include "dev/i2cmaster.h"  // Include IC driver
#include "dev/ds3231-sensor.h"     // Include sensor driver
 
#define TMP102_READ_INTERVAL (CLOCK_SECOND/2)  // Poll the sensor every 500 ms
 
PROCESS (temp_process, "Test Temperature process");
AUTOSTART_PROCESSES (&temp_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;


static uint32_t
get_time(void)
{
  uint32_t time;
  time = (uint32_t)ds3231_sensor.value(DS3231_SENSOR_GET_EPOCH_SECONDS_MSB) << 16;
  time |= (uint32_t)ds3231_sensor.value(DS3231_SENSOR_GET_EPOCH_SECONDS_LSB);
  return(time);
}

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
 
	printf ("RTC = %lu\n", ds3231_get_epoch_seconds());
      }
  }
  PROCESS_END ();
}
