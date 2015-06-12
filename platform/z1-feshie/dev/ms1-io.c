#include "contiki.h"
#include "contiki-conf.h"
#include "dev/reset-sensor.h"



/* Platform Headers */
#include "platform-conf.h"

#include "ms1-io.h"
//#define MS1_IO_DEBUG
#ifdef MS1_IO_DEBUG
  #include <stdio.h>
  #define PRINTF(...) printf(__VA_ARGS__)
#else
  #define PRINTF(...)
#endif

void ms1_io_init(void){
  PRINTF("MS1 IO Init\n");
  // Set up sense control pin
  SENSE_EN_PORT(SEL) &= ~BV(SENSE_EN_PIN);
  SENSE_EN_PORT(DIR) |= BV(SENSE_EN_PIN);
  SENSE_EN_PORT(REN) &= ~BV(SENSE_EN_PIN);
  SENSE_EN_PORT(OUT) &= ~BV(SENSE_EN_PIN);

  //Make sure all analogue input pins are inputs.
  P6DIR = 0x00;
  P6SEL = 0x00;
  
  /* Ensure that rain bucket is input. */
  P2DIR &= ~BV(0);

  // Set up radio control pin
  RADIO_EN_PORT(SEL) &= ~BV(RADIO_EN_PIN);
  RADIO_EN_PORT(DIR) |= BV(RADIO_EN_PIN);
  RADIO_EN_PORT(REN) &= ~BV(RADIO_EN_PIN);
  PRINTF("\tTurning on radio\n");
  //Turn on by default
  ms1_radio_on();
  ms1_sense_off(); //turn off sensors by default
}

void ms1_radio_on(void){
  PRINTF("Turning on radio\n");
  RADIO_EN_PORT(OUT) |= BV(RADIO_EN_PIN);
}

void ms1_radio_off(void){
  PRINTF("Turning off radio\n");
  RADIO_EN_PORT(OUT) &= ~BV(RADIO_EN_PIN);
}

void ms1_sense_on(void){
  PRINTF("Turning on sense\n");
  SENSE_EN_PORT(OUT) |= BV(SENSE_EN_PIN);
}

void ms1_sense_off(void){
  PRINTF("Turning off sense\n");
  SENSE_EN_PORT(OUT) &= ~BV(SENSE_EN_PIN);
}
