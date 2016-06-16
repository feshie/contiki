/**
 * Board-initialisation for the Muntjac
 */

#include "contiki-conf.h"
#include "board.h"
#include "spi-arch.h"
#include "dev/xmem.h"

void board_init() {
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
