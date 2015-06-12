#ifndef SAMPLER_H
#define SAMPLER_H

	#include "contiki.h"
	#include "contiki-conf.h"
	#include <stdio.h>

	#include "config.h"
	// Sensors
	#include "sampling-sensors.h"
	#include "ms1-io.h"
	#include "filenames.h"

	// Config
	#include "settings.pb.h"
	#include "readings.pb.h"
	// Protobuf
	#include "pb_decode.h"
	#include "pb_encode.h"


	#include "z1-sampler-config-defaults.h"


	#include "dev/temperature-sensor.h"
	#include "dev/battery-sensor.h"
	#include "dev/protobuf-handler.h"
	#include "dev/event-sensor.h"
	#include "cfs/cfs.h"
#ifdef SPI_LOCKING
	#include "dev/cc1120.h"
	#include "dev/cc1120-arch.h"
#endif
	#include "platform-conf.h"


	void avr_timer_handler(void *p);
	void refreshSensorConfig(void);
	void refreshPosterConfig(void);

	PROCESS_NAME(sample_process);



	#define AVR_TIMEOUT_SECONDS 10
	//#define SAMPLE_SEND
	#ifdef SAMPLE_SEND
		#define IMMEDIATE_SEND 1
	#else
		#define IMMEDIATE_SEND 0
	#endif

#endif
