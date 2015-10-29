/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	serial.c	
 *	
 *	Initialisation & Commands for Serial communication on the Freescale FRDM-KL25z Freedom Board
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	25/2/2013	Rev.01		Includes functions for UART0 ONLY.  Configured to 
 *							function on PTA1 and PTA2. Configured for 38400 8 N 1. 
 *							Configured to run from MCGFLLCLK @ 20.97152MHz. Uses 
 *							Interrupt for RX, does NOT use interrupt for TX.
 *	7/3/2013	Rev.02		Modified UART0 IRQ to use a Callback if one is registered.
 *	12/3/2013	Rev.03		Enabled dynamic baud rate setting.
 *	5/2/2014	Rev.04		Added code for UART1 and UART2. Still static pin mappings.
 *	11/2/2014	Rev.05		
 *				
 *
 *	Page references relate to the KL25 Sub-Family Reference Manual, Document 
 *	No. KL25P80M48SF0RM, Rev. 3 September 2012. Available on 25/02/2013 from: 
 *	http://cache.freescale.com/files/32bit/doc/ref_manual/KL25P80M48SF0RM.pdf?fr=gdc
 *	
 *	Page references to "M0 Book" refer to "The Definitive Guide to the 
 *	ARM Cortex-M0" by Joseph Yiu, ISBN 978-0-12-385477-3.
 *	
 *
 *	***NB*** 	This file is intended for use with a new "Bareboard"  
 *				project (with norapid-development) in Code Warrior.		
 *				If this is not the case, you MUST ensure that an
 *				appropriate entry of "UART0_IRQHandler" exists in  
 *				entry 28 (address 0x00000070) of the VectorTable.
 *
 *				Appropriate entries for the interrupt handler exist
 *				in the vector table contained in the generated 
 *				kinetis_sysinit.c file
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

#include <stdlib.h>
#include <stdio.h>
#include <MKL25Z4.h>                   /* I/O map for MKL25Z128VLK4 */
#include "nvic.h"
#include "serial.h"
#include "cpu.h"

/* Baud Rate is set using a combination of the Baud Rate Divider and Over Sampling Ratio.	*/
/* BRD of 0x22 and OSR of 0x0F give a baud rate of 38550.5882 which functions with 38400.	*/		
#define UART0_OSR		0x0F

/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------- Local Definitions --------------------------------------------------------*/

/* Register Mask definitions.  Other used definitions are defined in MKL25Z4.h				*/
#define UART_C2_TeRe	0x0C		/* UART C2 register Transmitter & Receiver enable mask.	*/
#define UART_S1_Err		0x0F		/* UART S1 status register error mask.					*/

/*--------------------------------------------------------- End Definitions ---------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------*/

static void (*UART0_callback)(char);
static void (*UART1_callback)(char);
static void (*UART2_callback)(char);


/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* Initialisation Functions */

void serial_init(uint32_t UART0_baudrate, uint32_t UART1_baudrate, uint32_t UART2_baudrate)		/* Initialise Serial port(s).	*/
{
	UART0_init(UART0_baudrate);
	UART1_init(UART1_baudrate);
	UART2_init(UART2_baudrate);
}


