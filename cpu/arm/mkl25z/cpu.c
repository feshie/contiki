/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	cpu.c	
 *	
 *	Initialisation & Interrupt handler for CPU on the Freescale FRDM-KL25z Freedom Board
 *	
 *	Author: Graeme Bragg    
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	27/2/2013	Rev.01		Configures CPU for 20.97152MHz Bus, Core and MCG Clock sources from the internal 32K reference oscillator.
 *							CPU runs in FEI. MGCIRCLK source is set to the slow internal oscillator.
 *							VLPM, LLS and VLLS are "allowed". NMI Interrupt is enabled on PTA4, Reset is PTA20.
 *	5/3/2013	Rev.02		Functions to enter low power modes.						
 *	12/3/2013	Rev.03		Added definitions for clock values.	
 *	18/2/2014	Rev.04		Options for 21MHz (from 32k internal) or 48MHz (from 8MHz external) clocks.
 *	
 *
 *	Page references relate to the KL25 Sub-Family Reference Manual, Document No. KL25P80M48SF0RM, Rev. 3 September 2012
 *	Available on 25/02/2013 from http://cache.freescale.com/files/32bit/doc/ref_manual/KL25P80M48SF0RM.pdf?fr=gdc
 *	
 *	Page references to "M0 Book" refer to "The Definitive Guide to the ARM Cortex-M0" by Joseph Yiu, ISBN 978-0-12-385477-3.
 *	
 *
 *	***NB*** This file is intended for use with a new "Bareboard" project (with no rapid-development) in Code Warrior.		
 *				If this is not the case, you MUST ensure that an appropriate entry of "NMI_Handler" exists in entry 2 
 *				(address 0x00000008) of the VectorTable.
 *
 *				Appropriate entries for the interrupt handler exist in the vector table contained in the generated 
 *				kinetis_sysinit.c file
 */				
/*-----------------------------------------------------------------------------------------------------------------------------------*/

#include "derivative.h"                  /* I/O map for MKL25Z128VLK4 */
#include "cpu.h"

void port_enable(uint8_t PortMask)		/* Enable clock to used ports.  This is required before configuring the port. Page 206. */
{
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;			/* Enable clock to PortA as this is ALWAYS required for NMI and Reset pin. */
	
	if(PortMask & PORTB_EN_MASK)				/* If PortB Enable Mask, */
	{
		SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;		/* Enable Clock to PortB. */
	}
	if(PortMask & PORTC_EN_MASK)				/* If PortC Enable Mask, */
	{
		SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;		/* Enable Clock to PortC. */
	}
	if(PortMask & PORTD_EN_MASK)				/* If PortD Enable Mask, */
	{
		SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;		/* Enable Clock to PortD. */
	}
	if(PortMask & PORTE_EN_MASK)				/* If PortE Enable Mask, */
	{
		SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		/* Enable Clock to PortE. */
	}
}

void cpu_init(void)
{
	
	/* CPU Pin Allocations */
	PORTA_PCR20 = (uint32_t)((PORTA_PCR20 & ~0x01000000) | 0x0700);			/* Set PORTA_PCR20 as Reset, ISF=0,MUX=7 */
	PORTA_PCR4 = (uint32_t)((PORTA_PCR4 & ~0x01000000) | 0x0700);			/* Set PORTA_PCR4 as NMI, ISF=0,MUX=7. */
	
	NVIC->IP[1] &= (uint32_t)~0x00FF0000;          							/* Set NVIC_IPR1: Irq 4 to 7 Priority Register.	PRI_6=0 */
            
	/*lint -save  -e950 Disable MISRA rule (1.1) checking. */\
		asm("CPSIE i");\
	/*lint -restore Enable MISRA rule (1.1) checking. */\
}


void cpu_run(void)							/* Place core into RUN mode. */
{
	SCB->SCR &= (uint32_t)~(SCB_SCR_SLEEPDEEP_MASK | SCB_SCR_SLEEPONEXIT_MASK);		/* Clear Sleep Deep mask and Sleep on Exit mask. M0 book Page 457. */
}

