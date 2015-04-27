#ifndef SAMPLER_CONFIG_H
#define SAMPLER_CONFIG_H

	#define SAMPLE_CONFIG 1
	#define COMMS_CONFIG 2

	#include <stdlib.h>
	// Config
	#include "settings.pb.h"
	#include "readings.pb.h"

	// Protobuf
	#include "dev/pb_decode.h"
	#include "dev/pb_encode.h"

	#include "cfs/cfs.h"
	#include "contiki.h"

	#include <stdlib.h>
	#include <stdio.h>
#ifdef SPI_LOCKING
	#include "dev/cc1120.h"
	#include "dev/cc1120-arch.h"
#endif
	#include "contiki-conf.h"
	#include "platform-conf.h"
	#include "net/netstack.h"

	uint8_t set_config(void *pb, uint8_t config);
	uint8_t get_config(void *pb, uint8_t config);
	void print_sensor_config(SensorConfig *conf);
	void print_comms_config(POSTConfig *conf);

#endif
