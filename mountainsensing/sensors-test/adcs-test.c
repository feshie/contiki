/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark of Advancare,SL
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

/**
 * \file
 *         Testing the Potentiometer in Zolertia Z1 Starter Platform.
 * \author
 *         Enric M. Calvo <ecalvo@zolertia.com>
 */

#include "contiki.h"
#include "dev/adc1-sensor.h"
#include "dev/adc2-sensor.h"
#include "dev/batv-sensor.h"
#include "dev/ms1-io.h"
#include "dev/leds.h"
#include <stdio.h>		


static rtimer_clock_t t0;
//#define delay (RTIMER_SECOND * 2)
uint32_t delay =((uint32_t)RTIMER_SECOND);

/*---------------------------------------------------------------------------*/
PROCESS(test_adc_process, "Testing ADC  measurementin Z1 Feshie");
AUTOSTART_PROCESSES(&test_adc_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_adc_process, ev, data)
{

  PROCESS_BEGIN();
  ms1_io_init();
  ms1_sense_on();
  leds_on(LEDS_GREEN);
  while(1) {
    SENSORS_ACTIVATE(adc1_sensor);
    uint16_t value = adc1_sensor.value(0);
    printf("%i ", value);
    SENSORS_DEACTIVATE(adc1_sensor);
    SENSORS_ACTIVATE(adc2_sensor);
    uint16_t value2 = adc2_sensor.value(0);
    printf("%i ", value2);
    SENSORS_DEACTIVATE(adc2_sensor);
    SENSORS_ACTIVATE(batv_sensor);
    uint16_t bat_value = batv_sensor.value(0);
    printf("%i\n", bat_value);
    SENSORS_DEACTIVATE(batv_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + delay)));
  }


  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

