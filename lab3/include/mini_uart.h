#ifndef	_MINI_UART_H
#define	_MINI_UART_H

#define debug() uart_printf("%s:%d, %s\n", __FILE__, __LINE__, __FUNCTION__)

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_send_string(char* str);
void uart_send_hex(unsigned int *n);
void uart_send_hex64(unsigned long long *n);
void uart_printf(char* fmt, ...);

void uart_set_receive_interrupt(int enable);
void uart_set_transmit_interrupt(int enable);
void uart_set_aux_interrupt(int enable);
void uart_irq_handler();
void demo_uart_async(void);
#endif  /*_MINI_UART_H */