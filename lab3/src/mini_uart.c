#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "peripherals/irq.h"
#include "utils.h"
#include "m_string.h"
#include "mini_uart.h"


void uart_send(char c) {
    
    while (1) {
        asm volatile("nop");
        if (get32(AUX_MU_LSR_REG) & 0x20)
            break;
    }
    put32(AUX_MU_IO_REG, c);
    if(c == '\n')
        uart_send('\r');
}

char uart_recv(void) {
    while (1) {
        asm volatile("nop");
        if (get32(AUX_MU_LSR_REG) & 0x01)
            break;
    }
    char ret = get32(AUX_MU_IO_REG) & 0xFF;
    return (ret=='\r')?'\n':ret;
}

void uart_send_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        uart_send((char)str[i]);
    }
}

void uart_send_hex(unsigned int *n){
    char c;
    int hex;
    uart_send_string("0x");
    for(int i=28; i>=0; i-=4){
        hex = (*n >> i) & 0xF;
        c = (hex > 9)? 'A' - 10 + hex: '0' + hex;
        uart_send(c);
    }
}

void uart_send_hex64(unsigned long long *n){
    char c;
    int hex;
    uart_send_string("0x");
    for(int i=60; i>=0; i-=4){
        hex = (*n >> i) & 0xF;
        c = (hex > 9)? 'A' - 10 + hex: '0' + hex;
        uart_send(c);
    }
}

void uart_init(void) {

    unsigned int selector;
    selector = get32(GPFSEL1);  // GPFSEL1 reg control alternative funct
    selector &= ~(7 << 12);     // clean gpio14 
    selector |= 2 << 12;        // set alt5 for gpio14
    selector &= ~(7 << 15);     // clean gpio15
    selector |= 2 << 15;        // set alt5 for gpio 15
    put32(GPFSEL1, selector);

    // remove both the pull-up/down state
    put32(GPPUD, 0);
    delay(150);
    put32(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    put32(GPPUDCLK0, 0);

    // Enable mini uart (this also enables access to its registers)
    put32(AUX_ENABLES, 1); //If this bit is set the interrupt line is asserted whenever

    // Disable auto flow control and disable receiver and transmitter (for now)
    put32(AUX_MU_CNTL_REG, 0);

    // Disable receive and transmit interrupts
    put32(AUX_MU_IER_REG, 0);
    
    // Enable 8 bit mode
    put32(AUX_MU_LCR_REG, 3);
    
    // Set RTS line to be always high
    put32(AUX_MU_MCR_REG, 0);

    // Set baud rate to 115200
    put32(AUX_MU_BAUD_REG, 270);


    put32(AUX_MU_IIR_REG, 6);

    // Finally, enable transmitter and receiver
    put32(AUX_MU_CNTL_REG, 3);

}

void uart_printf(char* fmt, ...){
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    char str[2048];
    m_vsprintf(str, fmt, args);
    char *s = str;
    while (*s) uart_send(*s++);
}

/*------------------ ASYNC -------------------*/
#define BUFFER_SIZE 32
char read_buffer[BUFFER_SIZE];
char write_buffer[BUFFER_SIZE];
int read_st = 0;
int read_ed = 0;
int write_st = 0;
int write_ed = 0;

int uart_async_readline(char* target, int len){
    int i;
    len -= 1;
    target[len] = '\0';
    for (i = 0; i < len; i++){
        while (read_st == read_ed)
            asm volatile("nop");
   
        char c = read_buffer[read_st++];
        read_st %= BUFFER_SIZE;
        if (c == '\n'){
            target[i] = '\0';
            break;
        } else{
            target[i] = c;
        }
    }
    target[i] = '\0';
    return i;
}

int uart_async_send_string(char* str) {
	if(m_strlen(str) >= BUFFER_SIZE-1)
        return 0;
	for(int i = 0; str[i] != '\0'; i++){
		if (str[i] == '\n') {
			write_buffer[write_ed++] = '\r';
			write_ed %= BUFFER_SIZE;
			write_buffer[write_ed++] = '\n';
			write_ed %= BUFFER_SIZE;
		} else {
			write_buffer[write_ed++] = str[i];
			write_ed %= BUFFER_SIZE;
		}
	}
	uart_set_transmit_interrupt(1);
	return 1;
}

void uart_set_transmit_interrupt(int enable){
    unsigned reg = get32(AUX_MU_IER_REG);
    // bcm2837 p12, enable transmit INTERRUPT 
    reg &= ~(AUX_MU_IER_REG_TRANSMIT_INTERRUPT);
    if(enable) reg |= AUX_MU_IER_REG_TRANSMIT_INTERRUPT;
    put32(AUX_MU_IER_REG, reg);
}

void uart_set_receive_interrupt(int enable){
    unsigned reg = get32(AUX_MU_IER_REG);
    // bcm2837 p12, enable receive INTERRUPT 
    reg &= ~(AUX_MU_IER_REG_RECEIVE_INTERRUPT); 
    if(enable) reg |= AUX_MU_IER_REG_RECEIVE_INTERRUPT;
    put32(AUX_MU_IER_REG, reg);
}

void uart_set_aux_interrupt(int enable){
    // bcm2837 p116, enable aux interrupt
    unsigned int reg = get32(ENABLE_IRQS_1);
    reg &= ~(AUX_INT);
    if(enable) reg |= AUX_INT;
    put32(ENABLE_IRQS_1, reg);
}


void demo_uart_async(void){
    uart_set_receive_interrupt(1);
    uart_set_aux_interrupt(1);
    char buffer[BUFFER_SIZE];
    for(int i=0; i<3; ++i){
        uart_async_send_string("Async > ");
		uart_async_readline(buffer, BUFFER_SIZE);
		uart_async_send_string("[Async recv] ");
		uart_async_send_string(buffer);
		uart_async_send_string("\r\n");
    }
    uart_set_aux_interrupt(0);
    uart_set_receive_interrupt(0);
    return ;
}

void uart_irq_write(){
    while (write_st != write_ed) {
        uart_send(write_buffer[write_st++]);
        write_st %= BUFFER_SIZE;
    }
    uart_set_transmit_interrupt(0);
}

void uart_irq_read(){
    char c = uart_recv();
    read_buffer[read_ed++] = c;
    read_ed %= BUFFER_SIZE;
    uart_send(c);
    *(unsigned int *)AUX_MU_IER_REG |= (AUX_MU_IER_REG_RECEIVE_INTERRUPT);
}



