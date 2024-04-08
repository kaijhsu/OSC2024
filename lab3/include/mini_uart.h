#ifndef	_MINI_UART_H
#define	_MINI_UART_H

// #define debug() asm("nop");
#define debug(fmt, ...) \
    uart_printf("\n[DEBUG] %s:%d, %s: "fmt"\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

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
void uart_irq_read();
void uart_irq_write();
void demo_uart_async(void);
#endif  /*_MINI_UART_H */