void cpu_wait(void)							/* Place Core into WAIT mode (or VLPW if in VLPR). */
{
	SCB->SCR &= (uint32_t)~(SCB_SCR_SLEEPDEEP_MASK); 								/* Clear Sleep Deep mask so that WFI puts CPU into wait. M0 book Page 457. */
	SCB->SCR |= (uint32_t)SCB_SCR_SLEEPONEXIT_MASK; 									/* Set Sleep On Exit mask so that CPU returns to wait after interrupt. M0 book Page 457. */
	asm("DSB");																		/* DSB instruction to ensure effect of of previous writes.  See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHBGGHF.html */
	asm("WFI");																		/* Enter sleep/wait. Page 141 to 143. */
}

void cpu_stop(Type_StopMode StopMode)		/* Place core into one of the STOP modes. Similar to ARM Deep Sleep. */
{
	/* Clear Low Leakage Wakeup Unit flags */
	LLWU_F1 = (uint8_t)0xFF;														/* CLear LLWU_F1: Write 1 to all bits. Page 255. */
	LLWU_F2 = (uint8_t)0xFF;														/* CLear LLWU_F2: Write 1 to all bits. Page 257. */
	//LLWU_F3 = (uint8_t)0xFF;														/* CLear LLWU_F3: Write 1 to all bits. Page 258. */
	LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK;											/* Clear LLWU_FILT1: Write 1 to FILTF bit. Page 260. */
	LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK;											/* Clear LLWU_FILT2: Write 1 to FILTF bit. Page 261. */
		
	SCB->SCR |= (uint32_t)(SCB_SCR_SLEEPDEEP_MASK); 									/* Set Sleep Deep mask so that WFI puts CPU into sleep. M0 book Page 457. */
	
	switch(StopMode) {																/* Set the stop mode */
		case Mode_Stop:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x00);			/* Set SMC_PMCTRL: Normal Stop. STOPM=4. Page 221. */
							SMC_STOPCTRL = (uint8_t)((SMC_STOPCTRL & ~SMC_STOPCTRL_PSTOPO_MASK) | 0x00);	/* Set SMC_STOPCTRL: Normal Stop. PSTOPO = 0. Page 222. */
							break;
		
		case Mode_PStop1:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x00);			/* Set SMC_PMCTRL: Normal Stop. STOPM=4. Page 221. */
							SMC_STOPCTRL = (uint8_t)((SMC_STOPCTRL & ~SMC_STOPCTRL_PSTOPO_MASK) | 0x40);	/* Set SMC_STOPCTRL: Partial Stop 1. PSTOPO = 1. Page 222. */
							break;
			
		case Mode_PStop2:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x00);			/* Set SMC_PMCTRL: Normal Stop. STOPM=4. Page 221. */
							SMC_STOPCTRL = (uint8_t)((SMC_STOPCTRL & ~SMC_STOPCTRL_PSTOPO_MASK) | 0x80);	/* Set SMC_STOPCTRL: Partial Stop 2. PSTOPO = 2. Page 222. */
							break;
			
		case Mode_VLPS:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x02);			/* Set SMC_PMCTRL: Very-Low-Power Stop. STOPM=4. Page 221. */
							break;
						
		case Mode_LLS:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x03);			/* Set SMC_PMCTRL: Low-Leakage Stop. STOPM=4. Page 221. */
							break;
						
		case Mode_VLLS0:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x04);			/* Set SMC_PMCTRL: Very-Low-Leakage Stop. STOPM=4. Page 221. */
							SMC_STOPCTRL = (uint8_t)((SMC_STOPCTRL & ~SMC_STOPCTRL_VLLSM_MASK) | 0x40);		/* Set SMC_STOPCTRL: VLLS0. VLLS = 0. Page 222. */
							SMC_STOPCTRL = SMC_STOPCTRL_PORPO_MASK;											/* Set SMC_STOPCTRL: Disable POR in VLLS0. PSTOPO=0,PORPO=1,VLLSM=0. Page 222. */
							break;
						
		case Mode_VLLS1:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x04);			/* Set SMC_PMCTRL: Very-Low-Leakage Stop. STOPM=4. Page 221. */
							SMC_STOPCTRL = (uint8_t)((SMC_STOPCTRL & ~SMC_STOPCTRL_VLLSM_MASK) | 0x01);		/* Set SMC_STOPCTRL: VLLSp. VLLS = 1. Page 222. */
							break;
						
		case Mode_VLLS3:	
							SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x04);			/* Set SMC_PMCTRL: Very-Low-Leakage Stop. STOPM=4. Page 221. */
							SMC_STOPCTRL = (uint8_t)((SMC_STOPCTRL & ~SMC_STOPCTRL_VLLSM_MASK) | 0x03);		/* Set SMC_STOPCTRL: VLLS3. VLLS = 3. Page 222. */
							break;
		
		default:			SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & ~SMC_PMCTRL_STOPM_MASK) | 0x00);			/* Default to Normal STOP. */
	}
	
	SCB->SCR &= (uint32_t)~(SCB_SCR_SLEEPONEXIT_MASK);								/* Clear Sleep On Exit mask so that CPU does not return to sleep after interrupt. M0 book Page 457. */
	//SCB_SCR |= (uint32_t)SCB_SCR_SLEEPONEXIT_MASK; 									/* Set Sleep On Exit mask so that CPU returns to sleep after interrupt.	M0 book Page 457. */
	(void)(SMC_PMCTRL == 0);        												/* Dummy read of SMC_PMCTRL to ensure the register is written */
	asm("DSB");																		/* DSB instruction to ensure effect of of previous writes.  See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHBGGHF.html */
	asm("WFI");																		/* Enter sleep/wait. Page 141 to 143. */
}


