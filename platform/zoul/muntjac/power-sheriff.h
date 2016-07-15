#include "lpm.h"

#define POWER_SHERIFF_HIGH_POWER LPM_PM0

#define POWER_SHERIFF_LOW_POWER LPM_CONF_MAX_PM

/**
 * Enable low power mode.
 * Allows the CC2538 to come down POWER_MGR_LOW_POWER,
 * as long as no other unfinished requests for high power mode have been made.
 */
void power_sheriff_low_power(void);

/**
 * Enable high power mode.
 * Prevents the CC2538 from going past POWER_MGR_HIGH_POWER.
 */
void power_sheriff_high_power(void);

