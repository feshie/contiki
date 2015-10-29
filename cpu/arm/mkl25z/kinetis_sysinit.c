/*
 *    kinetis_sysinit.c - Default init routines for Flycatcher
 *                     		Kinetis ARM systems
 *    Copyright © 2012 Freescale semiConductor Inc. All Rights Reserved.
 *    
 *    Modified by Graeme Bragg, g.bragg@ecs.soton.ac.uk, 23/1/2014:
 *    	Separate Default interrupt handler for each interrupt to make debugging easier.
 */
 
#include "kinetis_sysinit.h"
#include "derivative.h"
#include <stdlib.h>

/**
 **===========================================================================
 **  External declarations
 **===========================================================================
 */
#if __cplusplus
extern "C" {
#endif
extern uint32_t __vector_table[];
extern unsigned long _estack;
extern uint32_t __SP_INIT;
extern void __thumb_startup( void );
#if __cplusplus
}
#endif

/**
 **===========================================================================
 **  Default interrupt handlers
 **===========================================================================
 */
void Default_Handler()
{
	printf("Default Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_NMI()
{
	printf("NMI Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_HardFault()
{
	printf("HardFault Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_SVC()
{
	printf("SVC Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_PendSV()
{
	printf("PendSV Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_SysTick()
{
	printf("Systick Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_DMA0()
{
	printf("DMA0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_DMA1()
{
	printf("DMA1 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_DMA2()
{
	printf("DMA2 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_DMA3()
{
	printf("DMA3 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_MCM()
{
	printf("MCM Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_FTFL()
{
	printf("FTFL Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_PMC()
{
	printf("PMC Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_LLW()
{
	printf("LLW Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_I2C0()
{
	printf("I2C0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_I2C1()
{
	printf("I2C1 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_SPI0()
{
	printf("SPI0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_SPI1()
{
	printf("SPI1 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_UART0()
{
	printf("UART0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_UART1()
{
	printf("UART1 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_UART2()
{
	printf("UART2 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_ADC0()
{
	printf("ADC0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_CMP0()
{
	printf("CMP0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_FTM0()
{
	printf("FTM0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_FTM1()
{
	printf("FTM1 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_FTM2()
{
	printf("FTM2 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_RTC_Alarm()
{
	printf("RTC Alarm Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_RTC_Seconds()
{
	printf("RTC Seconds Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_PIT()
{
	printf("PIT Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_USBOTG()
{
	printf("USBOTG Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_DAC0()
{
	printf("DAC0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_TSI0()
{
	printf("TSI0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_MCG()
{
	printf("MCG Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_LPTimer()
{
	printf("LPTMR Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_PORTA()
{
	printf("PORTA Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_PORTD()
{
	printf("PORTD Handler - BREAKPOINT");
	__asm("bkpt");
}

/**
 **===========================================================================
 **  Reset handler
 **===========================================================================
 */

void __init_hardware(void)
{
  /*** ### MKL25Z128VLK4 "Cpu" init code ... ***/
  /*** PE initialization code after reset ***/
	SCB_VTOR = (uint32_t)__vector_table; /* Set the interrupt vector table position */
	//SCB_VTOR = (uint32_t)(&__vect_table); /* Set the interrupt vector table position */
	// Disable the Watchdog because it may reset the core before entering main().
	SIM_COPC = KINETIS_WDOG_DISABLED_CTRL;
}

/* Weak definitions of handlers point to Default_Handler if not implemented */
void NMI_Handler() __attribute__ ((weak, alias("Default_Handler_NMI")));
void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler_HardFault")));
void SVC_Handler() __attribute__ ((weak, alias("Default_Handler_SVC")));
void PendSV_Handler() __attribute__ ((weak, alias("Default_Handler_PendSV")));
void SysTick_Handler() __attribute__ ((weak, alias("Default_Handler_SysTick")));

void DMA0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA0")));
void DMA1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA1")));
void DMA2_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA2")));
void DMA3_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA3")));
void MCM_IRQHandler() __attribute__ ((weak, alias("Default_Handler_MCM")));
void FTFL_IRQHandler() __attribute__ ((weak, alias("Default_Handler_FTFL")));
void PMC_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PMC")));
void LLW_IRQHandler() __attribute__ ((weak, alias("Default_Handler_LLW")));
void I2C0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_I2C0")));
void I2C1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_I2C1")));
void SPI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_SPI0")));
void SPI1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_SPI1")));
void UART0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_UART0")));
void UART1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_UART1")));
void UART2_IRQHandler() __attribute__ ((weak, alias("Default_Handler_UART2")));
void ADC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_ADC0")));
void CMP0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_CMP0")));
void FTM0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_FTM0")));
void FTM1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_FTM1")));
void FTM2_IRQHandler() __attribute__ ((weak, alias("Default_Handler_FTM2")));
void RTC_Alarm_IRQHandler() __attribute__ ((weak, alias("Default_Handler_RTC_Alarm")));
void RTC_Seconds_IRQHandler() __attribute__ ((weak, alias("Default_Handler_RTC_Seconds")));
void PIT_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PIT")));
void USBOTG_IRQHandler() __attribute__ ((weak, alias("Default_Handler_USBOTG")));
void DAC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DAC0")));
void TSI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_TSI0")));
void MCG_IRQHandler() __attribute__ ((weak, alias("Default_Handler_MCG")));
void LPTimer_IRQHandler() __attribute__ ((weak, alias("Default_Handler_LPTimer")));
void PORTA_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PORTA")));
void PORTD_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PORTD")));

/* The Interrupt Vector Table */
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    /* Processor exceptions */
    (void(*)(void)) &_estack,
    __thumb_startup,
    NMI_Handler,
    HardFault_Handler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,

    /* Interrupts */
    DMA0_IRQHandler, /* DMA Channel 0 Transfer Complete and Error */
    DMA1_IRQHandler, /* DMA Channel 1 Transfer Complete and Error */
    DMA2_IRQHandler, /* DMA Channel 2 Transfer Complete and Error */
    DMA3_IRQHandler, /* DMA Channel 3 Transfer Complete and Error */
    MCM_IRQHandler, /* Normal Interrupt */
    FTFL_IRQHandler, /* FTFL Interrupt */
    PMC_IRQHandler, /* PMC Interrupt */
    LLW_IRQHandler, /* Low Leakage Wake-up */
    I2C0_IRQHandler, /* I2C0 interrupt */
    I2C1_IRQHandler, /* I2C1 interrupt */
    SPI0_IRQHandler, /* SPI0 Interrupt */
    SPI1_IRQHandler, /* SPI1 Interrupt */
    UART0_IRQHandler, /* UART0 Status and Error interrupt */
    UART1_IRQHandler, /* UART1 Status and Error interrupt */
    UART2_IRQHandler, /* UART2 Status and Error interrupt */
    ADC0_IRQHandler, /* ADC0 interrupt */
    CMP0_IRQHandler, /* CMP0 interrupt */
    FTM0_IRQHandler, /* FTM0 fault, overflow and channels interrupt */
    FTM1_IRQHandler, /* FTM1 fault, overflow and channels interrupt */
    FTM2_IRQHandler, /* FTM2 fault, overflow and channels interrupt */
    RTC_Alarm_IRQHandler, /* RTC Alarm interrupt */
    RTC_Seconds_IRQHandler, /* RTC Seconds interrupt */
    PIT_IRQHandler, /* PIT timer all channels interrupt */
    Default_Handler, /* Reserved interrupt 39/23 */
    USBOTG_IRQHandler, /* USB interrupt */
    DAC0_IRQHandler, /* DAC0 interrupt */
    TSI0_IRQHandler, /* TSI0 Interrupt */
    MCG_IRQHandler, /* MCG Interrupt */
    LPTimer_IRQHandler, /* LPTimer interrupt */
    Default_Handler, /* Reserved interrupt 45/29 */
    PORTA_IRQHandler, /* Port A interrupt */
    PORTD_IRQHandler /* Port D interrupt */
};

