/**
 * \file
 *         Header file for the MKL25Z-specific rtimer code
 * \author
 *         Graeme Bragg <g.bragg@ecs.soton.ac.uk>
 */

#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#define RTIMER_ARCH_SECOND 125

void rtimer_arch_set(rtimer_clock_t t);

rtimer_clock_t rtimer_arch_now(void);

#endif /* RTIMER_ARCH_H_ */
