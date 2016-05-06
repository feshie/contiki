
#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include <stdint.h>
#include <stdbool.h>

#include "platform-conf.h"

#include "dev/leds.h"

#define CCIF
#define CLIF

#define SYSTICK_DEBUG
#define SYSTICK_DEBUG_PORT	PORTD
#define SYSTICK_DEBUG_GPIO	GPIOD
#define SYSTICK_DEBUG_PIN	7

#define CLOCK_CONF_SECOND 128

typedef uint16_t rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((int16_t)((a)-(b)) < 0)

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

/* Platform typedefs */
typedef uint32_t clock_time_t;
typedef uint32_t uip_stats_t;

/* Enable watchdog. */
#define DISABLE_WDOG	1

/* Set allowable low-power modes. Disable VLLS. */
#define SYSTEM_SMC_PMPROT_VALUE		0x28

//#define true 1
//#define false 0

#ifndef BV
#define BV(x) (1<<(x))
#endif


/*--------------------------------------- LED Config ----------------------------------------*/

#define PLATFORM_HAS_LEDS   1

#define LED_RED_PORT		PORTB
#define LED_RED_GPIO		GPIOB
#define LED_RED_PIN			18

#define LED_GREEN_PORT		PORTB
#define LED_GREEN_GPIO		GPIOB
#define LED_GREEN_PIN		19

#define LED_BLUE_PORT		PORTD
#define LED_BLUE_GPIO		GPIOD
#define LED_BLUE_PIN		1


/*--------------------------------------- CC1120 Pins ----------------------------------------*/

#define CC1120_INT_PORT		PORTA
#define CC1120_INT_GPIO		GPIOA
#define CC1120_INT_PIN		5

#define CC1120_CSn_PORT		PORTD
#define CC1120_CSn_GPIO		GPIOD
#define CC1120_CSn_PIN		0

#define CC1120_CSnCHK_PORT		PORTC
#define CC1120_CSnCHK_GPIO		GPIOC
#define CC1120_CSnCHK_PIN		17

#define CC1120_RST_PORT		PORTB
#define CC1120_RST_GPIO		GPIOB
#define CC1120_RST_PIN		8

#define CC1120_MISO_PORT	PORTC
#define CC1120_MISO_GPIO	GPIOC
#define CC1120_MISO_PIN		17

#define CC1120_GPIO0_PORT	PORTA
#define CC1120_GPIO0_GPIO	GPIOA
#define CC1120_GPIO0_PIN	5

#define CC1120_GPIO2_PORT	PORTA
#define CC1120_GPIO2_GPIO	GPIOA
#define CC1120_GPIO2_PIN	12

#define CC1120_GPIO3_PORT	PORTA
#define CC1120_GPIO3_GPIO	GPIOA
#define CC1120_GPIO3_PIN	13

#define NETSTACK_CONF_RADIO cc1120_driver
//#define NETSTACK_CONF_RADIO nullradio_driver

#define CONTIKIMAC_CONF_CCA_CHECK_TIME		RTIMER_ARCH_SECOND/1600
#define CONTIKIMAC_CONF_CCA_COUNT_MAX		2
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
#define RDC_CONF_HARDWARE_CSMA 0
#define RDC_CONF_HARDWARE_ACK 1
#define CONTIKIMAC_CONF_INTER_PACKET_INTERVAL	0	//RTIMER_ARCH_SECOND/400	/* ~2.5ms */
#define CONTIKIMAC_CONF_CCA_SLEEP_TIME  RTIMER_ARCH_SECOND/210 //210 ~4.8ms 140			/* 140 = ~7.1ms, 286 = ~3.5ms */
#define CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED  RTIMER_ARCH_SECOND/20	/* ~50ms */
#define CONTIKIMAC_CONF_SHORTEST_PACKET_SIZE 36

#define NULLRDC_CONF_802154_AUTOACK_HW	1

/*---------------------------------------- IP Config ----------------------------------------*/
#define WITH_ASCII 1


#define PLATFORM_HAS_BUTTON 0

#if NETSTACK_CONF_WITH_IPV6

/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#define NETSTACK_CONF_MAC     csma_driver
#define NETSTACK_CONF_RDC     contikimac_driver
//#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_CONF_FRAMER  framer_802154


#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 63
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER 0
#define NETSTACK_RDC_CHANNEL_CHECK_RATE  8


#define QUEUEBUF_CONF_NUM                4


#else /* WITH_UIP6 */

/* Network setup for non-IPv6 (rime). */
#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC     csma_driver
#define NETSTACK_CONF_RDC     contikimac_driver
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#define NETSTACK_CONF_FRAMER  framer_802154

#define COLLECT_CONF_ANNOUNCEMENTS       1
#define CONTIKIMAC_CONF_ANNOUNCEMENTS    0

#define CONTIKIMAC_CONF_COMPOWER         1
#define XMAC_CONF_COMPOWER               1
#define CXMAC_CONF_COMPOWER              1

#define COLLECT_NBR_TABLE_CONF_MAX_NEIGHBORS      32

#define QUEUEBUF_CONF_NUM          8


#endif /* WITH_UIP6 */

#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 0

#define NETSTACK_RADIO_MAX_PAYLOAD_LEN 125

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL              42
#endif /* RF_CHANNEL */

#define IEEE802154_CONF_PANID       0xABCD




#ifdef NETSTACK_CONF_WITH_IPV6

#define LINKADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_ROUTER                 1
#define NETSTACK_CONF_WITH_IPV6_RPL               1

/**
 * Feshie deployment max neighbors is 5 (Router1). 8 is a safe max.
 */
#define NBR_TABLE_CONF_MAX_NEIGHBORS     8

/**
 * Feshie deployment has 8 nodes -> max 8 routes. 10 is a safe max.
 */
#define UIP_CONF_MAX_ROUTES   10

#define UIP_CONF_ND6_SEND_RA		0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define NETSTACK_CONF_WITH_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         0
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#define UIP_CONF_BUFFER_SIZE		240

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC	1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#else /* NETSTACK_CONF_WITH_IPV6 */
#define UIP_CONF_IP_FORWARD      1
#define UIP_CONF_BUFFER_SIZE     108
#endif /* NETSTACK_CONF_WITH_IPV6 */

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1


#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_RECEIVE_WINDOW  48
#define UIP_CONF_TCP_MSS         48
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
//#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

#define UIP_CONF_TCP_SPLIT       0


#endif /* CONTIKI_CONF_H_CDBB4VIH3I__ */
