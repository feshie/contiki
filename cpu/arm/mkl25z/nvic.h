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
* This file is part of the Contiki operating system.
*/

/**
* \file
* Header file for the MKL25Z NVIC functions.
*
* \author
* Graeme Bragg - <g.bragg@ecs.soton.ac.uk>
*/

#ifndef NVIC_H_MKL25z__
#define NVIC_H_MKL25Z__

#include <MKL25Z4.h>


#define IRQ_DMA_Channel0		0
#define IRQ_DMA_Channel1		1
#define IRQ_DMA_Channel2		2
#define IRQ_DMA_Channel3		3
#define IRQ_FTFA				5
#define IRQ_PMC					6
#define IRQ_LLWU				7
#define IRQ_I2C0				8
#define IRQ_I2C1				9
#define IRQ_SPI0				10
#define IRQ_SPI1				11
#define IRQ_UART0				12
#define IRQ_UART1				13
#define IRQ_UART2				14
#define IRQ_ADC0				15
#define IRQ_CMP0				16
#define IRQ_TPM0				17
#define IRQ_TPM1				18
#define IRQ_TPM2				19
#define IRQ_RTC_Alarm			20
#define IRQ_RTC_Seconds			21
#define IRQ_PIT					22
#define IRQ_USB					24
#define IRQ_DAC0				25
#define IRQ_TSI0				26
#define IRQ_MCG					27
#define IRQ_LPTMR0				28
#define IRQ_PORTA				30
#define IRQ_PORTD				31

#define NVIC_IPR_Shift_Int0		6
#define NVIC_IPR_Shift_Int1		14
#define NVIC_IPR_Shift_Int2		22
#define NVIC_IPR_Shift_Int3		30


void NVIC_ENABLE_INT(uint32_t IRQ);

void NVIC_DISABLE_INT(uint32_t IRQ);

void NVIC_SET_PENDING(uint32_t IRQ);

void NVIC_CLEAR_PENDING(uint32_t IRQ);

void NVIC_Set_Priority(uint32_t IRQ, uint8_t priority);

void NVIC_SET_SYSTICK_PRI(uint8_t priority);

#endif /* NVIC_H_MKL25Z__ */