/**
 * \file
 *         TI CC1120 driver.
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 *         Phil Basford <pjb@ecs.soton.ac.uk>
 *	ECS, University of Southampton
 */
 
#include "contiki.h"
#include "contiki-conf.h"

#include <watchdog.h>

/* TURN OFF LEDS COMPLETELY TO SAVE POWER */
#undef CC1120LEDS
#if CC1120LEDS
#include "dev/leds.h"
#endif

/* CC1120 headers. */
#include "cc1120.h"
#include "cc1120-arch.h"
#include "cc1120-config.h"

#include "net/rime/rime.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "net/mac/contikimac/contikimac.h"


/* LEDs. */
#undef LEDS_ON
#undef LEDS_OFF
#if CC1120LEDS
#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#else
#define LEDS_ON(x)
#define LEDS_OFF(x)
#endif

/* Printf definitions for debug */
#if CC1120DEBUG || DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#if CC1120DEBUG || CC1120RXDEBUG || DEBUG
#define PRINTFRX(...) printf(__VA_ARGS__)
#else
#define PRINTFRX(...) do {} while (0)
#endif

#if CC1120DEBUG || CC1120RXDEBUG || DEBUG
#define PRINTFRXERR(...) printf(__VA_ARGS__)
#else
#define PRINTFRXERR(...) do {} while (0)
#endif

#if CC1120DEBUG || DEBUG || CC1120TXDEBUG
#define PRINTFTX(...) printf(__VA_ARGS__)
#else
#define PRINTFTX(...) do {} while (0)
#endif

#if CC1120DEBUG || CC1120TXERDEBUG || DEBUG || CC1120TXDEBUG
#define PRINTFTXERR(...) printf(__VA_ARGS__)
#else
#define PRINTFTXERR(...) do {} while (0)
#endif

#if CC1120DEBUG || CC1120INTDEBUG || DEBUG
#define PRINTFINT(...) printf(__VA_ARGS__)
#else
#define PRINTFINT(...) do {} while (0)
#endif

#if CC1120DEBUG || CC1120INTDEBUG || CC1120RXERDEBUG || CC1120RXDEBUG || DEBUG
#define PRINTFINTRX(...) printf(__VA_ARGS__)
#else
#define PRINTFINTRX(...) do {} while (0)
#endif

#if C1120PROCESSDEBUG		
#define PRINTFPROC(...) printf(__VA_ARGS__)
#else
#define PRINTFPROC(...) do {} while (0)
#endif

#if CC1120STATEDEBUG
#define PRINTFSTATE(...) printf(__VA_ARGS__)
#else
#define PRINTFSTATE(...) do {} while (0)
#endif					


/* Busy Wait for time-outable waiting. */
#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)


/*Define default RSSI Offset if it is not defined. */
#ifndef CC1120_RSSI_OFFSET
#define CC1120_RSSI_OFFSET		0x9A
#endif

#ifndef CC1200_RSSI_OFFSET
#define CC1200_RSSI_OFFSET		0x9D
#endif

#define CC1120_ACK_PENDING				0x01			
#define CC1120_RX_FIFO_OVER				0x02
#define CC1120_RX_FIFO_UNDER			0x04
#define CC1120_TX_FIFO_ERROR			0x08
#define CC1120_RX_ON					0x10
#define CC1120_TX_COMPLETE				0x20
#define CC1120_TRANSMITTING				0x40
#define CC1120_INTERRUPT_PENDING		0x80

#define ACK_LEN 3
#define ACK_FRAME_CONTROL_LSO	0x02
#define ACK_FRAME_CONTROL_MSO	0x00

#define CC1120_802154_FCF_ACK_REQ			0x20
#define CC1120_802154_FCF_DEST_ADDR_16BIT	0x08
#define CC1120_802154_FCF_DEST_ADDR_64BIT	0x0C


/* -------------------- Internal Function Definitions. -------------------- */
static void on(void);
static void off(void);
static void processor(void);
static void cc1120_gpio_config(void);
static void cc1120_misc_config(void);

static radio_result_t set_value(radio_param_t param, radio_value_t value);
static radio_result_t get_value(radio_param_t param, radio_value_t *value);
static radio_result_t set_object(radio_param_t param, const void *src, size_t size);
static radio_result_t get_object(radio_param_t param, void *dest, size_t size);

/* ---------------------- CC1120 SPI Functions ----------------------------- */
static void cc1120_spi_disable(void);
static uint8_t cc1120_spi_write_addr(uint16_t addr, uint8_t burst, uint8_t rw);
static void cc1120_write_txfifo(uint8_t *payload, uint8_t payload_len);

/* ------------------- CC1120 State Set Functions -------------------------- */
static uint8_t cc1120_set_idle(void);
static uint8_t cc1120_set_rx(void);
static uint8_t cc1120_set_tx(void);
static uint8_t cc1120_flush_rx(void);
static uint8_t cc1120_flush_tx(void);


PROCESS(cc1120_process, "CC1120 driver");

/* -------------------- Radio Driver Structure ---------------------------- */
const struct radio_driver cc1120_driver = {
	cc1120_driver_init,
	cc1120_driver_prepare,
	cc1120_driver_transmit,
	cc1120_driver_send_packet,
	cc1120_driver_read_packet,
	cc1120_driver_channel_clear,
	cc1120_driver_receiving_packet,
	cc1120_driver_pending_packet,
	cc1120_driver_on,
	cc1120_driver_off,
	get_value,
	set_value,
	get_object,
	set_object
};



/* ------------------- Internal variables -------------------------------- */
static volatile uint8_t ack_tx, current_channel, next_channel, packet_pending, broadcast, ack_seq, tx_seq = 0;
static volatile uint8_t rx_rssi, rx_lqi, lbt_success, radio_pending, txfirst, txlast, tx_err = 0;
static volatile uint8_t locked, lock_on, lock_off, radio_part = 0;

static uint8_t ack_buf[ACK_LEN];
static uint8_t tx_buf[CC1120_MAX_PAYLOAD];

static uint8_t tx_len;

/* ------------------- Radio Driver Functions ---------------------------- */
int 
cc1120_driver_init(void)
{
	PRINTF("**** CC1120 Radio  Driver: Init ****\n");
	
	/* Initialise arch  - pins, spi, turn off cc2420 */
	cc1120_arch_init();
	cc1120_arch_pin_init();
	
	/* Reset CC1120 */
	cc1120_arch_reset();
	
	/* Check CC1120 - we read the part number register as a test. */
	radio_part = cc1120_spi_single_read(CC1120_ADDR_PARTNUMBER);
	
	switch(radio_part) {
		case CC1120_PART_NUM_CC1120:
			printf("CC1120");
			cc1120_register_config();
      
      		cc1120_spi_single_write(CC1120_ADDR_PKT_CFG1, 0x05);		                    /* Set PKT_CFG1 - CRC Configured as 01 and status bytes appended, No data whitening, address check or byte swap.*/
      		cc1120_spi_single_write(CC1120_ADDR_FIFO_CFG, 0x80);		                    /* Set RX FIFO CRC Auto flush. */		
      		cc1120_spi_single_write(CC1120_ADDR_AGC_GAIN_ADJUST, (CC1120_RSSI_OFFSET));	/* Set the RSSI Offset. This is a two's compliment number. */
      		cc1120_spi_single_write(CC1120_ADDR_AGC_CS_THR, (CC1120_CS_THRESHOLD));   	/* Set Carrier Sense Threshold. This is a two's compliment number. */

			break;
		case CC1120_PART_NUM_CC1121:
			printf("CC1121");
			break;
		case CC1120_PART_NUM_CC1125:
			printf("CC1125");
			break;
		case CC1120_PART_NUM_CC1200:
			printf("CC1200");
			cc1200_register_config();
      
      		cc1120_spi_single_write(CC1200_ADDR_PKT_CFG1, 0x03);		                    /* Set PKT_CFG1 - CRC Configured as 01 and status bytes appended, No data whitening, address check or byte swap.*/
	  		cc1120_spi_single_write(CC1200_ADDR_FIFO_CFG, 0x80);		                    /* Set RX FIFO CRC Auto flush. */		
      		cc1120_spi_single_write(CC1200_ADDR_AGC_GAIN_ADJUST, (CC1200_RSSI_OFFSET));	/* Set the RSSI Offset. This is a two's compliment number. */
      		cc1120_spi_single_write(CC1200_ADDR_AGC_CS_THR, (CC1120_CS_THRESHOLD));   	/* Set Carrier Sense Threshold. This is a two's compliment number. */

			break;
		case CC1120_PART_NUM_CC1201:
			printf("CC1201");
			break;
		default:	/* Not a supported chip or no chip present... */
			printf("*** ERROR: No Radio ***\n");
			while(1)	/* Spin ad infinitum as we cannot continue. */
			{
				watchdog_periodic();	/* Feed the dog to stop reboots. */
			}
			break;
	}
	
	// TODO: Cover sync-word errata somewhere?
	
	/* Configure CC1120 */
	cc1120_gpio_config();
	cc1120_misc_config();
	
	printf(" detected & Configured\n\r");
	
	/* Set Channel */
	cc1120_set_channel(RF_CHANNEL);                            
	
    /* Set radio off */
	cc1120_driver_off();
	
	process_start(&cc1120_process, NULL);
	
	/* Enable CC1120 interrupt. */
	cc1120_arch_interrupt_enable();
	
	PRINTF(" Init\n");
	return 1;
}

