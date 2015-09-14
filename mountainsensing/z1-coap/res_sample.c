/**
 * @file res_sample.c
 * Sample resource.
 * Serves stored samples, and can delete them.
 * Arthur Fabre 2015
 */

#include "er-server.h"
#include "rest-engine.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "readings.pb.h"
#include "store.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG_ON
#include "debug.h"

/**
 * Maximum length of a URI
 */
#define MAX_URI_LENGTH  20

/**
 * Indicates no trailing sample id was found in the URI
 */
#define NO_SAMPLE_ID        -1

/**
 * Indicates a sample id was found in the URI, but that it could not be sucesfully parsed
 */
#define INVALID_SAMPLE_ID   -2

/**
 * The separator used in URIs, either as a character, or as a string literal.
 */
#define SEPARATOR_CHAR  '/'
#define SEPARATOR_STR   "/"

/**
 * Get handler for Samples.
 * Supports the optional param id. Serves latest if id isn't specified.
 * Format is GET /sample/23 to get sample #23. GET /sample to get the latest sample.
 */
static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * Delete handler for Samples.
 * Supports deleting arbitrary Samples.
 * Format is DELETE /sample/23 to delete sample #23
 */
static void res_delete_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * Parse a trailing sample id from a URI.
 * Deals with validating the ID, trailing slashes.
 * @return NO_SAMPLE_ID if no trailing sample id was found in the uri.
 *         INVALID_SAMPLE_ID If a trailing sample if was found, but it could not be parsed.
 *         The sample id on success.
 */
static int16_t parse_sample_id(void *request);

/**
 * Sample ressource.
 * Parent ressource as we use URL based parametes (like GET /sample/32)
 */
PARENT_RESOURCE(res_sample, "Sample", res_get_handler, NULL, NULL, res_delete_handler);

void res_get_handler(void* request, void* response, uint8_t *payload_buffer, uint16_t preferred_size, int32_t *offset) {
    static uint8_t sample_buffer[Sample_size];
    static uint8_t sample_len;
    int16_t sample_id;
    uint8_t payload_len;

    int16_t current_offset = *offset;

    DEBUG("Serving request! Offset %d, PrefSize %d\n", current_offset, preferred_size);

    // Only get data if this is the first request of a blockwise transfer
    if (current_offset == 0) {

        sample_id = parse_sample_id(request);

        // Error out if a sample specified was invalid
        if (sample_id == INVALID_SAMPLE_ID) {
            DEBUG("Get request with invalid sample id!\n");
            REST.set_response_status(response, REST.status.BAD_REQUEST);
            return;
        }

        // Get the latest sample if no sample id was specified
        if (sample_id == NO_SAMPLE_ID) {
            sample_len = store_get_latest_raw_sample(sample_buffer);
        } else {
            sample_len = store_get_raw_sample(sample_id, sample_buffer);
        }

        if (!sample_len) {
            DEBUG("Unable to get sample!\n");
            REST.set_response_status(response, REST.status.NOT_FOUND);
            return;
        }
    }

    DEBUG("Got a Sample, size: %u\n", sample_len);

    // If whatever we have to send fits in one block, just send that
    if (sample_len - current_offset <= preferred_size) {
        DEBUG("Request fits in a single block\n");

        payload_len = sample_len - current_offset;

        // Indicates this is the last chunk
        *offset = -1;

    // Otherwise copy data in chunks to the buffer
    } else {
        DEBUG("Request will be split into chunks\n");

        payload_len = preferred_size;

        *offset += preferred_size;
    }

    memcpy(payload_buffer, sample_buffer + current_offset, payload_len);

    REST.set_header_content_type(response, REST.type.APPLICATION_OCTET_STREAM);
    REST.set_response_payload(response, payload_buffer, payload_len);
    DEBUG("Done!\n");
}

void res_delete_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    int16_t sample_id;

    sample_id = parse_sample_id(request);

    if (sample_id == NO_SAMPLE_ID || sample_id == INVALID_SAMPLE_ID) {
        DEBUG("Delete request with invalid / missing sample id!\n");
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }

    DEBUG("Delete request for: %d\n", sample_id);

    if (!store_delete_sample(sample_id)) {
        DEBUG("Failed to delete sample\n");
        REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
        return;
    }

    REST.set_response_status(response, REST.status.DELETED);
}

int16_t parse_sample_id(void *request) {
    const char *uri_path;
    int uri_length;
    // Need extra byte for null terminator
    char terminated_uri_path[MAX_URI_LENGTH + 1];
    char *token_ptr;
    char *end_ptr;
    int16_t sample_id;

    uri_length = REST.get_url(request, &uri_path);

    if (uri_length > MAX_URI_LENGTH) {
        uri_length = MAX_URI_LENGTH;
    }

    // The returned uir isn't NULL terminated - need to do it ourselves
    strncpy(terminated_uri_path, uri_path, uri_length);
    terminated_uri_path[uri_length] = '\0';
    token_ptr = &terminated_uri_path[0];

    // Parse the uri
    strsep(&token_ptr, SEPARATOR_STR);
    if (token_ptr == NULL) {
        DEBUG("Request with no sample id!\n");
        return NO_SAMPLE_ID;
    }

    // Convert the string to an int
    errno = 0;
    sample_id = strtoul(token_ptr, &end_ptr, 0);
    // Errno being set indicates an error occured.
    // endptr != \0 indicates that the entire string was not parsed succesfully - we want to ensure the entire string is valid.
    // endptr == SEPARATOR is valid to accept trailing SEPARATOR
    if (errno != 0 || (*end_ptr != '\0' && *end_ptr != SEPARATOR_CHAR)) {
        DEBUG("Request with invalid sample id!\n");
        return INVALID_SAMPLE_ID;
    }

    return sample_id;
}
