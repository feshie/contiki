#ifndef ER_SERVER_H
#define ER_SERVER_H

#include <stdint.h>
#include "contiki.h"
#include "cfs/cfs.h"
#include "sampling-sensors.h"
#include "dev/reset-sensor.h"
#include "z1-coap-config-defaults.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Config
#include "settings.pb.h"
#include "readings.pb.h"

// Networking
#include "contiki-net.h"

PROCESS_NAME(er_server_process);

#endif
