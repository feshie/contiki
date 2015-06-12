/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark of Advancare,SL
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
*/ 

/**
 * \file
 *         Testing the Potentiometer in Zolertia Z1 Starter Platform.
 * \author
 *         Enric M. Calvo <ecalvo@zolertia.com>
 */

#include "contiki.h"
#include <stdio.h>		
#include "sys/stimer.h"
static struct stimer *t0;
//#define delay RTIMER_SECOND
unsigned delay = 100;

/*---------------------------------------------------------------------------*/
PROCESS(test_stimer_second, "Testing stimer_second length");
AUTOSTART_PROCESSES(&test_stimer_second);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_stimer_second, ev, data)
{

  PROCESS_BEGIN();
  P4SEL &= ~0x01;
  P4DIR |= 0x01;
  P4REN |= 0x01;
  while(1) {
    stimer_set(t0, 1);
    P4OUT = P4OUT ^ 0x01; //toggle output
    printf("Start: %lu Interval: %lu\n", t0->start, t0->interval);
    printf("Delay = %u\n", delay);
    printf("Clock seconds: %lu\n", clock_seconds());
    printf("toggled: %i\n", (int)P4OUT);
    while(!stimer_expired(t0));
  }


  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

