/*
 * Based on Z1-Websense, which has the following licence:
 *
 * Copyright (c) 2011, Zolertia(TM) is a trademark by Advancare,SL
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
 */

/**
 * \file
 *         adapted from Battery and Temperature IPv6 Demo for Zolertia Z1
 * \author
 *          Dan Playle      <djap1g12@soton.ac.uk>
 *          Philip Basford  <pjb@ecs.soton.ac.uk>
 *          Graeme Bragg    <gmb1g08@ecs.soton.ac.uk>
 *          Tyler Ward      <tw16g08@ecs.soton.ac.uk>
 *          Kirk Martinez   <km@ecs.soton.ac.uk>
 */

 // General
#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>

#include "z1-sampler.h"

#ifndef CC11xx_CC1120
  #include "dev/cc2420.h"
#endif

//Other parts of the application
#include "sampler.h"
#include "poster.h"
#include "ms_webserver.h"

#include "platform-conf.h"


#define DEBUG 

#ifdef DEBUG 
    #define DPRINT(...) printf(__VA_ARGS__)
#else
    #define DPRINT(...)
#endif

PROCESS(feshie_sense_process, "Feshie Sense");

AUTOSTART_PROCESSES(&feshie_sense_process);

PROCESS_THREAD(feshie_sense_process, ev, data)
{
    PROCESS_BEGIN();
    #ifndef CC11xx_CC1120
        cc2420_set_txpower(31);
    #endif
    #ifdef SPI_LOCKING
        printf(">>>>>SPI Locking enabled<<<<<\n");
    #endif
    #ifdef Z1_SAMPLER_AVR_DISABLE
        printf("####AVR Disabled####\n");
    #endif
    process_start(&web_process, NULL);
    process_start(&sample_process, NULL);


  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
