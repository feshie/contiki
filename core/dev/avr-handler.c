#include "avr-handler.h"

#define DEBUG_ON
#include "debug.h"
#include "contiki-conf.h"

PROCESS(avr_process, "AVR Process");

/**
 * Event to get data from an AVR
 */
#define AVR_EVENT_GET_DATA 1
#define AVR_EVENT_GOT_DATA 3

/**
 * Timeout for receiving a reply in seconds
 */
#define AVR_TIMEOUT 10

/**
 * Maximum number of retries.
 * There will be AVR_RETRY + 1 attempts in total.
 */
#define AVR_RETRY 3

/**
 * RS484 AVR protocol defintions
 */
#define AVR_OPCODE_ECHO 0x00
#define AVR_OPCODE_LIST 0x01
#define AVR_OPCODE_GET_DATA 0x02
#define AVR_OPCODE_SET_GAIN 0x03
#define AVR_OPCODE_RESPONSE 0xFF
#define AVR_MASTER_ADDR 0x00

/**
 * We need 4 extra bytes - two at the start for the address + type,
 * and two at then end for the checksum
 */
static uint8_t buffer[sizeof(((Sample_AVR_t *)0)->bytes) + 4];
static uint8_t buffer_len;

/**
 * True if we're expecting to receive things, false otherwise
 */
static volatile bool isReceiving;

/**
 *
 */
static void (*serial_write)(uint8_t *buf, int len);

/**
 *
 */
static void (*callback)(bool isSuccess);

/**
 * The exact crc function used in the AVR & python
 * There is a contiki CRC function but not sure it'll
 * behave in the same way.
 */
static uint16_t crc16(uint16_t crc, uint8_t a);
static uint16_t crc16_all(uint16_t crc, uint8_t *buf, uint8_t len);

/**
 *
 */
static bool check_message(uint8_t *buf, uint8_t buf_len);

/**
 *
 */
static bool send_message(uint8_t addr, uint8_t opcode, uint8_t *payload, uint8_t payload_length);

/**
 *
 */
static bool process_message(uint8_t *buf, uint8_t bytes, Sample_AVR_t *data);

int avr_input_byte(uint8_t byte) {
    //DEBUG("Got some data!\n");

    if (!isReceiving) {
        DEBUG("Not receiving! Discarding data\n");
        return false;
    }

    // If the buffer is full, just ignore whatever extra data comes in
    if (buffer_len >= sizeof(buffer)) {
        DEBUG("Buffer full! Discarding data\n");
        return false;
    }

    buffer[buffer_len++] = byte;

    //DEBUG("Data saved as %d\n", buffer_len);

    // If this is a valid message, notify the process
    if (check_message(buffer, buffer_len)) {
        process_post(&avr_process, AVR_EVENT_GOT_DATA, NULL);
    }

    return true;
}

PROCESS_THREAD(avr_process, ev, data) {
    static struct etimer avr_timeout_timer;
    static uint8_t attempt;
    static uint8_t id;
    static Sample_AVR_t *sample;

    PROCESS_BEGIN();

    while (true) {
        PROCESS_WAIT_EVENT();

        DEBUG("Got event %d\n", ev);

        if (ev == AVR_EVENT_GET_DATA) {

            id = *((uint8_t *) data);
            sample = data;

            for (attempt = 0; attempt < AVR_RETRY; attempt++) {
                buffer_len = 0;

                DEBUG("Getting data from avr %x, attempt %d\n", id, attempt);

                isReceiving = true;

                // Request data from the node
                send_message(id, AVR_OPCODE_GET_DATA, NULL, (int)NULL);

                // Wait for the data
                etimer_set(&avr_timeout_timer, CLOCK_SECOND * AVR_TIMEOUT);
                DEBUG("Yielding until we get all the data\n");
                PROCESS_WAIT_EVENT_UNTIL(ev == AVR_EVENT_GOT_DATA || etimer_expired(&avr_timeout_timer));
                etimer_stop(&avr_timeout_timer);

                if (ev == AVR_EVENT_GOT_DATA) {
                    DEBUG("Data received!\n");
                } else {
                    DEBUG("Timeout reached\n");
                }

                isReceiving = false;

                DEBUG("Received %d bytes: ", buffer_len);
#ifdef DEBUG_ON
                int i;
                for (i = 0; i < buffer_len; i++) {
                    printf("%02x,", buffer[i]);
                }
                printf("\n");
#endif

                // Attempt to parse the data
                if (process_message((uint8_t *) buffer, buffer_len, sample)) {
                    break;
                }
            }

            DEBUG("Done after %d retries\n", attempt);

            // Call the callback
            callback(attempt < AVR_RETRY);

        } else {
            DEBUG("Unknown Event %d!\n", ev);
        }
    }

    PROCESS_END();
}

