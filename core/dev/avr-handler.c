#include "avr-handler.h"

/**
 * Turn debuggin on.
 * NOTE: A lot of debugging calls are disabled as interrupts are not rentrant,
 * and the debugging calls can cause us to miss interrupts.
 */
//#define DEBUG_ON
#ifdef DEBUG_ON
    #include <stdio.h>
    #define DEBUG(...) printf(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif

PROCESS(avr_process, "AVR Process");

/**
 * Event to get data from an AVR
 */
#define AVR_EVENT_GET_DATA 1

/**
 * Event indicating we've received valid data from an AVR
 */
#define AVR_EVENT_GOT_DATA 2

/**
 * Timeout for receiving a reply in seconds
 */
#define AVR_TIMEOUT 10

/**
 * Maximum number of retries.
 * There will be AVR_RETRY attempts in total.
 */
#define AVR_RETRY 4

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
 * The destination address (ie 1st byte) of the received message.
 */
static uint8_t incm_dest;

/**
 * The type (ie 2nd byte) of the received message.
 */
static uint8_t incm_type;

/**
 * The size of the received message.
 * This should / could only only be a byte, but the odd number of bytes messes up alignmenent and the compiler generates
 * broken code (msp430 cannot deal with relocation of unaligned addresses)
 */
static uint16_t incm_num;

/**
 * The crc of the received message (ie last 2 bytes).
 */
static uint8_t incm_crc[2];

/**
 * The avr_data to write the received message payload to.
 */
static struct avr_data *incm_data;

/**
 * True if we're expecting to receive things, false otherwise
 */
static volatile bool isReceiving;

/**
 * Function to write to the serial port.
 * @param buf The data to write.
 * @param len The amount of bytes to write.
 */
static void (*serial_write)(uint8_t *buf, int len);

/**
 * Callback for when we're done dealing with an AVR.
 * @param isSuccess True if we succesfully read data from the AVR and wrote it
 * to the passed in avr_data, false otherwise.
 */
static void (*callback)(bool isSuccess);

/**
 * Add a byte to a CRC.
 * @param crc The existing CRC. 0xFFFF should be used as a starting value.
 * @param a The byte to add
 * @return The new crc
 *
 * Note: This is the exact crc function used in the AVR & python
 * There is a contiki CRC function but not sure it'll
 * behave in the same way.
 */
static uint16_t crc16(uint16_t crc, uint8_t a);

/**
 * Add several bytes to a CRC.
 * @param crc The existing CRC. 0xFFFF should be used as a starting value.
 * @param buf The bytes to add
 * @param len The number of bytes to add
 * @return The new crc
 */
static uint16_t crc16_all(uint16_t crc, uint8_t *buf, uint8_t len);

/**
 * Chack a given message is a valid response message.
 * (ie the message is for us, is of type response, and has a valid crc).
 * @param addr The address of the AVR
 * @param opcode The type of the message
 * @param payload The payload to send in the message
 * @parama payload_length The length of the payload
 * @param crc The two crc bytes of the message
 */
static bool check_message(uint8_t addr, uint8_t opcode, uint8_t *payload, uint8_t payload_length, uint8_t crc[2]);

/**
 * Send a message to an AVR
 * @param addr The address of the AVR
 * @param opcode The type of the message
 * @param payload The payload to send in the message
 * @parama payload_length The length of the payload
 * @return True if the message was sent, false otherwise.
 */
static bool send_message(uint8_t addr, uint8_t opcode, uint8_t *payload, uint8_t payload_length);

int avr_input_byte(uint8_t byte) {
    //DEBUG("Got some data!\n");

    if (!isReceiving) {
        DEBUG("Not receiving! Discarding data\n");
        return false;
    }

    // If the buffer is full, just ignore whatever extra data comes in
    if (*incm_data->len >= incm_data->size) {
        DEBUG("Incm Buffer full, len %d size %d! Discarding data\n", *incm_data->len, incm_data->size);
        return false;
    }

    incm_num++;

    // Cases 1-4 progressively "load the bases",
    // default case assumes bases are loaded and simply shuffles everything by one
    // to store the new byte
    // Cases 1-3 return directly, as a message of that length can't be valid, so there's no point in checking.
    switch (incm_num) {
        case 1:
            // First byte is the address
            incm_dest = byte;
            return true;

        case 2:
            // Second byte is the type
            incm_type = byte;
            return true;

        case 3:
            incm_crc[0] = byte;
            return true;

        case 4:
            incm_crc[1] = byte;
            break;

        default:
            incm_data->data[(*incm_data->len)++] = incm_crc[0];
            incm_crc[0] = incm_crc[1];
            incm_crc[1] = byte;
            break;
    }

    // If this is a valid message, notify the process
    if (check_message(incm_dest, incm_type, incm_data->data, *incm_data->len, incm_crc)) {
        process_post(&avr_process, AVR_EVENT_GOT_DATA, NULL);
    }

    return true;
}

