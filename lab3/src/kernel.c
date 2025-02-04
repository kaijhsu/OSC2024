#include "mini_uart.h"
#include "shell.h"
#include "peripherals/devicetree.h"
#include "utils.h"
#include "timer.h"
#include "exception.h"

void kernel_main(fdt_header *devicetree_ptr) {
	debug("");
	set_devicetree_addr(devicetree_ptr);
	set_exception_vector_table();
	uart_init();
	uart_printf("\nHello World!\n");
	timer_init();
	timer_add(2, print_cpu_time, 0);
	set_interrupt_el1(1);
	shell_loop();
}