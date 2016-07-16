#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>

#include "sampler.h"

#include "contiki-conf.h"
#include "contiki.h"

#include "store.h"
#include "ms-io.h"
#include "dev/avr-handler.h"
#include "pb_decode.h"
#include "z1-coap-config-defaults.h"
#include "settings.pb.h"
#include "readings.pb.h"
#include "power.pb.h"
#include "net/rpl/rpl.h"

#define DEBUG_ON
#include "debug.h"

PROCESS(sample_process, "Sample Process");

/**
 * Event sent to the sampler when the current sample is done,
 * and needs to be saved.
 */
#define SAMPLER_EVENT_SAVE_SAMPLE 1

/**
 * Event sent to the sampler when the configuration
 * should be reloaded from Flash.
 */
#define SAMPLER_EVENT_RELOAD_CONFIG 2

/**
 * The current sensor config being used.
 */
static SensorConfig config;

/**
 * The current sample we're working on.
 */
static Sample sample;

/**
 * True if the AVRs have been handled and we should do the PowerBoards now, False otherwise.
 */
static bool is_avr_complete;

/**
 * Buffer to store incomming Power board data.
 * We can't use the Sample struct directly, as we need to decode the incomming data.
 */
static uint8_t power_data[PowerInfo_size];

/**
 * Length (not size) use of the power_data buffer.
 */
static uint8_t power_len;

/**
 * Struct used to get data from an AVR
 */
static struct avr_data data;

/**
 * Callback invoked by the avr-handler.
 * @param isSuccess True if data was succesfully received, false otherwise.
 */
static void avr_callback(bool isSuccess);

/**
 * Start receiving data from an AVR.
 * @return True if the get-data operation was succesfully initiated, false otherwise.
 */
static bool start_avr(void);

/**
 * Finalize our state when we are done receiving data from an AVR.
 * @param isSuccess True if data was succesfully received, false otherwise.
 */
static void end_avr(bool isSuccess);

/**
 * Start receiving data from a power board.
 * @return True if the get-data operation was succesfully initiated, false otherwise.
 */
static bool start_power(void);

/**
 * Finalize our state when we are done receiving data from a power board.
 * @param isSuccess True if data was succesfully received, false otherwise.
 */
static void end_power(bool isSuccess);

/**
 * Instruct us to turn sense off,
 * and save the current sample we're working on (ie we're done with it).
 */
static void save_sample(void);

/**
 * Get and set the latest config from the Store.
 * If no config exists, set it to DEFAULT_CONFIG.
 */
static void refresh_config(void);

/**
 * Print the sensor config for debugging.
 */
static void print_config(SensorConfig *conf);

PROCESS_THREAD(sample_process, ev, data) {
    static struct etimer sample_timer;

    PROCESS_BEGIN();

    refresh_config();

    ms_init();

    // Set the AVR callback
    avr_set_callback(&avr_callback);

    while(true) {

        {
            uint32_t time;
            ms_get_time(&time);
            etimer_set(&sample_timer, CLOCK_SECOND * (config.interval - (time % config.interval)));
        }

        PROCESS_WAIT_EVENT();

        // If it's time to sample
        if (ev == PROCESS_EVENT_TIMER && etimer_expired(&sample_timer)) {

            DEBUG("Sampling\n");

            // Clear the previous sample, as it may have leftover things set we don't anticipate
            memset(&sample, 0, sizeof(sample));

            ms_sense_on();

            ms_get_time(&sample.time);

            sample.has_temp = ms_get_temp(&sample.temp);

            sample.has_humid = ms_get_humid(&sample.humid);

            sample.has_ADC1 = ms_get_adc1(&sample.ADC1);

            sample.has_ADC2 = ms_get_adc2(&sample.ADC2);

            sample.has_rain = ms_get_rain(&sample.rain);

            sample.has_accX = sample.has_accY = sample.has_accZ = ms_get_acc(&sample.accX, &sample.accY, &sample.accZ);

            // If we don't have a power board, use good old batt volts
            if (!config.has_powerID && ms_get_batt(&sample.batt)) {
                sample.which_battery = Sample_batt_tag;
            }

            // If we have no AVR, consider it done
            is_avr_complete = !config.has_avrID;

            // No avr
            if (is_avr_complete) {
                // If we have a power board, and it has been queud successfully for getting data, wait for that
                if (config.has_powerID && start_power()) {
                    continue;
                }

                // Otherwise we're done
                save_sample();
                continue;
            }

            // If we have an AVR, and it has been queud successfully for getting data, wait for that
            // Let the sampler know we'll call it back
            if (config.has_avrID && start_avr()) {
                continue;
            }

            // Otherwise we're done
            save_sample();

        } else if (ev == SAMPLER_EVENT_SAVE_SAMPLE) {
            ms_sense_off();

            int16_t id = store_save_sample(&sample);

            if (id) {
                DEBUG("Sample saved with id %d\n", id);
            } else {
                DEBUG("Failed to save sample!\n");
            }

        } else if (ev == SAMPLER_EVENT_RELOAD_CONFIG) {
            refresh_config();

            DEBUG("Refreshed Sensor config to:\n");
            print_config(&config);
        }
    }
    PROCESS_END();
}

