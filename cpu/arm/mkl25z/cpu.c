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

#include <MKL25Z4.h>                   /* I/O map for MKL25Z128VLK4 */
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
	/* These two functions happen in startup code so not needed. */
	//SCB_VTOR = (uint32_t)(&__vect_table); 		/* Set the interrupt vector table position */
	//SIM_COPC = 0x00;							/* disable watchdog. */
	
	
	port_enable(0);								/* Enable clocks to Port A. */
	

	/* System clock initialisation */
#if defined(__21mhzIR)	/* 20.97MHz core and bus speed from internal 32kHz reference oscillator. */
	SIM_CLKDIV1 = (uint32_t)0x0000;      										/* Set System Clock Divider Register to Divide-by-1 for OUTDIV1 and OUTDIV4.  Page 210. */
	SIM_SOPT2 &= ~(SIM_SOPT2_PLLFLLSEL_MASK | SIM_SOPT2_TPMSRC_MASK); 			/* Set MCGFLLCLK without divide-by-two and clear TPMSRC. Page 195. */
	SIM_SOPT2 |= SIM_SOPT2_TPMSRC(0x01);										/* Set TPM Clock Source to MCGFLLCLK. Page 195. */
	
	/* Stay in FEI Mode */
	MCG_C1 |= (MCG_C1_IREFSTEN_MASK | MCG_C1_IRCLKEN_MASK | MCG_C1_IREFS_MASK);	/* Set MCG_C1 for Enabled Slow internal ref, enabled in STOP mode. IREFS=1,IRCLKEN=1,IREFSTEN=1. Page 371. */
	MCG_C2 = 0x00;    															/* Set MCG_C2: LOCRE0=0,RANGE0=0,HGO0=0,EREFS0=0,LP=0,IRCS=0. Page 373. */
	MCG_C4 &= ~0xE0;		     												/* Set MCG_C4: Set FLL clock speed. DMX32=0,DRST_DRS=0, Internal 32K with FLL multiplication of 640 gives 20.97152MHz. Page 374. */                      
	OSC0_CR = 0x00;     														/* Set OSC0_CR: ERCLKEN=0,EREFSTEN=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */                     
	MCG_C5 = 0x00;																/* Set MCG_C5: PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=0. Page 376. */                       
	MCG_C6 = 0x00;																/* Set MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=0. Page 377. */
	while((MCG_S & MCG_S_IREFST_MASK) == 0x00) {} 								/* Check that the source of the FLL reference clock is the internal reference clock. */
	while((MCG_S & 0x0C) != 0x00) {}											/* Wait until output of the FLL is selected */

		
