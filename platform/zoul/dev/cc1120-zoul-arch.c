
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
 *         Architecture specific CC1120 functions for the Zolertia Zoul
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 *         Phil Basford <pjb@ecs.soton.ac.uk>
 */

#include "cc1120.h"
#include "cc1120-arch.h"
#include "cc1120-const.h"
#include "board.h"

#include "spi-arch.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"

#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#define LEDS_ON(x) leds_on(x)
#endif

#include <stdio.h>
#include <watchdog.h>

static uint8_t enabled;

/* Busy Wait for time-outable waiting. */
#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)

#define CC1200_SPI_CLK_PORT_BASE   GPIO_PORT_TO_BASE(SPI0_CLK_PORT)
#define CC1200_SPI_CLK_PIN_MASK    GPIO_PIN_MASK(SPI0_CLK_PIN)
#define CC1200_SPI_MOSI_PORT_BASE  GPIO_PORT_TO_BASE(SPI0_TX_PORT)
#define CC1200_SPI_MOSI_PIN_MASK   GPIO_PIN_MASK(SPI0_TX_PIN)
#define CC1200_SPI_MISO_PORT_BASE  GPIO_PORT_TO_BASE(SPI0_RX_PORT)
#define CC1200_SPI_MISO_PIN_MASK   GPIO_PIN_MASK(SPI0_RX_PIN)
#define CC1200_SPI_CSN_PORT_BASE   GPIO_PORT_TO_BASE(CC1200_SPI_CSN_PORT)
#define CC1200_SPI_CSN_PIN_MASK    GPIO_PIN_MASK(CC1200_SPI_CSN_PIN)
#define CC1200_GDO0_PORT_BASE      GPIO_PORT_TO_BASE(CC1200_GDO0_PORT)
#define CC1200_GDO0_PIN_MASK       GPIO_PIN_MASK(CC1200_GDO0_PIN)
#define CC1200_GDO2_PORT_BASE      GPIO_PORT_TO_BASE(CC1200_GDO2_PORT)
#define CC1200_GDO2_PIN_MASK       GPIO_PIN_MASK(CC1200_GDO2_PIN)
#define CC1200_RESET_PORT_BASE     GPIO_PORT_TO_BASE(CC1200_RESET_PORT)
#define CC1200_RESET_PIN_MASK      GPIO_PIN_MASK(CC1200_RESET_PIN)

#if CC1120INTDEBUG || DEBUG
	#define PRINTFINT(...) printf(__VA_ARGS__)
#else
	#define PRINTFINT(...) do {} while (0)
#endif

#if CC1120_DEBUG_PINS
void
cc1120_debug_pin_cca(uint8_t val)
{
	if(val == 1) {
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC2_PIN));	
	} else {
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC2_PIN));
	}
}

void
cc1120_debug_pin_rx(uint8_t val)
{
	if(val == 1) {
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC1_PIN));	
	} else {
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC1_PIN));
	}	
}
#endif

/* ---------------------------- Init Functions ----------------------------- */
/*---------------------------------------------------------------------------*/
void
cc1120_int_handler(uint8_t port, uint8_t pin)
{
	PRINTFINT("\n\tInt\n");
	/* To keep the gpio_register_callback happy */
	cc1120_interrupt_handler();
}

/*---------------------------------------------------------------------------*/

void
cc1120_arch_init(void)
{
	/* First leave RESET high */
	GPIO_SOFTWARE_CONTROL(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);
	GPIO_SET_OUTPUT(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);
	ioc_set_over(CC1200_RESET_PORT, CC1200_RESET_PIN, IOC_OVERRIDE_OE);
	GPIO_SET_PIN(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);

	/* Initialize CSn, enable CSn and then wait for MISO to go low*/
	spix_cs_init(CC1200_SPI_CSN_PORT, CC1200_SPI_CSN_PIN);

	/* Initialize SPI */
	spix_init(CC1200_SPI_INSTANCE);
	//spix_set_mode(CC1200_SPI_INSTANCE, SSI_CR0_FRF_MOTOROLA,
    //               0, 0, 8);

	/* Configure GPIOx */
	GPIO_SOFTWARE_CONTROL(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
	GPIO_SET_INPUT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
	GPIO_SOFTWARE_CONTROL(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
	GPIO_SET_INPUT(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);

	/* Leave CSn as default */
	cc1120_arch_spi_disable();

	/* Ensure MISO is high */
	BUSYWAIT_UNTIL(
		GPIO_READ_PIN(CC1200_SPI_MISO_PORT_BASE, CC1200_SPI_MISO_PIN_MASK),
		RTIMER_SECOND / 10);
	
#if CC1120_DEBUG_PINS
	/* Use ADC 2 as Channel Clear. */
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC1_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC1_PIN));
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC1_PIN));
	
	/* Use ADC 1 as RX Indicator. */
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC2_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC2_PIN));
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ADC_SENSORS_PORT), GPIO_PIN_MASK(ADC_SENSORS_ADC2_PIN));
#endif
	
}

