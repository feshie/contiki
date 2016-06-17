#include <string.h>
#include <stdint.h>
#include "contiki-conf.h"
#include "contiki.h"
#include "reset-sensor.h"
#include "dev/rom-util.h"
#include "dev/cc2538-dev.h"

/**
 * Reset counter in ROM
 * On first boot after being flashed, with will be 0xFFFFFFFF (due to being erased by the flasher),
 * which will naturally wrap around to 0.
 */
uint32_t const volatile * const reset_counter = (uint32_t *) CC2538_DEV_FLASH_ADDR;

// Ensure we have enough room in the reserved space for the reset_counter
_Static_assert(sizeof(*reset_counter) < RESET_SENSOR_SIZE, "Reset sensor size mismatch");

const struct sensors_sensor reset_sensor; 

static int reset_counter_get(int type) {
    return *reset_counter;
}

static int reset_counter_update(int type, int c) {
    uint32_t count = *reset_counter + 1;

    // We can only change bits from 1 -> 0 without erasing the whole page
    reset_counter_reset();
    rom_util_program_flash(&count, (uintptr_t) reset_counter, sizeof(*reset_counter));

    return reset_counter_get(0);
}

void reset_counter_reset(void) {
    rom_util_page_erase((uintptr_t) reset_counter, sizeof(*reset_counter));
}

static int reset_counter_status(int type) {
    return 1;
}

SENSORS_SENSOR(reset_sensor, "Resets", reset_counter_get, reset_counter_update, reset_counter_status);
