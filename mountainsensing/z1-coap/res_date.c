/**
 * date resource
 * GET RTC date-time
 * POST RTC date-time to set the rtc
 * only for feshie nodes
 */
#include "er-server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include "dev/ds3231-sensor.h"  // Clock


static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    char message[13];
    int length;

	/* read RTC and get Epoch (int_32) */
    //ltoa(ds3231_get_epoch_seconds() , buffer, length);
    sprintf(message, "%ld",ds3231_get_epoch_seconds() );
	length = strlen(message);
    memcpy(buffer, message, length);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_header_etag(response, (uint8_t *) &length, 1);
    REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_date, "title=\"date: ?len=0..\";rt=\"Text\"", res_get_handler, NULL, NULL, NULL);