#else	/*48 MHz with a 24MHz bus clock generated from an 8MHz crystal. */
	SIM_CLKDIV1 = (SIM_CLKDIV1_OUTDIV1(0x01) | SIM_CLKDIV1_OUTDIV4(0x01));		/* Set OUTDIV1 and OUTDIV4 to "divide-by-2". */
	SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK; 										/* Select PLL as a clock source for various peripherals */
	SIM_SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK; 										/* Clear TPMSRC. Page 195. */
	SIM_SOPT2 |= SIM_SOPT2_TPMSRC(0x01);    									/* Set TPM Clock Source to MCGFLLCLK. Page 195. */
	
	PORTA_PCR18 &= ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK);					/* Set Port A Pin 18 as EXTAL0. */
	PORTA_PCR19 &= ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK);					/* Set Port A Pin 19 as XTAL0. */
	
	/* Need to go from FEI to FBE to PBE to PEE mode. */
	/* See Page 384 for function diagram and descriptions. */
	
	/* Switch to FBE Mode */
	OSC0_CR = OSC_CR_ERCLKEN_MASK; 												/* Enable External Reference Clock. */
	MCG_C2 = (MCG_C2_RANGE0(0x02) | MCG_C2_EREFS0_MASK);						/* Set Freq range to VHF & enable 8MHz oscillator. */
	MCG_C1 = (MCG_C1_CLKS(0x02) | MCG_C1_FRDIV(0x03) | MCG_C1_IRCLKEN_MASK);   	/* Set CLK SRC to external reference, Set divide factor to 64 & enable internal ref. */ 
	MCG_C4 &= ~(MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS_MASK);
	MCG_C5 = MCG_C5_PRDIV0(0x01);  												/* Set PLL Ext Ref Divider to divide factor of 2. */
	MCG_C6 = 0x00;													
	
	while((MCG_S & MCG_S_IREFST_MASK) != 0x00) { 								/* Check that the source of the FLL reference clock is the external reference clock. */
	}
	while((MCG_S & 0x0C) != 0x08) {    											/* Wait until external reference clock is selected as MCG output */
	}
	
	/* Switch to PBE Mode */
	OSC0_CR = OSC_CR_ERCLKEN_MASK;  											/* Enable External Reference Clock. */
	MCG_C1 = (MCG_C1_CLKS(0x02) | MCG_C1_FRDIV(0x03) | MCG_C1_IRCLKEN_MASK); 	/* Select external Ref Clock, Divide Fact of 128, Int Ref enabled. */
	MCG_C2 = (MCG_C2_RANGE0(0x02) | MCG_C2_EREFS0_MASK);						/* Set Freq range to VHF & enable 8MHz oscillator. */
	MCG_C5 = MCG_C5_PRDIV0(0x01);  												/* Set PLL Ext Ref Divider to divide factor of 2. */
	MCG_C6 = MCG_C6_PLLS_MASK;													/* Select PLL. */
	
	while((MCG_S & 0x0CU) != 0x08U) {    										/* Wait until external reference clock is selected as MCG output */
	}
	while((MCG_S & MCG_S_LOCK0_MASK) == 0x00U) { 								/* Wait until locked */
	}
	
	/* Switch to PEE Mode */
	OSC0_CR = OSC_CR_ERCLKEN_MASK; 												/* Enable External Reference Clock. */
	
	MCG_C1 = (MCG_C1_FRDIV(0x03) | MCG_C1_IRCLKEN_MASK); 						/* Select PLL as Clock Source, Divide Fact of 128, Int Ref enabled. */
	MCG_C2 = (MCG_C2_RANGE0(0x02) | MCG_C2_EREFS0_MASK);						/* Set Freq range to VHF & enable 8MHz oscillator. */
	MCG_C5 = MCG_C5_PRDIV0(0x01);  												/* Set PLL Ext Ref Divider to divide factor of 2. */
	MCG_C6 = MCG_C6_PLLS_MASK;													/* Select PLL. */
	while((MCG_S & 0x0CU) != 0x0CU) {   										/* Wait until output of the PLL is selected */
	}
#endif

	/* Setup the internal oscillator for MCGIRCLK. */
#if defined(OSCK32KSEL_RTCIN)	/* 32kHz internal "slow" oscillator with a link to RTC IN, or a 32K square wave source. */
	port_enable(PORTC_EN_MASK);									/* Enable Port C clock. */			
	SIM_SOPT1 &= ~SIM_SOPT1_OSC32KSEL_MASK;						/* Clear RTC Clock Source. */
	SIM_SOPT1 |= 0x00080000;									/* Set ERCLK32K source to RTC_CLKIN. Page 193. */
	SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_CLKOUTSEL_MASK) | 0x80;	/* Set CLKOUTSEL to MCGIRCLK (32k internal oscillator). */
	
	PORTC_PCR1 = ((PORTC_PCR1 & ~0x01000600) | 0x0100);			/* Set PORTC_PCR1 as RTC_CLKIN, ISF=0,MUX=1. */
	PORTC_PCR3 = ((PORTC_PCR3 & ~0x01000200) | 0x0500);			/* Set PORTC_PCR3 as CLKOUT. ISF=0,MUX=5. */

#else	/* 1kHz internal low power oscillator. */
	SIM_SOPT1 &= ~SIM_SOPT1_OSC32KSEL_MASK;						/* Set OSC32KSEL to LPO 1KHz so that LPTMR will work.  RTC will NOT work properly. */
	SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_CLKOUTSEL_MASK) | 0x70;	/* Set CLKOUTSEL to LPO Clock. */
