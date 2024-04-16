#ifndef	_MINI_UART_H
#define	_MINI_UART_H

#define FONT_RESET        "\x1b[0m"
#define FONT_BLACK        "\x1b[30m"
#define FONT_RED          "\x1b[31m"
#define FONT_GREEN        "\x1b[32m"
#define FONT_YELLOW       "\x1b[33m"
#define FONT_BLUE         "\x1b[34m"
#define FONT_MAGENTA      "\x1b[35m"
#define FONT_CYAN         "\x1b[36m"
#define FONT_WHITE        "\x1b[37m"
#define FONT_BOLD         "\x1b[1m"
#define FONT_ITALIC       "\x1b[3m"
#define FONT_UNDERLINE    "\x1b[4m"
#define BG_BLACK          "\x1b[40m"
#define BG_RED            "\x1b[41m"
#define BG_GREEN          "\x1b[42m"
#define BG_YELLOW         "\x1b[43m"
#define BG_BLUE           "\x1b[44m"
#define BG_MAGENTA        "\x1b[45m"
#define BG_CYAN           "\x1b[46m"
#define BG_WHITE          "\x1b[47m"

#define STYLE(str, style) style str FONT_RESET

// #define debug() asm("nop");
#define debug(fmt, ...) \
    uart_printf("["STYLE("DEBUG", FONT_CYAN)"] %s:%d, "STYLE("%s", FONT_CYAN)": "fmt"\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

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