void UART0_init(uint32_t baudrate) 		/* Initialise UART0.			*/
{
	if(baudrate != NULL)
	{
		uint16_t bauddiv;

		/* Configure Clocking for UART0					*/
		SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;      										/* SIM_SCGC4: System Clock Gate Register 4, Page 204.  UART0=1 */
		SIM_SOPT2 = (uint32_t)((SIM_SOPT2 & ~SIM_SOPT2_UART0SRC_MASK) | 0x04000000);	/* SIM_SOPT2: Set UART0 Clock Source, Page 195. UART0SRC=1 */

		/* Configure pins: PTA1 as RX, PTA2 as TX		*/
		PORTA_PCR1 |=  0x205;                  				/* PORTA_PCR2: PortA, Pin2 Control Register, Page 183. Sets PTA1 pin as UART0_RX */                              
		PORTA_PCR2 |=  0x205;    							/* PORTA_PCR2: PortA, Pin2 Control Register, Page 183. Sets PTA2 pin as UART0_TX */

		/* Configure Interrupt Controller for UART0		*/
		NVIC_Set_Priority(IRQ_UART0, 0x80);					/* Set UART0 priority to 2. */
		NVIC_ENABLE_INT(IRQ_UART0);							/* Enable UART0 NVIC Interrupt. */	
		UART0_callback = NULL;								/* Set Callback to NULL */
	
		//NVIC_IPR3 = (uint32_t)((NVIC_IPR3 & (uint32_t)~0x7F) | (uint32_t)0x80);	/* Set NVIC_IPR3: Irq 12 to 15 Priority Register. PRI_12=0x80. */                                             
		//NVIC_ISER |= 0x1000; 													/* Set NVIC_ISER: Irq 0 to 31 Set-Enable Register. SETENA|=0x1000. */
															
		
		/* Configure UART0 Registers					*/
		//Clear control registers to ensure reset state.
		UART0_C2 = 0x00;   									/* Clear UART0_C2: Disable TX/RX for update of settings, disable interrupts. Page 728. */
		UART0_C1 = 0x00;									/* Clear UART0_C1: Enable in wait mode & disable loop mode. Page 726. */
		UART0_C3 = 0x00;									/* Clear UART0_C3: disable transmit inversion & interrupts. Page 733. */
		UART0_C4 = 0x00;									/* Clear UART0_C4: Disable Match Address Mode & clear OSR. Page 736. */
		UART0_C5 = 0x00;									/* Clear UART0_C5: disable DMA & both-edge sampling. Page 737. */
		UART0_BDH = 0x00;									/* Clear UART0_BDH: Disable other interrupts. Page 725. */
		UART0_MA1 = 0x00;									/* Clear UART0_MA1: Match Address Register 1, only used if MAEN1 in UART0_C4, Page 735. MA=0. */
		UART0_MA2 = 0x00;									/* Clear UART0_MA2: Match Address Register 2, only used if MAEN2 in UART0_C4, Page 736. MA=0. */
		
		//Configure Baud Rate, 8 data bits, no parity and 1 stop bit
		bauddiv = CORECLK / ((UART0_OSR + 1) * baudrate);						/* Calculate Baud Rate divisor.  BR = BCLK / ((OSR +1) * BRD).  Page 726. */
		UART0_BDL = (uint8_t)(bauddiv & 0x00FF);								/* Set UART0_BDL: lower Baud Rate Divisor. Page 725. */
		UART0_BDH |= (uint8_t)((bauddiv >> 8) & 0x001F);						/* Set UART0_BDH: upper Baud Rate Divisor. Page 725. */
		UART0_C4 = (UART0_C4 & ~UART0_C4_OSR_MASK) | ((UART0_OSR) & 0x1F);		/* Set UART0_C4: oversampling rate. Page 736. */
		UART0_C1 &= ~(UART0_C1_M_MASK);											/* Set UART0_C1: 8 data bits. Page 726. */
		UART0_C1 &= ~(UART0_C1_PE_MASK);										/* Set UART0_C1: no parity. Page 726. */
		UART0_BDH &= ~(UART0_BDH_SBNS_MASK);									/* Set UART0_BDH: 1 stop bit. Page 725. */
		if ( 4 <= UART0_OSR && UART0_OSR <= 7)									/* If OSR is between 4 and 7 */
		{
			UART0_C5 |= UART0_C5_BOTHEDGE_MASK;									/* enable both edge receive sampling as per Page 737. */
		}
		  
		// Clear status Registers  
		UART0_S1 |= (uint8_t)(UART_S1_IDLE_MASK | UART_S1_Err);	/* UART0_S1: Set UART0 Status Register S1, Page 729. IDLE=1,OR=1,NF=1,FE=1,PF=1. */
		UART0_S2 = (uint8_t)0x00;								/* UART0_S2: Set UART0 Status Register S2, Page 729. LBKDIF=0,RXEDGIF=0,MSBF=0,RXINV=0,RWUID=0,BRK13=0,LBKDE=0,RAF=0. */
		
		//Enable the Transmitter & Receiver after UART0 settings update
		UART0_C2 |= (uint8_t)UART_C2_TeRe;  					/* UART0_C2: Enable TX/RX, Page 728.  TIE=0,TCIE=0,RIE=0,ILIE=0,TE=1,RE=1,RWU=0,SBK=0. */
	}
}