#endif 
	
	/* Initialization of the RCM module */
	RCM_RPFW &= ~0x1F;																	/* Set RCM_RPFW: Reset Pin Filter Width register. RSTFLTSEL=0. Page 269. */                         
	RCM_RPFC &= ~0x07;   																/* Set RCM_RPFC: Reset Pin Filter Control register. RSTFLTSS=0,RSTFLTSRW=0. Page 268. */
	
	/* Initialization of the PMC module */
	PMC_LVDSC1 = (PMC_LVDSC1_LVDACK_MASK | PMC_LVDSC1_LVDRE_MASK);								/* Set PMC_LVDSC1: Clear LVD flag & enable LVD auto reset. LVDACK=1,LVDIE=0,LVDRE=1,LVDV=0. Page 240. */
	PMC_LVDSC2 = (PMC_LVDSC2_LVWACK_MASK);														/* Set PMC_LVDSC2: Clear LVD warning flag. LVWACK=1,LVWIE=0,LVWV=0. Page 241. */
	PMC_REGSC = (uint8_t)~(PMC_REGSC_BGEN_MASK | PMC_REGSC_ACKISO_MASK | PMC_REGSC_BGBE_MASK);	/* Set PMC_REGSC: BGEN=0,ACKISO=0,BGBE=0. Page 242. */                          
	SMC_PMPROT = (uint8_t)(SMC_PMPROT_AVLLS_MASK | SMC_PMPROT_ALLS_MASK | SMC_PMPROT_AVLP_MASK);/* Set SMC_PMPROT: Setup Power mode protection register to allow low power modes. AVLP=1,ALLS=1,AVLLS=1.  Page 219. */
	
	/* CPU Pin Allocations */
	PORTA_PCR20 = (uint32_t)((PORTA_PCR20 & ~0x01000000) | 0x0700);								/* Set PORTA_PCR20 as Reset, ISF=0,MUX=7 */
	PORTA_PCR4 = (uint32_t)((PORTA_PCR4 & ~0x01000000) | 0x0700);								/* Set PORTA_PCR4 as NMI, ISF=0,MUX=7. */
	
	NVIC_IPR1 &= (uint32_t)~0x00FF0000;          												/* Set NVIC_IPR1: Irq 4 to 7 Priority Register.	PRI_6=0 */
            
	/*lint -save  -e950 Disable MISRA rule (1.1) checking. */\
		asm("CPSIE i");\
	/*lint -restore Enable MISRA rule (1.1) checking. */\
}


void cpu_run(void)							/* Place core into RUN mode. */
{
	SCB_SCR &= (uint32_t)~(SCB_SCR_SLEEPDEEP_MASK | SCB_SCR_SLEEPONEXIT_MASK);		/* Clear Sleep Deep mask and Sleep on Exit mask. M0 book Page 457. */
}

void cpu_wait(void)							/* Place Core into WAIT mode (or VLPW if in VLPR). */
{
	SCB_SCR &= (uint32_t)~(SCB_SCR_SLEEPDEEP_MASK); 								/* Clear Sleep Deep mask so that WFI puts CPU into wait. M0 book Page 457. */
	SCB_SCR |= (uint32_t)SCB_SCR_SLEEPONEXIT_MASK; 									/* Set Sleep On Exit mask so that CPU returns to wait after interrupt. M0 book Page 457. */
	asm("DSB");																		/* DSB instruction to ensure effect of of previous writes.  See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHBGGHF.html */
	asm("WFI");																		/* Enter sleep/wait. Page 141 to 143. */
}

void cpu_stop(Type_StopMode StopMode)		/* Place core into one of the STOP modes. Similar to ARM Deep Sleep. */
{
	/* Clear Low Leakage Wakeup Unit flags */
	LLWU_F1 = (uint8_t)0xFF;														/* CLear LLWU_F1: Write 1 to all bits. Page 255. */
	LLWU_F2 = (uint8_t)0xFF;														/* CLear LLWU_F2: Write 1 to all bits. Page 257. */
	LLWU_F3 = (uint8_t)0xFF;														/* CLear LLWU_F3: Write 1 to all bits. Page 258. */
	LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK;											/* Clear LLWU_FILT1: Write 1 to FILTF bit. Page 260. */
	LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK;											/* Clear LLWU_FILT2: Write 1 to FILTF bit. Page 261. */
		
	SCB_SCR |= (uint32_t)(SCB_SCR_SLEEPDEEP_MASK); 									/* Set Sleep Deep mask so that WFI puts CPU into sleep. M0 book Page 457. */
	
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
	
	SCB_SCR &= (uint32_t)~(SCB_SCR_SLEEPONEXIT_MASK);								/* Clear Sleep On Exit mask so that CPU does not return to sleep after interrupt. M0 book Page 457. */
	//SCB_SCR |= (uint32_t)SCB_SCR_SLEEPONEXIT_MASK; 									/* Set Sleep On Exit mask so that CPU returns to sleep after interrupt.	M0 book Page 457. */
	(void)(SMC_PMCTRL == 0);        												/* Dummy read of SMC_PMCTRL to ensure the register is written */
	asm("DSB");																		/* DSB instruction to ensure effect of of previous writes.  See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHBGGHF.html */
	asm("WFI");																		/* Enter sleep/wait. Page 141 to 143. */
}


void NMI_Handler(void)		/* NMI Interrupt Handler.  Required as NMI fires during init and default causes a break.		*/
{
	;
}
