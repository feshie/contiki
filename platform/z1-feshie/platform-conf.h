
/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *         Platform configuration for the Z1-feshie platform
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#define SPI_LOCKING
//#define LOCKING_DEBUG
#ifdef LOCKING_DEBUG
 #include <stdio.h>
 #define LPRINT(...) printf(__VA_ARGS__)
#else
 #define LPRINT(...)
#endif
/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */
#define ZOLERTIA_Z1 1  /* Enric */

#define PLATFORM_HAS_LEDS   1
#define PLATFORM_HAS_BUTTON 1

/* CPU target speed in Hz */
#define F_CPU 8000000uL /* 8MHz by default */
//Enric #define F_CPU 3900000uL /*2457600uL*/

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 128UL

#define BAUD2UBR(baud) ((F_CPU/baud))

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#include "msp430def.h"

/* XXX Temporary place for defines that are lacking in mspgcc4's gpio.h */
#ifdef __IAR_SYSTEMS_ICC__
#ifndef P1SEL2_
#define P1SEL2_              (0x0041u)  /* Port 1 Selection 2*/
DEFC(   P1SEL2             , P1SEL2_)
#endif
#ifndef P5SEL2_
#define P5SEL2_              (0x0045u)  /* Port 5 Selection 2*/
DEFC(   P5SEL2             , P5SEL2_)
#endif
#else /* __IAR_SYSTEMS_ICC__ */
#ifdef __GNUC__
#ifndef P1SEL2_
  #define P1SEL2_             0x0041  /* Port 1 Selection 2*/
  sfrb(P1SEL2, P1SEL2_);
#endif
#ifndef P5SEL2_
  #define P5SEL2_             0x0045  /* Port 5 Selection 2*/
  sfrb(P5SEL2, P5SEL2_);
#endif
#endif /* __GNUC__ */
#endif /* __IAR_SYSTEMS_ICC__ */

/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long off_t;

/* the low-level radio driver */
//#define NETSTACK_CONF_RADIO   cc1120_driver

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */

/* LED ports */
#define LEDS_PxDIR P5DIR
#define LEDS_PxOUT P5OUT
#define LEDS_CONF_RED    0x10
#define LEDS_CONF_GREEN  0x40
#define LEDS_CONF_YELLOW 0x20

/* DCO speed resynchronization for more robust UART, etc. */
#define DCOSYNCH_CONF_ENABLED 0
#define DCOSYNCH_CONF_PERIOD 30

#define ROM_ERASE_UNIT_SIZE  512
#define XMEM_ERASE_UNIT_SIZE (64*1024L)


#define CFS_CONF_OFFSET_TYPE    long

/* Use the first 64k of external flash for node configuration */
#define NODE_ID_XMEM_OFFSET     (0 * XMEM_ERASE_UNIT_SIZE)
#define RESET_COUNTER_XMEM_OFFSET     (1 * XMEM_ERASE_UNIT_SIZE)

/* Use the second 64k of external flash for codeprop. */
#define EEPROMFS_ADDR_CODEPROP  (2 * XMEM_ERASE_UNIT_SIZE)

#define CFS_XMEM_CONF_OFFSET    (3 * XMEM_ERASE_UNIT_SIZE)
#define CFS_XMEM_CONF_SIZE      (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_RAM_CONF_SIZE 4096



/* **************************************************************************** */
/* ------------------------------- SPI Related -------------------------------- */
/* **************************************************************************** */
#define SPI_TXBUF UCB0TXBUF
#define SPI_RXBUF UCB0RXBUF

/* USART0 Tx ready? */
#define	SPI_WAITFOREOTx() while ((UCB0STAT & UCBUSY) != 0)

/* USART0 Rx ready? */
#define	SPI_WAITFOREORx() while ((IFG2 & UCB0RXIFG) == 0)

/* USART0 Tx buffer ready? */
#define SPI_WAITFORTxREADY() while ((IFG2 & UCB0TXIFG) == 0)

#define MOSI           1  /* P3.1 - Output: SPI Master out - slave in (MOSI) */
#define MISO           2  /* P3.2 - Input:  SPI Master in - slave out (MISO) */
#define SCK            3  /* P3.3 - Output: SPI Serial Clock (SCLK) */



/* **************************************************************************** */
/* ------------------------------- MS1 Related -------------------------------- */
/* **************************************************************************** */

#define SENSE_EN_PORT(type) P4##type		/* Sense enable Pin. */
#define SENSE_EN_PIN 0

#define RADIO_EN_PORT(type) P4##type		/* Radio power enable Pin. */
#define RADIO_EN_PIN 2

#define RS485_TXEN_PORT(type) P2##type		/* TX Enable pin for RS485 Driver. */
#define RS485_TXEN_PIN 3

#define UART1_RX_PORT(type) P3##type		/* UART1 RX Pin. */
#define UART1_RX_PIN 7

#define UART1_TX_PORT(type) P3##type		/* UART1 TX Pin. */
#define UART1_TX_PIN 6