int
cc1120_driver_prepare(const void *payload, unsigned short len)
{
	PRINTFTX("**** Radio Driver: Prepare ****\n");

	if(len > CC1120_MAX_PAYLOAD) {
		/* Packet is too large - max packet size is 125 bytes. */
		PRINTFTXERR("!!! PREPARE ERROR: Packet too large. !!!\n");
		return RADIO_TX_ERR;
	}
	
	PRINTFTX("\t%d bytes\n", len);
	
	/* Make sure that the TX FIFO is empty as we only want a single packet in it. */
	cc1120_flush_tx();
			
	/* Write to the FIFO. */
	cc1120_write_txfifo((uint8_t *)payload, len);
	ack_tx  = 0;
	
	/* Keep a local copy of the FIFO. */
	memcpy(tx_buf, payload, len);
	tx_len = len;
	RIMESTATS_ADD(lltx);
	
	if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_null)) {
		broadcast = 1;
		PRINTFTX("\tBroadcast\n");
	} else {
		broadcast = 0;
		PRINTFTX("\tUnicast, Seqno = %d\n", tx_seq);
	}
	tx_seq = ((uint8_t *)payload)[2];   //packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);

	lbt_success = 0;
	
	/* If radio is meant to be in RX, put it back in. */
	if(radio_pending & CC1120_RX_ON) {	
		cc1120_set_state(CC1120_STATE_RX);
	}
	
	return RADIO_TX_OK;
}

int
cc1120_driver_transmit(unsigned short transmit_len)
{
	PRINTFTX("\n\n**** Radio Driver: Transmit ****\n");
	uint8_t txbytes, cur_state, marc_state;
	rtimer_clock_t t0;
	watchdog_periodic();	/* Feed the dog to stop reboots. */
	
	/* Check that the packet is not too large. */
	if(transmit_len > CC1120_MAX_PAYLOAD) {
		/* Packet is too large - max packet size is 125 bytes. */
		PRINTFTX("!!! TX ERROR: Packet too large. !!!\n");

		return RADIO_TX_ERR;
	}
	CC1120_LOCK_SPI();
	radio_pending |= CC1120_TRANSMITTING;
	radio_pending &= ~(CC1120_TX_COMPLETE);
	
	LEDS_ON(LEDS_GREEN);
	
	if(ack_tx == 1) {
		/* Wrong data in the FIFO. Re-write the FIFO. */
		cc1120_flush_tx();
		cc1120_write_txfifo(tx_buf, tx_len);
		ack_tx = 0;
	}
	
	/* check if this is a retransmission */
	txbytes = cc1120_read_txbytes();
	if((txbytes == 0) && (txfirst != txlast)) {
		PRINTFTX("\tRetransmit last packet.\n");

		/* Retransmit last packet. */
		cc1120_set_state(CC1120_STATE_IDLE);
		
		/* These registers should only be written in IDLE. */
		cc1120_spi_single_write(CC1120_ADDR_TXFIRST, txfirst);
		cc1120_spi_single_write(CC1120_ADDR_TXLAST, txlast);
		txbytes = cc1120_read_txbytes();
	}
	
	if(txbytes != tx_len + 1) {
		/* re-load the FIFO. */
		cc1120_flush_tx();
		cc1120_write_txfifo(tx_buf, tx_len);
		ack_tx = 0;
	}

	/* Store TX Pointers. */
	txfirst =  cc1120_spi_single_read(CC1120_ADDR_TXFIRST);
	txlast = cc1120_spi_single_read(CC1120_ADDR_TXLAST);
	
	/* Set correct TXOFF mode. */
	if(broadcast) {
		/* Not expecting an ACK. */
		cc1120_spi_single_write(CC1120_ADDR_RFEND_CFG0, 0x00);	/* Set TXOFF Mode to IDLE as we are not expecting ACK. */
	} else {
		/* Expecting an ACK. */
		cc1120_spi_single_write(CC1120_ADDR_RFEND_CFG0, 0x30);	/* Set TXOFF Mode to RX as we are expecting ACK. */
	}
	
#if RDC_CONF_HARDWARE_CSMA
	/* If we use LBT... */
	if(lbt_success == 0) {
		PRINTFTX("\tTransmitting with LBT.\n");
		
		/* Set RX if radio is not already in it. */
		if(cc1120_get_state() != CC1120_STATUS_RX) {   
			PRINTFTX("\tEnter RX for CCA.\n");		
			cc1120_set_state(CC1120_STATE_RX);
		}
		
		PRINTFTX("\tWait for valid RSSI.");
		
		/* Wait for RSSI to be valid. */
		while(!(cc1120_spi_single_read(CC1120_ADDR_RSSI0) & (CC1120_RSSI_VALID))) {
			PRINTFTX(".");
			watchdog_periodic();	/* Feed the dog to stop reboots. */
		}
		PRINTFTX("\n");
		PRINTFTX("\tTX: Enter TX\n");
	} else {
		/* Retransmitting last packet not using LBT. */
		PRINTFTX("Retransmitting last  packet.\n");
		if(cc1120_get_state() == CC1120_STATUS_RX) {   
			PRINTFTX("\tEnter IDLE.\n");		
			cc1120_set_state(CC1120_STATE_IDLE);
		}
	}

	t0 = RTIMER_NOW();
	cc1120_spi_cmd_strobe(CC1120_STROBE_STX);	/* Strobe TX. */
	cur_state = cc1120_get_state();
	
	/* Block until in TX.  If timeout is reached, strobe IDLE and 
	 * reset CCA to clear TX & flush FIFO. */
#if CC1120_GPIO_MODE == 0
	while(!(radio_pending & CC1120_TX_COMPLETE)) {
#elif CC1120_GPIO_MODE == 2
	while(cc1120_arch_read_gpio2()) {
#elif CC1120_GPIO_MODE == 3
	while(cc1120_arch_read_gpio3()) {
#endif
		watchdog_periodic();	/* Feed the dog to stop reboots. */
		
		if(RTIMER_CLOCK_LT((t0 + CC1120_LBT_TIMEOUT), RTIMER_NOW()) ) {
			/* Timeout reached. */
			cc1120_set_state(CC1120_STATE_IDLE);
			
			// TODO: Do we need to reset the CCA mode?
			
			/* Set Energest and TX flag. */
			ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
			radio_pending &= ~(CC1120_TRANSMITTING);
				
			/* Turn off LED if it is being used. */
			LEDS_OFF(LEDS_GREEN);	

			cc1120_flush_tx();		
			CC1120_RELEASE_SPI();	
			
			PRINTFTXERR("!!! TX ERROR: Collision before TX - Timeout reached. !!!\n");
			RIMESTATS_ADD(contentiondrop);
			lbt_success = 0;
			
			if(radio_pending & CC1120_RX_ON) {
				on();
			}
			/* Return Collision. */
			return RADIO_TX_COLLISION;
		} else if (radio_pending & CC1120_TX_FIFO_ERROR) {
			/* Set Energest and TX flag. */
			ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
			radio_pending &= ~(CC1120_TRANSMITTING);
			cc1120_flush_tx();

			PRINTFTXERR("!!! TX ERROR: FIFO Error. !!!\n");	
			LEDS_OFF(LEDS_GREEN);		/* Turn off LED if it is being used. */
			
			CC1120_RELEASE_SPI();
			lbt_success = 0;
			if(radio_pending & CC1120_RX_ON) {
				on();
			}
			return RADIO_TX_ERR;
		}
		cur_state = cc1120_get_state();
	}
	lbt_success = 1;
	
#else /* RDC_CONF_HARDWARE_CSMA */
	PRINTFTX("\tTransmitting without LBT.\n");
	PRINTFTX("\tTX: Enter TX\n");

	/* Enter TX. */
	cur_state = cc1120_set_state(CC1120_STATE_TX);

	if(cur_state != CC1120_STATUS_TX) {
		/* We didn't TX... */
		radio_pending &= ~(CC1120_TRANSMITTING);

		PRINTFTXERR("!!! TX ERROR: did not enter TX. Current state = %02x !!!\n", cur_state);
		
		if(radio_pending & CC1120_TX_FIFO_ERROR) {
			cc1120_flush_tx();
		}

		CC1120_RELEASE_SPI();
		LEDS_OFF(LEDS_GREEN);	/* Turn off LED if it is being used. */			
		
		if(radio_pending & CC1120_RX_ON) {
			on();
		}	
		
		return RADIO_TX_ERR;
	}
#endif /* WITH_SEND_CCA */	
	
	PRINTFTX("\tTX: in TX.");
	ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
	t0 = RTIMER_NOW();
	
	/* Block till TX is complete. */	
#if CC1120_GPIO_MODE == 0
	/* Wait for CC1120 interrupt handler to set CC1120_TX_COMPLETE. */
	while(!(radio_pending & CC1120_TX_COMPLETE)) {
#elif CC1120_GPIO_MODE == 2
	PRINTFTX(" - Wait for GPIO2");
	while(cc1120_arch_read_gpio2()) {
#elif CC1120_GPIO_MODE == 3
	PRINTFTX(" - Wait for GPIO3");
	while(cc1120_arch_read_gpio3()) {
#endif
		
		watchdog_periodic();	/* Feed the dog to stop reboots. */
		PRINTFTX(".");
		
		if(radio_pending & CC1120_TX_FIFO_ERROR) {
			/* TX FIFO has underflowed during TX.  Need to flush TX FIFO. */
			cc1120_flush_tx();
			txfirst = 0;
			txlast = 0;
			break;
		}
		
		if(RTIMER_CLOCK_LT((t0 + RTIMER_SECOND/20), RTIMER_NOW())) {
			/* Timeout for TX. At 802.15.4 50kbps data rate, the entire 
			 * TX FIFO (all 128 bits) should be transmitted in 0.02 
			 * seconds. Timeout set to 0.05 seconds to be sure.  If 
			 * the interrupt has not fired by this time then something 
			 * went wrong. 
			 * 
			 * This timeout needs to be adjusted if lower data rates 
			 * are used. */	 
			if((cc1120_read_txbytes() == 0) && !(radio_pending & CC1120_TX_FIFO_ERROR)) {
				/* We have actually transmitted everything in the FIFO. */
				radio_pending |= CC1120_TX_COMPLETE;
				PRINTFTXERR("!!! TX ERROR: TX timeout reached but packet sent !!!\n");
			} else {
				cc1120_set_state(CC1120_STATE_IDLE);
				PRINTFTXERR("!!! TX ERROR: TX timeout reached !!!\n");						
			}
			break;
		}
	}
	
	PRINTFTX("\n");	
	ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);			
	LEDS_OFF(LEDS_GREEN);
	CC1120_RELEASE_SPI();
	radio_pending &= ~(CC1120_TRANSMITTING);
	t0 = RTIMER_NOW();
		
	if(cc1120_read_txbytes() > 0) {			
		if(tx_err > CC1120_TX_ERR_COUNT) {
			/* Reset the radio. */
			PRINTFTXERR("\tTX !OK - Radio reset... ");
			cc1120_arch_reset();
			radio_part = cc1120_spi_single_read(CC1120_ADDR_PARTNUMBER);
			switch(radio_part) {
				case CC1120_PART_NUM_CC1120:
					cc1120_register_config();
					cc1120_spi_single_write(CC1120_ADDR_PKT_CFG1, 0x05);
					cc1120_spi_single_write(CC1120_ADDR_FIFO_CFG, 0x80);		
					cc1120_spi_single_write(CC1120_ADDR_AGC_GAIN_ADJUST, (CC1120_RSSI_OFFSET));
					cc1120_spi_single_write(CC1120_ADDR_AGC_CS_THR, (CC1120_CS_THRESHOLD));   	
					break;
				case CC1120_PART_NUM_CC1121:
					break;
				case CC1120_PART_NUM_CC1125:
					break;
				case CC1120_PART_NUM_CC1200:
					cc1200_register_config();
					cc1120_spi_single_write(CC1200_ADDR_PKT_CFG1, 0x03);		                    
					cc1120_spi_single_write(CC1200_ADDR_FIFO_CFG, 0x80);		                    		
					cc1120_spi_single_write(CC1200_ADDR_AGC_GAIN_ADJUST, (CC1120_RSSI_OFFSET));	
					cc1120_spi_single_write(CC1200_ADDR_AGC_CS_THR, (CC1120_CS_THRESHOLD));   	
					break;
				case CC1120_PART_NUM_CC1201:
					break;
				default:	/* Not a supported chip or no chip present... */
					printf("*** ERROR: No Radio ***\n");
					while(1)	/* Spin ad infinitum as we cannot continue. */
					{
						watchdog_periodic();	/* Feed the dog to stop reboots. */
					}
					break;
			}
			cc1120_set_channel(next_channel);
			tx_err = 0;
			txfirst = 0;
			txlast = 0;
			printf("OK!\n");
			
		} else {
			PRINTFTXERR("\tTX !OK %d.\n", cc1120_read_txbytes() );
			tx_err++;	
			cc1120_flush_tx();		/* Flush TX FIFO. */
			cc1120_flush_rx();		/* Flush RX FIFO. */
		}
		radio_pending &= ~(CC1120_ACK_PENDING);
		if(radio_pending & CC1120_RX_ON) {
			on();
		}
		
		return RADIO_TX_ERR;
	} else {
		PRINTFTX("\tTX OK.\n");
		tx_err = 0;

		cur_state = cc1120_get_state();
		marc_state = cc1120_spi_single_read(CC1120_ADDR_MARCSTATE) & 0x1F;	
		
		if((marc_state == CC1120_MARC_STATE_MARC_STATE_TX_END) && (cur_state == CC1120_STATUS_TX)) {
			cc1120_set_state(CC1120_STATE_IDLE);
		} else if(cur_state == CC1120_STATUS_TX) {
			/* Should never get here, just here for security. */
			printf("!!!!! TX ERROR: Still in TX according to status byte. !!!\n");
			cc1120_set_state(CC1120_STATE_IDLE);
		}
		
		if(broadcast) {
			/* We have TX'd broadcast successfully. We are not expecting 
			 * an ACK so clear RXFIFO incase, wait for interpacket and return OK. */
			PRINTFTX("\tBroadcast TX OK\n");
			cc1120_flush_rx();	
			radio_pending &= ~(CC1120_ACK_PENDING);	/* NOT expecting and ACK. */	
			cc1120_flush_tx();
			while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CC1120_INTER_PACKET_INTERVAL)) { 
					watchdog_periodic();
			}
			if(radio_pending & CC1120_RX_ON) {
				on();
			}
			
			return RADIO_TX_OK;	
		} else {
			/* We have successfully sent the packet but want an ACK. */
			if(packet_pending) {
				/* There is a packet inthe FIFO that should not be there */
				cc1120_flush_rx();
			}
			if(cc1120_get_state() != CC1120_STATUS_RX) {
				/* Need to be in RX for the ACK. */
				cc1120_set_state(CC1120_STATE_RX);
			}
			radio_pending |= CC1120_ACK_PENDING;
			if(tx_seq == 0) {
				ack_buf[2] = 128;	/* Handle wrapping sequence numbers. */
			} else {
				ack_buf[2] = 0;
			}
			
			/* Wait for the ACK. */
			while((RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CC1120_INTER_PACKET_INTERVAL))) {
				/* Wait till timeout or ACK is received. */
				watchdog_periodic();		/* Feed the dog to stop reboots. */
			}
			
			if(cc1120_get_state() != CC1120_STATUS_RX || cc1120_read_rxbytes() > 0) {
				/* We have received something. */
				transmit_len = cc1120_spi_single_read(CC1120_FIFO_ACCESS);
				
				if(transmit_len == 3) {
					/* It is an ACK. */
					CC1120_LOCK_SPI();

					cc1120_arch_spi_enable();
					cc1120_arch_rxfifo_read(ack_buf, transmit_len);
					cc1120_spi_disable();
					
					ack_seq = ack_buf[2];	
				
					CC1120_RELEASE_SPI();
				}	
			}
			
			radio_pending &= ~(CC1120_ACK_PENDING);
			cc1120_flush_tx();
			cc1120_flush_rx();	
			if(radio_pending & CC1120_RX_ON) {
				on();
			}
	
			if(ack_buf[2] == tx_seq) {
				/* We have received the required ACK. */
				PRINTFTX("\tACK Rec %d\n", ack_buf[2]);
				return RADIO_TX_OK;
			} else {
				/* No ACK received. */
				PRINTFTX("\tNo ACK received.\n");
				return RADIO_TX_NOACK;
			}
		}	
		cc1120_flush_tx();
		if(radio_pending & CC1120_RX_ON) {
			on();
		}
		return RADIO_TX_OK;
	}
	cc1120_flush_tx();
	if(radio_pending & CC1120_RX_ON) {
		on();
	}
	return RADIO_TX_ERR;
}

