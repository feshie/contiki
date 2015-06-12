#ifndef CC1120_H
#define CC1120_H

/* Contiki Headers. */
#include "contiki.h"
#include "contiki-conf.h"

/* Network Headers. */
#include "dev/radio.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"


/* CC1120 headers. */
#include "cc1120-const.h"
#include "cc1120-config.h"
#include "cc1120-arch.h"

/* Platform Headers */
#include "platform-conf.h"

/* Misc Headers. */
#include <string.h>
#include <stdio.h>

#define CC1120_MAX_PAYLOAD 125
#define CC1120_MIN_PAYLOAD 3

#if CC1120_FHSS_FCC_50 && CC1120_FHSS_ETSI_50
#error Error: FHSS, both CC1120_FHSS_ETSI_50 and CC1120_FHSS_FCC_50 defined. Please set only one.
#endif

#if CC1120_FHSS_ETSI_50
/* ETSI EN 300 220, 50 channels: Channel 0 863 Mhz, channel 49 869.125MHz.
 * The "BASE_FREQ" is the FREQ[2,1,0] setting for 863.000000MHz. Channel spacing 
 * is 125KHz. Actual register setting is worked out as FREQ[2,1,0] = CC1120_BASE_FREQ + 
 * (Channel * CC1120_CHANNEL_MULTIPLIER) */
#define CC1120_BASE_FREQ	0x6BE000
#define CC1120_CHANNEL_MULTIPLIER	0x400

#elif CC1120_FHSS_FCC_50
/* FHSS 902 -- 928 MHz (FCC Part 15.247; 15.249). BASE_FREQ gives 902.000000MHz.
 * Actual FREQ[2,1,0] is worked out as per the description for 868MHz. */
#define CC1120_BASE_FREQ	0x70C000
#define CC1120_CHAN_MULTI	1024


#else
#error Unknown FHSS frequencies, please define CC1120_FHSS_ETSI_50 or CC1120_FHSS_FCC_50
#endif

extern const struct radio_driver cc11xx_driver;

/* --------------------- Radio Driver Functions ---------------------------- */
int cc1120_driver_init(void);
int cc1120_driver_prepare(const void *payload, unsigned short len);
int cc1120_driver_transmit(unsigned short transmit_len);
int cc1120_driver_send_packet(const void *payload, unsigned short payload_len);
int cc1120_driver_read_packet(void *buf, unsigned short buf_len);
int cc1120_driver_channel_clear(void);
int cc1120_driver_receiving_packet(void);
int cc1120_driver_pending_packet(void);
int cc1120_driver_on(void);
int cc1120_driver_off(void);


/* -------------------- CC1120 Support Functions --------------------------- */
uint8_t cc1120_set_channel(uint8_t channel);
uint8_t cc1120_get_channel(void);
uint8_t cc1120_read_txbytes(void);
uint8_t cc1120_read_rxbytes(void);

/* ---------------------- CC1120 SPI Functions ----------------------------- */
uint8_t cc1120_spi_cmd_strobe(uint8_t strobe);
uint8_t cc1120_spi_single_read(uint16_t addr);
uint8_t cc1120_spi_single_write(uint16_t addr, uint8_t val);

void CC1120_LOCK_SPI(void);
void CC1120_RELEASE_SPI(void);


/* --------------------- CC1120 State Functions ---------------------------- */
uint8_t cc1120_set_state(uint8_t state);
uint8_t cc1120_get_state(void);



/* -------------------- CC1120 Interrupt Handler --------------------------- */
int cc1120_interrupt_handler(void);


#endif /* CC11xx_H */
