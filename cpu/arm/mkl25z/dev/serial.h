/*-----------------------------------------------------------------------------------------------------------------------------------*/
/*	serial.c	
 *	
 *	Initialisation & Commands for Serial communication on the Freescale FRDM-KL25z Freedom Board
 *	
 *	Author: Graeme Bragg
 * 			ARM-ECS / Pervasive Systems Centre
 * 			School of Electronics & Computer Science
 * 			University of Southampton
 * 
 * 
 *	25/2/2013	Rev.01		Includes functions for UART0 ONLY.  Configured to function on PTA1 and PTA2.
 *								Configured for 38400 8 N 1.  Configured to run from MCGFLLCLK @ 20.97152MHz 
 *								Uses Interrupt for RX, does NOT use interrupt for TX.
 *	7/3/2013	Rev.02		Modified UART0 IRQ to use a Callback if one is registered.				
 *	12/3/2013	Rev.03		Enabled dynamic baud rate setting.
 *	5/2/2014	Rev.04		Added code for UART1 and UART2. Still static pin mappings.
 *
 *
 *	Page references relate to the KL25 Sub-Family Reference Manual, Document No. KL25P80M48SF0RM, Rev. 3 September 2012
 *	Available on 25/02/2013 from http://cache.freescale.com/files/32bit/doc/ref_manual/KL25P80M48SF0RM.pdf?fr=gdc
 *	
 *	Page references to "M0 Book" refer to "The Definitive Guide to the ARM Cortex-M0" by Joseph Yiu, ISBN 978-0-12-385477-3.
 *	
 *
 *	***NB*** This file is intended for use with a new "Bareboard" project (with no rapid-development) in Code Warrior.		
 *				If this is not the case, you MUST ensure that an appropriate entry of "UART0_IRQHandler" exists in entry 28 
 *				(address 0x00000070) of the VectorTable.
 *
 *				Appropriate entries for the interrupt handler exist in the vector table contained in the generated 
 *				kinetis_sysinit.c file
 */				
/*-----------------------------------------------------------------------------------------------------------------------------------*/

#ifndef __SERIAL_H
#define __SERIAL_H

void serial_init(uint32_t UART0_baudrate, uint32_t UART1_baudrate, uint32_t UART2_baudrate);		/* Initialise all Serial port(s).	*/
void UART0_init(uint32_t baudrate);					/* Initialise UART0 ONLY. */
void UART1_init(uint32_t baudrate);					/* Initialise UART1 ONLY. */
void UART2_init(uint32_t baudrate);					/* Initialise UART2 ONLY. */

void UART0_reg_callback(void* UART0_Callback_Ptr);	/* Register a callback function for use in UART0_IRQHandler.	*/
void UART1_reg_callback(void* UART1_Callback_Ptr);	/* Register a callback function for use in UART1_IRQHandler.	*/
void UART2_reg_callback(void* UART2_Callback_Ptr);	/* Register a callback function for use in UART2_IRQHandler.	*/


void UART_PutChar(uint8_t uart, char outchar);		/* Put single character into UART output register.				*/
void UART0_PutChar(char outChar);					/* Put single character into UART0 output register.				*/
void UART1_PutChar(char outChar);					/* Put single character into UART1 output register.				*/
void UART2_PutChar(char outChar);					/* Put single character into UART2 output register.				*/


void UART_SendString(uint8_t uart, const unsigned char *str);
void UART0_SendString(const unsigned char *str);	/* Print a string to UART0.										*/
void UART1_SendString(const unsigned char *str);	/* Print a string to UART1.										*/
void UART2_SendString(const unsigned char *str);	/* Print a string to UART2.										*/


void UART0_IRQ_en(void);								/* Enable UART0 RX interrupt. */
void UART1_IRQ_en(void);								/* Enable UART1 RX interrupt. */
void UART2_IRQ_en(void);								/* Enable UART2 RX interrupt. */

void UART0_IRQ_dis(void);								/* Disable UART0 RX interrupt. */
void UART1_IRQ_dis(void);								/* Disable UART1 RX interrupt. */
void UART2_IRQ_dis(void);								/* Disable UART2 RX interrupt. */

void UART0_IRQHandler(void);							/* UART0 Interrupt Handler. */
void UART1_IRQHandler(void);							/* UART1 Interrupt Handler. */
void UART2_IRQHandler(void);							/* UART2 Interrupt Handler. */

#endif /* __SERIAL_H */