int
cc1120_driver_send_packet(const void *payload, unsigned short payload_len)
{
	PRINTFTX("**** Radio Driver: Send ****\n");
	if(cc1120_driver_prepare(payload, payload_len) != RADIO_TX_OK) {
		return RADIO_TX_ERR;
	}
	
	return cc1120_driver_transmit(payload_len);
}

int
cc1120_driver_read_packet(void *buf, unsigned short buf_len)
{
	PRINTF("**** Radio Driver: Read ****\n");

	uint8_t length, i, rxbytes;
	linkaddr_t dest;
	rtimer_clock_t t0;   
		
	if(radio_pending & CC1120_RX_FIFO_UNDER) {
		/* FIFO underflow */
		cc1120_flush_rx();
		PRINTFRXERR("\tERROR: RX FIFO underflow.\n");		
		return 0;		
	}
	
	rxbytes = cc1120_read_rxbytes();
	
	if(rxbytes < CC1120_MIN_PAYLOAD) {
		/* not enough data. */
		cc1120_flush_rx();
		RIMESTATS_ADD(tooshort);
		
		PRINTFRXERR("\tERROR: Packet too short\n");
		return 0;
	}
	
	/* Read length byte. */
	length = cc1120_spi_single_read(CC1120_FIFO_ACCESS);
	
	if((length + 2) > rxbytes) {
		/* Packet too long, out of Sync? */
		cc1120_flush_rx();
			
		RIMESTATS_ADD(badsynch);
		PRINTFRXERR("\tERROR: not enough data in FIFO. Length = %d, rxbytes = %d\n", length, rxbytes);
		return 0;
	} else if(length > CC1120_MAX_PAYLOAD) {
		/* Packet too long, out of Sync? */
		cc1120_flush_rx();
		
		RIMESTATS_ADD(badsynch);
		PRINTFRXERR("\tERROR: Packet longer than FIFO or bad sync\n");
		return 0;
	} else if((length) > buf_len) {
		/* Packet is too long. */
		cc1120_flush_rx();
		
		RIMESTATS_ADD(toolong);
		PRINTFRXERR("\tERROR: Packet too long for buffer\n");	
		return 0;
	}
	
	CC1120_LOCK_SPI();
	PRINTFRX("\tPacket received.\n"); 
	watchdog_periodic();	/* Feed the dog to stop reboots. */
		
	/* We have received a normal packet. Read the packet. */
	PRINTFRX("\tRead FIFO.\n");

	cc1120_arch_spi_enable();
	cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_BURST_BIT | CC1120_READ_BIT);
	
	for(i = 0; i < length; i++) {
		((uint8_t *)buf)[i] = cc1120_arch_spi_rw_byte(0);
		
		if((i == 13) && (((uint8_t *)buf)[0] & CC1120_802154_FCF_ACK_REQ)) {
			/* ACK handling. */

			/* Get the address in the correct order. */
			if((((uint8_t *)buf)[1] & 0x0C) == 0x0C) {
				/* Long address. */
				dest.u8[7] = ((uint8_t *)buf)[5];
				dest.u8[6] = ((uint8_t *)buf)[6];
				dest.u8[5] = ((uint8_t *)buf)[7];
				dest.u8[4] = ((uint8_t *)buf)[8];
				dest.u8[3] = ((uint8_t *)buf)[9];
				dest.u8[2] = ((uint8_t *)buf)[10];
				dest.u8[1] = ((uint8_t *)buf)[11];
				dest.u8[0] = ((uint8_t *)buf)[12];
			} else if((((uint8_t *)buf)[1] & 0x08) == 0x08) {
				/* Short address. */
				dest.u8[1] = ((uint8_t *)buf)[5];
				dest.u8[0] = ((uint8_t *)buf)[6];
			}
			
			/* Work out if we need to send an ACK. */
			if(linkaddr_cmp(&dest, &linkaddr_node_addr)) {
				PRINTFRX("\tSending ACK\n");


				
				cc1120_spi_disable();                                                
				watchdog_periodic();	/* Feed the dog to stop reboots. */
				
				/* Populate ACK Frame buffer. */
				ack_buf[0] = ACK_FRAME_CONTROL_LSO;
				ack_buf[1] = ACK_FRAME_CONTROL_MSO;
				ack_buf[2] = ((uint8_t *)buf)[2];

				radio_pending |= CC1120_TRANSMITTING;				
				radio_pending &= ~(CC1120_TX_COMPLETE);
				
				/* Make sure that the TX FIFO is empty & write ACK to it. */
				cc1120_flush_tx();
				cc1120_write_txfifo(ack_buf, ACK_LEN);
				ack_tx = 1;
				
				/* Transmit ACK WITHOUT LBT. */
				cc1120_spi_cmd_strobe(CC1120_STROBE_STX);
				t0 = RTIMER_NOW(); 
				
				/* Block till TX is complete. */	
#if CC1120_GPIO_MODE == 0
					/* Wait for CC1120 interrupt handler to set CC1120_TX_COMPLETE. */
					while(!(radio_pending & CC1120_TX_COMPLETE)) {
#elif CC1120_GPIO_MODE == 2
					PRINTFTX(" - Wait for GPIO2");
					while(cc1120_arch_read_gpio2()) {
#elif CC1120_GPIO_MODE == 3
					PRINTFTX(" - Wait for GPIO3");
					while(cc1120_arch_read_gpio3()) {
#endif									
					watchdog_periodic();	/* Feed the dog to stop reboots. */
					PRINTFRX(".");
					
					if(radio_pending & CC1120_TX_FIFO_ERROR) {
						/* TX FIFO has underflowed during ACK TX.  Need to flush TX FIFO. */
						cc1120_flush_tx();
						break;
					}
					
					if(RTIMER_CLOCK_LT((t0 + RTIMER_SECOND/20), RTIMER_NOW())) {
						/* Timeout for TX. At 802.15.4 50kbps data rate, the entire 
						 * TX FIFO (all 128 bits) should be transmitted in 0.02 
						 * seconds. Timeout set to 0.05 seconds to be sure.  If 
						 * the interrupt has not fired by this time then something 
						 * went wrong. 
						 * 
						 * This timeout needs to be adjusted if lower data rates 
						 * are used. */	 
						if((cc1120_read_txbytes() == 0) && !(radio_pending & CC1120_TX_FIFO_ERROR)) {
							/* We have actually transmitted everything in the FIFO. */
							radio_pending |= CC1120_TX_COMPLETE;
							PRINTFTXERR("!!! TX ERROR: TX timeout reached but ACK sent !!!\n");
						} else {
							cc1120_set_state(CC1120_STATE_IDLE);
							PRINTFTXERR("!!! TX ERROR: TX timeout reached !!!\n");						
						}
						break;
					}
				}
				cc1120_flush_tx();			/* Make sure that the TX FIFO is empty. */
				radio_pending &= ~(CC1120_TRANSMITTING);
				
				cc1120_arch_spi_enable();	/* Re-enable burst access to read remaining data. */
				(void) cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_BURST_BIT | CC1120_READ_BIT);
			}
		}
	}
	cc1120_spi_disable();
	watchdog_periodic();
	
	PRINTFRX("\tPacketRead\n");
	
	if(radio_pending & CC1120_RX_FIFO_UNDER) {
		/* FIFO underflow */
		CC1120_RELEASE_SPI();
		cc1120_flush_rx();
		PRINTFRXERR("\tERROR: RX FIFO underflow. Meant to have %d bytes\n", length);	
		return 0;		
	}
	
	CC1120_RELEASE_SPI();
	
	rx_rssi = cc1120_spi_single_read(CC1120_FIFO_ACCESS);
	rx_lqi = cc1120_spi_single_read(CC1120_FIFO_ACCESS) & CC1120_LQI_MASK;
		
	if(radio_pending & CC1120_RX_FIFO_UNDER) {
		/* FIFO underflow */
		cc1120_flush_rx();
		PRINTFRXERR("\tERROR: RX FIFO underflow.\n");
		return 0;		
	}
	
	RIMESTATS_ADD(llrx);
	PRINTFRX("\tRX OK - %d byte packet.\n", length);
	
	cc1120_flush_rx();	/* Make sure that the RX FIFO is empty. */
	
	/* Return read length. */
	return length;
}

