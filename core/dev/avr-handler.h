#ifndef AVR_HANDLER
#define AVR_HANDLER

#include <stdbool.h>
#include <stdint.h>
#include "contiki.h"

PROCESS_NAME(avr_process);

/**
 * A struct used to request data from an AVR
 */
struct avr_data {

    /**
     * The ID of the AVR to sample from.
     */
    uint8_t id;

    /**
     * The size of data. (ie the bytes of memory allocated for data).
     */
    uint8_t size;

    /**
     * The data.
     */
    uint8_t *data;

    /**
     * The length of data. (ie the number of bytes used).
     */
    uint8_t *len;
};

/**
 * Get data from an AVR with a given ID
 * @param data Pointer to a avr_data struct that will be filled with the data obtained from the AVR. It's size should be set to
 * the max_size of the buffer it points to.
 * @return True on success, false otherwise
 */
bool avr_get_data(struct avr_data *data);

/**
 * Set the function to use to output data.
 * This function should typically be the output function of the serial port
 * we're set to receive data from.
 */
void avr_set_output(void (*out)(uint8_t *data, int len));

/**
 * Give us a single input byte received from the serial port.
 * @param byte The byte received.
 */
int avr_input_byte(uint8_t byte);

/**
 * Set the call back to use on succesfully processing an AVR request.
 */
void avr_set_callback(void (*callback)(bool isSuccess));

#endif // AVR_HANDLER
