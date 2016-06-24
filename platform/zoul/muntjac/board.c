/**
 * Board-initialisation for the Muntjac
 */

#include <stdio.h>
#include "contiki-conf.h"
#include "board.h"
#include "spi-arch.h"
#include "dev/xmem.h"
#include "dev/i2c.h"
#include "reset-sensor.h"

void board_init() {
    // Update and print reset sensor.
    printf("Reset count: %d\n", reset_sensor.configure(0,0));

    // TODO - setup all output pins as GPIO

    // Initialize and deselect SD CSN
    spix_cs_init(USD_CSN_PORT, USD_CSN_PIN);
    SPIX_CS_SET(USD_CSN_PORT, USD_CSN_PIN);

    /* Ensure that the SD card is powered OFF. */
    GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(PWR_SD_EN_PORT), GPIO_PIN_MASK(PWR_SD_EN_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(PWR_SD_EN_PORT), GPIO_PIN_MASK(PWR_SD_EN_PIN));
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(PWR_SD_EN_PORT), GPIO_PIN_MASK(PWR_SD_EN_PIN));

	/* Initialise the onboard flash. */
    xmem_init();

	/* Ensure that the CC1120/CC1200 is powered ON. */
    GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(PWR_RADIO_EN_PORT), GPIO_PIN_MASK(PWR_RADIO_EN_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(PWR_RADIO_EN_PORT), GPIO_PIN_MASK(PWR_RADIO_EN_PIN));
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(PWR_RADIO_EN_PORT), GPIO_PIN_MASK(PWR_RADIO_EN_PIN));

	// TODO - these are MS1/MS2 specific, do we want to stick in their own init file?
	// maybe with RTC stuff and #define it in?. Could use presence (or lack of) RTC to
	// determine whether we are connected to an MS1/MS2.
	/* Ensure that Sensors are powered OFF. */
    GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(PWR_SENSE_EN_PORT), GPIO_PIN_MASK(PWR_SENSE_EN_PIN));

	/* Ensure that RS485 TXEN is DISABLED. */
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(RS485_TXEN_PORT), GPIO_PIN_MASK(RS485_TXEN_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(RS485_TXEN_PORT), GPIO_PIN_MASK(RS485_TXEN_PIN));
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(RS485_TXEN_PORT), GPIO_PIN_MASK(RS485_TXEN_PIN));

    /* Setup I2C */
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
}
