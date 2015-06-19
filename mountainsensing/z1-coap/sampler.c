#include "sampler.h"

#include "contiki-conf.h"
#include <stdio.h>

#include "store.h"

// Sensors
#include "sampling-sensors.h"
#include "ms1-io.h"

// Config
#include "settings.pb.h"
#include "readings.pb.h"

#include "z1-coap-config-defaults.h"

#include "dev/temperature-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/protobuf-handler.h"
#include "dev/event-sensor.h"
#include "platform-conf.h"
#include <stdbool.h>

#define DEBUG_ON
#include "debug.h"

PROCESS(sample_process, "Sample Process");

/**
 * Timeout for avr sensors in seconds.
 */
#define AVR_TIMEOUT_SECONDS 10

/**
 * If defined, do not turn sensor power off
 */
//#define SENSE_ON

/**
 * Event to tell the sampler process to reload the config.
 * The data pointer is not used.
 */
#define SAMPLER_EVENT_REFRESH_CONFIG  1

/**
 * The current sensor config being used.
 */
static SensorConfig sensor_config;

static process_event_t protobuf_event;

/**
 * Get and set the latest config from the Store.
 * If no config exists, set it to DEFAULT_CONFIG.
 */
static void refresh_config(void);

/**
 * Print the sensor config for debugging.
 */
static void print_sensor_config(SensorConfig *conf);


PROCESS_THREAD(sample_process, ev, data) {
    static struct etimer sample_timer;
    static Sample sample;
    static int i;
    static uint8_t avr_id;
    static struct ctimer avr_timeout_timer;
    static uint8_t avr_recieved;
    static uint8_t avr_retry_count;
    static int16_t id;

    PROCESS_BEGIN();

    refresh_config();
    avr_recieved = 0;
    avr_retry_count = 0;

    protobuf_event = process_alloc_event();
    protobuf_register_process_callback(&sample_process, protobuf_event) ;
    DEBUG("Refreshed Sensor config to:\n");
    print_sensor_config(&sensor_config);

#ifdef SENSE_ON
    ms1_sense_on();
    DEBUG("Sensor power permanently on\n");
#endif

    SENSORS_ACTIVATE(event_sensor);

    while(true) {
        etimer_set(&sample_timer, CLOCK_SECOND * (sensor_config.interval - (get_time() % sensor_config.interval)));
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sample_timer));

        switch(ev) {
            case SAMPLER_EVENT_REFRESH_CONFIG:
                refresh_config();
                break;

            case PROCESS_EVENT_TIMER:

                ms1_sense_on();
                sample.time = get_time();

                sample.batt = get_sensor_batt();
                sample.has_batt = true;

                SENSORS_ACTIVATE(temperature_sensor);
                sample.temp = get_sensor_temp();
                sample.has_temp = true;
                SENSORS_DEACTIVATE(temperature_sensor);

                sample.accX = get_sensor_acc_x();
                sample.accY = get_sensor_acc_y();
                sample.accZ = get_sensor_acc_z();

                sample.has_accX = true;
                sample.has_accY = true;
                sample.has_accZ = true;

                if(sensor_config.hasADC1) {
                    sample.has_ADC1 = 1;
                    sample.ADC1 = get_sensor_ADC1();
                }
                if(sensor_config.hasADC2) {

                    sample.has_ADC2 = true;
                    sample.ADC2 = get_sensor_ADC2();
                }
                if(sensor_config.hasRain) {
                    sample.has_rain = true;
                    sample.rain = get_sensor_rain();
                }

                DEBUG("Sampling from %d AVRs\n", sensor_config.avrIDs_count);
                for(i = 0; i < sensor_config.avrIDs_count; i++) {
                    avr_id = sensor_config.avrIDs[i] & 0xFF; //only use 8bits
                    DEBUG("AVR ID: %d\n", avr_id);
                    avr_recieved = 0;
                    avr_retry_count = 0;
                    data = NULL;
                    do {
                        protobuf_send_message(avr_id, PROTBUF_OPCODE_GET_DATA, NULL, (int)NULL);
                        DEBUG("Sent message %d\n", i);
                        i = i + 1;
                        ctimer_set(&avr_timeout_timer, CLOCK_SECOND * AVR_TIMEOUT_SECONDS, avr_timer_handler, NULL);
                        PROCESS_YIELD_UNTIL(ev == protobuf_event);
                        if(data != NULL) {
                            DEBUG("\tavr data recieved on retry %d\n", avr_retry_count);
                            ctimer_stop(&avr_timeout_timer);
                            protobuf_data_t *pbd;
                            pbd = data;
                            sample.has_AVR = 1;
                            sample.AVR.size = pbd->length;
                            static uint8_t k;
                            for(k=0; k < pbd->length; k++){
                                sample.AVR.bytes[k] = pbd->data[k];
                            }
#ifdef AVRDEFBUG
                            DEBUG("\tRecieved %d bytes\t", pbd->length);
                            static uint8_t j;
                            for(j = 0; j < pbd->length; j++) {
                                DEBUG("%d:", pbd->data[j]);
                            }
                            DEBUG("\n");
#endif
                            //process data
                            if (avr_id < 0x10) {
                                //it's a temp accel chain and so needs to be read twice to get valid data
                                //otherwise just once is ok
                                // It will break out of this loop when avr_recieved == 2
                                avr_recieved++;
                            } else {
                                avr_recieved = 2;
                            }
                        } else {
                            DEBUG("AVR timedout\n");
                            avr_retry_count++;
                        }
                        DEBUG("avr_recieved = %d\n", avr_recieved);
                    } while(avr_recieved < 2 && avr_retry_count < PROTOBUF_RETRIES);
                }
#ifndef SENSE_ON
                ms1_sense_off();
#endif

                id = store_save_sample(&sample);

                if (id < 0) {
                    DEBUG("Failed to save sample!\n");
                } else {
                    DEBUG("Sample saved with id %d\n", id);
                }

                break;
        }
    }
    PROCESS_END();
}

static void refresh_config(void) {
    if (!store_get_config(&sensor_config)) {
        // Config file does not exist! Use default and set file
      	DEBUG("No Sensor config found\n");
        sensor_config = SENSOR_DEFAULT_CONFIG;
        store_save_config(&sensor_config);
    } else {
      	DEBUG("Sensor config loaded\n");
    }
}

void print_sensor_config(SensorConfig *conf) {
    static uint8_t i;
    DEBUG("\tInterval = %d\n", (unsigned int)conf->interval);

    DEBUG("\tADC1: %s\n", conf->hasADC1 ? "yes" : "no");
    DEBUG("\tADC2: %s\n", conf->hasADC2 ? "yes" : "no");
    DEBUG("\tRain: %s\n", conf->hasRain ? "yes" : "no");

    DEBUG("\t%d AVRs\n", conf->avrIDs_count);
    for (i = 0; i < conf->avrIDs_count; i++) {
        DEBUG("\t\t AVR %d: %02x\n", i, (int)conf->avrIDs[i] & 0xFF);
    }
}

void sampler_refresh_config(void) {
    process_post(&sample_process, SAMPLER_EVENT_REFRESH_CONFIG, NULL);
}

void avr_timer_handler(void *p) {
	process_post(&sample_process, protobuf_event, (process_data_t)NULL);
}