int
cc1120_driver_channel_clear(void)
{
	PRINTF("**** Radio Driver: CCA ****\n");
	uint8_t cca, cur_state, rssi0, was_off;
	rtimer_clock_t t0;
	
	was_off = 0;
#if CC1120_DEBUG_PINS
	cc1120_debug_pin_cca(1);
#endif

	if(locked) {
		PRINTF("SPI Locked\n");
#if CC1120_DEBUG_PINS
		cc1120_debug_pin_cca(0);
#endif		
		return 0;
	}

	if(radio_pending & CC1120_TRANSMITTING) {
		/* cannot be clear in TX. */
#if CC1120_DEBUG_PINS
		cc1120_debug_pin_cca(0);
#endif
		return 0;
	}
	
	if(!(radio_pending & CC1120_RX_ON)) {
		/* Radio is off for some reason. */
		was_off = 1;
		on();
	}
		
	CC1120_LOCK_SPI();
	
	cur_state = cc1120_get_state();
	
	while((cur_state == CC1200_STATUS_CALIBRATE) || (cur_state == CC1200_STATUS_SETTLING)) {
		cur_state = cc1120_get_state();
	}

	if(cur_state != CC1120_STATUS_RX ) {
		/* Not in RX... */
		on();
	}
	
	if(cc1120_spi_single_read(CC1120_ADDR_MODEM_STATUS1) & CC1120_MODEM_STATUS1_SYNC_FOUND) {
		/* Currently receving a packet, or at least a Sync word. */
		cca = 0;
	} else {
		/* Wait till the CARRIER_SENSE is valid. */
		watchdog_periodic();
		clock_wait(CLOCK_SECOND/200);	/* Wait for 5ms. */
		rssi0 = cc1120_spi_single_read(CC1120_ADDR_RSSI0);
		t0 = RTIMER_NOW();
		while(!(rssi0 & CC1120_CARRIER_SENSE_VALID)) {
			cur_state = cc1120_get_state();
			if((radio_pending & CC1120_TRANSMITTING) || (cur_state == CC1120_STATUS_TX)) {
				/* We have started a TX. cannot be clear in TX. */
				CC1120_RELEASE_SPI();
				return 0;
			}	
			if((cur_state == CC1200_STATUS_CALIBRATE) || (cur_state == CC1200_STATUS_SETTLING)) {
				/* We are calibrating for some reason. Not clear. */
				CC1120_RELEASE_SPI();
				return 0;
			}
			
			if(RTIMER_CLOCK_LT((t0 + RTIMER_SECOND/10), RTIMER_NOW())) {
				printf("\t RSSI Timeout, RSSI0 = 0x%02x, state = 0x%02x.\n", rssi0, cc1120_get_state());		
			
				CC1120_RELEASE_SPI();
				return 0;
			}
			rssi0 = cc1120_spi_single_read(CC1120_ADDR_RSSI0);
			watchdog_periodic();
		}
	
		if(rssi0 & CC1120_RSSI0_CARRIER_SENSE) {
			cca = 0;
			PRINTF("\t Channel NOT clear.\n");
		} else {
			cca = 1;
			PRINTF("\t Channel clear.\n");
		}
	}
	
	CC1120_RELEASE_SPI();
	
	if(was_off){
		/* If we were off, turn radio back off.*/
		off();
	}

	return cca;
}

