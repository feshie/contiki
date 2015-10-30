
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

//#define PLATFORM_HAS_LEDS   1
//#define PLATFORM_HAS_BUTTON 1

/* CPU target speed in Hz */
#define F_CPU 48000000uL /* 48MHz by default */

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 128UL

/* Types for clocks and uip_stats */
//typedef unsigned short uip_stats_t;
//typedef unsigned long clock_time_t;
//typedef unsigned long off_t;

/* the low-level radio driver */
#define NETSTACK_CONF_RADIO   cc1120_driver


/* **************************************************************************** */
/* ------------------------------- SPI Related -------------------------------- */
/* **************************************************************************** */




/* **************************************************************************** */
/* ------------------------------- MS1 Related -------------------------------- */
/* **************************************************************************** */




/* **************************************************************************** */
/* --------------------------- M25P80 Flash Related --------------------------- */
/* **************************************************************************** */



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

//#define CC1120LEDS				1

#define CC1120_LBT_TIMEOUT 		RTIMER_ARCH_SECOND			//80
#define CC1120_ACK_WAIT			RTIMER_ARCH_SECOND/667	/* ~1.5ms. */

#define CC1120_INTER_PACKET_INTERVAL	RTIMER_ARCH_SECOND/300 //275 //222

#define CC1120_EN_TIMEOUT		RTIMER_ARCH_SECOND/500

#define CC1120_FHSS_ETSI_50		1
#define CC1120_FHSS_FCC_50		0

#define CC1120_OFF_STATE CC1120_STATE_IDLE

#define CC1120_SINGLE_INTERRUPT
#define CC1120_DUAL_IO

#define CC1120_GPIO0_FUNC	CC1120_GPIO_MCU_WAKEUP
//#define CC1120_GPIO0_FUNC	(CC1120_GPIO_PKT_SYNC_RXTX| CC1120_GPIO_INV_MASK)	
//#define CC1120_GPIO2_FUNC
//#define CC1120_GPIO3_FUNC	CC1120_GPIO_RXFIFO_THR_PKT
#define CC1120_GPIO3_FUNC	CC1120_GPIO_RX0TX1_CFG //CC1120_GPIO_MARC_2PIN_STATUS0	//(CC1120_GPIO_PKT_SYNC_RXTX| CC1120_GPIO_INV_MASK)	




/* --------------------------- CC1120 Pin Mappings. --------------------------- */




/* **************************************************************************** */
/* ------------------------------ CC2420 Related ------------------------------ */
/* **************************************************************************** */

#endif /* __PLATFORM_CONF_H__ */

