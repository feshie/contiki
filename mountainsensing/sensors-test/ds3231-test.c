/* test rtc temp sensor
*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2cmaster.h"  // Include IC driver
#include "dev/ds3231-sensor.h"		// RTC contains a T sensor
 
// temp sensor on rtc only updates every 64s
#define READ_INTERVAL (CLOCK_SECOND)  

static struct etimer et;

static uint32_t
get_time(void)
{
  uint32_t time;
  time = (uint32_t)ds3231_sensor.value(DS3231_SENSOR_GET_EPOCH_SECONDS_MSB) << 1;
  time |= (uint32_t)ds3231_sensor.value(DS3231_SENSOR_GET_EPOCH_SECONDS_LSB);
  return(time);
}

PROCESS (kirks_process, "ds3231 Temp process");
AUTOSTART_PROCESSES (&kirks_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
 
PROCESS_THREAD (kirks_process, ev, data)
{
  PROCESS_BEGIN ();
 
  {
	int rtctemp;
 
 
    while (1)
      {
        etimer_set(&et, READ_INTERVAL);          // Set the timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  // wait for its expiration
	if( ev == PROCESS_EVENT_TIMER) {
		rtctemp = ds3231_temperature();
		printf("rtc temp * 100 %d\n",rtctemp);
		}
      }
  }
  PROCESS_END ();
}