/* **************************************************************************** */
/* --------------------------- M25P80 Flash Related --------------------------- */
/* **************************************************************************** */
#define FLASH_CS	4	/* P4.4 Output */
#define FLASH_HOLD	7	/* P5.7 Output */

#define SPI_FLASH_ENABLE()  ( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() ( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()		( P5OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()		( P5OUT |=  BV(FLASH_HOLD) )


/* **************************************************************************** */
/* ------------------------------ CC1120 Related ------------------------------ */
/* **************************************************************************** */

//#define CC1120DEBUG		1
//#define CC1120TXDEBUG		1
#define CC1120TXERDEBUG		1
//#define CC1120RXDEBUG		1
#define CC1120RXERDEBUG		1
//#define CC1120INTDEBUG		1
//#define C1120PROCESSDEBUG	1
//#define CC1120ARCHDEBUG		1
//#define CC1120STATEDEBUG	1

#define RF_CHANNEL				42

#define CC1120_CS_THRESHOLD		0x9C	/*-100dBm */

/* Other possible sensible values:
 * 0xC4	-60dBm.
 * 0xBF	-65dBm.
 * 0xBA	-70dBm.
 * 0xB5	-75dBm.
 * 0xB0 -80dBm.
 * 0xAB -85dBm.
 * 0xA6 -90dBm.
 * 0xA5 -91dBm.
 * 0xA4 -92dBm.
 * 0xA3 -93dBm.
 * 0xA2 -94dBm.
 * 0xA1 -95dBm.
 * 0xA0 -96dBm.
 * 0x9F -97dBm.
 * 0x9E -98dBm.
 * 0x9D -99dBm.
 * 0x9C -100dBm.
 * 0x9B -101dBm.
 * 0x9A -102dBm.
 * 0x99 -103dBm
 * 0x98 -104dBm.
 * 0x97 -105dBm.
 * 0x96 -106dBm.
 * 0x95 -107dBm.
 * 0x94 -108dBm.
 * 0x93 -109dBm.
 * 0x92 -110dBm.
 */
//#define CC1120_RSSI_OFFSET	0x9A

#define CC1120LEDS				1

#define CC1120_LBT_TIMEOUT 		RTIMER_ARCH_SECOND			//80
#define CC1120_ACK_WAIT			RTIMER_ARCH_SECOND/667	/* ~1.5ms. */

#define CC1120_INTER_PACKET_INTERVAL	RTIMER_ARCH_SECOND/300 //275 //222

#define CC1120_EN_TIMEOUT		RTIMER_ARCH_SECOND/500

#define CC1120_FHSS_ETSI_50		1
#define CC1120_FHSS_FCC_50		0

#define CC1120_OFF_STATE CC1120_STATE_IDLE

#define CC1120_GPIO_MODE	3

#define CC1120_GPIO0_FUNC	CC1120_GPIO_MCU_WAKEUP
//#define CC1120_GPIO0_FUNC	(CC1120_GPIO_PKT_SYNC_RXTX| CC1120_GPIO_INV_MASK)	
//#define CC1120_GPIO2_FUNC
//#define CC1120_GPIO3_FUNC	CC1120_GPIO_RXFIFO_THR_PKT
#define CC1120_GPIO3_FUNC	CC1120_GPIO_RX0TX1_CFG //CC1120_GPIO_MARC_2PIN_STATUS0	//(CC1120_GPIO_PKT_SYNC_RXTX| CC1120_GPIO_INV_MASK)	




/* --------------------------- CC1120 Pin Mappings. --------------------------- */
#define CC1120_RESET_PORT(type)	   P2##type
#define CC1120_RESET_PIN       6

#define CC1120_SPI_CSN_PORT(type)  P2##type
#define CC1120_SPI_CSN_PIN     1

#define CC1120_SPI_MOSI_PORT(type)  P3##type
#define CC1120_SPI_MOSI_PIN    1

#define CC1120_SPI_MISO_PORT(type)  P3##type
#define CC1120_SPI_MISO_PIN    2

#define CC1120_SPI_SCLK_PORT(type)  P3##type
#define CC1120_SPI_SCLK_PIN    3

#define CC1120_GDO0_PORT(type) P1##type
#define CC1120_GDO0_PIN        0

#ifdef CC1120_GPIO2_FUNC
#define CC1120_GDO1_PORT(type) P1##type
#define CC1120_GDO1_PIN        0
#endif

#ifdef CC1120_GPIO3_FUNC
#define CC1120_GDO3_PORT(type) P4##type
#define CC1120_GDO3_PIN        3

#endif



/* **************************************************************************** */
/* ------------------------------ CC2420 Related ------------------------------ */
/* **************************************************************************** */

/* Pin Mappings. */
#define CC2420_CSN_PORT(type) P3##type
#define CC2420_CSN_PIN	      0

#define CC2420_PWR_PORT(type) P4##type
#define CC2420_PWR_PIN        4

#define CC2420_RESET_PORT(type)    P4##type
#define CC2420_RESET_PIN           6

#endif /* __PLATFORM_CONF_H__ */
