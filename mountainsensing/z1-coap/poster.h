#ifndef POSTER_H
#define POSTER_H
	#include "filenames.h"
	#include "config.h"
	#include "contiki.h"
	#include "contiki-conf.h"
	#include <stdio.h>
	#include <stdint.h>
	#include "z1-coap-config-defaults.h"
	#include "settings.pb.h"
	#include "cfs/cfs.h"
	// Networking
	#include "contiki-net.h"
	#include "sampling-sensors.h"
	#include "uip-ds6.h"
	#include "uip-debug.h"
#ifdef SPI_LOCKING
	#include "dev/cc1120.h"
	#include "dev/cc1120-arch.h"
#endif
	#include "platform-conf.h"

	#define CONNECTION_RETRIES 5
	#define LIVE_CONNECTION_TIMEOUT 20
	#define DATA_BUFFER_LENGTH 256
	#define PSOCK_BUFFER_LENGTH 120

	int handle_connection(char *data_buffer, uint8_t data_length, uint8_t *http_status, struct psock *p,  uint8_t *psock_buffer);
	uint8_t load_file(char *data_buffer, char *filename);

	#define POSTDEFBUG
	#ifdef POSTDEFBUG
	    #define PPRINT(...) printf(__VA_ARGS__)
	#else
	    #define PPRINT(...)
	#endif

#endif
