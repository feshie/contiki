/**
 * Board-initialisation for the Muntjac
 */

#include <stdio.h>
#include "contiki-conf.h"
#include "board.h"
#include "spi-arch.h"
#include "dev/xmem.h"
#include "reset-sensor.h"

void board_init() {
    // Update and print reset sensor
    printf("Reset count: %d\n", reset_sensor.configure(0,0));

    // TODO - setup all output pins as GPIO

    // Initialize and deselect SD CSN
    spix_cs_init(USD_CSN_PORT, USD_CSN_PIN);
    SPIX_CS_SET(USD_CSN_PORT, USD_CSN_PIN);

    // Power off SD card
    GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(USD_PWR_PORT), GPIO_PIN_MASK(USD_PWR_PIN));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(USD_PWR_PORT), GPIO_PIN_MASK(USD_PWR_PIN));
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(USD_PWR_PORT), GPIO_PIN_MASK(USD_PWR_PIN));

    xmem_init();
}