int
cc1120_driver_receiving_packet(void)
{
	PRINTF("**** Radio Driver: Receiving Packet? ");
	uint8_t pqt;
	if(locked) {
		PRINTF("SPI Locked\n");
		return 0;
	} else {
		if((radio_pending & CC1120_TRANSMITTING) || (cc1120_get_state() != CC1120_STATUS_RX)) {
			/* Can't be receiving in TX or with the radio Off. */
			PRINTF(" - NO, in TX or Radio OFF. ****\n");
			return 0;
		} else {
			pqt = cc1120_spi_single_read(CC1120_ADDR_MODEM_STATUS1);			/* Check PQT. */
			if((pqt & CC1120_MODEM_STATUS1_SYNC_FOUND)) { 
				PRINTF(" Yes. ****\n");
				return 1;
			} else {
				/* Not receiving */
				PRINTF(" - NO. ****\n");
				return 0;
			}
		}
		return 0;
	}
}

int
cc1120_driver_pending_packet(void)
{
	PRINTFRX("**** Radio Driver: Pending Packet? ");
	if((packet_pending > 0)) {
		PRINTFRX(" yes ****\n");
		return 1;		
	} else {
		PRINTFRX(" no ****\n");
		return 0;
	}
}

int
cc1120_driver_on(void)
{
	PRINTF("**** Radio Driver: On ****\n");
	/* Set CC1120 into RX. */
	// TODO: If we are in SLEEP before this, do we need to do a cal and reg restore?
	
	if(locked) {
		lock_on = 1;
		lock_off = 0;
		return 1;
	}
	
	on();
	return 1;
}

int
cc1120_driver_off(void)
{
	PRINTF("**** Radio Driver: Off ****\n");

	if(locked) {
		/* Radio is locked, indicate that we want to turn off. */
		lock_off = 1;
		lock_on = 0;
		return 1;
	}
	
	off();
	return 1;
}


/* --------------------------- CC1120 Support Functions --------------------------- */
void
cc1120_gpio_config(void)
{
	cc1120_spi_single_write(CC1120_ADDR_IOCFG0, CC1120_GPIO0_FUNC);
	
#ifdef CC1120_GPIO2_FUNC
	cc1120_spi_single_write(CC1120_ADDR_IOCFG2, CC1120_GPIO2_FUNC);
#endif

#ifdef CC1120_GPIO3_FUNC
	cc1120_spi_single_write(CC1120_ADDR_IOCFG3, CC1120_GPIO3_FUNC);
#endif

}

void
cc1120_misc_config(void)
{
	cc1120_spi_single_write(CC1120_ADDR_PKT_CFG0, 0x20);		/* Set PKT_CFG1 for variable length packet. */
	cc1120_spi_single_write(CC1120_ADDR_RFEND_CFG1, 0x0F);		/* Set RXEND to go into IDLE after good packet and to never timeout. */
	cc1120_spi_single_write(CC1120_ADDR_RFEND_CFG0, 0x30);		/* Set TXOFF to go to RX for ACK and to stay in RX on bad packet. */
	
#if RDC_CONF_HARDWARE_CSMA	
	cc1120_spi_single_write(CC1120_ADDR_PKT_CFG2, 0x10);		/* Configure Listen Before Talk (LBT), see Section 6.12 on Page 42 of the CC1120 userguide (swru295) for details. */
#else
	cc1120_spi_single_write(CC1120_ADDR_PKT_CFG2, 0x0C);		/* Let the MAC handle Channel Clear. CCA indication is given if below RSSI threshold and NOT receiving packet. */
#endif	
}

uint8_t
cc1120_set_channel(uint8_t channel)
{
	uint32_t freq_registers;
	
  
  
  switch(radio_part) {
	case CC1120_PART_NUM_CC1120:
	case CC1120_PART_NUM_CC1121:
	case CC1120_PART_NUM_CC1125:
		freq_registers = CC1120_CHANNEL_MULTIPLIER;
		freq_registers *= channel;
		freq_registers += CC1120_BASE_FREQ;
		break;
		
	case CC1120_PART_NUM_CC1200:
    	case CC1120_PART_NUM_CC1201:
      		if(channel == 0) {
        		freq_registers = CC1200_BASE_FREQ;
     		} else {
       			freq_registers = CC1200_CHANNEL_MULTIPLIER;
        		freq_registers *= channel;
        
#if CC1120_FHSS_ETSI_50        
        		freq_registers += (((channel - 1)/5) + 1);
#elif CC1120_FHSS_FCC_50
        		freq_registers += (((channel + 1)/5) + 1);
#endif         
        		freq_registers += CC1200_BASE_FREQ;
      		}   
			break;
		
	default:	/* Not a supported chip or no chip present... */
		printf("*** ERROR: No Radio ***\n");
		while(1)	/* Spin ad infinitum as we cannot continue. */
		{
			watchdog_periodic();	/* Feed the dog to stop reboots. */
		}
		break;
  }
  
	cc1120_spi_single_write(CC1120_ADDR_FREQ0, ((unsigned char*)&freq_registers)[0]);
	cc1120_spi_single_write(CC1120_ADDR_FREQ1, ((unsigned char*)&freq_registers)[1]);
	cc1120_spi_single_write(CC1120_ADDR_FREQ2, ((unsigned char*)&freq_registers)[2]);
	
	printf("Frequency set to %02x %02x %02x (Requested %02x %02x %02x)\n", cc1120_spi_single_read(CC1120_ADDR_FREQ2), 
		cc1120_spi_single_read(CC1120_ADDR_FREQ1), cc1120_spi_single_read(CC1120_ADDR_FREQ0), ((unsigned char*)&freq_registers)[2], ((unsigned char*)&freq_registers)[1], ((unsigned char*)&freq_registers)[0]);
	
	/* If we are an affected part version, carry out calibration as per CC112x/CC1175 errata. 
	 * See http://www.ti.com/lit/er/swrz039b/swrz039b.pdf, page 3 for details.*/
	if(cc1120_spi_single_read(CC1120_ADDR_PARTVERSION) == 0x21) {
		uint8_t original_fs_cal2, calResults_for_vcdac_start_high[3], calResults_for_vcdac_start_mid[3];
		
		/* Set VCO cap Array to 0. */
		cc1120_spi_single_write(CC1120_ADDR_FS_VCO2, 0x00);				
		/* Read FS_CAL2 */
		original_fs_cal2 = cc1120_spi_single_read(CC1120_ADDR_FS_CAL2);
		/* Write FS_CAL2 as original_fs_cal2 +2 */
		cc1120_spi_single_write(CC1120_ADDR_FS_CAL2, (original_fs_cal2 + 2));
		/* Strobe CAL and wait for completion. */
		cc1120_set_state(CC1120_STATE_CAL);
		while(cc1120_get_state() == CC1120_STATUS_CALIBRATE);
		/* Read FS_VCO2, FS_VCO4, FS_CHP. */
		calResults_for_vcdac_start_high[0] = cc1120_spi_single_read(CC1120_ADDR_FS_VCO2);
		calResults_for_vcdac_start_high[1] = cc1120_spi_single_read(CC1120_ADDR_FS_VCO4);
		calResults_for_vcdac_start_high[2] = cc1120_spi_single_read(CC1120_ADDR_FS_CHP);
		/* Set VCO cap Array to 0. */
		cc1120_spi_single_write(CC1120_ADDR_FS_VCO2, 0x00);
		/* Write FS_CAL2 as original_fs_cal2 */
		cc1120_spi_single_write(CC1120_ADDR_FS_CAL2, original_fs_cal2);
		/* Strobe CAL and wait for completion. */
		cc1120_set_state(CC1120_STATE_CAL);
		while(cc1120_get_state() == CC1120_STATUS_CALIBRATE);
		/* Read FS_VCO2, FS_VCO4, FS_CHP. */
		calResults_for_vcdac_start_mid[0] = cc1120_spi_single_read(CC1120_ADDR_FS_VCO2);
		calResults_for_vcdac_start_mid[1] = cc1120_spi_single_read(CC1120_ADDR_FS_VCO4);
		calResults_for_vcdac_start_mid[2] = cc1120_spi_single_read(CC1120_ADDR_FS_CHP);
		
		if(calResults_for_vcdac_start_high[0] > calResults_for_vcdac_start_mid[0]) {
			cc1120_spi_single_write(CC1120_ADDR_FS_VCO2, calResults_for_vcdac_start_high[0]);
			cc1120_spi_single_write(CC1120_ADDR_FS_VCO4, calResults_for_vcdac_start_high[1]);
			cc1120_spi_single_write(CC1120_ADDR_FS_VCO4, calResults_for_vcdac_start_high[2]);
		} else {
			cc1120_spi_single_write(CC1120_ADDR_FS_VCO2, calResults_for_vcdac_start_mid[0]);
			cc1120_spi_single_write(CC1120_ADDR_FS_VCO4, calResults_for_vcdac_start_mid[1]);
			cc1120_spi_single_write(CC1120_ADDR_FS_VCO4, calResults_for_vcdac_start_mid[2]);
		}
	} else {
		/* Strobe CAL and wait for completion. */
		cc1120_set_state(CC1120_STATE_CAL);
		while(cc1120_get_state() == CC1120_STATUS_CALIBRATE);
	}
	
	current_channel = channel;
	next_channel = channel;
	return current_channel;
}

