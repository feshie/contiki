#include <contiki.h>
#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include <sys/rtimer.h>
#include <sys/energest.h>
#include "derivative.h"

#include "cpu.h"
#include "nvic.h"
#include "debug-uart.h"

/* Sanity check reload value for SysTick counter */
#if DEFAULT_SYSTEM_CLOCK % CLOCK_SECOND
	/* Clock speed and CLOCK_SECOND gives a remainder, bad choices */
	#error SYSCLOCK/CLOCK_SECOND gives a remainder: Accuracy of Systick is reduced.
#endif

#if (CLOCK_SETUP == 0)
	/* MCGIRCLK = 32kHz, OSCERCLK = NA. */
	#define CLOCK_TPM0_PRESCALE 	0x00U
	#define CLOCK_LPTMR_SOURCE		0x00U
	#define CLOCK_LPTMR_PRESCALE	0x00U

	#warning CLOCK_SETUP = 0 reduces timing accuracy!
		
#elif (CLOCK_SETUP == 1)
	/* MCGIRCLK = 32kHz, OSCERCLK = 8MHz. */
	#define CLOCK_TPM0_PRESCALE 	0x00U
	#define CLOCK_LPTMR_SOURCE		0x03U
	#define CLOCK_LPTMR_PRESCALE	0x02U

#elif (CLOCK_SETUP == 2)
	/* MCGIRCLK = 4MHz. OSCERCLK = NA. */
	#define CLOCK_TPM0_PRESCALE 	0x07U
	#define CLOCK_LPTMR_SOURCE		0x00U
	#define CLOCK_LPTMR_PRESCALE	0x01U

	#warning CLOCK_SETUP = 2 reduces timing accuracy!

#elif (CLOCK_SETUP == 3)
	/* MCGIRCLK = 32kHz. OSCERCLK = 4MHz. */
	#define CLOCK_TPM0_PRESCALE 	0x00U
	#define CLOCK_LPTMR_SOURCE		0x03U
	#define CLOCK_LPTMR_PRESCALE	0x01U

#elif (CLOCK_SETUP == 4)
	/* MCGIRCLK = 32kHz, OSCERCLK = 8MHz. */
	#define CLOCK_TPM0_PRESCALE 	0x00U
	#define CLOCK_LPTMR_SOURCE		0x03U
	#define CLOCK_LPTMR_PRESCALE	0x02U

#else 
	#error Unsupported CLOCK_SETUP, cannot accurately set counter parameters.
#endif

#define RTIMER_CLOCK_TICK_RATIO (RTIMER_SECOND / CLOCK_SECOND)


static volatile uint64_t rt_ticks_startup, rt_ticks_epoch;

void
clock_init()
{
	/* Need to configure a timer to use for the counter value. */
	
	/* Make sure MCGIRCLK is enabled & set to run in STOP. */
	MCG_C1 |= MCG_C1_IRCLKEN_MASK | MCG_C1_IREFSTEN_MASK;
	
	/* Configure TPM0 Clocking. */
	SIM_SOPT2 &= ~(SIM_SOPT2_TPMSRC_MASK);		/* Clear TPM Clock source. */
	SIM_SOPT2 |= SIM_SOPT2_TPMSRC(0x03); 		/* Set the TPM clock to MCGIRCLK */
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;			/* Enable clock to TPM. */

	/* Configure TPM0. */
	TPM0_CNT = TPM_CNT_COUNT(0x00);      						/* Reset counter register */
	TPM0_MOD = TPM_MOD_MOD(0xFFFF);      						/* Set up modulo register use all 16-bits */
	TPM0_SC = TPM_SC_TOF_MASK | TPM_SC_PS(CLOCK_TPM0_PRESCALE); /* Clear TOF & set Prescale. */
	TPM0_CONF = 0x00U;											/* Clear CONF so that TPM works in WAIT */
	
	/* Configure Channel 0 for RTIMER interrupts. */
	TPM0_C0SC = TPM_CnSC_MSA_MASK; 				/* Config Channel 0 for software compare. */
	TPM0_C0V = TPM_CnV_VAL(0x00);      			/* Set up channel value register */
	
	/* Clear other channel status and control register */
	TPM0_C1SC = 0x00U;
	TPM0_C2SC = 0x00U;
	TPM0_C3SC = 0x00U;
	TPM0_C4SC = 0x00U;
	TPM0_C5SC = 0x00U;
	
	/* Configure TPM0 interrupt in NVIC. */
	NVIC_SetPriority(TPM0_IRQn, 0x80);			/* Set priority of TPM0 interrupt. */
	
	rt_ticks_startup = 0;
	rt_ticks_epoch = 0;
	
	TPM0_SC = TPM_SC_CMOD(0x01); 				/* Enable the counter. */
	
	
	/* We also need a timer for clock_delay_usec.  Use LPTMR for this:
	 * Configured to tick every microsecond, or as close as we can get. 
	 * TPM1 would be easier BUT all TPMs must have same clock source. */
	SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;						/* Enable clock to LPTMR. */
	LPTMR0_CSR = LPTMR_CSR_TCF_MASK;						/* Clear LPTMR0 TCF flag & disable LPTMR. */
	
#if (CLOCK_SETUP == 0)
	LPTMR0_PSR = LPTMR_PSR_PCS(CLOCK_LPTMR_SOURCE) 			/* Configure LPTMR prescale and clock source. */
				| LPTMR_PSR_PRESCALE(CLOCK_LPTMR_PRESCALE)
				| LPTMR_PSR_PBYP_MASK;						/* With prescaler bypass. */
#else	
	LPTMR0_PSR = LPTMR_PSR_PCS(CLOCK_LPTMR_SOURCE) 			/* Configure LPTMR prescale and clock source. */
				| LPTMR_PSR_PRESCALE(CLOCK_LPTMR_PRESCALE);	
#endif	

	
	/* Configure the Systick to fire CLOCK_SECOND number of times each second.
	 * This is configured with the CLOCK_CONF_SECOND define in contiki-conf.h
	 * and has a default value of 32 in Contiki but a platform default of 128.
	 */
	if(SysTick_Config(DEFAULT_SYSTEM_CLOCK/CLOCK_SECOND))
	{
		printf("ERROR: Invalid Systick Setting in clock.c\n\r");	
	}
}


