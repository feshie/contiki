/*
  FILE    : kinetis_sysinit.h
  PURPOSE : system initialization header for Kinetis ARM architecture
  LANGUAGE: C
  Copyright © 2012 Freescale semiConductor Inc. All Rights Reserved.
  
  Modified by Graeme Bragg, g.bragg@ecs.soton.ac.uk, 23/1/2014:
  Separate Default interrupt handler for each interrupt to make debugging easier.
*/
#ifndef KINETIS_SYSINIT_H
#define KINETIS_SYSINIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Word to be written in SIM_COP in order to disable the Watchdog */
#define KINETIS_WDOG_DISABLED_CTRL	0x0

/* 
	Initializes the Kinetis hardware: e.g. disables the Watchdog
*/
void __init_hardware();

/*
** ===================================================================
**     Method      :  Default_Handler
**
**     Description :
**         The default interrupt handler.
** ===================================================================
*/
void Default_Handler();
void Default_Handler_NMI();
void Default_Handler_HardFault();
void Default_Handler_SVC();
void Default_Handler_PendSV();
void Default_Handler_SysTick();
void Default_Handler_DMA0();
void Default_Handler_DMA1();
void Default_Handler_DMA2();
void Default_Handler_DMA3();
void Default_Handler_MCM();
void Default_Handler_FTFL();
void Default_Handler_PMC();
void Default_Handler_LLW();
void Default_Handler_I2C0();
void Default_Handler_I2C1();
void Default_Handler_SPI0();
void Default_Handler_SPI1();
void Default_Handler_UART0();
void Default_Handler_UART1();
void Default_Handler_UART2();
void Default_Handler_ADC0();
void Default_Handler_CMP0();
void Default_Handler_FTM0();
void Default_Handler_FTM1();
void Default_Handler_FTM2();
void Default_Handler_RTC_Alarm();
void Default_Handler_RTC_Seconds();
void Default_Handler_PIT();
void Default_Handler_USBOTG();
void Default_Handler_DAC0();
void Default_Handler_TSI0();
void Default_Handler_MCG();
void Default_Handler_LPTimer();
void Default_Handler_PORTA();
void Default_Handler_PORTD();

#ifdef __cplusplus
}
#endif

#endif /* #ifndef KINETIS_SYSINIT_H */