uint8_t
cc1120_get_channel(void)
{
	return current_channel;
}

uint8_t
cc1120_read_txbytes(void)
{
	return cc1120_spi_single_read(CC1120_ADDR_NUM_TXBYTES);
}

uint8_t
cc1120_read_rxbytes(void)
{
	return cc1120_spi_single_read(CC1120_ADDR_NUM_RXBYTES);
}


/* -------------------------- CC1120 Internal Functions --------------------------- */
static void
on(void)
{
	uint8_t state = cc1120_get_state();
	
	if((radio_pending & CC1120_RX_FIFO_UNDER) || (radio_pending & CC1120_RX_FIFO_OVER) 
	   		| (state == CC1200_STATUS_RX_FIFO_ERROR)) {
		/* RX FIFO has previously overflowed or underflowed, flush. */
		cc1120_flush_rx();
		state = cc1120_get_state();
	}
	
	radio_pending |= CC1120_RX_ON;
	if(state != CC1120_STATUS_RX) {
		cc1120_set_state(CC1120_STATE_RX);		/* Put radio into RX. */
	}
	
	ENERGEST_ON(ENERGEST_TYPE_LISTEN);
}

static void
off(void)
{
	/* Wait for any current TX to end */
	uint8_t cur_state = cc1120_get_state();                                                               \
    rtimer_clock_t t0 = RTIMER_NOW();    
	
    while((cur_state == CC1120_STATUS_TX) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (RTIMER_SECOND/10))) {
		cur_state = cc1120_get_state(); 	
	}
  
	cur_state = cc1120_set_state(CC1120_STATE_IDLE);		/* Set state to IDLE.  This will flush the RX FIFO if there is an error. */
	
#if CC1120_DEBUG_PINS
	cc1120_debug_pin_rx(0);
#endif
	
	radio_pending &= ~(CC1120_RX_ON);
	ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
	
	if(cur_state != CC1120_OFF_STATE) {
		cc1120_set_state(CC1120_OFF_STATE);			/* Put radio into the off state defined in platform-conf.h. */
	}
}

static void
enter_low_power(void)
{
#if CC1120_OFF_STATE != CC1120_STATE_IDLE
    // If we're not in RX, transimitting, or have any acks pending, enter that state
    if (!(radio_pending & CC1120_RX_ON || radio_pending & CC1120_TRANSMITTING || radio_pending & CC1120_ACK_PENDING || locked > 0)) {
        cc1120_set_state(CC1120_OFF_STATE);
    }
#endif
}

void
CC1120_LOCK_SPI(void)
{
	locked++;
}

void 
CC1120_RELEASE_SPI(void)
{
	locked--;
	
	if(locked < 0) {
		printf("\n\r\tSTRAY CC1120_RELEASE_SPI\n\r");
		locked = 0;
	}
	
	if(locked == 0) {		
		if(next_channel != current_channel)
		{
			cc1120_set_channel(next_channel);	
		}	
		
		if(lock_off) {
			off();
			lock_off = 0;
		} else if(lock_on) {
			on();
			lock_on = 0;
		}
	}
}


/* ---------------------------- CC1120 State Functions ---------------------------- */
uint8_t
cc1120_set_state(uint8_t state)
{
	uint8_t cur_state;
	
	/* Check for FIFO errors. */
	cur_state = cc1120_get_state();			/* Get the current state. */
	
	if(cur_state == CC1120_STATUS_RX_FIFO_ERROR) {
		/* If there is a RX FIFO Error, clear it. */
		cc1120_flush_rx();
		cur_state = cc1120_get_state();
	}
	
	if(cur_state == CC1120_STATUS_TX_FIFO_ERROR) {
		/* If there is a TX FIFO Error, clear it. */
		cc1120_flush_tx();
		cur_state = cc1120_get_state();
	}
	
	/* Change state. */
	switch(state) {
		case CC1120_STATE_FSTXON:	/* Can only enter from IDLE, TX or RX. */
								PRINTFSTATE("\t\tEntering FSTXON (%02x) from %02x\n", state, cur_state);
															
								if(!((cur_state == CC1120_STATUS_IDLE) 
									|| (cur_state == CC1120_STATUS_TX)
									|| (cur_state == CC1120_STATUS_FSTXON))) {
									/* If we are not in IDLE or TX or FSTXON, get us to IDLE.
									 * While we can enter FSTXON from RX, it may leave stuff stuck in the FIFO. */
									cc1120_set_idle();
								}
								if(!(cur_state == CC1120_STATUS_FSTXON)) {
									cc1120_spi_cmd_strobe(CC1120_STROBE_SFSTXON);	/* Intentional Error to catch warnings. */
									while(cc1120_get_state() != CC1120_STATUS_FSTXON);
								}
								return CC1120_STATUS_FSTXON;
								
		case CC1120_STATE_XOFF:		/* Can only enter from IDLE. */
								PRINTFSTATE("\t\tEntering XOFF (%02x) from %02x\n", state, cur_state);
						
								/* If we are not in IDLE, get us there. */
								if(cur_state != CC1120_STATUS_IDLE) {
									cc1120_set_idle();
								}
								cc1120_spi_cmd_strobe(CC1120_STROBE_SXOFF);
								return CC1120_STATUS_IDLE;
								
		case CC1120_STATE_CAL:		/* Can only enter from IDLE. */
								PRINTFSTATE("\t\tEntering CAL (%02x) from %02x \n", state, cur_state);

								/* If we are not in IDLE, get us there. */
								if(cur_state != CC1120_STATUS_IDLE) {
									cc1120_set_idle();
								}
								cc1120_spi_cmd_strobe(CC1120_STROBE_SCAL);
								while(cc1120_get_state() != CC1120_STATUS_CALIBRATE);
								return CC1120_STATUS_CALIBRATE;
								
		case CC1120_STATE_RX:		/* Can only enter from IDLE, FSTXON or TX. */
								PRINTFSTATE("\t\tEntering RX (%02x) from %02x\n", state, cur_state);

								if (cur_state == CC1120_STATUS_RX) {
									cc1120_set_idle();
									cc1120_flush_rx();
									return cc1120_set_rx();
								}								
								else if((cur_state == CC1120_STATUS_IDLE) 
									|| (cur_state == CC1120_STATUS_FSTXON)
									|| (cur_state == CC1120_STATUS_TX)) {
									/* Return RX state. */
#if CC1120_DEBUG_PINS
									cc1120_debug_pin_rx(1);
#endif
									return cc1120_set_rx();
								} else {
									/* We are in a state that will end up in IDLE, FSTXON or TX. Wait till we are there. */
									while(!((cur_state == CC1120_STATUS_IDLE)
										|| (cur_state == CC1120_STATUS_FSTXON)
										|| (cur_state == CC1120_STATUS_TX)) ) {
											cur_state = cc1120_get_state();
										}
									
									/* Return RX state. */
									return cc1120_set_rx();
								}
								break;
								
		case CC1120_STATE_TX:		/* Can only enter from IDLE, FSTXON or RX. */
								PRINTFSTATE("\t\tEntering TX (%02x) from %02x\n", state, cur_state);

								if((cur_state == CC1120_STATUS_RX))
								{
									/* Get us out of RX. */
									cur_state = cc1120_set_idle();
								}

								if((cur_state == CC1120_STATUS_IDLE) 
								|| (cur_state == CC1120_STATUS_FSTXON))
								{
									/* Return TX state. */
									return cc1120_set_tx();
								}
								else
								{
									/* We are in a state that will end up in IDLE, FSTXON or RX. Wait till we are there. */
									while(!((cur_state == CC1120_STATUS_IDLE)
										|| (cur_state == CC1120_STATUS_FSTXON)
										|| (cur_state == CC1120_STATUS_RX)) )
										{
											cur_state = cc1120_get_state();
										}
										
									/* Return TX state. */
									return cc1120_set_tx();
								}
								break;
								
		case CC1120_STATE_IDLE:		/* Can enter from any state. */
								PRINTFSTATE("\t\tEntering IDLE (%02x) from %02x\n", state, cur_state);
								/* If we are already in IDLE, do nothing and return the current state. */
								if(cur_state == CC1120_STATUS_RX) { 
									cur_state = cc1120_get_state();
								}
			
								if(cur_state != CC1120_STATUS_IDLE)
								{
									/* Set Idle. */
									cc1120_set_idle();
								}

								/* Return IDLE state. */
								return CC1120_STATUS_IDLE;
								
		case CC1120_STATE_SLEEP:	/* Can only enter from IDLE. */
								PRINTFSTATE("\t\tEntering SLEEP (%02x) from %02x\n", state, cur_state);

								/* If we are not in IDLE, get us there. */
								if(cur_state != CC1120_STATUS_IDLE)
								{
									cc1120_set_idle();
								}
								cc1120_spi_cmd_strobe(CC1120_STROBE_SPWD);

								return CC1120_STATUS_IDLE;
								break;
								
		default:				printf("!!! INVALID STATE REQUESTED !!!\n"); 
								return CC1120_STATUS_STATE_MASK;
								break;	
	}
}

