/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	cc1120-mkl25z-rf-arch.c	
 *	
 *	Architecture code for CC1120 on the the Freescale FRDM-KL25z Freedom Board with Uni of Southampton RF module
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 *	1/5/2014	Rev.01	Modified cc1120-msp-arch.c from Thingsquare Mist for 
 *						the FRDM-KL25Z-RF platform running Contiki.	
 *
 *	Page references relate to the KL25 Sub-Family Reference Manual, Document 
 *	No. KL25P80M48SF0RM, Rev. 3 September 2012. Available on 25/02/2013 from: 
 *	http://cache.freescale.com/files/32bit/doc/ref_manual/KL25P80M48SF0RM.pdf?fr=gdc
 *	
 *	Page references to "M0 Book" refer to "The Definitive Guide to the 
 *	ARM Cortex-M0" by Joseph Yiu, ISBN 978-0-12-385477-3.
 */
/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	cc1120-mkl25z-rf-arch.c	
 *	
 *	Architecture code for CC1120 on the the Freescale FRDM-KL25z Freedom Board with Uni of Southampton RF module
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	1/5/2014	Rev.01		
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


#include "contiki.h"
#include "contiki-net.h"
#include <cc1120-config.h>
#include <cc11xx.h>
#include <cc11xx-arch.h>

#include <MKL25Z4.h>                   /* I/O map for MKL25Z128VLK4 */
#include "cpu.h"
#include "spi.h"
#include "nvic.h"
//#include "dev/leds.h"

#include <stdio.h>

#define READ_BIT 0x80
#define TEST_VALUE 0xA5

uint8_t cc1120_csn_check(void);
void ccc1120_reset(void);

