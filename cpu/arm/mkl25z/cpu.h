/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	cpu.h	
 *	
 *	Initialisation & Interrupt handler for CPU on the Freescale FRDM-KL25z Freedom Board
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	27/2/2012	Rev.01		Configures CPU for 20.97152MHz Bus, Core and MCG Clock sources from the internal 32K reference oscillator.
 *							CPU runs in FEI. MGCIRCLK source is set to the slow internal oscillator.
 *							VLPM, LLS and VLLS are "allowed". NMI Interrupt is enabled on PTA4, Reset is PTA20.
 *	5/3/2013	Rev.02		Functions to enter low power modes.	
 *	12/3/2013	Rev.03		Added definitions for clock values.	
 *	18/2/2014	Rev.04		Options for 21MHz (from 32k internal) or 48MHz (from 8MHz external) clocks.
 *	
 *	
 *	***NB*** This file is intended for use with a new "Bareboard" project (with no rapid-development) in Code Warrior.		
 *				If this is not the case, you MUST ensure that an appropriate entry of "NMI_Handler" exists in entry 2 
 *				(address 0x00000008) of the VectorTable.
 *
 *				Appropriate entries for the interrupt handler exist in the vector table contained in the generated 
 *				kinetis_sysinit.c file	
 */						
/*-----------------------------------------------------------------------------------------------------------------------------------*/

#ifndef __CPU_H
#define __CPU_H

#define PORTB_EN_MASK	0x01
#define PORTC_EN_MASK	0x02
#define PORTD_EN_MASK	0x04
#define PORTE_EN_MASK	0x08

#if defined(__21mhzIR)
	#define CORECLK			20971520
	#define BUSCLK			20971520
#else
	#define CORECLK			48000000
	#define BUSCLK			24000000
#endif 

#if defined(OSCK32KSEL_RTCIN)
	#define MCGIRCLK		32768
#else
	#define MCGIRCLK		1000
#endif 




typedef enum {
  Mode_Stop,
  Mode_PStop1,
  Mode_PStop2,
  Mode_LLS,
  Mode_VLPS,
  Mode_VLLS0,
  Mode_VLLS1,
  Mode_VLLS3
} Type_StopMode;            				/* Stop Mode selection. */

void port_enable(uint8_t PortMask);			/* Enable clock to used ports.  This is required before configuring the port. */

void cpu_init(void);						/* Configure CPU for 20MHz clock from 32K internal oscillator */

/* Power Control Functions */
void cpu_run(void);							/* Enter run mode. */
void cpu_wait(void);						/* Enter wait mode. */
void cpu_stop(Type_StopMode StopMode);		/* Enter stop mode. */


void NMI_Handler(void);						/* NMI Interrupt Handler */

#endif /* __CPU_H */
