#ifndef MS_WEBSERVER_H
#define MS_WEBSERVER_H
	#include <stdint.h>
	#include "contiki.h"
	#include "cfs/cfs.h"
	#include "sampling-sensors.h"
	#include "dev/reset-sensor.h"
	#include "sampler.h"
	#include "poster.h"
	#include "web_defines.h"
	#include "z1-sampler-config-defaults.h"
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	// Config
	#include "settings.pb.h"
	#include "readings.pb.h"

	// Networking
	#include "contiki-net.h"
	#include "net/netstack.h"

	#define URL_PARAM_LENGTH 8
	#define WEB_BUFF_LENGTH 128
	#define URL_LENGTH (WEB_BUFF_LENGTH - 4)

	PROCESS_NAME(web_process);
	uint8_t get_url_param(char *par, char *url, char *key);
	int flash_du(int *filec, uint32_t *bytes);

#endif