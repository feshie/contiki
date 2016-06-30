#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// Change our RDC driver to our own custom driver
#ifdef NETSTACK_CONF_RDC
#undef NETSTACK_CONF_RDC
#endif /* NETSTACK_CONF_RDC */
#define NETSTACK_CONF_RDC           sniffer_fake_rdc_driver

#ifdef CC2420_CONF_CHECKSUM
#undef CC2420_CONF_CHECKSUM
#endif
#define CC2420_CONF_CHECKSUM		0

#ifdef CC2420_CONF_AUTOACK
#undef CC2420_CONF_AUTOACK
#endif
#define CC2420_CONF_AUTOACK		0


/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0


#endif /* __PROJECT_CONF_H__ */

