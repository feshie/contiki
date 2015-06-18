#ifndef SAMPLER_H
#define SAMPLER_H

#include "contiki.h"
#include "contiki-conf.h"
#include <stdio.h>

#include "store.h"

// Sensors
#include "sampling-sensors.h"
#include "ms1-io.h"

// Config
#include "settings.pb.h"
#include "readings.pb.h"

#include "z1-coap-config-defaults.h"

#include "dev/temperature-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/protobuf-handler.h"
#include "dev/event-sensor.h"
#include "platform-conf.h"

void avr_timer_handler(void *p);
void refreshSensorConfig(void);

PROCESS_NAME(sample_process);

#define AVR_TIMEOUT_SECONDS 10

#endif // ifndef SAMPLER_H
