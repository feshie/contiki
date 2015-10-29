/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	nvic.c	
 *	
 *	NVIC commands & definitions for the Freescale FRDM-KL25z Freedom Board
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	11/2/2014	Rev.01		Includes functions for enabling & disabling
 *							interrupts, setting priorities and clearing 
 *							& setting pending flags.  Also includes the
 *							ability to set priority for normal IRQ
 *							interrupts and Systick.
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


/**
* \file
* MKL25Z NVIC functions.
*
* \author
* Graeme Bragg - <g.bragg@ecs.soton.ac.uk>
*/



#include "nvic.h"

/*---------------------------------------------------------------------------*/
/**
* \brief 			Enable specified interrupt in the ARM NVIC.
* \param IRQ 		IRQ of relevant interrupt as specified in Table 3-7. on Page 52 of the KL25 Sub-Family Reference Manual.
* \return 			Nil.
*
* Enable the specified interrupt in the Nested Vector
* Interrupt Controller (NVIC) after clearing any 
* pending interrupts.
*/
void 
NVIC_ENABLE_INT(uint32_t IRQ)
{
	NVIC_ICPR != (1 << IRQ);		/* Clear any pending interrupts. */
	NVIC_ISER |= (1 << IRQ);		/* Enable the specified interrupt in the NVIC. */
}

/*---------------------------------------------------------------------------*/
/**
* \brief 			Disable specified interrupt in the ARM NVIC.
* \param IRQ 		IRQ of relevant interrupt as specified in Table 3-7. on Page 52 of the KL25 Sub-Family Reference Manual.
* \return 			Nil.
*
* Disable the specified interrupt in the Nested Vector
* Interrupt Controller (NVIC).
*/
void NVIC_DISABLE_INT(uint32_t IRQ)
{
	NVIC_ICER |= (1 << IRQ);		/* Disable the specified interrupt in the NVIC. */
	//NVIC_ISER &= ~(1 << IRQ);
}

/*---------------------------------------------------------------------------*/
/**
* \brief 			Set a pending interrupt for the specified interrupt in the ARM NVIC.
* \param IRQ 		IRQ of relevant interrupt as specified in Table 3-7. on Page 52 of the KL25 Sub-Family Reference Manual.
* \return 			Nil.
*
* Set a pending interrupt for the specified interrupt in the Nested Vector
* Interrupt Controller (NVIC).
*/
void NVIC_SET_PENDING(uint32_t IRQ)
{
	NVIC_ISPR |= (1 << IRQ);		/* Set pending interrupt in the NVIC. */
}

/*---------------------------------------------------------------------------*/
/**
* \brief 			Clear a pending interrupt for the specified interrupt in the ARM NVIC.
* \param IRQ 		IRQ of relevant interrupt as specified in Table 3-7. on Page 52 of the KL25 Sub-Family Reference Manual.
* \return 			Nil.
*
* Clear a pending interrupt for the specified interrupt in the Nested Vector
* Interrupt Controller (NVIC).
*/
void NVIC_CLEAR_PENDING(uint32_t IRQ)
{
	NVIC_ICPR |= (1 << IRQ);		/* Clear pending interrupt in the NVIC. */
}

/*---------------------------------------------------------------------------*/
/**
* \brief 			Set the priority of the specified interrupt in the ARM NVIC.
* \param IRQ 		IRQ of relevant interrupt as specified in Table 3-7. on Page 52 of the KL25 Sub-Family Reference Manual.
* \param priority	Priority of the interrupt. Value between 0 and 3.
* \return 			Nil.
*
* Set the priority of the specified interrupt in the Nested Vector
* Interrupt Controller (NVIC).  The priority is a two-bit value with 
* 0 being the highest priority.
*
* Either a priority of 0 to 3 or 128
*/
void NVIC_Set_Priority(uint32_t IRQ, uint8_t priority)
{
	/* If priority has been set as 0-3, convert it. */
	if(priority < 4) {
		priority = (priority & 3) << 6;
	}
	
	/* Set priority for specified interrupt after clearing any existing priority. */
	switch (IRQ/4){
			case 0:	NVIC_IPR0 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR0 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 1:	NVIC_IPR1 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR1 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 2:	NVIC_IPR2 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR2 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 3:	NVIC_IPR3 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR3 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 4:	NVIC_IPR4 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR4 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 5:	NVIC_IPR5 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR5 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 6:	NVIC_IPR6 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR6 |= (priority & 0xC0) << ((IRQ & 3) * 8);
					break;
			case 7:	NVIC_IPR7 &= ~(0xC0 << ((IRQ & 3) * 8));
					NVIC_IPR7 |= (priority & 3) << (((IRQ & 3) * 8) + 6);
					break;
			default:
					break;
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief 			Set the priority of the SYSTICK interrupt.
* \param priority	Priority of the interrupt.
* \return 			Nil.
*
* Set the priority of the SYSTICK interrupt. 
*/
void NVIC_SET_SYSTICK_PRI(uint8_t priority)
{
	/* Set SysTick priority after clearing any existing priority. */
	SCB_SHPR2 &= ~(0xff << 24);
	SCB_SHPR2 |= priority << 24;
}