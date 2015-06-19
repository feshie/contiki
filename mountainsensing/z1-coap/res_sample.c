/**
 * @file
 * Sample ressource.
 * Serves stored samples, and can delete them.
 *
 * TODO Does not yet serve any arbitrary readings
 * TODO Does not yest support deleteing
 */
#include "er-server.h"
#include "rest-engine.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "readings.pb.h"
#include "store.h"

#define DEBUG_ON
#include "debug.h"

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    static Sample sample;
    static uint8_t pb_buffer[Sample_size];
    static pb_ostream_t pb_ostream;
    static uint8_t buffer_len;

    DEBUG("Serving request! Offset %d, PrefSize %d\n", (int) *offset, preferred_size);

    if (!store_get_latest_sample(&sample)) {
        // 500 internal error
        DEBUG("Unable to get sample!\n");
        REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
        return;
    }

    DEBUG("Got sample with id %d\n", sample.id);

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
    pb_encode_delimited(&pb_ostream, Sample_fields, &sample);

    // If whatever we have to send fits in one block, just send that
    if (pb_ostream.bytes_written - *offset <= preferred_size) {
        DEBUG("Request fits in a single block\n");

        buffer_len = pb_ostream.bytes_written - *offset;

        // Indicates this is the last chunk
        *offset = -1;

    // Otherwise copy data in chunks to the buffer
    } else {
        DEBUG("Request will be split into chunks\n");

        buffer_len = preferred_size;

        *offset += preferred_size;
    }

    memcpy(buffer, pb_buffer + *offset, buffer_len);

    REST.set_header_content_type(response, REST.type.APPLICATION_OCTET_STREAM);
    REST.set_response_payload(response, buffer, buffer_len);
}

RESOURCE(res_sample, "Sample", res_get_handler, NULL, NULL, NULL);

