/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
#include "dev/serial-timeout.h"
#include "dev/protobuf-handler.h"
#include <string.h> /* for memcpy() */
#include <stdio.h>
#include "lib/ringbuf.h"
#include "contiki.h"
#include "contiki-conf.h"

//#define SERIAL_TIMEOUT_DEBUG

#ifdef SERIAL_TIMEOUT_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif


#ifdef SERIAL_TIMEOUT_CONF_BUFSIZE
#define SER_BUFSIZE SERIAL_TIMEOUT_CONF_BUFSIZE
#else /* SERIAL_LINE_CONF_BUFSIZE */
#define SER_BUFSIZE 128
#endif /* SERIAL_LINE_CONF_BUFSIZE */

#if (SER_BUFSIZE & (SER_BUFSIZE - 1)) != 0
#error SERIAL_TIMEOUT_CONF_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change SERIAL_TIMEOUT_CONF_BUFSIZE in contiki-conf.h.
#endif


static uint8_t rxbuf_data[SER_BUFSIZE];
static rtimer_clock_t ser_timer;
static uint8_t rxbytes;

PROCESS(serial_timeout_process, "Serial timeout driver");

extern process_event_t serial_timeout_event_message;

/*---------------------------------------------------------------------------*/
int
serial_timeout_input_byte(unsigned char c)
{
  ser_timer = RTIMER_NOW(); /*Reset the timeout timer */
//  PRINTF("Byte recieved...");
  if(rxbytes ==0){
    /*This was the first in a potential batch */
//    PRINTF("first\n");
    rxbuf_data[rxbytes++] = c; /* Store byte in buffer */
  }else{ /*We're still in the timeout period from another byte */
//    PRINTF("not first\n");
    if(rxbytes >= SER_BUFSIZE){
      /* Overflow */
//      PRINTF("OVERFLOW\n");
      //TODO handle this
      return 0;
    }else{
//      PRINTF("Storing rxbytes in %i\n", rxbytes);
      rxbuf_data[rxbytes++] = c;

    }
  }

  /* Wake up consumer process */
  if(rxbytes > 0){
      process_poll(&serial_timeout_process);
      return 1;
  }
  return 2;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_timeout_process, ev, data)
{
  PROCESS_BEGIN();
  printf("Serial timeout process started\n");
  static uint8_t buf[SER_BUFSIZE];
  static uint8_t bytes;
  while (1){
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
      if(rxbytes == 0){
        /* Stops spurios polls that seem to come in for no known reason */
        PRINTF("Rx bytes = 0\n");
        continue;
      }

    while (RTIMER_CLOCK_LT(RTIMER_NOW(), (ser_timer + SERIAL_TIMEOUT_VALUE)));
    if (rxbytes > SER_BUFSIZE){
      PRINTF("Serial recieve overflow");
    }else{
#ifdef SERIAL_TIMEOUT_DEBUG
      printf("Timeout reached\n");
      printf("Recieved Bytes: %i\n", rxbytes);
      int i = 0;
      while (i < SER_BUFSIZE){
        printf("%i:", (int)rxbuf_data[i++]);
      }
      printf("\n");
#endif
      memcpy( buf, rxbuf_data, rxbytes); 
      bytes = rxbytes;
      rxbytes = 0;
      memset(rxbuf_data, 0, SER_BUFSIZE); /*Reset buffer*/ 
      if (bytes != 0){
        PRINTF("CALLING PROTOBUG\n");
        protobuf_process_message(buf, bytes);    
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
serial_timeout_init(void)
{
  rxbytes = 0; /*Intially no bytes recieved */
  memset(rxbuf_data, 0, SER_BUFSIZE); /*Set buffer to 0 */ 
  process_start(&serial_timeout_process, NULL);
}
/*---------------------------------------------------------------------------*/