static void refresh_config(void) {
    if (!store_get_config(&config)) {
        // Config file does not exist! Use default and set file
      	DEBUG("No Sensor config found\n");
        config = SENSOR_DEFAULT_CONFIG;
        store_save_config(&config);
    } else {
      	DEBUG("Sensor config loaded\n");
    }

    // Set the RPL mode. Doesn't really belong here, but oh well
    DEBUG("Setting RPL mode to %d\n", config.routingMode);
    rpl_set_mode(config.routingMode);
}

void print_config(SensorConfig *conf) {
    DEBUG("\tInterval = %d\n", (unsigned int)conf->interval);

    DEBUG("\tADC1: %s\n", conf->hasADC1 ? "yes" : "no");
    DEBUG("\tADC2: %s\n", conf->hasADC2 ? "yes" : "no");
    DEBUG("\tRain: %s\n", conf->hasRain ? "yes" : "no");

    if (conf->has_avrID) {
        DEBUG("\tAVR: %02X\n", conf->avrID);
    }

    if (conf->has_powerID) {
        DEBUG("\tPower: %02X\n", conf->powerID);
    }

    DEBUG("\tRoutingMode: %d\n", conf->routingMode);
}

void sampler_refresh_config(void) {
    DEBUG("Config marked for refresh!\n");
    process_post(&sample_process, SAMPLER_EVENT_RELOAD_CONFIG, NULL);
}

void save_sample(void) {
    process_post(&sample_process, SAMPLER_EVENT_SAVE_SAMPLE, NULL);
}

void avr_callback(bool isSuccess) {
    // If the avr isn't done yet, this must be the end of it
    if (!is_avr_complete) {
        end_avr(isSuccess);
        is_avr_complete = true;

        // If we have a power board and we've succesfully queued a get data, wait until we get called again
        if (config.has_powerID && start_power()) {
            return;
        }

    // Otherwise it must be the end of the power board
    } else {
        end_power(isSuccess);
    }

    save_sample();
}

bool start_avr(void) {
    // Use the buffer in the sample directly
    data.data = sample.AVR.bytes;
    data.len = &sample.AVR.size;
    data.id = config.avrID;
    // Size of the buffer is the size of the buffer in the Sample
    data.size = sizeof(sample.AVR.bytes);

    DEBUG("Getting data from avr 0x%02X\n", config.avrID);

    return avr_get_data(&data);
}

void end_avr(bool isSuccess) {
    DEBUG("Sampled from avr. Success: %d\n", isSuccess);
    sample.has_AVR = isSuccess;
}

bool start_power(void) {
    // Use the power_data buffer
    data.data = power_data;
    data.len = &power_len;
    data.id = config.powerID;
    data.size = sizeof(power_data);

    DEBUG("Getting data from power 0x%02X\n", config.powerID);

    return avr_get_data(&data);
}

void end_power(bool isSuccess) {
    DEBUG("Sampled from power board. Success: %d\n", isSuccess);

    if (!isSuccess) {
        return;
    }

    pb_istream_t istream = pb_istream_from_buffer(data.data, *data.len);
    if (!pb_decode(&istream, PowerInfo_fields, &sample.power)) {
        return;
    }

    sample.which_battery = Sample_power_tag;
    return;
}
