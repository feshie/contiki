
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
 *         Header file for architecture specific CC1120 functions 
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 *         Phil Basford <pjb@ecs.soton.ac.uk>
 */

#ifndef CC1120_ARCH_H
#define CC1120_ARCH_H

#include "cc1120.h"

/* Contiki Headers. */
#include "contiki.h"
#include "contiki-conf.h"

/* Platform Headers */
#include "platform-conf.h"

/* Misc Headers. */
#include <string.h>
#include <stdio.h>

void cc1120_arch_init(void);
void cc1120_arch_reset(void);
void cc1120_arch_spi_enable(void);
void cc1120_arch_spi_disable(void);
uint8_t cc1120_arch_spi_enabled(void);
uint8_t cc1120_arch_read_gpio3(void);

uint8_t cc1120_arch_spi_rw_byte(uint8_t);
uint8_t cc1120_arch_txfifo_load(uint8_t *packet, uint8_t packet_length);
void cc1120_arch_rxfifo_read(uint8_t *packet, uint8_t packet_length);
void cc1120_arch_interrupt_enable(void);
void cc1120_arch_interrupt_disable(void);
void cc1120_arch_interrupt_acknowledge(void);

void cc1120_arch_pin_init(void);

#if CC1120_CCA_PIN_PRESENT
uint8_t cc1120_arch_read_cca(void);
#endif

#endif