PROCESS_THREAD(avr_process, ev, data_ptr) {
    static struct etimer avr_timeout_timer;
    static uint8_t attempt;
    static uint8_t num_required;
    static uint8_t success_num;

    PROCESS_BEGIN();

    while (true) {
        PROCESS_WAIT_EVENT();

        DEBUG("Got event %d\n", ev);

        if (ev == AVR_EVENT_GET_DATA) {

            incm_data = data_ptr;
            num_required = 1;

            success_num = 0;

            // If it's a temp accel chain, it needs to be read twice to get valid data
            if (incm_data->id < 0x10) {
                num_required = 2;
            }

            for (attempt = 0; attempt < AVR_RETRY * num_required; attempt++) {

                DEBUG("Getting data from avr %x, attempt %d, success_num %d\n", incm_data->id, attempt, success_num);

                // Reset the len of the payload
                *incm_data->len = 0;
                incm_num = 0;

                etimer_set(&avr_timeout_timer, CLOCK_SECOND * AVR_TIMEOUT);

                isReceiving = true;

                // Request data from the node
                send_message(incm_data->id, AVR_OPCODE_GET_DATA, NULL, (int)NULL);

                // Wait for the data
                PROCESS_WAIT_EVENT_UNTIL(ev == AVR_EVENT_GOT_DATA || etimer_expired(&avr_timeout_timer));
                etimer_stop(&avr_timeout_timer);

                isReceiving = false;

                success_num += (ev == AVR_EVENT_GOT_DATA);

                DEBUG("Received %d bytes. Success_num %d ADDR %u TYPE %u CRC %u: ", incm_num, success_num, incm_dest, incm_type, *((uint16_t *) incm_crc));
#ifdef DEBUG_ON
                int i;
                for (i = 0; i < *incm_data->len; i++) {
                    printf("%02x,", incm_data->data[i]);
                }
                printf("\n");
#endif

                // If we got enough successful rpelies
                if (success_num >= num_required) {
                    break;
                }
            }

            DEBUG("Done after %d retries\n", attempt);

            callback(success_num >= num_required);
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
    //DEBUG("Dest: %02x\n", addr);
    //DEBUG("Optcode: %02x\n", opcode);
    //DEBUG("Payload length: %i\n", payload_length);

    if (serial_write == NULL) {
        DEBUG("No writebyte specified\n");
        return false;
    }

    // Add the address and type to the crc
    uint16_t crc = crc16(0xFFFF, addr);
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

bool check_message(uint8_t addr, uint8_t opcode, uint8_t *payload, uint8_t payload_length, uint8_t crc[2]) {
    if (addr != AVR_MASTER_ADDR) {
        //DEBUG("Not for us");
        return false;
    }

    if (opcode != AVR_OPCODE_RESPONSE) {
        //DEBUG("Not a response packet\n");
        return false;
    }

    // MSP430 and avr proto are both little endian, we can just cast the result
    uint16_t rcv_crc = *((uint16_t *) crc);
    //DEBUG("Recieved CRC: %d\n", rcv_crc);

    uint16_t cal_crc = crc16(0xFFFF, addr);
    cal_crc = crc16(cal_crc, opcode);
    cal_crc = crc16_all(cal_crc, payload, payload_length);

    //DEBUG("Calculated CRC: %d\n", cal_crc);

    if (rcv_crc != cal_crc) {
        //DEBUG("CRCs do not match: Ignoring\n");
        return false;
    }

    //DEBUG("CRCs match\n");

    return true;
}

void avr_set_callback(void (*cb)(bool isSuccess)) {
    callback = cb;
}

bool avr_get_data(struct avr_data *data) {
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

    DEBUG("Sending event for avr %x, size %d\n", data->id, data->size);

    return process_post(&avr_process, AVR_EVENT_GET_DATA, data) == PROCESS_ERR_OK;
}

void avr_set_output(void (*wb)(uint8_t *buf, int len)) {
    serial_write = wb;
}
