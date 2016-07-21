/**
 * @file res_config
 * Config ressource
 * Serves the current config, and supports setting the config.
 * Arthur Fabre 2015
 */

#include "er-server.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "settings.pb.h"
#include "store.h"
#include <inttypes.h>
#include <stdlib.h>
#include <sampler.h>

#define DEBUG_ON
#include "debug.h"

/**
 * Get handler for Config.
 * Returns the curent configuration.
 */
static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * Post handler for Config.
 * Sets the current config.
 */
static void res_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * Buffer to store a config. Used for both get and post - requests will fail if they are interleaved.
 */
static uint8_t config_buffer[SensorConfig_size];

/**
 * Length of the configuration buffer.
 */
static uint8_t config_len;

/**
 * Config ressource.
 */
RESOURCE(res_config, "Config", res_get_handler, res_post_handler, NULL, NULL);

void res_get_handler(void* request, void* response, uint8_t *payload_buffer, uint16_t preferred_size, int32_t *offset) {
    uint8_t payload_len;
    int16_t current_offset = *offset;

    DEBUG("Serving request! Offset %d, PrefSize %d\n", current_offset, preferred_size);

    // Only get data if this is the first request of a blockwise transfer
    if (current_offset == 0) {

        config_len = store_get_raw_config(config_buffer);

        if (!config_len) {
            // 500 internal error
            DEBUG("Unable to get config!\n");
            REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
            return;
        }
    }

    DEBUG("Got config\n");

    // If whatever we have to send fits in one block, just send that
    if (config_len - current_offset <= preferred_size) {
        DEBUG("Request fits in a single block\n");

        payload_len = config_len - current_offset;

        // Indicates this is the last chunk
        *offset = -1;

    // Otherwise copy data in chunks to the buffer
    } else {
        DEBUG("Request will be split into chunks\n");

        payload_len = preferred_size;

        *offset += preferred_size;
    }

    memcpy(payload_buffer, config_buffer + current_offset, payload_len);

    REST.set_header_content_type(response, REST.type.APPLICATION_OCTET_STREAM);
    REST.set_response_payload(response, payload_buffer, payload_len);
}

void res_post_handler(void* request, void* response, uint8_t *payload_buffer, uint16_t preferred_size, int32_t *offset) {
    uint8_t *incoming;
    size_t incoming_len;
    pb_istream_t pb_istream;
    SensorConfig config;
    coap_packet_t *coap_req = (coap_packet_t *)request;

    DEBUG("Config post request!\n");

    // Check there's a payload, and get it
    if ((incoming_len = REST.get_request_payload(request, (const uint8_t **)&incoming))) {

        if (coap_req->block1_num * coap_req->block1_size + incoming_len <= SensorConfig_size) {

            DEBUG("Got config payload\n");

            // Store the payload in our static buffer
            memcpy(config_buffer + coap_req->block1_num * coap_req->block1_size, incoming, incoming_len);
            config_len = coap_req->block1_num * coap_req->block1_size + incoming_len;

            // If this is the last packet
            if (!coap_req->block1_more) {

                DEBUG("Last (or only) config block, saving...\n");

                pb_istream = pb_istream_from_buffer(config_buffer, config_len);

                // Check it was sucesfully decoded.
                if (!pb_decode_delimited(&pb_istream, SensorConfig_fields, &config)) {
                    DEBUG("Failed to decode config!\n");
                    REST.set_response_status(response, REST.status.BAD_REQUEST);
                    return;
                }

                // Check the config is sane
                if (!sampler_check_config(&config)) {
                    DEBUG("Unsane config!\n");
                    REST.set_response_status(response, REST.status.BAD_REQUEST);
                    return;
                }

                // Save it
                if (!store_save_config(&config)) {
                    DEBUG("Failed to save config!\n");
                    REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
                    return;
                }

                DEBUG("Config saved and decoded\n");

                sampler_refresh_config();
            }

            REST.set_response_status(response, REST.status.CHANGED);
            coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);

        } else {
            DEBUG("Config post request too big!\n");
            REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
            return;
        }

    } else {
        DEBUG("Unable to get payload!\n");
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }
}
