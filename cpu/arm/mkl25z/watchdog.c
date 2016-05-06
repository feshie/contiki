#include "dev/watchdog.h"
#include "derivative.h"
#include "contiki-conf.h"
#include "cpu.h"
#include <stdio.h>

static volatile uint8_t watchdog_status;

void watchdog_init(void) {
	/* Once configured, it cannot be reconfigured or stopped. */
	
#if (DISABLE_WDOG == 0)
	/* Lock in Watchdog settings. */
	/* Configure Watchdog to timout after 1.024s. */
	SIM_COPC = SIM_COPC_COPT(0x03);	
#endif
	
}

void watchdog_start(void) {
	/* The watchdog starts with the core. */
#if (DISABLE_WDOG == 0)	
	CPU_Watchdog_Enable();
#endif	
}

void watchdog_periodic(void) {
	/* This function is called periodically to restart the watchdog
	   timer. */
#if (DISABLE_WDOG == 0)	
	SIM_SRVCOP = 0x55;
	SIM_SRVCOP = 0xAA;
#endif	
}

void watchdog_stop(void) {
	/* We cannot actually stop the watchdog once running. */
#if (DISABLE_WDOG == 0)	
	CPU_Watchdog_Disable();
#endif		
}

void watchdog_reboot(void) {
	/* Trigger a reboot by writing junk to the COP register. */
	//printf("\n\rCode-Induced Watchdog.\n\r");
	//SIM_SRVCOP = 0xFF;
	uint16_t endian = SCB->AIRCR & 0x8000;
	
	SCB->AIRCR = 0x05FA0004 | endian;
}


#if (DISABLE_WDOG == 0)	
uint8_t CPU_Watchdog_Disabled(void) {
	return watchdog_status;
}

void CPU_Watchdog_Disable(void) {
	watchdog_status = 1;
}
void CPU_Watchdog_Enable(void) {
	watchdog_status = 0;
}
#endif	
