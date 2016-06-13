#include "sampler.h"

#include "contiki-conf.h"
#include <stdio.h>

#include "store.h"

// Sensors
#include "sampling-sensors.h"

// Config
#include "settings.pb.h"
#include "readings.pb.h"

#include "z1-coap-config-defaults.h"

#include "contiki-conf.h"
#include <stdbool.h>

// Routing mode
#include "net/rpl/rpl.h"

#define DEBUG_ON
#include "debug.h"

PROCESS(sample_process, "Sample Process");

/**
 * Event sent to the Sampler when extra processsing for
 * sampler_get_extra is complete.
 */
#define SAMPLER_EVENT_EXTRA_PERFORMED 1

/**
 * Event sent to the sampler when the configuration
 * should be reloaded from Flash.
 */
#define SAMPLER_EVENT_RELOAD_CONFIG 2

/**
 * The current sensor config being used.
 */
static SensorConfig sensor_config;

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
    int16_t id;

    PROCESS_BEGIN();

    refresh_config();

    sampler_init();

    while(true) {

        etimer_set(&sample_timer, CLOCK_SECOND * (sensor_config.interval - (sampler_get_time() % sensor_config.interval)));
        PROCESS_WAIT_EVENT();

        // If it's time to sample
        if (ev == PROCESS_EVENT_TIMER && etimer_expired(&sample_timer)) {

            DEBUG("Sampling\n");

            // Clear the previous sample, as it may have leftover things set we don't anticipate
            memset(&sample, 0, sizeof(sample));

            sample.time = sampler_get_time();

            sample.batt = sampler_get_batt();
            sample.has_batt = true;

            sample.temp = sampler_get_temp();
            sample.has_temp = true;

#ifndef FESHIE_NO_ACC
            sample.accX = sampler_get_acc_x();
            sample.has_accX = true;
            sample.accY = sampler_get_acc_y();
            sample.has_accY = true;
            sample.accZ = sampler_get_acc_z();
            sample.has_accZ = true;
#endif // FESHIE_NO_ACC

            // If get_extra requires some asynch things, wait until they're completed
            if(!sampler_get_extra(&sample, &sensor_config)) {
                DEBUG("Yielding for sampling_sensors_extra\n");
            } else {
                process_post(&sample_process, SAMPLER_EVENT_EXTRA_PERFORMED, NULL);
            }

        } else if (ev == SAMPLER_EVENT_EXTRA_PERFORMED) {
            id = store_save_sample(&sample);

            if (id) {
                DEBUG("Sample saved with id %d\n", id);
            } else {
                DEBUG("Failed to save sample!\n");
            }

        } else if (ev == SAMPLER_EVENT_RELOAD_CONFIG) {
            refresh_config();

            DEBUG("Refreshed Sensor config to:\n");
            print_sensor_config(&sensor_config);
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

void sampler_extra_performed(void) {
    process_post(&sample_process, SAMPLER_EVENT_EXTRA_PERFORMED, NULL);
}

void sampler_refresh_config(void) {
    DEBUG("Config marked for refresh!\n");
    process_post(&sample_process, SAMPLER_EVENT_RELOAD_CONFIG, NULL);
}
