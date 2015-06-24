/**
 * @file
 * Config ressource
 * Serves the current config, and supports setting the config.
 */

#include "er-server.h"
#include "rest-engine.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "settings.pb.h"
#include "store.h"
#include <inttypes.h>
#include <stdlib.h>

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
//static void res_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * Config ressource.
 */
RESOURCE(res_config, "Config", res_get_handler, /*res_post_handler*/ NULL, NULL, NULL);

void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    static SensorConfig config;
    static int32_t current_offset;
    static uint8_t pb_buffer[SensorConfig_size];
    static pb_ostream_t pb_ostream;
    static uint8_t buffer_len;

    DEBUG("Serving request! Offset %d, PrefSize %d\n", (int) *offset, preferred_size);

    current_offset = *offset;

    // Only get data if this is the first request of a blockwise transfer
    if (current_offset == 0 && !store_get_config(&config)) {
        // 500 internal error
        DEBUG("Unable to get config!\n");
        REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
        return;
    }

    DEBUG("Got config\n");

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
    pb_encode_delimited(&pb_ostream, SensorConfig_fields, &config);

    // If whatever we have to send fits in one block, just send that
    if (pb_ostream.bytes_written - current_offset <= preferred_size) {
        DEBUG("Request fits in a single block\n");

        buffer_len = pb_ostream.bytes_written - current_offset;

        // Indicates this is the last chunk
        *offset = -1;

    // Otherwise copy data in chunks to the buffer
    } else {
        DEBUG("Request will be split into chunks\n");

        buffer_len = preferred_size;

        *offset += preferred_size;
    }

    memcpy(buffer, pb_buffer + current_offset, buffer_len);

    REST.set_header_content_type(response, REST.type.APPLICATION_OCTET_STREAM);
    REST.set_response_payload(response, buffer, buffer_len);
}

/*
void res_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    static coap_packet_t *const coap_req;
    static uint8_t *incoming;
    static uint8_t pb_buffer[SensorConfig_size];
    static uint8_t buffer_len;
    static size_t len;
    static int ct;
    static SensorConfig config;
    static pb_istream_t pb_istream;

    coap_req = (coap_packet_t *)request;

    if ((len = REST.get_request_payload(request, (const uint8_t **)&incoming))) {

        if (coap_req->block1_num * coap_req->block1_size + len <= SensorConfig_size) {

            memcpy(pb_buffer + coap_req->block1_num * coap_req->block1_size, incoming, len);


            pb_istream = pb_istream_from_buffer(incoming, len);

            // Check it was sucesfully decoded.
            if (!pb_decode(&pb_istream, SensorConfig_fields, config)) {
                REST.set_response_status(response, REST.status.BAD_REQUEST);
                return;
            }


            REST.set_response_status(response, REST.status.CHANGED);
            coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);

        } else {
            REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
            return;
        }

    } else {
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }

    static int16_t sample_id;

    sample_id = parse_sample_id(request);

    if (sample_id == NO_SAMPLE_ID || sample_id == INVALID_SAMPLE_ID) {
        DEBUG("Delete request with invalid / missing sample id!\n");
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }

    DEBUG("Delete request for: %d\n", sample_id);

    if (!store_delete_sample(&sample_id)) {
        DEBUG("Failed to delete sample\n");
        REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
        return;
    }

    REST.set_response_status(response, REST.status.DELETED);
}*/
