/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/hwconf.h"
#include "dev/button-sensor.h"
#include "isr_compat.h"

const struct sensors_sensor event_sensor;	//event sensor structure
const struct sensors_sensor button_sensor;	//button sensor struct

static struct timer debouncetimer;		//debounce timer for button
static struct timer debouncetimer_event;	//debounce timer for event sensor
static int status(int type);

static int eventcount;	//stores sount of rising edges revieved


HWCONF_PIN(BUTTON, 2, 5);
HWCONF_IRQ(BUTTON, 2, 5);

HWCONF_PIN(EVENT_SENSOR,2,0);			//create EVENT_SENSOR_... functions
HWCONF_IRQ(EVENT_SENSOR,2,0);			//create EVENT_SENSOR_... IRQ functions

// RAIN GUAGE IS ON P2.0
//button is on p2.5

/*---------------------------------------------------------------------------*/
/* ISR routine for irq_p2
 * 
 * evaluates of button has been pressed or an if an event has been recorded
 */
ISR(PORT2, irq_p2)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(BUTTON_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 4);
      sensors_changed(&button_sensor);
      LPM4_EXIT;
    }
  }

  if(EVENT_SENSOR_CHECK_IRQ()) {				//if the event sensoe has a rising edge interupt waiting
    if(timer_expired(&debouncetimer_event)) {			//and if the debounce perios has ended
      timer_set(&debouncetimer_event, CLOCK_SECOND / 4);	//reset debounce counter
      eventcount++;						//increase count of events
      sensors_changed(&event_sensor);				//report this change
    }
  }

  P2IFG = 0x00;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return BUTTON_READ() || !timer_expired(&debouncetimer);
}

static int
value_event(int reset)
{
  if(reset)							//if we should reset the value
  {
    int i=eventcount;						//get count of events
    eventcount=0;						//reset event counter
    return i;							//return value
  }
  else
  {
    return eventcount;						//return count of events
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch (type) {
  case SENSORS_ACTIVE:
    if (c) {
      if(!status(SENSORS_ACTIVE)) {
	timer_set(&debouncetimer, 0);
	BUTTON_IRQ_EDGE_SELECTD();

	BUTTON_SELECT();
	BUTTON_MAKE_INPUT();

	BUTTON_ENABLE_IRQ();
      }
    } else {
      BUTTON_DISABLE_IRQ();
    }
    return 1;
  }
  return 0;
}
static int
configure_event(int type, int c)
{
  switch (type) {
  case SENSORS_ACTIVE:
    if (c) {
      if(!status(SENSORS_ACTIVE)) {
	timer_set(&debouncetimer_event, 0);
	eventcount=0;					//reset event counter to 0
	EVENT_SENSOR_IRQ_EDGE_SELECTU();		//look for a rising edge

	EVENT_SENSOR_SELECT();
	EVENT_SENSOR_MAKE_INPUT();

	EVENT_SENSOR_ENABLE_IRQ();
      }
    } else {
      EVENT_SENSOR_DISABLE_IRQ();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_IRQ_ENABLED();
  }
  return 0;
}
static int
status_event(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return EVENT_SENSOR_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);
SENSORS_SENSOR(event_sensor, "Event Count", value_event,configure_event,status_event);
