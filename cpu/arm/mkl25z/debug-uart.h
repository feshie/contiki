#ifndef DEBUG_UART_H_MKL25Z__
#define DEBUG_UART_H_MKL25Z__

#ifndef dbg_setup_uart
#define dbg_setup_uart dbg_setup_uart_default
#endif

void
dbg_setup_uart(void);

void
dbg_set_input_handler(void* DBG_Callback_Ptr);

unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len);


void
dbg_putchar(const char ch);

void
dbg_blocking_putchar(const char ch);

void
dbg_drain(void);

#endif /* DEBUG_UART_H_MKL25Z__ */
