#include <sys/rtimer.h>

#include <stdio.h>
#include <MKL25Z4.h>

#include "nvic.h"

#include "rtimer-arch.h"



void
rtimer_arch_init(void)
{
	LPTMR0_CSR &= ~0xFF;											/* Clear CSR settings. */
	
	SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;								/* Set SIM_SCGC5: Enable LPTMR clock. LPTMR=1. Page 206. */
	
	LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;								/* Clear TCF. */
	
	LPTMR0_PSR = LPTMR_PSR_PRESCALE(0x10) | LPTMR_PSR_PCS(0x01); 	/* Set LPTMR0_PSR: Configure for 125Hz counter: 1kHz LPO as clock source with divide-by-8 prescale. Page 90, 123 & 590. */
	
	NVIC_Set_Priority(IRQ_LPTMR0, 0x80);							/* Set priority of LPTMR0 interrupt. */
	NVIC_ENABLE_INT(IRQ_LPTMR0);									/* Enable LPTMR0 interrupt in NVIC. */
	
	LPTMR0_CMR = 0xFFFF;											/* Set compare register to trigger interrupt at overflow. */
	
	LPTMR0_CSR |= LPTMR_CSR_TIE_MASK | LPTMR_CSR_TEN_MASK ;				/* Enable timer and interrupt. */
	
	//PRINTF("rtimer_arch_init: Done\n");
}

void
rtimer_arch_schedule(rtimer_clock_t t)
{
	LPTMR0_CMR = t;
}


rtimer_clock_t
rtimer_arch_now(void)
{
  return LPTMR0_CNR;
}

/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* Interrupt Handler */
void LPTimer_IRQHandler(void)
{
	LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;				/* Clear it.														*/
	rtimer_run_next();
}