/* Interrupt enable/disable functions. */
unsigned long __attribute__((naked))
cpu_cpsie(void)
{
  unsigned long ret;

  /* Read PRIMASK and enable interrupts */
  __asm("    mrs     r0, PRIMASK\n"
        "    cpsie   i\n"
        "    bx      lr\n"
        : "=r" (ret));

  /* The inline asm returns, we never reach here.
   * We add a return statement to keep the compiler happy */
  return ret;
}

unsigned long __attribute__((naked))
cpu_cpsid(void)
{
  unsigned long ret;

  /* Read PRIMASK and disable interrupts */
  __asm("    mrs     r0, PRIMASK\n"
        "    cpsid   i\n"
        "    bx      lr\n"
        : "=r" (ret));

  /* The inline asm returns, we never reach here.
   * We add a return statement to keep the compiler happy */
  return ret;
}

/*---------------------------------------------------------------------------*/
void NMI_Handler(void)		/* NMI Interrupt Handler.  Required as NMI fires during init and default causes a break.		*/
{
	printf("NMI Fire");;
}

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

void Default_Handler_TPM0()
{
	printf("TPM0 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_TPM1()
{
	printf("TPM1 Handler - BREAKPOINT");
	__asm("bkpt");
}

void Default_Handler_TPM2()
{
	printf("TPM2 Handler - BREAKPOINT");
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

/* Weak definitions of handlers point to Default_Handler if not implemented */
void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler_HardFault")));
void SVC_Handler() __attribute__ ((weak, alias("Default_Handler_SVC")));
void PendSV_Handler() __attribute__ ((weak, alias("Default_Handler_PendSV")));
void SysTick_Handler() __attribute__ ((weak, alias("Default_Handler_SysTick")));
void DMA0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA0")));
void DMA1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA1")));
void DMA2_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA2")));
void DMA3_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DMA3")));
void LLWU_IRQHandler() __attribute__ ((weak, alias("Default_Handler_LLW")));
void I2C0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_I2C0")));
void I2C1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_I2C1")));
void SPI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_SPI0")));
void SPI1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_SPI1")));
void UART0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_UART0")));
void UART1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_UART1")));
void UART2_IRQHandler() __attribute__ ((weak, alias("Default_Handler_UART2")));
void ADC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_ADC0")));
void CMP0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_CMP0")));
void TPM0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_TPM0")));
void TPM1_IRQHandler() __attribute__ ((weak, alias("Default_Handler_TPM1")));
void TPM2_IRQHandler() __attribute__ ((weak, alias("Default_Handler_TPM2")));
void RTC_IRQHandler() __attribute__ ((weak, alias("Default_Handler_RTC_Alarm")));
void RTC_Seconds_IRQHandler() __attribute__ ((weak, alias("Default_Handler_RTC_Seconds")));
void PIT_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PIT")));
void USB0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_USBOTG")));
void DAC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_DAC0")));
void TSI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_TSI0")));
void MCG_IRQHandler() __attribute__ ((weak, alias("Default_Handler_MCG")));
void LPTMR0_IRQHandler() __attribute__ ((weak, alias("Default_Handler_LPTimer")));
void PORTA_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PORTA")));
void PORTD_IRQHandler() __attribute__ ((weak, alias("Default_Handler_PORTD")));