/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_enable(void)
{
  printf("CC1120 select");
  /* Set CSn to low (0) */
  SPI0_csn_low();

  /* The MISO pin should go low before chip is fully enabled. */
  while (GPIOC_PDOR & 0x020000);
  printf("ed\n\r");
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_disable(void)
{
  /* Set CSn to high (1) */
  SPI0_csn_high();
}
/*---------------------------------------------------------------------------*/
int
cc1120_arch_spi_rw_byte(unsigned char c)
{
	return SPI_single_tx_rx(c, 0);
}
/*---------------------------------------------------------------------------*/
int
cc1120_arch_spi_rw(unsigned char *inBuf, unsigned char *outBuf, int len)
{
  int i;
  if(inBuf == NULL && outBuf == NULL) {
    /* error: both buffers are NULL */
    return 1;
  } else if(inBuf == NULL) {
    for(i = 0; i < len; i++) {
      (void)SPI_single_tx_rx(outBuf[i], 0);
    }
  } else if(outBuf == NULL) {
    for(i = 0; i < len; i++) {
      inBuf[i] = SPI_single_tx_rx(0, 0);
    }
  } else {
    for(i = 0; i < len; i++) {
      inBuf[i] = SPI_single_tx_rx(outBuf[i],0);
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_init(void)
{
  uint8_t test3, test2, test1, test = 0;
  
  printf("conf SPI...");
  SPI0_init();			/* Configure SPI with CSn. */

  printf("conf pins...");
  /* Configure CC1120 CSn Sense pin - Port C, Pin 17. */
  GPIOD_PDDR &= ~GPIO_PDDR_PDD(0x020000); 					/* Set pin as Input. */                                                  
                                         
  PORTC_PCR17 &= ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK);	/* Clear PCR Multiplex and Interrupt flag. */
  PORTC_PCR17 |= PORT_PCR_MUX(0x01);						/* Set mux to be basic pin. */
   
   
  /* Configure CC1120 Reset pin. */
  GPIOB_PDDR &= ~GPIO_PDDR_PDD(0x0100); 					/* Set pin as Input initially for HiZ. */                                                  
  GPIOB_PDOR &= ~GPIO_PDOR_PDO(0x0100);   					/* Set initialisation value on 1 */                                           

  PORTB_PCR8 &= ~(PORT_PCR_MUX_MASK);						/* Clear PCR Multiplex. */
  PORTB_PCR8 |= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01);		/* Clear ISF & Set mux to be basic pin. */
 
  
  /* Configure CC1120 GPIO pins. */
  /* GPIO0 is connected to Port A, Pin 5. 
   * Configure pin to have rising-edge interrupt with the 
   * pull-down resistor enabled. Clear ISF flag & set MUX
   * to GPIO. */
  //PORTA_PCR5 = PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09) | PORT_PCR_MUX(0x01) | PORT_PCR_PE_MASK;
  PORTA_PCR5 &= ~PORT_PCR_MUX_MASK;						/* Clear PCR Multiplex. */
  PORTA_PCR5 |= (PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01));	/* Clear ISF & set MUX to be basic pin. */
  
  GPIOA_PDDR &= ~GPIO_PDDR_PDD(0x3020);						/* Set pins as Input. */
  
  //NVIC_Set_Priority(IRQ_PORTA, 1);							/* Set Interrupt priority. */
  
  /* GPIO2 is connected to Port A, Pin 12. 
   * Clear ISF flag & set MUX to GPIO. */
  PORTA_PCR12 &= ~PORT_PCR_MUX_MASK;						/* Clear PCR Multiplex. */
  PORTA_PCR12 |= (PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01));	/* Clear ISF & set MUX to be basic pin. */
  
  /* GPIO3 is connected to Port A, Pin 13. 
   * Clear ISF flag & set MUX to GPIO. */
  PORTA_PCR13 &= ~PORT_PCR_MUX_MASK;						/* Clear PCR Multiplex. */
  PORTA_PCR13 |= (PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x01));	/* Clear ISF & set MUX to be basic pin. */
  
  printf("Reset CC1120...");
  /* Reset CC1120. */
  cc1120_reset();
  
  printf("Check CC1120...");
  cc1120_arch_spi_enable();
  test3 = CC11xx_ARCH_SPI_RW_BYTE(46);
  test2 = CC11xx_ARCH_SPI_RW_BYTE(TEST_VALUE);
  test1 = CC11xx_ARCH_SPI_RW_BYTE(46 | READ_BIT);
  test = CC11xx_ARCH_SPI_RW_BYTE(0);
   
  /* Deselect radio. */
  cc1120_arch_spi_disable();
  if(test != TEST_VALUE)
  {
	printf("*** NOT OK! ***\n\r");
	while(1)
	{
		printf("Check CC1120 and reset.\n\r");
	}
  }
  else
  {
		printf("OK\n\r\n\r");
  }
  //clock_delay(400);

  /* Enable Radio. */
  //cc1120_arch_spi_enable();
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_enable(void)
{
  /* Enable interrupt. */
  //NVIC_ENABLE_INT(IRQ_PORTA);
}
/*---------------------------------------------------------------------------*/

uint8_t 
cc1120_csn_check(void)
{
	/* Check if SO is high. 1 if not, 0 if it is. */
	return ((FGPIOC_PDOR & 0x020000) != 0) ? 0 : 1;
}

void
cc1120_reset(void)
{
	/* Deselect CC1120. */
	cc1120_arch_spi_disable();
	
	/*Reset CC1120. */
	GPIOB_PDDR |= GPIO_PDDR_PDD(0x0100);
	//clock_delay(600);
	GPIOB_PDDR &= ~GPIO_PDDR_PDD(0x0100);
}


/*---------------------------------------------------------------------------*/
void PORTA_IRQHandler(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(PORTA_PCR5 & PORT_PCR_ISF_MASK) {
    if(cc11xx_rx_interrupt()) {
      //LPM4_EXIT;
    }
  }

  /* Reset interrupt trigger */
  PORTA_PCR5 = PORT_PCR_ISF_MASK;
  NVIC_CLEAR_PENDING(IRQ_PORTA);
  
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/

