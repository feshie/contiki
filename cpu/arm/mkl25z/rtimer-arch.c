
#include <stdio.h>

#include "rtimer-arch.h"

/*---------------------------------------------------------------------------*/
static volatile rtimer_clock_t next_trigger;
/*---------------------------------------------------------------------------*/

void
rtimer_arch_init(void)
{	
	/* TPM0 already configured and counting. */
	return;
}

void
rtimer_arch_schedule(rtimer_clock_t t)
{
	rtimer_clock_t now;
	
	/* Sanity check Value. */
	now = RTIMER_NOW();
	if((t - now) < 7) {
		t = now + 7;	
	}

	TPM0_C0V = t;						/* Set TPM0_C0V Channel 0 compare value. */
	next_trigger = t;					/* Store the value. */
	
	NVIC_EnableIRQ(TPM0_IRQn);			/* Enable TPM0 interrupt in NVIC. */
	TPM0_C0SC = TPM_CnSC_CHF_MASK 		/* Enable channel interrupt & clear flag. */
				| TPM_CnSC_CHIE_MASK 
				| TPM_CnSC_MSA_MASK;
		
}


rtimer_clock_t
rtimer_arch_now(void)
{
  	/* Return the TPM value. */
	return TPM0_CNT;
}

rtimer_clock_t
rtimer_arch_next_trigger()
{
  return next_trigger;
}

/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* Interrupt Handler */
void TPM0_IRQHandler(void)
{
	
	ENERGEST_ON(ENERGEST_TYPE_IRQ);
	printf("RT\r\n");
	next_trigger = 0;
	
	NVIC_ClearPendingIRQ(TPM0_IRQn);			/* Clear the pending bit in NVIC. */
	TPM0_C0SC = ~(TPM_CnSC_CHF_MASK 			/* Acknowledge interrupt & disable channel. */
				  & TPM_CnSC_CHIE_MASK) 
				| TPM_CnSC_MSA_MASK;
	
	NVIC_DisableIRQ(TPM0_IRQn);					/* Disable TPM0 interrupt in NVIC. */
	
	TPM0_C0V = 0x00U;							/* Clear the channel value. */	
	
	rtimer_run_next();
	
	ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
