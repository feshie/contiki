#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "contiki-conf.h"
#include "contiki.h"
#include "platform-conf.h"
#include "sys/autostart.h"
#include "dev/leds.h"

#include <MKL25Z4.h>
#include "nvic.h"
#include "debug-uart.h"
#include "cpu.h"
#include "spi.h"
#include "gpio.h"
#include "cc1120.h"
#include "cc1120-arch.h"

//#include "dev/slip.h"
#include "dev/watchdog.h"
#include "sys/clock.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

#include "net/rime/rime.h"

#include "sys/node-id.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef NETSTACK_CONF_WITH_IPV4
#define NETSTACK_CONF_WITH_IPV4 0
#endif

#if NETSTACK_CONF_WITH_IPV4
#include "net/ip/uip.h"
#include "net/ipv4/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/ipv4/uip-over-mesh.h"
static struct uip_fw_netif slipif =
{ UIP_FW_NETIF(192, 168, 1, 2, 255, 255, 255, 255, slip_send) };
static struct uip_fw_netif meshif =
{ UIP_FW_NETIF(172, 16, 0, 0, 255, 255, 0, 0, uip_over_mesh_send) };

#endif /* NETSTACK_CONF_WITH_IPV4 */

#define UIP_OVER_MESH_CHANNEL 8
#if NETSTACK_CONF_WITH_IPV4
static uint8_t is_gateway;
#endif /* NETSTACK_CONF_WITH_IPV4 */

unsigned short node_id = 0x55;
unsigned char node_mac[8];

unsigned int idle_count = 0;

void
uip_log(char *msg)
{
	puts(msg);
}

static void
set_rime_addr(void)
{
	linkaddr_t addr;
	//int i;

	memset(&addr, 0, sizeof(linkaddr_t));
#if NETSTACK_CONF_WITH_IPV6
	memcpy(addr.u8, node_mac, sizeof(addr.u8));
#else
	if(node_id == 0) {
		for(i = 0; i < sizeof(linkaddr_t); ++i) {
		  	addr.u8[i] = node_mac[7 - i];
		}
	} else {
		addr.u8[0] = node_id & 0xff;
		addr.u8[1] = node_id >> 8;
	}
#endif
	linkaddr_set_node_addr(&addr);

}

int
main(void)
{
	cpu_init();
	watchdog_stop();
	port_enable(PORTB_EN_MASK | PORTC_EN_MASK | PORTD_EN_MASK | PORTE_EN_MASK);

	dbg_setup_uart();
	clock_init();
	rtimer_init();
	
	cpu_reboot_src();		/* Print the cause of the reboot. */ 

	gpio_init();
	leds_init();
	
	leds_on(LEDS_RED);
	
	
	node_mac[0] = 0xc1;  /* Hardcoded for Z1 */
	node_mac[1] = 0x0c;  /* Hardcoded for Revision C */
	node_mac[2] = 0x00;  /* Hardcoded to arbitrary even number so that
						  the 802.15.4 MAC address is compatible with
						  an Ethernet MAC address - byte 0 (byte 2 in
						  the DS ID) */
	node_mac[3] = 0x00;  /* Hardcoded */
	node_mac[4] = 0x00;  /* Hardcoded */
	node_mac[5] = 0x00;  /* Hardcoded */
	node_mac[6] = (uint8_t)(12345678 >> 8);
	node_mac[7] = (uint8_t)(12345678 & 0xff);

	process_init();
	process_start(&etimer_process, NULL);
	ctimer_init();
	SPI0_init();
	set_rime_addr();

	leds_on(LEDS_GREEN);
	
	NETSTACK_CONF_RADIO.init();
	
	leds_off(LEDS_RED);

	printf(CONTIKI_VERSION_STRING " started. ");

#if WITH_UIP6
	memcpy(&uip_lladdr.addr, node_mac, sizeof(uip_lladdr.addr));

	/* Setup X-MAC for 802.15.4 */
	queuebuf_init();

	//NETSTACK_RDC.init();
	//NETSTACK_MAC.init();
	//NETSTACK_NETWORK.init();
	netstack_init();
	
	PRINTF(" Net: %s\n", NETSTACK_NETWORK.name);

	printf("%s %s, channel check rate %d Hz, radio channel %d\n",
		 NETSTACK_MAC.name, NETSTACK_RDC.name,
		 CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
						 NETSTACK_RDC.channel_check_interval()),
		 RF_CHANNEL);

	queuebuf_init();
	process_start(&tcpip_process, NULL);

	printf("Tentative link-local IPv6 address ");
	{
		uip_ds6_addr_t *lladdr;
		int i;
		lladdr = uip_ds6_get_link_local(-1);
		
		for(i = 0; i < 7; ++i) {
			printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
				 lladdr->ipaddr.u8[i * 2 + 1]);
		}
		
		printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
	}

	if(!UIP_CONF_IPV6_RPL) {
		uip_ipaddr_t ipaddr;
		int i;
		uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
		uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
		uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
		printf("Tentative global IPv6 address ");
		
		for(i = 0; i < 7; ++i) {
			printf("%02x%02x:",
				 ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
		}
		printf("%02x%02x\n",
		ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
	}

#else /* WITH_UIP6 */

	//NETSTACK_RDC.init();
	//NETSTACK_MAC.init();
	//NETSTACK_NETWORK.init();
	netstack_init();

	printf("%s %s, channel check rate %lu Hz, radio channel %u\n",
		 NETSTACK_MAC.name, NETSTACK_RDC.name,
		 CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
						 NETSTACK_RDC.channel_check_interval()),
		 RF_CHANNEL);
#endif /* WITH_UIP6 */

#if NETSTACK_CONF_WITH_IPV4
	process_start(&tcpip_process, NULL);
	process_start(&uip_fw_process, NULL);	/* Start IP output */
	{
		uip_ipaddr_t hostaddr, netmask;

		uip_init();

		uip_ipaddr(&hostaddr, 172,16,
			   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
		uip_ipaddr(&netmask, 255,255,0,0);
		uip_ipaddr_copy(&meshif.ipaddr, &hostaddr);

		uip_sethostaddr(&hostaddr);
		uip_setnetmask(&netmask);
		uip_over_mesh_set_net(&hostaddr, &netmask);
		/*    uip_fw_register(&slipif);*/
		//uip_over_mesh_set_gateway_netif(&slipif);
		uip_fw_default(&meshif);
		uip_over_mesh_init(UIP_OVER_MESH_CHANNEL);
		printf("uIP started with IP address %d.%d.%d.%d\n",
		uip_ipaddr_to_quad(&hostaddr));
	}
#endif /* WITH_UIP */

	leds_on(LEDS_BLUE);
	
	energest_init();
	ENERGEST_ON(ENERGEST_TYPE_CPU);
	
	leds_off(LEDS_GREEN);

	autostart_start(autostart_processes);
	
	watchdog_start();
	
	leds_off(LEDS_BLUE);
	
	while(1) {
		uint8_t r;
		
		do {
			/* Reset watchdog. */
			watchdog_periodic();
			r = process_run();
		} while(r > 0);
		
		cpu_wait();
	}

	return 0;
}
