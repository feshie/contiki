#include "dev/watchdog.h"

void watchdog_init(void) {

}

void watchdog_start(void) {
	/* We setup the watchdog to reset the device after 2.048 seconds,
	   unless watchdog_periodic() is called. */
	//halInternalEnableWatchDog();
}

void watchdog_periodic(void) {
	/* This function is called periodically to restart the watchdog
	   timer. */
	//halResetWatchdog();
}

void watchdog_stop(void) {
	//halInternalDisableWatchDog(MICRO_DISABLE_WATCH_DOG_KEY);
}

void watchdog_reboot(void) {
	//halReboot();
}

