
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
 *         Architecture specific CC1120 functions for the Zolertia Z1 
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 *         Phil Basford <pjb@ecs.soton.ac.uk>
 */

#include "cc1120.h"
#include "cc1120-arch.h"
#include "cc1120-const.h"

#include "dev/spi.h"
#include "dev/leds.h"

#include "isr_compat.h"
#include <stdio.h>
#include <watchdog.h>

#define LEDS_ON(x) leds_on(x)

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
	/* Configure pins.  This may have already been done but func is repeat-execution safe. */
	cc1120_arch_pin_init();
	
	/* Init SPI.  May have already done but we need to ensure SPI is configured.  On Z1 this is done in main. */
	//spi_init();

	/* Setup GPIO pins.  All are Inputs for now. */
	CC1120_GDO0_PORT(SEL) &= ~BV(CC1120_GDO0_PIN);
	CC1120_GDO0_PORT(DIR) &= ~BV(CC1120_GDO0_PIN);
	CC1120_GDO0_PORT(REN) |= BV(CC1120_GDO0_PIN);
	CC1120_GDO0_PORT(OUT) &= ~BV(CC1120_GDO0_PIN);
	
	/* Set CC1120 to a rising edge interrupt. */
	CC1120_GDO0_PORT(IES) &= ~BV(CC1120_GDO0_PIN);
	
#ifdef CC1120_GPIO2_FUNC	
	CC1120_GDO2_PORT(SEL) &= ~BV(CC1120_GDO2_PIN);
	CC1120_GDO2_PORT(DIR) &= ~BV(CC1120_GDO2_PIN);
#endif


	CC1120_GDO3_PORT(SEL) &= ~BV(CC1120_GDO3_PIN);
	CC1120_GDO3_PORT(DIR) &= ~BV(CC1120_GDO3_PIN);
	CC1120_GDO3_PORT(REN) |= BV(CC1120_GDO3_PIN);
	CC1120_GDO3_PORT(OUT) &= ~BV(CC1120_GDO3_PIN);

}

/*---------------------------------------------------------------------------*/

void 
cc1120_arch_pin_init(void)
{
	/* Turn off CC2420. */
	CC2420_PWR_PORT(DIR) |= BV(CC2420_PWR_PIN);
	CC2420_PWR_PORT(OUT) &= ~BV(CC2420_PWR_PIN); 
	CC2420_CSN_PORT(DIR) |= BV(CC2420_CSN_PIN);
	CC2420_CSN_PORT(OUT) |= BV(CC2420_CSN_PIN); 
	CC2420_RESET_PORT(DIR) |= BV(CC2420_RESET_PIN);
	CC2420_RESET_PORT(OUT) |= BV(CC2420_RESET_PIN);
	
	/* Setup !RESET and CSn pins. */
	CC1120_SPI_CSN_PORT(DIR) |= BV(CC1120_SPI_CSN_PIN);		/* Set CSn pin to Output. */
	CC1120_SPI_CSN_PORT(SEL) &= ~BV(CC1120_SPI_CSN_PIN);	/* Set CSn pin to GPIO mode. */
	CC1120_SPI_CSN_PORT(OUT) |= BV(CC1120_SPI_CSN_PIN);		/* Set CSn high to de-select radio. */
	enabled = 0;

	CC1120_RESET_PORT(DIR) |= BV(CC1120_RESET_PIN);			/* Set !Reset pin to Output. */
	CC1120_RESET_PORT(SEL) &= ~BV(CC1120_RESET_PIN);		/* Set !Reset pin to GPIO mode. */
	CC1120_RESET_PORT(OUT) |= BV(CC1120_RESET_PIN);			/* Set !Reset high to ensure radio does not hog SPI. */
}


/* ---------------------------- Reset Functions ---------------------------- */
void
cc1120_arch_reset(void)
{
	CC1120_SPI_CSN_PORT(OUT) |= BV(CC1120_SPI_CSN_PIN);	/* Assert CSn to de-select CC1120. */
	CC1120_RESET_PORT(OUT) &= ~BV(CC1120_RESET_PIN);	/* Clear !Reset pin. */
	clock_delay_usec(CC1120_RESET_DELAY_USEC);	/* Delay for a little. */
	CC1120_RESET_PORT(OUT) |= BV(CC1120_RESET_PIN);		/* Assert !Reset pin. */
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
		
		/* Set CSn to low to select CC1120 */
		CC1120_SPI_CSN_PORT(OUT) &= ~BV(CC1120_SPI_CSN_PIN);
		
		watchdog_periodic();

		/* The MISO pin should go LOW before chip is fully enabled. */
		while(CC1120_SPI_MISO_PORT(IN) & BV(CC1120_SPI_MISO_PIN))
		{
			if(RTIMER_CLOCK_LT((t0 + CC1120_EN_TIMEOUT), RTIMER_NOW()) )
			{
				watchdog_periodic();
				if(i == 0)
				{
					/* Timeout.  Try a SNOP and a re-enable once. */
					(void) cc1120_arch_spi_rw_byte(CC1120_STROBE_SNOP);		/* SNOP. */
					CC1120_SPI_CSN_PORT(OUT) |= BV(CC1120_SPI_CSN_PIN);		/* Disable. */
					clock_wait(50);											/* Wait. */
					CC1120_SPI_CSN_PORT(OUT) &= ~BV(CC1120_SPI_CSN_PIN);	/* Enable. */
					
					i++;
				}
				else
				{
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
	if(enabled)
	{
		/* Set CSn to high (1) */
		CC1120_SPI_CSN_PORT(OUT) |= BV(CC1120_SPI_CSN_PIN);
		
		enabled = 0;
	}
}

/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_spi_rw_byte(uint8_t val)
{
	SPI_WAITFORTx_BEFORE();
	SPI_TXBUF = val;
	//SPI_WAITFOREOTx(); /* Causes SPI hanging when used with burst read/write. */
	SPI_WAITFOREORx();
	return SPI_RXBUF;
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
		//cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_STANDARD_BIT | CC1120_WRITE_BIT);
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
		//(void) cc1120_arch_spi_rw_byte(CC1120_FIFO_ACCESS | CC1120_STANDARD_BIT | CC1120_READ_BIT);
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


/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_read_gpio3(void)
{
	if(CC1120_GDO3_PORT(IN) & BV(CC1120_GDO3_PIN))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/* -------------------------- Interrupt Functions -------------------------- */

/* On the Z1, the interrupt is shared with the ADXL345 accelerometer.  
 * The interrupt routine is handled in adcl345.c. */
 
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_enable(void)
{
	/* Reset interrupt trigger */
	CC1120_GDO0_PORT(IFG) &= ~BV(CC1120_GDO0_PIN);
	/* Enable interrupt on the GDO0 pin */
	CC1120_GDO0_PORT(IE) |= BV(CC1120_GDO0_PIN);
}

/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_disable(void)
{
	/* Disable interrupt on the GDO0 pin */
	CC1120_GDO0_PORT(IE) &= ~BV(CC1120_GDO0_PIN);
	/* Reset interrupt trigger */
	CC1120_GDO0_PORT(IFG) &= ~BV(CC1120_GDO0_PIN);
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_acknowledge(void)
{
	/* Disable interrupt on the GDO0 pin */
	CC1120_GDO0_PORT(IE) &= ~BV(CC1120_GDO0_PIN);
	/* Reset interrupt trigger */
	CC1120_GDO0_PORT(IFG) &= ~BV(CC1120_GDO0_PIN);
	/* Enable interrupt on the GDO0 pin */
	CC1120_GDO0_PORT(IE) |= BV(CC1120_GDO0_PIN);
}


