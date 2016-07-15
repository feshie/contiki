#include "power-sheriff.h"

/**
 * Number of unterminated high power requests
 */
static volatile int high_power_requests = 0;

void power_sheriff_low_power(void) {
    high_power_requests--;

    if (high_power_requests <= 0) {
        lpm_set_max_pm(POWER_SHERIFF_LOW_POWER);
    }
}

void power_sheriff_high_power(void) {
    high_power_requests++;

    lpm_set_max_pm(POWER_SHERIFF_HIGH_POWER);
}
