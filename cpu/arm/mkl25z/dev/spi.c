/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	spi.c	
 *	
 *	SPI functions for the Freescale FRDM-KL25z Freedom Board
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	1/5/2014	Rev.01		Includes functions for initialising SPI0 for CC1120,
 *							controlling CSn and sending data.
 *
 *	Page references relate to the KL25 Sub-Family Reference Manual, Document 
 *	No. KL25P80M48SF0RM, Rev. 3 September 2012. Available on 25/02/2013 from: 
 *	http://cache.freescale.com/files/32bit/doc/ref_manual/KL25P80M48SF0RM.pdf?fr=gdc
 *	
 *	Page references to "M0 Book" refer to "The Definitive Guide to the 
 *	ARM Cortex-M0" by Joseph Yiu, ISBN 978-0-12-385477-3.
 *	
 *
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
/*-----------------------------------------------------------------------------------------------------------------------------------*/

#include <MKL25Z4.h>                   /* I/O map for MKL25Z128VLK4 */
#include "cpu.h"

#include "spi.h"

#include <stdlib.h>

void SPI0_init(void)
{
  port_enable(PORTD_EN_MASK | PORTC_EN_MASK); 				/* Make sure that clocks are enable to Port D and Port C */
  
  /* Configure SPI0. */
  SIM_SCGC4 |= SIM_SCGC4_SPI0_MASK;      					/* Enable clock to SPI Module */
  
  PORTD_PCR3 &= ~PORT_PCR_MUX_MASK; 						/* Clear Port D, Pin 3 Mux. */
  PORTD_PCR3 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x02);     /* Clear ISF & Set Port D, Pin 3 as MISO. */                                           
  
  PORTD_PCR2 &= ~PORT_PCR_MUX_MASK;							/* Clear Port D, Pin 2 Mux. */
  PORTD_PCR2 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x02);     /* Clear ISF & Set Port D, Pin 2 as MOSI. */                                             
  
  PORTC_PCR5 &= ~PORT_PCR_MUX_MASK;							/* Clear Port C, Pin 5 Mux. */
  PORTC_PCR5 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x02); 	/* Clear ISF & Set Port C, Pin 5 as Clock. */
  
  SPI0_C1 = (SPI_C1_MSTR_MASK | SPI_C1_SSOE_MASK);					/* Set SPI0_C1: Master Mode */
  //SPI0_C2 = SPI_C2_MODFEN_MASK;      								/* Set SPI0_C2: Default state */  
  SPI0_BR = (SPI_BR_SPPR(0x02) | SPI_BR_SPR(0x00));					/* Set baud rate register */
  //printf("SPI_BR=%02x\n\r", SPI0_BR);
  
  SPI0_C1 |= SPI_C1_SPE_MASK; 		          						/* Enable SPI module */
  //printf("\n\rSPI0_C1 = %d...", SPI0_C1);
}


/* Single SPI Send/Recieve. */
uint8_t 
SPI_single_tx_rx(uint8_t in, uint8_t module) {
	
	if(SPI0_S & SPI_S_SPRF_MASK) {					/* If there is stray data in the SPI data register, discard it. */
		(void)SPI0_D;
	}

	while((SPI0_S & SPI_S_SPTEF_MASK) == 0){		/* Wait for the transmitter to be empty. */
	}
	
	SPI0_D = in;									/* Write the data. */
	
	while((SPI0_S & SPI_S_SPTEF_MASK) == 0){		/* Wait for the transmitter to be empty. */
	}
	
	while((SPI0_S & SPI_S_SPRF_MASK) == 0) {		/* Wait for the receiver to be full. */	
	}
	
	return (SPI0_D);
}

/* Send data to SPI Address. */
uint8_t SPI_tx_and_rx(uint8_t addr, uint8_t value, uint8_t module) {

	uint8_t result;
	(void)SPI_single_tx_rx(addr,0); //Throw away the first received byte
	result = SPI_single_tx_rx(value,0);
	return result;
}