/*---------------------------------------------------------------------------*/

void 
cc1120_arch_pin_init(void)
{
	GPIO_SOFTWARE_CONTROL(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
	GPIO_SET_INPUT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
	GPIO_DETECT_EDGE(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
	GPIO_TRIGGER_SINGLE_EDGE(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);

	GPIO_DETECT_RISING(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);

	ioc_set_over(CC1200_GDO0_PORT, CC1200_GDO0_PIN, IOC_OVERRIDE_PUE);
	nvic_interrupt_enable(CC1200_GPIOx_VECTOR);
	gpio_register_callback(cc1120_int_handler, CC1200_GDO0_PORT,
						 CC1200_GDO0_PIN);
}


/* ---------------------------- Reset Functions ---------------------------- */
void
cc1120_arch_reset(void)
{
	cc1120_arch_spi_disable();										/* Assert CSn to de-select CC1120. */

	
	GPIO_CLR_PIN(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);	/* Clear !Reset pin. */
	clock_delay_usec(CC1120_RESET_DELAY_USEC);						/* Delay for a little. */
	GPIO_SET_PIN(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);	/* Assert !Reset pin. */
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
		
		/* Set CSn to low (0) */
		GPIO_CLR_PIN(CC1200_SPI_CSN_PORT_BASE, CC1200_SPI_CSN_PIN_MASK);
		
		watchdog_periodic();

		/* The MISO pin should go LOW before chip is fully enabled. */
		while(GPIO_READ_PIN(CC1200_SPI_MISO_PORT_BASE, CC1200_SPI_MISO_PIN_MASK))
		{
			if(RTIMER_CLOCK_LT((t0 + CC1120_EN_TIMEOUT), RTIMER_NOW()) )
			{
				watchdog_periodic();
				if(i == 0)
				{
					/* Timeout.  Try a SNOP and a re-enable once. */
					(void) cc1120_arch_spi_rw_byte(CC1120_STROBE_SNOP);					/* SNOP. */
					GPIO_SET_PIN(CC1200_SPI_CSN_PORT_BASE, CC1200_SPI_CSN_PIN_MASK);	/* Disable. */
					clock_wait(50);														/* Wait. */
					GPIO_CLR_PIN(CC1200_SPI_CSN_PORT_BASE, CC1200_SPI_CSN_PIN_MASK);	/* Enable. */
					
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
	/* Set CSn to high (1) */
	GPIO_SET_PIN(CC1200_SPI_CSN_PORT_BASE, CC1200_SPI_CSN_PIN_MASK);
		
	enabled = 0;
}

/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_spi_rw_byte(uint8_t val)
{
	SPI_WAITFORTx_BEFORE();
	SPIX_BUF(CC1200_SPI_INSTANCE) = val;
	SPIX_WAITFOREOTx(CC1200_SPI_INSTANCE);
	SPIX_WAITFOREORx(CC1200_SPI_INSTANCE);
	return SPIX_BUF(CC1200_SPI_INSTANCE);
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
cc1120_arch_read_gpio2(void)
{
	return GPIO_READ_PIN(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
}

uint8_t
cc1120_arch_read_gpio3(void)
{
	return 0;
}

/* -------------------------- Interrupt Functions -------------------------- */

/* On the Z1, the interrupt is shared with the ADXL345 accelerometer.  
 * The interrupt routine is handled in adcl345.c. */
 
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_enable(void)
{
	PRINTFINT("\nInterrupt Enabled\n");
	/* Reset interrupt trigger */
	
	/* Enable interrupt on the GDO0 pin */
	GPIO_ENABLE_INTERRUPT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
}

/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_disable(void)
{
	PRINTFINT("\nInterrupt Disabled\n");
	/* Disable interrupt on the GDO0 pin */
	GPIO_DISABLE_INTERRUPT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
	/* Reset interrupt trigger */
	
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_acknowledge(void)
{
	
}


