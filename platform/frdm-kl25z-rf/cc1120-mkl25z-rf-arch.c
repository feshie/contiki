
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

#include <stdio.h>
#include <watchdog.h>

#define READ_BIT 0x80
#define TEST_VALUE 0xA5

#define DEBUG 1
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
  	GPIOA_PDDR &= (uint32_t)~GPIO_PDDR_PDD(0x2000); 				/* Set Pin A13 as input for GPIO3 */
	GPIOA_PDDR &= (uint32_t)~GPIO_PDDR_PDD(0x1000);					/* Set Pin A12 as input for GPIO2 */

	PRINTF("\tRadio Interrupt...\n\r");
	GPIOA_PDDR &= (uint32_t)~GPIO_PDDR_PDD(0x20);					/* Set Pin A5 as input for GPIO2 */
	PORTA_PCR13 = PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);				/* Set Pin A13 to be GPIO, no interrupt, clear ISF. */
	PORTA_PCR12 = PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);				/* Set Pin A12 to be GPIO, no interrupt, clear ISF. */		
	PORTA_PCR5 = PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0x09);	/* Set Pin A5 to be GPIO, Rising edge interrupt, clear ISF. */
	NVIC_Set_Priority(IRQ_PORTA, 1);						/* Set Interrupt priority. */
                                  
        PRINTF("\tOK!\n\r");   	                            

}

/*---------------------------------------------------------------------------*/

void 
cc1120_arch_pin_init(void)
{
	/* Configure CSn pin on Port D, Pin 0 */
  	GPIOD_PDDR |= GPIO_PDDR_PDD(0x01); 				/* Set pin as Output.*/                                 
  	GPIOD_PDOR |= GPIO_PDOR_PDO(0x01);    				/* Set initialisation value to 1 */                                           
  	PORTD_PCR0 = PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);		/* Clear ISF & set MUX to be basic pin. */
	enabled = 0;

  	/* Configure CC1120 Reset pin on Port B, Pin 8. */
  	GPIOB_PDDR |= GPIO_PDDR_PDD(0x0100); 				/* Set pin as Output. */                                                  
  	GPIOB_PDOR |= GPIO_PDOR_PDO(0x0100);   				/* Set initialisation value on 1 */                                           
  	PORTB_PCR8 = PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);		/* Clear ISF & Set mux to be basic pin. */


	GPIOD_PDDR &= (uint32_t)~GPIO_PDDR_PDD(0x08);			/* Set Pin D3 as an Input for reading MISO */

	GPIOD_PSOR |= GPIO_PSOR_PTSO(0x01);				/* Make sure that the CSn is set HIGH. */
	GPIOB_PSOR |= GPIO_PSOR_PTSO(0x0100);				/* Make sure that the Reset is set HIGH. */
}


/* ---------------------------- Reset Functions ---------------------------- */
void
cc1120_arch_reset(void)
{
	GPIOD_PSOR |= GPIO_PSOR_PTSO(0x01);			/* Assert CSn to de-select CC1120. */
	GPIOB_PCOR |= GPIO_PCOR_PTCO(0x0100);			/* Clear !Reset pin. */
	clock_delay(CC1120_RESET_DELAY_USEC/100);		/* Delay for a little. */
	GPIOB_PSOR |= GPIO_PSOR_PTSO(0x0100);			/* Assert !Reset pin. */
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
		GPIOD_PCOR |= GPIO_PCOR_PTCO(0x01);	
		
		PORTD_PCR3 &= ~PORT_PCR_MUX_MASK; 			  /* Clear Port D, Pin 3 Mux. */
  		PORTD_PCR3 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);     /* Set Port D, Pin 3 GPIO. */
		
	
		
		watchdog_periodic();

		/* The MISO pin should go LOW before chip is fully enabled. */
		while(GPIOD_PDIR & GPIO_PDIR_PDI(0x08))
		{
			if(RTIMER_CLOCK_LT((t0 + CC1120_EN_TIMEOUT), RTIMER_NOW()) )
			{
				watchdog_periodic();
				if(i == 0)
				{
					/* Timeout.  Try a SNOP and a re-enable once. */
					PORTD_PCR3 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x02); 	/* Set Port D, Pin 3 back to MISO. */
					(void) cc1120_arch_spi_rw_byte(CC1120_STROBE_SNOP);	/* SNOP. */
					GPIOD_PSOR |= GPIO_PSOR_PTSO(0x01);			/* Disable. */
					clock_wait(50);											/* Wait. */
					PORTD_PCR3 &= ~PORT_PCR_MUX_MASK; 			/* Clear Port D, Pin 3 Mux. */
  					PORTD_PCR3 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);   /* Set Port D, Pin 3 GPIO. */
					GPIOD_PCOR |= GPIO_PCOR_PTCO(0x01);			/* Enable. */
					
					i++;
				}
				else
				{
					break;
				}
				
				t0 = RTIMER_NOW(); 		/* Reset timeout. */
			}
		}
		PORTD_PCR3 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x02); 		/* Set Port D, Pin 3 back to MISO. */
	
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
		GPIOD_PSOR |= GPIO_PSOR_PTSO(0x01);
		enabled = 0;
	}
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


/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_read_gpio3(void)
{
	if(GPIOA_PDIR & GPIO_PDIR_PDI(0x2000))
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
	PORTA_PCR5 = PORT_PCR_ISF_MASK;
  	NVIC_CLEAR_PENDING(IRQ_PORTA);
	/* Enable interrupt on the GDO0 pin */
	NVIC_ENABLE_INT(IRQ_PORTA);
}

/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_disable(void)
{
	/* Disable interrupt on the GDO0 pin */
	NVIC_DISABLE_INT(IRQ_PORTA);
	/* Reset interrupt trigger */
	PORTA_PCR5 = PORT_PCR_ISF_MASK;
  	NVIC_CLEAR_PENDING(IRQ_PORTA);
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_acknowledge(void)
{
	/* Reset interrupt trigger */
  	PORTA_PCR5 = PORT_PCR_ISF_MASK;
  	NVIC_CLEAR_PENDING(IRQ_PORTA);
}

/*---------------------------------------------------------------------------*/
void PORTA_IRQHandler(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(PORTA_PCR5 & PORT_PCR_ISF_MASK) {
    cc1120_interrupt_handler();
  }
 
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/






