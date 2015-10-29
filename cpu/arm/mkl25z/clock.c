#include <contiki.h>
#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include <MKL25Z4.h>

#include "cpu.h"
#include "nvic.h"
#include "debug-uart.h"

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

//void
//SysTick_handler(void) __attribute__ ((interrupt));

void
SysTick_Handler(void)
{
	(void)SYST_CSR;						/* Dummy read CSR register to clear Count flag. SysTick->CTRL in CMSIS */
	//SCB_ICSR = SCB_ICSR_PENDSTCLR_MASK;	/* Clear pending interrupt in SCB. */
	current_clock++;						/* Increment current clock. */
	if(etimer_pending() && etimer_next_expiration_time() >= current_clock) {
		etimer_request_poll();
		/* printf("%d,%d\n", clock_time(),etimer_next_expiration_time  	()); */
	}
	if (--second_countdown == 0) {
		current_seconds++;
		second_countdown = CLOCK_SECOND;
	}
}


void
clock_init()
{
	NVIC_SET_SYSTICK_PRI(8);				/* Set Systick priority. */
	SYST_RVR = CORECLK/16/CLOCK_SECOND;		/* Set reload value.  SysTick->LOAD in CMSIS. */
	SYST_CSR = 	SysTick_CSR_ENABLE_MASK | 	/* Enable SysTick. */
				SysTick_CSR_TICKINT_MASK; 		/* Enable tick interrupt. */	
	/* Leave clock source as "external" clock.  This is core clock / 16, so 48/16 = 3MHz. */
	/* In CMSIS, this would be: SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT; */
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

void
clock_delay(unsigned int t)
{
	volatile uint8_t i;
	for(i=0; i<(t*1.25); i++) {
		/* NOTE: this is not really accurate, as not sure yet about the cycle counts. Assuming 0.75 cycle for a NOP */
		asm("nop \n\t");
	} /* just something to wait, NOT the requested cycles */
}

unsigned long
clock_seconds(void)
{
  return current_seconds;
}
