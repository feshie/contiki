
/* 
 * Copyright (c) 2014, University of Southampton, Electronics and Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */				

 /**
 * \file
 *         Architecture specific CC1120 functions for the FRDM-KL25Z-RF
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 *         Phil Basford <pjb@ecs.soton.ac.uk>
 */

#include "derivative.h"
#include "cc1120.h"
#include "cc1120-arch.h"
#include "cc1120-const.h"

#include "cpu.h"
#include "spi.h"
#include "nvic.h"
#include "gpio.h"

#include <stdio.h>
#include <watchdog.h>

#define READ_BIT 0x80
#define TEST_VALUE 0xA5


#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static uint8_t enabled;

/* Busy Wait for time-outable waiting. */
#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)


/* ---------------------------- Init Functions ----------------------------- */
/*---------------------------------------------------------------------------*/

void
cc1120_arch_init(void)
{
	PRINTF("\n\rConfigure CC1120 (arch)...\n\r");	
	/* Configure pins.  This may have already been done but func is repeat-execution safe. */
	PRINTF("\tRadio Control Pins\n\r");
	cc1120_arch_pin_init();
	
	/* Init SPI.  May have already done but we need to ensure SPI is configured.*/
	PRINTF("\tSPI...\n\r");
  	SPI0_init();			/* Configure SPI with CSn. */


	/* Configure GPIO Input pins */
	PRINTF("\tGPIO Pins...\n\r");
	gpio_set_input(CC1120_GPIO2_GPIO, port_pin_to_mask(CC1120_GPIO2_PIN));						/* Set Pin A12 as input for GPIO2 */
	port_conf_pin(CC1120_GPIO2_PORT, CC1120_GPIO2_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));/* Set Pin A12 to be GPIO. */

	gpio_set_input(CC1120_GPIO3_GPIO, port_pin_to_mask(CC1120_GPIO3_PIN));						/* Set Pin A13 as input for GPIO3 */
	port_conf_pin(CC1120_GPIO3_PORT, CC1120_GPIO3_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));/* Set Pin A13 to be GPIO. */

	
	PRINTF("\tRadio Interrupt...\n\r");
	
	gpio_set_input(CC1120_INT_GPIO, port_pin_to_mask(CC1120_INT_PIN));							/* Set Pin A5 as input for GPIO0 */
	port_conf_pin(CC1120_INT_PORT, CC1120_INT_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));	/* Set Pin A5 to be GPIO. */
                                  
	port_register_callback(&cc1120_interrupt_handler, CC1120_INT_PORT, CC1120_INT_PIN);			/* Register callback. */
	port_conf_pin_int_disable(CC1120_GPIO0_PORT, CC1120_GPIO0_PIN);								/* Disable interrupt for now. */
	
    PRINTF("\tOK!\n\r");   	                            

}

/*---------------------------------------------------------------------------*/

void 
cc1120_arch_pin_init(void)
{
	/* Configure CSn pin on Port D, Pin 0 */
	gpio_set_output(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));						/* Set pin as Output.*/ 
	port_conf_pin(CC1120_CSn_PORT, CC1120_CSn_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));/* Clear ISF & set MUX to be basic pin. */
	enabled = 0;

  	/* Configure CC1120 Reset pin on Port B, Pin 8. */
	gpio_set_output(CC1120_RST_GPIO, port_pin_to_mask(CC1120_RST_PIN));						/* Set pin as Output.*/ 
	port_conf_pin(CC1120_RST_PORT, CC1120_RST_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));/* Clear ISF & set MUX to be basic pin. */

	/* Configure C17 to read MISO. */
	gpio_set_input(CC1120_CSnCHK_GPIO, port_pin_to_mask(CC1120_CSnCHK_PIN));						/* Set pin C17 as Input.*/
	port_conf_pin(CC1120_CSnCHK_PORT, CC1120_CSnCHK_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));	/* Clear ISF & set MUX to be basic pin. */

	gpio_set_pin(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));						/* Make sure that the CSn is set HIGH. */
	gpio_set_pin(CC1120_RST_GPIO, port_pin_to_mask(CC1120_RST_PIN));						/* Make sure that the Reset is set HIGH. */
}


