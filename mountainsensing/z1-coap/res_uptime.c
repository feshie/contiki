/**
 * \file res_date.c
 * Uptime ressource. Reports how long the node has been up for.
 * Arthur Fabre 2016
 */
#include "er-server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include <inttypes.h>

#define DEBUG_ON
#include "debug.h"

static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    uint32_t time = clock_seconds();

    int length = snprintf((char *)buffer, preferred_size, "%" PRIu32, time);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_uptime, "Uptime", res_get_handler, NULL, NULL, NULL);
