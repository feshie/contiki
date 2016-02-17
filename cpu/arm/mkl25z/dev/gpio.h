/*
 * Copyright (c) 2012, Graeme Bragg
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
 */
/**
 * \addtogroup kl25z
 * @{
 *
 * \defgroup kl25z-gpio MKL25Z GPIO and Port
 *
 * Driver for the MKL25Z GPIO controller
 * @{
 *
 * \file
 * Header file with function declarations for the MKL25Z GPIO module
 */
#ifndef GPIO_H_
#define GPIO_H_

#include "derivative.h"
#include <stdint.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
/**
 * \name GPIO Pin Multiplexer Settings.
 * @{
 */
#define PORT_PCR_MUX_DISABLE	0x000
#define PORT_PCR_MUX_GPIO		0x100
#define PORT_PCR_MUX_ALT2		0x200
#define PORT_PCR_MUX_ALT3		0x300
#define PORT_PCR_MUX_ALT4		0x400
#define PORT_PCR_MUX_ALT5		0x500
#define PORT_PCR_MUX_ALT6		0x600
#define PORT_PCR_MUX_ALT7		0x700
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name GPIO Pin Interrupt Settings.
 * @{
 */
#define PORT_PCR_IRQC_DISABLE		0x00000
#define PORT_PCR_IRQC_DMA_RISING	0x10000
#define PORT_PCR_IRQC_DMA_FALLING	0x20000
#define PORT_PCR_IRQC_DMA_EDGE		0x30000
#define PORT_PCR_IRQC_ZERO			0x80000
#define PORT_PCR_IRQC_RISING		0x90000
#define PORT_PCR_IRQC_FALLING		0xA0000
#define PORT_PCR_IRQC_EDGE			0xB0000
#define PORT_PCR_IRQC_ONE			0xC0000

/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name GPIO Manipulation Functions
 * @{
 */

/** \brief initialise the Port interrupt callback function arrays.
 */
void gpio_init();

/**
 * \brief Register Port interrupt callback function
 * \param f Pointer to a function to be called when \a pin of \a port
 *          generates an interrupt
 * \param port Associate \a f with this port. \e port must be specified with
 *        its PORTx_BASE_PTR.
 * \param pin Associate \a f with this pin, which is specified by number
 *        from 0 to 31
 */
void port_register_callback(void* f, PORT_Type *port, uint8_t pin);

/**
 * \brief Convert a pin number (0 to 31) to a pin mask.
 * \param pin Associate \a f with this pin, which is specified by number
 *        from 0 to 31
 * \return Pinmask for the selected pin.
 */
uint32_t port_pin_to_mask(uint8_t pin);

/** \brief Configure the specified Pin of the port with PORTx_BASE_PTR with PCR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 * \param PCR - The PCR settings to be applied to the pin
 */
void port_conf_pin(PORT_Type *Port, uint8_t Pin, uint32_t PCR);

/** \brief Disable interrupt on the specified Pin of the port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 */
void port_conf_pin_int_disable(PORT_Type *Port, uint8_t Pin);

/** \brief Configure zero-level interrupt on the specified Pin of the port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 */
void port_conf_pin_int_zero(PORT_Type *Port, uint8_t Pin);

/** \brief Configure rising-edge interrupt on the specified Pin of the port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 */
void port_conf_pin_int_rise(PORT_Type *Port, uint8_t Pin);

/** \brief Configure falling-edge interrupt on the specified Pin of the port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 */
void port_conf_pin_int_fall(PORT_Type *Port, uint8_t Pin);

/** \brief Configure either-edge interrupt on the specified Pin of the port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 */
void port_conf_pin_int_edge(PORT_Type *Port, uint8_t Pin);

/** \brief Configure one-level interrupt on the specified Pin of the port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 */
void port_conf_pin_int_one(PORT_Type *Port, uint8_t Pin);

/** \brief Return the Interrupt Status Flags of port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \return The value of ISFR for the given Port.
 */
uint32_t port_read_isf(PORT_Type *Port);

/** \brief Return the Interrupt Status Flags of Pin in port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be conigured, from 0 to 31.
 * \return 1 if ISF set, 0 if not.
 */
bool port_read_pin_isf(PORT_Type *Port, uint8_t Pin);

/** \brief Clear the Interrupt Status Flag of port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
void port_clr_isf(PORT_Type *Port, uint32_t Pin_Mask);

/** \brief Return the Interrupt Status of the Pin in port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be read, from 0 to 31.
 * \return The TRUE if the ISF flag is set, FALSE if not.
 */
bool port_pin_read_isf(PORT_Type *Port, uint8_t Pin);

/** \brief Clear the Interrupt Status Flag of Pin in port with PORTx_BASE_PTR.
 * \param Port - Port base pointer
 * \param Pin - The number of the pin to be read, from 0 to 31.
 */
void port_pin_clr_isf(PORT_Type *Port, uint8_t Pin);


/** \brief Set pins with Pin_Mask of port with GPIOn_BASE_PTR to input.
 * \param Port - Port register offset
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
void gpio_set_input(GPIO_Type *Port, uint32_t Pin_Mask);

/** \brief Set pins with Pin_Mask of port with GPIOn_BASE_PTR to output.
 * \param Port - Port register offset
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
void gpio_set_output(GPIO_Type *Port, uint32_t Pin_Mask);

/** \brief Set pins with Pin_Mask of port with GPIOn_BASE_PTR high.
 * \param Port - Port register offset
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
void gpio_set_pin(GPIO_Type *Port, uint32_t Pin_Mask);

/** \brief Clear pins with Pin_Mask of port with GPIOn_BASE_PTR  low.
 * \param Port - Port register offset
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
void gpio_clr_pin(GPIO_Type *Port, uint32_t Pin_Mask);

/** \brief Toggle pins with Pin_Mask of port with GPIOn_BASE_PTR  low.
 * \param Port - Port register offset
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
void gpio_tgl_pin(GPIO_Type *Port, uint32_t Pin_Mask);


/** \brief Read pins with Pin_Mask of port with GPIOn_BASE_PTR.
 * \param Port GPIO Port register offset
 * \param Pin_Mask Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 * \return The value of the pins specified by Pin_Mask
 *
 * This macro will \e not return 0 or 1. Instead, it will return the values of
 * the pins specified by PIN_MASK ORd together. Thus, if you pass 0xC3
 * (0x80 | 0x40 | 0x02 | 0x01) as the Pin_Mask and pins 7 and 0 are high,
 * the macro will return 0x81.
 */
uint32_t gpio_read_pin(GPIO_Type *Port, uint32_t Pin_Mask);

/** @} */

#endif /* GPIO_H_ */

/**
 * @}
 * @}
 */
