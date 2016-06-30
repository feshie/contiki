/**
 * \file res_date.c
 * date resource
 * GET RTC date-time
 * POST RTC date-time to set the rtc
 * only for feshie nodes
 * Arthur Fabre 2015
 */
#include "er-server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include <errno.h>
#include <inttypes.h>
#include "sampling-sensors.h"

#define DEBUG_ON
#include "debug.h"

static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    uint32_t time;

    if (!sampler_get_time(&time)) {
        REST.set_response_status(response, REST.status.SERVICE_UNAVAILABLE);
        return;
    }

    int length = snprintf((char *)buffer, preferred_size, "%" PRIu32, time);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, buffer, length);
}

static void res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    uint8_t *incoming;
    uint8_t incoming_len;
    char *end_ptr;
    char *start_ptr;
    uint32_t seconds;

    DEBUG("Attempting to set time!\n");

    if (!(incoming_len = REST.get_request_payload(request, (const uint8_t **)&incoming))) {
        DEBUG("Failed to get payload\n");
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }

    start_ptr = (char *) incoming;

    // Convert the string to an int
    errno = 0;
    seconds = strtol(start_ptr, &end_ptr, 0);
    // Errno being set indicates an error occured.
    // The enptr checks that the entire payload we got was succesfully decoded, and not just the start of it.
    if (errno != 0 || ((char *) incoming) + incoming_len > end_ptr) {
        DEBUG("Failed to parse epoch\n");
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }

    DEBUG("Requested time is %" PRIu32 "\n", seconds);

    if (!sampler_set_time(seconds)) {
        DEBUG("Failed to set epoch\n");
        REST.set_response_status(response, REST.status.SERVICE_UNAVAILABLE);
        return;
    }

    REST.set_response_status(response, REST.status.CHANGED);
}

RESOURCE(res_date, "title=\"date: ?len=0..\";rt=\"Text\"", res_get_handler, res_post_put_handler, NULL, NULL);

