/**
 * Reboot ressource.
 * Get returns reboot count.
 * Post reboots the node.
 */
#include "er-server.h"
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "watchdog.h"
#include "ms-io.h"

#define DEBUG_ON
#include "debug.h"

static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    uint16_t reboots;
    // Max value of uint16_t is 5 chars + 1 null terminator
    char message[6];

    if (!ms_get_reboot(&reboots)) {
        REST.set_response_status(response, REST.status.SERVICE_UNAVAILABLE);
        return;
    }

    int length = snprintf(message, sizeof(message), "%u", reboots);
    memcpy(buffer, message, length);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, buffer, length);
}

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    watchdog_reboot();
}

RESOURCE(res_reboot, "Reboot the node", res_get_handler, res_post_handler, NULL, NULL);