void UART1_init(uint32_t baudrate) 		/* Initialise UART1.			*/
{
	if(baudrate != NULL)
	{
		uint16_t bauddiv;

		/* Configure Clocking for UART0					*/
		SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;      		/* SIM_SCGC4: System Clock Gate Register 4, Page 204.  UART1=1 */
		
		/* Configure pins: PTE1 as RX, PTE0 as TX		*/
		PORTE_PCR1 |= 0x300;                                                 
		PORTE_PCR0 |= 0x300; 
		
		/* Configure Interrupt Controller for UART1		*/
		NVIC_Set_Priority(IRQ_UART1, 0x80);					/* Set UART1 priority to 2. */
		NVIC_ENABLE_INT(IRQ_UART1);							/* Enable UART1 NVIC Interrupt. */	
		UART1_callback = NULL;								/* Set Callback to NULL */
		
		//NVIC_IPR3 = (uint32_t)((NVIC_IPR3 & (uint32_t)~(uint32_t)(NVIC_IP_PRI_13(0x7F))) | (uint32_t)(NVIC_IP_PRI_13(0x80)));                                                  
		//NVIC_ISER |= 0x2000;  
		
		
		/* Configure UART1 Registers					*/
		//Clear control registers to ensure reset state.
		UART1_C2 = 0x00;   									/* Clear UART1_C2: Disable TX/RX for update of settings, disable interrupts. Page 753. */
		UART1_C1 = 0x00;									/* Clear UART1_C1: Enable in wait mode & disable loop mode. Page 752. */
		UART1_C3 = 0x00;									/* Clear UART1_C3: disable transmit inversion & interrupts. Page 758. */
		UART1_C4 = 0x00;									/* Clear UART1_C4: Disable Match Address Mode & clear OSR. Page 760. */
		UART1_BDH = 0x00;									/* Clear UART1_BDH: Disable other interrupts. Page 751. */
		UART1_BDL = 0x00;									/* Clear UART1_BDL: Commit BDH changes. Page 751. */
				
		//Configure Baud Rate, 8 data bits, no parity and 1 stop bit
		bauddiv = BUSCLK / baudrate;											/* Calculate Baud Rate divisor.  BR = BCLK / BRD.  Page 762. */
		UART1_BDH = (uint8_t)((bauddiv >> 8) & 0x001F);							/* Set UART1_BDH: upper Baud Rate Divisor. Page 751. */
		UART1_BDL = (uint8_t)(bauddiv & 0x00FF);								/* Set UART1_BDL: lower Baud Rate Divisor. Page 751. */
		

		// Clear status Registers  
		/* UART1_S1: No writes needed to clear. Page 755. */
		UART1_S2 = (uint8_t)0x00;								/* UART1_S2: Set UART1 Status Register S2, Page 756. */
		
		//Enable the Transmitter & Receiver after UART0 settings update
		UART1_C2 |= (uint8_t)UART_C2_TeRe;  					/* UART1_C2: Enable TX/RX, Page 753.  TIE=0,TCIE=0,RIE=0,ILIE=0,TE=1,RE=1,RWU=0,SBK=0. */
	}
}

void UART2_init(uint32_t baudrate) 		/* Initialise UART1.			*/
{
	if(baudrate != NULL)
	{
		uint16_t bauddiv;

		/* Configure Clocking for UART0					*/
		SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;      		/* SIM_SCGC4: System Clock Gate Register 4, Page 204.  UART1=1 */
		
		/* Configure pins: PTE23 as RX, PTE22 as TX		*/
		PORTE_PCR23 |= 0x400;                                                 
		PORTE_PCR22 |= 0x400; 
		
		/* Configure Interrupt Controller for UART2		*/
		NVIC_Set_Priority(IRQ_UART2, 0x80);					/* Set UART2 priority to 2. */
		NVIC_ENABLE_INT(IRQ_UART2);							/* Enable UART2 NVIC Interrupt. */	
		UART2_callback = NULL;								/* Set Callback to NULL */
		
		//NVIC_IPR3 = (uint32_t)((NVIC_IPR3 & (uint32_t)~(uint32_t)(NVIC_IP_PRI_14(0x7F))) | (uint32_t)(NVIC_IP_PRI_14(0x80)));
		//NVIC_ISER |= 0x4000;  
		
		/* Configure UART1 Registers					*/
		//Clear control registers to ensure reset state.
		UART2_C2 = 0x00;   									/* Clear UART2_C2: Disable TX/RX for update of settings, disable interrupts. Page 753. */
		UART2_C1 = 0x00;									/* Clear UART2_C1: Enable in wait mode & disable loop mode. Page 752. */
		UART2_C3 = 0x00;									/* Clear UART2_C3: disable transmit inversion & interrupts. Page 758. */
		UART2_C4 = 0x00;									/* Clear UART2_C4: Disable Match Address Mode & clear OSR. Page 760. */
		UART2_BDH = 0x00;									/* Clear UART2_BDH: Disable other interrupts. Page 751. */
		UART2_BDL = 0x00;									/* Clear UART2_BDL: Commit BDH changes. Page 751. */
				
		//Configure Baud Rate, 8 data bits, no parity and 1 stop bit
		bauddiv = BUSCLK / baudrate;											/* Calculate Baud Rate divisor.  BR = BCLK / BRD.  Page 762. */
		UART2_BDH = (uint8_t)((bauddiv >> 8) & 0x001F);							/* Set UART2_BDH: upper Baud Rate Divisor. Page 751. */
		UART2_BDL = (uint8_t)(bauddiv & 0x00FF);								/* Set UART2_BDL: lower Baud Rate Divisor. Page 751. */
		

		// Clear status Registers  
		/* UART1_S1: No writes needed to clear. Page 755. */
		UART2_S2 = (uint8_t)0x00;								/* UART1_S2: Set UART1 Status Register S2, Page 756. */
		
		//Enable the Transmitter & Receiver after UART0 settings update
		UART2_C2 |= (uint8_t)UART_C2_TeRe;  					/* UART1_C2: Enable TX/RX, Page 753.  TIE=0,TCIE=0,RIE=0,ILIE=0,TE=1,RE=1,RWU=0,SBK=0. */
	}
}

