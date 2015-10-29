#include <string.h>
#include <MKL25Z4.h>
#include "nvic.h"
#include "serial.h"
#include "debug-uart.h"


#define DBG_UART_BAUD		38400

void
dbg_set_input_handler(void* DBG_Callback_Ptr){
	UART0_reg_callback(DBG_Callback_Ptr);
	UART0_IRQ_en();
}

void
dbg_setup_uart_default()
{
	UART0_init(DBG_UART_BAUD);
}
   
unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
  unsigned int i=0;
	while(seq && *seq!=0) {
		if( i >= len) { break; }
		dbg_putchar(*seq++); i++;
	}
	return i;
}

static unsigned char dbg_write_overrun = 0;

void
dbg_putchar(const char ch)
{
  UART0_PutChar(ch);
}

void
dbg_blocking_putchar(const char ch)
{
  UART0_PutChar(ch);
}