/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return rt_ticks_startup / RTIMER_CLOCK_TICK_RATIO;
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  rt_ticks_epoch = (uint64_t)sec * RTIMER_SECOND;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return rt_ticks_epoch / RTIMER_SECOND;
}
/*---------------------------------------------------------------------------*/



void
clock_wait(clock_time_t i)
{
	clock_time_t start;

	start = clock_time();
	while(clock_time() - start < (clock_time_t)i);
}

/* Arch-specific clock delay. Uses XXX for accuracy. */
void
clock_delay_usec(uint16_t dt)
{
	LPTMR0_CSR = LPTMR_CSR_TCF_MASK;			/* Clear LPTMR0 TCF flag & disable LPTMR. */

#if (CLOCK_SETUP == 0)
	/* Set CMR to be as close as possible but we only have a 30.5uS resolution. */
	if (dt % 31 == 0) {
		LPTMR0_CMR = dt / 31;
	} else {
		LPTMR0_CMR = (dt / 31) + 1;	
	}								
#else	
	LPTMR0_CMR = dt;							/* Set Compare to number of ms. */
#endif
	
	LPTMR0_CSR = LPTMR_CSR_TEN_MASK;			/* Enable the counter. */
	
	while(!(LPTMR0_CSR & LPTMR_CSR_TCF_MASK));	/* Block until the compare flag is set. */
	LPTMR0_CSR = LPTMR_CSR_TCF_MASK;			/* Clear LPTMR0 TCF flag & disable LPTMR. */	
}

void
clock_delay(unsigned int i)
{
	clock_delay_usec(i);
}


/*---------------------------------------------------------------------------*/

void update_ticks(void)
{
	rtimer_clock_t now;
	uint64_t prev_rt_ticks_startup, cur_rt_ticks_startup_hi;
	
	now = RTIMER_NOW();										/* Get the current TPM0 count. */
	prev_rt_ticks_startup = rt_ticks_startup;				
	cur_rt_ticks_startup_hi = prev_rt_ticks_startup >> 16;		
	
	while(TPM0_SC & TPM_SC_TOF_MASK) {	/* Check to see if the counter has overflowed. 		*/
		TPM0_SC |= TPM_SC_TOF_MASK;		/* If TPM0_SC TOF bit is still set after a write, 	*/
		cur_rt_ticks_startup_hi++;		/* it means that there has been a second overflow. 	*/
	}
	
	rt_ticks_startup = cur_rt_ticks_startup_hi << 16 | now;
	
	rt_ticks_epoch += rt_ticks_startup - prev_rt_ticks_startup;
  	
	/*
	 * Inform the etimer library that the system clock has changed and that an
	 * etimer might have expired.
	 */
	if(etimer_pending()) {
		etimer_request_poll();
	}
}


/* Systick Interrupt Handler. */
void SysTick_Handler()
{
	ENERGEST_ON(ENERGEST_TYPE_IRQ);
	(void)SysTick->CTRL;					/* Dummy read CSR register to clear Count flag. SysTick->CTRL in CMSIS */
	SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;		/* Clear pending interrupt in SCB. */
	
	update_ticks();
	//printf("ST\r\n");

#if (DISABLE_WDOG == 0)
	if(CPU_Watchdog_Disabled()) {
		SIM_SRVCOP = 0x55;
		SIM_SRVCOP = 0xAA;
	}
#endif		
	
	ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