uint8_t
cc1120_get_state(void)
{
	uint8_t state;
	state = cc1120_spi_cmd_strobe(CC1120_STROBE_SNOP);
	state &= CC1120_STATUS_STATE_MASK;
	
#if CC1120_DEBUG_PINS
	if(state == CC1120_STATUS_RX) {
		cc1120_debug_pin_rx(1);
	} else {
		cc1120_debug_pin_rx(0);
	}
#endif
	
	return state;
	//return (cc1120_spi_cmd_strobe(CC1120_STROBE_SNOP) & CC1120_STATUS_STATE_MASK);
}


/* -------------------------- CC1120 State Set Functions -------------------------- */
uint8_t
cc1120_set_idle(void)
{
	PRINTFSTATE("Entering IDLE ");
	
	uint8_t cur_state;
	
	/* Send IDLE strobe. */
	cur_state = cc1120_spi_cmd_strobe(CC1120_STROBE_SIDLE);

	/* Spin until we are in IDLE. */
	while(cur_state != CC1120_STATUS_IDLE) {
		PRINTFSTATE(".");
		clock_delay(10);
		if(cur_state == CC1120_STATUS_TX_FIFO_ERROR) {
			cc1120_spi_cmd_strobe(CC1120_STROBE_SFTX);
		} else if(cur_state == CC1120_STATUS_RX_FIFO_ERROR) {		
			cc1120_spi_cmd_strobe(CC1120_STROBE_SFRX);
		}
		cur_state = cc1120_get_state();
	}
	PRINTFSTATE("OK\n");

	/* Return IDLE state. */
	return CC1120_STATUS_IDLE;
}

uint8_t
cc1120_set_rx(void)
{
	/* Enter RX. */
	cc1120_spi_cmd_strobe(CC1120_STROBE_SRX);

	/* Spin until we are in RX. */
	BUSYWAIT_UNTIL((cc1120_get_state() == CC1120_STATUS_RX), RTIMER_SECOND/10);
	
	return cc1120_get_state();
}

uint8_t
cc1120_set_tx(void)
{
	uint8_t cur_state;

	/* Enter TX. */
	cc1120_spi_cmd_strobe(CC1120_STROBE_STX);
	
	/* If we are NOT in TX, Spin until we are in TX. */
	cur_state = cc1120_get_state();
	while(cur_state != CC1120_STATUS_TX) {	
		cur_state = cc1120_get_state();
		if(cur_state == CC1120_STATUS_TX_FIFO_ERROR) {	
			/* TX FIFO Error - flush TX. */	
			return cc1120_flush_tx();
		}
		clock_delay(1);
	}		

	// TODO: give this a timeout?

	/* Return TX state. */
	return CC1120_STATUS_TX;
}

uint8_t
cc1120_flush_rx(void)
{
	uint8_t cur_state = cc1120_get_state();
	rtimer_clock_t t0 = RTIMER_NOW();
	
	if((cur_state != CC1120_STATUS_IDLE) && (cur_state != CC1120_STATUS_RX_FIFO_ERROR)) {
		/* If not in IDLE or TXERROR, get to IDLE. */
		if(cur_state == CC1120_STATUS_TX_FIFO_ERROR) {
			/* TX FIFO Error.  Flush TX FIFO. */
			cc1120_spi_cmd_strobe(CC1120_STROBE_SFTX);
		}
		while((cur_state != CC1120_STATUS_IDLE) 
				&& RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (RTIMER_SECOND / 10))) {
			if(cur_state == CC1120_STATUS_TX_FIFO_ERROR) {
				/* TX FIFO Error.  Flush TX FIFO. */
				cc1120_spi_cmd_strobe(CC1120_STROBE_SFTX);
			}
			/* Get the current state and strobe IDLE. */
			cur_state = cc1120_get_state();
			cc1120_spi_cmd_strobe(CC1120_STROBE_SIDLE);
			watchdog_periodic();
		}
	}
	
	/* FLush RX FIFO. */
	cc1120_spi_cmd_strobe(CC1120_STROBE_SFRX);

	/* Spin until we are in IDLE. */
	BUSYWAIT_UNTIL((cc1120_get_state() == CC1120_STATUS_IDLE), RTIMER_SECOND/10);

	radio_pending &= ~(CC1120_RX_FIFO_OVER | CC1120_RX_FIFO_UNDER);
	packet_pending = 0;
	LEDS_OFF(LEDS_RED);

	/* Return IDLE state. */
	return CC1120_STATUS_IDLE;
}

uint8_t
cc1120_flush_tx(void)
{
	uint8_t cur_state = cc1120_get_state();
	rtimer_clock_t t0 = RTIMER_NOW();
	
	if((cur_state != CC1120_STATUS_IDLE) && (cur_state != CC1120_STATUS_TX_FIFO_ERROR)) {
		/* If not in IDLE or TXERROR, get to IDLE. */
		if(cur_state == CC1120_STATUS_RX_FIFO_ERROR) {
			/* RX FIFO Error.  Flush RX FIFO. */
			cc1120_spi_cmd_strobe(CC1120_STROBE_SFRX);
		}
		while((cur_state != CC1120_STATUS_IDLE) 
				&& RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (RTIMER_SECOND / 10))) {
			if(cur_state == CC1120_STATUS_RX_FIFO_ERROR) {
				/* RX FIFO Error.  Flush RX FIFO. */
				cc1120_spi_cmd_strobe(CC1120_STROBE_SFRX);
			}
			/* Get the current state and strobe IDLE. */
			cur_state = cc1120_get_state();
			cc1120_spi_cmd_strobe(CC1120_STROBE_SIDLE);
			watchdog_periodic();
		}
	}
	
	/* FLush TX FIFO. */
	cc1120_spi_cmd_strobe(CC1120_STROBE_SFTX);
	watchdog_periodic();
	
	/* Spin until we have flushed TX and are in IDLE. */
	while((cur_state != CC1120_STATUS_IDLE) 
			&& RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (RTIMER_SECOND / 5))) {
		if(cur_state == CC1120_STATUS_TX_FIFO_ERROR) {
			/* (Another) TX FIFO error, flush. */
			cc1120_spi_cmd_strobe(CC1120_STROBE_SFTX);
		} else if(cur_state == CC1120_STATUS_RX_FIFO_ERROR) {
			/* RX FIFO Error. Flush it. */
			cc1120_spi_cmd_strobe(CC1120_STROBE_SFRX);
		} else if (cur_state != CC1120_STATUS_IDLE) {
			/* Not in IDLE - re-strobe. */
			cc1120_spi_cmd_strobe(CC1120_STROBE_SIDLE);
		}
		/* Get the current state. */
		cur_state = cc1120_get_state();
		watchdog_periodic();
	}

	/* Clear CC1120_TX_FIFO_ERROR Flag. */
	radio_pending &= ~(CC1120_TX_FIFO_ERROR);
	
	/* Return last state. */
	return cur_state;
}

/* --------------------------- Radio Parameter Functions --------------------------- */
/* Set a radio parameter object. */
radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{

  return RADIO_RESULT_NOT_SUPPORTED;

}

/* Get a radio parameter object. */
radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{

  return RADIO_RESULT_NOT_SUPPORTED;

}

/* Set a radio parameter value. */
radio_result_t
set_value(radio_param_t param, radio_value_t value)
{

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
			if(value == RADIO_POWER_MODE_ON) {
			  on();
			  return RADIO_RESULT_OK;
			}

			if(value == RADIO_POWER_MODE_OFF) {
			  off();
			  return RADIO_RESULT_OK;
			}

			return RADIO_RESULT_INVALID_VALUE;

		  break;

  case RADIO_PARAM_CHANNEL:
			if((value > 49) | (value < 0)) {
			  return RADIO_RESULT_INVALID_VALUE;
			}

			/*
			 * We always return OK here even if the channel update was
			 * postponed. rf_channel is NOT updated in this case until
			 * the channel update was performed. So reading back
			 * the channel using get_value() might return the "old" channel
			 * until the channel was actually changed
			 */
		  	if(locked) {
				/* Radio is busy, defer channel change till unlock. */	
		  		next_channel = value;
			} else {
				/* Set the new channel. */ 
		    	cc1120_set_channel(value);
			}

			return RADIO_RESULT_OK;
		 
		  break;

  case RADIO_PARAM_CCA_THRESHOLD:
  case RADIO_PARAM_PAN_ID:
  case RADIO_PARAM_16BIT_ADDR:
  case RADIO_PARAM_RX_MODE:
  case RADIO_PARAM_TX_MODE:
  case RADIO_PARAM_TXPOWER:
  case RADIO_PARAM_RSSI:
  case RADIO_PARAM_64BIT_ADDR:
  default:
    		return RADIO_RESULT_NOT_SUPPORTED;
  }
}

/* Get a radio parameter value. */
radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
			if(radio_pending & CC1120_RX_ON) {
			  *value = (radio_value_t)RADIO_POWER_MODE_ON;
			} else {
			  *value = (radio_value_t)RADIO_POWER_MODE_OFF;
			}
			return RADIO_RESULT_OK;

  case RADIO_PARAM_CHANNEL:

			*value = (radio_value_t)current_channel;
			return RADIO_RESULT_OK;


  case RADIO_CONST_CHANNEL_MIN:
			*value = (radio_value_t)0;
			return RADIO_RESULT_OK;

  case RADIO_CONST_CHANNEL_MAX:
			*value = (radio_value_t)49;
			return RADIO_RESULT_OK;

  case RADIO_PARAM_PAN_ID:
  case RADIO_PARAM_16BIT_ADDR:
  case RADIO_PARAM_RX_MODE:
  case RADIO_PARAM_TX_MODE:
  case RADIO_PARAM_TXPOWER:
  case RADIO_PARAM_CCA_THRESHOLD:
  case RADIO_PARAM_RSSI:
  case RADIO_PARAM_64BIT_ADDR:  
  case RADIO_CONST_TXPOWER_MIN:
  case RADIO_CONST_TXPOWER_MAX:
  default:
    		return RADIO_RESULT_NOT_SUPPORTED;

  }

}

