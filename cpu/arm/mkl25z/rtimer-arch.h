/**
 * \file
 *         Header file for the MKL25Z-specific rtimer code
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 */

#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include <sys/rtimer.h>
#include "derivative.h"

#include "contiki.h"

#if (CLOCK_SETUP == 2)
	/* If CLOCK_SETUP is 2, the fast interal reference clock is used. Need to account for this. */
	#define RTIMER_ARCH_SECOND 31250
#else 
	#define RTIMER_ARCH_SECOND 32768
#endif

rtimer_clock_t rtimer_arch_now(void);

rtimer_clock_t rtimer_arch_next_trigger(void);

#endif /* RTIMER_ARCH_H_ */
