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
#include "event-sensor.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
#include "board.h"
#include <stdio.h>

#define DEBOUNCE_SECS (1 / 4)

// Measure events for the rain
#define EVENT_PORT RAIN_INT_PORT
#define EVENT_PIN RAIN_INT_PIN
#define EVENT_VECTOR RAIN_INT_VECTOR

const struct sensors_sensor event_sensor;

static struct timer debounce_timer;

static int event_count;

static int status(int type);

static void event_isr(uint8_t port, uint8_t pin) {
    if (timer_expired(&debounce_timer)) {
        timer_set(&debounce_timer, CLOCK_SECOND); // * DEBOUNCE_SECS);
        event_count++;
        sensors_changed(&event_sensor);
    }
}

static int value(int reset) {
    int curr_event_count = event_count;

    // Reset the event counter if requested
    if (reset) {
        event_count = 0;
    }

    return curr_event_count;
}

static int configure(int type, int c) {
    switch (type) {

        case SENSORS_ACTIVE:
            // If enabling has been requested, and we're not already active
            if (c && !status(SENSORS_ACTIVE)) {

				// Set up a rising edge interrupt
				GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));
				GPIO_SET_INPUT(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));
				GPIO_DETECT_EDGE(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));
				GPIO_TRIGGER_SINGLE_EDGE(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));
				GPIO_DETECT_RISING(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));

                // Disable any overrides enabling pull up / downs, or output
				ioc_set_over(EVENT_PORT, EVENT_PIN, IOC_OVERRIDE_DIS);

				gpio_register_callback(&event_isr, EVENT_PORT, EVENT_PIN);

                // Set the timer to 0 so it is already expired on the first ISR
                timer_set(&debounce_timer, 0);
                event_count = 0;

                // Enable the interupt
				GPIO_ENABLE_INTERRUPT(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));
				nvic_interrupt_enable(EVENT_VECTOR);

            } else {
                GPIO_DISABLE_INTERRUPT(GPIO_PORT_TO_BASE(EVENT_PORT), GPIO_PIN_MASK(EVENT_PIN));
            }

            return 1;

        default:
            return 0;
    }
}

static int status(int type) {
    switch (type) {
        case SENSORS_ACTIVE:
        case SENSORS_READY:
            // If the interrupt is enabled, we're active
			return (REG(GPIO_PORT_TO_BASE(EVENT_PORT) + GPIO_IE) & GPIO_PIN_MASK(EVENT_PIN));
        default:
            return 0;
    }
}

SENSORS_SENSOR(event_sensor, "Event Count", value, configure, status);
