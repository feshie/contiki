
#include "gpio.h"
#include "contiki.h"
#include "nvic.h"
#include "sys/energest.h"
#include <string.h>

static void (*PortA_Callbacks[32])(void);
static void (*PortD_Callbacks[32])(void);


void
gpio_init()
{
	memset(PortA_Callbacks, 0, sizeof(PortA_Callbacks));
	memset(PortD_Callbacks, 0, sizeof(PortD_Callbacks));
	NVIC_CLEAR_PENDING(IRQ_PORTA);
	NVIC_ENABLE_INT(IRQ_PORTA);
	NVIC_Set_Priority(IRQ_PORTA, 1);
	
	NVIC_CLEAR_PENDING(IRQ_PORTD);
	NVIC_ENABLE_INT(IRQ_PORTD);
	NVIC_Set_Priority(IRQ_PORTD, 1);
}

void
port_register_callback(void* f, PORT_Type *port, uint8_t pin)
{
	if(port == PORTA) {
		PortA_Callbacks[pin] = f;
	} else if(port == PORTD) {
		PortD_Callbacks[pin] = f;
	} else {
		printf("ERROR: Attempt to register interrupt on un-supported port.\n\r");
	}
}

/*---------------------------------------------------------------------------*/
void
port_conf_pin(PORT_Type *Port, uint8_t Pin, uint32_t PCR)
{
	Port->PCR[Pin] = PCR;
}

uint32_t 
port_pin_to_mask(uint8_t pin)
{
	return (1 << pin);
}

void port_conf_pin_int_disable(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] = Port->PCR[1 << Pin] & ~(PORT_PCR_IRQC_MASK);	/* Clear interrupt trigger. */
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;							/* Clear ISF. */
}

void
port_conf_pin_int_zero(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;							/* Clear ISF. */
	Port->PCR[Pin] = Port->PCR[1 << Pin] & ~(PORT_PCR_IRQC_MASK);	/* Clear interrupt trigger. */
	Port->PCR[Pin] |= PORT_PCR_IRQC_ZERO;							/* Set Interrupt trigger. */
}

void
port_conf_pin_int_rise(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;							/* Clear ISF. */
	Port->PCR[Pin] = Port->PCR[1 << Pin] & ~(PORT_PCR_IRQC_MASK);	/* Clear interrupt trigger. */
	Port->PCR[Pin] |= PORT_PCR_IRQC_RISING;						/* Set Interrupt trigger. */
}

void
port_conf_pin_int_fall(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;							/* Clear ISF. */
	Port->PCR[Pin] = Port->PCR[1 << Pin] & ~(PORT_PCR_IRQC_MASK);	/* Clear interrupt trigger. */
	Port->PCR[Pin] |= PORT_PCR_IRQC_FALLING;						/* Set Interrupt trigger. */
}

void
port_conf_pin_int_edge(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;							/* Clear ISF. */
	Port->PCR[Pin] = Port->PCR[1 << Pin] & ~(PORT_PCR_IRQC_MASK);	/* Clear interrupt trigger. */
	Port->PCR[Pin] |= PORT_PCR_IRQC_EDGE;							/* Set Interrupt trigger. */
}

void
port_conf_pin_int_one(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;							/* Clear ISF. */
	Port->PCR[Pin] = Port->PCR[1 << Pin] & ~(PORT_PCR_IRQC_MASK);	/* Clear interrupt trigger. */
	Port->PCR[Pin] |= PORT_PCR_IRQC_ONE;							/* Set Interrupt trigger. */
}

uint32_t
port_read_isf(PORT_Type *Port)
{
	return Port->ISFR;
}

void
port_clr_isf(PORT_Type *Port, uint32_t Pin_Mask)
{
	Port->ISFR = Pin_Mask;
}

bool
port_read_pin_isf(PORT_Type *Port, uint8_t Pin)
{
	if(Port->PCR[Pin] & PORT_PCR_ISF_MASK) {
		return 1;
	} else {
		return 0;
	}
}

void
port_pin_clr_isf(PORT_Type *Port, uint8_t Pin)
{
	Port->PCR[Pin] |= PORT_PCR_ISF_MASK;
}


/*---------------------------------------------------------------------------*/

void 
gpio_set_input(GPIO_Type *Port, uint32_t Pin_Mask)
{
	Port->PDDR &= ~(Pin_Mask);
}

void
gpio_set_output(GPIO_Type *Port, uint32_t Pin_Mask)
{
	Port->PDDR |= Pin_Mask;
}

void
gpio_set_pin(GPIO_Type *Port, uint32_t Pin_Mask)
{
	Port->PSOR |= Pin_Mask;
}

void
gpio_clr_pin(GPIO_Type *Port, uint32_t Pin_Mask)
{
	Port->PCOR |= Pin_Mask;
}

void
gpio_tgl_pin(GPIO_Type *Port, uint32_t Pin_Mask)
{
	Port->PTOR |= Pin_Mask;
}

uint32_t
gpio_read_pin(GPIO_Type *Port, uint32_t Pin_Mask)
{
	return (((Port->PDOR & Port->PDDR) | (Port->PDIR & ~(Port->PDDR))) & Pin_Mask);
}

/*---------------------------------------------------------------------------*/

/** \brief Port A Interrupt Handler.
 */
void PORTA_IRQHandler(void)
{
	uint32_t isf; 
	uint8_t i;
	
	ENERGEST_ON(ENERGEST_TYPE_IRQ);
		
	isf = port_read_isf(PORTA);

	for(i = 0; i < 32; i++) {
		if(isf & (1 << i)) {				/* Check if pin i has triggered. */
			if(PortA_Callbacks[i] != NULL) {		/* Check if we have a callback registered for pin i. */
				(*PortA_Callbacks[i])();		/* Call the callback. */
				port_pin_clr_isf(PORTA, i);		/* Clear the interrupt status flags. */
			}
		}
	}
	
	NVIC_CLEAR_PENDING(IRQ_PORTA);

	ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

/** \brief Port D Interrupt Handler.
 */
void PORTD_IRQHandler(void)
{
	uint32_t isf;
	uint8_t i;
	
	ENERGEST_ON(ENERGEST_TYPE_IRQ);

	isf = port_read_isf(PORTA);
	
	for(i = 0; i < 32; i++) {
		if(isf & (1 << i)) {				/* Check if pin i has triggered. */
			if(PortD_Callbacks[i] != NULL) {		/* Check if we have a callback registered for pin i. */
				(*PortD_Callbacks[i])();		/* Call the callback. */
				port_pin_clr_isf(PORTD, i);		/* Clear the interrupt status flags. */
			}
		}
	}
	
	NVIC_CLEAR_PENDING(IRQ_PORTD);
 
	ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}