void UART0_reg_callback(void* UART0_Callback_Ptr)	/* Register a callback function for use in UART0_IRQHandler.	*/
{
	UART0_callback = UART0_Callback_Ptr;
}

void UART1_reg_callback(void* UART1_Callback_Ptr)	/* Register a callback function for use in UART0_IRQHandler.	*/
{
	UART1_callback = UART1_Callback_Ptr;
}

void UART2_reg_callback(void* UART2_Callback_Ptr)	/* Register a callback function for use in UART0_IRQHandler.	*/
{
	UART2_callback = UART2_Callback_Ptr;
}


/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* Serial Functions */
void UART_PutChar(uint8_t uart, char outchar)
{
	switch (uart){
		case 0:	UART0_PutChar(outchar);
				break;
		case 1:	UART1_PutChar(outchar);
				break;
		case 2:	UART2_PutChar(outchar);
				break;
		default:
				break;
		}
}

void UART0_PutChar(char outChar)	/* Put single character into UART0 output register								*/
{
	while(!(UART0_S1 & UART_S1_TDRE_MASK));		/* Check for space in TX Buffer, checking TDRE bit in UART0_S1. Page 729.  	*/
	//while((UART0_S1 & 0x80) != 0x80);
	UART0_D = (uint8_t)outChar;							/* Write char to UART0 Data buffer. Page 734.								*/
}

void UART1_PutChar(char outChar)	/* Put single character into UART1 output register								*/
{
	while(!(UART1_S1 & UART_S1_TDRE_MASK));		/* Check for space in TX Buffer, checking TDRE bit in UART0_S1. Page 729.  	*/
	//while((UART0_S1 & 0x80) != 0x80);
	UART1_D = (uint8_t)outChar;							/* Write char to UART0 Data buffer. Page 734.								*/
}

void UART2_PutChar(char outChar)	/* Put single character into UART2 output register								*/
{
	while(!(UART2_S1 & UART_S1_TDRE_MASK));		/* Check for space in TX Buffer, checking TDRE bit in UART0_S1. Page 729.  	*/
	//while((UART0_S1 & 0x80) != 0x80);
	UART2_D = (uint8_t)outChar;							/* Write char to UART0 Data buffer. Page 734.								*/
}



void UART_SendString(uint8_t uart, const unsigned char *str)
{
	switch (uart){
		case 0:	UART0_SendString(str);
				break;
		case 1:	UART1_SendString(str);
				break;
		case 2:	UART2_SendString(str);
				break;
		default:
				break;
		}

}

void UART0_SendString(const unsigned char *str)
{
	while(*str!='\0')
	{
		UART0_PutChar(*str++);		
	}
}

void UART1_SendString(const unsigned char *str)
{
	while(*str!='\0')
	{
		UART1_PutChar(*str++);		
	}
}

void UART2_SendString(const unsigned char *str)
{
	while(*str!='\0')
	{
		UART2_PutChar(*str++);		
	}
}


/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* Interrupt Control Functions */
void UART0_IRQ_en(void)
{
	UART0_C2 |= UART_C2_RIE_MASK;				/* UART0_C2: enable RX interrupt, Page 728. */
}

void UART1_IRQ_en(void)
{
	UART1_C2 |= UART_C2_RIE_MASK;				/* UART1_C2: enable RX interrupt, Page 753. */
}

void UART2_IRQ_en(void)
{
	UART2_C2 |= UART_C2_RIE_MASK;				/* UART2_C2: enable RX interrupt, Page 753. */
}


void UART0_IRQ_dis(void)
{
	UART0_C2 &= ~UART_C2_RIE_MASK;				/* UART0_C2: enable RX interrupt, Page 728. */
}

void UART1_IRQ_dis(void)
{
	UART1_C2 &= ~UART_C2_RIE_MASK;				/* UART1_C2: enable RX interrupt, Page 753. */
}

