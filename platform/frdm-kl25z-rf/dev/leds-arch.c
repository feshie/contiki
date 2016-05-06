
/**
 * \addtogroup FRDM-KL25Z
 * @{
 *
 * \defgroup FRDM-KL25Z-leds LED driver
 *
 * LED driver implementation for the FRDM-KL25Z
 * @{
 *
 * \file
 * LED driver implementation for the FRDM-KL25Z
 * \author
 * Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 */


/*---------------------------------------------------------------------------*/
/**
 * \brief Initialise the required GPIO pins. 
 *
 *        The FRDM-KL25Z has a tri-colour LED:
 *			Red is connected to PTB18
 *			Green is connected to PTB19
 *			Blue is connected to PTD1
 */	

#include "contiki.h"
#include "dev/leds.h"
#include "gpio.h"
#include "derivative.h" 
#include "cpu.h"
	
void
leds_arch_init(void)
{
	port_enable(PORTB_EN_MASK | PORTD_EN_MASK);		/* Enable Port B and Port D for the LEDs. */
	
	port_conf_pin(LED_RED_PORT, LED_RED_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));		/* Config Red LED. */
	gpio_set_output(LED_RED_GPIO, port_pin_to_mask(LED_RED_PIN));							/* Set as output. */
	
	port_conf_pin(LED_GREEN_PORT, LED_GREEN_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));	/* Config Green LED. */
	gpio_set_output(LED_GREEN_GPIO, port_pin_to_mask(LED_GREEN_PIN));						/* Set as output. */
	
	port_conf_pin(LED_BLUE_PORT, LED_BLUE_PIN, (PORT_PCR_MUX_GPIO | PORT_PCR_ISF_MASK));	/* Config Blue LED. */
	gpio_set_output(LED_BLUE_GPIO, port_pin_to_mask(LED_BLUE_PIN));							/* Set as output. */
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
 	uint8_t leds;
	
	leds = gpio_read_pin(LED_RED_GPIO, LED_RED_PIN) == 0? LEDS_RED : 0;
	leds |= gpio_read_pin(LED_GREEN_GPIO, LED_GREEN_PIN) == 0? LEDS_GREEN : 0;
	leds |= gpio_read_pin(LED_BLUE_GPIO, LED_BLUE_PIN) == 0? LEDS_BLUE : 0;
	
	return leds;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_GREEN) {
    gpio_clr_pin(LED_GREEN_GPIO, port_pin_to_mask(LED_GREEN_PIN));
  } else {
    gpio_set_pin(LED_GREEN_GPIO, port_pin_to_mask(LED_GREEN_PIN));
  }

  if(leds & LEDS_BLUE) {
    gpio_clr_pin(LED_BLUE_GPIO, port_pin_to_mask(LED_BLUE_PIN));
  } else {
    gpio_set_pin(LED_BLUE_GPIO, port_pin_to_mask(LED_BLUE_PIN));
  }

  if(leds & LEDS_RED) {
    gpio_clr_pin(LED_RED_GPIO, port_pin_to_mask(LED_RED_PIN));
  } else {
    gpio_set_pin(LED_RED_GPIO, port_pin_to_mask(LED_RED_PIN));
  }
}
/*---------------------------------------------------------------------------*/