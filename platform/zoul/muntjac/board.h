/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2015, Zolertia
 * Copyright (c) 2016, University of Southampton, updated for the Muntjac
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
 * \addtogroup zoul-platforms
 * @{
 *
 * \defgroup muntjac Muntjac platform
 *
 * The Muntjac is a platform similar to the Zolertia RE-Mote (CC2538 & CC1200/CC1120)
 * that is pin-compatible with the Zolertia Z1.
 *
 * The Muntjac was designed by the University of Southampton as part of the Mountainsensing
 * project and is intended to be used with an MS1/MS2 baseboard. The Muntjac includes a TMP102
 * temperature sensor, M25P16 2MB flash, SD card and dedicated antenna outputs for 868MHz (SMA)
 * and 2.4GHz (U.FL). Unlike the Z1, separate SPI buses are used for the radio and storage.
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other Muntjac, MS1/MS2 peripherals and sensors.
 *
 * This file can be used as the basis to configure other platforms using the
 * cc2538 SoC.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the
 * Muntjac platform, cc2538-based
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
#ifndef BOARD_H_
#define BOARD_H_

// Override the Coffee config to use the data flash and not program flash
#define COFFEE_CONF_CUSTOM_PORT "muntjac/muntjac-coffee-arch.h"

#include "dev/gpio.h"
#include "dev/nvic.h"

/*---------------------------------------------------------------------------*/
/** \name Connector headers
 *
 * The Muntjac has three banks of header rows which are in the same formfactor and layout
 * as the Zolertia Z1. Due to the limited number of pins available on the CC2538, not all
 * pins on the headers are connected. Ports are defined as per the Z1 datasheet.
 * Port orientation is defined with the Micro-USB and U.FL connectors to north and the
 * SMA south.
 *
 *
 * North Port (JP1A) - Analogue IO: Only three ADCs are available on this port.
 * ADC3 and ADC 1 can be used with Z1 Phidgets. ADC2 is NOT 5V tollerant. 
 * ----------------------+---+---+---------------------------------------------
 * PIN_NAME              |  Pin  |   PIN_NAME
 * ----------------------+---+---+---------------------------------------------
 * GND                   |-01|02-|   GND
 * USB+5V                |-03|04-|   VCC+3.3V
 *                       |-05|06-|   ADC3 (PA2)
 *                       |-07|08-|   
 * ADC2 (PA4)            |-09|10-|   
 * GND                   |-11|12-|   GND
 * USB+5V                |-13|14-|   VCC+3.3V
 *                       |-15|16-|   ADC1 (PA5)
 * ----------------------+---+---+---------------------------------------------
 *
 *
 * East Port (JP1B) - Digital Busses
 * The TMP102 is powered from the sensor power control pin (PD1) so is powered
 * whenever the external sensors are enabled.
 * ----------------------+---+---+---------------------------------------------
 * PIN_NAME              |  Pin  |   PIN_NAME
 * ----------------------+---+---+---------------------------------------------
 * USBGND                |-17|18-|   GND
 * D_P                   |-19|20-|   +3.3V_Serial
 * D_N                   |-21|22-|   +3.3V
 * USB+5V                |-23|24-|   GND
 * PWR_SENSE_EN (PD1)    |-25|26-|   I2C_SCL (PC3)
 *                       |-27|28-|   I2C_SDA (PC2)
 *                       |-29|30-|   PWR_RADIO_EN (PD2)
 * UART1_RX (PC1)        |-31|32-|   PWR_SENSE_EN (PD1)
 * UART1_TX (PC0)        |-33|34-|   RADIO_SPI_CLK (PB2)
 * UART0_RX (PA0)        |-35|36-|   RADIO_SPI_MOSI (PB1)
 * UART0_TX (PA1)        |-37|38-|   RADIO_SPI_MISO (PB3)
 * ----------------------+---+---+---------------------------------------------
 *
 *
 * South Port (JP1C) - Miscellaneous pins.
 * TEMP_ALERT is only connected to the temperature alert pin of the TMP102. It is
 * not connected to the CC2538 but can be used for an external alert.
 * The SD/Flash memory SPI bus is brought out on this port rather than the east port
 * to maintain compatibility with MS1.
 * ----------------------+---+---+---------------------------------------------
 * PIN_NAME              |  Pin  |   PIN_NAME
 * ----------------------+---+---+---------------------------------------------
 * RADIO_GPIO0 (PB4)     |-54|53-|   UART0_TX (PA1)
 * SD_SPI_MISO (PC6)     |-52|51-|   
 * SD_SPI_MOSI (PC5)     |-50|49-|   
 * RADIO_SPI_CSn (PB5)   |-48|47-|   UART0_RX (PA0)
 * RS485_TXEN (PD0)      |-46|45-|   SD_SPI_CLK (PC4)
 * USER_BUTTON (PA3)     |-44|43-|   RADIO_RESET_N (PC7)
 * RADIO_GPIO2 (PB0)     |-42|41-|   TEMP_ALERT
 * +3.3V                 |-40|39-|   GND
 * ----------------------+---+---+---------------------------------------------
 */
/*---------------------------------------------------------------------------*/
/** \name Muntjac LED configuration
 *
 * LEDs on the Muntjac are connected in the same way as the RE-Mote:
 * - LED1 (Red)    -> PD5
 * - LED2 (Green)  -> PD4
 * - LED3 (Blue)   -> PD3
 *
 * LED pins are not exposed on the external headers.
 * @{
 */
/*---------------------------------------------------------------------------*/
/* Some files include leds.h before us, so we need to get rid of defaults in
 * leds.h before we provide correct definitions */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_BLUE
#undef LEDS_RED
#undef LEDS_CONF_ALL

/* In leds.h the LEDS_BLUE is defined by LED_YELLOW definition */
#define LEDS_GREEN    (1 << 4) /**< LED1 (Green) -> PD4 */
#define LEDS_BLUE     (1 << 3) /**< LED2 (Blue)  -> PD3 */
#define LEDS_RED      (1 << 5) /**< LED3 (Red)   -> PD5 */

#define LEDS_CONF_ALL (LEDS_GREEN | LEDS_BLUE | LEDS_RED)

#define LEDS_LIGHT_BLUE (LEDS_GREEN | LEDS_BLUE) /**< Green + Blue (24)       */
#define LEDS_YELLOW     (LEDS_GREEN | LEDS_RED)  /**< Green + Red  (48)       */
#define LEDS_PURPLE     (LEDS_BLUE  | LEDS_RED)  /**< Blue + Red   (40)       */
#define LEDS_WHITE      LEDS_ALL                 /**< Green + Blue + Red (56) */

/* Notify various examples that we have LEDs */
#define PLATFORM_HAS_LEDS        1
/** @} */
/*---------------------------------------------------------------------------*/
/** \name USB configuration
 *
 * The USB pullup is enabled by an external resistor, not mapped to a GPIO
 */
#ifdef USB_PULLUP_PORT
#undef USB_PULLUP_PORT
#endif
#ifdef USB_PULLUP_PIN
#undef USB_PULLUP_PIN
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART configuration
 *
 * On the Muntjac, the UARTs are connected to the following ports/pins:
 *
 * - UART0:
 *   - RX:  PA0, connected to CP2104 serial-to-usb converter TX pin
 *   - TX:  PA1, connected to CP2104 serial-to-usb converter RX pin
 * - UART1:
 *   - RX:  PC1
 *   - TX:  PC0
 *
 * UART0 and UART1 are both configured without HW pull-up resistor and neither 
 * port supports CTS or RTS. UART0 and UART1 are exposed on the east port. 
 * UART0 is also exposed on the south port.
 * @{
 */
#define UART0_RX_PORT            GPIO_A_NUM
#define UART0_RX_PIN             0
#define UART0_TX_PORT            GPIO_A_NUM
#define UART0_TX_PIN             1

#define UART1_RX_PORT            GPIO_C_NUM
#define UART1_RX_PIN             1
#define UART1_TX_PORT            GPIO_C_NUM
#define UART1_TX_PIN             0
#define UART1_CTS_PORT           (-1)
#define UART1_CTS_PIN            (-1)
#define UART1_RTS_PORT           (-1)
#define UART1_RTS_PIN            (-1)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ADC configuration
 *
 * These values configure which CC2538 pins and ADC channels to use for the ADC
 * inputs. The Muntjac has three accessible ADCs exposed on the north port. 
 * ADC 1 and ADC 3 are connected next to 3.3V and ground so that they can be
 * used with phidget-like connectors. All of the exposed ADCs support 3.3V
 * operation only.
 * 
 * - ADC1 (PA5): ADC1 on Z1 header. Connected to ADC1 header on MS1/MS2.
 * - ADC2 (PA4): ADC2 on Z1 header. Connected to ADC2 header on MS1/MS2.
 * - ADC3 (PA2): ADC7 on Z1 header. Used for Batvolts on MS1/MS1.
 *
 * The other ADC channels of the CC2538 are not available for use as the pins
 * are used for the user button and the SD card.
 * @{
 */
#define ADC_SENSORS_PORT         GPIO_A_NUM /**< ADC GPIO control port */

#ifndef ADC_SENSORS_CONF_ADC1_PIN
#define ADC_SENSORS_ADC1_PIN     5             /**< ADC1 to PA5, 3V3    */
#else
#if ((ADC_SENSORS_CONF_ADC1_PIN != -1) && (ADC_SENSORS_CONF_ADC1_PIN != 5))
#error "ADC1 channel should be mapped to PA5 or disabled with -1"
#else
#define ADC_SENSORS_ADC1_PIN ADC_SENSORS_CONF_ADC1_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC3_PIN
#define ADC_SENSORS_ADC3_PIN     2             /**< ADC3 to PA2, 3V3    */
#else
#if ((ADC_SENSORS_CONF_ADC3_PIN != -1) && (ADC_SENSORS_CONF_ADC3_PIN != 2))
#error "ADC3 channel should be mapped to PA2 or disabled with -1"
#else
#define ADC_SENSORS_ADC3_PIN ADC_SENSORS_CONF_ADC3_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC2_PIN
#define ADC_SENSORS_ADC2_PIN     4             /**< ADC2 to PA4, 3V3    */
#else
#if ((ADC_SENSORS_CONF_ADC2_PIN != -1) && (ADC_SENSORS_CONF_ADC2_PIN != 4))
#error "ADC2 channel should be mapped to PA4 or disabled with -1"
#else
#define ADC_SENSORS_ADC2_PIN ADC_SENSORS_CONF_ADC2_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC4_PIN
#define ADC_SENSORS_ADC4_PIN     (-1)          /**< ADC4 not declared    */
#else
#error "ADC4 should be disabled"
#endif

#ifndef ADC_SENSORS_CONF_ADC5_PIN
#define ADC_SENSORS_ADC5_PIN     (-1)          /**< ADC5 not declared    */
#else
#error "ADC5 should be disabled"
#endif

#ifndef ADC_SENSORS_CONF_ADC6_PIN
#define ADC_SENSORS_ADC6_PIN     (-1)          /**< ADC6 not declared    */
#else
#error "ADC6 should be disabled"
#endif

#ifndef ADC_SENSORS_CONF_MAX
#define ADC_SENSORS_MAX          3             /**< Maximum sensors    */
#else
#if (ADC_SENSORS_CONF_MAX > 3)
#error "Too many ADC sensors defined. Maximum is 3."
#else
#define ADC_SENSORS_MAX          ADC_SENSORS_CONF_MAX
#endif
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Muntjac Button configuration
 *
 * Buttons on the RE-Mote are connected as follows:
 * - BUTTON_USER  -> PA3, USR user button, shared with bootloader
 * - BUTTON_RESET -> RESET_N line, RST reset CC2538
 * @{
 */
/** BUTTON_USER -> PA3 */
#define BUTTON_USER_PORT       GPIO_A_NUM
#define BUTTON_USER_PIN        3
#define BUTTON_USER_VECTOR     NVIC_INT_GPIO_PORT_A

/* Notify various examples that we have a user button.
 * This is hard-coded as ADC6 cannot be used.
 */
#define PLATFORM_HAS_BUTTON  1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI0) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI0) lines,
 * reserved exclusively for the CC1200 RF transceiver.  These pins are exposed 
 * on the east port but their use for anything else should be avoided.
 * TX -> MOSI, RX -> MISO
 * @{
 */
#define SPI0_CLK_PORT             GPIO_B_NUM
#define SPI0_CLK_PIN              2
#define SPI0_TX_PORT              GPIO_B_NUM
#define SPI0_TX_PIN               1
#define SPI0_RX_PORT              GPIO_B_NUM
#define SPI0_RX_PIN               3

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI1) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI1) lines,
 * shared by the micro SD card and M25P16 flash. This bus is exposed on the
 * south port. No pins are available for external CSn without sacrificing other
 * functionality.
 * TX -> MOSI, RX -> MISO
 * @{
 */
#define SPI1_CLK_PORT            GPIO_C_NUM
#define SPI1_CLK_PIN             4
#define SPI1_TX_PORT             GPIO_C_NUM
#define SPI1_TX_PIN              5
#define SPI1_RX_PORT             GPIO_C_NUM
#define SPI1_RX_PIN              6
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name I2C configuration
 *
 * These values configure which CC2538 pins to use for the I2C lines, exposed
 * over JP6 connector, also available as testpoints T2 (PC2) and T3 (PC3).
 * The I2C bus is used for the RTC on MS1/MS2 and is shared with the on-board TMP102.
 * The I2C is exposed on the east port. Due to pin limitations, no interrupt pin is 
 * exposed.
 * @{
 */
#define I2C_SCL_PORT             GPIO_C_NUM
#define I2C_SCL_PIN              3
#define I2C_SDA_PORT             GPIO_C_NUM
#define I2C_SDA_PIN              2
#define I2C_INT_PORT             (-1)
#define I2C_INT_PIN              (-1)
#define I2C_INT_VECTOR           (-1)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name CC1200 configuration
 *
 * These values configure the required pins to drive the CC1200
 * These pins are exposed on the east port but their use for anything 
 * else should be avoided.
 * @{
 */
 
 
/* TODO Consolidate. */
#define RADIO_SPI_CLK_PORT		SPI0_CLK_PORT
#define RADIO_SPI_CLK_PIN		SPI0_CLK_PIN
#define RADIO_SPI_MOSI_PORT		SPI0_TX_PORT
#define RADIO_SPI_MOSI_PIN		SPI0_TX_PIN
#define RADIO_SPI_MISO_PORT		SPI0_RX_PORT
#define RADIO_SPI_MISO_PIN		SPI0_RX_PIN
#define RADIO_SPI_CSN_PORT		GPIO_B_NUM
#define RADIO_SPI_CSN_PIN		5
#define RADIO_RESET_PORT		GPIO_C_NUM
#define RADIO_RESET_PIN			7
#define RADIO_GPIO0_PORT		GPIO_B_NUM
#define RADIO_GPIO0_PIN			4
#define RADIO_GPIO2_PORT		GPIO_B_NUM
#define RADIO_GPIO2_PIN			2
 
 
#define CC1200_SPI_INSTANCE         0
#define CC1200_SPI_SCLK_PORT        SPI0_CLK_PORT
#define CC1200_SPI_SCLK_PIN         SPI0_CLK_PIN
#define CC1200_SPI_MOSI_PORT        SPI0_TX_PORT
#define CC1200_SPI_MOSI_PIN         SPI0_TX_PIN
#define CC1200_SPI_MISO_PORT        SPI0_RX_PORT
#define CC1200_SPI_MISO_PIN         SPI0_RX_PIN
#define CC1200_SPI_CSN_PORT         GPIO_B_NUM
#define CC1200_SPI_CSN_PIN          5
#define CC1200_GDO0_PORT            GPIO_B_NUM
#define CC1200_GDO0_PIN             4
#define CC1200_GDO2_PORT            GPIO_B_NUM
#define CC1200_GDO2_PIN             0
#define CC1200_RESET_PORT           GPIO_C_NUM
#define CC1200_RESET_PIN            7
#define CC1200_GPIOx_VECTOR         NVIC_INT_GPIO_PORT_B
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Storage Configuration
 *
 * These values configure the required pins to drive the built-in flash storage.
 * The Muntjac has the facility for both a microSD card and an on-board M25P16 2MB
 * flash. Both are connected to SSI1/SPI1.
 * 
 * PA6 is used to disable power to the SD card. The M25P16 is always powered. The
 * control and data lines for the SD card are buffered through a level translator
 * to prevent the SD card drawing (mA +) quiescent current through CSn and the SPI
 * bus.
 *
 * PB7 is used for SD card presence detection, but can be used for !HOLD on the
 * M25P16 with a hardware modification if required.
 * @{
 */
#define USD_CLK_PORT             SPI1_CLK_PORT
#define USD_CLK_PIN              SPI1_CLK_PIN
#define USD_MOSI_PORT            SPI1_TX_PORT
#define USD_MOSI_PIN             SPI1_TX_PIN
#define USD_MISO_PORT            SPI1_RX_PORT
#define USD_MISO_PIN             SPI1_RX_PIN
#define USD_CSN_PORT             GPIO_A_NUM
#define USD_CSN_PIN              7
#define USD_PWR_PORT             GPIO_A_NUM
#define USD_PWR_PIN              6
#define USD_DETECT_PORT          GPIO_B_NUM
#define USD_DETECT_PIN           7

#define FLASH_CSN_PORT           GPIO_B_NUM
#define FLASH_CSN_PIN			 6
#define FLASH_SPI_INSTANCE       1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name On-board external WDT
 * The Muntjac does not feature an on-board external WDT like the RE-Mote.
 * @{
 */
#define EXT_WDT_PORT                (-1)
#define EXT_WDT_PIN                 (-1)

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "UoS Muntjac platform"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