void UART2_IRQ_dis(void)
{
	UART2_C2 &= ~UART_C2_RIE_MASK;				/* UART2_C2: enable RX interrupt, Page 753. */
}


/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* Interrupt Handler */
void UART0_IRQHandler(void)					/* UART0 Interrupt Handler 		*/													
{
  register uint16_t UART0_IntStatReg = UART0_S1;				/* Copy of UART_S1, register for speed.								*/
			/* Check for error flags 		*/
	if (UART0_IntStatReg & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) 		
	{	
		UART0_S1 = (uint8_t)UART_S1_Err;					/* Write to status register to acknowledge error.					*/
		(void) UART0_D;										/* Dummy read of input data.										*/
		UART0_IntStatReg &= (uint8_t)~UART_S1_RDRF_MASK;	/* Clear RDRF flag so that data is junked.							*/
	}
	if (UART0_C2 & UART_C2_TIE_MASK)															/* TX Interrupt enabled 		*/
	{
		UART0_C2 &= (uint8_t)~UART_C2_TIE_MASK;				/* We should never get here, disable TX interrupt					*/
	}
	if (UART0_IntStatReg & UART_S1_RDRF_MASK)													/* RX Interrupt 				*/
	{
		register uint8_t inChar = UART0_D;								/* Read input data													*/
		if (UART0_callback != NULL)							/* If a call back function has been registered						*/
		{
			(*UART0_callback)(inChar);						/* Call it and send received character								*/
		}
		else												/* else																*/
		{
			#ifdef DEBUG									/* Break if in debug, otherwise do nothing.							*/
			__asm("bkpt");
			#endif
			//UART0_PutChar(inChar);							/* Echo the character back out of UART0								*/
		}
	}
}

void UART1_IRQHandler(void)					/* UART1 Interrupt Handler 		*/													
{
  register uint16_t UART1_IntStatReg = UART1_S1;			/* Copy of UART_S1, register for speed.								*/
			/* Check for error flags 		*/
	if (UART1_IntStatReg & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) 		
	{	
		UART1_S1 = (uint8_t)UART_S1_Err;					/* Write to status register to acknowledge error.					*/
		(void) UART1_D;										/* Dummy read of input data.										*/
		UART1_IntStatReg &= (uint8_t)~UART_S1_RDRF_MASK;	/* Clear RDRF flag so that data is junked.							*/
	}
	if (UART1_C2 & UART_C2_TIE_MASK)															/* TX Interrupt enabled 		*/
	{
		UART1_C2 &= (uint8_t)~UART_C2_TIE_MASK;				/* We should never get here, disable TX interrupt					*/
	}
	if (UART1_IntStatReg & UART_S1_RDRF_MASK)													/* RX Interrupt 				*/
	{
		register uint8_t inChar = UART1_D;					/* Read input data													*/
		if (UART1_callback != NULL)							/* If a call back function has been registered						*/
		{
			(*UART1_callback)(inChar);						/* Call it and send received character								*/
		}
		else												/* else																*/
		{
			#ifdef DEBUG									/* Break if in debug, otherwise do nothing.							*/
			__asm("bkpt");
			#endif
		}
	}
}

void UART2_IRQHandler(void)					/* UART2 Interrupt Handler 		*/													
{
  register uint16_t UART2_IntStatReg = UART2_S1;			/* Copy of UART_S1, register for speed.								*/
			/* Check for error flags 		*/
	if (UART2_IntStatReg & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) 		
	{	
		UART2_S1 = (uint8_t)UART_S1_Err;					/* Write to status register to acknowledge error.					*/
		(void) UART2_D;										/* Dummy read of input data.										*/
		UART2_IntStatReg &= (uint8_t)~UART_S1_RDRF_MASK;	/* Clear RDRF flag so that data is junked.							*/
	}
	if (UART2_C2 & UART_C2_TIE_MASK)															/* TX Interrupt enabled 		*/
	{
		UART2_C2 &= (uint8_t)~UART_C2_TIE_MASK;				/* We should never get here, disable TX interrupt					*/
	}
	if (UART2_IntStatReg & UART_S1_RDRF_MASK)													/* RX Interrupt 				*/
	{
		register uint8_t inChar = UART2_D;					/* Read input data													*/
		if (UART2_callback != NULL)							/* If a call back function has been registered						*/
		{
			(*UART2_callback)(inChar);						/* Call it and send received character								*/
		}
		else												/* else																*/
		{
			#ifdef DEBUG									/* Break if in debug, otherwise do nothing.							*/
			__asm("bkpt");
			#endif
		}
	}
}

