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

// Routing mode
#include "net/rpl/rpl.h"

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
 * True if we should reload our config, false otherwise.
 * Intitially set to true to load our config on start.
 */
static bool should_refresh_config = true;

/**
 * The current sensor config being used.
 */
static SensorConfig sensor_config;

/**
 * Asynchronous event sent by the AVR Handler when it receives data fro man AVR sensor.
 */
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
    static struct etimer avr_timeout_timer;
    static uint8_t avr_recieved;
    static uint8_t avr_retry_count;
    static int16_t id;
    static uint8_t j;

    PROCESS_BEGIN();

    avr_recieved = 0;
    avr_retry_count = 0;

    protobuf_event = process_alloc_event();
    protobuf_register_process_callback(&sample_process, protobuf_event) ;

#ifdef SENSE_ON
    ms1_sense_on();
    DEBUG("Sensor power permanently on\n");
#endif

    SENSORS_ACTIVATE(event_sensor);

    while(true) {
        // Reload our config if we need to
        if (should_refresh_config) {
            refresh_config();
            should_refresh_config = false;

            DEBUG("Refreshed Sensor config to:\n");
            print_sensor_config(&sensor_config);
        }

        etimer_set(&sample_timer, CLOCK_SECOND * (sensor_config.interval - (get_time() % sensor_config.interval)));
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sample_timer));

        if (ev == PROCESS_EVENT_TIMER) {

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

            if (sensor_config.hasADC1) {
                sample.has_ADC1 = true;
                sample.ADC1 = get_sensor_ADC1();
            }

            if (sensor_config.hasADC2) {
                sample.has_ADC2 = true;
                sample.ADC2 = get_sensor_ADC2();
            }

            if (sensor_config.hasRain) {
                sample.has_rain = true;
                sample.rain = get_sensor_rain();
            }

            DEBUG("Sampling from %d AVRs\n", sensor_config.avrIDs_count);
            for (i = 0; i < sensor_config.avrIDs_count; i++) {
                avr_id = sensor_config.avrIDs[i] & 0xFF; //only use 8bits
                DEBUG("AVR ID: %d\n", avr_id);
                avr_recieved = 0;
                avr_retry_count = 0;

                do {
                    protobuf_send_message(avr_id, PROTBUF_OPCODE_GET_DATA, NULL, (int)NULL);
                    DEBUG("Retry %d\n", avr_retry_count);

                    etimer_set(&avr_timeout_timer, CLOCK_SECOND * AVR_TIMEOUT_SECONDS);

                    DEBUG("Yielding for timeout\n");

                    PROCESS_WAIT_EVENT_UNTIL(ev == protobuf_event || etimer_expired(&avr_timeout_timer));

                    if (ev == PROCESS_EVENT_TIMER) {
                        DEBUG("AVR Timeout Reached!\n");
                        avr_retry_count++;

                    } else if (ev == protobuf_event) {

                        if (data != NULL) {
                            DEBUG("\tavr data recieved on retry %d\n", avr_retry_count);
                            protobuf_data_t *pbd;
                            pbd = data;
                            sample.has_AVR = 1;
                            sample.AVR.size = pbd->length;
                            for(j=0; j < pbd->length; j++){
                                sample.AVR.bytes[j] = pbd->data[j];
                            }
                            DEBUG("\tRecieved %d bytes\t", pbd->length);
                            for(j = 0; j < pbd->length; j++) {
                                DEBUG("%d:", pbd->data[j]);
                            }
                            DEBUG("\n");
                            //process data
                            if (avr_id < 0x10) {
                                //it's a temp accel chain and so needs to be read twice to get valid data
                                //otherwise just once is ok
                                // It will break out of this loop when avr_recieved == 2
                                avr_recieved++;
                            } else {
                                avr_recieved = 2;
                            }
                        }
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

            continue;
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

    // Set the RPL mode. Doesn't really belong here, but oh well
    DEBUG("Setting RPL mode to %d\n", sensor_config.routingMode);
    rpl_set_mode(sensor_config.routingMode);
}

void print_sensor_config(SensorConfig *conf) {
    static uint8_t i;
    DEBUG("\tInterval = %d\n", (unsigned int)conf->interval);

    DEBUG("\tADC1: %s\n", conf->hasADC1 ? "yes" : "no");
    DEBUG("\tADC2: %s\n", conf->hasADC2 ? "yes" : "no");
    DEBUG("\tRain: %s\n", conf->hasRain ? "yes" : "no");

    DEBUG("\t%d AVRs\n", conf->avrIDs_count);
    for (i = 0; i < conf->avrIDs_count; i++) {
        // uint32_t is not necessarilly an unsigned int. Cast it to an int, and mask out the sign bits.
        DEBUG("\t\t AVR %d: %02x\n", i, (int)conf->avrIDs[i] & 0xFF);
    }

    DEBUG("\tRoutingMode: %d\n", conf->routingMode);
}

void sampler_refresh_config(void) {
    DEBUG("Config marked for refresh!\n");
    should_refresh_config = true;
}
