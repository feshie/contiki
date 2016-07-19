/**
 * @file
 * Definitions of the default configurations to use.
 */

#ifndef Z1_COAP_DEFAULTS_H
#define Z1_COAP_DEFAULTS_H

#include "settings.pb.h"
#include <stdbool.h>

/**
 * Default configuration to use when no configuration is curently defined.
 */
SensorConfig SENSOR_DEFAULT_CONFIG = {
    .interval = 1200,
    .has_avrID = false,
    .avrID = 0x00,
    .has_powerID = false,
    .powerID = 0x00,
    .hasADC1 = false,
    .hasADC2 = false,
    .hasRain = false,
    .routingMode = SensorConfig_RoutingMode_MESH
};

#endif // ifndef Z1_COAP_DEFAULTS_H