uint16_t crc16(uint16_t crc, uint8_t a) {
    //DEBUG("Starting CRC with 0x%04x\n", crc);
    uint8_t i;
    //DEBUG("Processing CRC for 0x%04x\n", a);
    crc ^= (uint16_t)a;
    //DEBUG("After XOR = 0x%04x\n", crc);
    for (i = 0; i < 8; ++i) {
        if (crc & 1) {
            crc = (crc >> 1) ^ 0xA001;
        } else {
            crc = (crc >> 1);
        }
    //DEBUG("CRC r%d = 0x%04x\n",i, crc);
    }
    //DEBUG("CRC RET = 0x%04x\n", crc);
    return crc;
}

uint16_t crc16_all(uint16_t crc, uint8_t *buf, uint8_t len) {
    uint8_t i;
    for (i = 0; i < len; i++) {
        crc = crc16(crc, buf[i]);
    }
    return crc;
}

bool send_message(uint8_t addr, uint8_t opcode, uint8_t *payload, uint8_t payload_length) {
    uint16_t crc = 0xFFFF;

    //DEBUG("Dest: %02x\n", addr);
    //DEBUG("Optcode: %02x\n", opcode);
    //DEBUG("Payload length: %i\n", payload_length);

    if (serial_write == NULL) {
        DEBUG("No writebyte specified\n");
        return false;
    }

    // Add the address and type to the crc
    crc = crc16(crc, addr);
    crc = crc16(crc, opcode);

    // Add the payload to the crc
    crc = crc16_all(crc, payload, payload_length);

    DEBUG("CRC is %u High: %x Low: %x High2: %x Low2: %x\n", crc, (crc >> 8) & 0xFF, crc & 0xFF, *(((uint8_t *) &crc) + 1), *((uint8_t *) &crc));

    serial_write(&addr, 1);
    serial_write(&opcode, 1);
    serial_write(payload, payload_length);

    // MSP430 is little endian - low byte is at lowest address.
    // We use little endian for the avr proto too
    serial_write(((uint8_t *) &crc), 2);

    return true;
}

bool check_message(uint8_t *buf, uint8_t buf_len) {
    if (buf_len <= 4) {
        //DEBUG("Too small for valid protocol buffer: %d\n", buf_len);
        return false;
    }

    if (buf[1] != AVR_OPCODE_RESPONSE) {
        //DEBUG("Not a response packet\n");
        return false;
    }

    if (buf[0] != AVR_MASTER_ADDR) {
        //DEBUG("Not for us");
        return false;
    }

    uint16_t rcv_crc = ((uint16_t)buf[buf_len - 1] << 8) | buf[buf_len - 2];
    //DEBUG("Recieved CRC: %d\n", rcv_crc);

    // Ignore the crc from the crc calculations
    uint16_t cal_crc = crc16_all(0xFFFF, buf, buf_len - 2);

    //DEBUG("Calculated CRC: %d\n", cal_crc);

    if (rcv_crc != cal_crc) {
        //DEBUG("CRCs do not match: Ignoring\n");
        return false;
    }

    //DEBUG("CRCs match\n");

    return true;
}

bool process_message(uint8_t *buf, uint8_t buf_len, Sample_AVR_t *data) {
    if (!check_message(buf, buf_len)) {
        return false;
    }

    // Ignore the first 2 and last 2 bytes
    memcpy(data->bytes, buf + 2, buf_len - 4);
    data->size = buf_len - 4;

    return true;
}

void avr_set_callback(void (*cb)(bool isSuccess)) {
    callback = cb;
}

bool avr_get_data(uint8_t id, Sample_AVR_t *data) {
    // If we don't have a callback set, we can't do anything
    if (callback == NULL) {
        DEBUG("Callback not set!\n");
        return false;
    }

    // If we're already in the process of receiving data, try later
    if (isReceiving) {
        DEBUG("Already receiving!\n");
        return false;
    }

    DEBUG("Sending event for avr %x\n", id);
    *((uint8_t *) data) = id;

    DEBUG("Data %p bytes %p\n", data, data->bytes);

    return process_post(&avr_process, AVR_EVENT_GET_DATA, data) == PROCESS_ERR_OK;
}

void avr_set_output(void (*wb)(uint8_t *buf, int len)) {
    serial_write = wb;
}
