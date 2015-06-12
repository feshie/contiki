/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 *         Utility to store a node id in the external flash
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki-conf.h"
#include "dev/xmem.h"
#include <string.h>
#include "lib/sensors.h"
#include "contiki.h"

const struct sensors_sensor reset_sensor; 

/*---------------------------------------------------------------------------*/
static int reset_counter_get(int type)
{
  unsigned char buf[12];					//create buffer
  xmem_pread(buf, 12, RESET_COUNTER_XMEM_OFFSET);		//read buffer from flash
  if(buf[0] == 0x21 &&						//check id byte
     buf[1] == 0x42) {						//and again
    return (buf[2] << 8) | buf[3];				//return buffer
  } else {
    return 0;							//return no counter
  }
}
/*---------------------------------------------------------------------------*/
static int reset_counter_update(int type, int c)
{
  unsigned char buf[12];					//create a buffer to write to
  int reset_counter = reset_counter_get(0);		//get current reset counter
  reset_counter++;						//increment counter
  buf[0] = 0x21;						//id byte 1
  buf[1] = 0x42;						//id byte 2
  buf[2] = reset_counter >> 8;					//upper byte of counter
  buf[3] = reset_counter & 0xff;				//lower byte of counter
  xmem_erase(XMEM_ERASE_UNIT_SIZE, RESET_COUNTER_XMEM_OFFSET);	//erase
  xmem_pwrite(buf, 12, RESET_COUNTER_XMEM_OFFSET);			//write
  return reset_counter;
}
/*---------------------------------------------------------------------------*/
void reset_counter_reset()
{
  unsigned char buf[12];					//create a buffer to write to
  unsigned int reset_counter = 0;				//get current reset counter
  buf[0] = 0x21;						//id byte 1
  buf[1] = 0x42;						//id byte 2
  buf[2] = reset_counter >> 8;					//upper byte of counter
  buf[3] = reset_counter & 0xff;				//lower byte of counter
  xmem_erase(XMEM_ERASE_UNIT_SIZE, RESET_COUNTER_XMEM_OFFSET);	//erase
  xmem_pwrite(buf, 12, RESET_COUNTER_XMEM_OFFSET);			//write
}
/*---------------------------------------------------------------------------*/
static int reset_counter_status(int type)
{
  return 1;
}

SENSORS_SENSOR(reset_sensor, "Resets", 
	reset_counter_get , reset_counter_update , reset_counter_status);


