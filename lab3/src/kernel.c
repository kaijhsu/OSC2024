#include "mini_uart.h"
#include "shell.h"
#include "peripherals/devicetree.h"
#include "utils.h"
#include "timer.h"
#include "exception.h"

void kernel_main(fdt_header *devicetree_ptr) {
	
	set_devicetree_addr(devicetree_ptr);
	set_exception_vector_table();
	uart_init();
	
	enable_core_timer();
	set_interrupt_el1(1);
	// set_timer(2);
	uart_printf("\nHello World!\n");	
	demo_uart_async();
	shell_loop();
}