/* ---------------------------- Reset Functions ---------------------------- */
void
cc1120_arch_reset(void)
{
	cc1120_arch_spi_disable();
	gpio_set_pin(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));	/* Assert CSn to de-select CC1120. */
	gpio_clr_pin(CC1120_RST_GPIO, port_pin_to_mask(CC1120_RST_PIN));	/* Clear !Reset pin. */
	clock_delay(CC1120_RESET_DELAY_USEC/100);							/* Delay for a little. */
	gpio_set_pin(CC1120_RST_GPIO, port_pin_to_mask(CC1120_RST_PIN));	/* Assert !Reset pin. */
	enabled = 0;
}


/* ----------------------------- SPI Functions ----------------------------- */
uint8_t
cc1120_arch_spi_enabled(void)
{
	return enabled;
}

void
cc1120_arch_spi_enable(void)
{
	if(!enabled)
	{
		rtimer_clock_t t0 = RTIMER_NOW(); 
		int i = 0;
			
		gpio_clr_pin(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));	/* Set CSn to low to select CC1120 */
		
		watchdog_periodic();

		/* The MISO pin should go LOW before chip is fully enabled. */
		while(gpio_read_pin(CC1120_CSnCHK_GPIO, port_pin_to_mask(CC1120_CSnCHK_PIN)))
		{
			if(RTIMER_CLOCK_LT((t0 + CC1120_EN_TIMEOUT), RTIMER_NOW()) )
			{
				watchdog_periodic();
				if(i == 0)
				{
					/* Timeout.  Try a SNOP and a re-enable once. */
					(void) cc1120_arch_spi_rw_byte(CC1120_STROBE_SNOP);					/* SNOP. */
					gpio_set_pin(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));	/* Disable. */
					clock_wait(50);														/* Wait. */
					gpio_clr_pin(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));	/* Enable. */
					
					i++;
				}
				else
				{
					printf("Radio Barf");
					break;
				}
				
				t0 = RTIMER_NOW(); 		/* Reset timeout. */
			}
		}
		
		enabled = 1;
	}
}

/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_disable(void)
{
	/* Set CSn to high (1) */
	gpio_set_pin(CC1120_CSn_GPIO, port_pin_to_mask(CC1120_CSn_PIN));
	enabled = 0;
}

/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_spi_rw_byte(uint8_t val)
{
	return SPI_single_tx_rx(val, 0);
}

/*---------------------------------------------------------------------------*/
uint8_t 
cc1120_arch_txfifo_load(uint8_t *packet, uint8_t packet_length)
{
	uint8_t status, i;
	//status = cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_STANDARD_BIT | CC1120_WRITE_BIT);
	status = cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_BURST_BIT | CC1120_WRITE_BIT);
	cc1120_arch_spi_rw_byte(packet_length);
	
	for(i = 0; i < packet_length; i++)
	{
		cc1120_arch_spi_rw_byte(packet[i]);
	}
	
	return status;
}


/*---------------------------------------------------------------------------*/
void 
cc1120_arch_rxfifo_read(uint8_t *packet, uint8_t packet_length)
{
	uint8_t i;
	
	(void) cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_BURST_BIT | CC1120_READ_BIT);
	
	for(i = 0; i < packet_length; i++)
	{
		packet[i] = cc1120_arch_spi_rw_byte(0);
	}
	watchdog_periodic();
}


/*---------------------------------------------------------------------------*/
uint8_t 
cc1120_arch_read_cca(void)
{
	/*if(CC1120_GDO3_PORT(IN) & BV(CC1120_GDO3_PIN))
	{
		return 1;
	}
	else
	{*/
		return 0;
	//}
}


uint8_t
cc1120_arch_read_gpio2(void)
{
		return 0;
}

/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_read_gpio3(void)
{
	if(gpio_read_pin(CC1120_GPIO3_GPIO,port_pin_to_mask(CC1120_GPIO3_PIN)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/* -------------------------- Interrupt Functions -------------------------- */

void
cc1120_arch_interrupt_enable(void)
{
	/* Reset interrupt trigger */
	port_pin_clr_isf(CC1120_INT_PORT, CC1120_INT_PIN);
	
	port_conf_pin_int_rise(CC1120_INT_PORT, CC1120_INT_PIN);

}

/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_disable(void)
{
	/* Disable interrupt on the GDO0 pin */
	port_conf_pin_int_disable(PORTA, 5);
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_acknowledge(void)
{
	/* Reset interrupt trigger */
  	port_pin_clr_isf(CC1120_INT_PORT, CC1120_INT_PIN);
  	
  	if(SPI0_S & SPI_S_SPRF_MASK) {					/* If there is stray data in the SPI data register, discard it. */
		(void)SPI0_D;
	}
}