/* ----------------------------- CC1120 SPI Functions ----------------------------- */
void
cc1120_spi_disable(void) 
{
	cc1120_arch_spi_disable();	
	if(radio_pending & CC1120_INTERRUPT_PENDING){
		//radio_pending &= ~(CC1120_INTERRUPT_PENDING);
		radio_pending &= ~(CC1120_INTERRUPT_PENDING);
		cc1120_interrupt_handler();
	}
}
		
uint8_t
cc1120_spi_cmd_strobe(uint8_t strobe)
{
	cc1120_arch_spi_enable();
	strobe = cc1120_arch_spi_rw_byte(strobe);
	cc1120_spi_disable();
	
	return strobe;
}

uint8_t
cc1120_spi_single_read(uint16_t addr)
{
	cc1120_arch_spi_enable();
	cc1120_spi_write_addr(addr, CC1120_STANDARD_BIT, CC1120_READ_BIT);
	addr = cc1120_arch_spi_rw_byte(0);		/* Get the value.  Re-use addr to save a byte. */ 
	cc1120_spi_disable();
	
	return addr;
}

uint8_t
cc1120_spi_single_write(uint16_t addr, uint8_t val)
{
	cc1120_arch_spi_enable();
	addr = cc1120_spi_write_addr(addr, CC1120_STANDARD_BIT, CC1120_WRITE_BIT);	/* Read the status byte. */
	cc1120_arch_spi_rw_byte(val);		
	cc1120_spi_disable();
	
	return addr;
}

static uint8_t
cc1120_spi_write_addr(uint16_t addr, uint8_t burst, uint8_t rw)
{
	uint8_t status;
	if(addr & CC1120_EXTENDED_MEMORY_ACCESS_MASK) {
		status = cc1120_arch_spi_rw_byte(CC1120_ADDR_EXTENDED_MEMORY_ACCESS | rw | burst); 
		(void) cc1120_arch_spi_rw_byte(addr & CC1120_ADDRESS_MASK);
	} else {
		status = cc1120_arch_spi_rw_byte(addr | rw | burst);
	}
	
	return status;
}

static void
cc1120_write_txfifo(uint8_t *payload, uint8_t payload_len)
{
	cc1120_arch_spi_enable();
	cc1120_arch_txfifo_load(payload, payload_len);
	cc1120_spi_disable();
	
	PRINTFTX("\t%d bytes in fifo (%d + length byte requested)\n", cc1120_read_txbytes(), payload_len);
}



/* -------------------------- CC1120 Interrupt Handler --------------------------- */
int
cc1120_interrupt_handler(void)
{
	uint8_t marc_status;
/* turn on flickering LED if defined */
/* #ifndef CC1120LEDSNB */
	LEDS_ON(LEDS_BLUE);
/*#endif */
	cc1120_arch_interrupt_acknowledge();
	
	/* Check if we have interrupted an SPI function, if so flag that interrupt is pending. */
	if(cc1120_arch_spi_enabled()) {
		if(locked) {
			radio_pending |= CC1120_INTERRUPT_PENDING;
			return 0;
		} else {
			cc1120_spi_disable();	
		}
	}
	
	marc_status = cc1120_spi_single_read(CC1120_ADDR_MARC_STATUS1);
		
	if(marc_status == CC1120_MARC_STATUS_OUT_NO_FAILURE) {
		LEDS_OFF(LEDS_BLUE);
		if((cc1120_get_state() == CC1120_STATUS_IDLE) && (cc1120_read_rxbytes() > 0)) {
			marc_status = CC1120_MARC_STATUS_OUT_RX_FINISHED;
		} else {
            enter_low_power();
			return 0;
		}
	}
	
	PRINTFINT("\t CC1120 Int. %d\n", marc_status);
	
	if(marc_status == CC1120_MARC_STATUS_OUT_RX_FINISHED) {
		/* Ignore as it should be an ACK. */
		if(radio_pending & CC1120_ACK_PENDING) {
			LEDS_OFF(LEDS_BLUE);
            enter_low_power();
			return 0;
		}	
		
		/* We have received a packet.  This is done first to make RX faster. */
		packet_pending++;
		
		process_poll(&cc1120_process);
		LEDS_OFF(LEDS_BLUE);
		return 1;
	}	
	
	switch (marc_status){
		case CC1120_MARC_STATUS_OUT_TX_FINISHED:	
			/* TX Finished. */
			radio_pending |= CC1120_TX_COMPLETE;													
			break;
			
		case CC1120_MARC_STATUS_OUT_RX_OVERFLOW:	
			/* RX FIFO has overflowed. */
			PRINTFRXERR("\t!!! RX FIFO Error: Overflow. !!!\n");
			cc1120_flush_rx();	
			if(radio_pending & CC1120_RX_ON)
			{	
				on();
			}						
			break;
			
		case CC1120_MARC_STATUS_OUT_RX_UNDERFLOW:	
			/* RX FIFO has underflowed. */
			PRINTFRXERR("\t!!! RX FIFO Error: Underflow. !!!\n");
			radio_pending |= CC1120_RX_FIFO_UNDER;			
			printf("RUF\n\r");						
			break;	
		
		case CC1120_MARC_STATUS_OUT_RX_TIMEOUT:	
			/* RX terminated due to timeout.  Should not get here as there is  */	
			printf("RX Timeout.\n\r");						
			break;
			
		case CC1120_MARC_STATUS_OUT_TX_OVERFLOW:	
			/* TX FIFO has overflowed. */
			radio_pending |= CC1120_TX_FIFO_ERROR;
			printf("TOF\n\r");											
			break;
			
		case CC1120_MARC_STATUS_OUT_TX_UNDERFLOW:	
			/* TX FIFO has underflowed. */
			PRINTFTXERR("\t!!! TX FIFO Error: Underflow. !!!\n");
			radio_pending |= CC1120_TX_FIFO_ERROR;	
			printf("TUF\n\r");				
			break;
				
		case CC1120_MARC_STATUS_OUT_RX_TERMINATION:	
			/* RX Terminated on CS or PQT. */
			printf("RXT\n\r");
			break;
			
		case CC1120_MARC_STATUS_OUT_EWOR_SYNC_LOST:	
			/* EWOR Sync lost. */	
			printf("EWOR\n\r");							
			break;
			
		case CC1120_MARC_STATUS_OUT_PKT_DISCARD_LEN:	
			/* Packet discarded due to being too long. Flush RX FIFO? */	
			printf("LEN\n\r");
			break;
			
		case CC1120_MARC_STATUS_OUT_PKT_DISCARD_ADR:	
			/* Packet discarded due to bad address - should not get here 
			 * as address matching is not being used. Flush RX FIFO? */		
			printf("DISC\n\r");				
			break;
			
		case CC1120_MARC_STATUS_OUT_PKT_DISCARD_CRC:	
			/* Packet discarded due to bad CRC. Should not need to flush 
			 * RX FIFO as CRC_AUTOFLUSH is set in FIFO_CFG*/	
			printf("CRC\n\r");		
			break;	
			
		case CC1120_MARC_STATUS_OUT_TX_ON_CCA_FAIL:	
			/* TX on CCA Failed due to busy channel. */									
			break;
			
		default:
			break;
	}	
	LEDS_OFF(LEDS_BLUE);
    enter_low_power();
	return 1;
}



/* ----------------------------------- CC1120 Process ------------------------------------ */

PROCESS_THREAD(cc1120_process, ev, data)
{	
	PROCESS_POLLHANDLER(processor());					/* Register the Pollhandler. */
	
	PROCESS_BEGIN();
	printf("CC1120 Driver Start\n");	
	
	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT);	/* Wait till the process is terminated. */
	
	printf("CC1120 Driver End\n");
	PROCESS_END();
}

		
void processor(void)
{			
	uint8_t len;	
	uint8_t buf[CC1120_MAX_PAYLOAD];
			
	PRINTFPROC("** Process Poll **\n");
	LEDS_ON(LEDS_RED);
	watchdog_periodic();	
	
	len = cc1120_driver_read_packet(buf, CC1120_MAX_PAYLOAD);
	
	if(len) {
		packetbuf_clear(); /* Clear the packetbuffer. */
	
		PRINTFPROC("\tPacket Length: %d\n", len);	
		
		/* Load the packet buffer. */
		memcpy(packetbuf_dataptr(), (void *)buf, len);
		
		/* Read RSSI & LQI. */
		packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rx_rssi);
		packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, rx_lqi);
		
		/* Set packet buffer length. */
		packetbuf_set_datalen(len);		/* Set Packetbuffer length. */
		
		NETSTACK_RDC.input();
	}
		
	LEDS_OFF(LEDS_RED);
	if(radio_pending & CC1120_RX_ON) {
		on();
	}

    enter_low_power();
}
