#ifndef AVR_HANDLER
#define AVR_HANDLER

#include <stdbool.h>
#include <stdint.h>
#include "contiki.h"
#include "readings.pb.h"

PROCESS_NAME(avr_process);

/**
 * Get data from an AVR with a given ID
 * @param id The id of the AVR
 * @param data A Sample_AVR_t (a nanopb struct) that will be filled with the data obtained from the AVR
 * @return True on success, false otherwise
 */
bool avr_get_data(uint8_t id, Sample_AVR_t *data);

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
