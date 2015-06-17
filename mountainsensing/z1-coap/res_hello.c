/**
 * Test HELLO resource
 * does almost nothing - useful for tests
 */
#include "er-server.h"
#include "rest-engine.h"
#include "sampler.h"
#include "dev/batv-sensor.h" // Batt


/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    //char const * const message = "Hello World!";
    char message[30];
    int length;
	float f;

	ms1_sense_on();

	f = get_sensor_batt();
	ms1_sense_off();
	sprintf(message,"node batV %d.%d\n",(int)f,(int)(f * 1000) );
	length = strlen(message);
    memcpy(buffer, message, length);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_header_etag(response, (uint8_t *) &length, 1);
    REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_hello, "title=\"Hello world: ?len=0..\";rt=\"Text\"", res_get_handler, NULL, NULL, NULL);

